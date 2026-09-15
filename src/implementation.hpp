#ifndef HTTPVO_IMPLEMENTAION_HPP
#define HTTPVO_IMPLEMENTATION_HPP

#include "include/definition.hpp"
#include "include/constants.hpp"
#include "include/bits.hpp"
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
        static constexpr T __size = N;
        T __used = 0;

        struct __pair
        {
            T len, pos;
        };

        struct {
            __pair name, value;
        } pair [N];

        constexpr u64_t size(void) noexcept
        {
            return __size;
        }

        u64_t used(void) const noexcept
        {
            return __used;
        }

        u64_t set_used(T i) noexcept
        {
            assert( i < __size );
            return __used = i;
        }

        auto &get(T i) const noexcept
        {
            assert( i < __size );
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

            __i    = i;
            __incr = incr;
            __max  = max;
        }


        int set(std::size_t max, std::size_t incr=1, std::size_t i=0) noexcept
        {
             if (i and (__i > max or incr > max))
                return -1;
            __incr = incr;
            __max  = max;
            __i    = i;
            return 0;
        }

        inline int set_incr(std::size_t incr) noexcept
        {
            if (incr > __max)
                return -1;
            __incr = incr;
            return 0;
        }

        inline std::size_t get_incr(void) const noexcept
        {
            return __incr;
        }

        std::size_t at(void) const noexcept
        {
            return __i;
        }

        std::size_t size(void) const noexcept
        {
            return __i;
        }

        std::size_t capacity(void) const noexcept
        {
            return __max;
        }
        
        std::size_t iszero(void) const noexcept
        {
            return __i == 0;
        }

        inline std::size_t incr_by(std::size_t incr) noexcept
        {
            assert(__i <= (__max - incr));
            return __i += incr;
        }

        inline std::size_t decr_by(std::size_t decr) noexcept
        {
            assert(__i >= decr);
            return __i -= decr;
        }

        inline std::size_t incr(void) noexcept
        {
            assert(__i <= (__max - __incr));
            return __i += __incr;
        }

        inline std::size_t decr(void) noexcept
        {
            //assert(__i >= __incr);
            return __i -= __incr;
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
        std::size_t __i;
        std::size_t __incr;
        std::size_t __max;
    };

    struct alignas(1) State
    {
        bool request_completed : 1;
        bool pending_value     : 1;
        bool trailing_ret      : 1;
        bool trailing_wsp      : 1;
        bool parse_completed   : 1;

        inline bool completed_request_line(bool x)  { return request_completed = x; }
        inline void set_pending_value(bool x)       { pending_value = x; }
        inline void set_trailing_ret(bool x)        { trailing_ret  = x; }
        inline void set_trailing_wsp(bool x)        { trailing_wsp  = x; }
        inline bool completed_request_line(void)  const { return request_completed; }
        inline bool has_pending_value(void)       const { return pending_value;     }
        inline bool has_trailing_ret(void)        const { return trailing_ret;      }
        inline bool has_trailing_wsp(void)        const { return trailing_wsp;      }
    };

    struct ReqLine
    {
        using inttype = u64_t;
        /*
            [N]   request | response
            ______________|________
            [3]  NULL     | NULL
            [2]  method   | version
            [1]  uri      | status
            [0]  version  | msg
        */
        u64_t req_line[4];
    };

    struct Reqtype
    {
        using req_index = const int (&)[];
        enum type : int {
            request  = 0,
            response = 1,
        };

        static constexpr int index[2][3] = {
            ////////////////////////////////////////////////////
            //// REQUEST {req_method, req_uri, req_version} ////
            ////////////////////////////////////////////////////
            {0, 1, 2},
            ////////////////////////////////////////////////////
            //// RESPONSE {req_version, req_stat, req_msg} /////
            ////////////////////////////////////////////////////
            {2, 1, 0},
        };
    };

    class http
    {
    public:
        http(void) : at_start_line{true} {}

        void reset(std::size_t run_size=0, std::size_t incr=0, std::size_t out_size=0)
        {
            req_type  = Reqtype::type::request; out_reader = {out_size, 1, 3};
            in_reader = {run_size, incr}; version = -1; n_bytes_to_complete = 0;
            state     = {0}; at_start_line = true;
            reqline.req_line[2] = reqline.req_line[1] = reqline.req_line[0] = 0;
        }

        int type(void)
        {
            return static_cast<int>(req_type);
        }

        int parse_header_line_sc(void *in, std::size_t in_size, std::size_t run_size);
    private:
        // header line (version, method, version, status, message)
        ReqLine reqline {0};
        // internal in & out buffer counter
        Reader in_reader {0, 64}, out_reader {0, 1, 3};
        // request type (request or response)
        Reqtype::type req_type = Reqtype::type::request;
        // http minor version (the major is tested to be 1)
        int  version {-1};
        // number of expected eop (end of parse) bytes (crlfcrlf)
        int  n_bytes_to_complete {0};
        // state
        State state {0};
        // true after reset
        bool at_start_line;

        template <typename T, T out_size, int N>
        int parse(void *in, std::size_t in_size, req<T, out_size>& out, std::size_t run_size, std::size_t rem);
        template<int N>
        int parse_request_line(void *in, std::size_t size, const simdv<N>& v, u64_t& lf, u64_t& cr, u64_t& crlf);
        template <typename T, T out_size, int N>
        int parse_header(void *in, std::size_t in_size, req<T, out_size>& out, const simdv<N>& v, u64_t lf, u64_t cr, u64_t crlf);
        template <typename T, T out_size>
        int nparse_no_rescan(void *in, std::size_t in_size, std::size_t run_size, req<T, out_size> &out);

        inline bool parse_failed(int stat)
        {
            return stat < 0;
        }

        inline int set_minor_version(u8_t i)
        {
            return (this->version = i ^ '\x30') < 10;
        }

        inline bool req_version_is_http_1(void *b)
        {
            return common::version_is_http_1(b) and set_minor_version(reinterpret_cast<u8_t *>(b)[7]);
        }

        inline u16_t req_size(ReqLine::inttype (&req)[], int i)
        {
            return this->req_type is Reqtype::type::request ?  req[i - 0] - (req[i + 1])
                                                            : (req[i - 1] - (req[i - 0]) - 1); // -1 for the sp seperator
        }

        inline bool set_version_tag(void *in)
        {
            static constexpr u8_t req_version_required_size = 8; // len(HTTP/1.x)
            auto i = Reqtype::index[req_type][0];
            bool is_correct_size = req_size(reqline.req_line, i) == req_version_required_size;
            return is_correct_size and req_version_is_http_1(reinterpret_cast<u8_t *>(in) + reqline.req_line[i + 1]);
        }

        inline int end_of_header_line(void *in, auto error)
        {
            state.completed_request_line(true);
            return -(error or set_version_tag(in) isnot http_1);
        };
    };
};
#endif //IMPLEMENTATION_HPP