#ifndef HTTPVO_SIMD_IMPLEMENTAION_HPP
#define HTTPVO_SIMD_IMPLEMENTAION_HPP
#include "../../include/definition.hpp"
#include "../../include/constants.hpp"
#include "../../include/bits.hpp"

namespace httpvo::simd
{
     constexpr int VMIN = setup::neon ? 8 : (std::numeric_limits<uint_t>::digits + 1) / 8; // minimum simd width
     constexpr int VMAX = 32; // maximum simd width

     template <int N=VMIN>
     struct simdv;

     template <class... Ts>
     struct SIMD_IMPLEMENTATION_SET{};

     template <bool...>
     struct BOOL_SET;

      template <class T, class Ts = void>
      struct IS_SIMD;

     using SIMD_SET = SIMD_IMPLEMENTATION_SET<simdv<VMIN>, simdv<16>, simdv<32>>;

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
}
#endif // HTTPVO_SIMD_IMPLEMENTAION_HPP