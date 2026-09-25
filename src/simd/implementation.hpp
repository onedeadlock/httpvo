#ifndef HTTPVO_SIMD_IMPLEMENTAION_HPP
#define HTTPVO_SIMD_IMPLEMENTAION_HPP
#include "../common/common.hpp"
#include "../include/definition.hpp"

namespace httpvo::simd
{
    template <int N> struct alignas(N) simdv;

     #ifndef FORCE_SIMD_32
          static constexpr int max = 64;
     #else
          static constexpr int max = 32;
     #endif
     

     template<> 
     struct alignas(8) simdv<8>
     {
          u64_t v;
          constexpr simdv(u64_t vv) : v(vv){}
          constexpr simdv(const simdv& vv) : v(vv.v){}
          explicit  simdv(const void *b)
          {
              #if HAVE__ARM_NEON__
               return vget_lane_u64(vld1_u64(reinterpret_cast<const u64_t *>(b)), 0);
              #elif __HAVE_SUPPORT_FOR_UNALIGNED__
               v = reinterpret_cast<u64_t *>(b)[0];
              #endif
               __builtin_memcpy(&v, b, 8);
          }

          static inline simdv load(const void *b)
          {
               return simdv(b);
          }

          inline u64_t to_bitmask(void)
          {
               return v;
          }

          inline simdv cmpeq(const simdv& u)
          {
               #if HAVE__ARM_NEON__
               return vget_lane_u64(vreinterpret_u64(vceq_u8(vcreate_u8(v), vcreate_u8(0))), 0);
               #endif
               return bits::andnot(constant::c80, (v ^ u.v) | (((v ^ u.v) & constant::c7f) + constant::c7f));
          }

          inline simdv splat(const u8_t x)
          {
               return x * constant::c01;
          }

          template <u8_t a>
          inline simdv cmplt(const u8_t _aa=0)
          {
               static_assert(a < 0x7f);
               static constexpr u64_t aa = splat(0x7f + a);
               return (aa - (v & constant::c7f)) & bits::andnot(constant::c80, v);
          }

          template <u8_t a, u8_t b>
          inline simdv cmpgt_lt(const u8_t _aa = 0, const u8_t _bb = 0)
          {
               static_assert(a < 0x7f && b < 0x80);
               #if HAVE__ARM_NEON__
               if constexpr (sizeof(T) == 8)
               {
                    uint8x8_t aa = vdup_n_u8(a);
                    uint8x8_t bb = vdup_n_u8(b);
                    uint8x8_t x = vcreate_u8(v);
                    uint8x8_t o = vand_u8(vcgt_u8(x, aa), vclt_u8(x, bb));
                    return vget_lane_u64(vand_u64(vreinterpret_u64_u8(o), vcreate_u64(constant::c80)), 0);
               }
               #endif
               static constexpr u64_t aa = splat(0x7f - a);
               static constexpr u64_t bb = splat(0x7f + b);
               return (bb - (v & constant::c7f)) & (aa + (v & constant::c7f)) & bits::andnot(constant::c80, v);
          }
     };
}
#endif // HTTPVO_SIMD_IMPLEMENTAION_HPP