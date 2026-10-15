#ifndef HTTPVO_SIMD_WESTMERE_HPP
#define HTTPVO_SIMD_WESTMERE_HPP
#include "../implementation.hpp"

namespace httpvo::simd
{
    template<>
    struct simdv<16>
    {
        static constexpr int bitpos = 1;

        __m128i lo;

        simdv(const simdv& v)  : lo{v.lo}{}
        simdv(__m128i u)       : lo{u}{}
        explicit simdv(const void *b)
        {
            lo = _mm_loadu_si128(reinterpret_cast<const __m128i *>(b));
        }

        TARGET("sse4")
        inline u64_t to_bitmask(void) const
        {
            return _mm_movemask_epi8(lo);
        }

        static inline u64_t countzero_bitmask(const mask_t m)
        {
               return bits::tzcnt(m);
        }

        template<u8_t a=0, u8_t b=0>
        TARGET("sse4")
        inline simdv cmpngt_lt()
        {
            __m128i x = _mm_add_epi8(lo, _mm_set1_epi8(static_cast<u8_t>(0x80 - a)));
            return _mm_cmpgt_epi8(x, _mm_set1_epi8(static_cast<u8_t>((b - a) - 0x80)));
        }

        TARGET("sse4")
        inline simdv shuf_table(const simdv& tab) const
        {
            return _mm_shuffle_epi8(tab.lo, lo);
        }

        TARGET("sse4")
        inline simdv operator&(const simdv &v) const
        {
            return _mm_and_si128(lo, v.lo);
        }

        TARGET("sse4")
        inline mask_t operator&(const mask_t m) const
        {
            return to_bitmask() & m;
        }

        TARGET("sse4")
        inline simdv operator>>(const int r) const
        {
            return _mm_srli_epi64(lo, r);
        }

        TARGET("sse4")
        inline simdv operator|(const simdv& v) const
        {
            return _mm_or_si128(lo, v.lo);
        }

        TARGET("sse4")
        inline simdv operator==(const simdv &v) const
        {
            return _mm_cmpeq_epi8(lo, v.lo);
        }

        TARGET("sse4")
        inline simdv operator<=(const simdv &v) const
        {
            return _mm_cmpeq_epi8(_mm_min_epi8(lo, v.lo), lo);
        }

        TARGET("sse4")
        inline simdv andeqz(const simdv& v)
        {
            return _mm_cmpeq_epi8(_mm_and_si128(lo, v.lo), _mm_setzero_si128());
        }

        TARGET("sse4")
        inline simdv andnot(const simdv &v) const
        {
            // a & ~b
            return _mm_andnot_si128(v.lo, lo);
        }

        TARGET("sse4")
        inline simdv setzero(void) const
        {
            return _mm_setzero_si128();
        }

        TARGET("sse4")
        inline operator mask_t(void) const
        {
            return _mm_movemask_epi8(lo);
        }

        TARGET("sse4")
        static inline simdv splat(const int c)
        {
            return _mm_set1_epi8((char)c);
        }

        TARGET("sse4")
        static inline simdv load_tbl1(const void * const b)
        {
            // load a 16B table
            return _mm_loadu_si128(reinterpret_cast<const __m128i *>(b));
        }
    };
}
#endif // HTTPVO_SIMD_WESTMERE_HPP