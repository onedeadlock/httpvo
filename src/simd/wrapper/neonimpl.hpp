#ifndef HTTPVO_SIMD_ARM_NEON_HPP
#define HTTPVO_SIMD_ARM_NEON_HPP
#include "simd.hpp"

namespace httpvo::simd
{

    template <>
    struct simdv<8>
    {
        static constexpr int bitpos = 8;
        static constexpr int N = 8;

        uint8x8_t v;
        simdv(void) = default;
        constexpr simdv(const uint8x8_t x) : v( x ) {}
        constexpr simdv(const simdv&    x) : v(x.v) {}
        explicit  simdv(const void *const b)
        {
            v = vld1_u64(reinterpret_cast<const uint8x8_t *>(b));
        }

        inline mask_t to_bitmask(void) const
        {
            return vget_lane_u64(vand_u64(vreinterpret_u64_u8(v), vcreate_u64(constant::c80)));
        }

        inline operator mask_t(void) const
        {
            return to_bitmask();
        }

        inline simdv operator==(const int x) const
        {
            return vceq_u8(v, vdup_n_u8(x));
        }

        inline simdv operator|(const simdv &x) const
        {
            return vorr_u8(v, x.v);
        }

        static inline uint8x8_t splat(const int x)
        {
            return vdup_n_u8(x);
        }

        template <int A, int B>
        inline uint8x8_t cmpngt_lt(void) const
        {
            uint8x8_t x = vsub_u8(v, vdup_n_u8(A));
            return vcgt_u8(x, vdup_n_u8(B - A));
        }

        template <int ALPHAONLY>
        inline mask_t fast_not_tchar(void) const
        {
            // TODO use TABLE
            uint8x8_t x = vsub_u8(v, vdup_n_u8(0x41)); // shift v so that v < 26 for any alphabet char
            uint8x8_t is_not_letter = vcgt_u8(x, vdup_n_u8(0x19));
            if HTTPVO_CXX17_CONSTEXPR (ALPHAONLY)
                return is_not_letter;
            else
            {
                uint8x8_t is_not_number = vcgt_u8(veor_u8(v, vcreate_u8(constant::c30)), vdup_n_u8(0x9));
                uint8_t is_not_hyphen   = vmvn_u8(vceq_u8(v, vdup_n_u8(0x2d)));
                return vand_u8(vand_u8(is_not_letter, is_not_number), is_not_hyphen);
            }
        }

        template <bool INCLUDE_TAB = setup::priotize_horizontal_tab>
        inline uint8x8_t is_not_print_char(void) const
        {
            const uint8x8_t not_print = vorr_u8(vclt_u8(v, vcreate_u8(constant::c20)), vceq_u8(v, vcreate_u8(constant::c7f)));
            if HTTPVO_CXX17_CONSTEXPR (INCLUDE_TAB)
                return vorr_u8(not_print, vceq_u8(v, vcreate_u8(constant::c09)));
            else
                return not_print;
        }

        static inline uint8x8_t countzero_bitmask(const mask_t m)
        {
            return bits::tzcnt<u64_t>(m) >> 3;
        }
    };

    template <>
    struct simdv<16>
    {
        static constexpr int bitpos = 4;
        static constexpr int N = 16;

        uint8x16_t v;

        simdv(void) = default;
        simdv(const uint8x16_t x) : v{ x } {}
        simdv(const simdv&     x) : v{x.v} {}
        explicit simdv(const void *b)
        {
            v = vld1q_u8(reinterpret_cast<const uint8x16_t *>(b));
        }

        TARGET("neon")
        inline u64_t to_bitmask(void) const
        {
            const uint8x8_t x = vshrn_n_u16(vreintrepretq_u16_u8(v), 4);
            return vget_lane_u64(vreintrepretq_u64_u8(x), 0);
        }

        static inline u64_t countzero_bitmask(const mask_t m)
        {
            return bits::tzcnt(m) >> 2;
        }

        template <u8_t a = 0, u8_t b = 0>
        TARGET("neon")
        inline simdv cmpngt_lt()
        {
            const uint8x8_t x = vsubq_u8(v, vdupq_n_u8(a));
            return vcgtq_u8(w, vdupq_n_u8(b - a));
        }

        TARGET("neon")
        inline simdv shuf_table(const simdv &tab) const
        {
            return vqtbl1q_u8(v, tab.v);
        }

        TARGET("neon")
        inline simdv operator&(const int x) const
        {
            return vandq_u8(v, vdupq_n_u8(x));
        }

        TARGET("neon")
        inline simdv operator<<(const int r) const
        {
            return vshlq_n_u8(v, r);
        }

        TARGET("neon")
        inline simdv operator|(const simdv &x) const
        {
            return vorrq_u8(v, x.v);
        }

        TARGET("neon")
        inline simdv operator^(const simdv &x) const
        {
            return veorq_u8(v, x.v);
        }

        TARGET("neon")
        inline simdv operator==(const int x) const
        {
            return vceqq_u8(v, vdupq_n_u8(x));
        }

        TARGET("neon")
        inline simdv operator<=(const int x) const
        {
            return vcleq_u8(v, vdupq_n_u8(x));
        }

        TARGET("neon")
        inline simdv operator>(const simdv &x) const
        {
            return vcgtq_u8(v, x.v);
        }

        TARGET("neon")
        inline simdv andnot(const simdv &x) const
        {
            // a & ~b
            return vbicq_u8(x.v, v);
        }

        TARGET("neon")
        inline simdv andeqz(const simdv &x) const
        {
            return vmvnq_u8(vtstq_u8(v, x.v));
        }

        TARGET("neon")
        inline operator mask_t(void) const
        {
            return to_bitmask();
        }

        TARGET("neon")
        static inline simdv load_tbl1(const void *const b)
        {
            // vad a 16B table
            return simdv(b);
        }
    };
}
#endif // HTTPVO_SIMD_ARM_NEON_HPP