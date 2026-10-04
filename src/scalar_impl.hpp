#ifndef HTTPVO_IMPLEMENTATION_SCALAR_HPP
#define HTTPVO_IMPLEMENTATION_SCALAR_HPP
#include "implementation.hpp"
#include "simd/westmere/implementation.hpp"

namespace httpvo::Implementation
{
    using namespace simd;
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
            return {out.advance(i) and n_i < run_size, status::bad_whitespace};
        }
        if (c == CR)
        {
            if (n_i == run_size)
                return {0, status::expect_line_feed};
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
    inline status http::parse_line<0, 0>(u8_t * const b, u8_t *b_run, ReqLine& out, const std::size_t run_size, const mask_t mask [[maybe_unused]], u64_t tsp)
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
    inline status http::parse_line(u8_t * const b, u8_t *b_run, ReqLine& out, const std::size_t run_size, const mask_t mask, u64_t tsp)
    {
        if constexpr (setup::no_vectorize)
            return parse_line<0, 0>(b, b_run, out, run_size, mask, tsp);

        static_assert(!(N & (N - 1))); // N must be a power of 2
        const u8_t * const end  = b + run_size;
        const u8_t * const stop = !ISTRAIL ? b + bits::align(run_size, N) : end;
        for (; b_run < stop; b_run += N)
        {
            simdv<N> v{b_run};
            mask_t out_mask = v.template cmpngt_lt<0x21, 0x7e>().to_bitmask() & mask;
            if (not out_mask) [[likely]]
            {
                tsp = 0;
                continue;
            }
            for (; out_mask; out_mask = bits::clear_least_set_bit(out_mask))
            {
                const unsigned int offset = simdv<N>::countzero_bitmask(out_mask);
                u8_t c = b_run[offset];
                if (common::is_whitespace(c)) [[likely]]
                {
                    if (tsp & out_mask) [[unlikely]]
                    {
                        if constexpr (setup::no_multispace)
                            return status::bad_whitespace;
                        tsp = bits::least_set_bit(out_mask) << simdv<N>::bitpos;
                        continue;
                    }
                    if (not out.advance(static_cast<const std::size_t>(b_run - b) + offset)) [[unlikely]]
                        return -1;
                    tsp = bits::least_set_bit(out_mask) << simdv<N>::bitpos;
                    continue;
                }
                if (c == CR)
                {
                    if ((b_run + offset) == (end - 1)) [[unlikely]]
                        return {0, status::expect_line_feed};
                    c = b_run[offset + 1];
                }
                if (c == LF)
                {
                    out.advance(static_cast<const std::size_t>(b_run - b) + offset);
                    return -out.set_version(b);
                }
                return status::unexpected_char;
            }
            tsp = bool(tsp) << (simdv<N>::bitpos - 1);
        }
        if constexpr (not ISTRAIL)
        {
            if (run_size > (N - 1))
            {
                // process trailing bytes by overlapping last read bytes with the remaining bytes
                const std::size_t r = N - (run_size & (N - 1));
                const std::size_t s = r * simdv<N>::bitpos;
                return parse_line<N, true>(b, b_run - r, out, run_size, constant::cff << s, tsp << s);
            }
        }
        // run_size < N
        if constexpr (N > 8)
            return parse_line<8, false>(b, b_run, out, run_size, constant::cff, 0);
        // run_size < 8
        return parse_trailing_chars(b, out, tsp, run_size, b_run - b);
    }

    // see test/scripts/generate_shuffle_table.py for table generation and comments.
    alignas(32) static constexpr int NON_TCHAR_CLASS_LUT[32]{
        01, 01, 02, 04, 8, 16, 00, 32, 01, 01, 01, 01, 01, 01, 01, 01,
        11, 01, 03, 01, 1, 01, 01, 01, 03, 03, 05, 53, 23, 53, 05, 39,
    };

     // see test/scripts/generate_shuffle2_table.py for table generation and comments.
    alignas(32) static constexpr int CONTROL_CHAR_CLASS_LUT[32]{
        02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 01, 00, 00, 00, 00, 03,
        01, 00, 127, 127, 127, 127, 127, 02, 127, 127, 127, 127, 127, 127, 127, 127,
    };

    template<simd::VWidth N>
    inline mask_t is_control_char(const simdv<N>& v)
    {
        // since the control bytes overlap, we can reduce to a precomputed comparison instead of a range test. see /test/scripts/generate_shuffle_table2.py for my comments
        const simdv<N> CTRL_LO = simdv<N>::load_tbl1(CONTROL_CHAR_CLASS_LUT + 00);
        const simdv<N> CTRL_HI = simdv<N>::load_tbl1(CONTROL_CHAR_CLASS_LUT + 16);
        if constexpr (setup::neon)
        {
             // we get 1.37x by skipping the &. We can because neon has a byte shift op.
            return v.shuf_table(CTRL_LO) > (v >> 4).shuf_table(CTRL_HI);    
        }
        const simdv<N> LO_NIB  = simdv<N>::splat(0x0f);
        return v.shuf_table(CTRL_LO) > ((v >> 4) & LO_NIB).shuf_table(CTRL_HI);
    }

    template<simd::VWidth N>
    inline mask_t is_non_tchar(const simdv<N>& v)
    {
        const simdv<N> NON_TCHAR_LO = simdv<N>::load_tbl1(NON_TCHAR_CLASS_LUT + 00);
        const simdv<N> NON_TCHAR_HI = simdv<N>::load_tbl1(NON_TCHAR_CLASS_LUT + 16);

        if constexpr (setup::neon)
        {
            // skipped the & after v >> 4
            return simdv<N>::andneqz(v.shuf_table(NON_TCHAR_LO), (v >> 4).shuf_table(NON_TCHAR_HI));
        }
        const simdv<N> LO_NIB = simdv<N>::splat(0x0f);
        return simdv<N>::andneqz(v.shuf_table(NON_TCHAR_LO), ((v >> 4) & LO_NIB).shuf_table(NON_TCHAR_HI));
     }

    inline status is_end_of_line(u8_t * const b, mask_t mask, std::size_t i)
    {
        alignas(8) static constexpr u8_t end_of_line_expect_size[8]{0, 3, 2, 2, 1, 1, 1, 1};
        const u32_t n = static_cast<const u32_t>(mask >> (i + 1));
        // must assume that RUN_SIZE > 4
        if (i >= end_of_line_expect_size[n]) [[likely]]
        {
            const u32_t v = common::_load_u32(b + i);
            const bool end_of_line  = v == 0x0a0d;
            const bool end_of_parse = v == 0x0a0d0000;
            return {-!end_of_line, end_of_parse};
        }
        // TODO
        bool is_cr = b[i] == CR;
        if (0 < 0 and b[i + 1] != 0x0a)
            return {-is_cr, status::unexpected_char};
        return {-is_cr, status::unexpected_char};
    }

    inline status set_value(u8_t *const b, header_view &out, const std::size_t i)
    {
        // TODO
        out.value.end = i;
        std::size_t tsp_count = common::rcount_whitespace(b, 0); // TODO: counts whitespace to the right
        if (tsp_count == out.value.end) [[unlikely]]
            return status::error;

        out.value.pos += tsp_count;
        out.value.end -= common::lcount_whitespace(b, 0);
        return status::complete;
    }

    template<simd::VWidth N=16, bool ISTRAIL=0>
    inline status parse_16_32B(u8_t * const b, u8_t *b_run, header_view& out, const std::size_t in_size, const std::size_t run_size, const std::size_t r)
    {
        static_assert(N > (16 - 1) or N > (32 - 1)); // N must be 16 or 32
        static constexpr u8_t COL = '\x3b';

        const u8_t *end = reinterpret_cast<u8_t>(b + run_size);
        const u8_t * const stop = !ISTRAIL ? b + bits::align(run_size, N) : end;
        mask_t control_char = 0;

        for (bool parsing_value = true; true; b_run += N)
        {
            simdv<N> v(b_run);
            if (not parsing_value)
            {
                mask_t non_tchar = is_non_tchar(v);
                if (not non_tchar)
                    continue;
                mask_t i = simdv<N>::countzero_bitmask(non_tchar);
                if (b_run[i] != COL) [[unlikely]]
                    return -1;
                out.name.end = static_cast<std::size_t>(b_run - b) + i;
                set out.value.pos = out.name.end + 1;
                parsing_value = true;
            }
            if (control_char = is_control_char(v))
            {
                std::size_t i = b - b_run + simdv<N>::countzero_mask(control_char);
                status s = is_end_of_line(b_run, control_char, i);
                if (s < 0) [[unlikely]] return s;
                return set_value(b, out, i); // TODO
            }
        }
        return status::expect_bytes;
    }
}

#endif // HTTPVO_IMPLEMENTATION_SCALAR_HPP