#ifndef HTTPVO_SIMD_WESTMERE_HPP
#define HTTPVO_SIMD_WESTMERE_HPP
#include "simd.hpp"

namespace httpvo::simd
{
    template<>
    struct simdv<16>
    {
        static constexpr int bitpos = 1;
        static constexpr int N      = 16;

        __m128i v;

        simdv(void) = default;
        simdv(__m128i u)      : v{u}{}
        simdv(const simdv& x) : v{x.v}{}
        explicit simdv(const void *b) noexcept
        {
            v = _mm_loadu_si128(reinterpret_cast<const __m128i *>(b));
        }

        inline mask_t to_bitmask(void) const noexcept
        {
            return _mm_movemask_epi8(v);
        }

        inline simdv operator|(const simdv& x) const noexcept
        {
            return _mm_or_si128(v, x.v);
        }

        inline simdv operator==(const int x) const noexcept
        {
            return _mm_cmpeq_epi8(v, _mm_set1_epi8(x));
        }

        inline operator mask_t(void) const noexcept
        {
            return _mm_movemask_epi8(v);
        }

        template<int A=0, int B=0>
        inline simdv off_range(void) const noexcept
        {
            constexpr int s = static_cast<int>(0x80 - A);
            constexpr int b = static_cast<int>((B - A) - 0x80);
            __m128i x = _mm_add_epi8(v, _mm_set1_epi8(s));
            return _mm_cmpgt_epi8(x, _mm_set1_epi8(b));
        }

        template <int ALPHAONLY>
        inline simdv fast_not_tchar(void) const noexcept
        {
            return _mm_setzero_si128();
        }

        inline simdv is_not_tchar(void) const noexcept
        {
            __m128i ld0    = _mm_load_si128(reinterpret_cast<const __m128i *>(TCHAR_CLASS_LUT + 00));
            __m128i ld1    = _mm_load_si128(reinterpret_cast<const __m128i *>(TCHAR_CLASS_LUT + 16));
            __m128i hifb   = _mm_and_si128(_mm_srli_epi64(v, 4), _mm_set1_epi8(0xf)); // high four bits
            return _mm_cmpgt_epi8(_mm_shuffle_epi8(ld0, v), _mm_shuffle_epi8(ld1, hifb));
        }

        template <bool INCLUDE_TAB = setup::priotize_horizontal_tab>
        inline simdv is_not_print_char(void) const noexcept
        {
            __m128i ld0    = _mm_load_si128(reinterpret_cast<const __m128i *>(CONTROL_CHAR_CLASS_LUT + 00));
            __m128i ld1    = _mm_load_si128(reinterpret_cast<const __m128i *>(CONTROL_CHAR_CLASS_LUT + 16));
            __m128i hifb   = _mm_and_si128(_mm_srli_epi64(v, 4), _mm_set1_epi8(0xf)); // high four bits
            __m128i cls    = _mm_and_si128(_mm_shuffle_epi8(ld0, v), _mm_shuffle_epi8(ld1, hifb));
            return _mm_cmpeq_epi8(cls, _mm_setzero_si128());
        }

        static inline int countzero_bitmask(const mask_t m)
        {
               return bits::countzero<u32_t>(static_cast<const u32_t>(m));
        }
    };
}
#endif // HTTPVO_SIMD_WESTMERE_HPP