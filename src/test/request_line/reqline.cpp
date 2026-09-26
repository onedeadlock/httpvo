#include <iostream>
#include "../../scalar_impl.hpp"
int main(void)
{
    alignas(8) char request[] = "GET . HTTP/1.1\r\n";
    std::size_t len = std::strlen(request);
    httpvo::Implementation::http parser;
    httpvo::ReqLine req{0};

    req.request();

    httpvo::Implementation::_Status stat = parser.scparse_header_line<8>((httpvo::u8_t *)request, req, len , 0, httpvo::constant::cff, 0);

    if (stat < 0)
    {
        std::cerr << stat.error_code() << " " << stat.status << " parsing error" << std::endl;
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
    
    return 0;
}
