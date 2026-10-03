#include "definition.hpp"

namespace httpvo
{
    struct header_view
    {
        struct
        {
            std::size_t pos;
            std::size_t end;
        } name, value;
    };
}