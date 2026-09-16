#ifndef HTTPVO_COMMON_HPP
#define HTTPVO_COMMON_HPP
#include "../include/definition.hpp"
#include "../include/constants.hpp"
#include "../include/bits.hpp"
#include "../include/tables.hpp"

namespace httpvo::common
{
    inline constexpr u64_t _dup(u8_t v)
    {
        return U64(v) * constant::c01;
    }

    inline u64_t _load_u64(void *b)
    {
        #if HAVE__ARM_NEON__
        return vget_lane_u64(vld1_u64(reinterpret_cast<const u64_t *>(b)), 0);
        #elif __HAVE_SUPPORT_FOR_UNALIGNED__
        return reinterpret_cast<u64_t *>(b)[0];
        #endif
        u64_t v;
        __builtin_memcpy(&v, b, 8);
        return v;
    }

    inline u64_t _cmpeqz(u64_t v)
    {
        #if HAVE__ARM_NEON__
        return vget_lane_u64(vreinterpret_u64(vceq_u8(vcreate_u8(v), vcreate_u8(0))), 0);
        #endif
        return bits::andnot(constant::c80, v | ((v & constant::c7f) + constant::c7f));
    }

    inline u64_t _cmpeq(u64_t u, u64_t v)
    {
        // The scalar approach ties with ARM's 64bits intrinsic perf in a 64bit machine. However, in a 32bit machine, the scalar emits twice the instructions (no native 64bits reqister), and very slow when called repeatedly
        #if HAVE__ARM_NEON__
        return vget_lane_u64(vreinterpret_u64(vceq_u8(vcreate_u8(u), vcreate_u8(v))), 0);
        #endif
        return _cmpeqz(u ^ v);
    }

    inline u64_t _cmpeq(u64_t u, u64_t v, u64_t w)
    {
        return _cmpeq(u, v) | _cmpeq(u, w);
    }

    inline u64_t _cmpgtz(u64_t v)
    {
        return (v | ((v & constant::c7f) + constant::c7f)) & constant::c80;
    }

    template<u8_t A>
    inline u64_t _cmplt(u64_t v)
    {
        static_assert(A < 0x7f);
        static constexpr u64_t a = _dup(0x7f + A);
        return (a - (v & constant::c7f)) & bits::andnot(constant::c80, v);
    }

    template<u8_t A>
    inline u64_t _cmpgt(u64_t v)
    {
        static_assert(A < 0x7f);
        static constexpr u64_t a = _dup(0x7f - A);
        return (v | (a + (v & constant::c7f))) & constant::c80;
    }

    template <u8_t A, u8_t B, typename T>
    inline u64_t _cmp_gt_and_lt(T v)
    {
        #if HAVE__ARM_NEON__ && 0 // scalar is faster
        if constexpr (sizeof(T) == 8)
        {
            static const uint8x8_t a = vdup_n_u8(A);
            static const uint8x8_t b = vdup_n_u8(B);
            uint8x8_t x = vcreate_u8(v);
            uint8x8_t o = vand_u8(vcgt_u8(x, a), vclt_u8(x, b));
            return vget_lane_u64(vreinterpret_u64_u8(o), 0);
        }
        #endif
        static_assert(A < 0x7f && B < 0x80);
        static constexpr T a = static_cast<T>(_dup(0x7f - A));
        static constexpr T b = static_cast<T>(_dup(0x7f + B));
        return (b - (v & constant::c7f)) & (a + (v & constant::c7f)) & bits::andnot(constant::c80, v);
    }

    inline u64_t ascii_letters(u64_t v)
    {
        return (constant::Z - (v & constant::AZ_const)) & (constant::A + (v & constant::AZ_const)) & bits::andnot(constant::c80, v);
    }

    inline u64_t ascii_numbers_v(u64_t v)
    {
        return _cmp_gt_and_lt<'\x2f', '\x3a'>(v);
    }

    inline u64_t ascii_numbers(u64_t v)
    {
        return _cmplt<10>(v ^ constant::c30);
    }

    inline u64_t ascii_hyphen(u64_t v)
    {
        return _cmpeqz(v ^ constant::hyphen);
    }

    inline u64_t ascii_fast_tchar(u64_t v)
    {
        return ascii_letters(v) | ascii_numbers(v) | ascii_hyphen(v);
    }

    inline u64_t non_printable(u64_t v)
    {
        // Non printable characters here are 0x7f (DEL) or characters below 0x20 (sp)
        static constexpr u64_t u = constant::c80 | constant::c20;
        return (u - (((v & constant::c7f) + constant::c01) & constant::c7f)) & bits::andnot(constant::c80, v);
    }

    inline u8_t is_whitespace(u8_t x)
    {
        return (x == '\x20') or (x == '\x09'); // only for space and horizontal tab
    };

    inline std::size_t rcount_whitespace(void *b, u64_t len)
    {
        std::size_t i = 0;
        while (i < len and is_whitespace(reinterpret_cast<u8_t *>(b)[i++])) pass();
        return i;
    }
    
    inline std::size_t lcount_whitespace(void *b, u64_t len)
    {
        std::size_t i = len;
        while (i and is_whitespace(reinterpret_cast<u8_t *>(b)[--i])) pass();
        return len - i;
    }
    
    inline bool is_valid_name_token_(u8_t *b)
    {
        auto &x = tables::tchar_map;
        if constexpr (OPTIMIZE_FOR_MOST_CASE)
        {
            return x[b[0]] & x[b[1]] & x[b[2]] & x[b[3]] &
                   x[b[4]] & x[b[5]] & x[b[6]] & x[b[7]];
        }
        // most compilers will unroll this anyway
        int i = 0;
        while (i < 8 and x[b[i++]]) [[likely]] pass();
        return i == 8;
    }

    make_flat inline bool is_valid_name_token(void *b)
    {
        if constexpr (OPTIMIZE_FOR_MOST_CASE)
        {
            // Most tokens in  header names are usually a-z, A-Z, 0-9 or -
            return ascii_fast_tchar(reinterpret_cast<u64_t *>(b)[0]) or is_valid_name_token_(reinterpret_cast<u8_t *>(b));
        }
        return is_valid_name_token_(reinterpret_cast<u8_t *>(b));
    }

    inline bool is_valid_name_token_loop(u8_t *b, std::size_t len)
    {
        auto &x = tables::tchar_map;
        int i = 0;
        while (i < len and x[b[i++]]) [[likely]] pass();
        return i == len;
    }

    make_flat inline bool is_valid_name(u8_t *b, std::size_t& len)
    {
        if constexpr (not STRICT_HTTP or IGNORE_LEADING_SP)
            len -= is_whitespace(b[len - 1]);
        const u8_t *end = b + (len & ~(constant::int_size - 1));
        for (; b != end and is_valid_name_token(b); b += 8) [[likely]] pass();
        const u64_t r = len % constant::int_size;
        if (b != end or not r)
            return b == end;
        return is_valid_name_token_loop(b, r);
    }

    inline bool version_is_http_1(void *b)
    {
        return constant::mask_http_1 == (reinterpret_cast<u64_t *>(b)[0] & 0x00ffffffffffffffULL);
    }  
}
#endif // HTTPVO_COMMON_HPP