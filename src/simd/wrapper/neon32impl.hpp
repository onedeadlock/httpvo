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
        inline simdv cmpngt_lt(void) const noexcept
        {
            uint8x8_t x = vsub_u8(v, vdup_n_u8(A));
            return vcgt_u8(x, vdup_n_u8(B - A));
        }

        template <int ALPHAONLY>
        inline simdv fast_not_tchar(void) const noexcept
        {
            // TODO use TABLE
            uint8x8_t x = vsub_u8(v, vdup_n_u8(0x41)); // shift v so that v < 26 for any alphabet char
            uint8x8_t is_not_letter = vcgt_u8(x, vdup_n_u8(0x19));
            if HTTPVO_CXX17_CONSTEXPR (ALPHAONLY)
                return is_not_letter;
            else
            {
                uint8x8_t is_not_number = vcgt_u8(veor_u8(v, vcreate_u8(constant::c30)), vdup_n_u8(0x9));
                uint8x8_t is_not_hyphen = vmvn_u8(vceq_u8(v, vdup_n_u8(0x2d)));
                return vand_u8(vand_u8(is_not_letter, is_not_number), is_not_hyphen);
            }
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