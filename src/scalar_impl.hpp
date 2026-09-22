#ifndef HTTPVO_IMPLEMENTATION_SCALAR_HPP
#define HTTPVO_IMPLEMENTATION_SCALAR_HPP
#include "implementation.hpp"

namespace httpvo::Implementation
{
    inline bool is_valid(u8_t i) { return i > 0x20 and i < 0x7f; }

    inline int parse_single_char(u8_t *b, ReqLine& req, bool& tsp, std::size_t run_size, std::size_t i)
    {
        u8_t c = b[i];
        u8_t n_i = i + 1;
        if (common::is_whitespace(c)) [[unlikely]]
        {
            if (tsp) [[unlikely]]
                return -1;
            tsp = true;
            return req.add_len(1) and n_i < run_size;
        }
        if (c == '\xd' or c == '\xa') [[unlikely]]
        {
            if (c == '\xd' and n_i < run_size)
                return -(b[n_i] != '\xa');
            return 0;
        }
        if (not is_valid(c)) [[unlikely]]
            return -1;
        tsp = false;
        return n_i < run_size;
    }

    int parse_char_unroll(u8_t *b, ReqLine& req, std::size_t run_size, bool trailing_wsp)
    {
        int stat = 0;
        if ((stat = parse_single_char(b, req, trailing_wsp, run_size, 0)) > 0)
            return stat;
        if ((stat = parse_single_char(b, req, trailing_wsp, run_size, 1)) > 0)
            return stat;
        if ((stat = parse_single_char(b, req, trailing_wsp, run_size, 2)) > 0)
            return stat;
        if ((stat = parse_single_char(b, req, trailing_wsp, run_size, 3)) > 0)
            return stat;
        if ((stat = parse_single_char(b, req, trailing_wsp, run_size, 4)) > 0)
            return stat;
        if ((stat = parse_single_char(b, req, trailing_wsp, run_size, 5)) > 0)
            return stat;
        if ((stat = parse_single_char(b, req, trailing_wsp, run_size, 6)) > 0)
            return stat;
        //unreachable
        return 0;
    }

     int http::parse_header_line_sc(void *in, std::size_t in_size, std::size_t run_size)
    {
        std::size_t i = in_reader.at();
        reqline.request();

        u8_t *b   = reinterpret_cast<u8_t *>(in) + i;
        bool tsp = false;

        for (std::size_t stop_size = run_size & ~(8ULL - 1); i < stop_size; i += 8)
        {
            if constexpr (NO_VECTORIZE)
                break;
            u64_t v = common::_load_u64(b + i);
            u64_t out_mask = ~common::_cmp_gt_and_lt<0x20, 0x7f>(v) & constant::c80;
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
                    reqline.add_len(i + k);
                    continue;
                }
                if (c == '\xd' or c == '\xa')
                {
                    if (c == '\xd' and (k + 1) < run_size) [[unlikely]]
                        return -(vp[1] != '\xa');   
                    return 0;
                }
                return -1;
            }
        }
        return i != run_size and parse_char_unroll(b + i, reqline, run_size & (8 - 1), tsp);
    }
}

#endif // HTTPVO_IMPLEMENTATION_SCALAR_HPP