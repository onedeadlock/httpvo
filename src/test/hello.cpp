#include <iostream>
#include <vector>
#include <cstdio>

int main(void)
{


    std::vector<int> x {0, 1, 3, 4, 4, 5, 5};

    for (int i : x)
        std::cout << i << std::endl
        ;
    return 0;
}