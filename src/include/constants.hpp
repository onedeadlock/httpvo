#ifndef HTTPVO_CONSTANTS_HPP
#define HTTPVO_CONSTANTS_HPP
#include "definition.hpp"

namespace httpvo::constant
{
    constexpr INT c01 = INT(0x0101010101010101ULL);
    constexpr INT c09 = INT(0x0909090909090909ULL);
    constexpr INT c0a = INT(0x0a0a0a0a0a0a0a0aULL);
    constexpr INT c0d = INT(0x0d0d0d0d0d0d0d0dULL);
    constexpr INT c20 = INT(0x2020202020202020ULL);
    constexpr INT c30 = INT(0x3030303030303030ULL);
    constexpr INT c3a = INT(0x3a3a3a3a3a3a3a3aULL);
    constexpr INT c7f = INT(0x7f7f7f7f7f7f7f7fULL);
    constexpr INT c9f = INT(0x9f9f9f9f9f9f9f9fULL);
    constexpr INT ca0 = INT(0xa0a0a0a0a0a0a0a0ULL);
    constexpr INT cdf = INT(0xdfdfdfdfdfdfdfdfULL);
    constexpr INT cff = INT(0xffffffffffffffffULL);
    constexpr INT c80 = INT(0x8080808080808080ULL);

    constexpr u64_t compress = 0x0002040810204081ULL;

    constexpr u32_t DeBruijn32_const = 0x077cb531U;
    constexpr u64_t DeBruijn64_const = 0x03f79d71b4cb0a89ULL;

    constexpr u8_t  DeBruijn32_seq[32]{
        0,  1,  28, 2,  29, 14, 24, 3,
        30, 22, 20, 15, 25, 17, 4,  8,
        31, 27, 13, 23, 21, 19, 16, 7,
        26, 12, 18, 6,  11, 5,  10, 9};

    constexpr u8_t DeBruijn64_seq[64]{
        0,  47, 1,  56, 48, 27, 2,  60,
        57, 49, 41, 37, 28, 16, 3,  61,
        54, 58, 35, 52, 50, 42, 21, 44,
        38, 32, 29, 23, 17, 11, 4,  62,
        46, 55, 26, 59, 40, 36, 15, 53,
        34, 51, 20, 43, 31, 22, 10, 45,
        25, 39, 14, 33, 19, 30, 9,  24,
        13, 18, 8,  12, 7,  6,  5,  63};
}
#endif // HTTPVO_CONSTANTS_HPP