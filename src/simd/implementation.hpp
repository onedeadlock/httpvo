#ifndef HTTPVO_SIMD_IMPLEMENTAION_HPP
#define HTTPVO_SIMD_IMPLEMENTAION_HPP
#include "./../include/definition.hpp"

#if HTTPVO_HAVE__AVX2__
#    include "wrapper/avx2impl.hpp"
#endif

#if HTTPVO_HAVE__SSE4_2__
#    include "wrapper/sse4impl.hpp"
#endif

#if HTTPVO_HAVE__ARM_NEON__
#    include "wrapper/neonimpl.hpp"
#else
#    include "wrapper/fallback.hpp"
#endif
#endif // HTTPVO_SIMD_IMPLEMENTAION_HPP