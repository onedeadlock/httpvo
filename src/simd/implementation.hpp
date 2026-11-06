#ifndef HTTPVO_SIMD_IMPLEMENTAION_HPP
#define HTTPVO_SIMD_IMPLEMENTAION_HPP
#include "../include/definition.hpp"

namespace httpvo::simd
{
     using VWidth = u64_t;
     using mask_t = u64_t;

     template <VWidth N>
     struct simdv;

     template <class... Ts>
     struct SIMD_IMPL_SET{};

     using SIMD_SET = SIMD_IMPL_SET<simdv<8>, simdv<16>, simdv<32>>;

     template <class T>
     using REMOVE_CVREF = std::remove_cv_t<std::remove_reference_t<T>>;

     template <class T, class Ts>
     struct IS_SIMD_IMPL_SET;

     template <class T, class... Ts>
     struct IS_SIMD_IMPL_SET<T, SIMD_IMPL_SET<Ts...>> : std::integral_constant<bool, (std::is_same_v<REMOVE_CVREF<T>, Ts> || ...)>{};

     template <class T>
     using is_simd_v = IS_SIMD_IMPL_SET<T, SIMD_SET>;

     template<>
     struct simdv<8>
     {
#         if HTTPVO_HAVE__ARM_NEON__
          using v64_t = uint8x8_t;
#         else
          using v64_t = u64_t;
#         endif
          static constexpr int bitpos = 8;
         
          v64_t v;
          
          constexpr simdv(v64_t vv) : v(vv){}
          constexpr simdv(const simdv& vv) : v(vv.v){}
          explicit  simdv(const void * const b)
          {
#              if HTTPVO_HAVE__ARM_NEON__
                    v = vld1_u64(reinterpret_cast<const v64_t *>(b));
#              else
                    httpvo_memcopy(&v, b, 8);
#endif
          }

          static inline u64_t _load_u64(const void * const b)
          {
               u64_t v;
               httpvo_memcopy(&v, b, 8);
               return v;
          }

          static inline u32_t _load_u32(const void * const b)
          {
               u32_t v;
               httpvo_memcopy(&v, b, 4);
               return v;
          }

          inline mask_t to_bitmask(void) const
          {
#              if HTTPVO_HAVE__ARM_NEON__
               return vget_lane_u64(vand_u64(vreinterpret_u64_u8(v), vcreate_u64(constant::c80)));
#              endif
               return v;
          }

          inline operator mask_t(void) const
          {
               return to_bitmask();
          }

          inline u64_t _cmpzero_u64(const u64_t x) const
          {
               return bits::andnot(constant::c80, x | ((x & constant::c7f) + constant::c7f));
          }

          inline simdv cmpzero(void)
          {
#              if HAVE__ARM_NEON__
                    return vceq_u8(v, vcreate_u8(0));
#              endif
               return _cmpzero_u64(v);
          }

          inline simdv cmpeq(const simdv u) const
          {
#              if HTTPVO_HAVE__ARM_NEON__
                   return vceq_u8(v, u.v);
#              endif
               return _cmpzero_u64(v ^ u.v);
          }

          static inline bool testmask_u64(const void * const b, const mask_t mask_v, const mask_t mask_z)
          {
               return (_load_u64(b) & mask_v) ^ mask_z;
          }

          static inline constexpr v64_t splat(const int x)
          {
#              if HTTPVO_HAVE__ARM_NEON__
                   return vdup_n_u8(x);
#              endif
               return x * constant::c01;
          }

          template <u8_t a>
          inline v64_t cmplt(void) const
          {
#              if HTTPVO_HAVE__ARM_NEON__
                    return vclt_u8(v, vdup_n_u8(a));
#              endif
               static_assert(a < 0x7f);
               static constexpr v64_t aa = splat(0x7f + a);
               return (aa - (v & constant::c7f)) & bits::andnot(constant::c80, v);
          }

          template <u8_t A>
          inline v64_t cmpgt(u64_t v)
          {
#              if HTTPVO_HAVE__ARM_NEON__
                    return vcgt_u8(v, vdup_n_u8(a));
#              endif
               static_assert(A < 0x7f);
               static constexpr u64_t a = _dup(0x7f - A);
               return (v | (a + (v & constant::c7f))) & constant::c80;
          }

          template <u8_t A, u8_t B>
          inline v64_t cmpngt_lt(void) const
          {
               static_assert(A < 0x7f && B < 0x80);
#              if HTTPVO_HAVE__ARM_NEON__
                    uint8x8_t x = vsub_u8(v, vdup_n_u8(A));
                    return vcgt_u8(x, vdup_n_u8(B - A));
#              else
                    static constexpr v64_t a = splat(0x7f + A);
                    static constexpr v64_t b = splat(0x7f - B);
                    const v64_t x = v & constant::c7f;
                    return ((a - x) | (b + x)) & bits::andnot(constant::c80, v);
#              endif
          }

          inline v64_t _is_not_letter(void) const
          {
               constexpr v64_t A = splat(0x80 + 0x40);
               constexpr v64_t Z = splat(0x80 - 0x5b);
               constexpr v64_t AZNORM = constant::c7f & constant::cdf;

               const v64_t v_norm_letter_case = v & AZNORM; // Convert ['a' - 'z'] to ['A' - 'Z']
               const v64_t is_above_z = Z + v_norm_letter_case;
               const v64_t is_below_a = A - v_norm_letter_case;
               return is_below_a | is_above_z;
          }

          inline v64_t is_not_letter(void) const
          {
#              if HTTPVO_HAVE__ARM_NEON__
                    const v64_t v_norm_letter_case = vand_u8(v, vcreate_u8(constant::cdf));
                    const v64_t x = vsub_u8(v_norm_letter_case, vdup_n_u8(0x41)); // flatten range so that x < 26 for any alphabet char
                    return vcgt_u8(x, vdup_n_u8(0x19));
#              else
                    return (_is_not_letter() | v) & constant::c80;
#              endif
          }

          inline v64_t _is_not_number(void) const
          {
               constexpr v64_t N10 = splat(0x80 - 0xa);
               constexpr v64_t NUM_NORM = constant::c30;

               const v64_t v_norm_numbers = (v & constant::c7f) ^ NUM_NORM; // convert ['0' - '9'] to [0 - 9]
               return N10 + v_norm_numbers; // is greater than 9
          }

          inline v64_t is_not_number(void) const
          {
#              if HTTPVO_HAVE__ARM_NEON__
                   return vcgt_u8(veor_u8(v, vcreate_u8(constant::c30)), vdup_n_u8(0x9));
#              else
                   return (_is_not_number() | v) & constant::c80;
#              endif
          }

          inline v64_t _is_not_hyphen() const
          {
               constexpr v64_t HYPHEN_NORM = splat(0x2d);
               const v64_t set_all_hypens_to_zero = (v & constant::c7f) ^ HYPHEN_NORM;
               return set_all_hypens_to_zero + constant::c7f; // extract zero lanes
          }

          inline v64_t is_not_hyphen() const
          {
#              if HTTPVO_HAVE__ARM_NEON__
                   return vmvn_u8(vceq_u8(v, vdup_n_u8(0x2d)));
#              else
                   return (_is_not_hyphen() | v) & constant::c80;
#              endif
          }

          template<int ALPHAONLY>
          inline mask_t fast_not_tchar(void) const
          {
               if constexpr (ALPHAONLY)
                    return is_not_letter();
               else
               {
#                   if HTTPVO_HAVE__ARM_NEON__
                         return vand_u8(vand_u8(is_not_letter(), is_not_number()), is_not_hyphen());
#                   else
                         return ((_is_not_letter() & _is_not_number()) & (_is_not_hyphen() | v)) & constant::c80;
#                   endif
               }
          }

          inline v64_t _not_printable_no_tab(const v64_t v) const
          {
               // :: v < 0x80
               // @not_print: (159 - v) > 127 when v < 32 (control bytes) therefore its bit 7 is left set whereas v => 32 unsets it
               // @is_del:    v + 1, shifts 0x7f to 0x80 setting bit 7
               const v64_t not_print = constant::c9f - v;
               const v64_t is_del    = v + constant::c01;
               return not_print | is_del;
          }

          inline v64_t _not_printable_tab(const v64_t v) const
          {
               // :: v < 0x80
               const v64_t is_htab = (v ^ constant::c09) - constant::c01;
               return _not_printable_no_tab(v) ^ is_htab;
          }

          template<bool INCLUDE_TAB=setup::priotize_horizontal_tab>
          inline v64_t is_not_print_char(void) const
          {
#              if HTTPVO_HAVE__ARM_NEON__
                    const v64_t not_print = vorr_u8(vclt_u8(v, vcreate_u8(constant::c20)), vceq_u8(v, vcreate_u8(constant::c7f)));
                    if constexpr (INCLUDE_TAB)
                         return vorr_u8(not_print, vceq_u8(v, vcreate_u8(constant::c09)));
                    else not_print;
#              else
                    const v64_t v_no_overflow = v & constant::c7f;
                    if constexpr (INCLUDE_TAB)
                         return _not_printable_tab(v_no_overflow)    & bits::andnot(constant::c80, v);
                    else return _not_printable_no_tab(v_no_overflow) & bits::andnot(constant::c80, v);
#              endif
          }

          static inline simdv load(const void * const b)
          {
               return simdv(b);
          }

          static inline v64_t countzero_bitmask(const mask_t m)
          {
               return bits::tzcnt(m) >> 3;
          }
     };
}
#endif // HTTPVO_SIMD_IMPLEMENTAION_HPP