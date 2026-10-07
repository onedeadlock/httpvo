#ifndef HTTPVO_IMPLEMENTATION_HPP
#define HTTPVO_IMPLEMENTATION_HPP

#include "include/definition.hpp"
#include "include/status.hpp"
#include "include/constants.hpp"
#include "include/bits.hpp"
#include "include/reqline.hpp"
#include "include/headerline.hpp"
#include "common/common.hpp"

#if   HTTPVO_HAVE__AVX2__
#    include "simd/haswell/implementation.hpp"
#endif
#if HTTPVO_HAVE__SSE4_2__
#    include "simd/westmere/implementation.hpp"
#endif
#if HTTPVO_HAVE__ARM_NEON__
#    include "simd/neon/implementation.hpp"
#endif
#include "simd/implementation.hpp"

namespace httpvo::Implementation
{
    class http
    {
    public:
        http(void) {}
        template <simd::VWidth N, bool ISTRAIL>
        status parse_line(u8_t *, u8_t *, ReqLine &, const std::size_t, const simd::mask_t, u64_t);
        template <simd::VWidth N, bool ISTRAIL>
        inline status parse_header(u8_t *, u8_t *, header_view&, const std::size_t, const std::size_t, const std::size_t);
        private:
        int n_bytes_to_complete = 0; // number of expected eop (end of parse) bytes (crlfcrlf)
        bool at_start_line = true;
    };
}
#endif //IMPLEMENTATION_HPP