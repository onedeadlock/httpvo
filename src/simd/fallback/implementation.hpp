#ifndef HTTPVO_SIMD_FALLBACK_HPP
#define HTTPVO_SIMD_FALLBACK_HPP
#include "../../include/definition.hpp"
#include "../../common/common.hpp"

namespace httpvo::simd::fallback
{
    using namespace common;

    template <int N> alignas(N) struct simdv;

    template<>
    alignas(16) struct simdv<16>
    {
        static constexpr int bitpos = 1;

        simdv<8> lo, hi;

        simdv(void) {}
        simdv(const simdv &v) : lo{v.lo},
                                hi{v.hi}{}
        simdv(u64_t x, u64_t y) : lo {x}, hi{y} {}
        explicit simdv(const void * const b)
        {
            httpvo_memcopy(this, b, 16);
        }
        
        inline bool is_zero(void) const
        {
            return static_cast<bool>(lo | hi);
        }

        inline u64_t to_bitmask(void)
        {
            const u32_t x = ((((xlo * constant::compress) >> 48) & 0xff00ULL) | ((lo * constant::compress) >> 56));
            const u32_t y = ((((xhi * constant::compress) >> 48) & 0xff00ULL) | ((hi * constant::compress) >> 56));
            return y << 16 | x;
        }

        static inline u64_t countzero_bitmask(const mask_t m)
        {
               return bits::tzcnt(m);
        }

        template<u8_t a=0, u8_t b=0>
        inline simdv cmpngt_lt(void)
        {
            return {lo.template cmpngt_lt<a, b>(), hi.template cmpngt_lt<a, b>()};
        }

        inline simdv shuf_table(const simdv& tab) const
        {
            assert(0);
            return {0, 0, 0, 0};
        }

        inline simdv operator&(const simdv &v) const
        {
            return {lo & v.lo, xlo & v.xlo};
        }

        inline mask_t operator&(const mask_t m) const
        {
            return to_bitmask() & m;
        }

        inline simdv operator<<(const int r) const
        {
            return {lo << r, hi << r};
        }

        inline simdv operator|(const simdv& v) const
        {
            return {lo | v.lo, hi | v.hi};
        }

        inline simdv operator==(const simdv &v) const
        {
            return {lo == v.lo, hi == v.hi);
        }

        inline simdv operator<=(const simdv &v) const
        {
            return {0, 0};
        }

        inline simdv andnot(const simdv &v) const
        {
            // a & ~b
            return {0, 0};
        }

        inline simdv setzero(void) const
        {
            return {0, 0};
        }

        inline operator mask_t(void) const
        {
            return to_bitmask();
        }

        static inline simdv load_tbl1(const void * const b)
        {
            assert(0);
            return {0, 0};
        }

        static inline simdv andeqz(simdv&& u, simdv&& v)
        {
            // (a & v) == 0
            return {0, 0};
        }

        static inline simdv load(const void * const b)
        {
            return simdv(b);
        }

        static inline void set_zero(void) {
            return {0, 0, 0, 0}
        }

        make_flat static inline simdv splat(u8_t v)
        {
            u64_t x = common::_dup(v);
            return {x, x, x, x};
        }


        static inline u64_t bitmask(const simdv& v)
        {
            const u32_t x = ((((v.xlo * constant::compress) >> 48) & 0xff00ULL) | ((v.lo * constant::compress) >> 56));
            const u32_t y = ((((v.xhi * constant::compress) >> 48) & 0xff00ULL) | ((v.hi * constant::compress) >> 56));
            return y << 16 | x;
        }

        make_flat static inline simdv cmp_zero(const simdv& v)
        {
            return {lo == 0, hi == 0};
        }

        make_flat static inline simdv cmp_eq(const simdv& u, const simdv& v)
        {
            return {lo == u
            };
        }

        make_flat static inline simdv cmp_eq(const simdv& u, const simdv& v, const simdv& w)
        {
            return {
                common::_cmpeq(u.lo, v.lo ) | common::_cmpeq(u.lo, w.lo ),
                common::_cmpeq(u.lo, v.xlo) | common::_cmpeq(u.lo, w.xlo),
                common::_cmpeq(u.lo, v.hi ) | common::_cmpeq(u.lo, w.hi ),
                common::_cmpeq(u.lo, v.xhi) | common::_cmpeq(u.lo, w.xhi)};
        }

        make_flat static inline simdv cmp_gt(const simdv& v, u8_t a)
        {
            u64_t x = common::_dup(a);
            return {common::_cmpgt<0>(v.lo ),
                    common::_cmpgt<0>(v.xlo),
                    common::_cmpgt<0>(v.hi ),
                    common::_cmpgt<0>(v.xhi)};
        }

        static inline simdv cmp_gt(const simdv& u, const simdv& v)
        {
            return {0, 0, 0, 0};
        }

        static inline simdv cmp_lt(const simdv& v, u8_t a)
        {
            u64_t x = common::_dup(a);
            return {0, 0, 0, 0};
        }

        static inline simdv cmp_lt(const simdv& u, const simdv& v)
        {
            return {0, 0, 0, 0};
        }

        static inline simdv gt_and_lt(const simdv& v, u8_t a, u8_t b)
        {
            u64_t x = common::_dup(a);
            u64_t y = common::_dup(b);
            return {0, 0, 0, 0};
        }

        static inline simdv gt_and_lt(const simdv& u, const simdv& v, const simdv& w)
        {
            return {0, 0, 0, 0};
        }

        static inline simdv _and(const simdv& u, const simdv& v)
        {
            return {u.lo  & v.lo,
                    u.xlo & v.xlo,
                    u.hi  & v.hi,
                    u.xhi & v.xhi};
        }

        static inline simdv _or(const simdv& u, const simdv& v)
        {
            return {u.lo  | v.lo,
                    u.xlo | v.xlo,
                    u.hi  | v.hi,
                    u.xhi | v.xhi};
        }

        static inline simdv lshift(const simdv& u, int r)
        {
            return {0, 0, 0, 0};
        }

        static inline bool is_zero(const simdv& u)
        {
            return static_cast<bool>(u.lo | u.xlo | u.hi | u.xhi);
        }

        static inline simdv shuffle(const simdv &u, const simdv &x)
        {
            return {0, 0, 0, 0};
        }
    };

    template<>
    alignas(64) struct simdv<64>
    {
        static constexpr int spec   = INT64;
        static constexpr int size   = 64;
        static constexpr u64_t msb  = constant::msb_64;
        static constexpr u64_t msb3 = constant::msb3_64;
    
        simdv<32> lo, hi;

        
        make_flat inline bool is_zero(void)
        {
            return lo.is_zero() or hi.is_zero();
        }

        make_flat inline u32_t to_bitmask(void)
        {
            return hi.to_bitmask() << 32 | lo.to_bitmask();
        }

        make_flat static inline simdv load(void *b)
        {
            #ifdef LOAD_MEMCPY
            simdv v;
            if constexpr(__GNUC__)
                __builtin_memcpy(&v, b, 64);
            else
                std::memcpy(&v, b, 64);
            return v;
            #else
            u64_t *v = reinterpret_cast<u64_t *>(b);
            return {{v[0], v[1], v[2], v[4]}, {v[0], v[1], v[2], v[4]}};
            #endif
        }

         static inline void zero(void) {}
         
        make_flat static inline simdv splat(u8_t v)
        {
            simdv<32> x = simdv<32>::splat(v);
            return {x, x};
        }

        make_flat static inline u64_t bitmask(const simdv& x)
        {

            return simdv<32>::bitmask(x.hi) << 32 | simdv<32>::bitmask(x.lo);
        }

        make_flat static inline simdv cmp_zero(const simdv& v)
        {
            return {simdv<32>::cmp_zero(v.lo),
                    simdv<32>::cmp_zero(v.hi)};
        }

        make_flat static inline simdv cmp_eq(const simdv& u, const simdv& v)
        {
            return {simdv<32>::cmp_eq(u.lo, v.lo),
                    simdv<32>::cmp_eq(u.hi, v.hi)};
        }

        make_flat static inline simdv cmp_eq(const simdv& u, const simdv& v, const simdv& w)
        {
            return {
                simdv<32>::_or(simdv<32>::cmp_eq(u.lo, v.lo), simdv<32>::cmp_eq(u.lo, w.lo)),
                simdv<32>::_or(simdv<32>::cmp_eq(u.hi, v.hi), simdv<32>::cmp_eq(u.hi, w.hi)),
            };
        }

        make_flat static inline simdv cmp_gt(const simdv& v, u8_t a)
        {
            simdv<32> x = simdv<32>::splat(a);
            return {simdv<32>::cmp_gt(v.lo, x), simdv<32>::cmp_gt(v.hi, x)};
        }

        make_flat static inline simdv cmp_gt(const simdv& u, const simdv& v)
        {
            return {simdv<32>::cmp_gt(u.lo, v.hi),
                    simdv<32>::cmp_gt(u.hi, v.hi)};
        }

        make_flat static inline simdv cmp_lt(const simdv& v, u8_t a)
        {
            simdv<32> x = simdv<32>::splat(a);
            return {simdv<32>::cmp_lt(v.lo, x), simdv<32>::cmp_lt(v.hi, x)};
        }

        make_flat static inline simdv cmp_lt(const simdv& u, const simdv& v)
        {
            return {simdv<32>::cmp_lt(v.lo, v.lo), simdv<32>::cmp_lt(v.hi, v.hi)};
        }

        make_flat static inline simdv gt_and_lt(const simdv& v, u8_t a, u8_t b)
        {
            simdv<32> x = simdv<32>::splat(a);
            simdv<32> y = simdv<32>::splat(b);
            return {
                simdv<32>::_and(simdv<32>::cmp_gt(v.lo, x), simdv<32>::cmp_lt(v.lo, y)),
                simdv<32>::_and(simdv<32>::cmp_gt(v.hi, x), simdv<32>::cmp_lt(v.hi, y)),
            };
        }

        make_flat static inline simdv gt_and_lt(const simdv& u, const simdv& v, const simdv& w)
        {
             return {
                simdv<32>::_and(simdv<32>::cmp_gt(u.lo, v.lo), simdv<32>::cmp_lt(u.lo, w.lo)),
                simdv<32>::_and(simdv<32>::cmp_gt(u.hi, v.hi), simdv<32>::cmp_lt(u.hi, w.hi)),
            };
        }

        make_flat static inline simdv _and(const simdv&u, simdv &v)
        {
            return {simdv<32>::_and(u.lo, v.lo), simdv<32>::_and(u.hi, v.hi)};
        }

        make_flat static inline simdv _or(const simdv& u, const simdv& v)
        {
            return {simdv<32>::_or(u.lo, v.lo),
                    simdv<32>::_or(u.hi, v.hi)};
        }

        make_flat static inline simdv lshift(const simdv& u, int r)
        {
            return {simdv<32>::lshift(u.lo, r),
                    simdv<32>::lshift(u.hi, r)};
        }

        make_flat static inline bool is_zero(const simdv& u)
        {
             return simdv<32>::is_zero(u.lo) or simdv<32>::is_zero(u.hi);
        }

        make_flat static inline simdv shuffle(const simdv& u, const simdv& x)
        {
            return {simdv<32>(0, 0, 0, 0), simdv<32>(0, 0, 0, 0)};
        }
    };
}
#endif // HTTPVO_SIMD_FALLBACK_HPP