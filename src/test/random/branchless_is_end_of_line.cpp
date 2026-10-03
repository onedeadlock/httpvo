#include <cstdint>
#include <iostream>
#include "../../common/common.hpp"

int main(void)
{
    
    static constexpr uint16_t alignas(8) end[4]{0x0d0a0d, 0x0a0d};
    //static constexpr uint8_t  alignas(8) eop_tab[8]{0, 2, 1, 0, 3, 0, 2, 1};
    alignas(8) static constexpr uint8_t eop_tal[8]{0, 3, 2, 2, 1, 1, 1, 1};

    // 0001
    uint32_t v = 0b0001; // cr

    uint32_t i = __builtin_ctzl(v) + 1;

    uint32_t p = v >> i;

    uint32_t n = eop_tal[p];

    if (0 < eop_tab[n])
    {
        return -(v ^ 0x0d) - (v ^ 0x0a00);
    }

    uint32_t d = v ^ 0x0a0d0a0d;
    bool end   = v & 0xff;
    bool parse = v & 0xff00;

    n = p ^ end[p];

    std::cout << n << std::endl;

    //uint32_t n = end[0];
   // uint32_t x = v >> 0;
    //uint32_t maybe_end_of_line = x ^ (n & 0xff);
    //uint32_t maybe_end_of_parse = x ^ n;
}