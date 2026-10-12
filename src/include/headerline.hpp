#include "definition.hpp"

namespace httpvo
{
    struct header_view
    {
        struct
        {
            std::size_t pos;
            std::size_t end;
            inline int commit(const u8_t *const b)
            {
                while (pos != end and common::is_whitespace(b[pos++]))
                    pass();
                while (pos != end and common::is_whitespace(b[end--]))
                    pass();
                return pos == end;
            }
        } name, value;
    };
}