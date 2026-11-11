#ifndef HTTPVO_SIMD_WESTMERE_HPP
#define HTTPVO_SIMD_WESTMERE_HPP
#include "simd.hpp"

namespace httpvo::simd
{
    template<>
    struct simdv<16>
    {
        static constexpr int bitpos = 1;
        static constexpr int N = 16;

        __m128i v;

        simdv(void) = default;
        simdv(const __m128i x) : v{ x }{}
        simdv(const simdv&  x) : v{x.v}{}
        explicit simdv(const void *b)
        {
            v = _mm_loadu_si128(reinterpret_cast<const __m128i *>(b));
        }

        TARGET("sse4")
        inline mask_t to_bitmask(void) const
        {
            return _mm_movemask_epi8(v);
        }

        template<int a=0, int b=0>
        TARGET("sse4")
        inline simdv cmpngt_lt(void) const
        {
            __m128i x = _mm_add_epi8(v, _mm_set1_epi8(static_cast<int>(0x80 - a)));
            return _mm_cmpgt_epi8(x, _mm_set1_epi8(static_cast<int>((b - a) - 0x80)));
        }

        TARGET("sse4")
        inline simdv cmpngt_lt_man(const simdv a, const simdv b)
        {
            return _mm_cmpgt_epi8(_mm_add_epi8(v, a.v), b.v);
        }

        TARGET("sse4")
        inline simdv shuf_table(const simdv& tab) const
        {
            return _mm_shuffle_epi8(tab.v, v);
        }

        TARGET("sse4")
        inline simdv operator&(const int x) const
        {
            return _mm_and_si128(v, _mm_set1_epi8(x));
        }

        TARGET("sse4")
        inline simdv operator>>(const int r) const
        {
            return _mm_srli_epi64(v, r);
        }

        TARGET("sse4")
        inline simdv operator|(const int x) const
        {
            return _mm_or_si128(v, _mm_set1_epi8(x));
        }

        TARGET("sse4")
        inline simdv operator|(const simdv& x) const
        {
            return _mm_or_si128(v, x.v);
        }

        TARGET("sse4")
        inline simdv operator^(const simdv& x) const
        {
            return _mm_xor_si128(v, x.v);
        }

        TARGET("sse4")
        inline simdv operator^(const int x) const
        {
            return _mm_xor_si128(v, _mm_set1_epi8(x));
        }

        TARGET("sse4")
        inline simdv operator==(const int x) const
        {
            return _mm_cmpeq_epi8(v, _mm_set1_epi8(x));
        }

        TARGET("sse4")
        inline simdv operator<=(const int x) const
        {
            return _mm_cmpeq_epi8(_mm_min_epi8(v, _mm_set1_epi8(x)), v);
        }

        TARGET("sse4")
        inline simdv operator>(const simdv& x) const
        {
            return _mm_cmpgt_epi8(v, x.v);
        }

        TARGET("sse4")
        inline simdv andeqz(const simdv& x) const
        {
            return _mm_cmpeq_epi8(_mm_and_si128(v, x.v), _mm_setzero_si128());
        }

        TARGET("sse4")
        inline simdv andnot(const simdv& x) const
        {
            // a & ~b
            return _mm_andnot_si128(x.v, v);
        }

        TARGET("sse4")
        inline operator mask_t(void) const
        {
            return _mm_movemask_epi8(v);
        }

        static inline u64_t countzero_bitmask(const mask_t m)
        {
               return bits::tzcnt<u32_t>(static_cast<const u32_t>(m));
        }

        TARGET("sse4")
        static inline simdv splat(const int c)
        {
            return _mm_set1_epi8(c);
        }

        TARGET("sse4")
        static inline simdv load_tbl1(const void * const b)
        {
            // vad a 16B table
            return _mm_loadu_si128(reinterpret_cast<const __m128i *>(b));
        }
    };
}
#endif // HTTPVO_SIMD_WESTMERE_HPP