#ifndef HTTPVO_IMPLEMENTATION_SCALAR_HPP
#define HTTPVO_IMPLEMENTATION_SCALAR_HPP
#include "implementation.hpp"

namespace httpvo::Implementation
{
    inline bool is_valid(u8_t i) { return i > 0x20 and i < 0x7f; }

    inline int parse_single_char(u8_t *b, ReqLine& req, bool& tsp, std::size_t run_size, std::size_t i)
    {
        u8_t n_i = i + 1;
        u8_t c   = b[i];
        if (common::is_whitespace(c)) [[unlikely]]
        {
            if (tsp) [[unlikely]]
                return -1;
            tsp = true;
            return req.add_len(i + 1) and n_i < run_size;
        }
        if (c == '\xd') [[unlikely]]
        {
            if (n_i < run_size and b[n_i] != '\xa')
                return -1;
            return 0; // TODO: set trailing ret
        }
        if (c == '\xa')
            return 0;
        if (not is_valid(c)) [[unlikely]]
            return -1;
        tsp = false;
        return n_i < run_size;
    }

    inline int parse_trailing_chars(u8_t *b, ReqLine& req, std::size_t i, std::size_t run_size, bool trailing_wsp)
    {
        int stat = 0;
        if      ((stat = parse_single_char(b+i, req, trailing_wsp, run_size, 0)) < 1);
        else if ((stat = parse_single_char(b+i, req, trailing_wsp, run_size, 1)) < 1);
        else if ((stat = parse_single_char(b+i, req, trailing_wsp, run_size, 2)) < 1);
        else if ((stat = parse_single_char(b+i, req, trailing_wsp, run_size, 3)) < 1);
        else if ((stat = parse_single_char(b+i, req, trailing_wsp, run_size, 4)) < 1);
        else if ((stat = parse_single_char(b+i, req, trailing_wsp, run_size, 5)) < 1);
        else if ((stat = parse_single_char(b+i, req, trailing_wsp, run_size, 6)) < 1);
        if (stat != 0) 
            return stat;
        return req.set_version(b);
    }

     int http::scparse_header_line(u8_t *b, ReqLine& req, std::size_t in_size, std::size_t run_size)
    {
        std::size_t i = in_reader.at();
        bool tsp      = state.has_trailing_wsp();

        for (std::size_t stop_size = run_size & ~(8ULL - 1); i < stop_size; i += 8)
        {
            u64_t v = common::_load_u64(b + i);
            u64_t out_mask = ~common::_cmp_gt_and_lt<0x19, 0x7f>(v) & constant::c80;
            if (not out_mask) [[likely]]
            {
                tsp = 0;
                continue;
            }
            for (; out_mask and not reqline.complete(); out_mask &= out_mask - 1)
            {
                u64_t k  = bits::tzcnt(out_mask) / 8;
                u8_t *vp = reinterpret_cast<u8_t *>(&v) + k;

                u8_t  c  = vp[0];
                if (common::is_whitespace(c)) [[likely]]
                {
                    if (tsp) [[unlikely]]
                        return -1;
                    req.add_len(i + k);
                    continue;
                }
                if (c == '\xd')
                {
                    state.set_trailing_ret(true);
                    if ((k + 1) == run_size) [[unlikely]]
                        return req.set_version(b) - 1;
                    c = vp[1];
                }
                if (c == '\xa') [[likely]]
                    return req.set_version(b);
                return -1;
            }
        }
        if (i != run_size)
            return parse_trailing_chars(b, req, i, run_size & (8 - 1), tsp);
        return 1;
    }
}

#endif // HTTPVO_IMPLEMENTATION_SCALAR_HPP