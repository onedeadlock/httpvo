#ifndef DHTTP_SIMD_IMPLEMENTAION_HPP
#define DHTTP_SIMD_IMPLEMENTAION_HPP
#include "../include/definition.hpp"

namespace dhttp::simd
{
    template <int N> alignas(N) struct simdv;

     #ifndef FORCE_SIMD_32
          static constexpr int max = 64;
     #else
          static constexpr int max = 32;
     #endif
     
     #if HAVE__SSE2__
     #    include "westmere/implementation.hpp"
          //using namespace simd::westmere;
     #else
     #    include "fallback/implementation.hpp"
#         //using namespace dhttp::simd::fallback; 
     #endif
}
#endif // DHTTP_SIMD_IMPLEMENTAION_HPP