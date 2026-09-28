#include <iostream>
#include "../../scalar_impl.hpp"
int main(void)
{
    alignas(8) char request[] = "GET . HTTP/1.1\r\n";
    std::size_t len = std::strlen(request);
    httpvo::Implementation::http parser;
    httpvo::ReqLine req{0};

    req.request();

    httpvo::status stat = parser.parse_line<0>((httpvo::u8_t *)request, (httpvo::u8_t *)request, req, len, httpvo::constant::cff, 0);

    if (stat < 0)
    {
        std::cerr << stat.error_code() << " " << stat.stat << " parsing error" << std::endl;
        return -1;
    }

    if (std::strncmp(request + req.start_of_method_msg(), "GET", req.method_or_msg_size()) != 0)
    {
        std::cerr << "parsing method failed\n";
        return -1;
    }

    if (std::strncmp(request + req.start_of_status_uri(), ".", req.status_uri_size()) != 0)
    {
        std::cerr << "parsing uri failed\n";
        return -1;
    }

    if (std::strncmp(request + req.start_of_version(), "HTTP/1.1", req.version_size()) != 0)
    {
        std::cerr << "parsing version failed\n";
        return -1;
    }
    
    std::cout << "PASSED" << std::endl;
    return 0;
}
