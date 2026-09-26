#ifndef HTTPVO_IMPLEMENTATION_SCALAR_HPP
#define HTTPVO_IMPLEMENTATION_SCALAR_HPP
#include "implementation.hpp"
#include "simd/implementation.hpp"

namespace httpvo::Implementation
{
    static constexpr int CR = '\xd';
    static constexpr int LF = '\xa';

    inline bool is_valid(u8_t i) { return i > 0x20 and i < 0x7f; }

    inline _Status parse_single_char(u8_t *b, ReqLine& req, bool& tsp, std::size_t run_size, std::size_t in_size)
    {
        std::size_t n_i = in_size + 1;
        u8_t c   = b[in_size];
        if (common::is_whitespace(c)) [[unlikely]]
        {
            if (tsp) [[unlikely]]
                return -1;
            tsp = true;
            return {req.add_len(in_size) and n_i < run_size, -2};
        }
        if (c == CR) [[unlikely]]
        {
            if (n_i == run_size)
                return {0, -2};
            c = b[n_i];
        }
        if (c == LF)
        {
            req.add_len(in_size);
            return {req.set_version(b), 0};
        }
        if (not is_valid(c)) [[unlikely]]
            return -1;
        tsp = false;
        return n_i < run_size;
    }

    inline _Status parse_trailing_chars(u8_t *b, ReqLine& req, std::size_t run_size, std::size_t in_size, bool trailing_wsp)
    {
        _Status stat {0};
        if ((stat = parse_single_char(b, req, trailing_wsp, run_size, in_size+0)) < 1) return stat;
        if ((stat = parse_single_char(b, req, trailing_wsp, run_size, in_size+1)) < 1) return stat;
        if ((stat = parse_single_char(b, req, trailing_wsp, run_size, in_size+2)) < 1) return stat;
        if ((stat = parse_single_char(b, req, trailing_wsp, run_size, in_size+3)) < 1) return stat;
        if ((stat = parse_single_char(b, req, trailing_wsp, run_size, in_size+4)) < 1) return stat;
        if ((stat = parse_single_char(b, req, trailing_wsp, run_size, in_size+5)) < 1) return stat;
        if ((stat = parse_single_char(b, req, trailing_wsp, run_size, in_size+6)) < 1) return stat;
        return stat;
    }

    template<simd::VWidth N=32, bool ISTRAIL=0>
    _Status http::scparse_header_line(u8_t *b, ReqLine& req, std::size_t run_size, std::size_t in_size, u64_t mask, u64_t tsp)
    {
        static_assert(!(N & (N - 1))); // N must be a power of 2
        std::size_t stop_size;
        if constexpr (ISTRAIL) stop_size = run_size & ~(N - 1); else stop_size = run_size;
        for (; in_size < stop_size; in_size += 8)
        {
            simd::simdv<N> v{b + in_size};
            simd::mask_t out_mask = v.template cmpngt_lt<0x21, 0x7e>().to_bitmask() & mask;
            if (not out_mask) [[likely]]
            {
                tsp = 0;
                continue;
            }
            for (; out_mask; out_mask &= out_mask - 1)
            {
                const unsigned int offset  = simd::simdv<N>::countz_bitmask(out_mask);
                u8_t c = (b + in_size)[offset];

                if (common::is_whitespace(c)) [[likely]]
                {
                    if (tsp & out_mask) [[unlikely]]
                        return -1;
                    if (not req.add_len(in_size + offset)) [[unlikely]]
                        return -1;
                    tsp = out_mask << simd::simdv<N>::bitpos;
                    continue;
                }
                if (c == CR)
                {
                    if ((offset + 1) == run_size) [[unlikely]]
                        return {0, -2};
                    c = (b + in_size)[offset + 1];
                }
                if (c == LF)
                {
                    req.add_len(in_size + offset);
                    return req.set_version(b);
                }
                return -1;
            }
            tsp = bool(tsp) << (simd::simdv<N>::bitpos - 1);
        }
        if constexpr (ISTRAIL)
        {
            if (run_size > N - 1)
            {
                // process trailing bytes by overlapping last read bytes with the remaining bytes
                const std::size_t r = run_size & (N - 1);
                const std::size_t s = r * simd::simdv<N>::bitpos;
                return scparse_header_line<N, true>(b, req, run_size, in_size - (N - r), constant::cff << s, tsp << s);
            }
        }
        // bytes (run_size) are genuinely lesser than N
        if constexpr (N > 8)
            return scparse_header_line<8, false>(b, req, run_size, in_size, constant::cff, 0);
        return parse_trailing_chars(b, req, run_size, in_size, tsp);
    }
}

#endif // HTTPVO_IMPLEMENTATION_SCALAR_HPP