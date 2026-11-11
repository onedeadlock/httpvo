#include "simd.hpp"

namespace httpvo::simd
{
    template <>
    struct simdv<VMIN>
    {
        static constexpr int bitpos = 8;
        static constexpr int N      = VMIN;

        uint_t v;
        simdv(void) = default;
        constexpr simdv(const uint_t    x) : v( x ) {}
        constexpr simdv(const simdv& x) : v(x.v) {}
        explicit  simdv(const void *const b) noexcept
        {
            httpvo_memcopy(&v, b, VMIN);
        }

        static inline constexpr uint_t splat(const int x)
        {
            return x * constant::c01;
        }

        static inline int countzero_bitmask(const mask_t m)
        {
            return bits::tzcnt<uint_t>(static_cast<const uint_t>(m)) >> 3;
        }

        inline mask_t to_bitmask(void) const noexcept
        {
            return v;
        }

        inline operator mask_t(void) const noexcept
        {
            return v;
        }

        inline simdv operator==(const int x) const noexcept
        {
            const uint_t u = v ^ splat(x);
            return bits::andnot(constant::c80, u | ((u & constant::c7f) + constant::c7f));
        }

        inline simdv operator|(const simdv &x) const noexcept
        {
            return v | x.v;
        }

        template <int A, int B>
        inline uint_t cmpngt_lt(void) const noexcept
        {
            static_assert(A < 0x7f && B < 0x80);
            constexpr uint_t a = splat(0x7f + A);
            constexpr uint_t b = splat(0x7f - B);
            const     uint_t x = v & constant::c7f;
            return ((a - x) | (b + x)) & bits::andnot(constant::c80, v);
        }

        inline uint_t _is_not_letter(const uint_t u) const noexcept
        {
            // :: u < 0x80
            constexpr uint_t A = splat(0x80 + 0x40);
            constexpr uint_t Z = splat(0x80 - 0x5b);

            const  uint_t x  = u & constant::cdf; // Convert ['a' - 'z'] to ['A' - 'Z']
            return (Z + x) | (A - x); // above Z or below A 
        }

        inline uint_t _is_not_number(const uint_t u) const noexcept
        {
            // :: u < 0x80
            constexpr uint_t N10 = splat(0x80 - 0xa);

            const uint_t x = u ^ constant::c30; // convert ['0' - '9'] to [0 - 9]
            return     x + N10; // is greater than 9                    
        }

        inline uint_t _is_not_hyphen(const uint_t u) const noexcept
        {
            // :: u < 0x80
            constexpr uint_t HYPHEN_NORM = splat(0x2d);
            const uint_t to_zero = (u & constant::c7f) ^ HYPHEN_NORM; // set all hyphens to zero
            return     to_zero + constant::c7f; // extract zero lanes
        }

        template <int ALPHAONLY>
        inline mask_t fast_not_tchar(void) const noexcept
        {
            const uint_t x = v & constant::c80; // make v < 0x80
            if HTTPVO_CXX17_CONSTEXPR (ALPHAONLY)
                return (_is_not_letter(x) | v) & constant::c80;
            else 
                return ((_is_not_letter(x) & _is_not_number(x)) & (_is_not_hyphen(x) | v)) & constant::c80;
        }

        template <bool INCLUDE_TAB = setup::priotize_horizontal_tab>
        inline uint_t is_not_print_char(uint_t v) const noexcept
        {
            const uint_t x = v & constant::c7f; // make v < 0x80
            // @not_print: (159 - v) > 127 when v < 32 (control bytes) therefore its bit 7 is left set whereas v => 32 unsets it
            // @is_del:    v + 1, shifts 0x7f to 0x80 setting bit 7
            const uint_t not_print = constant::c9f - x;
            const uint_t is_del    = x + constant::c01;
            const uint_t out       = not_print | is_del;

            if HTTPVO_CXX17_CONSTEXPR (INCLUDE_TAB)
            {
                 const uint_t is_htab = (x ^ constant::c09) - constant::c01;
                 return (out ^ is_htab) & bits::andnot(constant::c80, v); // HTAB exclude from bad chars
            }
            else return out & bits::andnot(constant::c80, v); // HTAB include in bad chars 
        }
    };
} // namespace simd