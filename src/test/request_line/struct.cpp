#include <string>
#include <iostream>
#include "../../implementation.hpp"

using namespace httpvo;

void test(void)
{
    std::string request  = "GET ./index HTTP/1.1 ";
    std::string response = "HTTP/1/1 200 OK ";

    Implementation::ReqLine x;
    x.request();

    std::size_t i = 0;
    for (auto c : request)
    {
        if (common::is_whitespace(c))
        {
            //x.post() += i;
        }
        i++;
    }

    std::cout << ((x._sm >> 0) & 0b11) << std::endl;
    std::cout << (int)(x._st) << std::endl;
    std::cout << x.version_start() << " " << x.version_size() << std::endl;
}

int main(void)
{
    test();
    return 0;
}