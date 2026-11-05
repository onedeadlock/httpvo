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

    template<typename T, bool SPCONLY>
    inline T is_whitespace(const T& c)
    {
        static_assert(std::is_integral<T>() or simd::is_simd_v<T>());
        if constexpr (SPCONLY) return c == Character::Space;
        else return (c == Character::Space) | (c == Character::HorizontalTab);
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
            if constexpr (not setup::arm64)
                return ((v <= Character::NonPrintMax) | (v == Character::Delete)) ^ (v == Character::HorizontalTab); // no vqtbl1q
            else 
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

    inline int http::end_of_parse(const u32_t v) noexcept
    {
        constexpr u32_t CRLF = 0x0a0d;
        is_end_of_parse_marker = v == (CRLF | (CRLF << 16));
        return (v & 0xffffu) == CRLF;
    }

    inline int http::end_of_parse_single_load(const u8_t * const b) noexcept
    {
        if constexpr (setup::little_endian and not setup::no_vectorize)
            return end_of_parse(simdv<8>::_load_u32(b));
        else
        {
#           define USH(c, n) (static_cast<u32_t>(c) << (n))
            // GCC and clang should fold these shifts into a single DWORD load (with a bswap in Big Endian mode)
            const u32_t LD = USH(b[0], 0) | USH(b[1], 8) | USH(b[2], 16) | USH(b[3], 24);
            return end_of_parse(LD);
#           undef USH
         }
    }
     
    inline int http::is_not_end_of_line(const u8_t * const b, const std::size_t rem) noexcept
    {
        // :: assume rem >= 2
        if constexpr (setup::end_of_line_is_line_feed)
            pass(); // TODO
        if (rem > 3) [[likely]]
            return not is_end_of_parse_single_load(b);
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
            simdv<N> v{p};
            if (not parsing_value)
            {
                mask_t non_tchar = is_non_tchar<N>(v);
                if (not non_tchar)
                    continue;
                const u8_t * const q = p + simdv<N>::countzero_bitmask(non_tchar);
                if (q[0] != Character::Colon) [[unlikely]]
                    return status::unexpected_char;
                out.set_name_end(static_cast<std::size_t>(q - b), nullptr);
                parsing_value = true;
            }
            if (mask_t control_char = is_control_char<N>(v))
            {
                const u8_t * const q = p + simdv<N>::countzero_bitmask(control_char);
                const std::size_t q_offset = static_cast<std::size_t>(q - b);
                if (is_not_end_of_line(q, run_size - q_offset)) [[unlikely]]
                    return status::unexpected_char; // TODO: end_of_line function changed
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

    inline std::size_t _offset_of_non_tchar(const u8_t * const b, const std::size_t i, const std::size_t len)
    {
        std::size_t j = i;
        for (; j < len and TCHAR_LUT[b[j]] & 1; j++) pass();
        return j;
    }

    inline std::size_t _offset_of_non_tchar_mask(const u8_t * const b, const mask_t mask)
    {
        for (mask_t m = mask; m; m = bits::clear_least_set_bit(m))
            if (const std::size_t offset = simdv<8>::countzero_bitmask(m); TCHAR_LUT[b[offset]] & 1) return offset;
        return 8;
    }

    template<bool ALPHAONLY>
    make_flat inline std::size_t offset_of_non_tchar(const u8_t * const b)
    {
        if constexpr (not setup::no_vectorize)
        {
            if (const mask_t mask = simdv<8>(b).fast_not_tchar<ALPHAONLY>()) [[unlikely]] // fast pre-check
                return _offset_of_non_tchar_mask(b, mask);
            return 8;
        }
        return _offset_of_non_tchar(b, 0, 8);
    }

    template<bool ALPHAONLY>
    make_flat inline std::size_t find_non_tchar(const u8_t * const b, const std::size_t len)
    {
        if (setup::optimize_for_most_case or len < 8)
            return _offset_of_non_tchar(b, 0, len);
        const u8_t * const end = b + (len - 8);
        for (const u8_t *p = b; p < end; p += 8)
        {
            if (const std::size_t offset = offset_of_non_tchar<ALPHAONLY>(p); offset < 8)
                return offset + static_cast<std::size_t>(p - b);
        }
        return offset_of_non_tchar<ALPHAONLY>(end) + (len - 8);
    }

    inline std::size_t _offset_of_bad_char_in_value(const u8_t * const b, const std::size_t i, const std::size_t len)
    {
        std::size_t j = i;
        for (; j < len and TCHAR_LUT[b[j]]; j++) pass();
        return j;
    }

    inline std::size_t offset_of_bad_char_in_value(const u8_t *const p)
    {
        if constexpr (setup::no_vectorize)
            return _offset_of_bad_char_in_value(p, 0, 8);
        if (const mask_t offset = simdv<8>(p).is_not_print_char())
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
                return static_cast<std::size_t>(p - b) + offset;
        }
        return offset_of_bad_char_in_value(end) + (len - 8);
    }

    inline status::statat_t find_request_method_end(const u8_t * const b, const std::size_t run_size, ReqLine& out)
    {
        if (const std::size_t offset = find_non_tchar<1>(b, run_size)) [[likely]]
        {
            if (not is_whitespace<u8_t, true>(b[offset])) [[unlikely]]
                return status::unexpected_char;
            return {out.advance(offset), offset};
        }
        out.meth = true;
        return status::expect_bytes;
    }

    template <int X, int Y, simd::VWidth N>
    inline std::size_t is_out_of_range(const u8_t * const p)
    {
        static_assert(X == 0x21 and Y == 0x7e);
        if (const mask_t mask = simdv<N>(p).template cmpngt_lt<X, Y>()) [[unlikely]]
            return simdv<N>::countzero_bitmask(mask);
        return N;
    };

    template<>
    inline std::size_t is_out_of_range<0x21, 0x7e, 1>(const u8_t * const p)
    {
        static_assert(static_cast<i8_t>(0x80) == static_cast<i8_t>(-128));
        return static_cast<const i8_t>(*p + 0x5f) > -0x23;
    }

    template<simd::VWidth N>
    inline std::size_t first_non_whitespace(const u8_t * const p)
    {
         constexpr mask_t ALL_WHITESPACE = bits::mask_fill<mask_t>(N, simdv<N>::bitpos);
         const mask_t mask = is_whitespace<simdv<N>, true>(simdv<N>(p));
         if (mask != ALL_WHITESPACE) [[likely]]
              return simdv<N>::countzero_bitmask(mask);
          return N;
}

    template<simd::VWidth N>
    inline const u8_t * trim_whitespace(const u8_t *p, const u8_t * const end)
    {
        if (not is_whitespace<u8_t, true>(p[0])) [[likely]]
            return p;
        if constexpr (setup::no_multispace)
            return end;
        else
        {
              const u8_t * const stop = end - N;
              for (; p < stop; p += N)
              {
                   const std::size_t offset = first_non_whitespace<N>(p);
                   if (offset < N) return p + offset;
               }
               return stop + first_non_whitespace<N>(stop);
         }
    }

    template<>
    inline const u8_t * trim_whitespace<1>(const u8_t *p, const u8_t * const end)
    {
        if (is_whitespace<u8_t, true>(p[0]))
        {
            if constexpr (setup::no_multispace) return end;
            else while (++p < end and is_whitespace<u8_t, true>(p[0])) pass();
        }
        return p;
    }

    template <simd::VWidth N>
    inline const u8_t * _find_request_url_end(const u8_t * const b, const u8_t *p, const std::size_t len)
    {
        const u8_t * const stop = b + (len - N);
        for (; p < stop; p += N)
        {
            if (const std::size_t offset = is_out_of_range<0x21, 0x7e, N>(p); offset < N)
                return p + offset;
        }
        return stop + is_out_of_range<0x21, 0x7e, N>(stop);
    }

    template <>
    inline const u8_t * _find_request_url_end<1>(const u8_t * const b, const u8_t *p, const std::size_t len)
    {
        for ( const u8_t *const stop = b + len; p < stop and not is_out_of_range<0x21, 0x7e, 1>(p); p += 1)
            pass();
        return p;
    }

    inline status::statat_t __status_bad_or_expect(const u8_t * const b, const u8_t * const p)
    {
        if constexpr (setup::no_multispace)
            return {status::bad_whitespace, static_cast<std::size_t>(p - b)};
        else return {status::expect_bytes,  static_cast<std::size_t>(p - b)};
    }
   
    template <simd::VWidth N>
    inline status::statat_t _find_request_url_end_main(const u8_t * const b, const u8_t *p, const std::size_t run_size, ReqLine& out)
    {
        const u8_t * const end = b + run_size;
        if (p = trim_whitespace<N>(p, end); p == end) [[unlikely]]
            return __status_bad_or_expect(b, p);

        p = _find_request_url_end<N>(b, p, run_size);
        if  (p == end) [[unlikely]]
            return {status::expect_bytes, run_size};
        if (not is_whitespace<u8_t, true>(p[0])) [[unlikely]]
            return {status::unexpected_char, static_cast<std::size_t>(p - b)};
        p += 1;
        if (p < end and (p = trim_whitespace<N>(p, end)) == end) [[unlikely]]
            return __status_bad_or_expect(b, p);
        const std::size_t offset = static_cast<std::size_t>(p - b);
        return {out.advance(offset), offset};
    }
   
    template <simd::VWidth N>
    inline status::statat_t find_request_url_end(const u8_t * const b, const u8_t *p, const std::size_t run_size, ReqLine& out)
    {
        /* TODO: check run_size logic. run_size here is not actual buffer size but the buffer_size - method end offset */
        if constexpr (setup::no_vectorize)
            return _find_request_url_end_main<1>(b, p, run_size, out);
        else
        {
            if (run_size < N)
            {
                if constexpr (N > 8) return _find_request_url_end_main<8>(b, p, run_size, out);
                else return _find_request_url_end_main<1>(b, p, run_size, out);
            }
            return _find_request_url_end_main<N>(b, p, run_size,out);
        }
    }

    template<int TYPE>
    inline int is_protocol(const u64_t proto)
    {
        constexpr u64_t http_slash_one_dot = 0x2e312f5054544800ULL;
        constexpr u64_t rtsp_slash_one_dot = 0x2e312f5053545200ULL;
        constexpr u64_t rtsp_slash_two_dot = 0x2e322f5053545200ULL;
        if constexpr (TYPE == setup::rtsp)
            return (proto == rtsp_slash_two_dot) | (protocol == rtsp_slash_one_dot);
        else return proto == http_slash_one_dot;
    }

    template<int TYPE>
    inline int is_expected_protocol(const u8_t * const b)
    {
        
        if constexpr (setup::little_endian and not setup::no_vectorize)
            return is_protocol<TYPE>(simd::simdv<8>::_load_u64(b) << 8);
        else
        {
#           define USH(c, n) (static_cast<u32_t>(c) << (n))
            // Again, GCC and clang should fold these shifts into a single QWORD load (with a bswap on Big Endian mode). It is not the case for MSVC, but the cost against byte compares is almost the same.
            // @discard_reginfo: clang sees that byte 7 is unused, discards it, and so emit 3 loads (first 4 bytes, next two bytes and the last byte) instead of 1 for the 8 bytes.
            u64_t LD = 0;
            LD |= static_cast<u64_t>(USH(b[0], 0U) | USH(b[1], 8U) | USH(b[2], 16) | USH(b[3], 24));
            LD |= static_cast<u64_t>(USH(b[4], 32) | USH(b[5], 40) | USH(b[6], 48) | USH(b[7], 56));
            httpvo_discard_reginfo(LD);
            return is_protocol<TYPE>(LD << 8);
#           undef USH
        }
    }
    
    inline int is_expected_minor_version(const int minor, ReqLine& out)
    {
         return (out.set_version = minor ^ 0x30) < 10;
    }

    template<int TYPE>
    inline int set_version(const u8_t * const b, ReqLine& out)
    {
        // :: assume len(b) >= 8
        return is_expected_protocol<TYPE>(b) & is_expected_minor_version(b[7], out);
    }

    template<simd::VWidth N>
    [[nodiscard]] inline status::statat_t http::parse_request(u8_t * const b, u8_t *p, ReqLine& out, const std::size_t run_size) noexcept
    {
        const status::statat_t meth_end = find_request_method_end(b, run_size, out);
        if (meth_end != 0) [[unlikely]]
            return meth_end;
        const status::statat_t val_end = find_request_url_end<N>(b, p + meth_end.offset, run_size,  out);
        if (val_end != 0) [[unlikely]]
            return val_end;
        const std::size_t o = val_end.offset;
        if ((run_size - o) < 10)
            return {status::expect_bytes, run_size};
        const int expected_version = set_version<setup::http>(b + o, out);
        if (not expected_version or is_not_end_of_line(b + (o + 8), run_size - (o + 8))) [[unlikely]]
            return {status::error, o};
        return {0, o + 10};
    }

    template<simd::VWidth N>
    [[nodiscard]] inline status::statat_t http::parse_response(u8_t * const b, u8_t *p, ReqLine& out, const std::size_t run_size) noexcept
    {
        return 0;
    }

    template<>
    [[nodiscard]] inline status http::parse_header<0>(u8_t * const b, u8_t *b_run [[maybe_unused]], header_view& out, const std::size_t in_size [[maybe_unused]], const std::size_t run_size) noexcept
    {  
        const std::size_t n_offset = find_non_tchar<0>(b, run_size);
        if (n_offset == run_size) [[unlikely]]
            return status::expect_bytes;
        if (b[n_offset] != Character::Colon) [[unlikely]]
            return status::unexpected_char;
        out.set_name_end(n_offset, nullptr);
        
        const size_t v_offset = find_bad_char_in_value(b, n_offset, run_size);
        if (v_offset == run_size) [[unlikely]]
            return status::expect_bytes;
        if (is_not_end_of_line(b + v_offset, run_size - v_offset)) [[unlikely]]
            return status::unexpected_char;
        return out.set_value_end(v_offset, reinterpret_cast<const u8_t *>(b));
    }
} // NAMESPACE::HTTVO
#endif // HTTPVO_IMPLEMENTATION_SCALAR_HPP
