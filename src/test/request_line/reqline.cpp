#include <iostream>
#include "../../scalar_impl.hpp"
int main(void)
{
    alignas(8) char request[] = "GET ./svindex HTTP/1.1\r\n";
    std::size_t len = std::strlen(request);
    httpvo::Implementation::http parser;
    httpvo::ReqLine req{0};

    req.request();

    int stat = parser.scparse_header_line((httpvo::u8_t *)request, req, len, len);
    std::cout << stat << std::endl;
    if (stat < 0)
    {
        std::cerr << "parsing error" << std::endl;
    }
    std::cout << req.req[0] << std::endl;
    std::cout << int(req.st) << std::endl;
    if (std::strncmp(request + req.start_of_version(), "HTTP/1.1", req.version_size()) != 0)
    {
        std::cerr << "parsing version failed\n";
        return -1;
    }
        return 0;
    std::cout << request + req.start_of_method_msg() << std::endl;
    std::cout << request + req.start_of_status_uri() << std::endl;
    std::cout << request + req.start_of_version() << std::endl;
    std::cout << stat << std::endl;
    return 0;
}
