#ifndef HTTPVO_SIMD_IMPLEMENTAION_HPP
#define HTTPVO_SIMD_IMPLEMENTAION_HPP
#include "../include/definition.hpp"

namespace httpvo::simd
{
     constexpr int MINWIDTH = std::numeric_limits<INT>::digits / 8; // minimum supported width
     constexpr int MAXWIDTH = 32; // maximum supported width

     template <int N=MINWIDTH>
     struct simdv;

     template <class... Ts>
     struct SIMD_IMPLEMENTATION_SET{};

     template <bool...>
     struct BOOL_SET;

      template <class T, class Ts = void>
      struct IS_SIMD;

     using SIMD_SET = SIMD_IMPLEMENTATION_SET<simdv<MINWIDTH>, simdv<16>, simdv<32>>;

     template <class T>
     using REMOVE_CVREF_T = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

     template <bool... Bs>
     using ANY_OF_T = std::integral_constant<bool, not std::is_same<BOOL_SET<false, Bs...>, BOOL_SET<Bs..., false>>::value>;

     template <class T, class... Ts>
     struct IS_SIMD <T, SIMD_IMPLEMENTATION_SET<Ts...>> : ANY_OF_T<std::is_same<REMOVE_CVREF_T<T>, Ts>::value...>{};

     template <class T>
     using is_simd = IS_SIMD<T, SIMD_SET>;

     constexpr u8_t width_select[8]{31, 63, 15, 0, 31};
     constexpr u8_t mask_select[8]{0xff, 0xaa, 0, 0x88, 0, 0, 0, 0x80};

     template<int N>
     constexpr int mask_width(void)
     {
          static_assert(N and bits::is_power_of_two(N));
          return static_cast<int>(width_select[N >> 3]);
     }

     template <typename T, T width, T splat>
     constexpr T mask_fill(void)
     {
          static_assert(std::is_integral<T>() and not std::is_same<T, bool>() and std::is_unsigned<T>());
          // fill & mask
          return static_cast<T>((1ULL << width) - 1) & static_cast<T>(mask_select[splat - 1] * constant::c01);
     }

     template<>
     struct simdv<8>
     {
#         if HTTPVO_HAVE__ARM_NEON__
          using Intv = uint8x8_t;
#         else
          using Intv = INT;
#         endif
          static constexpr int bitpos = MINWIDTH;
          static constexpr int N  = MINWIDTH;
         
          Intv v;
          simdv(void) = default;
          constexpr simdv(Intv vv) : v(vv){}
          constexpr simdv(const simdv& vv) : v(vv.v){}
          explicit  simdv(const void * const b)
          {
#              if HTTPVO_HAVE__ARM_NEON__
                    v = vld1_u64(reinterpret_cast<const Intv *>(b));
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

          inline simdv operator==(const int x) const
          {
#              if HTTPVO_HAVE__ARM_NEON__
                    return vceq_u8(v, vdup_n_u8(x));
#              else
                    const Intv u = v ^ splat(x);
                    return bits::andnot(constant::c80, u | ((u & constant::c7f) + constant::c7f));
#              endif

          }

           inline simdv operator|(const simdv& x) const
          {
#              if HTTPVO_HAVE__ARM_NEON__
                    return vorr_u8(v, x.v);
#              else
                    return v | x.v;
#              endif
          }

          static inline constexpr Intv splat(const int x)
          {
#              if HTTPVO_HAVE__ARM_NEON__
                   return vdup_n_u8(x);
#              endif
               return x * constant::c01;
          }

          template <u8_t a>
          inline Intv cmplt(void) const
          {
#              if HTTPVO_HAVE__ARM_NEON__
                    return vclt_u8(v, vdup_n_u8(a));
#              endif
               static_assert(a < 0x7f);
               constexpr Intv aa = splat(0x7f + a);
               return (aa - (v & constant::c7f )) & bits::andnot( constant::c80, v);
          }

          template <u8_t A>
          inline Intv cmpgt(u64_t v)
          {
#              if HTTPVO_HAVE__ARM_NEON__
                    return vcgt_u8(v, vdup_n_u8(a));
#              endif
               static_assert(A < 0x7f);
               constexpr u64_t a = splat(0x7f - A);
               return (v | (a + (v & constant::c7f ))) & constant::c80;
          }

          template <u8_t A, u8_t B>
          inline Intv cmpngt_lt(void) const
          {
               static_assert(A < 0x7f && B < 0x80);
#              if HTTPVO_HAVE__ARM_NEON__
                    uint8x8_t x = vsub_u8(v, vdup_n_u8(A));
                    return vcgt_u8(x, vdup_n_u8(B - A));
#              else
                    static constexpr Intv a = splat(0x7f + A);
                    static constexpr Intv b = splat(0x7f - B);
                    const Intv x = v & constant::c7f;
                    return ((a - x) | (b + x)) & bits::andnot(constant::c80, v);
#              endif
          }

          inline INT _is_not_letter(void) const
          {
               constexpr INT A = splat(0x80 + 0x40);
               constexpr INT Z = splat(0x80 - 0x5b);
               constexpr INT AZNORM = constant::c7f & constant::cdf;

               const INT v_norm_letter_case = v & AZNORM; // Convert ['a' - 'z'] to ['A' - 'Z']
               const INT is_above_z = Z + v_norm_letter_case;
               const INT is_below_a = A - v_norm_letter_case;
               return is_below_a | is_above_z;
          }

          inline Intv is_not_letter(void) const
          {
#              if HTTPVO_HAVE__ARM_NEON__
                    const Intv v_norm_letter_case = vand_u8(v, vcreate_u8(constant::cdf));
                    const Intv x = vsub_u8(v_norm_letter_case, vdup_n_u8(0x41)); // flatten range so that x < 26 for any alphabet char
                    return vcgt_u8(x, vdup_n_u8(0x19));
#              else
                    return (_is_not_letter() | v) & constant::c80;
#              endif
          }

          inline INT _is_not_number(void) const
          {
               constexpr INT N10 = splat(0x80 - 0xa);
               constexpr INT NUM_NORM = constant::c30;

               const INT v_norm_numbers = (v & constant::c7f) ^ NUM_NORM; // convert ['0' - '9'] to [0 - 9]
               return N10 + v_norm_numbers; // is greater than 9
          }

          inline Intv is_not_number(void) const
          {
#              if HTTPVO_HAVE__ARM_NEON__
                   return vcgt_u8(veor_u8(v, vcreate_u8(constant::c30)), vdup_n_u8(0x9));
#              else
                   return (_is_not_number() | v) & constant::c80;
#              endif
          }

          inline INT _is_not_hyphen() const
          {
               constexpr INT HYPHEN_NORM = splat(0x2d);
               const INT set_all_hyphens_to_zero = (v & constant::c7f) ^ HYPHEN_NORM;
               return set_all_hyphens_to_zero + constant::c7f; // extract zero lanes
          }

          inline Intv is_not_hyphen() const
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
               if HTTPVO_CXX17_CONSTEXPR (ALPHAONLY)
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

          inline INT _not_printable_no_tab(const INT v) const
          {
               // :: v < 0x80
               // @not_print: (159 - v) > 127 when v < 32 (control bytes) therefore its bit 7 is left set whereas v => 32 unsets it
               // @is_del:    v + 1, shifts 0x7f to 0x80 setting bit 7
               const INT not_print = constant::c9f - v;
               const INT is_del    = v + constant::c01;
               return not_print | is_del;
          }

          inline INT _not_printable_tab(const INT v) const
          {
               // :: v < 0x80
               const INT is_htab = (v ^ constant::c09) - constant::c01;
               return _not_printable_no_tab(v) ^ is_htab;
          }

#         if HTTPVO_HAVE__ARM_NEON__
          template<bool INCLUDE_TAB=setup::priotize_horizontal_tab>
          inline uint8x8_t _is_not_print_char(uint8x8_t v) const
          {
               const uint8x8_t not_print = vorr_u8(vclt_u8(v, vcreate_u8(constant::c20)), vceq_u8(v, vcreate_u8(constant::c7f)));
               if HTTPVO_CXX17_CONSTEXPR (INCLUDE_TAB)
                    return vorr_u8(not_print, vceq_u8(v, vcreate_u8(constant::c09)));
               else return not_print;
          }
#         else
          template<bool INCLUDE_TAB=setup::priotize_horizontal_tab>
          inline INT _is_not_print_char(INT v) const
          {
               const INT v_no_overflow = v & constant::c7f;
               if HTTPVO_CXX17_CONSTEXPR (INCLUDE_TAB)
                    return _not_printable_tab(v_no_overflow)    & bits::andnot(constant::c80, v);
               else return _not_printable_no_tab(v_no_overflow) & bits::andnot(constant::c80, v);
          }
#         endif

          template<bool INCLUDE_TAB=setup::priotize_horizontal_tab>
          inline Intv is_not_print_char(void) const
          {
               return _is_not_print_char(v);
          }

          static inline simdv load(const void * const b)
          {
               return simdv(b);
          }

          static inline Intv countzero_bitmask(const mask_t m)
          {
               return bits::tzcnt<INT>(m) >> 3;
          }
     };
}
#endif // HTTPVO_SIMD_IMPLEMENTAION_HPP