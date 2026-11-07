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

#ifndef HTTPVO_DEBUG
#    define HTTPVO_DEBUG 0
#else
#    include <iostream>
#endif

#if defined(__GNUC__)
#    define HTTPVO_HAVE_GNUC__ __GNUC__
#else
#    define HTTPVO_HAVE_GNUC__ 0
#endif

#if defined(__clang__)
#    define HTTPVO_HAVE_CLANG__ __clang__
#else
#    define HTTPVO_HAVE_CLANG__ 0
#endif

#if HTTPVO_HAVE_CLANG__ || HTTPVO_HAVE_GNUC__
#    define HTTPVO_GNUC_COMPARTIBLE__ 1
#else
#    define HTTPVO_GNUC_COMPARTIBLE__ 0
#endif

#if defined(_MSC_VER) && !defined(__clang__)
#    define HTTPVO_HAVE_MSVC__ _MSC_VER
#else
#   define HTTPVO_HAVE_MSVC__ 0
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

#ifndef HTTPVO_STRICT_HTTP
#    define HTTPVO_STRICT_HTTP    1
#    define HTTPVO_STRICT_DELIM   1
#    define HTTPVO_NO_LEADING_WSP 1
#    define HTTPVO_NO_MULTI_WSP   1
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

#ifndef HTTPVO_NO_COPY_TRAILS
#    define HTTPVO_NO_COPY_TRAILS 0
#endif

#if 0
#if  HTTPVO_GNUC_COMPARTIBLE__
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

#if  HTTPVO_GNUC_COMPARTIBLE__
#    define inline    __attribute__((__always_inline__)) inline
#    define make_flat __attribute__((flatten))
#elif HTTPVO_HAVE_MSVC__
#    define inline  __forceinline
#    define make_flat 
#else
#    define inline inline
#    define make_flat 
#endif

#if HTTPVO_HAVE_GNUC__
#    define TARGET(str) __attribute__((target(str)))
#else
#    define TARGET(str) 
#endif

#if HTTPVO_GNUC_COMPARTIBLE__
#   define httpvo_memcopy __builtin_memcpy
#   define httpvo_discard_reginfo(r) __asm__("" : "+r"(r))
#   define HTTPO_START_NO_DIAGNOSE_ARRAY_BOUNDS\
        _Pragma("GCC diagnostic push")\
        _Pragma("GCC diagnostic ignored \"-Warray-bounds\"")
#   define HTTPO_STOP_NO_DIAGNOSE_ARRAY_BOUNDS\
        _Pragma("GCC diagnostic pop")
#else
#   define httpvo_memcopy memcpy
#   define httpvo_discard_reginfo(r) ((void)0)
#   if HTTPVO_HAVE_MSVC__
#       define HTTPO_START_NO_DIAGNOSE_ARRAY_BOUNDS\
            __Pragma(warning(push))\
            __Pragma(warning(disable: 4789))
#       define HTTPO_STOP_NO_DIAGNOSE_ARRAY_BOUNDS\
            __Pragma(warning(pop))
#   endif
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
    using mask_t = std::uint64_t;

    auto pass   = []{};

     namespace setup
     {
        static constexpr u8_t avx_512 = 0;
        static constexpr u8_t avx_2   = HTTPVO_HAVE__AVX2__;
        static constexpr u8_t sse_4_2 = HTTPVO_HAVE__SSE4_2__;
        static constexpr u8_t neon    = HTTPVO_HAVE__ARM_NEON__;
        static constexpr u8_t int_64  = 1;

        static constexpr u8_t arm64  = 0;
        static constexpr u8_t arm32  = 0;
        static constexpr u8_t x86_64 = 0;
        static constexpr bool little_endian = HTTPVO_LITTLE_ENDIAN;

        static constexpr u8_t rtsp = 4;
        static constexpr u8_t http = 8;

        static constexpr bool debug             = HTTPVO_DEBUG;
        static constexpr bool no_multispace     = HTTPVO_NO_MULTI_WSP;
        static constexpr bool no_leading_space  = HTTPVO_NO_LEADING_WSP;
        static constexpr bool no_vectorize      = HTTPVO_NO_VECTORIZE;
        static constexpr bool end_of_line_is_line_feed = true; // TODO
        static constexpr bool optimize_for_most_case  = HTTPVO_OPTIMIZE_FOR_MOST_CASE;
        static constexpr bool priotize_horizontal_tab = false; // TODO

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
#endif // HTTPVO_DEFINITION_HPP