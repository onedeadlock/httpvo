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
        static constexpr int bitpos = 1;
        static constexpr int N      = 16;

        uint8x8_t v;

        simdv(void) = default;
        simdv(uint8x8_t u)    : v{u}{}
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
            const uint8x8_t x = vsubq_u8(v, vdupq_n_u8(A));
            return vcgtq_u8(w, vdupq_n_u8(B - A));
        }

        template <int ALPHAONLY>
        inline simdv fast_not_tchar(void) const noexcept
        {
            return vdupq_n_u8(0);
        }

        inline simdv is_not_tchar(void) const noexcept
        {
            uint8x8_t ld0 = vqld1q_u8(reinterpret_cast<char *>(TCHAR_CLASS_LUT + 00));
            uint8x8_t ld1 = vqld1q_u8(reinterpret_cast<char *>(TCHAR_CLASS_LUT + 16));
            uint8x8_t lo  = vandq_u8(v, vdupq_n_u8(v, 0xf));
            uint8x8_t hi  = vshlq_n_u8(v, 4);
            return vcgtq_u8(lo, hi);
        }

        template <bool INCLUDE_TAB = setup::priotize_horizontal_tab>
        inline simdv is_not_print_char(void) const noexcept
        {
            if HTTPVO_CXX17_CONSTEXPR (setup::arm32)
            {
                // TODO: No vqld1q, do range comparison
            }
            uint8x8_t ld0 = vqld1q_u8(reinterpret_cast<char *>(CONTROL_CHAR_CLASS_LUT + 00));
            uint8x8_t ld1 = vqld1q_u8(reinterpret_cast<char *>(CONTROL_CHAR_CLASS_LUT + 16));
            uint8x8_t lo  = vandq_u8(v, vdupq_n_u8(v, 0xf));
            uint8x8_t hi  = vshlq_n_u8(v, 4);
            return vmvnq_u8(vtstq_u8(lo, hi));
        }

        static inline int countzero_bitmask(const mask_t m)
        {
               return bits::tzcnt<u32_t>(static_cast<const u32_t>(m));
        }
    };

    template <>
    struct simdv<16>
    {
        static constexpr int bitpos = 4;
        static constexpr int N      = 16;

        uint8x16_t v;

        simdv(void) = default;
        simdv(const uint8x16_t x) : v{ x } {}
        simdv(const simdv&     x) : v{x.v} {}
        explicit simdv(const void *b) noexcept
        {
            v = vld1q_u8(b);
        }

        static inline const simdv& load_tbl1(const void *const b)
        {
            // laad a 16B table
            return simdv(b);
        }

        static inline int countzero_bitmask(const mask_t m)
        {
            return bits::tzcnt<u64_t>(static_cast<u64_t>(m)) >> 2;
        }

        inline u64_t to_bitmask(void) const noexcept
        {
            const uint8x8_t x = vshrn_n_u16(vreintrepretq_u16_u8(v), 4);
            return vget_lane_u64(vreintrepretq_u64_u8(x), 0);
        }

        template <u8_t A = 0, u8_t B = 0>
        inline simdv cmpngt_lt(void) const noexcept
        {
            const uint8x8_t x = vsubq_u8(v, vdupq_n_u8(A));
            return vcgtq_u8(w, vdupq_n_u8(B - A));
        }

        inline simdv shuf_table(const simdv &tab) const noexcept
        {
            return vqtbl1q_u8(v, tab.v);
        }

        inline simdv operator&(const int x) const noexcept
        {
            return vandq_u8(v, vdupq_n_u8(x));
        }

        inline simdv operator<<(const int r) const noexcept
        {
            return vshlq_n_u8(v, r);
        }

        inline simdv operator|(const simdv &x) const noexcept
        {
            return vorrq_u8(v, x.v);
        }

        inline simdv operator^(const simdv &x) const noexcept
        {
            return veorq_u8(v, x.v);
        }

        inline simdv operator==(const int x) const noexcept
        {
            return vceqq_u8(v, vdupq_n_u8(x));
        }

        inline simdv operator<=(const int x) const noexcept
        {
            return vcleq_u8(v, vdupq_n_u8(x));
        }

        inline simdv operator>(const simdv &x) const noexcept
        {
            return vcgtq_u8(v, x.v);
        }

        inline simdv andnot(const simdv &x) const noexcept
        {
            // a & ~b
            return vbicq_u8(x.v, v);
        }

        inline simdv andeqz(const simdv &x) const noexcept
        {
            return vmvnq_u8(vtstq_u8(v, x.v));
        }

        inline operator mask_t(void) const noexcept
        {
            return to_bitmask();
        }
    };
}
#endif // HTTPVO_SIMD_ARM_NEON_HPP