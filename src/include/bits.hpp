#ifndef HTTPVO_BITS_HPP
#define HTTPVO_BITS_HPP
#if   HTTPVO_HAVE_MSVC__
#   include <intrin.h>
#elif HTTPVO_GNUC_COMPAT__
#include <x86intrin.h>
#endif
#include "definition.hpp"
#include "constants.hpp"

namespace httpvo::bits
{
    __attribute__((const)) inline u64_t andnot(const u64_t x, const u64_t y)
    {
        return x & ~y;
    }

     __attribute__((const)) inline u64_t lowest_set_bit(const u64_t x)
    {
        return x & -x; // blsi
    }

     __attribute__((const)) inline u64_t clear_lowest_set_bit(const u64_t x)
    {
        return x & (x - 1); // blsr
    }

    inline u64_t ltrim(u64_t x)
    {
        return andnot(x, x << 1);
    }

    inline u64_t rtrim(u64_t x)
    {
        return andnot(x, x >> 1);
    }

    inline u64_t bltrim(u64_t x)
    {
        return andnot(x, x << 8);
    }

    inline u64_t brtrim(u64_t x)
    {
        return andnot(x, x >> 8);
    }

    inline u64_t tzmask(u64_t x)
    {
        return andnot(x - 1, x);
    }

    inline u64_t blsmask(u64_t x)
    {
        return x ^ (x - 1);
    }

    inline u64_t blsfill(u64_t x)
    {
        return x | (x - 1);
    }

    inline u64_t xlsfill(u64_t x)
    {
        return x ^ -x;
    }

    u64_t tzcnt(u64_t x)
    {
        if constexpr (setup::debug)
            assert(x > 0);
#if defined(_tzcnt_u64)
            return _tzcnt_u64(x);
#elif HTTPVO_GNUC_COMPAT__
        return __builtin_ctzll(x);
#else
            x |= x >> 1;  x |= x >> 2;
            x |= x >> 4;  x |= x >> 8;
            x |= x >> 16; x |= x >> 32;
            return constant::DeBruijn64_seq[(x * constant::DeBruijn64_const) >> 58];
#endif
    }
}
#endif // HTTPVO_BITS_HPP