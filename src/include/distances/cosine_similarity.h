#ifndef TEAM33_DISTANCES_COSINE_SIMILARITY_H
#define TEAM33_DISTANCES_COSINE_SIMILARITY_H

#include <cmath>
#include <immintrin.h>
#include <common/vector_reductions.h>

/**
 * @brief Computes to Cosine Similarity between arrays a and b.
 *
 * @param a Vector of length >= n
 * @param b Vector of length >= n
 * @param n Size of the vectors
 * @return double
 */
inline double CosineSimilarity(const double* a, const double* b, size_t n) {
    double dot_product = 0.0;
    double mag_a = 0.0;
    double mag_b = 0.0;

    for(size_t i = 0; i < n; ++i) {
        dot_product += a[i] * b[i];
        mag_a += a[i]*a[i];
        mag_b += b[i]*b[i];
    }

    return 1.0 - (dot_product / sqrt(mag_a * mag_b));
}


/**
 * @brief Unrolled version of function CosineSimilarity.
 *
 * @param a Vector of length >= n
 * @param b Vector of length >= n
 * @param n Size of the vectors
 * @return double
 */
inline double CosineSimilarityUnrolled(const double* a, const double* b, size_t n) {
    double dot_product_0 = 0.0;
    double mag_a_0 = 0.0;
    double mag_b_0 = 0.0;
    double dot_product_1 = 0.0;
    double mag_a_1 = 0.0;
    double mag_b_1 = 0.0;

    size_t i = 0;
    for(; i < n-1; i+=2) {
        dot_product_0 += a[i] * b[i];
        mag_a_0 += a[i]*a[i];
        mag_b_0 += b[i]*b[i];

        dot_product_1 += a[i+1] * b[i+1];
        mag_a_1 += a[i+1]*a[i+1];
        mag_b_1 += b[i+1]*b[i+1];
    }

    // scalar clean-up
    for(; i < n; i++) {
        dot_product_0 += a[i] * b[i];
        mag_a_0 += a[i]*a[i];
        mag_b_0 += b[i]*b[i];
    }

    dot_product_0 += dot_product_1;
    mag_a_0 += mag_a_1;
    mag_b_0 += mag_b_1;
    return 1.0 - (dot_product_0 / sqrt(mag_a_0 * mag_b_0));
}

/**
 * @brief 4-unrolled version of function CosineSimilarity
 *
 * @param a Vector of length >= n
 * @param b Vector of length >= n
 * @param n Size of the vectors
 * @return double
 */
inline double CosineSimilarity_4Unrolled(
    const double* a, const double* b, size_t n) {
    double mag_a_0 = 0.0;
    double mag_b_0 = 0.0;
    double mag_a_1 = 0.0;
    double mag_b_1 = 0.0;
    double mag_a_2 = 0.0;
    double mag_b_2 = 0.0;
    double mag_a_3 = 0.0;
    double mag_b_3 = 0.0;
    double dot_product_0 = 0.0;
    double dot_product_1 = 0.0;
    double dot_product_2 = 0.0;
    double dot_product_3 = 0.0;

    long int i = 0;
    long int m = (long int)n;
    for(; i < m - 3; i += 4) {
        dot_product_0 += a[i  ] * b[i  ];
        mag_a_0 += a[i  ] * a[i  ];
        mag_b_0 += b[i  ] * b[i  ];
        dot_product_1 += a[i+1] * b[i+1];
        mag_a_1 += a[i+1] * a[i+1];
        mag_b_1 += b[i+1] * b[i+1];
        dot_product_2 += a[i+2] * b[i+2];
        mag_a_2 += a[i+2] * a[i+2];
        mag_b_2 += b[i+2] * b[i+2];
        dot_product_3 += a[i+3] * b[i+3];
        mag_a_3 += a[i+3] * a[i+3];
        mag_b_3 += b[i+3] * b[i+3];
    }

    // scalar clean-up
    for(; i < m; i++) {
        dot_product_0 += a[i] * b[i];
        mag_a_0 += a[i]*a[i];
        mag_b_0 += b[i]*b[i];
    }

    dot_product_0 = dot_product_0 + dot_product_1 + dot_product_2 +
        dot_product_3;
    mag_a_0 = mag_a_0 + mag_a_1 + mag_a_2 + mag_a_3;
    mag_b_0 = mag_b_0 + mag_b_1 + mag_b_2 + mag_b_3;
    return 1.0 - (dot_product_0 / sqrt(mag_a_0 * mag_b_0));
}

/**
 * @brief SIMD-vectorized version of function CosineSimilarity
 *
 * @param a Vector of length >= n
 * @param b Vector of length >= n
 * @param n Size of the vectors
 * @return double
 */
inline double CosineSimilarity_Vectorized(
    const double* a, const double* b, size_t n) {


    __m256d a_val, b_val;
    __m256d mag_a_accum = _mm256_setzero_pd(),
        mag_b_accum = _mm256_setzero_pd(),
        dot_product_accum = _mm256_setzero_pd();

    long int i = 0;
    long int m = (long int)n;
    for (; i < m - 3; i += 4) {
        a_val = _mm256_loadu_pd(&a[i]);
        b_val = _mm256_loadu_pd(&b[i]);
        dot_product_accum = _mm256_add_pd(
            dot_product_accum, _mm256_mul_pd(a_val, b_val));
        mag_a_accum = _mm256_add_pd(
            mag_a_accum, _mm256_mul_pd(a_val, a_val));
        mag_b_accum = _mm256_add_pd(
            mag_b_accum, _mm256_mul_pd(b_val, b_val));
    }

    double dot_product = _mm256_reduce_sum_pd(dot_product_accum);
    double mag_a = _mm256_reduce_sum_pd(mag_a_accum);
    double mag_b = _mm256_reduce_sum_pd(mag_b_accum);

    for(; i < m; i++) {
        dot_product += a[i] * b[i];
        mag_a += a[i] * a[i];
        mag_b += b[i] * b[i];
    }

    return 1. - (dot_product / sqrt(mag_a * mag_b));
    }
#endif
