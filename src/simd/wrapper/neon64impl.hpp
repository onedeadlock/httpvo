#ifndef HTTPVO_SIMD_ARM_NEON_HPP
#define HTTPVO_SIMD_ARM_NEON_HPP
#include "simd.hpp"

namespace httpvo::simd
{
    namespace httpvo::simd
{
    template<>
    struct simdv<16>
    {
        struct arm64_tag{};
        struct arm32_tag{};
        static constexpr int bitpos = 1;
        static constexpr int N      = 16;

        uint8x16_t v;

        simdv(void) = default;
        simdv(uint8x16_t   x) : v{ x }{}
        simdv(const simdv& x) : v{x.v}{}
        explicit simdv(const void *b) noexcept
        {
            v = vld1q_u8(b);
        }

        inline u64_t to_bitmask(void) const noexcept
        {
            const uint8x8_t x = vshrn_n_u16(vreintrepretq_u16_u8(v), 4);
            return vget_lane_u64(vreintrepretq_u64_u8(x), 0);
        }

        inline simdv operator|(const simdv& x) const noexcept
        {
            return vorrq_u8(v, x.v);
        }

        inline simdv operator==(const int x) const noexcept
        {
            return vceqq_u8(v, vdupq_n_u8(x));
        }

        inline operator mask_t(void) const noexcept
        {
            return to_bitmask(v);
        }

        template <u8_t A = 0, u8_t B = 0>
        inline simdv off_range(void) const noexcept
        {
            const uint8x16_t x = vsubq_u8(v, vdupq_n_u8(A));
            return vcgtq_u8(w, vdupq_n_u8(B - A));
        }

        template <int ALPHAONLY>
        inline simdv fast_not_tchar(void) const noexcept
        {
            return vdupq_n_u8(0);
        }

        inline simdv _is_not_tchar_arm32(arm32_tag) const noexcept
        {
            // TODO: check assembly, if the table load is hoisted out
            const uint8x16_t  ld0  = vld1q_u8(TCHAR_CLASS_LUT + 00);
            const uint8x16_t  ld1  = vld1q_u8(TCHAR_CLASS_LUT + 16);
            const uint8x8x2_t ldlo = {{vget_low_u8(ld0), vget_high_u8(ld0)}};
            const uint8x8x2_t ldhi = {{vget_low_u8(ld1), vget_high_u8(ld1)}};
            uint8x16_t lo  = vandq_u8(v, vdupq_n_u8(0xf));
            uint8x16_t hi  = vshrq_n_u8(v, 4);
            uint8x8_t  r0  = vtbl2_u8(ldlo, vget_low_u8(lo));
            uint8x8_t  r1  = vtbl2_u8(ldlo, vget_high_u8(lo));
            uint8x8_t  r3  = vtbl2_u8(ldhi, vget_low_u8(hi));
            uint8x8_t  r4  = vtbl2_u8(ldhi, vget_high_u8(hi));
            uint8x16_t rlo = vcombine_u8(r0, r1);
            uint8x16_t rhi = vcombine_u8(r3, r4);
            return vcgtq_u8(rlo, rhi);
        }

        inline simdv _is_not_tchar(arm64_tag) const noexcept
        {
            uint8x16_t ld0 = vld1q_u8(TCHAR_CLASS_LUT + 00);
            uint8x16_t ld1 = vld1q_u8(TCHAR_CLASS_LUT + 16);
            uint8x16_t lo  = vandq_u8(v, vdupq_n_u8(0xf));
            uint8x16_t hi  = vshlq_n_u8(v, 4);
            return vcgtq_u8(vqtbl1_u8(ld0, lo), vqtbl1_u8(ld1, hi));
        }
        
        inline simdv is_not_tchar(void) const noexcept
        {
            return _is_not_tchar(typename std::conditional<setup::arm64, arm64_type, arm32_type>::type{});
        }

        template <bool INCLUDE_TAB = setup::priotize_horizontal_tab>
        inline simdv _is_not_print_char(arm32_tag) const noexcept
        {
            const uint8x16_t not_print = vorrq_u8(vcltq_u8(v, vdupq_n_u8(0x20)), vceqq_u8(v, vdupq_n_u8(0x7f)));
            if HTTPVO_CXX17_CONSTEXPR (INCLUDE_TAB)
                return vorrq_u8(not_print, vceqq_u8(v, vdupq_n_u8(0x09)));
            else return not_print;
        }

        template <bool INCLUDE_TAB = setup::priotize_horizontal_tab>
        inline simdv _is_not_print_char(arm64_tag) const noexcept
        {
            uint8x16_t ld0 = vqld1q_u8(CONTROL_CHAR_CLASS_LUT + 00);
            uint8x16_t ld1 = vqld1q_u8(CONTROL_CHAR_CLASS_LUT + 16);
            uint8x16_t lo  = vandq_u8(v, vdupq_n_u8(0xf));
            uint8x16_t hi  = vshrq_n_u8(v, 4);
            return vmvnq_u8(vtstq_u8(lo, hi));
        }
  
        template <bool INCLUDE_TAB = setup::priotize_horizontal_tab>
        inline simdv is_not_print_char(void) const noexcept
        {
            return is_not_print_char<INCLUDE_TAB>(typename std::conditional<setup::arm64, arm64_tag, arm32_tag>::type{});
        }

        static inline int countzero_bitmask(const mask_t m)
        {
            return bits::countzero<u64_t>(static_cast<const u64_t>(m));
        }
    };
}
#endif // HTTPVO_SIMD_ARM_NEON_HPP