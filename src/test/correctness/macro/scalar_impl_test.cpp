#include <iostream>
#include <cstring>
#include "../../../scalar_impl.hpp"

#ifndef PUT
#define PUT(i) std::cout << (i) << std::endl
#endif

using namespace dhttp::Implementation;

static char request[] = "GET https://google.com/index HTTP1.1\r\n\r\n";

int main(void)
{
    http test;

    std::cout << test.type() << std::endl;

    std::size_t len = std::strlen(request);
int i = test.parse_header_line_sc(request, len, len);
   
    PUTI(i);
    return 0;
}