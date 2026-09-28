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
                const unsigned int offset = simd::simdv<N>::countz_bitmask(out_mask);
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

    template<simd::VWidth N=8, bool ISTRAIL=0>
    inline status http::parse_header(void * const b, u8_t *b_run, ReqLine& out, const std::size_t in_size, const std::size_t run_size, const std::size_t r)
    {
        static_assert(!(N & (N - 1))); // N must be a power of 2
        u8_t *end = reinterpret_cast<u8_t>(b + run_size);
        const u8_t * const stop = !ISTRAIL ? b + (run_size & ~(N - 1)) : end;
        const simd::simdv<N> v_lf = simd::simdv<N>::splat('\xa');
        const simd::simdv<N> v_cr = simd::simdv<N>::splat('\xd');

        for (; b_run < end; )
        {
            simd::simdv<N> v(b_run);
            u64_t lf   = v.cmp_eq(v, v_lf).to_bitmask();
            u64_t cr   = v.cmp_eq(v, v_cr).to_bitmask();
            u64_t crlf = cr & (lf << 1);

            // TODO: Parse Name and Value

            // maybe the end of us parsing this buffer (eop)
            if (auto eop = crlf & crlf >> 2)
                return 0;

            b_run += N; // next run

            // or maybe eop is incomplete; cases like cr, crlf, crlfcr
            if (auto eop = (lf | cr) >> N - 3; eop > 0b100) [[unlikely]]
            {
                // fast fail for (cr/lf)_*Non-crlf*_(cr/lf)
                if (eop & 0b101) [[unlikely]]
                    return -1;
                // the top three bits of intN in the eop mask can be 100, 110 or 111
                // in any of the cases, tab[top_three_bits_in_eop] gives us the number of bytes we need to check
                // also tab[tab[last_three_bits_in_eop]] gives the number of times we need to shift backward in order to read a complete crlfcrlf word
                alignas(8) static constexpr u8_t eop_tab[8]{0, 2, 1, 0, 3, 0, 2, 1};
                int n = eop_tab[eop];
                if (b != end or r >= n) [[likely]]
                    return -(reinterpret_cast<u32_t *>(b - N - eop_tab[n])[0] == 0xd0a0d0a);
                this->n_bytes_to_complete = n;
                return {0, status::expect_linefeed};  // we need atleast <= 3 bytes to confirm an exact eop
            }
        }
        return 0;
    }

    template<simd::VWidth N=8, bool ISTRAIL=0>
    inline status parse_32_64B(void * const b, u8_t *b_run, ReqLine& out, const std::size_t in_size, const std::size_t run_size, const std::size_t r)
    {
        static_assert(N > (32 - 1) or N > (64 - 1)); // N must be 32 or 64
        u8_t *end = reinterpret_cast<u8_t>(b + run_size);
        const u8_t * const stop = ISTRAIL ? b + (run_size & ~(N - 1)) : end;

        simd::simdv<N> hi = simdv<N>::splat(0x80);
        while (true)
        {
            simd::simdv<N> v(b_run);
            simd::simdv<N> name_class  = v.compare_table(b_run, CLASS);
            simd::simdv<N> value_class = simd::simdv<N>::and(name_class, simd::simdv<N>::not(v.cmpeq(hi)));

            mask_t name_mask = name_class.to_bitmask();
            mask_t col = bits::tzcnt(name_mask);
            if (b[col] != COL) [[unlikely]]
            {
                if constexpr (not setup::no_leading_space)
                    if (b[col] == SP and b[col + 1] == COL)
                        col -= 1; // TODO: do not modify col
                return -1;
            }
            mask_t value_mask = value_class.to_bitmask();
            // TODO: first, assume all is correct, trim whitespace, then check
            // run value mask tape
            
            b_run += N; // next run
            if (b_run >= stop) [[unlikely]]
            {
                // TODO: handle trailing bytes
            }
        }
    }
}

#endif // HTTPVO_IMPLEMENTATION_SCALAR_HPP