#ifndef HTTPVO_IMPLEMENTATION_SCALAR_HPP
#define HTTPVO_IMPLEMENTATION_SCALAR_HPP
#include "implementation.hpp"

namespace httpvo::Implementation
{
    using namespace simd;
    static constexpr u8_t CR  = '\xd';
    static constexpr u8_t LF  = '\xa';
    static constexpr u8_t TAB = '\x9';
    static constexpr u8_t DEL = '\x7f';
    static constexpr u8_t COL = '\x3a';
    static constexpr u8_t NON_PRINTABLE_MAX = '\x19';

    enum Character : u8_t {
        HorizontalTab  = '\x09',
        LineFeed       = '\x0a',
        CarriageReturn = '\x0d',
        NonPrintMax    = '\x19',
        Space          = '\x20',
        Colon          = '\x3a',
        Delete         = '\x7f',
    };

    inline bool is_valid(u8_t i) { return i > Character::Space and i < Character::Delete; }

    inline status parse_char(const u8_t * const b, ReqLine& out, u64_t& tsp, const std::size_t run_size, const std::size_t i)
    {
        // TODO: status sign changes: fix return values
        const std::size_t n_i = i + 1;
        u8_t c   = b[i];
        if (is_valid(c)) [[likely]]
        {
            tsp = false;
            return {n_i < run_size, status::expect_bytes};
        }
        if (common::is_whitespace(c))
        {
            if (tsp) [[unlikely]]
                return -1;
            tsp = true;
            return {out.advance(i) and n_i < run_size, status::bad_whitespace};
        }
        if (Character::CarriageReturn == c)
        {
            if (n_i == run_size)
                return {0, status::expect_line_feed};
            c = b[n_i];
        }
        if (Character::LineFeed == c)
        {
            out.advance(i);
            return out.set_version(b);
        }
        return status::unexpected_char;
    }

    inline status parse_chars(const u8_t * const b, ReqLine& out, u64_t &tsp, std::size_t run_size, std::size_t i)
    {
        status s {0};
        if ((s = parse_char(b, out, tsp, run_size, i+0)) < 1) return s;
        if ((s = parse_char(b, out, tsp, run_size, i+1)) < 1) return s;
        if ((s = parse_char(b, out, tsp, run_size, i+2)) < 1) return s;
        if ((s = parse_char(b, out, tsp, run_size, i+3)) < 1) return s;
        if ((s = parse_char(b, out, tsp, run_size, i+4)) < 1) return s;
        if ((s = parse_char(b, out, tsp, run_size, i+5)) < 1) return s;
        if ((s = parse_char(b, out, tsp, run_size, i+6)) < 1) return s;
        if constexpr (setup::no_vectorize)
            return parse_char(b, out, tsp, run_size, i+7);
        return s;
    }

    template<>
    inline status http::parse_line<0, 0>(u8_t * const b, u8_t *p, ReqLine& out, const std::size_t run_size, const mask_t mask [[maybe_unused]], u64_t tsp)
    {
        // TODO: OPTIMIZE THIS FUNCTION
        status s{0};
        for (std::size_t i = 0; true; i += 16)
        {
            if ((s = parse_chars(b, out, tsp, run_size, i+0)) < 1)
                return s;
            if ((s = parse_chars(b, out, tsp, run_size, i+8)) < 1)
                return s;
        }
        return s;
    }

    template<simd::VWidth N=8, bool ISTRAIL=0>
    [[nodiscard]] inline status http::parse_line(u8_t * const b, u8_t *p, ReqLine& out, const std::size_t run_size, const mask_t mask, u64_t tsp) noexcept
    {
        static_assert(bits::is_power_of_two(N));

        if constexpr (setup::no_vectorize)
            return parse_line<0, 0>(b, p, out, run_size, mask, tsp);
 
        const u8_t * const end  = b + run_size;
        const u8_t * const stop = not ISTRAIL ? b + bits::align(run_size, N) : end;
        for (; p < stop; p += N)
        {
            simdv<N> v{p};
            mask_t out_mask = v.template cmpngt_lt<0x21, 0x7e>().to_bitmask() & mask;
            if (not out_mask) [[likely]]
            {
                tsp = 0;
                continue;
            }
            for (; out_mask; out_mask = bits::clear_least_set_bit(out_mask))
            {
                const u8_t * const q = p + simdv<N>::countzero_bitmask(out_mask);
                const u8_t c = q[0];
                if (common::is_whitespace(c)) [[likely]]
                {
                    if (tsp & out_mask) [[unlikely]]
                    {
                        if constexpr (setup::no_multispace)
                            return status::bad_whitespace;
                    }
                    else if (out.advance(static_cast<const std::ptrdiff_t>(q - b)) == status::excess) [[unlikely]]
                        return status::expected_end_of_line;
                    tsp = bits::least_set_bit(out_mask) << simdv<N>::bitpos;
                    continue;
                }
                if (Character::CarriageReturn == c)
                {
                    if (q == (end - 1)) [[unlikely]]
                        return {0, status::expect_line_feed};
                    if (Character::LineFeed != q[1]) [[unlikely]]
                        return status::unexpected_char;
                    out.advance(static_cast<const std::ptrdiff_t>(q - b));
                    return out.set_version(b);
                }
                if constexpr (setup::end_of_line_is_line_feed)
                {
                    if (Character::LineFeed == c)
                    {
                        out.advance(static_cast<const std::ptrdiff_t>(q - b));
                        return out.set_version(b);
                    }
                }
                return status::unexpected_char;
            }
            tsp = bool(tsp) << (simdv<N>::bitpos - 1);
        }
        if (run_size > (N - 1)) [[likely]]
        {
            // process trailing bytes by overlapping last read bytes with the remaining bytes
            const std::size_t r = N - (run_size & (N - 1));
            const std::size_t s = r * simdv<N>::bitpos;
            return parse_line<N, true>(b, p - r, out, run_size, constant::cff << s, tsp << s);
        }
        // run_size < N
        if constexpr (N > 8)
            return parse_line<8, false>(b, p, out, run_size, constant::cff, 0);
        // run_size < 8
        return parse_chars(b, out, tsp, run_size, p - b);
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
        // since the control bytes overlap, we can reduce to a simple nibble comparison. see /test/scripts/generate_shuffle_table2.py for table generation and comments
        const simdv<N> CTRL_LO = simdv<N>::load_tbl1(CONTROL_CHAR_CLASS_LUT + 00);
        const simdv<N> CTRL_HI = simdv<N>::load_tbl1(CONTROL_CHAR_CLASS_LUT + 16);

        if constexpr (setup::neon)
        {
            if constexpr (not setup::arm64)
                return ((v <= Character::NonPrintMax) | (v == Character::Delete)) ^ (v == Character::HorizontalTab); // no single 16B lookup (vqtbl1q). shuffle is not worth it since we have to do two shuffles for each lane halves.
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
        const int _is_not_end_of_line = (b[0] != Character::CarriageReturn) & (b[1] != Character::LineFeed);
        e = (b[2] == Character::CarriageReturn and b[3] == Character::LineFeed);
        return _is_not_end_of_line;
    }

    inline int http::is_not_end_of_line(const u8_t * const b, const std::size_t rem)
    {
        if constexpr (setup::end_of_line_is_line_feed)
            pass(); // TODO

        if (rem > 3) [[likely]]
        {
            if constexpr (not setup::no_vectorize and setup::little_endian)
                return is_not_end_of_line_v(b, this->end_of_parse);
            return is_not_end_of_line_p(b, this->end_of_parse);
        }
        if (rem and b[1] != Character::LineFeed) [[unlikely]]
            return true;
        return b[0] != Character::LineFeed;
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
                out.set_name_end(static_cast<std::size_t>(q - b), nullptr);
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
    
    // 10 - name token; 11 = value token
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

    inline std::size_t is_valid_name_token_(const u8_t * const b)
    {
        std::size_t at = 0;
        for (; at < 8 and TCHAR_LUT[b[at]] & 1; at++) pass();
        return at;
    }

    inline std::size_t is_valid_name_token_l(const u8_t * const b, const std::size_t end, size_t at)
    {
        for (; at < end and TCHAR_LUT[b[at]] & 1; at++) pass();
        return at;
    }

    make_flat inline std::size_t is_valid_name_token(const u8_t * const b)
    {
        auto fb_valid_token = [](const u8_t *const b, const std::size_t off)
        { return is_valid_name_token_l(b, 8 - off, off); };
        if constexpr (setup::optimize_for_most_case and not setup::no_vectorize)
        {
            const simdv<8> v{b};
            if (const mask_t outside = bits::andnot(constant::c80, v.ascii_fast_tchar())) [[unlikely]] // some other char that is not A-Z0-9 or -, but may be valid
                return fb_valid_token(b, simdv<8>::countzero_bitmask(outside));
            return 8;
        }
        return is_valid_name_token_(b);
    }

    make_flat inline std::size_t is_valid_name(const u8_t * const b, const u8_t * const end, const std::size_t len)
    {
        for (const u8_t *p = b; p < end; p += 8)
        {
            if (std::size_t s = is_valid_name_token(p); s < 8)
                return s + static_cast<std::ptrdiff_t>(p - b);
        }
        if (const std::size_t r = len % 8)
            return is_valid_name_token_l(b, len - r, len);
        return len;
    }

    inline std::size_t is_valid_value_token_l(const u8_t *const b, const size_t len)
    {
        std::size_t at = 0;
        for (; at < len and TCHAR_LUT[b[at]]; at++) pass();
        return at;
    }

    inline std::size_t is_valid_value_token(const u8_t *const b)
    {
        if constexpr (setup::no_vectorize)
            return is_valid_value_token_l(b, 8);
        if (const mask_t offset = simdv<8>(b).non_printable_andn_tab())
            return simdv<8>::countzero_bitmask(offset);
        return 8;
    }

    make_flat inline std::size_t is_valid_value(const u8_t * const b, const u8_t *p, const u8_t * const end, const std::size_t len)
    {
        for (; p < end; p += 8)
        {
            if (const std::size_t offset = is_valid_value_token(p); offset < 8)
                return static_cast<std::ptrdiff_t>(p - b) + offset;
        }
        if (const std::size_t r = len % 8) [[likely]]
        {
            if (len > 8) [[likely]]
            {
                const u8_t * const q = end - (8 - r);
                if (const std::size_t offset = is_valid_value_token(q); offset < 8)
                    return static_cast<std::ptrdiff_t>(q - b) + offset;
                return len;
            }
            return is_valid_value_token_l(end - r, r);
        }
        return len;
    }

    template<>
    [[nodiscard]] inline status http::parse_header<0>(u8_t * const b, u8_t *b_run [[maybe_unused]], header_view& out, const std::size_t in_size [[maybe_unused]], const std::size_t run_size)
    {  
        const u8_t * const end = b + bits::align(run_size, 8);
        const std::size_t n_offset = is_valid_name(b, end, run_size);

        if (n_offset == run_size) [[unlikely]]
            return status::expect_bytes;
        if (b[n_offset] != Character::Colon) [[unlikely]]
            return status::unexpected_char;
        out.set_name_end(n_offset, nullptr);
        
        const size_t v_offset = is_valid_value(b, b + n_offset, end, run_size);
        if (v_offset == run_size)
            return status::expect_bytes;
        if (is_not_end_of_line(b + v_offset, run_size - v_offset))
            return status::unexpected_char;
        return out.set_value_end(v_offset, reinterpret_cast<const u8_t *>(b));
    }
} // NAMESPACE::HTTVO
#endif // HTTPVO_IMPLEMENTATION_SCALAR_HPP