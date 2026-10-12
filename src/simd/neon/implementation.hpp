#ifndef HTTPVO_SIMD_ARM_NEON_HPP
#define HTTPVO_SIMD_ARM_NEON_HPP
#include "../implementation.hpp"

namespace httpvo::simd
{
    template<>
    struct simdv<16>
    {
        static constexpr int bitpos = 4;

        uint8x16_t lo;

        simdv(simdv& v)     : lo{v.lo}{}
        simdv(simdv&& v)    : lo{v.lo}{}
        simdv(uint8x16_t u) : lo{u}{}
        explicit simdv(const void *b)
        {
            lo = vld1q_u8(reinterpret_cast<uint8x16_t *>(b));
        }

        TARGET("neon")
        inline bool is_zero(void)
        {
            assert(0 and "not implemented");
            return 0;
        }

        TARGET("neon")
        inline u64_t to_bitmask(void) const
        {
            const uint8x8_t x = vshrn_n_u16(vreintrepretq_u16_u8(lo), 4);
            return vget_lane_u64(vreintrepretq_u64_u8(x), 0);
        }

        static inline u64_t countzero_bitmask(const mask_t m)
        {
               return bits::tzcnt(m) >> 2;
        }

        template<u8_t a=0, u8_t b=0>
        TARGET("neon")
        inline simdv cmpngt_lt()
        {
            const uint8x8_t x = vsubq_u8(v, vdupq_n_u8(a));
            return vcgtq_u8(w, vdupq_n_u8(b - a));
        }

        TARGET("neon")
        inline simdv shuf_table(const simdv& tab) const
        {
            return vtbl1q_u8(lo, tab.lo);
        }

        TARGET("neon")
        inline simdv operator&(const simdv &v) const
        {
            return vandq_u8(lo, v.lo);
        }

        TARGET("neon")
        inline mask_t operator&(const mask_t m) const
        {
            return to_bitmask() & m;
        }

        TARGET("neon")
        inline simdv operator<<(const int r) const
        {
            return vshlq_n_u8(lo, r);
        }

        TARGET("neon")
        inline simdv operator|(const simdv& v) const
        {
            return vorrq_u8(lo, v.lo);
        }

        TARGET("neon")
        inline simdv operator==(const simdv &v) const
        {
            return vceqq_u8(lo, v.lo);
        }

        TARGET("neon")
        inline simdv operator<=(const simdv &v) const
        {
            return vcleq_u8(lo, v.lo);
        }

        TARGET("neon")
        inline simdv andnot(const simdv &v) const
        {
            // a & ~b
            return vbicq_u8(v.lo, lo);
        }

        TARGET("neon")
        inline simdv setzero(void) const
        {
            return vdupq_n_u8(0);
        }

        TARGET("neon")
        inline operator mask_t(void) const
        {
            return to_bitmask();
        }

        TARGET("neon")
        static inline simdv load_tbl1(const void * const b)
        {
            // load a 16B table
            return simdv(b);
        }

        TARGET("neon")
        static inline simdv andeqz(simdv&& u, simdv&& v)
        {
            return vmvnq_u8(vtstq_u8(u.lo, v.lo));
        }
    };
}
#endif // HTTPVO_SIMD_ARM_NEON_HPP