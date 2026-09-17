#ifndef HTTPVO_DEFINITION_H
#define HTTPVO_DEFINITION_H
#include <cstdint>
#include <cstring>
#include <array>
#include <type_traits>
#include <limits>
#include <cassert>

//#define __SSE4_2__ 1 // remove this

#define is ==
#define isnot !=

#if defined(__GNUC__) || (defined(__clang__) && !defined(_MSC_VER))
#    undef  __HAVE_GNUC__
#    define __HAVE_GNUC__ 1
#else
#    define __HAVE_GNUC__ 0
#endif
#if defined(_MSC_VER)
#    undef  __HAVE_MSVC__
#    define __HAVE_MSVC__ _MSC_VER
#else
#   define __HAVE_MSVC__ 0
#endif

#if defined(__AVX2__) || defined(__SSSE3__) || defined(__SSE4_2__) || defined(__SSE2__)
#    if defined(__AVX2__)
#        define HAVE__AVX2__   1
#    elif defined(__SSE2__)
#        define HAVE__SSE2__   1
#    elif defined(__SSSE3__)
#        define HAVE__SSSE3__  1
#    elif defined(__SSE4_2__)
#        define HAVE__SSE4_2__ 1
#    endif
#    include <immintrin.h>
#elif defined(__ARM_NEON)
#    define HAVE__ARM_NEON__ 1
#    include <arm_neon.h>
#endif

#ifndef HAVE__AVX2__
#    define HAVE__AVX2__ 0
#endif
#ifndef HAVE__SSE2__
#    define HAVE__SSE2__  0
#endif
#ifndef HAVE__SSE4_2__
#    define HAVE__SSE4_2__  0
#endif
#ifndef HAVE__ARM_NEON__
#   define HAVE__ARM_NEON__ 0
#endif

#define __UNALIGNED_ACCESS__ 0
#if defined(__x86_64__) || defined(__amd64__) || defined(__aarch64__)
#    define __HAVE_SUPPORT_FOR_UNALIGNED__ __UNALIGNED_ACCESS__
#elif defined(_M_X64) || defined(_M_AMD64) || defined(_M_ARM64)
#    define __HAVE_SUPPORT_FOR_UNALIGNED__ __UNALIGNED_ACCESS__
#else
#    define __HAVE_SUPPORT_FOR_UNALIGNED__ 0
#endif
/////////////////////////////////////
////////// PERFORMANCE //////////////
#ifndef OPTIMIZE_FOR_MOST_CASE
#    define OPTIMIZE_FOR_MOST_CASE 1
#endif

#ifndef SUPPORT_FULL_TCHAR
#    define SUPPORT_FULL_TCHAR 1
#endif

#ifndef STRICT_HTTP
#    define STRICT_HTTP 1
#    define HTTP_STRICT_DELIM 1
#endif

#ifndef HTTP_STRICT_DELIM
#    define HTTP_STRICT_DELIM 1
#endif

#ifndef NO_VECTORIZE
#    define NO_VECTORIZE 0
#endif

#ifndef IGNORE_LEADING_SP
#    define IGNORE_LEADING_SP 0
#endif
/////////////////////////////////////
/////////////////////////////////////

// specialization
#ifndef MIX_AVX2_SSE
#    define MIX_AVX2_SSE 0 
#endif
#ifndef MIX_AVX512_AVX2
#    define MIX_AVX512_AVX2 0
#endif

// no copying of trailing bytes
#ifndef NO_COPY_TRAILS
#    define NO_COPY_TRAILS 0
#endif

// branch prediction
#if 0
#if __HAVE_GNUC__
#    define likely(x)   (__builtin_expect(!!(x), 1))
#    define unlikely(x) (__builtin_expect(!!(x), 0))
#elif defined(__cplusplus) && __cplusplus >= 202002L
#    define likely(x)   (x) [[likely]]
#    define unlikely(x) (x) [[unlikely]]
#else
#    define likely(x)   (x)
#    define unlikely(x) (x)
#endif
#endif

// inline
#if   __HAVE_GNUC__
#    define inline    __attribute__((__always_inline__)) inline
#    define make_flat __attribute__((flatten))
#elif __HAVE_MSVC__
#    define inline  __forceinline
#    define make_flat 
#else
#    define inline inline
#    define make_flat 
#endif

// target
#if __HAVE_GNUC__
#    define TARGET(str) __attribute__((target(str)))
#else
#    define TARGET(str) 
#endif

#define U32(x)  static_cast<const u32_t>(x)
#define U64(x)  static_cast<const u64_t>(x)
#define U32P(b) reinterpret_cast<u32_t *>(b)[0]

//////////////////////////////
//////////// HTTPVO ///////////
//////////////////////////////
namespace httpvo
{
    template <typename base> struct simd64;
    
    using u8_t  = std::uint8_t;
    using i8_t  = std::int8_t;
    using u16_t = std::uint16_t;
    using u32_t = std::uint32_t;
    using u64_t = std::uint64_t;

    auto pass   = []{};

     static constexpr bool mix_avx512_avx2 = MIX_AVX512_AVX2;
     static constexpr bool mix_avx2_sse4   = MIX_AVX2_SSE;

     static constexpr u8_t AVX512 = 1;
     static constexpr u8_t AVX2   = 2;
     static constexpr u8_t SSE4   = 3;
     static constexpr u8_t INT64  = 4;
}
#endif // HTTPVO_DEFINITION_HPP