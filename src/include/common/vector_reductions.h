#ifndef TEAM33_DISTANCES_VECTOR_REDUCE_SUM_H
#define TEAM33_DISTANCES_VECTOR_REDUCE_SUM_H

#include <immintrin.h>

/**
 * @brief Efficient sum reduction of 256-bit double-precision vector
 *
 * @param v Packed double-precision floating-point elements to be summed up
 * @return double Sum of v
 */
inline double _mm256_reduce_sum_pd(__m256d v) {
    __m128d lo = _mm256_castpd256_pd128(v); // lat 0, cpi 0
    __m128d hi = _mm256_extractf128_pd(v, 1); // lat 3, cpi 1
    lo = _mm_add_pd(lo, hi); // lat 3, cpi 1
    hi = _mm_unpackhi_pd(lo, lo); // lat 1, cpi 1
    lo = _mm_add_pd(lo, hi); // lat 3, cpi 1
    return _mm_cvtsd_f64(lo); // lat 1, cpi 1
}

/**
 * @brief Efficient maximum reduction of 256-bit double-precision vector
 *
 * @param v Packed double-precision floating-point elements whose max is sought
 * @return double Maximum of v
 */
inline double _mm256_reduce_max_pd(__m256d v) {
    __m128d lo = _mm256_castpd256_pd128(v); // lat 0, cpi 0
    __m128d hi = _mm256_extractf128_pd(v, 1); // lat 3, cpi 1
    lo = _mm_max_pd(lo, hi); // lat 3, cpi 1
    hi = _mm_permute_pd(lo, 1); // lat 1, cpi 1
    lo = _mm_max_pd(lo, hi); // lat 3, cpi 1
    return _mm_cvtsd_f64(lo); // lat 1, cpi 1
}

#endif