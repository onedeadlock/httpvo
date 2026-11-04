#ifndef HTTPVO_CONSTANTS_HPP
#define HTTPVO_CONSTANTS_HPP
#include "definition.hpp"

namespace httpvo::constant
{
    static constexpr u64_t c01 = 0x0101010101010101ULL;
    static constexpr u64_t c09 = 0x0909090909090909ULL;
    static constexpr u64_t c0a = 0x0a0a0a0a0a0a0a0aULL;
    static constexpr u64_t c0d = 0x0d0d0d0d0d0d0d0dULL;
    static constexpr u64_t c20 = 0x2020202020202020ULL;
    static constexpr u64_t c30 = 0x3030303030303030ULL;
    static constexpr u64_t c3a = 0x3a3a3a3a3a3a3a3aULL;
    static constexpr u64_t c7f = 0x7f7f7f7f7f7f7f7fULL;
    static constexpr u64_t c9f = 0x9f9f9f9f9f9f9f9fULL;
    static constexpr u64_t ca0 = 0xa0a0a0a0a0a0a0a0ULL;
    static constexpr u64_t cdf = 0xdfdfdfdfdfdfdfdfULL;
    static constexpr u64_t cff = 0xffffffffffffffffULL;
    static constexpr u64_t c80 = 0x8080808080808080ULL;

    static constexpr u64_t compress = 0x0002040810204081ULL;

    static constexpr u64_t DeBruijn64_const = 0x03f79d71b4cb0a89ULL;

    static constexpr u8_t DeBruijn64_seq[64]{
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