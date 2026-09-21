#ifndef HTTPVO_IMPLEMENTATION_SCALAR_HPP
#define HTTPVO_IMPLEMENTATION_SCALAR_HPP
#include "implementation.hpp"

namespace httpvo::Implementation
{
    constexpr int CONTINUE = 1;

    struct Parse_state {
        std::size_t i;
        std::size_t j;
        bool tsp;
    };

    inline bool is_valid(u8_t i) { return i > 0x20 and i < 0x7f; }

    inline int parse_single_char(const u8_t *b, std::size_t run_size, auto& req, Parse_state& s)
    {
        u8_t c = b[s.i];
        if (common::is_whitespace(c)) [[unlikely]]
        {
            req[s.j--] += s.i;
            if (s.tsp) [[unlikely]]
                return -1;
            s.tsp = true;
            s.i += 1;
            return s.i < run_size and s.j;
        }
        if (c == '\xd' or c == '\xa') [[unlikely]]
        {
            if (c == '\xd' and (s.i + 1) < run_size and b[s.i + 1] != '\xa')
                return -1;
            return 0;
        }
        if (not is_valid(c))
            return -1;
        s.tsp = false;
        s.i += 1;
        return s.i < run_size and s.j;
    }

    int parse_char_unroll(u8_t *b, auto& req, std::size_t run_size, std::size_t pos, std::size_t j, bool trailing_wsp)
    {
        Parse_state s{pos, j, trailing_wsp};

        int stat = 0;
        while (true)
        {
            if ((stat = parse_single_char(b, run_size, req, s)) != CONTINUE)
                return stat;
            if ((stat = parse_single_char(b, run_size, req, s)) != CONTINUE)
                return stat;
            if ((stat = parse_single_char(b, run_size, req, s)) != CONTINUE)
                return stat;
            if ((stat = parse_single_char(b, run_size, req, s)) != CONTINUE)
                return stat;
            if ((stat = parse_single_char(b, run_size, req, s)) != CONTINUE)
                return stat;
            if ((stat = parse_single_char(b, run_size, req, s)) != CONTINUE)
                return stat;
            if ((stat = parse_single_char(b, run_size, req, s)) != CONTINUE)
                return stat;
            if ((stat = parse_single_char(b, run_size, req, s)) != CONTINUE)
                return stat;
        }
        __builtin_unreachable();
    }

     int http::parse_header_line_sc(void *in, std::size_t in_size, std::size_t run_size)
    {
        std::size_t i = in_reader.at();
        std::size_t j = 3;

        u8_t *b   = reinterpret_cast<u8_t *>(in) + i;
        auto& req = reqline.req;
        

        bool tsp = false;

        std::size_t stop_size = run_size & ~(8ULL - 1);
        for (; i < stop_size; i += 8)
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
            for (; out_mask; out_mask &= out_mask - 1)
            {
                u64_t k  = bits::tzcnt(out_mask) / 8;
                u8_t *vp = reinterpret_cast<u8_t *>(&v) + k;
                u8_t  c  = vp[0];
                if (common::is_whitespace(c)) [[likely]]
                {
                    if (tsp) [[unlikely]]
                        return -1;
                    req[j--] += i + k;
                    tsp = true;
                    continue;
                }
                if (c == '\xd' or c == '\xa')
                {
                    if (c == '\xd' and (k + 1) < run_size and vp[1] != '\xa') [[unlikely]]
                        return -1;
                    return 0;
                }
                return -1;
            }
        }
        return run_size != run_size or parse_char_unroll(b + i, req, run_size & (8 - 1), i, j, tsp);
    }
    

}

/*

        in_reader.incr_by(i + (run_size & (8 - 1)));
        return end_of_header_line(in, mask);
*/
#endif // HTTPVO_IMPLEMENTATION_SCALAR_HPP