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
        simdv(simdv&& v)       : lo{v.lo}{}
        simdv(__m128i u)       : lo{u}{}
        explicit simdv(const void *b)
        {
            lo = _mm_loadu_si128(reinterpret_cast<const __m128i *>(b));;
        }

        TARGET("sse4")
        inline bool is_zero(void)
        {
            assert(0 and "not implemented");
            return 0;
        }

        TARGET("sse4")
        inline u64_t to_bitmask(void)
        {
            return _mm_movemask_epi8(lo);
        }

        static inline u64_t countzero_bitmask(const mask_t m)
        {
               return bits::tzcnt(m);
        }

        template<u8_t a, u8_t b>
        TARGET("sse4")
        inline simdv cmpgt_lt(u8_t _aa=0, u8_t _bb=0)
        {
            if constexpr (setup::debug)
                assert(a < 0x80 and b < 0x80);

            __m128i x = _mm_set1_epi8(a);
            __m128i y = _mm_set1_epi8(b);
            return _mm_and_si128(_mm_cmpgt_epi8(lo, x), _mm_cmplt_epi8(lo, y));
        }

        template<u8_t a=0, u8_t b=0>
        TARGET("sse4")
        inline simdv cmpngt_lt(u8_t _aa=0, u8_t _bb=0)
        {
            __m128i x = _mm_add_epi8(lo, _mm_set1_epi8(static_cast<u8_t>(0x80 - a)));
            return _mm_cmpgt_epi8(x, _mm_set1_epi8(static_cast<u8_t>((b - a) - 0x80)));
        }

        TARGET("sse4")
        inline simdv shuf_table(const simdv& tab)
        {
            return _mm_shuffle_epi8(lo, tab.lo);
        }

        TARGET("sse4")
        inline simdv operator&(const simdv &v)
        {
            return _mm_and_si128(lo, v.lo);
        }

        TARGET("sse4")
        inline simdv operator<<(const int r)
        {
            return _mm_srli_si128(lo, r);
        }

        TARGET("sse4")
        inline simdv operator|(const simdv& v)
        {
            return _mm_or_si128(lo, v.lo);
        }

        TARGET("sse4")
        inline simdv operator==(const simdv &v)
        {
            // we use an unsigned compare here
            return _mm_cmpeq_epi8(lo, v.lo);
        }

        TARGET("sse4")
        inline simdv operator<=(const simdv &v)
        {
            // we use an unsigned compare here
            return _mm_cmpeq_epi8(_mm_min_epi8(lo, v.lo), lo);
        }

        TARGET("sse4")
        inline simdv andnot(const simdv &v)
        {
            // a & ~b
            return _mm_andnot_si128(v.lo, lo);
        }

        TARGET("sse4")
        inline simdv setzero(void)
        {
            return _mm_setzero_si128();
        }

        TARGET("sse4")
        inline operator mask_t(void)
        {
            return _mm_movemask_epi8(lo);
        }

        static inline simdv andneqz(const simdv& u, const simdv& v)
        {
            // (u & v) != 0
            assert(0 and "NO IMPLEMENTED");
            return simdv(nullptr);
        }
    };

    template<>
    struct simdv<32>
    {
        static constexpr int bitpos = 1;

        __m128i lo, hi;

        simdv(const simdv& v)  : lo{v.lo}, hi{v.hi}{}
        simdv(simdv&& v)       : lo{v.lo}, hi{v.hi}{}
        simdv(__m128i u, __m128i v) : lo{u}, hi{v}{}
        explicit simdv(const void *b)
        {
            lo = _mm_loadu_si128(reinterpret_cast<const __m128i *>(b));
            hi = _mm_loadu_si128(reinterpret_cast<const __m128i *>(reinterpret_cast<const u8_t *>(b) + 16));
        }

        TARGET("sse4")
        inline bool is_zero(void)
        {
            return is_zero({lo, hi});
        }

        TARGET("sse4")
        inline u64_t to_bitmask(void)
        {
            return static_cast<u32_t>(_mm_movemask_epi8(hi)) << 16 |
                   _mm_movemask_epi8(lo);
        }

         static inline u64_t countzero_bitmask(const mask_t m)
          {
               return bits::tzcnt(m);
          }

        template<u8_t a, u8_t b>
        TARGET("sse4")
        inline simdv cmpgt_lt(u8_t _aa=0, u8_t _bb=0)
        {
            if constexpr (setup::debug)
                assert(a < 0x80 and b < 0x80);

            __m128i x = _mm_set1_epi8(a);
            __m128i y = _mm_set1_epi8(b);
            return {
                _mm_and_si128(_mm_cmpgt_epi8(lo, x), _mm_cmplt_epi8(lo, y)),
                _mm_and_si128(_mm_cmpgt_epi8(hi, x), _mm_cmplt_epi8(hi, y)),
            };
        }

        template<u8_t a=0, u8_t b=0>
        TARGET("sse4")
        inline simdv cmpngt_lt(u8_t _aa=0, u8_t _bb=0)
        {
            if constexpr (setup::debug)
                assert(a < 0x80 and b < 0x80);

            __m128i x = _mm_set1_epi8(a);
            __m128i y = _mm_set1_epi8(b);
            return {
                _mm_or_si128(_mm_cmplt_epi8(lo, x), _mm_cmpgt_epi8(lo, y)),
                _mm_or_si128(_mm_cmplt_epi8(hi, x), _mm_cmpgt_epi8(hi, y)),
            };
        }

        TARGET("sse4")
        inline simdv shuf_table(const simdv& tab)
        {
            return {_mm_shuffle_epi8(lo, tab.lo), _mm_shuffle_epi8(hi, tab.hi)};
        }

        inline simdv operator&(const simdv& v)
        {
            return {_mm_and_si128(lo, v.lo), _mm_and_si128(hi, v.hi)};
        }

        inline simdv operator<<(const int r)
        {
            return {_mm_srli_si128(lo, r), _mm_srli_si128(hi, r)};
        }

        inline simdv operator>(const simdv& v)
        {
            return {_mm_cmpgt_epi8(lo, v.lo), _mm_cmpgt_epi8(hi, v.hi)};
        }

        inline simdv andnot(const simdv& v)
        {
            // a & ~b
            return {_mm_andnot_si128(v.lo, lo), _mm_andnot_si128(v.hi, hi)};
        }

        inline simdv setzero(void)
        {
            return {_mm_setzero_si128(), _mm_setzero_si128()};
        }

        TARGET("sse4")
        static inline simdv load(void *b)
        {
            return {_mm_loadu_si128(reinterpret_cast<__m128i *>(b)),
                    _mm_loadu_si128(reinterpret_cast<__m128i *>(reinterpret_cast<u8_t *>(b) + 16))};
        }

        static inline void zero(void) {}
       
        TARGET("sse4")
        static inline simdv splat(u8_t v)
        {
            return {_mm_set1_epi8(v), _mm_set1_epi8(v)};
        }

        TARGET("sse4")
        static inline u64_t bitmask(const simdv& v)
        {

            return static_cast<u32_t>(_mm_movemask_epi8(v.hi)) << 16 |
                   _mm_movemask_epi8(v.lo);
        }

        TARGET("sse4")
        static inline simdv cmp_zero(const simdv& v)
        {
            static __m128i z = _mm_setzero_si128();
            return {_mm_cmpeq_epi8(v.lo, z),
                    _mm_cmpeq_epi8(v.hi, z)};
        }

        TARGET("sse4")
        static inline simdv cmp_eq(const simdv& u, const simdv& v)
        {
            return {_mm_cmpeq_epi8(u.lo, v.lo),
                    _mm_cmpeq_epi8(u.hi, v.hi)};
        }

        TARGET("sse4")
        static inline simdv cmp_eq(const simdv& u, const simdv& v, const simdv& w)
        {
            return {
                _mm_or_si128(_mm_cmpeq_epi8(u.lo, v.lo), _mm_cmpeq_epi8(u.lo, w.lo)),
                _mm_or_si128(_mm_cmpeq_epi8(u.hi, v.hi), _mm_cmpeq_epi8(u.hi, w.hi)),
            };
        }

        TARGET("sse4")
        static inline simdv cmp_gt(const simdv& v, u8_t a)
        {
            __m128i x = _mm_set1_epi8(a);
            return {_mm_cmpgt_epi8(v.lo, x),
                    _mm_cmpgt_epi8(v.hi, x)};
        }

        TARGET("sse4")
        static inline simdv cmp_gt(const simdv& u, const simdv& v)
        {
            return {_mm_cmpgt_epi8(u.lo, v.hi),
                    _mm_cmpgt_epi8(u.hi, v.hi)};
        }

        TARGET("sse4")
        static inline simdv cmp_lt(const simdv& v, u8_t a)
        {
            __m128i x = _mm_set1_epi8(a);
            return {_mm_cmpgt_epi8(v.lo, x),
                    _mm_cmpgt_epi8(v.hi, x)};
        }

        TARGET("sse4")
        static inline simdv cmp_lt(const simdv& u, const simdv& v)
        {
            return {_mm_cmplt_epi8(u.lo, v.hi),
                    _mm_cmplt_epi8(u.hi, v.hi)};
        }

        TARGET("sse4")
        static inline simdv gt_and_lt(const simdv& u, const simdv& v, const simdv& w)
        {
            return {
                _mm_and_si128(_mm_cmpgt_epi8(u.lo, v.lo), _mm_cmplt_epi8(u.lo, w.lo)),
                _mm_and_si128(_mm_cmpgt_epi8(u.lo, v.hi), _mm_cmplt_epi8(u.lo, w.hi)),
            };
        }

        TARGET("sse4")
        static inline simdv _and(const simdv& u, const simdv& v)
        {
            return {_mm_and_si128(u.lo, v.lo),
                    _mm_and_si128(u.hi, v.hi)};
        }

        TARGET("sse4")
        static inline simdv _or(const simdv& u, const simdv& v)
        {
            return {_mm_or_si128(u.lo, v.lo),
                    _mm_or_si128(u.hi, v.hi)};
        }

        TARGET("sse4")
        static inline simdv lshift(const simdv& u, int r)
        {
            return {_mm_srli_si128(u.lo, r),
                    _mm_srli_si128(u.hi, r)};
        }

        TARGET("sse4")
        static inline bool is_zero(const simdv& u)
        {
            return _mm_test_all_zeros(u.lo, u.lo) or _mm_test_all_zeros(u.hi, u.hi);
        }

        static inline simdv shuffle(const simdv& u, const simdv& x)
        {
            return {_mm_shuffle_epi8(u.lo, x.lo),
                    _mm_shuffle_epi8(u.hi, x.hi)};
        }
    };

#    ifndef HTTPVO_NO_64B
    template<>
    struct simdv<64>
    {
        simdv<32> lo, hi;

        TARGET("sse4")
        inline bool is_zero(void)
        {
            return lo.is_zero() or hi.is_zero();
        }
        
        TARGET("sse4")
        make_flat static inline simdv load(void *b)
        {
            return {simdv<32>::load(reinterpret_cast<__m128i *>(b)),
                    simdv<32>::load(reinterpret_cast<__m128i *>(reinterpret_cast<u8_t *>(b) + 32))};
        }

        TARGET("sse4")
        make_flat static inline simdv splat(u8_t v)
        {
            return {simdv<32>::splat(v), simdv<32>::splat(v)};
        }

         TARGET("sse4")
        make_flat inline u64_t to_bitmask(void)
        {

            return hi.to_bitmask() << 32 | lo.to_bitmask();
        }

        TARGET("sse4")
        make_flat static inline u64_t bitmask(const simdv& v)
        {

            return simdv<32>::bitmask(v.hi) << 32 | simdv<32>::bitmask(v.lo);
        }

        TARGET("sse4")
        make_flat static inline simdv cmp_zero(const simdv& v)
        {
            return {simdv<32>::cmp_zero(v.lo),
                    simdv<32>::cmp_zero(v.hi)};
        }

        TARGET("sse4")
        make_flat static inline simdv cmp_eq(const simdv& u, const simdv& v)
        {
            return {simdv<32>::cmp_eq(u.lo, v.lo),
                    simdv<32>::cmp_eq(u.hi, v.hi)};
        }

        TARGET("sse4")
        make_flat static inline simdv cmp_eq(const simdv& u, const simdv& v, const simdv& w)
        {
            return {
                simdv<32>::_or(simdv<32>::cmp_eq(u.lo, v.lo), simdv<32>::cmp_eq(u.lo, w.lo)),
                simdv<32>::_or(simdv<32>::cmp_eq(u.hi, v.hi), simdv<32>::cmp_eq(u.hi, w.hi)),
            };
        }

        TARGET("sse4")
        make_flat static inline simdv cmp_gt(const simdv& v, u8_t a)
        {
            simdv<32> x = simdv<32>::splat(a);
            return {simdv<32>::cmp_eq(v.lo, x), simdv<32>::cmp_eq(v.hi, x)};
        }

        TARGET("sse4")
        make_flat static inline simdv cmp_gt(const simdv& u, const simdv& v)
        {
            return {simdv<32>::cmp_gt(u.lo, v.hi),
                    simdv<32>::cmp_gt(u.hi, v.hi)};
        }

        TARGET("sse4")
        make_flat static inline simdv cmp_lt(const simdv& v, u8_t a)
        {
            simdv<32> x = simdv<32>::splat(a);
            return {simdv<32>::cmp_lt(v.lo, x), simdv<32>::cmp_lt(v.hi, x)};
        }

        TARGET("sse4")
        make_flat static inline simdv cmp_lt(const simdv& u, const simdv& v)
        {
            return {simdv<32>::cmp_lt(v.lo, v.lo), simdv<32>::cmp_lt(v.hi, v.hi)};
        }

        TARGET("sse4")
        make_flat static inline simdv gt_and_lt(const simdv& v, u8_t a, u8_t b)
        {
            simdv<32> x = simdv<32>::splat(a);
            simdv<32> y = simdv<32>::splat(b);
            return {
                simdv<32>::_and(simdv<32>::cmp_gt(v.lo, x), simdv<32>::cmp_lt(v.lo, y)),
                simdv<32>::_and(simdv<32>::cmp_gt(v.hi, x), simdv<32>::cmp_lt(v.hi, y)),
            };
        }

        TARGET("sse4")
        make_flat static inline simdv gt_and_lt(const simdv& u, const simdv& v, const simdv& w)
        {
             return {
                simdv<32>::_and(simdv<32>::cmp_gt(u.lo, v.lo), simdv<32>::cmp_lt(u.lo, w.lo)),
                simdv<32>::_and(simdv<32>::cmp_gt(u.hi, v.hi), simdv<32>::cmp_lt(u.hi, w.hi)),
            };
        }

        TARGET("sse4")
        make_flat static inline simdv _and(const simdv&u, simdv &v)
        {
            return {simdv<32>::_and(u.lo, v.lo), simdv<32>::_and(u.hi, v.hi)};
        }

        TARGET("sse4")
        make_flat static inline simdv _or(const simdv& u, const simdv& v)
        {
            return {simdv<32>::_or(u.lo, v.lo),
                    simdv<32>::_or(u.hi, v.hi)};
        }

        TARGET("sse4")
        make_flat static inline simdv lshift(const simdv& u, int r)
        {
            return {simdv<32>::lshift(u.lo, r),
                    simdv<32>::lshift(u.hi, r)};
        }

        TARGET("sse4")
        make_flat static inline bool is_zero(const simdv& u)
        {
             return simdv<32>::is_zero(u.lo) or simdv<32>::is_zero(u.hi);
        }


        make_flat static inline simdv shuffle(const simdv& u, const simdv& v)
        {
            return {simdv<32>::shuffle(u.lo, v.lo),
                    simdv<32>::shuffle(u.hi, v.hi)};
        }
    };
#   endif // HTTPVO_NO_64B
}
#endif // HTTPVO_SIMD_WESTMERE_HPP