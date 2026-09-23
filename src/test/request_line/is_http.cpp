#include <iostream>
#include <cstdint>
#include "../../scalar_impl.hpp"

int main(void)
{
    alignas(8) char b[] = "HTTP/1.1";
    std::uint64_t is_http_mask = (*reinterpret_cast<std::uint64_t *>(b) & 0xffffffffffffffULL) == 0x002e312f50545448ULL;
    std::uint64_t is_http_rd = !((b[0] ^ '\x48') | (b[1] ^ '\x54') | (b[2] ^ '\x54') | (b[3] ^ '\x50')) and !((b[4] ^ '\x2f') | (b[5] ^ '\x31') | (b[6] ^ '\x2e'));
    
    // LITTLE ENDIAN
    std::cout << is_http_mask << "\n";
    std::cout << is_http_rd   << "\n";

    httpvo::ReqLine req;
    std::cout << req.version_is_http_1_mask(*reinterpret_cast<std::uint64_t *>(b)) << "\n";
    std::cout << req.version_is_http_1_rd(reinterpret_cast<httpvo::u8_t *>(b)) << "\n";
    return 0;
}