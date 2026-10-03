#include "definition.hpp"

namespace httpvo 
{
    struct status
    {
        enum : i8_t {
            expect_bytes    = 1,
            complete        = 0,
            error           = -1,
            overrun_error   = -1,
            expect_line_feed = -2,
            unexpected_char     = -3,
            bad_whitespace = -4
        };

        i8_t stat = 0;
        i8_t code = 0;

        inline constexpr status(i8_t s) : stat{s} {}
        inline constexpr status(i8_t s, i8_t e) : stat{s}, code{e} {}

        inline bool operator==(i8_t s) const
        {
            return stat == s;
        }

        inline bool operator==(const status &s) const
        {
            return stat == s.stat;
        }

        inline bool operator!=(i8_t s) const
        {
            return stat != s;
        }

        inline bool operator!=(const status &s) const
        {
            return stat != s.stat;
        }

        inline bool operator not(void) const
        {
            return not stat;
        }

        inline bool operator>(int s) const
        {
            return stat > s;
        }

        inline bool operator<(int s) const
        {
            return stat < s;
        }

        inline operator i8_t(void) const
        {
            return stat;
        }

        inline bool is_error(void) const
        {
            return code < 0;
        }

        inline int error_code(void) const
        {
            return code;
        }
    };
}