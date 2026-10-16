#ifndef HTTPVO_IMPLEMENTATION_SCALAR_HPP
#define HTTPVO_IMPLEMENTATION_SCALAR_HPP
#include "implementation.hpp"

namespace httpvo::Implementation
{
    using namespace simd;
    static constexpr u8_t CR  = '\xd';
    static constexpr u8_t LF  = '\xa';
    static constexpr u8_t TAB = '\x9';
    static constexpr u8_t DEL = '\x7f';
    static constexpr u8_t NON_PRINTABLE_MAX = '\x19';

    inline bool is_valid(u8_t i) { return i > 0x20 and i < 0x7f; }

    inline status parse_char(const u8_t * const b, ReqLine& out, u64_t& tsp, const std::size_t run_size, const std::size_t i)
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

    inline status parse_chars(const u8_t * const b, ReqLine& out, u64_t &tsp, std::size_t run_size, std::size_t i)
    {
        status s {0};
        if ((s = parse_char(b, out, tsp, run_size, i+0)) < 1) return s;
        if ((s = parse_char(b, out, tsp, run_size, i+1)) < 1) return s;
        if ((s = parse_char(b, out, tsp, run_size, i+2)) < 1) return s;
        if ((s = parse_char(b, out, tsp, run_size, i+3)) < 1) return s;
        if ((s = parse_char(b, out, tsp, run_size, i+4)) < 1) return s;
        if ((s = parse_char(b, out, tsp, run_size, i+5)) < 1) return s;
        if ((s = parse_char(b, out, tsp, run_size, i+6)) < 1) return s;
        if constexpr (setup::no_vectorize)
            return parse_char(b, out, tsp, run_size, i+7);
        return s;
    }

    template<>
    inline status http::parse_line<0, 0>(u8_t * const b, u8_t *b_run, ReqLine& out, const std::size_t run_size, const mask_t mask [[maybe_unused]], u64_t tsp)
    {
        // TODO: OPTIMIZE THIS FUNCTION
        // DO NOT CALL AS STANDALONE
        status s{0};
        for (std::size_t i = 0; true; i += 16)
        {
            if ((s = parse_chars(b, out, tsp, run_size, i+0)) < 1)
                return s;
            if ((s = parse_chars(b, out, tsp, run_size, i+8)) < 1)
                return s;
        }
        return s;
    }

    template<simd::VWidth N=8, bool ISTRAIL=0>
    [[nodiscard]] inline status http::parse_line(u8_t * const b, u8_t *b_run, ReqLine& out, const std::size_t run_size, const mask_t mask, u64_t tsp)
    {
        static_assert(bits::is_power_of_two(N));

        if constexpr (setup::no_vectorize)
            return parse_line<0, 0>(b, b_run, out, run_size, mask, tsp);

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
        return parse_chars(b, out, tsp, run_size, b_run - b);
    }

    // see test/scripts/generate_shuffle_table.py for table generation and comments.
    alignas(16) static constexpr u8_t TCHAR_CLASS_LUT[32]{
        58, 63, 62, 63, 63, 63, 63, 63, 62, 62, 61, 21, 52, 21, 61, 28,
        00, 00, 01, 02, 04,  8, 16, 32, 00, 00, 00, 00, 00, 00, 00, 00
    };

    // see test/scripts/generate_shuffle2_table.py for table generation and comments.
    alignas(16) static constexpr u8_t CONTROL_CHAR_CLASS_LUT[32]{
        002, 002, 002, 002, 002, 002, 002, 002, 002, 001, 002, 002, 002, 002, 002, 003,
        001, 000, 127, 127, 127, 127, 127, 002, 127, 127, 127, 127, 127, 127, 127, 127,
    };

    template<simd::VWidth N>
    inline mask_t is_control_char(const simdv<N>& v)
    {
        // since the control bytes overlap, we can reduce to a simple nibble comparison. see /test/scripts/generate_shuffle_table2.py for table generation and comments
        const simdv<N> CTRL_LO = simdv<N>::load_tbl1(CONTROL_CHAR_CLASS_LUT + 00);
        const simdv<N> CTRL_HI = simdv<N>::load_tbl1(CONTROL_CHAR_CLASS_LUT + 16);

        if constexpr (setup::neon)
        {
            if constexpr (not setup::arm64)
                return ((v <= NON_PRINTABLE_MAX) | (v == DEL)) ^ (v == TAB); // no single 16B lookup (vqtbl1q). shuffle is not worth it since we have to do two shuffles for each lane halves.
            return (v & 0xf).shuf_table(CTRL_LO) > (v >> 4).shuf_table(CTRL_HI);
        }    
        return v.shuf_table(CTRL_LO) > ((v >> 4) & 0xf).shuf_table(CTRL_HI);
    }

    template<simd::VWidth N>
    inline mask_t is_non_tchar(const simdv<N>& v)
    {
        const simdv<N> TCHAR_LO = simdv<N>::load_tbl1(TCHAR_CLASS_LUT + 00);
        const simdv<N> TCHAR_HI = simdv<N>::load_tbl1(TCHAR_CLASS_LUT + 16);

        if constexpr (setup::neon)
           return (v & 0xf).shuf_table(TCHAR_LO).andeqz((v >> 4).shuf_table(TCHAR_HI));
        // skipped first 'v & 0xf' since v >= 0x80 falls to 0 and that is what we need
        return v.shuf_table(TCHAR_LO).andeqz(((v >> 4) & 0xf).shuf_table(TCHAR_HI));
     }

    inline status is_end_of_line(u8_t * const b, const std::size_t rem)
    {
        static constexpr u32_t CRLF     = 0x0a0d;
        static constexpr u32_t CRLFCRLF = 0x0a0d0a0d;
        if (rem > 3) [[likely]]
        {
            const u32_t v = common::_load_u32(b);
            const i8_t isnot_eol = -( (v & 0xffffu) != CRLF );
            return {isnot_eol, v == CRLFCRLF};
        }

        if (rem and  b[1] != LF) [[unlikely]]
            return status::unexpected_char;
        const i8_t isnot_cr = -( b[0] != CR );
        return {isnot_cr, status::unexpected_char};
    }

    template<simd::VWidth N, bool ISTRAIL=0>
    [[nodiscard]] inline status http::parse_header(u8_t * const b, u8_t *b_run, header_view& out, const std::size_t in_size, const std::size_t run_size, const std::size_t r)
    {
        static_assert(N == 16 or N == 32);
       
        static constexpr u8_t COL = '\x3a';

        const u8_t * const stop = b + (!ISTRAIL ? bits::align(run_size, N) : run_size);

        bool parsing_value = false;
        for (; b_run < stop; b_run += N)
        {
            simdv<N> v(b_run);
            if (not parsing_value)
            {
                mask_t non_tchar = is_non_tchar(v);
                if (not non_tchar)
                    continue;
                const std::size_t offset = simdv<N>::countzero_bitmask(non_tchar);
                if (b_run[offset] != COL) [[unlikely]]
                    return status::unexpected_char;
                out.name.end = static_cast<std::size_t>(b_run - b) + offset;
                out.value.pos = out.name.end + 1;
                parsing_value = true;
            }
            if (mask_t control_char = is_control_char(v))
            {
                const std::size_t offset = simdv<N>::countzero_bitmask(control_char);
                const std::size_t at = static_cast<std::size_t>(b_run - b) + offset;
                status s = is_end_of_line(b_run + offset, run_size - at);
                if (s < 0) [[unlikely]] return s;
                out.value.end = at;
                s = out.value.commit(reinterpret_cast<const u8_t *>(b));
                return s;
            }
        }
        return status::expect_bytes;
    }
}
#endif // HTTPVO_IMPLEMENTATION_SCALAR_HPP