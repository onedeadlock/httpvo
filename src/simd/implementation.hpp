#ifndef HTTPVO_SIMD_IMPLEMENTAION_HPP
#define HTTPVO_SIMD_IMPLEMENTAION_HPP
#include "../common/common.hpp"
#include "../include/definition.hpp"

namespace httpvo::simd
{
     using VWidth = u64_t;
     using mask_t = u64_t;

     template <VWidth N>
     struct simdv;

     static constexpr VWidth TRAIL = 0;
#    ifndef HTTPVO_NO_64B
         static constexpr VWidth max = 64;
#    else
         static constexpr VWidth max = 32;
#    endif
     static constexpr     VWidth min = 8;

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
               #endif
               if constexpr (setup::support_unaligned)
                    v = reinterpret_cast<const v64_t *>(b)[0];
               else
                    __builtin_memcpy(&v, b, 8);
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

          inline simdv cmpeq(const simdv u) const
          {
#              if HTTPVO_HAVE__ARM_NEON__
                   return vceq_u8(v, u.v);
#              endif
               return bits::andnot(constant::c80, (v ^ u.v) | (((v ^ u.v) & constant::c7f) + constant::c7f));
          }

          static inline constexpr v64_t splat(const int x)
          {
#              if HTTPVO_HAVE__ARM_NEON__
                   return vdup_n_u8(x);
#              endif
               return x * constant::c01;
          }

          template <u8_t a>
          inline simdv cmplt(void) const
          {
#              if HTTPVO_HAVE__ARM_NEON__
               return vclt_u8(v, vdup_n_u8(a));
#              endif
               static_assert(a < 0x7f);
               static constexpr v64_t aa = splat(0x7f + a);
               return (aa - (v & constant::c7f)) & bits::andnot(constant::c80, v);
          }

          template <u8_t a, u8_t b>
          inline simdv cmpngt_lt(void) const
          {
               static_assert(a < 0x7f && b < 0x80);
#              if HTTPVO_HAVE__ARM_NEON__
                    uint8x8_t x = vsub_u8(v, vdup_n_u8(a));
                    return vcgt_u8(w, vdup_n_u8(b - a));
#              endif
               static constexpr v64_t aa = splat(0x7f + a);
               static constexpr v64_t bb = splat(0x7f - b);
               return ((aa - (v & constant::c7f)) | (bb + (v & constant::c7f))) & bits::andnot(constant::c80, v);
          }

          inline simdv ascii_letters(void) const
          {
#              if HTTPVO_HAVE__ARM_NEON__
                    assert(0);
#              else
                    return (constant::Z - (v & constant::AZ_const)) & (constant::A + (v & constant::AZ_const)) & bits::andnot(constant::c80, v);
#              endif
          }

          inline simdv ascii_numbers(void) const
          {
               const simdv x = v ^ constant::c30;
               return x.cmplt<10>();
          }

          inline simdv ascii_hyphen(void) const
          {
               return cmpeq(constant::hyphen);
          }

          inline mask_t ascii_fast_tchar(void) const
          {
               return ascii_letters() | ascii_numbers() | ascii_hyphen();
          }

          inline u64_t non_printable(void) const
          {
#              if HTTPVO_HAVE__ARM_NEON__
                    assert(0);
#              else
                    // Non printable characters here are 0x7f (DEL) or characters below 0x20 (sp)
                    static constexpr u64_t u = constant::c80 | constant::c20;
                    return (u - (((v & constant::c7f) + constant::c01) & constant::c7f)) & bits::andnot(constant::c80, v);
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