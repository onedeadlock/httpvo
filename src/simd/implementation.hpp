#ifndef HTTPVO_SIMD_IMPLEMENTAION_HPP
#define HTTPVO_SIMD_IMPLEMENTAION_HPP
#include "./../include/definition.hpp"

#if HTTPVO_HAVE__AVX2__
#    include "wrapper/avx2.hpp"
#endif

#if HTTPVO_HAVE__SSE4_2__
#    include "wrapper/sse4.hpp"
#endif

#if HTTPVO_HAVE__ARM_NEON__
#    include "wrapper/neon64.hpp"
#    include "wrapper/neon32.hpp"
#else
#    include "wrapper/fallback.hpp"
#endif
#endif // HTTPVO_SIMD_IMPLEMENTAION_HPP