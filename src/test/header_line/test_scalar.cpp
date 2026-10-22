#include <iostream>
#include "../../scalar_impl.hpp"
int main(void)
{
    alignas(16) char header[] = "username: michael saviour\r\n\r\npppp";
    std::size_t len = std::strlen(header);
    httpvo::Implementation::http parser;
    httpvo::header_view view {0};
    
    httpvo::status s = parser.parse_header<0>((httpvo::u8_t *)header, (httpvo::u8_t *)header, view, len, len);
   
    if (s < 0)
    {
        std::cout << "error " << (int)s.stat << ' ' << (int)s.code << std::endl;
        return 1;
    }

    std::cout << (int)s.stat << std::endl;
    std::cout << view.name_pos  << ' ' << view.name_end  << std::endl;
    std::cout << view.value_pos << ' ' << view.value_end << std::endl;
    return 0;
}