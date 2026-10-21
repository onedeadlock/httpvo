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
                for (; pos != end and common::is_whitespace(b[pos]); pos++) pass();
                for (; pos != end and common::is_whitespace(b[end]); end--) pass();
                return -(pos == end);
            }
        } name, value;
    };
}