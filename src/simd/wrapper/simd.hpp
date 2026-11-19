#ifndef HTTPVO_SIMD_HPP
#define HTTPVO_SIMD_HPP
#include "../../include/definition.hpp"
#include "../../include/constants.hpp"
#include "../../include/bits.hpp"

namespace httpvo::simd
{
     constexpr int VMIN = setup::neon ? 8 : std::numeric_limits<uint_t>::digits / 8; // minimum simd width
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

     // see test/scripts/generate_shuffle_table.py for table generation and comments.
     alignas(16) static constexpr u8_t TCHAR_CLASS_LUT[32]{
        58, 63, 62, 63, 63, 63, 63, 63, 62, 62, 61, 21, 52, 21, 61, 28,
        00, 00, 01, 02, 04,  8, 16, 32, 00, 00, 00, 00, 00, 00, 00, 00
     };

     // see test/scripts/generate_shuffle2_table.py for table generation and comments.
     alignas(16) static constexpr u8_t CONTROL_CHAR_CLASS_LUT[32]{
        002, 002, 002, 002, 002, 002, 002, 002, 002, 001, 002, 002, 002, 002, 002, 003,
        001, 000, 127, 127, 127, 127, 127, 002, 127, 127, 127, 127, 127, 127, 127, 127,
     };

     constexpr u8_t WIDTH_SELECT[8]{31, 63, 15, 0, 31};
     constexpr u8_t MASK_SELECT[8]{0xff, 0xaa, 0, 0x88, 0, 0, 0, 0x80};

     template<int I>
     constexpr mask_t make_mask(const int n)
     {
          if HTTPVO_CXX17_CONSTEXPR (setup::debug)
               assert(n <= std::numeric_limits<mask_t>::digits - 1);
          if HTTPVO_CXX17_CONSTEXPR (I == 1)
               return 0;
          else return (mask_t(1) << n) - 1;
     }

     template<int N>
     constexpr int mask_width(void)
     {
          static_assert(N and bits::is_power_of_two(N));
          return static_cast<int>(WIDTH_SELECT[N >> 3]);
     }

     template <typename T, T width, T splat>
     constexpr typename std::make_unsigned<T>::type mask_fill(void)
     {
          static_assert(std::is_integral<T>() and not std::is_same<T, bool>());
          // fill & mask
          using U = typename std::make_unsigned<T>::type;
          return static_cast<U>((1ULL << width) - 1) & static_cast<U>(MASK_SELECT[splat - 1] * constant::c01);
     }
}
#endif // HTTPVO_SIMD_IMPLEMENTAION_HPP