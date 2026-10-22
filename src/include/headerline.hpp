#include "definition.hpp"

namespace httpvo
{
    struct header_view
    {
        std::size_t name_pos=0, name_end;
        std::size_t value_pos,  value_end;
        inline void set_name_end(const std::size_t end, const u8_t *const b [[maybe_unused]])
        {
            name_end  = end;
            value_pos = end + 1;
        }
        inline int set_value_end(const std::size_t end, const u8_t *const b)
        {
            value_end = end;
            for (; value_pos != value_end and common::is_whitespace(b[value_pos]); value_pos++)
                pass();
            for (; value_pos != value_end and common::is_whitespace(b[value_end]); value_end--)
                pass();
            return -(value_pos == value_end);
        }
    };
}