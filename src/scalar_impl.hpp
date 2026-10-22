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
    static constexpr u8_t COL = '\x3a';
    static constexpr u8_t NON_PRINTABLE_MAX = '\x19';

    inline bool is_valid(u8_t i) { return i > 0x20 and i < 0x7f; }

    inline status parse_char(const u8_t * const b, ReqLine& out, u64_t& tsp, const std::size_t run_size, const std::size_t i)
    {
        // TODO: status sign changes: fix return values
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
            return out.set_version(b);
        }
        return status::unexpected_char;
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
        const u8_t * const stop = not ISTRAIL ? b + bits::align(run_size, N) : end;
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
                const std::size_t offset = simdv<N>::countzero_bitmask(out_mask);
                u8_t c = b_run[offset];
                if (common::is_whitespace(c)) [[likely]]
                {
                    if (tsp & out_mask) [[unlikely]]
                    {
                        if constexpr (setup::no_multispace)
                            return status::bad_whitespace;
                    }
                    else if (not out.advance(static_cast<const std::ptrdiff_t>(b_run - b) + offset)) [[unlikely]]
                        return status::expected_end_of_line;
                    tsp = bits::least_set_bit(out_mask) << simdv<N>::bitpos;
                    continue;
                }
                if (c == CR)
                {
                    const u8_t * const p = b_run + offset;
                    if (p == (end - 1)) [[unlikely]]
                        return {0, status::expect_line_feed};
                    if (p[1] != LF) [[unlikely]]
                        return status::unexpected_char;
                    out.advance(static_cast<const std::ptrdiff_t>(p - b));
                    return out.set_version(b);
                }
                if constexpr (setup::end_of_line_is_line_feed)
                {
                    if (c == LF)
                    {
                        out.advance(static_cast<const std::ptrdiff_t>(b_run - b) + offset);
                        return out.set_version(b);
                    }
                }
                return status::unexpected_char;
            }
            tsp = bool(tsp) << (simdv<N>::bitpos - 1);
        }
        if (run_size > (N - 1)) [[likely]]
        {
            // process trailing bytes by overlapping last read bytes with the remaining bytes
            const std::size_t r = N - (run_size & (N - 1));
            const std::size_t s = r * simdv<N>::bitpos;
            return parse_line<N, true>(b, b_run - r, out, run_size, constant::cff << s, tsp << s);
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

    inline status http::is_end_of_line(u8_t * const b, const std::size_t rem)
    {
        static constexpr u32_t CRLF     = 0x0a0d;
        static constexpr u32_t CRLFCRLF = 0x0a0d0a0d;
        if (rem > 3) [[likely]]
        {
            if constexpr (not setup::no_vectorize and setup::little_endian)
            {
                const u32_t v = common::_load_u32(b);
                this->end_of_parse = v == CRLFCRLF;
                return {(v & 0xffffu) != CRLF, status::unexpected_char};
            }
            const int isn_eol  = (b[0] != CR) & (b[1] != LF);
            this->end_of_parse =  b[2] == CR and b[3] == LF;
            return {isn_eol, status::unexpected_char};
        }
        if (rem and  b[1] != LF) [[unlikely]]
            return status::unexpected_char;
        return {b[0] != CR, status::unexpected_char};
    }

    template<simd::VWidth N, bool ISTRAIL=0>
    [[nodiscard]] inline status http::parse_header(u8_t * const b, u8_t *b_run, header_view& out, const std::size_t in_size, const std::size_t run_size, const std::size_t r)
    {
        static_assert(N == 16 or N == 32);
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
                const std::size_t at = static_cast<std::ptrdiff_t>(b_run - b) + offset;
                if (is_end_of_line(b_run + offset, run_size - at)) [[unlikely]]
                 return status::unexpected_char;
                out.value.end = at;
                return out.value.commit(reinterpret_cast<const u8_t *>(b));
            }
        }
        return status::expect_bytes;
    }
    
    // 10 - name token; 11 = value token
    static constexpr u8_t TCHAR_LUT[256]{
        00, 00, 00, 00, 00, 00, 00, 00, 10, 00, 00, 00, 00, 00, 00, 00,
        00, 00, 00, 00, 00, 00, 00, 00, 00, 10, 10, 10, 10, 10, 10, 10,
        10, 11, 10, 11, 11, 11, 11, 11, 10, 10, 11, 11, 10, 11, 11, 10,
        11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 10, 10, 10, 10, 10, 10,
        10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
        11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 10, 10, 10, 11, 11,
        11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
        11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 10, 11, 10, 00, 10,
        10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
        10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
        10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
        10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
        10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
        10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
        10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
        10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10};

    inline status::statat_t is_valid_name_token_(const u8_t * const b)
    {
        std::size_t at = 0;
        for (;  at < 8 and TCHAR_LUT[b[at]] & 1; at++) pass();
        return {at < 8, at};
    }

    inline status::statat_t is_valid_name_token_loop(const u8_t * const b, const std::size_t end, size_t at)
    {
        for (;  at < end and TCHAR_LUT[b[at]] & 1; at++) pass();
        return {at < end, at};
    }

    make_flat inline status::statat_t is_valid_name_token(const u8_t * const b)
    {
        if constexpr (setup::optimize_for_most_case and not setup::no_vectorize)
        {
            const simdv<8> v{b};
            if (const mask_t outside = bits::andnot(constant::c80, v.ascii_fast_tchar())) [[unlikely]] // some other char that is not A-Z0-9 or -, but may be valid
                return is_valid_name_token_loop(reinterpret_cast<const u8_t * const>(b), 8, simdv<8>::countzero_bitmask(outside));
            return {0, 8};
        }
        return is_valid_name_token_(reinterpret_cast<const u8_t * const >(b));
    }

    make_flat inline status::statat_t is_valid_name(const u8_t * const b, const u8_t * const end, const std::size_t len)
    {
        for (const u8_t *p = b; p < end; p += 8)
        {
            if (status::statat_t s = is_valid_name_token(p))
                return s += static_cast<std::ptrdiff_t>(p - b);
        }
        if (const std::size_t r = len % 8)
            return is_valid_name_token_loop(b, len - r, len);
        return {0, len};
    }

    inline std::size_t is_valid_value_token_loop(const u8_t *const b, const size_t len)
    {
        for (std::size_t at = 0; at < len; at++)
        {
            if (not TCHAR_LUT[b[at]])
                return at;
        }
        return 0;
    }

    inline std::size_t is_valid_value_token(const u8_t *const b)
    {
        if constexpr (setup::no_vectorize)
            return is_valid_value_token_loop(b, 8);
        if (const mask_t offset = simdv<8>(b).non_printable())
            return simdv<8>::countzero_bitmask(offset);
        return 0;
    }

    make_flat inline status::statat_t is_valid_value(const u8_t * const b, const u8_t * const end, const std::size_t len)
    {
        for (const u8_t *p = b; p < end; p += 8)
        {
            if (const std::size_t offset = is_valid_value_token(p))
                return {1, static_cast<std::ptrdiff_t>(p - b) + offset};
        }
        if (const std::size_t r = len % 8) [[likely]]
        {
            if (len > 8) [[likely]]
            {
                const u8_t * const p = end - (8 - r);
                if (const std::size_t offset = is_valid_value_token(p))
                    return {1, static_cast<std::ptrdiff_t>(p - b) + offset};
                return {0, len};
            }
            return is_valid_value_token_loop(end - r, r);
        }
        return {0, len};
    }

    template<>
    [[nodiscard]] inline status http::parse_header<0>(u8_t * const b, u8_t *b_run [[maybe_unused]], header_view& out, const std::size_t in_size [[maybe_unused]], const std::size_t run_size, const std::size_t r [[maybe_unused]])
    {  
        const u8_t * const end = b + bits::align(run_size, 8);
        status::statat_t  s  = is_valid_name(b, end, run_size);
        const std::size_t at = s.at;

        if (run_size == at) [[unlikely]]
            return status::expect_bytes;
        if (b[at] != COL) [[unlikely]]
            return status::unexpected_char;
        out.name.end = at;

        out.value.pos = at + 1;
        s = is_valid_value(b + (at + 1), end, run_size - at);
        if (run_size == s.at) 
            return status::expect_bytes;
        // TODO: end of line check
        out.value.end = s.at;
        return out.value.commit(reinterpret_cast<const u8_t *>(b));
    }
} // NAMESPACE::HTTVO
#endif // HTTPVO_IMPLEMENTATION_SCALAR_HPP