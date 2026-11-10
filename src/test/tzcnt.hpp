// tzcnt.hpp - zero-safe count-trailing-zeros for integral types.
//
// Returns the bit width of T for x == 0 (8, 16, 32, 64 ...), on every backend.
//
// Assumes the project provides: u32_t, u64_t, the HTTPVO_* feature macros,
// and scalar fallbacks tzcnt_scalar_u32 / tzcnt_scalar_u64.
//
// Tested: GCC 13.3 and Clang 18.1.3 on x86-64, with and without -mbmi.
// Untested: the MSVC and ARM branches.

#pragma once

#include <cstdint>
#include <limits>
#include <type_traits>

#if HTTPVO_HAVE_BMI || (HTTPVO_HAVE_MSVC__ && HTTPVO_HAVE_AVX2)
#  define HAVE_TZCNT 1
#  include <immintrin.h>
#else
#  define HAVE_TZCNT 0
#endif

#if HTTPVO_HAVE_MSVC__
#  include <intrin.h>
#endif

namespace detail {

// Returns 32 for x == 0.
inline int ctz32(u32_t x) noexcept
{
#if HAVE_TZCNT
    return static_cast<int>(_tzcnt_u32(x));
#elif HTTPVO_GNUC_COMPARTIBLE__
    return x ? __builtin_ctz(x) : 32;
#elif HTTPVO_HAVE_MSVC__ && HTTPVO_ARM_ARCH
    return static_cast<int>(_CountTrailingZeros(x));            // returns 32 for 0
#elif HTTPVO_HAVE_MSVC__
    unsigned long n;
    return _BitScanForward(&n, x) ? static_cast<int>(n) : 32;
#else
    return tzcnt_scalar_u32(x);
#endif
}

// Returns 64 for x == 0.
inline int ctz64(u64_t x) noexcept
{
#if HAVE_TZCNT && (defined(__x86_64__) || defined(_M_X64))
    return static_cast<int>(_tzcnt_u64(x));
#elif HTTPVO_GNUC_COMPARTIBLE__
    return x ? __builtin_ctzll(x) : 64;
#elif HTTPVO_HAVE_MSVC__ && HTTPVO_ARM_ARCH
    return static_cast<int>(_CountTrailingZeros64(x));          // returns 64 for 0
#elif HTTPVO_HAVE_MSVC__ && defined(_M_X64)
    unsigned long n;
    return _BitScanForward64(&n, x) ? static_cast<int>(n) : 64;
#else
    return tzcnt_scalar_u64(x);
#endif
}

} // namespace detail

// Note: the return type must stay int. On GCC (13.3) an unsigned return type
// stops the x ? ctz(x) : N pattern from collapsing to a bare tzcnt.
template<typename T>
int tzcnt(T x) noexcept
{
    static_assert(std::is_integral_v<T> && !std::is_same_v<T, bool>);
    using U = std::make_unsigned_t<T>;
    constexpr int N = std::numeric_limits<U>::digits;   // full width, sign bit included
    static_assert(N <= 64);
    const U u = static_cast<U>(x);

    if constexpr (N < 32)        // sentinel bit at position N makes zero return N
        return detail::ctz32(static_cast<u32_t>(u) | (u32_t{1} << N));
    else if constexpr (N == 32)
        return detail::ctz32(static_cast<u32_t>(u));
    else
        return detail::ctz64(static_cast<u64_t>(u));
}
