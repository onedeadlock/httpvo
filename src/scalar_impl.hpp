#ifndef HTTPVO_IMPLEMENTATION_SCALAR_HPP
#define HTTPVO_IMPLEMENTATION_SCALAR_HPP
#include "implementation.hpp"

namespace httpvo::Implementation
{
    inline bool is_valid(u8_t i) { return i > 0x20 and i < 0x7f; }

    int http::parse_header_line_sc(void *in, std::size_t in_size, std::size_t run_size)
    {
        u8_t *b = reinterpret_cast<u8_t *>(in) + 0;//in_reader.at();
        auto& req = reqline.req_line;

        u64_t mask = 0;
        u64_t crlf = 0;
        std::size_t j = out_reader.at();
        std::size_t i = 0;

        std::size_t stop_size = run_size & ~(8ULL - 1);
        for (; i < stop_size; i += 8)
        {
            if constexpr (NO_VECTORIZE)
                break;
            if (state.has_trailing_ret()) [[unlikely]]
            {
                if (b[i] != '\xa')
                    return -400;
                in_reader.incr_by(1);
                return end_of_header_line(in, mask);
            }

            u64_t v  = common::_load_u64(b + i);
            u64_t cr = common::_cmpeq(v, constant::c0d);
            u64_t lf = common::_cmpeq(v, constant::c0a);
            crlf = cr & (lf >> 8); 
            if constexpr (not HTTP_STRICT_DELIM)
                crlf |= lf;

            if ((crlf & 0x80) and this->at_start_line) [[unlikely]]
                return -400;

            u64_t sp    = common::_cmpeq(v, constant::c20) | common::_cmpeq(v, constant::c09);
            u64_t tchar = common::_cmp_gt_and_lt<'\x20', '\x7f'>(v);
            u64_t trailing_wsp  = static_cast<u64_t>(state.has_trailing_wsp()) << 7;
            u64_t single_wsp    = bits::andnot(bits::bltrim(sp), trailing_wsp);
            u64_t invalid_tchar = bits::andnot(constant::c80, tchar) ^ single_wsp;
            u64_t error_tchar   = invalid_tchar | lf | (cr & 0x0080808080808080ULL);

            if (error_tchar & bits::tzmask(crlf))
                   return -400;

            u64_t cr_lf_msk = cr | lf;
            for (mask = (sp | cr_lf_msk) & bits::blsmask(cr_lf_msk); mask and j; mask &= mask - 1)
                req[j--] += i + (bits::tzcnt(mask) >> 3);

            state.set_trailing_ret(static_cast<bool>(cr & constant::msb_64));
            state.set_trailing_wsp(static_cast<bool>(sp & constant::msb_64));
            if (crlf)
            {
                in_reader.incr_by(i + bits::tzcnt(mask)); return 0;
                return end_of_header_line(in, mask);
            }
            if (j == 0 and mask) [[unlikely]]
                return -400;   
        }

        for (std::size_t k = i; k < run_size and j; k++)
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
                continue;
            }
            state.set_trailing_wsp(false);
            if (not is_valid(c))
                return -400;
        }

        // TODO set out_reader
        in_reader.incr_by(i + (run_size & (8 - 1)));
        return end_of_header_line(in, mask);
    }

    int nparse_no_rescan(void *in, std::size_t in_size, std::size_t run_size, void *out, std::size_t out_size)
    {
        return 0;
    }

    
}
#endif // HTTPVO_IMPLEMENTATION_SCALAR_HPP