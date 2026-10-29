#include <type_traits>
#include <iostream>

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

int main(void)
{
    std::cout << (is_simd_v<int>()) << std::endl; // true
    std::cout << (is_simd_v<char>()) << std::endl; // true
    std::cout << (is_simd_v<wchar_t>()) << std::endl; // true
    std::cout << (is_simd_v<float>()) << std::endl; // false
}