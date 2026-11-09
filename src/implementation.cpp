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
        static_assert(std::is_integral<T>() or simd::is_simd<T>());
        if HTTPVO_CXX17_CONSTEXPR (SPCONLY) return c == Character::Space;
        else return (c == Character::Space) | (c == Character::HorizontalTab);
    }

    inline std::size_t size(const u8_t * const e, const u8_t * const s)
    {
        return static_cast<std::size_t>(e - s);
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
        if HTTPVO_CXX17_CONSTEXPR (setup::neon)
        {
            if HTTPVO_CXX17_CONSTEXPR (not setup::arm64)
                return ((v <= Character::NonPrintMax) | (v == Character::Delete)) ^ (v == Character::HorizontalTab); // no vqtbl1q
            else return (v & 0xf).shuf_table(CTRL_LO) > (v >> 4).shuf_table(CTRL_HI);
        }    
        return v.shuf_table(CTRL_LO) > ((v >> 4) & 0xf).shuf_table(CTRL_HI);
    }

    template<simd::VWidth N>
    inline mask_t is_non_tchar(const simdv<N>& v)
    {
        const simdv<N> TCHAR_LO = simdv<N>::load_tbl1(TCHAR_CLASS_LUT + 00);
        const simdv<N> TCHAR_HI = simdv<N>::load_tbl1(TCHAR_CLASS_LUT + 16);

        if HTTPVO_CXX17_CONSTEXPR (setup::neon)
           return (v & 0xf).shuf_table(TCHAR_LO).andeqz((v >> 4).shuf_table(TCHAR_HI));
        // skipped first 'v & 0xf' since v >= 0x80 falls to 0 and that is what is needed
        return v.shuf_table(TCHAR_LO).andeqz(((v >> 4) & 0xf).shuf_table(TCHAR_HI));
     }

    inline int Parse::end_of_parse(const u32_t v) noexcept
    {
        constexpr u32_t CRLF = 0x0a0d;
        end_of_parse_marker = v == (CRLF | (CRLF << 16));
        return (v & 0xffffu) == CRLF;
    }

    inline int Parse::end_of_parse_single_load(const u8_t * const b) noexcept
    {
        if HTTPVO_CXX17_CONSTEXPR (setup::little_endian and not setup::no_vectorize)
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
    
    inline int Parse::is_not_end_of_line(const u8_t * const b, const std::size_t rem) noexcept
    {
        // :: assume rem >= 2
        if HTTPVO_CXX17_CONSTEXPR (setup::end_of_line_is_line_feed)
            pass(); // TODO
        if HTTPVO_CXX17_LIKELY (rem > 3)
            return not end_of_parse_single_load(b);
         return (b[0] != Character::CarriageReturn) | (b[1] != Character::LineFeed);
    }
    
    //  3 -> name tchar; 2 -> value tchar
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

    template <int X, int Y, simd::VWidth N=1>
    inline std::size_t is_out_of_range(const u8_t *);

    template<>
    std::size_t is_out_of_range<0x21, 0x7e, 1>(const u8_t * const b)
    {
        return static_cast<const u8_t>(*b - 0x21) > 0x5d;
    }

    template <int X, int Y, simd::VWidth N>
    inline std::size_t is_out_of_range(const u8_t * const b)
    {
        static_assert(X == 0x21 and Y == 0x7e);
        const mask_t mask = simdv<N>(b).template cmpngt_lt<X, Y>();
        if HTTPVO_CXX17_UNLIKELY (mask)
            return simdv<N>::countzero_bitmask(mask);
        return N;
    };

    inline bool is_expected_minor_version(const int minor, ReqLine& out)
    {
         return (out.version = minor ^ 0x30) < 10;
    }

    template<class T>
    inline bool is_protocol(const u64_t proto)
    {
        constexpr u64_t http_slash_one_dot = 0x2e312f5054544800ULL;
        constexpr u64_t rtsp_slash_one_dot = 0x2e312f5053545200ULL;
        constexpr u64_t rtsp_slash_two_dot = 0x2e322f5053545200ULL;
        if HTTPVO_CXX17_CONSTEXPR (std::is_same<T, Rtsp_Protocol>())
            return (proto == rtsp_slash_two_dot) | (proto == rtsp_slash_one_dot);
        return proto == http_slash_one_dot;
    }

    template<class T>
    inline bool set_version(const u8_t * const b, ReqLine& out)
    {
        // :: assume len(b) >= 8
        u64_t LD = 0;
        if HTTPVO_CXX17_CONSTEXPR (setup::little_endian)
            LD = simd::simdv<8>::_load_u64(b);
        else
        {
#           define USH(c, n) (static_cast<u64_t>(c) << (n))
            // Again, GCC and clang should fold these shifts into a single QWORD load (with a bswap on Big Endian mode). It is not the case for MSVC, but the cost against per byte compare, is almost the same.
            // @discard_reginfo: clang sees that the last byte is discarded and then reused singly, so it emit 4 loads (first 4 bytes, next two + 1 bytes, and the last byte) instead of 1 for the 8 bytes.
            LD |= USH(b[0], 0U) | USH(b[1], 8U) | USH(b[2], 16) | USH(b[3], 24);
            LD |= USH(b[4], 32) | USH(b[5], 40) | USH(b[6], 48) | USH(b[7], 56);
            httpvo_discard_reginfo(LD);
#           undef USH
        }
        return is_protocol<T>(LD << 8) & is_expected_minor_version(LD >> 56, out);
    }

    inline std::size_t _offset_of_non_tchar(const u8_t * const b, const std::size_t i, const std::size_t len)
    {
        std::size_t j = i;
        for (; j < len and TCHAR_LUT[b[j]] == 3; j++) pass();
        return j;
    }

    inline std::size_t _offset_of_non_tchar_mask(const u8_t * const b, const mask_t mask)
    {
        for (mask_t m = mask; m; m = bits::clear_least_set_bit(m))
        {
            const std::size_t offset = simdv<8>::countzero_bitmask(m);
            if (TCHAR_LUT[b[offset]] != 3)
                return offset;
        }
        return 8;
    }
    
    template<bool ALPHAONLY>
    make_flat inline std::size_t offset_of_non_tchar(const u8_t * const b)
    {
        if HTTPVO_CXX17_CONSTEXPR (not setup::no_vectorize)
        {
            const mask_t mask = simdv<8>(b).fast_not_tchar<ALPHAONLY>();
            if HTTPVO_CXX17_LIKELY (mask) // fast pre-check
                return _offset_of_non_tchar_mask(b, mask);
            return 8;
        }
        else return _offset_of_non_tchar(b, 0, 8);
    }

    template<bool ALPHAONLY>
    make_flat inline std::size_t find_non_tchar(const u8_t * const b, const std::size_t len)
    {
        if (setup::optimize_for_most_case or len < 8)
            return _offset_of_non_tchar(b, 0, len);
        const u8_t * const stop = b + (len - 8);
        for (const u8_t *p = b; p < stop; p += 8)
        {
            const std::size_t offset = offset_of_non_tchar<ALPHAONLY>(p);
            if (offset < 8)
                return offset + size(p, b);
        }
        return offset_of_non_tchar<ALPHAONLY>(stop) + (len - 8);
    }

    inline std::size_t _offset_of_bad_char_in_value(const u8_t * const b, const std::size_t i, const std::size_t len)
    {
        std::size_t j = i;
        for (; j < len and TCHAR_LUT[b[j]]; j++) pass();
        return j;
    }

    inline std::size_t offset_of_bad_char_in_value(const u8_t *const b)
    {
        if HTTPVO_CXX17_CONSTEXPR (setup::no_vectorize)
            return _offset_of_bad_char_in_value(b, 0, 8);
        const mask_t offset = simdv<8>(b).is_not_print_char();
        if (offset)
            return simdv<8>::countzero_bitmask(offset);
        return 8;
    }

    make_flat inline std::size_t find_bad_char_in_value(const u8_t * const b, const std::size_t i, const std::size_t len)
    {
        if (len < 8) return _offset_of_bad_char_in_value(b, i, len);

        const u8_t * const stop = b + (len - 8);
        for (const u8_t *p = b + i; p < stop; p += 8)
        {
            const std::size_t offset = offset_of_bad_char_in_value(p);
            if HTTPVO_CXX17_UNLIKELY (offset < 8)
                return size(p, b) + offset;
        }
        return offset_of_bad_char_in_value(stop) + (len - 8);
    }

    template<simd::VWidth N>
    inline std::size_t first_non_whitespace(const u8_t * const b)
    {
        constexpr mask_t ALL_WHITESPACE = simd::mask_fill<mask_t, simd::mask_width<N>(), simdv<N>::bitpos>();
        static_assert(ALL_WHITESPACE != 0);
        const mask_t mask = is_whitespace<simdv<N>, true>(simdv<N>(b));
        if HTTPVO_CXX17_LIKELY (mask != ALL_WHITESPACE)
            return simdv<N>::countzero_bitmask(~mask & ALL_WHITESPACE);
        return N;
    }

    template<simd::VWidth N>
    inline const u8_t * trim_whitespace(const u8_t *b, const u8_t * const end)
    {
        if HTTPVO_CXX17_LIKELY (not is_whitespace<u8_t, true>(b[0]))
            return b;
        if HTTPVO_CXX17_CONSTEXPR (setup::no_multispace)
            return end;
        else
        {
            const u8_t * const stop = end - N;
            for (; b < stop; b += N)
            {
                const std::size_t offset = first_non_whitespace<N>(b);
                if HTTPVO_CXX17_LIKELY (offset < N)
                    return b + offset;
            }
            return stop + first_non_whitespace<N>(stop);
        }
    }

    template<>
    inline const u8_t * trim_whitespace<1>(const u8_t *b, const u8_t * const end)
    {
        if HTTPVO_CXX17_UNLIKELY (is_whitespace<u8_t, true>(b[0]))
        {
            if HTTPVO_CXX17_CONSTEXPR (setup::no_multispace) return end;
            else while (++b < end and is_whitespace<u8_t, true>(b[0])) pass();
        }
        return b;
    }

    inline status::statat_t find_request_method_end(const u8_t * const b, const std::size_t len, ReqLine& out)
    {
        const std::size_t offset = find_non_tchar<1>(b, len);
        if HTTPVO_CXX17_UNLIKELY (offset == len)
            return status::expect_bytes;
        if HTTPVO_CXX17_UNLIKELY (not is_whitespace<u8_t, true>(b[offset]))
            return status::unexpected_char;
        return {(len - offset) < setup::req_line_min_size - 2, out.advance(offset)};
    }

    HTTPVO_START_NO_DIAGNOSE_ARRAY_BOUNDS
    template <simd::VWidth N>
    inline const u8_t * _find_request_url_end(const u8_t *b, const u8_t * const end)
    {
        const u8_t * const stop = end - N;
        for (; b < stop; b += N)
        {
            const std::size_t offset = is_out_of_range<0x21, 0x7e, N>(b);
            if (offset < N)
                return b + offset;
        }
        return stop + is_out_of_range<0x21, 0x7e, N>(stop);
    }
    HTTPVO_START_NO_DIAGNOSE_ARRAY_BOUNDS
    
    template <>
    inline const u8_t *_find_request_url_end<1>(const u8_t *b, const u8_t *const end)
    {
        for (; b < end - 8; b += 8)
        {
            if (is_out_of_range<0x21, 0x7e>(b+0)){return b+0;} if (is_out_of_range<0x21, 0x7e>(b+1)){return b+1;}
            if (is_out_of_range<0x21, 0x7e>(b+2)){return b+2;} if (is_out_of_range<0x21, 0x7e>(b+3)){return b+3;}
            if (is_out_of_range<0x21, 0x7e>(b+4)){return b+4;} if (is_out_of_range<0x21, 0x7e>(b+5)){return b+5;}
            if (is_out_of_range<0x21, 0x7e>(b+6)){return b+6;} if (is_out_of_range<0x21, 0x7e>(b+7)){return b+7;}
        }
        for (; b < end and not is_out_of_range<0x21, 0x7e>(b); b += 1)
            pass();
        return b;
    }

    inline status::statat_t __status_bad_or_expect(const u8_t * const e, const u8_t * const s)
    {
        if HTTPVO_CXX17_CONSTEXPR (setup::no_multispace)
            return  {status::bad_whitespace, size(e, s)};
        else return {status::expect_bytes,   size(e, s)};
    }
   
    template <simd::VWidth N>
    inline status::statat_t _find_request_url_end_main(const u8_t *b, const std::size_t offset, const std::size_t len, ReqLine& out)
    {
        const u8_t * const end = (b + offset) + len;
        const u8_t *p = trim_whitespace<N>(b + offset, end);
        if HTTPVO_CXX17_UNLIKELY (p == end)
            return __status_bad_or_expect(p, b);
        p = _find_request_url_end<N>(p, end);
        if HTTPVO_CXX17_UNLIKELY (p == end)
            return {status::expect_bytes, size(p, b)};
        if HTTPVO_CXX17_UNLIKELY (not is_whitespace<u8_t, true>( p[0] ))
            return {status::unexpected_char, size(p, b)};
        p += 1;
        if HTTPVO_CXX17_UNLIKELY (p < end and (p = trim_whitespace<N>(p, end)) == end)
            return __status_bad_or_expect(p, b);
        return {size(end, p) < (setup::req_line_min_size - 4), out.advance(size(p, b))};
    }

    template <simd::VWidth N>
    inline status::statat_t find_request_url_end(const u8_t * const b, const std::size_t offset, const std::size_t len, ReqLine& out)
    {
        if HTTPVO_CXX17_CONSTEXPR (setup::no_vectorize)
            return _find_request_url_end_main<1>(b, offset, len, out);
        else
        {
            if HTTPVO_CXX17_LIKELY (len > (N - 1))
                return _find_request_url_end_main<N>(b, offset, len, out);
            if (len < 8) 
                return _find_request_url_end_main<1>(b, offset, len, out);
            return _find_request_url_end_main<8>(b, offset, len, out);
        }
    }

    template<simd::VWidth N>
    [[nodiscard]] inline status::statat_t end_of_method_and_url(const u8_t *b, const std::size_t run_size, ReqLine& out) noexcept
    {
        if (run_size < setup::req_line_min_size)
            return status::expect_bytes;
        const status::statat_t end_of_method = find_request_method_end(b, run_size, out);
        if HTTPVO_CXX17_UNLIKELY (end_of_method != 0)
            return end_of_method;
        const  std::size_t o = end_of_method.offset + 1;
        return find_request_url_end<N>(b, o, run_size - o, out);
    }

    template<simd::VWidth N, class Protocol=Http_Protocol>
    [[nodiscard]] inline status::statat_t Parse::request_line(const u8_t *b, const std::size_t run_size, ReqLine& out) noexcept
    {
        const status::statat_t end_of_mthurl = end_of_method_and_url<N>(b, run_size, out);
        if HTTPVO_CXX17_UNLIKELY (end_of_mthurl != 0)
            return end_of_mthurl;
        const std::size_t o = end_of_mthurl.offset;
        if HTTPVO_CXX17_UNLIKELY ((set_version<Protocol>(b + o, out) == status::unrecognized) | is_not_end_of_line(b + (o + 8), run_size - (o + 8)))
            return {status::error, o};
        return {0, o + 10};
    }

    template<simd::VWidth N, class Protocol=Http_Protocol>
    [[nodiscard]] inline status::statat_t Parse::response_line(const u8_t *b, const std::size_t run_size, ReqLine& out) noexcept
    {
        return 0;
    }

    template <simd::VWidth N>
    [[nodiscard]] inline status Parse::header(u8_t *const b, header_view &out, const std::size_t run_size) noexcept
    {
        static_assert(N == 16 or N == 32);
        const u8_t *const stop = b + bits::align(run_size, N);
        bool parsing_value = false;

        for (const u8_t *p = b; p < stop; p += N)
        {
            simdv<N> v{p};
            if (not parsing_value)
            {
                mask_t non_tchar = is_non_tchar<N>(v);
                if (not non_tchar)
                    continue;
                const u8_t *const q = p + simdv<N>::countzero_bitmask(non_tchar);
                if HTTPVO_CXX17_UNLIKELY (q[0] != Character::Colon)
                    return status::unexpected_char;
                out.set_name_end(static_cast<std::size_t>(q - b), nullptr);
                parsing_value = true;
            }
            if (mask_t control_char = is_control_char<N>(v))
            {
                const u8_t *const q = p + simdv<N>::countzero_bitmask(control_char);
                const std::size_t offset = size(q, b);
                if (is_not_end_of_line(q, run_size - offset))
                    return status::unexpected_char; // TODO: end_of_line function changed
                return out.set_value_end(offset, b);
            }
        }
        return status::expect_bytes;
    }

    template <>
    [[nodiscard]] inline status Parse::header<0>(u8_t *b, header_view &out, const std::size_t run_size) noexcept
    {
        const std::size_t n_offset = find_non_tchar<false>(b, run_size);
        if HTTPVO_CXX17_LIKELY (n_offset == run_size)
            return status::expect_bytes;
        if HTTPVO_CXX17_UNLIKELY (b[n_offset] != Character::Colon)
            return status::unexpected_char;
        out.set_name_end(n_offset, nullptr);

        const size_t v_offset = find_bad_char_in_value(b, n_offset, run_size);
        if (v_offset == run_size) [[unlikely]]
            return status::expect_bytes;
        if HTTPVO_CXX17_UNLIKELY (is_not_end_of_line(b + v_offset, run_size - v_offset))
            return status::unexpected_char;
        return out.set_value_end(v_offset, reinterpret_cast<const u8_t *>(b));
    }
} // NAMESPACE::HTTVO
#endif // HTTPVO_IMPLEMENTATION_SCALAR_HPP
