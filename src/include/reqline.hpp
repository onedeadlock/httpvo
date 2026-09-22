#ifndef HTTPVO_REQLINE_HPP
#define HTTPVO_REQLINE_HPP
#include "definition.hpp"

namespace httpvo
{
    /*
    * The Http/1 header line is saved in req[] such that, both positions of the version (HTTP/1.x),
    * in the request/response line is at the same index 0, in the array (same for the rest at index 1 and 2).
    * To arrange this, for request line, we insert decrementally from 3 (0 - 2) and do the reverse for
    * response line, since it's index version is already the first token to be read.
    * that is: 
    *
    *        [N]   request | response
    *        ______________|________
    *        [0]  version  | version
    *        [1]  uri      | status
    *        [2]  method   | message (optional)
    *        [3]  0        |    0
    *
    * INPUT: (GET . HTTP/1.1) - STORED: (HTTP/1.1 . GET)
    *    V            LEN
    * (METHOD)  =   N - (N - 1)   
    * (URI)     =  (N - (N - 1)) - 1(whitespace)
    * (VERSION) =  (N - (N - 1)) - 1(whitespace)
    * The above is certain, and independent of the order of our arrangement in the array, req.
    * In the case of 'method' in request and 'version' in response line where N - 1 is 0,
    * the last slot [3], is assigned 0, and never changes, so that we can wrap N - 1 to 3 and safely
    * read req[3] as 0
    */
 struct ReqLine
    {
        static constexpr u8_t m_3_2_1 = 0b111001U; // mask indices for request
        static constexpr u8_t m_1_0_3 = 0b010011U; // mask indices for response
   
        std::size_t req[4]{0};
        i8_t sm, st, end, i, sp;
   
        u8_t type(void)
        {
            return sm;
        }

        inline void reset(void)
        {
            req[0] = req[1] = req[2] = 0;
            sm = st = end = i = sp = 0;
        }

        inline void request(void)
        {
            req[0] = req[1] = req[2] = 0;
            sm = m_3_2_1, st = 2, end = i = -1, sp = 1;
        }

        inline void response(void)
        {
            req[0] = req[1] = req[2] = 0;
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

        inline void add(std::size_t len)
        {
            const u8_t x = st;
            st += i;
            req[x] += len;
        }

        inline bool add_len(std::size_t len)
        {
            u8_t x = st;
            st += i;
            req[x] += len;
            return complete();
        }

        inline u8_t at(void) const
        {
            return st;
        }

        inline bool complete(void) const
        {
            return st == end;
        }

        inline bool incomplete(void) const
        {
            return st != end;
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
}

#endif // HTTPVO_REQLINE