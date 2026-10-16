#include <iostream>
#include "../../scalar_impl.hpp"
int main(void)
{
    alignas(16) char header[] = "username: michael saviour\r\n\r\npppp";
    std::size_t len = std::strlen(header);
    httpvo::Implementation::http parser;
    httpvo::header_view view {0};
    
    httpvo::status s = parser.parse_header<16>((httpvo::u8_t *)header, (httpvo::u8_t *)header, view, len, len, 0);
   
    if (s < 0)
    {
        std::cout << "error " << (int)s.stat << ' ' << (int)s.code << std::endl;
        return 1;
    }

    std::cout << header[11] << std::endl;
    std::cout << (int)s.stat << std::endl;
    std::cout << view.name.pos  << ' ' << view.name.end  << std::endl;
    std::cout << view.value.pos << ' ' << view.value.end << std::endl;
    return 0;
}