#include <iostream>
#include <cstdint>

int main(void)
{
    uint64_t C = 0x80808000000000UL;
    std::cout << (__builtin_ctzll(C) / 8) << std::endl;
}