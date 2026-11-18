#include "definition.hpp"

namespace httpvo 
{
    struct status
    {
        enum : int {
            // state
            expect_bytes     =  1,
            complete         =  0,
            // error
            unexpected_error = -1,
            expect_line_feed = -2,
            unexpected_char  = -3,
            bad_whitespace   = -4,
            expected_end_of_line = -5,
            // info
            excess           =  0,
            unrecognized     =  0
        };

        int stat;
        std::size_t offset;
        constexpr status(const int s=0, const size_t o=0) : stat{s}, offset{o} {}
        constexpr operator int(void) const { return stat; }

        static status error_mutispace(const u8_t * const e, const u8_t * const * s)
        {
            const std::size_t offset = static_cast<std::size_t>(e - s);
            if HTTPVO_CXX17_CONSTEXPR (setup::no_multispace)
                return  {status::bad_whitespace, offset};
            else return {status::expect_bytes,   offset};
        }
    
       static status error(const int status, const std::size_t offset)
       {
           return {status, offset};
       }

       static status error(const int status, const u8_t * const e, const u8_t * const s)
       {
           return {status, static_cast<std::size_t>(e - s)};
       }
    };
}