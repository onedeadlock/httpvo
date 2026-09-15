#ifndef HTTPVO_IMPLEMENTATION_SCALAR_HPP
#define HTTPVO_IMPLEMENTATION_SCALAR_HPP
#include "implementation.hpp"
#include <iostream>
#include <cstdio>

namespace httpvo::Implementation
{
    inline bool is_valid(u8_t i) { return i > 0x20 and i < 0x7f; }

    int is_valid_tchar(u8_t *b, std::size_t size)
    {
        bool yes = true;
        switch (size)
        {
            case 1: yes = is_valid(b[0]); break;
            case 2: yes = is_valid(b[0]) & is_valid(b[1]); break;
            case 3: yes = is_valid(b[0]) & is_valid(b[1]) & is_valid(b[2]); break;
            case 4: yes = (common::_cmp_gt_and_lt<'\x20', '\x7f'>(U32P(b)) == 0x80808080U); break;
            case 5: yes = (common::_cmp_gt_and_lt<'\x20', '\x7f'>(U32P(b)) == 0x80808080U) & is_valid(b[4]); break;
            case 6: yes = (common::_cmp_gt_and_lt<'\x20', '\x7f'>(U32P(b)) == 0x80808080U) & is_valid(b[4]) & is_valid(b[5]);    break;
            case 7: yes =!(common::_cmp_gt_and_lt<'\x20', '\x7f'>(U32P(b)) ^ common::_cmp_gt_and_lt<'\x20', '\x7f'>(U32P(b+3))); break;
        }
        return yes;
    }
    int http::parse_header_line_sc(void *in, std::size_t in_size, std::size_t run_size)
    {
        u8_t *b = reinterpret_cast<u8_t *>(in) + in_reader.at(), yes = true;
        auto& req = reqline.req_line;

        u64_t mask = 0;
        u64_t crlf = 0;
        std::size_t j = out_reader.at();
        std::size_t i = 0;

        std::size_t stop_size = run_size & ~(8ULL - 1);
        for (; i < stop_size; i += 8)
        {
            if (state.has_trailing_ret()) [[unlikely]]
            {
                if (b[i] != '\xa')
                    return -400;
                in_reader.incr_by(1);
                return end_of_header_line(in, mask);
            }
            u64_t v;
            if constexpr (__HAVE_SUPPORT_FOR_UNALIGNED__)
                v = reinterpret_cast<u64_t *>(b + i)[0];
            else
                __builtin_memcpy(&v, b + i, 8);
            u64_t cr = common::_cmpeq(v, constant::c0d);
            u64_t lf = common::_cmpeq(v, constant::c0a);
            crlf = cr & (lf >> 8);

            if ((crlf & 0x80) and this->at_start_line) [[unlikely]]
                return -400;

            u64_t sp    = common::_cmpeq(v, constant::c20) | common::_cmpeq(v, constant::c09);
           
            #if 0
            u64_t tchar = common::_cmp_gt_and_lt<'\x20', '\x7f'>(v);
            u64_t trailing_wsp  = static_cast<u64_t>(state.has_trailing_wsp()) << 7;
            u64_t single_wsp    = bits::andnot (bits::bltrim(sp), trailing_wsp);
            u64_t invalid_tchar = bits::andnot(constant::c80, tchar) ^ single_wsp;
            u64_t error_tchar   = invalid_tchar | lf | (cr & 0x0080808080808080ULL);

            if (error_tchar & bits::tzmask(crlf))
               return -400;
               #endif
            for (mask = (sp | cr | lf) & bits::blsmask(cr | lf); mask and j; mask &= mask - 1)
                req[out_reader.decr()] += i + (bits::tzcnt(mask) >> 3);

            state.set_trailing_ret(static_cast<bool>(cr & constant::msb_64));
            state.set_trailing_wsp(static_cast<bool>(sp & constant::msb_64));
            if (crlf)
            {
                in_reader.incr_by(i + bits::tzcnt(mask));
                return end_of_header_line(in, mask);
            }
            if (j == 0 and mask) [[unlikely]]
                return -400;   
        }
        return 0;
        for (std::size_t k = i; k < run_size; k++)
        {
            u8_t c = b[k];

            if (c == '\xa' or c == '\xd') [[unlikely]]
            {
                bool have_bytes = (k + 1) < run_size;
                if (c == '\xd' and have_bytes and b[k + 1] == '\xa') [[likely]]
                    break;
                return -400;
            }
            if (common::is_whitespace(c)) [[unlikely]]
            {
                req[j--] += k;
                bool error_dup_wsp = state.has_trailing_wsp();
                state.set_trailing_wsp(true);
                if (error_dup_wsp or (k + 1 < run_size and common::is_whitespace(b[k + 1]))) [[unlikely]]
                    return -400;
            }
        }

        if (not is_valid_tchar(b + i, run_size & (8 - 1)))
            return -400;
        in_reader.incr_by(i + (run_size & (8 - 1)));
        return end_of_header_line(in, mask);
    }

    int nparse_no_rescan(void *in, std::size_t in_size, std::size_t run_size, void *out, std::size_t out_size)
    {
        return 0;
    }
}
#endif // HTTPVO_IMPLEMENTATION_SCALAR_HPP