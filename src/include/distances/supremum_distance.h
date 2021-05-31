#ifndef TEAM33_DISTANCES_SUPREMUM_DISTANCE_H
#define TEAM33_DISTANCES_SUPREMUM_DISTANCE_H

#include <cstddef>
#include <cstdlib>
#include <cmath>
#include <immintrin.h>
#include <common/vector_reductions.h>

/**
 * @brief Computes the Supremum distance between a and b
 *
 * @param a Vector of length >= n
 * @param b Vector of length >= n
 * @param n Size of the vectors
 * @return max(|a-b|)
 */
inline double SupremumDistance(const double* a, const double* b, size_t n) {
    double distance = 0.0;
    for(size_t i = 0; i < n; ++i) {
        double diff = fabs(a[i] - b[i]);
        distance = (distance > diff) ? distance : diff;
    }
    return distance;
}


/**
 * @brief 2-Unrolled version of function SupremumDistance.
 *
 * @param a Vector of length >= n
 * @param b Vector of length >= n
 * @param n Size of the vectors
 * @return max(|a-b|)
 */
inline double SupremumDistanceUnrolled(
    const double* a, const double* b, size_t n) {
    double distance = 0.0;
    double diff_0 = 0.0;
    double diff_1 = 0.0;

    size_t i = 0;
    for(; i < n-1; i+=2) {
        double diff_0 = fabs(a[i] - b[i]);
        double diff_1 = fabs(a[i+1] - b[i+1]);
        diff_0 = (diff_0 > diff_1) ? diff_0 : diff_1;
        distance = (distance > diff_0) ? distance : diff_0;
    }

    // scalar clean-up
    for (; i < n; i++) {
        double diff_0 = abs(a[i] - b[i]);
        distance = (distance > diff_0) ? distance : diff_0;
    }

    return distance;
}


/**
 * @brief 4-Unrolled version of function SupremumDistance.
 *
 * @param a Vector of length >= n
 * @param b Vector of length >= n
 * @param n Size of the vectors
 * @return max(|a-b|)
 */
inline double SupremumDistance_4Unrolled(
    const double* a, const double* b, size_t n) {
    double distance = 0.0;
    double diff_0 = 0.0;
    double diff_1 = 0.0;
    double diff_2 = 0.0;
    double diff_3 = 0.0;

    long int i = 0;
    long int m = (long int)n;
    for (; i < m - 3; i += 4) {
        double diff_0 = abs(a[i  ] - b[i  ]);
        double diff_1 = abs(a[i+1] - b[i+1]);
        double diff_2 = abs(a[i+2] - b[i+2]);
        double diff_3 = abs(a[i+3] - b[i+3]);
        diff_0 = (diff_0 > diff_1) ? diff_0 : diff_1;
        diff_2 = (diff_2 > diff_3) ? diff_2 : diff_3;
        diff_0 = (diff_0 > diff_2) ? diff_0 : diff_2;
        distance = (distance > diff_0) ? distance : diff_0;
    }

    // scalar clean-up
    for (; i < m; i++) {
        double diff_0 = abs(a[i] - b[i]);
        if (diff_0 > distance) {
            distance = diff_0;
        }
    }

    return distance;
}


#ifdef __AVX2__
/**
 * @brief SIMD-vectorized version of Supremum distance
 *
 * @param a Vector of length >= n
 * @param b Vector of length >= n
 * @param n Size of the vectors
 * @return max(|a-b|)
 */
inline double SupremumDistance_Vectorized(
    const double* a, const double* b, size_t n) {

    const __m256d _fabs_pd= _mm256_set1_pd(-0.); // 1<<63 for MSB: sign(double)
    __m256d a_val, b_val, diff_vec, dist_accum = _mm256_setzero_pd();

    long int i = 0;
    long int m = (long int)n;
    for (; i < m - 3; i += 4) {
        a_val = _mm256_loadu_pd(&a[i]);
        b_val = _mm256_loadu_pd(&b[i]);
        diff_vec = _mm256_sub_pd(a_val, b_val);
        diff_vec = _mm256_andnot_pd(_fabs_pd, diff_vec);
        dist_accum = _mm256_max_pd(dist_accum, diff_vec);
    }

    double distance = _mm256_reduce_max_pd(dist_accum);

    for (; i < m; i++) {
        double diff = fabs(a[i] - b[i]);
        distance = (distance > diff) ? distance : diff;
    }

    return distance;
}


/**
 * @brief 4-Unrolled SIMD-vectorized version of Supremum distance
 *
 * @param a Vector of length >= n
 * @param b Vector of length >= n
 * @param n Size of the vectors
 * @return max(|a-b|)
 */
inline double SupremumDistance_4Unrolled_Vectorized(
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
    for (; i < m - 15; i += 16) {
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
        dist_accum_0 = _mm256_max_pd(dist_accum_0, diff_vec_0);
        dist_accum_1 = _mm256_max_pd(dist_accum_1, diff_vec_1);
        dist_accum_2 = _mm256_max_pd(dist_accum_2, diff_vec_2);
        dist_accum_3 = _mm256_max_pd(dist_accum_3, diff_vec_3);
    }

    dist_accum_0 = _mm256_max_pd(dist_accum_0, dist_accum_1);
    dist_accum_2 = _mm256_max_pd(dist_accum_2, dist_accum_3);
    dist_accum_0 = _mm256_max_pd(dist_accum_0, dist_accum_2);

    for (; i < m - 3; i += 4) {
        a_val_0 = _mm256_loadu_pd(&a[i]);
        b_val_0 = _mm256_loadu_pd(&b[i]);
        diff_vec_0 = _mm256_sub_pd(a_val_0, b_val_0);
        diff_vec_0 = _mm256_andnot_pd(_fabs_pd, diff_vec_0);
        dist_accum_0 = _mm256_max_pd(dist_accum_0, diff_vec_0);
    }

    double distance = _mm256_reduce_max_pd(dist_accum_0);

    for (; i < m; i++) {
        double diff = fabs(a[i] - b[i]);
        distance = (distance > diff) ? distance : diff;
    }

    return distance;
}
#endif //__AVX2__
#endif
