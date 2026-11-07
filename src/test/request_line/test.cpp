#include <iostream>
#include <cstdlib>
#include "../../implementation.cpp"
int main(void)
{
    alignas(8) char request_line[] = "GET . HTTP/1.1\r\n";
    std::size_t len = std::strlen(request_line);
    httpvo::Implementation::Parse parser;
    httpvo::ReqLine req{0};

    auto status = parser.request_line<32>(reinterpret_cast<uint8_t *>(request_line), len, req);
    if (status != 0)
    {
        std::cout << status.stat << ' ' << status.offset << std::endl;
        exit(-1);
    }
    return 0;
}
