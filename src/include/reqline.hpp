#ifndef HTTPVO_REQLINE_HPP
#define HTTPVO_REQLINE_HPP
#include "definition.hpp"
#include "../simd/implementation.hpp"

namespace httpvo
{
    struct ReqLine
    {
        std::size_t req[3]{0};
        i8_t i=0, version=0;

        inline void reset(void)
        {
            req[0] = req[1] = i = version = 0;
        }

        inline int advance(const std::size_t len)
        {
            req[i] += len;
            return ++i == 2;
        }

        inline int minor_version(void)
        {
            return version;
        }
}

#endif // HTTPVO_REQLINE_HPP