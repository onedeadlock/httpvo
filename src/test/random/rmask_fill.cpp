#include <cstdio>
#include <cstdint>
#include <cassert>

template <typename T>
constexpr T mask_fill(T width, T splat)
{
    assert(!(width & (width - 1)) and ((splat == 1) or !(splat & (splat - 1))));
    constexpr unsigned char bit_tab [[maybe_unused]] [8] {0xff, 0xaa, 0, 0x88, 0, 0, 0, 0x80};
    const T mask [[maybe_unused]] = static_cast<T>((bit_tab[splat - 1] * 0x101010101010101ULL));
    const T fill [[maybe_unused]] = (static_cast<T>(1U) << width) - 1;
    return mask;
}

int main(void)
{
    std::printf("%llb\n", mask_fill<uint64_t>(32, 8));

    return 0;
}