#include "definition.hpp"

namespace httpvo 
{
    struct status
    {
        enum : int {
            expect_bytes     =  -1,
            complete         =  0,
            excess           =  0,
            error            =  1,
            expect_line_feed =  2,
            unexpected_char  =  3,
            bad_whitespace   =  4,
            expected_end_of_line = 5
        };

        struct statat_t {
            int stat;
            std::size_t at;
            constexpr statat_t(int s=0, std::size_t a=0) : stat{s}, at{a} {};
            inline constexpr statat_t(const statat_t &s) : stat{s.stat}, at{s.at} {};
            
            inline statat_t& operator+=(const std::size_t _at)
            {
                at += _at;
                return *this;
            }
            
            inline operator bool(void) const
            {
                return bool(stat);
            }
        };

        int stat = 0;
        int code = 0;

        inline constexpr status(int s) : stat{-s} {}
        inline constexpr status(int s, int e) : stat{-s}, code{-e} {}

        inline bool operator==(int s) const
        {
            return stat == s;
        }

        inline bool operator==(const status &s) const
        {
            return stat == s.stat;
        }

        inline bool operator!=(int s) const
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

        inline operator int(void) const
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