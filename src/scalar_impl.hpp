#ifndef HTTPVO_IMPLEMENTATION_SCALAR_HPP
#define HTTPVO_IMPLEMENTATION_SCALAR_HPP
#include "implementation.hpp"
#include "simd/westmere/implementation.hpp"

namespace httpvo::Implementation
{
    static constexpr int CR = '\xd';
    static constexpr int LF = '\xa';

    inline bool is_valid(u8_t i) { return i > 0x20 and i < 0x7f; }

    inline status parse_single_char(const u8_t * const b, ReqLine& out, u64_t& tsp, const std::size_t run_size, const std::size_t i)
    {
        const std::size_t n_i = i + 1;
        u8_t c   = b[i];
        if (is_valid(c)) [[likely]]
        {
            tsp = false;
            return {n_i < run_size, status::expect_bytes};
        }
        if (common::is_whitespace(c))
        {
            if (tsp) [[unlikely]]
                return -1;
            tsp = true;
            return {out.advance(i) and n_i < run_size, status::overrun_error};
        }
        if (c == CR)
        {
            if (n_i == run_size)
                return {0, status::expect_linefeed};
            c = b[n_i];
        }
        if (c == LF)
        {
            out.advance(i);
            return -out.set_version(b);
        }
        return -1;
    }

    inline status parse_trailing_chars(const u8_t * const b, ReqLine& out, u64_t &tsp, std::size_t run_size, std::size_t i)
    {
        status stat {0};
        if ((stat = parse_single_char(b, out, tsp, run_size, i+0)) < 1) return stat;
        if ((stat = parse_single_char(b, out, tsp, run_size, i+1)) < 1) return stat;
        if ((stat = parse_single_char(b, out, tsp, run_size, i+2)) < 1) return stat;
        if ((stat = parse_single_char(b, out, tsp, run_size, i+3)) < 1) return stat;
        if ((stat = parse_single_char(b, out, tsp, run_size, i+4)) < 1) return stat;
        if ((stat = parse_single_char(b, out, tsp, run_size, i+5)) < 1) return stat;
        if ((stat = parse_single_char(b, out, tsp, run_size, i+6)) < 1) return stat;
        if constexpr (setup::no_vectorize)
            return parse_single_char(b, out, tsp, run_size, i+7);
        return stat;
    }

    template<>
    inline status http::parse_line<0, 0>(u8_t * const b, u8_t *b_run, ReqLine& out, const std::size_t run_size, const simd::mask_t mask [[maybe_unused]], u64_t tsp)
    {
        // TODO: OPTIMIZE THIS FUNCTION
        // DO NOT CALL AS STANDALONE
        status stat {0};
        for (std::size_t i = 0; true; i += 16)
        {
            if ((stat = parse_trailing_chars(b, out, tsp, run_size, i+0)) < 1)
                return stat;
            if ((stat = parse_trailing_chars(b, out, tsp, run_size, i+8)) < 1)
                return stat;
        }
        return stat;
    }

    template<simd::VWidth N=8, bool ISTRAIL=0>
    inline status http::parse_line(u8_t * const b, u8_t *b_run, ReqLine& out, const std::size_t run_size, const simd::mask_t mask, u64_t tsp)
    {
        if constexpr (setup::no_vectorize)
            return parse_line<0, 0>(b, b_run, out, run_size, mask, tsp);

        static_assert(!(N & (N - 1))); // N must be a power of 2
        const u8_t * const end  = b + run_size;
        const u8_t * const stop = !ISTRAIL ? b + (run_size & ~(N - 1)) : end;
        for (; b_run < stop; b_run += N)
        {
            simd::simdv<N> v{b_run};
            simd::mask_t out_mask = v.template cmpngt_lt<0x21, 0x7e>().to_bitmask() & mask;
            if (not out_mask) [[likely]]
            {
                tsp = 0;
                continue;
            }
            for (; out_mask; out_mask = bits::clear_least_set_bit(out_mask))
            {
                const unsigned int offset = simd::simdv<N>::countzero_bitmask(out_mask);
                u8_t c = b_run[offset];
                if (common::is_whitespace(c)) [[likely]]
                {
                    if (tsp & out_mask) [[unlikely]]
                    {
                        if constexpr (setup::no_multispace)
                            return status::unwanted_whitespace;
                        tsp = bits::least_set_bit(out_mask) << simd::simdv<N>::bitpos;
                        continue;
                    }
                    if (not out.advance(static_cast<const std::size_t>(b_run - b) + offset)) [[unlikely]]
                        return -1;
                    tsp = bits::least_set_bit(out_mask) << simd::simdv<N>::bitpos;
                    continue;
                }
                if (c == CR)
                {
                    if ((b_run + offset) == (end - 1)) [[unlikely]]
                        return {0, status::expect_linefeed};
                    c = b_run[offset + 1];
                }
                if (c == LF)
                {
                    out.advance(static_cast<const std::size_t>(b_run - b) + offset);
                    return -out.set_version(b);
                }
                return status::unexpected_char;
            }
            tsp = bool(tsp) << (simd::simdv<N>::bitpos - 1);
        }
        if constexpr (not ISTRAIL)
        {
            if (run_size > (N - 1))
            {
                // process trailing bytes by overlapping last read bytes with the remaining bytes
                const std::size_t r = N - (run_size & (N - 1));
                const std::size_t s = r * simd::simdv<N>::bitpos;
                return parse_line<N, true>(b, b_run - r, out, run_size, constant::cff << s, tsp << s);
            }
        }
        // run_size < N
        if constexpr (N > 8)
            return parse_line<8, false>(b, b_run, out, run_size, constant::cff, 0);
        // run_size < 8
        return parse_trailing_chars(b, out, tsp, run_size, b_run - b);
    }

    // see table generation scripts and comments on test/scripts/generate_shuffle_table.py
    alignas(32) static constexpr int NON_TCHAR_CLASS_LUT[32]{
        00, 00, 01, 02, 04,  8, 16, 32, 00, 00, 00, 00, 00, 00, 00, 00,
        58, 63, 62, 63, 63, 63, 63, 63, 62, 62, 61, 21, 52, 21, 61, 28,
    };

     // see table generation scripts and comments on test/scripts/generate_shuffle2_table.py
    alignas(32) static constexpr int CONTROL_CHAR_CLASS_LUT[32]{
        02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 01, 00, 00, 00, 00, 03,
        01, 00, 127, 127, 127, 127, 127, 02, 127, 127, 127, 127, 127, 127, 127, 127,
    };

    template<simd::VWidth N>
    inline simd::mask_t is_control_char(const simd::simdv<N>& v)
    {
        // since the control bytes overlap, we can reduce to a precomputed comparison instead of a range test. see /test/scripts/generate_shuffle_table2.py for my comments
        const simd::simdv<N> CTRL_LO = simd::simdv<N>::load_tbl1(CONTROL_CHAR_CLASS_LUT + 00);
        const simd::simdv<N> CTRL_HI = simd::simdv<N>::load_tbl1(CONTROL_CHAR_CLASS_LUT + 16);
        if constexpr (setup::neon)
        {
             // we get 1.37x by skipping the &. We can because neon has a byte shift op.
            return v.shuf_table(CTRL_LO) > (v >> 4).shuf_table(CTRL_HI);    
        }
        const simd::simdv<N> LO_NIB  = simd::simdv<N>::splat(0x0f);
        return v.shuf_table(CTRL_LO) > ((v >> 4) & LO_NIB).shuf_table(CTRL_HI);
    }

    template<simd::VWidth N>
    inline simd::mask_t is_non_tchar(const simd::simdv<N>& v)
    {
        const simd::simdv<N> NON_TCHAR_LO = simd::simdv<N>::load_tbl1(NON_TCHAR_CLASS_LUT + 00);
        const simd::simdv<N> NON_TCHAR_HI = simd::simdv<N>::load_tbl1(NON_TCHAR_CLASS_LUT + 16);

        if constexpr (setup::neon)
        {
            // skipped the & after v >> 4
            return simd::simdv<N>::andneqz(v.shuf_table(NON_TCHAR_LO), (v >> 4).shuf_table(NON_TCHAR_HI));
        }
        const simd::simdv<N> LO_NIB = simd::simdv<N>::splat(0x0f);
        return simd::simdv<N>::andneqz(v.shuf_table(NON_TCHAR_LO), ((v >> 4) & LO_NIB).shuf_table(NON_TCHAR_HI));
     }

    template<simd::VWidth N=16, bool ISTRAIL=0>
    inline status parse_16_32B(u8_t * const b, u8_t *b_run, header_view& out, const std::size_t in_size, const std::size_t run_size, const std::size_t r)
    {
        static_assert(N > (16 - 1) or N > (32 - 1)); // N must be 16 or 32
        static constexpr u8_t COL = '\x3b';

        u8_t *end = reinterpret_cast<u8_t>(b + run_size);
        const u8_t * const stop = !ISTRAIL ? b + (run_size & ~(N - 1)) : end;
        
        for (bool parsing_value = true; true; b_run += N)
        {
            simd::simdv<N> v(b_run);
            if (not parsing_value)
            {
                simd::mask_t non_tchar = is_non_tchar(v);
                if (not non_tchar)
                    continue;
                simd::mask_t col = simd::simdv<N>::countzero_bitmask(non_tchar);
                if (b_run[col] != COL) [[unlikely]]
                    return -1;
                out.name = static_cast<std::size_t>(b_run - b) + col;
                parsing_value = true;
            }
            simd::mask_t control_char = is_control_char(v);
            // TODO
        }
    }
}

#endif // HTTPVO_IMPLEMENTATION_SCALAR_HPP