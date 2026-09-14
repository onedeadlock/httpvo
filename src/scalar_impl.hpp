#ifndef IMPLEMENTATION_SCALAR_HPP
#define IMPLEMENTATION_SCALAR_HPP
#include "implementation.hpp"
#include <iostream>
#include <cstdio>

#define PUT(i) printf("%llx\n", static_cast<uint64_t>((i)))
#define PUTB(i) printf("%064llb\n", static_cast<uint64_t>((i)))
#define PUTI(i) printf("%lld\n", static_cast<int64_t>((i)))
#define HERE(i) printf("Here at L%s\n", #i)

namespace dhttp::Implementation
{
    inline bool is_valid(u8_t i) { return i > 0x20 and i < 0x7f; }
    
    int http::parse_header_line_sc(void *in, std::size_t in_size, std::size_t run_size)
    {
        u64_t *b = reinterpret_cast<u64_t *>(reinterpret_cast<u8_t *>(in) + in_reader.at());
        auto& req = reqline.req_line;

        u64_t mask = 0, crlf = 0;
        std::size_t j = out_reader.at();
        std::size_t i = 0;

        u8_t yes;

        for (i = 0; run_size > 7; run_size -= 8, i++)
        {
            u64_t v = b[i];
            u64_t cr = common::_cmpeq(v, constant::c0d);
            u64_t lf = common::_cmpeq(v, constant::c0a);

            if (state.has_trailing_ret()) [[unlikely]]
            {
                if (not (lf & 0b1))
                    return -400;
                in_reader.incr_by(i * 8 + 1);
            }
            if (crlf = cr & (lf >> 8); crlf & 0b10 and this->unused) [[unlikely]]
                return -400;
            u64_t sp  = common::_cmpeq(v, constant::c20) | common::_cmpeq(v, constant::c09);
            u64_t tsp = static_cast<u64_t>(state.has_trailing_whitespace()) * 8;;
            u64_t tchar = common::_cmp_gt_and_lt<'\x20', '\x7f'>(v);
            u64_t err = bits::andnot(constant::c80, tchar) ^ bits::andnot(bits::ltrim_u64(sp), tsp) | lf | (cr & 0x0080808080808080ULL);
            if (err & bits::tzmask(crlf))
                return -400;

            for (mask = (sp | cr) & bits::blsmask(cr | lf); mask and j; mask &= mask - 1)
                req[out_reader.decr()] += i * 8 + bits::tzcnt(mask) / 8;

            state.set_trailing_ret(static_cast<bool>(cr & constant::msb_64));
            state.set_trailing_wsp(static_cast<bool>(sp & constant::msb_64));
            if (crlf)
                goto end;
            if (j == 0 and mask) [[unlikely]]
                return -400;   
        }

        for (u8_t *b = reinterpret_cast<u8_t *>(b); j and run_size--; i++)
        {
            u8_t c = b[run_size]; 
            if (bool is_cr = c == '\xa'; is_cr or c == '\xd') [[unlikely]]
            {
                if (not is_cr or (run_size and b[run_size - 1] == '\xd')) [[likely]]
                    break;
                return -400;
            }
            if (bool is_sp = common::is_whitespace(c)) [[unlikely]]
            {
                reqline.req_line[j] += i;
                j -= 1;
                if (state.has_trailing_whitespace() or (run_size and common::is_whitespace(b[run_size - 1]))) [[unlikely]]
                    return -400;
                state.set_trailing_wsp(is_sp);
            }
        }
        switch (run_size)
        {
            case 1: yes = is_valid(b[i]); break;
            case 2: yes = is_valid(b[i]) & is_valid(b[i+1]); break;
            case 3: yes = is_valid(b[i]) & is_valid(b[i+1]) & is_valid(b[i+2]); break;
            case 4: yes = (common::_cmp_gt_and_lt<'\x20', '\x7f'>(reinterpret_cast<u32_t *>(b+i)[0]) == 0x80808080U); break;
            case 5: yes = (common::_cmp_gt_and_lt<'\x20', '\x7f'>(reinterpret_cast<u32_t *>(b+i)[0]) == 0x80808080U) & is_valid(b[i+4]); break;
            case 6: yes = (common::_cmp_gt_and_lt<'\x20', '\x7f'>(reinterpret_cast<u32_t *>(b+i)[0]) == 0x80808080U) & is_valid(b[i+4]) & is_valid(b[i+5]); break;
            case 7: yes =!(common::_cmp_gt_and_lt<'\x20', '\x7f'>(reinterpret_cast<u32_t *>(b+i)[0]) ^ common::_cmp_gt_and_lt<'\x20', '\x7f'>(reinterpret_cast<u32_t *>(b + i - 1)[0])); break;
        }
        if (not yes) [[unlikely]]
            return -400;
        end:
        state.completed_request_line(true);
        return -(mask or req_version_tag(in, Reqtype::index[this->req_type]) isnot http_1);
    }

    int nparse_no_rescan(void *in, std::size_t in_size, std::size_t run_size, void *out, std::size_t out_size)
    {
        return 0;
    }
}
#endif