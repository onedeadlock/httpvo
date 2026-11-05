#ifndef HTTPVO_REQLINE_HPP
#define HTTPVO_REQLINE_HPP
#include "definition.hpp"
#include "../simd/implementation.hpp"

namespace httpvo
{
    struct ReqLine
    {
        std::size_t req[3]{0};
        i8_t i=0;
        i8_t vs=0;
        i8_t meth, url;

        inline void reset(void)
        {
            req[0] = req[1] = i = vs = 0;
        }

        inline int advance(const std::size_t len)
        {
            req[i] += len;
            return ++i == 2;
        }

        inline int minor_version(void)
        {
            return vs;
        }

        inline int set_minor_version(const u8_t i)
        {
            return (vs = i ^ 0x30) < 10;
        }

        inline bool version_is_http_1(const u8_t * const b) const
        {
            if constexpr (setup::little_endian && !setup::no_vectorize)
                return simd::simdv<8>::testmask_u64(b, 0x00ffffffffffffffULL, 0x002e312f50545448ULL);
            else
            {
#               define USH(c, n) (static_cast<const u32_t>(c) << (n))
                // GCC and clang fold these shifts to two loads (and bswap on big endian) so that the test is always correct.
                const u32_t LDLO = USH(b[0], 0) | USH(b[1], 8) | USH(b[2], 16) | USH((b[3], 24);
                const u32_t LDHI = USH(b[3], 0) | USH(b[4], 8) | USH(b[5], 16) | USH((b[6], 24); // added byte 4 of @LDLO to make it 4 bytes
                const u32_t is_not_http      = LDLO ^ 0x50545448U;
                const u32_t is_not_slash1dot = LDHI ^ 0x2e312f50U;
                return (is_not_http | is_not_slash1dot) == 0;
#               undef USH
            }
        }

        inline int set_version(const u8_t * const b)
        {
            // :: assume len(b) >= 8
            return version_is_http_1(b) & set_minor_version(b[7]);
        }
    };
}

#endif // HTTPVO_REQLINE_HPP