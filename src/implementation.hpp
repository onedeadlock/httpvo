#ifndef HTTPVO_IMPLEMENTATION_HPP
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
        static constexpr u8_t m_3_2_1 = 0b111001U;
        static constexpr u8_t m_1_0_3 = 0b010011U;

        /* request line is splitted and saved in the manner below:
            [N]   request | response
            ______________|________
            [0]  0        |    0
            [1]  version  | version
            [2]  uri      | status
            [3]  method   | message (optional)
        */
   
        std::size_t req[4]{0};
        i8_t sm, st, end, i, sp;
   
        u8_t type(void)
        {
            return sm;
        }

        inline void reset(void)
        {
            req[1] = req[2] = req[3] = 0;
            sm = st = end = i = sp = 0;
        }

        inline void request(void)
        {
            req[1] = req[2] = req[3] = 0;
            sm = m_3_2_1, st = 2, end = i = -1, sp = 1;
        }

        inline void response(void)
        {
            req[1] = req[2] = req[3] = 0;
            sm = m_1_0_3, st = 0, end = 3, i = 1, sp = 0;
        }

        inline std::size_t& next(void)
        {
            return req[st += i];
        }

        inline std::size_t& post(void)
        {
            const u8_t x = st;
            st += i;
            return req[x];
        }

        inline u8_t at(void) const
        {
            return st;
        }

        inline bool complete(void) const
        {
            return st == end;
        }

        inline std::size_t start_of_version(void) const 
        {
            return req[sm & 0b11] + sp; // +1 for sp (request:version)
        }

        inline std::size_t version_size(void) const 
        {
            return req[0] - start_of_version();
        }

        inline std::size_t start_of_status_uri(void) const
        {
            return req[(sm >> 2) & 0b11] + 1; // +1 for sp
        }

        inline std::size_t status_uri_size(void) const 
        {
            return req[1] - start_of_status_uri();
        }

        inline std::size_t start_of_method_msg(void) const 
        {
            return req[(sm >> 4) & 0b11] + !sp; // +1 for sp (msg)
        }
        
        inline std::size_t method_or_msg_size(void) const
        {
            return req[2] - start_of_method_msg();
        }
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

        int parse_header_line_sc(void *in, std::size_t in_size, std::size_t run_size);
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

        inline bool set_version_tag(void *in)
        {
            static constexpr u8_t req_version_required_size = 8; // len(HTTP/1.x)
            bool is_correct_size = reqline.version_size() == req_version_required_size;
            return is_correct_size and req_version_is_http_1(reinterpret_cast<u8_t *>(in) + reqline.start_of_version());
        }

        inline int end_of_header_line(void *in, auto error)
        {
            state.completed_request_line(true);
            return -(error or set_version_tag(in) isnot http_1);
        };
    };
};
#endif //IMPLEMENTATION_HPP