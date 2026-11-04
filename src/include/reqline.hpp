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
                const bool is_not_http          = (b[0] ^ 0x48) | (b[1] ^ 0x54) | (b[2] ^ 0x54) | (b[3] ^ 0x50);
                const bool is_not_slash_one_dot = (b[4] ^ 0x2f) | (b[5] ^ 0x31) | (b[6] ^ 0x2e);
                return not (is_not_http | is_not_slash_one_dot);
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