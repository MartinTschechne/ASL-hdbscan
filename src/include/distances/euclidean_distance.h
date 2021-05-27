#ifndef TEAM33_DISTANCES_EUCLIDIAN_DISTANCE_H
#define TEAM33_DISTANCES_EUCLIDIAN_DISTANCE_H

#include <cmath>
#include <immintrin.h>
#include <common/vector_reductions.h>

/**
 * @brief Computes the Euclidean distance between a and b
 *
 * @param a Vector of length >= n
 * @param b Vector of length >= n
 * @param n Size of the vectors
 * @return Euclidean distance
 */
inline double EuclideanDistance(const double* a, const double* b, size_t n) {
    double distance = 0.0;
    double diff = 0.0;
    for(size_t i = 0; i < n; ++i) {
        diff = a[i]-b[i];
        distance += diff * diff;
    }

    return sqrt(distance);
}


/**
 * @brief Unrolled version of function EuclideanDistance.
 *
 * @param a Vector of length >= n
 * @param b Vector of length >= n
 * @param n Size of the vectors
 * @return Euclidean distance
 */
inline double EuclideanDistanceUnrolled(const double* a, const double* b, size_t n) {
    double distance_0 = 0.0;
    double distance_1 = 0.0;
    double diff_0 = 0.0;
    double diff_1 = 0.0;
    size_t i = 0;

    for(; i < n-1; i += 2) {
        diff_0 = a[i]-b[i];
        distance_0 += diff_0 * diff_0;

        diff_1 = a[i+1]-b[i+1];
        distance_1 += diff_1 * diff_1;
    }

    // scalar clean-up
    for (; i < n; i++) {
        diff_0 = a[i]-b[i];
        distance_0 += diff_0 * diff_0;
    }

    return sqrt(distance_0 + distance_1);
}

/**
 * @brief 4-Unrolled version of function EuclideanDistance.
 *
 * @param a Vector of length >= n
 * @param b Vector of length >= n
 * @param n Size of the vectors
 * @return Euclidean distance
 */
inline double EuclideanDistance_4Unrolled(
    const double* a, const double* b, size_t n) {
    double distance_0 = 0.0;
    double distance_1 = 0.0;
    double distance_2 = 0.0;
    double distance_3 = 0.0;
    double diff_0 = 0.0;
    double diff_1 = 0.0;
    double diff_2 = 0.0;
    double diff_3 = 0.0;

    long int i = 0;
    long int m = (long int)n;
    for (; i < m - 3; i += 4) {
        diff_0 = a[i  ] - b[i  ];
        diff_1 = a[i+1] - b[i+1];
        diff_2 = a[i+2] - b[i+2];
        diff_3 = a[i+3] - b[i+3];

        distance_0 += diff_0 * diff_0;
        distance_1 += diff_1 * diff_1;
        distance_2 += diff_2 * diff_2;
        distance_3 += diff_3 * diff_3;
    }

    // scalar clean-up
    for (; i < m; i++) {
        diff_0 = a[i] - b[i];
        distance_0 += diff_0 * diff_0;
    }

    return sqrt(distance_0 + distance_1 + distance_2 + distance_3);
}

/**
 * @brief SIMD-vectorized version of Euclidean distance
 *
 * @param a Vector of length >= n
 * @param b Vector of length >= n
 * @param n Size of the vectors
 * @return Euclidean distance
 */
inline double EuclideanDistance_Vectorized(
    const double* a, const double* b, size_t n) {

    __m256d a_val_0, a_val_1, b_val_0, b_val_1, diff_vec_0, diff_vec_1;
    __m256d dist_accum_0 = _mm256_setzero_pd(),
        dist_accum_1 = _mm256_setzero_pd();

    long int i = 0;
    long int m = (long int)n;
    for (; i < m - 7; i += 8) {
        a_val_0 = _mm256_loadu_pd(&a[i]);
        a_val_1 = _mm256_loadu_pd(&a[i+4]);
        b_val_0 = _mm256_loadu_pd(&b[i]);
        b_val_1 = _mm256_loadu_pd(&b[i+4]);
        diff_vec_0 = _mm256_sub_pd(a_val_0, b_val_0);
        diff_vec_1 = _mm256_sub_pd(a_val_1, b_val_1);
        diff_vec_0 = _mm256_mul_pd(diff_vec_0, diff_vec_0);
        diff_vec_1 = _mm256_mul_pd(diff_vec_1, diff_vec_1);
        dist_accum_0 = _mm256_add_pd(dist_accum_0, diff_vec_0);
        dist_accum_1 = _mm256_add_pd(dist_accum_1, diff_vec_1);
    }

    dist_accum_0 = _mm256_add_pd(dist_accum_0, dist_accum_1);

    for (; i < m - 3; i += 4) {
        a_val_0 = _mm256_loadu_pd(&a[i]);
        b_val_0 = _mm256_loadu_pd(&b[i]);
        diff_vec_0 = _mm256_sub_pd(a_val_0, b_val_0);
        diff_vec_0 = _mm256_mul_pd(diff_vec_0, diff_vec_0);
        dist_accum_0 = _mm256_add_pd(dist_accum_0, diff_vec_0);
    }

    double distance = _mm256_reduce_sum_pd(dist_accum_0);

    for(; i < m; i++) {
        double diff = a[i] - b[i];
        distance += diff * diff;
    }

    return sqrt(distance);
}


#endif
