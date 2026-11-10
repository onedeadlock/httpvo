#ifndef HTTPVO_IMPLEMENTATION_HPP
#define HTTPVO_IMPLEMENTATION_HPP

#include "include/definition.hpp"
#include "include/status.hpp"
#include "include/constants.hpp"
#include "include/bits.hpp"
#include "include/reqline.hpp"
#include "include/headerline.hpp"
#include "simd/implementation.hpp"
#if   HTTPVO_HAVE__AVX2__
#    include "simd/haswell/implementation.hpp"
#endif
#if HTTPVO_HAVE__SSE4_2__
#    include "simd/westmere/implementation.hpp"
#endif
#if HTTPVO_HAVE__ARM_NEON__
#    include "simd/neon/implementation.hpp"
#endif

namespace httpvo::Implementation
{
    template<int>
    struct Protocol;

    using Rtsp_Protocol = Protocol<0>;
    using Http_Protocol = Protocol<1>;


    class Parse
    {
    public:
        Parse(void) : at_start_line{0}, end_of_parse_marker{0} {}
        template <int N, class Protocol>
        status::statat_t request_line(const u8_t *,  const std::size_t, ReqLine&) noexcept;
        template<int N, class Protocol>
        status::statat_t response_line(const u8_t *, const std::size_t, ReqLine&) noexcept;

        template<int N>
        status header(u8_t *, header_view&, const std::size_t) noexcept;
        private:
        bool at_start_line;
        bool end_of_parse_marker;
        int  end_of_parse(const u32_t v) noexcept;
        int  end_of_parse_single_load(const u8_t * const b) noexcept;
        int  is_not_end_of_line(const u8_t * const, const std::size_t) noexcept;
    };
}
#endif //IMPLEMENTATION_HPPc