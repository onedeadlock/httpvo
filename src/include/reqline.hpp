#ifndef HTTPVO_REQLINE_HPP
#define HTTPVO_REQLINE_HPP
#include "definition.hpp"

namespace httpvo
{
    struct ReqLine
    {
        std::size_t req[2];
        u8_t i, version;


        inline void reset(void)
        {
            req[0] = req[1] = i = version = 0;
        }

        inline std::size_t advance(const std::size_t len)
        {
             if HTTPVO_CXX17_CONSTEXPR (setup::debug)
                assert(1); // TODO
            req[i++] += len;
            return len;
        }

        inline int minor_version(void)
        {
            return version;
        }
    };
}

#endif // HTTPVO_REQLINE_HPP