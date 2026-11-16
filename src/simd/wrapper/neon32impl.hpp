#ifndef HTTPVO_SIMD_ARM_NEON_HPP
#define HTTPVO_SIMD_ARM_NEON_HPP
#include "simd.hpp"

namespace httpvo::simd
{

    template <>
    struct simdv<8>
    {
        static constexpr int bitpos = 8;
        static constexpr int N      = 8;

        uint8x8_t v;
        simdv(void) = default;
        constexpr simdv(const uint8x8_t x) : v( x ) {}
        constexpr simdv(const simdv&    x) : v(x.v) {}
        explicit  simdv(const void *const b) noexcept
        {
            v = vld1_u64(b);
        }

        static inline int countzero_bitmask(const mask_t m)
        {
            return bits::tzcnt<u64_t>(static_cast<u64_t>(m)) >> 3;
        }

        static inline simdv splat(const int x)
        {
            return vdup_n_u8(x);
        }

        inline mask_t to_bitmask(void) const noexcept
        {
            return vget_lane_u64(vand_u64(vreinterpret_u64_u8(v), vcreate_u64(constant::c80)));
        }

        inline operator mask_t(void) const noexcept
        {
            return to_bitmask();
        }

        inline simdv operator==(const int x) const noexcept
        {
            return vceq_u8(v, vdup_n_u8(x));
        }

        inline simdv operator|(const simdv &x) const noexcept
        {
            return vorr_u8(v, x.v);
        }

        template <int A, int B>
        inline simdv off_range(void) const noexcept
        {
            uint8x8_t x = vsub_u8(v, vdup_n_u8(A));
            return vcgt_u8(x, vdup_n_u8(B - A));
        }

        template <int ALPHAONLY>
        inline simdv fast_not_tchar(void) const noexcept
        {
            const uint8x16_t   ld0  = vld1q_u8(TCHAR_CLASS_LUT + 00);
            const uint8x16_t   ld1  = vld1q_u8(TCHAR_CLASS_LUT + 16);
            const uint8x16x2_t ldlo = {{vget_low_u8(ld0), vget_high_u8(ld0)}};
            const uint8x16x2_t ldhi = {{vget_low_u8(ld1), vget_high_u8(ld1)}};
            uint8x8_t lo  = vand_u8(v, vdup_n_u8(0xf));
            uint8x8_t hi  = vshr_n_u8(v, 4);
            uint8x8_t rlo = vtbl2_u8(ldlo, lo);
            uint8x8_t rhi = vtbl2_u8(ldhi, hi);
            return vcgt_u8(rlo, rhi);
        }

        template <bool INCLUDE_TAB = setup::priotize_horizontal_tab>
        inline simdv is_not_tchar(void) const noexcept
        {
            return fast_not_tchar<0>();
        }

        template <bool INCLUDE_TAB = setup::priotize_horizontal_tab>
        inline simdv is_not_print_char(void) const
        {
            const uint8x8_t not_print = vorr_u8(vclt_u8(v, vcreate_u8(constant::c20)), vceq_u8(v, vcreate_u8(constant::c7f)));
            if HTTPVO_CXX17_CONSTEXPR (INCLUDE_TAB)
                return vorr_u8(not_print, vceq_u8(v, vcreate_u8(constant::c09)));
            else return not_print;
        }
    }; // SIMDV
}
#endif // HTTPVO_SIMD_ARM_NEON_HPP