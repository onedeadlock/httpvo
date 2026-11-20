#ifndef HTTPVO_IMPLEMENTATION_SCALAR_HPP
#define HTTPVO_IMPLEMENTATION_SCALAR_HPP
#include "implementation.hpp"

namespace httpvo::Implementation
{
    using namespace simd;

    enum set : int {
        alpha =1,
        space =1,
        inverse =1,
    };

    enum Character : u8_t {
        HorizontalTab  = '\x09',
        LineFeed       = '\x0a',
        CarriageReturn = '\x0d',
        NonPrintMax    = '\x19',
        Space          = '\x20',
        Colon          = '\x3a',
        Delete         = '\x7f',
    };

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

    inline const void *setptr(const void *lhs, const void *rhs)
    {
        return (lhs = rhs);
    }

    inline u32_t load4(const u8_t * const b)
    {
        // :: assume len(b) >= 4
        u32_t LD = 0;
        if HTTPVO_CXX17_CONSTEXPR (setup::little_endian)
            httpvo_memcopy(&LD, b, 4);
        else
        {
#           define USH(c, n) (static_cast<u32_t>(c) << (n))
            // GCC and clang should fold these shifts into a DWORD load (with a bswap on Big Endian mode). It is not the case for MSVC, but in our use case,the other option cost as much or even more.
            // @discard_reginfo: clang may not emit a single load if LD is used in some ways (like shifting/masking out any of the bytes), so we force it.
            LD = USH(b[0], 0) | USH(b[1], 8) | USH(b[2], 16) | USH(b[3], 24);
            httpvo_discard_reginfo(LD);
#           undef USH
         }
         return LD;
    }

    inline u64_t load8(const u8_t * const b)
    {
        // :: assume len(b) >= 8
        u64_t LD = 0;
        if HTTPVO_CXX17_CONSTEXPR (setup::little_endian)
            httpvo_memcopy(&LD, b, 8);
        else
        {
#           define USH(c, n) (static_cast<u64_t>(c) << (n))
            // Again, GCC and clang should fold these shifts into a QWORD load with a bswap if needed.
            LD |= USH(b[0], 0U) | USH(b[1], 8U) | USH(b[2], 16) | USH(b[3], 24);
            LD |= USH(b[4], 32) | USH(b[5], 40) | USH(b[6], 48) | USH(b[7], 56);
            httpvo_discard_reginfo(LD);
#           undef USH
        }
        return LD;
    }

    inline int Parse::end_of_parse(const u8_t * const b) noexcept
    {
        constexpr u32_t CRLF = 0x0a0d;
        const u32_t v = load4(b);
        end_of_parse_marker = v == (CRLF | (CRLF << 16));
        return (v & 0xffffu) == CRLF;
    }
    
    inline int Parse::is_not_end_of_line(const u8_t * const b, const std::size_t rem) noexcept
    {
        // :: assume rem >= 2
        if HTTPVO_CXX17_CONSTEXPR (setup::end_of_line_is_line_feed)
            pass(); // TODO
        if HTTPVO_CXX17_LIKELY (rem > 3)
            return not end_of_parse(b);
         return (b[0] != Character::CarriageReturn) | (b[1] != Character::LineFeed);
    }

    template <int X, int Y, int N=1>
    inline std::size_t is_out_of_range(const u8_t *, const mask_t = constant::cff);

    template<>
    inline std::size_t is_out_of_range<0x21, 0x7e, 1>(const u8_t * const b, const mask_t)
    {
        return static_cast<const u8_t>(*b - 0x21) > 0x5d;
    }

    template <int X, int Y, int N>
    inline std::size_t is_out_of_range(const u8_t * const b, const mask_t m)
    {
        static_assert(X == 0x21 and Y == 0x7e);
        const mask_t mask = simdv<N>(b).template off_range<X, Y>().to_bitmask() & m;
        if (mask) return simdv<N>::countzero_bitmask(mask); else return N;
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
        const u64_t LD = load8(b);
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
            const std::size_t offset = simdv<>::countzero_bitmask(m);
            if (TCHAR_LUT[b[offset]] != 3)
                return offset;
        }
        return simdv<>::N;
    }
    
    template<bool ALPHAONLY>
    inline std::size_t offset_of_non_tchar(const u8_t * const b)
    {
        if HTTPVO_CXX17_CONSTEXPR (not setup::no_vectorize)
        {
            const mask_t mask = simdv<>(b).fast_not_tchar<ALPHAONLY>().to_bitmask();
            if HTTPVO_CXX17_LIKELY (mask) // fast pre-check
                return _offset_of_non_tchar_mask(b, mask);
            return simdv<>::N;
        }
        else return _offset_of_non_tchar(b, 0, simdv<>::N);
    }

    template<bool ALPHAONLY>
    inline std::size_t find_non_tchar(const u8_t * const b, const std::size_t len)
    {
        constexpr int N = simdv<>::N; 
        if (setup::optimize_for_most_case or len < N)
            return _offset_of_non_tchar(b, 0, len);
        const u8_t * const stop = b + (len - N);
        for (const u8_t *p = b; p < stop; p += N)
        {
            const std::size_t offset = offset_of_non_tchar<set::alpha>(p);
            if (offset < N)
                return offset + size(p, b);
        }
        return offset_of_non_tchar<set::alpha>(stop) + (len - N);
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
            return _offset_of_bad_char_in_value(b, 0, simdv<>::N);
        const mask_t offset = simdv<>(b).is_not_print_char().to_bitmask();
        return simdv<>::countzero_bitmask(offset);
    }

    inline std::size_t find_bad_char_in_value(const u8_t * const b, const std::size_t i, const std::size_t len)
    {
        constexpr int N = simdv<>::N;
        if (len < N) return _offset_of_bad_char_in_value(b, i, len);

        const u8_t * const stop = b + (len - N);
        for (const u8_t *p = b + i; p < stop; p += N)
        {
            const std::size_t offset = offset_of_bad_char_in_value(p);
            if HTTPVO_CXX17_UNLIKELY (offset < N)
                return size(p, b) + offset;
        }
        return offset_of_bad_char_in_value(stop) + (len - N);
    }

    template<int N>
    inline std::size_t first_non_whitespace(const u8_t * const b, const mask_t m=constant::cff)
    {
        constexpr mask_t ALL_WHITESPACE = simd::mask_fill<mask_t, simd::mask_width<N>(), simdv<N>::bitpos>();
        static_assert(ALL_WHITESPACE != 0);
        const mask_t mask = is_whitespace<simdv<N>, set::space>(simdv<N>(b)).to_bitmask() & m;
        return simdv<N>::countzero_bitmask(~mask & ALL_WHITESPACE);
    }

    template <int N>
    noinline const u8_t *_trim_whitespace(const u8_t *b, const u8_t *const end)
    {
        const u8_t *const stop = end - N;
        for (; b < stop; b += N)
        {
            const std::size_t offset = first_non_whitespace<N>(b);
            if HTTPVO_CXX17_LIKELY (offset < N)
                return b + offset;
        }
        return stop + first_non_whitespace<N>(stop, simd::make_mask<set::inverse>(size(end, stop)));
    }

    template<>
    noinline const u8_t * _trim_whitespace<1>(const u8_t *b, const u8_t * const end)
    {
        for (; b < end and is_whitespace<int, set::space>( int(b[0]) ); b++) pass();
        return b;
    }

     template<int N>
    inline const u8_t * trim_whitespace(const u8_t *b, const u8_t * const end)
    {
        if HTTPVO_CXX17_LIKELY (not is_whitespace<int, set::space>( int(b[0]) ))
            return b;
        if HTTPVO_CXX17_CONSTEXPR (setup::no_multispace)
            return end;
        else return _trim_whitespace<N>(b + 1, end);
    }

    HTTPVO_START_NO_DIAGNOSE_ARRAY_BOUNDS
    template <int N>
    inline const u8_t * _find_request_url_end(const u8_t *b, const u8_t * const end)
    {
        const u8_t * const stop = end - N;
        for (; b < stop; b += N)
        {
            const std::size_t offset = is_out_of_range<0x21, 0x7e, N>(b);
            if (offset < N)
                return b + offset;
        }
        const mask_t mask = simd::make_mask<set::inverse>(size(end, stop));
        return stop + is_out_of_range<0x21, 0x7e, N>(stop, mask);
    }
    HTTPVO_STOP_NO_DIAGNOSE_ARRAY_BOUNDS
    
    template <>
    inline const u8_t *_find_request_url_end<1>(const u8_t *b, const u8_t *const end)
    {
        const u8_t *const stop = reinterpret_cast<const u8_t *>(bits::align(uintptr_t(end), 8));
        for (; b < stop; b += 8)
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

    template <int N>
    inline status _find_request_url_end_main(const u8_t *b, const u8_t *p, const u8_t * const end, ReqLine& out)
    {
        p = trim_whitespace<N>(p, end);
        if HTTPVO_CXX17_UNLIKELY (p == end)
            return status::error_multispace(p, b);
        p = _find_request_url_end<N>(p, end);
        if HTTPVO_CXX17_UNLIKELY (p == end)
            return status::error(status::expect_bytes, p, b);
        if HTTPVO_CXX17_UNLIKELY (not is_whitespace<int, set::space>( int(p[0]) ))
            return status::error(status::unexpected_char, p, b);
        p += 1;
        if HTTPVO_CXX17_UNLIKELY (p < end and setptr(p, trim_whitespace<N>(p, end)) == end)
            return status::error_multispace(p, b);
        return {size(end, p) < (setup::req_line_min_size - 4), out.advance(size(p, b))};
    }

    inline status find_request_method_end(const u8_t * const b, const std::size_t len, ReqLine& out)
    {
        const std::size_t offset = find_non_tchar<set::alpha>(b, len);
        if HTTPVO_CXX17_UNLIKELY ((offset == 1) | (offset == len))
            return status::error(status::expect_bytes, len);
        if HTTPVO_CXX17_UNLIKELY (not is_whitespace<int, set::space>( int(b[offset]) ))
            return status::error(status::unexpected_char, offset);
        return {(len - offset) < setup::req_line_min_size - 2, out.advance(offset)};
    }

    template <int N>
    inline status find_request_url_end(const u8_t * const b, const u8_t *p, const u8_t *end, ReqLine& out)
    {
        if HTTPVO_CXX17_CONSTEXPR (setup::no_vectorize)
            return _find_request_url_end_main<1>(b, p, end, out);
        else
        {
            if HTTPVO_CXX17_LIKELY (end - p >= N)
                return _find_request_url_end_main<N>(b, p, end, out);
            return _find_request_url_end_main<1>(b, p, end, out);
        }
    }

    template<int N>
    [[nodiscard]] inline status end_of_method_and_url(const u8_t *b, const std::size_t run_size, ReqLine& out) noexcept
    {
        if (run_size < setup::req_line_min_size)
            return status::error(status::expect_bytes, run_size);
        const status end_of_method = find_request_method_end(b, run_size, out);
        if HTTPVO_CXX17_UNLIKELY (end_of_method != 0)
            return end_of_method;
        const  std::size_t o = end_of_method.offset + 1;
        return find_request_url_end<N>(b, b + o, b + run_size, out);
    }

    template<int N, class Protocol=Http_Protocol>
    [[nodiscard]] inline status Parse::request_line(const u8_t *b, const std::size_t run_size, ReqLine& out) noexcept
    {
        const status end_of_mthurl = end_of_method_and_url<N>(b, run_size, out);
        if HTTPVO_CXX17_UNLIKELY (end_of_mthurl != 0)
            return end_of_mthurl;
        const std::size_t o = end_of_mthurl.offset;
        if HTTPVO_CXX17_UNLIKELY (set_version<Protocol>(b + o, out) == status::unrecognized or is_not_end_of_line(b + (o + 8), run_size - (o + 8)))
            return status::error(status::unexpected_error, o);
        return {0, o + 10};
    }

    template<int N, class Protocol=Http_Protocol>
    [[nodiscard]] inline status Parse::response_line(const u8_t *b, const std::size_t run_size, ReqLine& out) noexcept
    {
        return 0;
    }

    template <int N>
    [[nodiscard]] inline status Parse::header(u8_t *const b, header_view &out, const std::size_t run_size) noexcept
    {
        static_assert(simd::is_simd<simdv<N>>() and N > 8);
        static_assert(N == 16 or N == 32);
        const u8_t *const stop = b + bits::align(run_size, N);
        bool parsing_value = false;

        for (const u8_t *p = b; p < stop; p += N)
        {
            simdv<N> v{p};
            if (not parsing_value)
            {
                mask_t non_tchar = v.is_non_tchar().to_bitmask();
                if (not non_tchar)
                    continue;
                const u8_t *const q = p + simdv<N>::countzero_bitmask(non_tchar);
                if HTTPVO_CXX17_UNLIKELY (q[0] != Character::Colon)
                    return status::unexpected_char;
                out.set_name_end(static_cast<std::size_t>(q - b), nullptr);
                parsing_value = true;
            }
            if (mask_t control_char = v.is_not_print_char().to_bitmask())
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
