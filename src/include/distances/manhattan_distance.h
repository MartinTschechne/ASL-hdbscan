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


#ifdef __AVX2__
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
    __m256d a_val_0, a_val_1, a_val_2, a_val_3, b_val_0, b_val_1,
        b_val_2, b_val_3, diff_vec_0, diff_vec_1, diff_vec_2, diff_vec_3;
    __m256d dist_accum_0 = _mm256_setzero_pd(),
        dist_accum_1 = _mm256_setzero_pd(),
        dist_accum_2 = _mm256_setzero_pd(),
        dist_accum_3 = _mm256_setzero_pd();

    long int i = 0;
    long int m = (long int)n;
    for(; i < m - 15; i += 16) {
        a_val_0 = _mm256_loadu_pd(&a[i]);
        a_val_1 = _mm256_loadu_pd(&a[i+4]);
        a_val_2 = _mm256_loadu_pd(&a[i+8]);
        a_val_3 = _mm256_loadu_pd(&a[i+12]);
        b_val_0 = _mm256_loadu_pd(&b[i]);
        b_val_1 = _mm256_loadu_pd(&b[i+4]);
        b_val_2 = _mm256_loadu_pd(&b[i+8]);
        b_val_3 = _mm256_loadu_pd(&b[i+12]);
        diff_vec_0 = _mm256_sub_pd(a_val_0, b_val_0);
        diff_vec_1 = _mm256_sub_pd(a_val_1, b_val_1);
        diff_vec_2 = _mm256_sub_pd(a_val_2, b_val_2);
        diff_vec_3 = _mm256_sub_pd(a_val_3, b_val_3);
        diff_vec_0 = _mm256_andnot_pd(_fabs_pd, diff_vec_0);
        diff_vec_1 = _mm256_andnot_pd(_fabs_pd, diff_vec_1);
        diff_vec_2 = _mm256_andnot_pd(_fabs_pd, diff_vec_2);
        diff_vec_3 = _mm256_andnot_pd(_fabs_pd, diff_vec_3);
        dist_accum_0 = _mm256_add_pd(dist_accum_0, diff_vec_0);
        dist_accum_1 = _mm256_add_pd(dist_accum_1, diff_vec_1);
        dist_accum_2 = _mm256_add_pd(dist_accum_2, diff_vec_2);
        dist_accum_3 = _mm256_add_pd(dist_accum_3, diff_vec_3);
    }

    dist_accum_0 = _mm256_add_pd(dist_accum_0, dist_accum_1);
    dist_accum_2 = _mm256_add_pd(dist_accum_2, dist_accum_3);
    dist_accum_0 = _mm256_add_pd(dist_accum_0, dist_accum_2);

    for(; i < m - 3; i += 4) {
        a_val_0 = _mm256_loadu_pd(&a[i]);
        b_val_0 = _mm256_loadu_pd(&b[i]);
        diff_vec_0 = _mm256_sub_pd(a_val_0, b_val_0);
        diff_vec_0 = _mm256_andnot_pd(_fabs_pd, diff_vec_0);
        dist_accum_0 = _mm256_add_pd(dist_accum_0, diff_vec_0);
    }

    double distance = _mm256_reduce_sum_pd(dist_accum_0);

    for (; i < m; i++) {
        distance += abs(a[i] - b[i]);
    }

    return distance;
}
#endif //__AVX2__
#endif
