#ifndef TEAM33_DISTANCES_MANHATTAN_DISTANCE_H
#define TEAM33_DISTANCES_MANHATTAN_DISTANCE_H

#include <immintrin.h>
#include <common/vector_reductions.h>

/**
 * @brief Computes the Manhattan distance between a and b
 *
 * @param a Vector of length >= n
 * @param b Vector of length >= n
 * @param n Size of the vectors
 * @return Manhattan distance
 */
inline double ManhattanDistance(const double* a, const double* b, size_t n) {
    double distance = 0.0;
    for(size_t i = 0; i < n; ++i) {
        distance += abs(a[i] - b[i]);
    }

    return distance;
}


/**
 * @brief Unrolled version of function ManhattanDistance.
 *
 * @param a Vector of length >= n
 * @param b Vector of length >= n
 * @param n Size of the vectors
 * @return Manhattan distance
 */
inline double ManhattanDistanceUnrolled(const double* a, const double* b, size_t n) {
    double distance_0 = 0.0;
    double distance_1 = 0.0;

    size_t i = 0;
    for(; i < n-1; i+=2) {
        distance_0 += abs(a[i] - b[i]);
        distance_1 += abs(a[i+1] - b[i+1]);
    }

    // scalar clean-up
    for (; i < n; i++) {
        distance_0 += abs(a[i] - b[i]);
    }

    return distance_0 + distance_1;
}

/**
 * @brief 4-Unrolled version of function ManhattanDistance.
 *
 * @param a Vector of length >= n
 * @param b Vector of length >= n
 * @param n Size of the vectors
 * @return Manhattan distance
 */
inline double ManhattanDistance_4Unrolled(
    const double* a, const double* b, size_t n) {
    double distance_0 = 0.0;
    double distance_1 = 0.0;
    double distance_2 = 0.0;
    double distance_3 = 0.0;

    long int i = 0;
    long int m = (long int)n;
    for(; i < m - 3; i += 4) {
        distance_0 += abs(a[i  ] - b[i  ]);
        distance_1 += abs(a[i+1] - b[i+1]);
        distance_2 += abs(a[i+2] - b[i+2]);
        distance_3 += abs(a[i+3] - b[i+3]);
    }

    // scalar clean-up
    for (; i < m; i++) {
        distance_0 += abs(a[i] - b[i]);
    }

    return distance_0 + distance_1 + distance_2 + distance_3;
}

/**
 * @brief SIMD-vectorized version of function ManhattanDistance.
 *
 * @param a Vector of length >= n
 * @param b Vector of length >= n
 * @param n Size of the vectors
 * @return Manhattan distance
 */
inline double ManhattanDistance_Vectorized(
    const double* a, const double* b, size_t n) {

     const __m256d _fabs_pd= _mm256_set1_pd(-0.); // 1<<63 for MSB: sign(double)
    __m256d a_val, b_val, diff_vec, dist_accum = _mm256_setzero_pd();

    long int i = 0;
    long int m = (long int)n;
    for(; i < m - 3; i += 4) {
        a_val = _mm256_loadu_pd(&a[i]);
        b_val = _mm256_loadu_pd(&b[i]);
        diff_vec = _mm256_sub_pd(a_val, b_val);
        diff_vec = _mm256_andnot_pd(_fabs_pd, diff_vec);
        dist_accum = _mm256_add_pd(dist_accum, diff_vec);
    }

    double distance = _mm256_reduce_sum_pd(dist_accum);

    for (; i < m; i++) {
        distance += abs(a[i] - b[i]);
    }

    return distance;
}

#endif
