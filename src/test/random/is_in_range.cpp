#include <iostream>
#include <cstdint>

int main(void)
{
    // a > 0x20 and < 0x80
    std::uint8_t a = 0x21;
    std::uint8_t b [[maybe_unused]] = 0x7f;

    std::int8_t bias = 0x80 - a; // 95
    std::int8_t range = (b - a) - 0x80; // -34

    std::cout << (int)bias << " ,s" << (int)range << std::endl;

    for (int i = 0; i < 256; i++)
    {
        if ((int)(int8_t)(i + bias) < -34)
        {
            if (i < 0x21 or i > 0x7e)
            {
                std::cout << "error" << std::endl;
                break;
            }
            std::cout << i << std::endl;
        }

        if (static_cast<int8_t>(i + 95) < -0x22)
        {
            if (i < 0x21 or i > 0x7e)
            {
                std::cout << "error" << std::endl;
                break;
            };
        }
    }
}