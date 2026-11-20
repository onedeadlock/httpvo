#include <cstddef>
#include <iostream>
using u64_t = int;

struct Mask
{
    u64_t mask;
    inline size_t first(const void * const b = nullptr)
    {
        if (mask) return 0;
        return 1;
    }

    inline size_t to_bitmask(void)
    {
        return mask;
    }
};

template <class T = Mask>
class Offset : public T
{
    public:
    constexpr Offset(u64_t m=0) {}
};

Offse<> Off(void)
{
    return 0;
}

int main(void)
{
    Mask x;
    std::cout << sizeof x << std::endl;
    std::cout << Off().first() << std::endl;
    return x.first();
}