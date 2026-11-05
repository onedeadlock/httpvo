#include "definition.hpp"

namespace httpvo 
{
    struct status
    {
        enum : int {
            // state
            expect_bytes     = -1,
            complete         =  0,
            // error
            error            =  1,
            expect_line_feed =  2,
            unexpected_char  =  3,
            bad_whitespace   =  4,
            expected_end_of_line = 5,
            // info
            excess           = 0,
            unrecognized     = 0
        };

        struct statat_t {
            int stat;
            std::size_t offset;
            constexpr statat_t(int s=0, std::size_t o=0) : stat{s}, offset{o} {};
            constexpr operator int(void) const { return stat; }
        };

        int stat, code;
        constexpr status(const int s=0, const int e=0) : stat{-s}, code{-e} {}
        constexpr operator int(void)   const { return stat; }
        constexpr bool is_error(void)  const { return code < 0; }
        constexpr int error_code(void) const { return code; }
    };
}