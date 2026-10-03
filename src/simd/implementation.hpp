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
     static constexpr VWidth min = 8;

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

          inline operator mask_t(void) const
          {
#              if HTTPVO_HAVE__ARM_NEON__
               return vget_lane_u64(vand_u64(vreinterpret_u64_u8(v), vcreate_u64(constant::c80)));
#              endif
               return v;
          }

          static inline simdv load(const void * const b)
          {
               return simdv(b);
          }

          inline mask_t to_bitmask(void) const
          {
               return v;
          }

          static inline v64_t countzero_bitmask(const mask_t m)
          {
               return bits::tzcnt(m) >> 3;
          }

          inline simdv cmpeq(const simdv& u) const
          {
#              if HTTPVO_HAVE__ARM_NEON__
                   return vceq_u8(v, u.v);
#              endif
               return bits::andnot(constant::c80, (v ^ u.v) | (((v ^ u.v) & constant::c7f) + constant::c7f));
          }

          inline constexpr v64_t splat_u64(const u8_t x) const
          {
               return x * constant::c01;
          }

          inline constexpr simdv splat(const u8_t x) const
          {
               return x * constant::c01;
          }

          template <u8_t a>
          inline simdv cmplt(const u8_t _aa=0) const
          {
#              if HTTPVO_HAVE__ARM_NEON__
               return vclt_u8(v, vdup_n_u8(a));
#              endif
               static_assert(a < 0x7f);
               static constexpr v64_t aa = splat(0x7f + a);
               return (aa - (v & constant::c7f)) & bits::andnot(constant::c80, v);
          }

          template <u8_t a, u8_t b>
          inline simdv cmpgt_lt(const u8_t _aa = 0, const u8_t _bb = 0) const
          {
               static_assert(a < 0x7f && b < 0x80);
#              if HTTPVO_HAVE__ARM_NEON__
                    if constexpr (sizeof(T) == 8)
                    {
                         uint8x8_t aa = vdup_n_u8(a);
                         uint8x8_t bb = vdup_n_u8(b);
                         return vand_u8(vcgt_u8(x, aa), vclt_u8(x, bb));
                    }
#              endif
               static constexpr v64_t aa = splat_u64(0x7f - a);
               static constexpr v64_t bb = splat_u64(0x7f + b);
               return (bb - (v & constant::c7f)) & (aa + (v & constant::c7f)) & bits::andnot(constant::c80, v);
          }

          template <u8_t a, u8_t b>
          inline simdv cmpngt_lt(const u8_t _aa = 0, const u8_t _bb = 0) const
          {
               static_assert(a < 0x7f && b < 0x80);
#              if HTTPVO_HAVE__ARM_NEON__
                    if constexpr (sizeof(T) == 8)
                    {
                         uint8x8_t aa = vdup_n_u8(a);
                         uint8x8_t x = vsub_u8(v, vdup_n_u8(a));
                         return vcgt_u8(x, vdup_n_u8(b - a));
                    }
#              endif
               static constexpr v64_t aa = splat_u64(0x7f + a);
               static constexpr v64_t bb = splat_u64(0x7f - b);
               return ((aa - (v & constant::c7f)) | (bb + (v & constant::c7f))) & bits::andnot(constant::c80, v);
          }
     };
}
#endif // HTTPVO_SIMD_IMPLEMENTAION_HPP