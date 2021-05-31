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
 * @brief Efficient sum reduction of 256-bit vector of 64-bit integers
 *
 * @param v Packed 64-bit integer elements to be summed up
 * @return size_t Sum of v
 */
inline size_t _mm256_reduce_sum_epi64(__m256i v) {
    __m128i lo = _mm256_castsi256_si128(v); // lat 0, cpi 0
    __m128i hi = _mm256_extractf128_si256(v, 1); // lat 3, cpi 1
    lo = _mm_add_epi64(lo, hi); // lat 3, cpi 1
    hi = _mm_unpackhi_epi64(lo, lo); // lat 1, cpi 1
    lo = _mm_add_epi64(lo, hi); // lat 3, cpi 1
    return _mm_cvtsi128_si64(lo); // lat 1, cpi 1
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
