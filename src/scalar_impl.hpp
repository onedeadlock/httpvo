#ifndef HTTPVO_IMPLEMENTATION_SCALAR_HPP
#define HTTPVO_IMPLEMENTATION_SCALAR_HPP
#include "implementation.hpp"

namespace httpvo::Implementation
{
    constexpr int TRAIL = 0;

    inline bool is_valid(u8_t i) { return i > 0x20 and i < 0x7f; }

    inline _Status parse_single_char(u8_t *b, ReqLine& req, bool& tsp, std::size_t run_size, std::size_t i)
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
        if (c == '\xd') [[unlikely]]
        {
            if (n_i == run_size)
                return {0, -2};
            c = b[n_i];
        }
        if (c == '\xa')
        {
            req.add_len(i);
            return {req.set_version(b), 0};
        }
        if (not is_valid(c)) [[unlikely]]
            return -1;
        tsp = false;
        return n_i < run_size;
    }

    inline _Status parse_trailing_chars(u8_t *b, ReqLine& req, std::size_t i, std::size_t run_size, bool trailing_wsp)
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

    template<int N=!TRAIL>
     _Status http::scparse_header_line(u8_t *b, ReqLine& req, std::size_t in_size, std::size_t run_size)
    {
        std::size_t i = in_size;
        std::size_t stop_size = 0;
        u64_t tsp     = 0;

        if constexpr (N != TRAIL) stop_size = run_size & ~(8ULL - 1); else stop_size = run_size;
        for (; i < stop_size; i += 8)
        {
            u64_t v = common::_load_u64(b + i);
            u64_t out_mask = ~common::_cmp_gt_and_lt<0x20, 0x7f>(v) & constant::c80;
            if constexpr (N == TRAIL)
                out_mask &= constant::cff << (run_size % 8 * 8); // mask out back-padded bytes
            if (not out_mask) [[likely]]
            {
                tsp = 0;
                continue;
            }
            for (; out_mask; out_mask &= out_mask - 1)
            {
                u64_t offset  = bits::tzcnt(out_mask) / 8;
                u8_t *vp = reinterpret_cast<u8_t *>(&v) + offset;
                u8_t  c  = vp[0];

                if (c == '\x20' or c == '\x9')
                {
                    if (tsp & out_mask) [[unlikely]]
                        return -1;
                    tsp = out_mask << 8;
                    if (not req.add_len(i + offset))
                        return -1;
                    continue;
                }
                if (c == '\xd')
                {
                    if ((offset + 1) == run_size) [[unlikely]]
                        return {0, -2};
                    c = vp[1];
                }
                if (c == '\xa')
                {
                    req.add_len(i + offset);
                    return req.set_version(b);
                }
                return -1;
            }
            tsp = bool(tsp) << 7;
            if constexpr (N == TRAIL)
                break;
        }
        if constexpr (N != TRAIL)
            if (run_size > 7)
                return scparse_header_line<TRAIL>(b, req, i - (run_size % 8), run_size);
        return parse_trailing_chars(b, req, i, run_size, tsp);
    }
}

/*

    7 bytes and run_size > 7

    run_size = 19
    i = 16
    rem = 3
    new = 19 - ( 8 - 3) = 19 - 5s
    

*/

#endif // HTTPVO_IMPLEMENTATION_SCALAR_HPP