#ifndef HTTPVO_IMPLEMENTATION_SCALAR_HPP
#define HTTPVO_IMPLEMENTATION_SCALAR_HPP
#include "implementation.hpp"
#include "simd/westmere/implementation.hpp"

namespace httpvo::Implementation
{
    static constexpr int CR = '\xd';
    static constexpr int LF = '\xa';

    inline bool is_valid(u8_t i) { return i > 0x20 and i < 0x7f; }

    inline _Status parse_single_char(const u8_t * const b, ReqLine& req, bool& tsp, std::size_t run_size, std::size_t i)
    {
        std::size_t n_i = i + 1;
        u8_t c   = b[i];
        if (common::is_whitespace(c)) [[unlikely]]
        {
            if (tsp) [[unlikely]]
                return -1;
            tsp = true;
            return {req.add_len(i) and n_i < run_size, -2};
        }
        if (c == CR) [[unlikely]]
        {
            if (n_i == run_size)
                return {0, -2};
            c = b[n_i];
        }
        if (c == LF)
        {
            req.add_len(i);
            return {req.set_version(b), 0};
        }
        if (not is_valid(c)) [[unlikely]]
            return -1;
        tsp = false;
        return n_i < run_size;
    }

    inline _Status parse_trailing_chars(const u8_t * const b, ReqLine& req, std::size_t run_size, std::size_t i, bool trailing_wsp)
    {
        _Status stat {0};
        if ((stat = parse_single_char(b, req, trailing_wsp, run_size, i+0)) < 1) return stat;
        if ((stat = parse_single_char(b, req, trailing_wsp, run_size, i+1)) < 1) return stat;
        if ((stat = parse_single_char(b, req, trailing_wsp, run_size, i+2)) < 1) return stat;
        if ((stat = parse_single_char(b, req, trailing_wsp, run_size, i+3)) < 1) return stat;
        if ((stat = parse_single_char(b, req, trailing_wsp, run_size, i+4)) < 1) return stat;
        if ((stat = parse_single_char(b, req, trailing_wsp, run_size, i+5)) < 1) return stat;
        if ((stat = parse_single_char(b, req, trailing_wsp, run_size, i+6)) < 1) return stat;
        return stat;
    }

    template<simd::VWidth N=8, bool ISTRAIL=0>
    inline _Status http::scparse_header_line(u8_t * const b, u8_t *b_run, const std::size_t run_size, ReqLine& req, simd::mask_t mask, u64_t tsp)
    {
        static_assert(!(N & (N - 1))); // N must be a power of 2
        const u8_t * const end  = b + run_size;
        const u8_t * const stop = ISTRAIL ? b + (run_size & ~(N - 1)) : end;
        for (; b_run < stop; b_run += N)
        {
            simd::simdv<N> v{b_run};
            simd::mask_t out_mask = v.template cmpngt_lt<0x21, 0x7e>().to_bitmask() & mask;
            if (not out_mask) [[likely]]
            {
                tsp = 0;
                continue;
            }
            for (; out_mask; out_mask &= out_mask - 1)
            {
                const unsigned int offset = simd::simdv<N>::countz_bitmask(out_mask);
                u8_t c = b_run[offset];
                if (common::is_whitespace(c)) [[likely]]
                {
                    if (tsp & out_mask) [[unlikely]]
                    {
                        if constexpr (setup::no_multispace)
                            return -1;
                        tsp = bits::lsb(out_mask) << simd::simdv<N>::bitpos;
                        continue;
                    }
                    if (not req.add_len(static_cast<const std::size_t>(b_run - b) + offset)) [[unlikely]]
                        return -1;
                    tsp = bits::lsb(out_mask) << simd::simdv<N>::bitpos;
                    continue;
                }
                if (c == CR)
                {
                    if ((b_run + offset) == (end - 1)) [[unlikely]]
                        return {0, -2};
                    c = b_run[offset + 1];
                }
                if (c == LF)
                {
                    req.add_len(static_cast<const std::size_t>(b_run - b) + offset);
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
                const std::size_t r = N - (run_size & (N - 1));
                const std::size_t s = r * simd::simdv<N>::bitpos;
                return scparse_header_line<N, true>(b, b_run - r, run_size, req, constant::cff << s, tsp << s);
            }
        }
        // run_size < N
        if constexpr (N > 8)
            return scparse_header_line<8, false>(b, b_run, run_size, req, constant::cff, 0);
        // run_size < 8
        return parse_trailing_chars(b, req, run_size, b_run - b, tsp);
    }
}

#endif // HTTPVO_IMPLEMENTATION_SCALAR_HPP