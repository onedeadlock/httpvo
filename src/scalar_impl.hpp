#ifndef HTTPVO_IMPLEMENTATION_SCALAR_HPP
#define HTTPVO_IMPLEMENTATION_SCALAR_HPP
#include "implementation.hpp"

namespace httpvo::Implementation
{
    constexpr int TRAIL = 0;
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

    template<int N=8>
     _Status http::scparse_header_line(u8_t *b, ReqLine& req, std::size_t run_size, std::size_t in_size, u64_t mask, u64_t tsp)
    {
        std::size_t stop_size;
        if constexpr (N != TRAIL) stop_size = run_size & ~(8ULL - 1); else stop_size = run_size;
        for (; in_size < stop_size; in_size += 8)
        {
            const u64_t out_mask = mask & (simdv<N>::load(b + in_size).cmpgt_lt<'\x20', '\x7f'>().to_bitmask());
            if (not out_mask) [[likely]]
            {
                tsp = 0;
                continue;
            }
            for (; out_mask; out_mask &= out_mask - 1)
            {
                const u64_t offset  = bits::tzcnt(out_mask) >> 3;
                u8_t c = (b + in_size)[offset];

                if (common::is_whitespace(c)) [[likely]]
                {
                    if (tsp & out_mask) [[unlikely]]
                        return -1;
                    if (not req.add_len(in_size + offset)) [[unlikely]]
                        return -1;
                    tsp = out_mask << 8;
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
            tsp = bool(tsp) << 7;
        }
        if constexpr (N != TRAIL)
        {
            if (run_size > 7)
            {
                // process trailing bytes by overlapping last read bytes with the remaining bytes
                const std::size_t r = run_size & (8 - 1);
                return scparse_header_line<TRAIL>(b, req, run_size, in_size - (8 - r), constant::cff << (r * 8), tsp << (r * 8));
            }
        }
        // bytes (run_size) are genuinely lesser than 8
        return parse_trailing_chars(b, req, run_size, in_size, tsp);
    }
}

#endif // HTTPVO_IMPLEMENTATION_SCALAR_HPP