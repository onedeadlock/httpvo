#ifndef HTTPVO_SIMD_HASWELL_HPP
#define HTTPVO_SIMD_HASWELL_HPP
#include "simd.hpp"

namespace httpvo::simd
{
    template<>
    struct simdv<32>
    {
        static constexpr int bitpos = 1;
        static constexpr int N = 32;

        __m256i v;

        simdv(void) = default;
        simdv(__m256i u)      : v{u}{}
        simdv(const simdv& x) : v{x.v}{}
        explicit simdv(const void *b) noexcept
        {
            v = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(b));
        }

        inline mask_t to_bitmask(void) const noexcept
        {
            return _mm256_movemask_epi8(v);
        }

        inline simdv operator|(const simdv& x) const noexcept
        {
            return _mm256_or_si256(v, x.v);
        }

        inline simdv operator==(const int x) const noexcept
        {
            return _mm256_cmpeq_epi8(v, _mm256_set1_epi8(x));
        }

        inline operator mask_t(void) const noexcept
        {
            return _mm256_movemask_epi8(v);
        }

        template<int A=0, int B=0>
        inline simdv off_range(void) const noexcept
        {
            constexpr int s = static_cast<int>(0x80 - A);
            constexpr int b = static_cast<int>((B - A) - 0x80);
            __m256i x = _mm256_add_epi8(v, _mm256_set1_epi8(s));
            return _mm256_cmpgt_epi8(x, _mm256_set1_epi8(b));
        }

        template <int ALPHAONLY>
        inline simdv fast_not_tchar(void) const noexcept
        {
            return _mm256_setzero_si256();
        }

        inline simdv is_not_tchar(void) const noexcept
        {
            __m256i ld0    = _mm256_broadcastsi128_si256(_mm_load_si128(reinterpret_cast<const __m128i *>(TCHAR_CLASS_LUT + 00)));
            __m256i ld1    = _mm256_broadcastsi128_si256(_mm_load_si128(reinterpret_cast<const __m128i *>(TCHAR_CLASS_LUT + 16)));
            __m256i hifb   = _mm256_and_si256(_mm256_srli_epi64(v, 4), _mm256_set1_epi8(0xf)); // high four bits
            return _mm256_cmpgt_epi8(_mm256_shuffle_epi8(ld0, v), _mm256_shuffle_epi8(ld1, hifb));
        }

        template <bool INCLUDE_TAB = setup::priotize_horizontal_tab>
        inline simdv is_not_print_char(void) const noexcept
        {
            __m256i ld0    = _mm256_broadcastsi128_si256(_mm_load_si128(reinterpret_cast<const __m128i *>(CONTROL_CHAR_CLASS_LUT + 00)));
            __m256i ld1    = _mm256_broadcastsi128_si256(_mm_load_si128(reinterpret_cast<const __m128i *>(CONTROL_CHAR_CLASS_LUT + 16)));
            __m256i hifb   = _mm256_and_si256(_mm256_srli_epi64(v, 4), _mm256_set1_epi8(0xf)); // high four bits
            __m256i cls    = _mm256_and_si256(_mm256_shuffle_epi8(ld0, v), _mm256_shuffle_epi8(ld1, hifb));
            return _mm256_cmpeq_epi8(cls, _mm256_setzero_si256());
        }

        static inline int countzero_bitmask(const mask_t m)
        {
               return bits::tzcnt<u32_t>(static_cast<const u32_t>(m));
        }
    };
}
#endif // HTTPVO_SIMD_WESTMERE_HPP