#include <iostream>
#include <cstdint>

int main(void)
{
    uint64_t c = ' ' * 0x0101010101010101ULL;
    uint64_t v = 0x200020432000ULL;
    uint64_t x = v ^ c;
    uint64_t C = ~(x | ((x & 0x7f7f7f7f7f7f7f7fULL) +  0x7f7f7f7f7f7f7f7fULL)) & 0x80808080808080ULL;
    printf("%llx\n", C);
}