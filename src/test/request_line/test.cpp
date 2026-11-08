#include <iostream>
#include <cstdlib>
#include <string>
#include <string_view>
#include "../../implementation.cpp"
int main(void)
{
    httpvo::Implementation::Parse parser;
    httpvo::ReqLine req{0};

    std::cout << (int)httpvo::setup::no_multispace << std::endl;
    {
        alignas(8) char request_line[] = "GET  . HTTP/1.1\r\n";
        std::size_t len = std::strlen(request_line);
        auto status = parser.request_line<8>(reinterpret_cast<uint8_t *>(request_line), len, req);
        if (status != 0)
        {
            std::cout << status.stat << ' ' << status.offset << std::endl;
            exit(-1);
        }

        std::size_t method_size = req.req[0] - 0;
        std::string_view method{request_line, method_size};
        std::cout << method_size << std::endl;
        std::cout << method << std::endl;

        std::size_t url_size = req.req[1] - req.req[0] - 1;
        std::string_view url{request_line + req.req[0] + 1, url_size};
        std::cout << url_size << std::endl;
        std::cout << url << std::endl;
        std::cout << int(req.version) << std::endl;
    }

    {
        alignas(8) char request_line[] = "GET https://www.google.com/search?q=how+to+download+only+a+folder+from+a+github+repo&gs_lcrp=EgZjaHJvbWUqCAgBEAAYFhgeMgYIABBFGDkyCAgBEAAYFhgeMggIAhAAGBYYHjINCAMQABiGAxiABBiKBTINCAQQABiGAxiABBiKBTINCAUQABiGAxiABBiKBTIHCAYQABjvBTIHCAcQABjvBTIKCAgQABiiBBiJBdIBCTE1NDMyajBqNKgCALACAQ&sourceid=chrome&ie=UTF-8&udm=50&fbs=ACAhTvwdifjn2dKXmjtOfBxVFl_k7xB_bJXhWBZp-REbIUHBWOp3CYiB4qKOY0dV7v7979NSd1v4STFn0mDYY346TlrUAoPA9pe4zEGs7jquRZoh90W8-EzpeHhpS29m8tPJSD2nUVKCx-XC-9uF4844JQt0SA7Y_jOo-h1clATovKVqHDSuneMk-mgZgiUhTJKr2E6Ok0PdPwf6TKePjGiixb9Nt_Xo3A&aep=10&ntc=1&sxsrf=AKL5yDgovRdfzIJMZO9WLUQjdyCf2x3ZmA%3A1793737240282&mstk=AUtExfBZPTspMrJWGyybvY6DCM33okrssV9BQehVuMh3GhGqU-9wL7D6blc3FrjUJ0XMfjlDlwezk63o_AxZqAc-MHp__EPUUeLLKU-vQAcMOfKZnuEm1t2rer2gMcRflA4MBffCxERyO_RtuzqbudCXHcca7SSmnJCX5Xu3V2-7yBLYkqN3fmBtwK1LvEt6tVlLaZ9vFAohs23PMv7iRUnkIi50BpJtS2hW6bz5lWfp0vIW90mDdz-A6VQsTs3qAW_wR6G0sgsomyX72MAs_Q_vO-Ph4nxXSEJ1c41_sP_ArF6O6S45fT2w6tKFyn7bbXTSFwe1MQyyjcUVKTcKgIO6r7Epi1eokd3PZN6L761VJb2ZfdwOqdhEjiGHrv0kEgJnHj2BzFpIe536W6SQLo0xS_I5Z9h4WGAY-Q&aioh=3&csuir=1&cs=1&mtid=GELqavywMPKwhbIP8LqW2AQ HTTP/1.1\r\n";
        std::size_t len = std::strlen(request_line);
        req.reset();
        auto status = parser.request_line<32>(reinterpret_cast<uint8_t *>(request_line), len, req);
        if (status != 0)
        {
            std::cout << status.stat << ' ' << status.offset << std::endl;
            exit(-1);
        }

        std::size_t method_size = req.req[0] - 0;
        std::string_view method{request_line, method_size};
        std::cout << method_size << std::endl;
        std::cout << method << std::endl;

        std::size_t url_size = req.req[1] - req.req[0] - 1;
        std::string_view url{request_line + req.req[0] + 1, url_size};
        std::cout << url_size << std::endl;
        std::cout << url << std::endl;
        std::cout << int(req.version) << std::endl;
    }
    return 0;
}
