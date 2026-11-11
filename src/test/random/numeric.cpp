#include <limits>
#include <iostream>

int main(void)
{
    std::cout << (std::numeric_limits<int>::digits + 1) / 8 << std::endl;
}