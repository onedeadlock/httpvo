#ifndef HTTPVO_IMPLEMENTATION_HPP
#define HTTPVO_IMPLEMENTATION_HPP

#include "include/definition.hpp"
#include "include/constants.hpp"
#include "include/bits.hpp"
#include "include/reqline.hpp"
#include "common/common.hpp"
//#include "simd/implementation.hpp"

namespace httpvo::Implementation
{
    constexpr int COMPLETE = 0;
    constexpr int EXPECT_DATA = 1;

    bool http_1 = true;
    bool done   = true;

    template <int N> struct simdv;

    template <typename T, T N>
    struct req
    {
        static_assert(std::is_integral_v<T> and (sizeof(T) < sizeof(u64_t)) and N > 0);
        static constexpr T _size = N;
        T _used = 0;

        struct _pair
        {
            T len, pos;
        };

        struct {
            _pair name, value;
        } pair [N];

        constexpr u64_t size(void) noexcept
        {
            return _size;
        }

        u64_t used(void) const noexcept
        {
            return _used;
        }

        u64_t set_used(T i) noexcept
        {
            assert( i < _size );
            return _used = i;
        }

        auto &get(T i) const noexcept
        {
            assert( i < _size );
            return pair[i];
        }

        auto &operator[](T i) noexcept
        {
            return pair[i];
        }
    };

    struct Reader {
        Reader(std::size_t max=std::numeric_limits<std::size_t>::max()-1, std::size_t incr=1, std::size_t i=0) noexcept
        {
            assert (max  < std::numeric_limits<std::size_t>::max());
            assert (incr < std::numeric_limits<std::size_t>::max());
            assert (i    < std::numeric_limits<std::size_t>::max());

            _i    = i;
            _incr = incr;
            _max  = max;
        }


        int set(std::size_t max, std::size_t incr=1, std::size_t i=0) noexcept
        {
             if (i and (_i > max or incr > max))
                return -1;
            _incr = incr;
            _max  = max;
            _i    = i;
            return 0;
        }

        inline int set_incr(std::size_t incr) noexcept
        {
            if (incr > _max)
                return -1;
            _incr = incr;
            return 0;
        }

        inline std::size_t get_incr(void) const noexcept
        {
            return _incr;
        }

        std::size_t at(void) const noexcept
        {
            return _i;
        }

        std::size_t size(void) const noexcept
        {
            return _i;
        }

        std::size_t capacity(void) const noexcept
        {
            return _max;
        }
        
        std::size_t iszero(void) const noexcept
        {
            return _i == 0;
        }

        inline std::size_t incr_by(std::size_t incr) noexcept
        {
            assert(_i <= (_max - incr));
            return _i += incr;
        }

        inline std::size_t decr_by(std::size_t decr) noexcept
        {
            assert(_i >= decr);
            return _i -= decr;
        }

        inline std::size_t incr(void) noexcept
        {
            assert(_i <= (_max - _incr));
            return _i += _incr;
        }

        inline std::size_t decr(void) noexcept
        {
            assert(_i >= _incr);
            return _i -= _incr;
        }

        inline std::size_t operator++(void)
        {
            return incr();
        }

        inline std::size_t operator--(void)
        {
            return decr();
        }

        private:
        std::size_t _i;
        std::size_t _incr;
        std::size_t _max;
    };

    struct alignas(1) State
    {
        bool request_completed : 1;
        bool pending_value     : 1;
        bool trailing_ret      : 1;
        bool trailing_wsp      : 1;
        bool parse_completed   : 1;

        inline bool completed_request_line(bool x)  { return request_completed = x; }
        inline bool set_pending_value(bool x)       { pending_value = x; return 0;  }
        inline bool set_trailing_ret(bool x)        { trailing_ret  = x; return 0;  }
        inline bool set_trailing_wsp(bool x)        { trailing_wsp  = x; return 0;  }
        inline bool completed_request_line(void)  const { return request_completed; }
        inline bool has_pending_value(void)       const { return pending_value;     }
        inline bool has_trailing_ret(void)        const { return trailing_ret;      }
        inline bool has_trailing_wsp(void)        const { return trailing_wsp;      }
    };

    class http
    {
    public:
        http(void) : reqline{0}, at_start_line{true} {}

        void reset(std::size_t run_size=0, std::size_t incr=0, std::size_t out_size=0)
        {
            out_reader = {out_size, 1, 3}; in_reader = {run_size, incr}; version = -1;
            n_bytes_to_complete = 0; state = {0}; at_start_line = true;
            reqline.reset();
        }

        int scparse_header_line(u8_t *, ReqLine&, std::size_t, std::size_t);
    private:
        // header line (version, method, version, status, message)
        ReqLine reqline;
        // internal in & out buffer counter
        Reader in_reader{0, 64}, out_reader{0, 1, 3};
        // minor version
        int  version{-1};
        // number of expected eop (end of parse) bytes (crlfcrlf)
        int  n_bytes_to_complete{0};
        State state{0};
        bool at_start_line;

        template <typename T, T out_size, int N>
        int parse(void *, std::size_t, req<T, out_size>&, std::size_t, std::size_t);
        template<int N>
        int parse_request_line(void *, std::size_t, const simdv<N>&, u64_t&, u64_t&, u64_t&);
        template <typename T, T out_size, int N>
        int parse_header(void *, std::size_t, req<T, out_size>&, const simdv<N>&, u64_t, u64_t, u64_t);
        template <typename T, T out_size>
        int nparse_no_rescan(void *, std::size_t, std::size_t, req<T, out_size> &);

        inline bool parse_failed(int stat)
        {
            return stat < 0;
        }
    };
};
#endif //IMPLEMENTATION_HPP