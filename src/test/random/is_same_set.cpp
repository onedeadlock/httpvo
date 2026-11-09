#include <type_traits>
#include <iostream>

#if 0
// c++17 - ...
template <class... Ts>
struct TYPE_SET {};

using SET = TYPE_SET<int, char, wchar_t>;

template<class T>
using NONCVREF = std::remove_cv_t<std::remove_reference_t<T>>;

template<class T, class Ts>
struct IS_SIMD_V{};

template<class T, class... Ts>
struct IS_SIMD_V <T, TYPE_SET<Ts...>> : std::integral_constant<bool, (std::is_same_v<NONCVREF<T>, Ts> || ...)>{};

template<class T>
using is_simd_v = IS_SIMD_V<T, SET>;
#endif

template <class... Ts>
struct SIMD_IMPL_SET;

using SIMD_SET = SIMD_IMPL_SET<int, char, float>;

template <class T>
using REMOVE_CVREF = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

template<bool...>
struct PACK;

template <bool... Bs>
using IS_SAME = std::integral_constant<bool, not std::is_same<PACK<false, Bs...>, PACK<Bs..., false>>()>;

template <class T, class Ts = void>
struct V;

template <class T, class... Ts>
struct V<T, SIMD_IMPL_SET<Ts...>> : IS_SAME<std::is_same<T, Ts>::value...>{};

template <class T>
using IS_SAME_TYPE = V<T, SIMD_SET>;

int main(void)
{
    std::cout << IS_SAME_TYPE<int>() << std::endl;
    std::cout << IS_SAME_TYPE<char>() << std::endl;
    std::cout << IS_SAME_TYPE<float>() << std::endl;
    std::cout << (IS_SAME_TYPE<double>() == 0) << std::endl;;
    return 0;
}