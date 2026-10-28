#ifndef HTTPVO_IMPLEMENTATION_SCALAR_HPP
#define HTTPVO_IMPLEMENTATION_SCALAR_HPP
#include "implementation.hpp"

namespace httpvo::Implementation
{
    using namespace simd;

    enum Character : u8_t {
        HorizontalTab  = '\x09',
        LineFeed       = '\x0a',
        CarriageReturn = '\x0d',
        NonPrintMax    = '\x19',
        Space          = '\x20',
        Colon          = '\x3a',
        Delete         = '\x7f',
    };

    inline bool is_whitespace(const int c)
    {
        return c == Character::Space or c == Character::HorizontalTab;
    }

    // see test/scripts/generate_shuffle_table.py for table generation and comments.
    alignas(16) static constexpr u8_t TCHAR_CLASS_LUT[32]{
        58, 63, 62, 63, 63, 63, 63, 63, 62, 62, 61, 21, 52, 21, 61, 28,
        00, 00, 01, 02, 04,  8, 16, 32, 00, 00, 00, 00, 00, 00, 00, 00
    };

    // see test/scripts/generate_shuffle2_table.py for table generation and comments.
    alignas(16) static constexpr u8_t CONTROL_CHAR_CLASS_LUT[32]{
        002, 002, 002, 002, 002, 002, 002, 002, 002, 001, 002, 002, 002, 002, 002, 003,
        001, 000, 127, 127, 127, 127, 127, 002, 127, 127, 127, 127, 127, 127, 127, 127,
    };

    template<simd::VWidth N>
    inline mask_t is_control_char(const simdv<N>& v)
    {
        const simdv<N> CTRL_LO = simdv<N>::load_tbl1(CONTROL_CHAR_CLASS_LUT + 00);
        const simdv<N> CTRL_HI = simdv<N>::load_tbl1(CONTROL_CHAR_CLASS_LUT + 16);
        if constexpr (setup::neon)
        {
            // no single 16B lookup (vqtbl1q). shuffle is not worth it since we have to do two shuffles for each lane halves.
            if constexpr (not setup::arm64)
                return ((v <= Character::NonPrintMax) | (v == Character::Delete)) ^ (v == Character::HorizontalTab); 
            // we have vqtbl1q
            return (v & 0xf).shuf_table(CTRL_LO) > (v >> 4).shuf_table(CTRL_HI);
        }    
        return v.shuf_table(CTRL_LO) > ((v >> 4) & 0xf).shuf_table(CTRL_HI);
    }

    template<simd::VWidth N>
    inline mask_t is_non_tchar(const simdv<N>& v)
    {
        const simdv<N> TCHAR_LO = simdv<N>::load_tbl1(TCHAR_CLASS_LUT + 00);
        const simdv<N> TCHAR_HI = simdv<N>::load_tbl1(TCHAR_CLASS_LUT + 16);

        if constexpr (setup::neon)
           return (v & 0xf).shuf_table(TCHAR_LO).andeqz((v >> 4).shuf_table(TCHAR_HI));
        // skipped first 'v & 0xf' since v >= 0x80 falls to 0 and that is what we need
        return v.shuf_table(TCHAR_LO).andeqz(((v >> 4) & 0xf).shuf_table(TCHAR_HI));
     }

    inline int is_not_end_of_line_v(const u8_t * const b, auto& e)
    {
        static constexpr u32_t CRLF = 0x0a0d, CRLFCRLF = 0x0a0d0a0d;
        const u32_t v = simdv<8>::_load_u32(b);
        e = (v == CRLFCRLF);
        return (v & 0xffffu) != CRLF;
    }

     inline int is_not_end_of_line_p(const u8_t * const b, auto& e)
    {
        const int _is_not_end_of_line = (b[0] != Character::CarriageReturn) | (b[1] != Character::LineFeed);
        e = (b[2] == Character::CarriageReturn and b[3] == Character::LineFeed);
        return _is_not_end_of_line;
    }

    inline int http::is_not_end_of_line(const u8_t * const b, const std::size_t rem) noexcept
    {
        // TODO: special function for mixed crlf and lf as end of line char. Fix return values not matching status code
        if constexpr (setup::end_of_line_is_line_feed)
            pass();

        if (rem > 3) [[likely]]
        {
            if constexpr (not setup::no_vectorize and setup::little_endian)
                return is_not_end_of_line_v(b, this->end_of_parse);
            return is_not_end_of_line_p(b, this->end_of_parse);
        }
        if (not rem) [[unlikely]]
            return status::expect_bytes | (b[0] != Character::CarriageReturn); 
         return (b[0] != Character::CarriageReturn) | (b[1] != Character::LineFeed);
    }

    template<simd::VWidth N, bool ISTRAIL=0>
    [[nodiscard]] inline status http::parse_header(u8_t * const b, u8_t *p, header_view& out, const std::size_t in_size, const std::size_t run_size) noexcept
    {
        static_assert(N == 16 or N == 32);
        const u8_t * const stop = b + (not ISTRAIL ? bits::align(run_size, N) : run_size);
        bool parsing_value = false;

        for (; p < stop; p += N)
        {
            simdv<N> v(p);
            if (not parsing_value)
            {
                mask_t non_tchar = is_non_tchar(v);
                if (not non_tchar)
                    continue;
                const u8_t * const q = p + simdv<N>::countzero_bitmask(non_tchar);
                if (q[0] != Character::Colon) [[unlikely]]
                    return status::unexpected_char;
                out.set_name_end(static_cast<std::ptrdiff_t>(q - b), nullptr);
                parsing_value = true;
            }
            if (mask_t control_char = is_control_char(v))
            {
                const u8_t * const q = p + simdv<N>::countzero_bitmask(control_char);
                const std::size_t q_offset = static_cast<std::ptrdiff_t>(q - b);
                if (is_not_end_of_line(q, run_size - q_offset)) [[unlikely]]
                    return status::unexpected_char;
                return out.set_value_end(q_offset, b);
            }
        }
        return status::expect_bytes;
    }
    
    // 2 - name token; 3 = value token
    static constexpr u8_t TCHAR_LUT[256]{
        0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2,
        2, 3, 2, 3, 3, 3, 3, 3, 2, 2, 3, 3, 2, 3, 3, 2,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2,
        2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 3, 3,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 2, 0, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};

    inline std::size_t _offset_of_non_tchar(const u8_t * const b)
    {
        std::size_t i = 0;
        for (; i < 8 and TCHAR_LUT[b[i]] & 1; i++) pass();
        return i;
    }

    inline std::size_t _offset_of_non_tchar_at(const u8_t * const b, const std::size_t i, const std::size_t len)
    {
        std::size_t j = i;
        for (; j < len and TCHAR_LUT[b[j]] & 1; j++) pass();
        return j;
    }

    make_flat inline std::size_t offset_of_non_tchar(const u8_t * const b)
    {
        if constexpr (setup::optimize_for_most_case and not setup::no_vectorize)
        {
            const simdv<8> v{b};
            if (const mask_t outside = v.ascii_fast_tchar() ^ constant::c80) [[unlikely]] // some other char that is not A-Z0-9 or -, but may be valid
                return _offset_of_non_tchar_at(b, simdv<8>::countzero_bitmask(outside), 8);
            return 8;
        }
        return _offset_of_non_tchar(b);
    }

    make_flat inline std::size_t find_non_tchar(const u8_t * const b, const std::size_t len)
    {
        if (len < 8) return _offset_of_non_tchar_at(b, 0, len);

        const u8_t * const end = b + (len - 8);
        for (const u8_t *p = b; p < end; p += 8)
        {
            if (std::size_t s = offset_of_non_tchar(p); s < 8)
                return s + static_cast<std::ptrdiff_t>(p - b);
        }
        return offset_of_non_tchar(end) + (len - 8);
    }

    inline std::size_t _offset_of_bad_char_in_value(const u8_t *const b, const std::size_t i, const std::size_t len)
    {
        std::size_t j = i;
        for (; j < len and TCHAR_LUT[b[j]]; j++) pass();
        return j;
    }

    inline std::size_t offset_of_bad_char_in_value(const u8_t *const b)
    {
        if constexpr (setup::no_vectorize)
            return _offset_of_bad_char_in_value(b, 0, 8);
        if (const mask_t offset = simdv<8>(b).non_printable_andn_tab())
            return simdv<8>::countzero_bitmask(offset);
        return 8;
    }

    make_flat inline std::size_t find_bad_char_in_value(const u8_t * const b, const std::size_t i, const std::size_t len)
    {
        if (len < 8) return _offset_of_bad_char_in_value(b, i, len);

        const u8_t * const end = b + (len - 8);
        for (const u8_t *p = b + i; p < end; p += 8)
        {
            if (const std::size_t offset = offset_of_bad_char_in_value(p); offset < 8)
                return static_cast<std::ptrdiff_t>(p - b) + offset;
        }
        return offset_of_bad_char_in_value(end) + (len - 8);
    }

    inline status::statat_t find_request_method_end(const u8_t * const b, const std::size_t run_size, ReqLine& out)
    {
        if (const std::size_t offset = find_non_tchar(b, run_size))
         {
            if (not is_whitespace(b[offset])) [[unlikely]]
                return status::unexpected_char;
            out.advance(offset);
            return {0, offset};
         }
        return status::expect_bytes;
    }

    template <simd::VWidth N>
    inline std::size_t is_in_range_20_7e(const u8_t *p, mask_t &mask)
    {
        if (const mask_t out_of_range = simdv<N>(p).template cmpngt_lt<0x21, 0x7e>().to_bitmask()) [[unlikely]]
        {
            mask = out_of_range;
            return simdv<N>::countzero_bitmask(mask);
        }
        return 8;
    };

    inline bool is_in_range_20_7e_s(int c)
    {
        // c > 0x20 and c < 0x7e
        return static_cast<i8_t>(c + 0x5f) < -0x22;
    }
    
    template <simd::VWidth N>
    inline std::size_t find_request_url_end_(const u8_t *const b, const u8_t *p)
    {
        for (const u8_t *const stop = b + (run_size - N); p < stop; p += N)
        {
            if (const std::size_t offset = is_in_range_20_7e<N>(p); offset < 8)
                return static_cast<std::ptrdiff_t>(p - b) + offset;
        }
        return is_in_range_20_7e<N>(stop) + (run_size - N);
    }

    template <simd::VWidth>
    inline status::statat_t find_request_url_end(const u8_t *const b, const u8_t *p, const std::size_t run_size, mask_t& mask)
    {
        if (run_size < (N - 1)) [[unlikely]]
        {
            if constexpr (N > 8)
                return find_request_url_end<8>(b, p, run_size);
            size_t i = 0;
            for (; is_in_range_20_7e_s(static_cast<i8_t>(p[i])); i++)
                return i;
        }
        for (;;)
        {
            std::mask_size_t mask = 0; // set in @find_request_url_end_
            const std::size_t offset = find_request_url_end_(b, p, mask);
            if (offset == run_size) [[unlikely]]
                return status::expect_bytes;
            while (mask)
            {
                const std::size_t i = simdv<N>::countzero_bitmask(mask);
                if (is_whitespace(p[0]))
                {
                    if (0) [[unlikely]]
                        continue;
                }
                return status::unexpected_char;
            }
        }

        for (const u8_t *const stop = b + (run_size - N); p < stop; p += N)
        {
            if (const mask_t out_of_range = simdv<N>(p).template cmpngt_lt<0x21, 0x7e>().to_bitmask()) [[unlikely]]
                if (status s = find_request_url_end_(b, p, mask))
                    return s;
        }
        if (mask_t out_of_range = simdv<N>(p).template cmpngt_lt<0x21, 0x7e>().to_bitmask()) [[unlikely]]
            return find_request_url_end_(b, p, mask);
        return status::expect_bytes;
    }

    template<simd::VWidth N=8, bool ISTRAIL=0>
    [[nodiscard]] inline status parse_request(u8_t * const b, u8_t *p, ReqLine& out, const std::size_t run_size, const mask_t mask, u64_t tsp) noexcept
    {
        const status::statat_t meth_end = find_request_method_end(b, run_size, out);
        if (meth_end == 0)
            return meth_end;
        return url_end = find_request_url_end(b, p + meth_end.offset, run_size);
    }

    template<simd::VWidth N=8, bool ISTRAIL=0>
    [[nodiscard]] inline status parse_response(u8_t * const b, u8_t *p, ReqLine& out, const std::size_t run_size, const mask_t mask, u64_t tsp) noexcept
    {
        // TODO
        return 0;
    }

    template<>
    [[nodiscard]] inline status http::parse_header<0>(u8_t * const b, u8_t *b_run [[maybe_unused]], header_view& out, const std::size_t in_size [[maybe_unused]], const std::size_t run_size) noexcept
    {  
        const std::size_t n_offset = find_non_tchar(b, run_size);
        if (n_offset == run_size) [[unlikely]]
            return status::expect_bytes;
        if (b[n_offset] != Character::Colon) [[unlikely]]
            return status::unexpected_char;
        out.set_name_end(n_offset, nullptr);
        
        const size_t v_offset = find_bad_char_in_value(b, n_offset, run_size);
        if (v_offset == run_size)
            return status::expect_bytes;
        if (is_not_end_of_line(b + v_offset, run_size - v_offset))
            return status::unexpected_char;
        return out.set_value_end(v_offset, reinterpret_cast<const u8_t *>(b));
    }
} // NAMESPACE::HTTVO
#endif // HTTPVO_IMPLEMENTATION_SCALAR_HPP