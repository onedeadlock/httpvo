#include <iostream>
#include "immintrin.h"

int main(void)
{
    alignas(16) unsigned char b[16]{0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0};

    __m128i v = _mm_load_si128(reinterpret_cast<__m128i *>(b));
    __m128i shf = _mm_srli_epi64(v, 4);
    _mm_store_si128(reinterpret_cast<__m128i *>(b), shf);

    std::cout << ((int)b[0] == 0xf) << std::endl;
}