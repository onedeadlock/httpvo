#ifndef HTTPVO_BITS_HPP
#define HTTPVO_BITS_HPP
#include "definition.hpp"
#include "constants.hpp"

namespace httpvo::bits
{
    inline u64_t andnot(const u64_t x, const u64_t y)
    {
        return x & ~y;
    }

    inline u64_t least_set_bit(const u64_t x)
    {
        return x & -x; // blsi
    }

    inline u64_t clear_least_set_bit(const u64_t x)
    {
        return x & (x - 1); // blsr
    }

    inline u64_t set_bits_up_to_least_bit(const u64_t x)
    {
        return x ^ (x - 1); // blsmask (includes least bit)
    }

    inline constexpr bool is_power_of_two(const u64_t x)
    {
        return !(x & (x - 1));
    }

    inline constexpr u64_t align(const u64_t x, const u64_t n)
    {
        return x & ~(n - 1);
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

    inline u64_t blsfill(u64_t x)
    {
        return x | (x - 1);
    }

    inline u64_t xlsfill(u64_t x)
    {
        return x ^ -x;
    }

    inline int tzcnt32T(const u32_t x)
    {
        return constant::DeBruijn32_seq[(least_set_bit(x) * constant::DeBruijn32_const) >> 27];
    }

    inline int tzcnt64T(const u64_t x)
    {
        return constant::DeBruijn64_seq[(set_bits_up_to_least_bit(x) * constant::DeBruijn64_const) >> 58];
    }

    inline int tzcnt32(const u32_t x) noexcept
    {
#       if HTTPVO_GNUC_COMPARTIBLE
            return x ? __builtin_ctz(x) : 32;
#       elif HTTPVO_HAVE_MSVC && HTTPVO_ARCH_ARM
            return static_cast<int>(_CountTrailingZeros(x));
#       elif HTTPVO_HAVE_MSVC
            unsigned long n;
            return x ? (_BitScanForward(&n, x), n) : 32;
#       else
            return tzcnt32T(x);
#       endif
    }

    inline int tzcnt64(const u64_t x) noexcept
    {
#       if HTTPVO_GNUC_COMPARTIBLE
            return x ? __builtin_ctzll(x) : 64;
#       elif HTTPVO_HAVE_MSVC && HTTPVO_ARM_ARCH
            return static_cast<int>(_CountTrailingZeros64(x));
#       elif HTTPVO_HAVE_MSVC && HTTPVO_ARCH_X64
            unsigned long n;
            return x ? (_BitScanForward64(&n, x), n) : 64;
#       else
            return tzcnt64T(x);
#       endif
    }

    template <typename T>
    inline int tzcnt(T x) noexcept
    {
        static_assert(std::is_integral<T>() and not std::is_same<T, bool>());
        using U = typename std::make_unsigned<T>::type;
        constexpr int N = std::numeric_limits<U>::digits;
        static_assert(N <= 64);
        const U v = static_cast<U>(x);
        if HTTPVO_CXX17_CONSTEXPR (N < 32) // (v | 1 << N) makes v == 0 return N
            return tzcnt32(static_cast<u32_t>(v) | (static_cast<u32_t>(1) << N));
        else if HTTPVO_CXX17_CONSTEXPR (N == 32)
            return tzcnt32(static_cast<u32_t>(v));
        return tzcnt64(static_cast<u64_t>(v));
    }
}
#endif // HTTPVO_BITS_HPP