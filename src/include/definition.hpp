#ifndef HTTPVO_DEFINITION_H
#define HTTPVO_DEFINITION_H
#include <cstdint>
#include <cstring>
#include <array>
#include <type_traits>
#include <limits>
#include <assert.h>

#if __has_include(<immintrin.h>)
#    include <immintrin.h>
#elif __has_include(<arm_neon.h>)
#    include <arm_neon.h>
#endif

#if 0
#    define HTTPVO_ARCH_ARM 1
#else
#    define HTTPVO_ARCH_ARM 0
#endif

#if defined(__x86_64__) || defined(_M_X64)
#    define HTTPVO_ARCH_X64 1
#else
#    define HTTPVO_ARCH_X64 0
#endif

#if defined(__GNUC__) && !defined(__clang__)
#    define HTTPVO_HAVE_GNUC __GNUC__
#else
#    define HTTPVO_HAVE_GNUC 0
#endif

#if defined(__clang__)
#    define HTTPVO_HAVE_CLANG __clang__
#else
#    define HTTPVO_HAVE_CLANG 0
#endif

#if HTTPVO_HAVE_CLANG || HTTPVO_HAVE_GNUC
#    define HTTPVO_GNUC_COMPARTIBLE 1
#else
#    define HTTPVO_GNUC_COMPARTIBLE 0
#endif

#if defined(_MSC_VER) && !defined(__clang__)
#    define HTTPVO_HAVE_MSVC _MSC_VER
#else
#   define HTTPVO_HAVE_MSVC 0
#endif

#if HTTPVO_HAVE_MSVC
#   include <intrin.h>
#endif

#if defined(__cplusplus)
#    define CXX_VERSION __cplusplus
#elif HTTPVO_HAVE_MSVC
#    define CXX_VERSION _MSVC_LANG
#else 
#    define CXX_VERSION 0
#endif

#if defined(__AVX2__)
#    define HTTPVO_HAVE__AVX2__   1
#else
#    define HTTPVO_HAVE__AVX2__   0
#endif

#if defined(__SSE2__)
#    define HTTPVO_HAVE__SSE2__   1
#else
#    define HTTPVO_HAVE__AVX2__   0
#endif

#if defined(__SSSE3__)
#    define HTTPVO_HAVE__SSSE3__   1
#else
#    define HTTPVO_HAVE__SSSE3__   0
#endif

#if defined(__SSE4_2__)
#    define HTTPVO_HAVE__SSE4_2__   1
#else
#    define HTTPVO_HAVE__SSE4_2__   0
#endif

#if defined(__ARM_NEON)
#    define HTTPVO_HAVE__ARM_NEON__ 1
# else
#    define HTTPVO_HAVE__ARM_NEON__ 0
#endif

#if defined(__BMI__)
#    define HTTPVO_HAVE__BMI__ 1
#else
#    define HTTPVO_HAVE__BMI__ 0
#endif

#if defined(__ORDER_LITTLE_ENDIAN__)
#   define HTTPVO_LITTLE_ENDIAN 1
#else
#  warning TODO: determine endianess at compile time
#  define HTTPVO_LITTLE_ENDIAN 0
#endif

#ifndef HTTPVO_OPTIMIZE_FOR_MOST_CASE
#    define HTTPVO_OPTIMIZE_FOR_MOST_CASE 1
#endif

#ifndef HTTPVO_SUPPORT_FULL_TCHAR
#    define HTTPVO_SUPPORT_FULL_TCHAR 1
#endif

#ifdef HTTPVO_STRICT_PARSER
#    undef  HTTPVO_NO_LEADING_WSP
#    undef  HTTPVO_NO_MULTI_WSP
#    undef  HTTPVO_ALLOW_EOL_AS_LINEFEED
#    define HTTPVO_STRICT_HTTP    1
#    define HTTPVO_NO_LEADING_WSP 1
#    define HTTPVO_NO_MULTI_WSP   1
#    define HTTPVO_ALLOW_EOL_AS_LINEFEED 0
#endif

#ifndef HTTPVO_STRICT_DELIM
#    define HTTPVO_STRICT_DELIM 0
#endif

#ifndef HTTPVO_NO_VECTORIZE
#    define HTTPVO_NO_VECTORIZE 0
#endif

#ifndef HTTPVO_NO_MULTI_WSP
#    define HTTPVO_NO_MULTI_WSP 1
#endif

#ifndef HTTPVO_NO_LEADING_WSP
#    define HTTPVO_NO_LEADING_WSP 0
#endif

#ifndef HTTPVO_ALLOW_EOL_AS_LINEFEED
#    define HTTPVO_ALLOW_EOL_AS_LINEFEED 0
#endif

#ifndef HTTPVO_PRIOTIZE_HTAB
#    define HTTPVO_PRIOTIZE_HTAB 0
#endif

#if CXX_VERSION >= 201703L
#    define HTTPVO_CXX17_LIKELY(...)   (__VA_ARGS__) [[likely]]
#    define HTTPVO_CXX17_UNLIKELY(...) (__VA_ARGS__) [[unlikely]]
#elif HTTPVO_GNUC_COMPARTIBLE
#    define HTTPVO_CXX17_LIKELY(...)   (__builtin_expect((__VA_ARGS__), 1))
#    define HTTPVO_CXX17_UNLIKELY(...) (__builtin_expect((__VA_ARGS__), 1))
#else
#    define HTTPVO_CXX17_LIKELY(x)   (x)
#    define HTTPVO_CXX17_UNLIKELY(x) (x)
#endif

#if CXX_VERSION >= 201703L
#    define HTTPVO_CXX17_CONSTEXPR constexpr
#else
#    define HTTPVO_CXX17_CONSTEXPR
#endif

#if HTTPVO_GNUC_COMPARTIBLE
#    define inline __attribute__((always_inline, flatten, artificial)) inline
#    define make_flat __attribute__((flatten))
#elif HTTPVO_HAVE_MSVC
#    define inline __declspec(forceinline flatten) inline
#    define make_flat 
#else
#    define inline inline
#    define make_flat 
#endif

#if HTTPVO_HAVE_GNUC
#    define TARGET(str) __attribute__((target(str)))
#else
#    define TARGET(str) 
#endif

#if HTTPVO_GNUC_COMPARTIBLE
#   define httpvo_memcopy __builtin_memcpy
#   define httpvo_discard_reginfo(r) __asm__("" : "+r"(r))
#   define HTTPVO_START_NO_DIAGNOSE_ARRAY_BOUNDS\
        _Pragma("GCC diagnostic push")\
        _Pragma("GCC diagnostic ignored \"-Warray-bounds\"")
#   define HTTPVO_STOP_NO_DIAGNOSE_ARRAY_BOUNDS\
        _Pragma("GCC diagnostic pop")
#else
#   define httpvo_memcopy memcpy
#   define httpvo_discard_reginfo(r) ((void)0)
#   if HTTPVO_HAVE_MSVC
#       define HTTPVO_START_NO_DIAGNOSE_ARRAY_BOUNDS\
            __Pragma(warning(push))\
            __Pragma(warning(disable: 4789))
#       define HTTPVO_STOP_NO_DIAGNOSE_ARRAY_BOUNDS\
            __Pragma(warning(pop))
#   else
#       define HTTPVO_START_NO_DIAGNOSE_ARRAY_BOUNDS
#       define HTTPVO_STOP_NO_DIAGNOSE_ARRAY_BOUNDS
#   endif
#endif

#ifndef HTTPVO_DEBUG
#    define HTTPVO_DEBUG 0
#else
#    include <iostream>
#endif

#define U32(x)  static_cast<const u32_t>(x)
#define U64(x)  static_cast<const u64_t>(x)
#define U32P(b) reinterpret_cast<u32_t *>(b)[0]

//////////////////////////////
//////////// HTTPVO ///////////
//////////////////////////////
namespace httpvo
{   
    using u8_t   = std::uint8_t;
    using i8_t   = std::int8_t;
    using u16_t  = std::uint16_t;
    using u32_t  = std::uint32_t;
    using u64_t  = std::uint64_t;
    using uint_t = typename std::conditional<std::numeric_limits<std::size_t>::digits < 64, u32_t, u64_t>::type;
    using mask_t = std::uint64_t;

    auto pass   = []{};

     namespace setup
     {
        constexpr u8_t avx_512 = 0;
        constexpr u8_t avx_2   = HTTPVO_HAVE__AVX2__;
        constexpr u8_t sse_4_2 = HTTPVO_HAVE__SSE4_2__;
        constexpr u8_t neon    = HTTPVO_HAVE__ARM_NEON__;
        constexpr u8_t int_64  = 1;

        constexpr u8_t arm64  = 0;
        constexpr u8_t arm32  = 0;
        constexpr u8_t x86_64 = 0;
        constexpr bool little_endian = HTTPVO_LITTLE_ENDIAN;

        constexpr u8_t rtsp = 4;
        constexpr u8_t http = 8;

        constexpr bool debug             = HTTPVO_DEBUG;
        constexpr bool no_multispace     = HTTPVO_NO_MULTI_WSP;
        constexpr bool no_leading_space  = HTTPVO_NO_LEADING_WSP;
        constexpr bool no_vectorize      = HTTPVO_NO_VECTORIZE;
        constexpr bool end_of_line_is_line_feed = HTTPVO_ALLOW_EOL_AS_LINEFEED;
        constexpr bool optimize_for_most_case  = HTTPVO_OPTIMIZE_FOR_MOST_CASE;
        constexpr bool priotize_horizontal_tab = HTTPVO_PRIOTIZE_HTAB;

        constexpr u8_t req_line_min_size = 14;
        constexpr u8_t hd_line_min_size  = 5;
     }
}
#undef HTTPVO_DEBUG
#undef HTTPVO_NO_MULTI_WSP
#undef HTTPVO_LITTLE_ENDIAN
#undef HTTPVO_NO_LEADING_WSP
#undef HTTPVO_NO_VECTORIZE
#undef HTTPVO_OPTIMIZE_FOR_MOST_CASE
#undef HTTPVO_ALLOW_EOL_AS_LINEFEED
#undef HTTPVO_PRIOTIZE_HTAB
#endif // HTTPVO_DEFINITION_HPP