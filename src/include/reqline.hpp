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
                return (simd::simdv<8>::_load_u64(b) << 8) == 0x2e312f5054544800ULL;
            else
            {
#               define USH(c, n) (static_cast<u32_t>(c) << (n))
                // GCC and clang fold these shifts to a single load (and bswap on big endian). It is not the case for MSVC, but the cost of byte compares is the same.
                // @discard_reginfo: clang sees that byte 7 is unused, discards it, and so emit 3 loads (first 4 bytes, next two bytes and the last byte) instead of 1 for the 8 bytes.
                u64_t LD = 0;
                LD |= static_cast<u64_t>(USH(b[0], 0U) | USH(b[1], 8U) | USH(b[2], 16) | USH(b[3], 24));
                LD |= static_cast<u64_t>(USH(b[4], 32) | USH(b[5], 40) | USH(b[6], 48) | USH(b[7], 56));
                httpvo_discard_reginfo(LD);
                return (LD << 8) == 0x2e312f5054544800ULL;
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