#ifndef HTTPVO_REQLINE_HPP
#define HTTPVO_REQLINE_HPP
#include "definition.hpp"
#include "../common/common.hpp"

namespace httpvo
{
    /*
    * The Http/1 header line is saved in req[] such that, both positions of the version (HTTP/1.x),
    * in the request/response line is at the same index 0, of the array. Therefore, for request line, we insert decrementally from 2 - 0 and do the reverse for
    * response line (version is already the first token)
    * Only the last position of the token is stored during read. The start position and length is computed later.
    *
    *        [N]   request | response
    *        ______________|________
    *        [0]  version  | version
    *        [1]  uri      | status
    *        [2]  method   | message (optional)
    *        [3]  0        |    0
    *
    * INPUT:     GET . HTTP/1.1
    * STORED AS: HTTP/1.1 . GET
    *
    * The size of each tokens is gotten by computing:
    *    [TOKEN]        [LEN]
    *  METHOD/MGS    2 - (N - 1)   
    *  URI/STATUS    1 - (N - 1) - 1(whitespace)
    *  VERSION       0 - (N - 1) - 1(whitespace)
    * 
    * Where N is the actual start position of the tokens (GET(0) before URI(1) before VERSION(2), etc) as specified in rfc7....
    * When N - 1 < 0 (the case for first tokens, GET in request and VERSION in response line),
    * we wrap to index 3 of the array, which is always set to zero, so that the subtraction leaves the value unchanged
    */
    struct ReqLine
    {
        static constexpr u8_t m_3_2_1 = 0b111001U; // mask indices for request
        static constexpr u8_t m_1_0_3 = 0b010011U; // mask indices for response
        static constexpr u8_t required_version_size = 8;

        std::size_t req[4]{0};
        i8_t sm, st, end, i, sp, vs;

        u8_t type(void)
        {
            return sm;
        }

        inline void reset(void)
        {
            req[0] = req[1] = req[2] = 0;
            sm = st = end = i = sp = vs = 0;
        }

        inline void request(void)
        {
            req[0] = req[1] = req[2] = 0;
            sm = m_3_2_1, st = 2, end = i = -1, sp = 1, vs = 0;
        }

        inline void response(void)
        {
            req[0] = req[1] = req[2] = 0;
            sm = m_1_0_3, st = 0, end = 3, i = 1, sp = 0, vs = 0;
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

        inline bool complete(void) const
        {
            return st == end;
        }

        inline bool add_len(std::size_t len)
        {
            u8_t x = st;
            st += i;
            req[x] += len;
            return complete();
        }

        inline int minor_version(void)
        {
            return vs;
        }

        inline bool set_minor_version(u8_t i)
        {
            return (vs = i ^ '\x30') < 10;
        }

        inline bool version_is_http_1_mask(u64_t v)
        {
            return not ((v & 0x00ffffffffffffffULL) ^ 0x002e312f50545448ULL);
        }

        inline bool version_is_http_1_rd(u8_t *b)
        {
            return !((b[0] ^ '\x48') | (b[1] ^ '\x54') | (b[2] ^ '\x54') | (b[3] ^ '\x50')) and !((b[4] ^ '\x2f') | (b[5] ^ '\x31') | (b[6] ^ '\x2e'));
        }

        inline bool version_is_http_1(u8_t *b)
        {
            if (std::uintptr_t(b) & (8 - 1))
                return version_is_http_1_mask(reinterpret_cast<u64_t *>(b)[0]) and set_minor_version(b[7]);
            return version_is_http_1_rd(b) and set_minor_version(b[7]);
        }

        inline bool is_expected_version_size(void) const
        {
            return version_size() == required_version_size;
        }

        inline bool set_version(u8_t *b)
        {
            return is_expected_version_size() and version_is_http_1(b + start_of_version());
        }
    };
}

#endif // HTTPVO_REQLINE_HPP