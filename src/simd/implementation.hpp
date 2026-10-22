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
                    httpvo_memcopy(&v, b, 8);
          }

          static inline u32_t _load_u64(const void * const b)
          {
               if constexpr (setup::support_unaligned)
                    return reinterpret_cast<const u64_t *>(b)[0];
               u64_t v;
               httpvo_memcopy(&v, b, 8);
               return v;
          }

          static inline u32_t _load_u32(const void * const b)
          {
               if constexpr (setup::support_unaligned)
                    return reinterpret_cast<const u32_t *>(b)[0];
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
          inline simdv cmplt(void) const
          {
#              if HTTPVO_HAVE__ARM_NEON__
                    return vclt_u8(v, vdup_n_u8(a));
#              endif
               static_assert(a < 0x7f);
               static constexpr v64_t aa = splat(0x7f + a);
               return (aa - (v & constant::c7f)) & bits::andnot(constant::c80, v);
          }

          template <u8_t A>
          inline u64_t cmpgt(u64_t v)
          {
#              if HTTPVO_HAVE__ARM_NEON__
                    return vcgt_u8(v, vdup_n_u8(a));
#              endif
               static_assert(A < 0x7f);
               static constexpr u64_t a = _dup(0x7f - A);
               return (v | (a + (v & constant::c7f))) & constant::c80;
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
               // Non printable characters here are 0x7f (DEL) or characters below 0x20 (sp)
#              if HTTPVO_HAVE__ARM_NEON__
                    assert(0);
#              else
                    // v + 1 wraps v == 0x7f to zero. The check becomes v + 1 > 0x20.  
                    static constexpr u64_t u = constant::c80 | constant::c20;
                    return (u - (((v & constant::c7f) + constant::c01) & constant::c7f)) & bits::andnot(constant::c80, v);
#              endif
          }

          inline u64_t non_printable_andn_tab(void) const
          {
               // Non printable characters here are 0x7f (DEL) or characters below 0x20 (sp) excluding tab
#              if HTTPVO_HAVE__ARM_NEON__
                    assert(0);
#              else
               // individual compare for DEL, HTAB and >= 0x20
               const u64_t v_no_overflow =  v | constant::c80;
               const u64_t not_htab      = (v_no_overflow ^ constant::c09) - constant::c01;
               const u64_t is_printable  = (v_no_overflow - constant::c20) ^ (v_no_overflow - constant::c7f); // v >= 0x20 and not DEL
               return bits::andnot(not_htab, is_printable | v) & constant::c80;
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