#ifndef HTTPVO_SIMD_HASWELL_HPP
#define HTTPVO_SIMD_HASWELL_HPP
#include "../implementation.hpp"

namespace httpvo::simd
{
    template<>
    struct simdv<32>
    {
        static constexpr int bitpos = 1;

        __m256i lo;

        simdv(const simdv& v)  : lo{v.lo}{}
        simdv(simdv&& v)       : lo{v.lo}{}
        simdv(__m256i u)       : lo{u}{}
        explicit simdv(const void *b)
        {
            lo = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(b));;
        }

        TARGET("avx2")
        inline bool is_zero(void)
        {
            return _mm256_testz_si256(lo, lo);
        }

        TARGET("avx2")
        inline u64_t to_bitmask(void) const
        {
            return _mm256_movemask_epi8(lo);
        }

        static inline u64_t countzero_bitmask(const mask_t m)
        {
               return bits::tzcnt(m);
        }

        template<u8_t a=0, u8_t b=0>
        TARGET("avx2")
        inline simdv cmpngt_lt(void)
        {
            __m256i x = _mm256_add_epi8(lo, _mm256_set1_epi8(static_cast<u8_t>(0x80 - a)));
            return _mm256_cmpgt_epi8(x, _mm256_set1_epi8(static_cast<u8_t>((b - a) - 0x80)));
        }

        TARGET("avx2")
        inline simdv shuf_table(const simdv& tab) const
        {
            return _mm256_shuffle_epi8(tab.lo, lo);
        }

        TARGET("avx2")
        inline simdv operator&(const int x) const
        {
            return _mm256_and_si256(lo, _mm256_set1_epi8(x));
        }

        TARGET("avx2")
        inline simdv operator>>(const int r) const
        {
            return _mm256_srli_epi64(lo, r);
        }

        TARGET("avx2")
        inline simdv operator|(const int x) const
        {
            return _mm256_or_si256(lo, _mm256_set1_epi8(x));
        }

        TARGET("avx2")
        inline simdv operator^(const int x) const
        {
            return _mm256_xor_si256(lo, _mm256_set1_epi8(x));
        }

        TARGET("avx2")
        inline simdv operator==(const int x) const
        {
            return _mm256_cmpeq_epi8(lo, _mm256_set1_epi8(x));
        }

        TARGET("avx2")
        inline simdv operator<=(const int x) const
        {
            return _mm256_cmpeq_epi8(_mm256_min_epi8(lo, _mm256_set1_epi8(x)), lo);
        }

        TARGET("avx2")
        inline simdv operator>(const simdv &v) const
        {
            return _mm256_cmpgt_epi8(lo, v.lo);
        }

        TARGET("avx2")
        inline simdv andeqz(const simdv& v)
        {
            return _mm256_cmpeq_epi8(_mm256_and_si256(lo, v.lo), _mm256_setzero_si256());
        }

        TARGET("avx2")
        inline simdv andnot(const simdv &v) const
        {
            // a & ~b
            return _mm256_andnot_si256(v.lo, lo);
        }

        TARGET("avx2")
        inline simdv setzero(void) const
        {
            return _mm256_setzero_si256();
        }

        TARGET("avx2")
        inline operator mask_t(void) const
        {
            return _mm256_movemask_epi8(lo);
        }

        TARGET("avx2")
        static inline simdv load_tbl1(const void * const b)
        {
            // load a 16B table
            if (setup::debug)
                assert(!(std::uintptr_t(b) & (16 - 1))); // must have an alignment of 16
            return _mm256_broadcastsi128_si256(_mm_load_si128(reinterpret_cast<const __m128i *>(b)));
        }
    };
}
#endif // HTTPVO_SIMD_WESTMERE_HPP