#ifndef TEAM33_DISTANCES_PEARSON_CORRELATION_H
#define TEAM33_DISTANCES_PEARSON_CORRELATION_H

#include <cmath>
#include <cstdio>
#include <immintrin.h>
#include <common/vector_reductions.h>

/**
 * @brief Computes the pearson correlation d = 1 - (cov(X,Y) / (std_dev(X) * std_dev(Y)))
 *
 * @param a Vector of length >= n
 * @param b Vector of length >= n
 * @param n Size of the vectors
 * @return Person correlation
 */
inline double PearsonCorrelation(const double* a, const double* b, size_t n) {
    double mean_a = 0.0;
    double mean_b = 0.0;

    for(size_t i = 0; i < n; ++i) {
        mean_a += a[i];
        mean_b += b[i];
    }

    mean_a /= n;
    mean_b /= n;

    double cov = 0.0;
    double std_a = 0.0;
    double std_b = 0.0;

    for(size_t i = 0; i < n; ++i) {
        cov += ((a[i] - mean_a) * (b[i] - mean_b));
        std_a += ((a[i] - mean_a) * (a[i] - mean_a));
        std_b += ((b[i] - mean_b) * (b[i] - mean_b));
    }

    return (1.0 - (cov / sqrt(std_a * std_b)));
}


/**
 * @brief 2-unrolled version of function PearsonCorrelation.
 *
 * @param a Vector of length >= n
 * @param b Vector of length >= n
 * @param n Size of the vectors
 * @return Person correlation
 */
inline double PearsonCorrelationUnrolled(
    const double* a, const double* b, size_t n) {

    double mean_a_0 = 0.0;
    double mean_b_0 = 0.0;
    double mean_a_1 = 0.0;
    double mean_b_1 = 0.0;

    size_t i = 0;
    for(; i < n-1; i+=2) {
        mean_a_0 += a[i];
        mean_a_1 += a[i+1];

        mean_b_0 += b[i];
        mean_b_1 += b[i+1];
    }

    // scalar clean-up
    for (; i < n; i++) {
        mean_a_0 += a[i];
        mean_b_0 += b[i];
    }

    mean_a_0 = (mean_a_0 + mean_a_1)/n;
    mean_b_0 = (mean_b_0 + mean_b_1)/n;

    double cov_0 = 0.0;
    double std_a_0 = 0.0;
    double std_b_0 = 0.0;
    double cov_1 = 0.0;
    double std_a_1 = 0.0;
    double std_b_1 = 0.0;

    i = 0;
    for(; i < n-1; i+=2) {
        cov_0 += ((a[i] - mean_a_0) * (b[i] - mean_b_0));
        std_a_0 += ((a[i] - mean_a_0) * (a[i] - mean_a_0));
        std_b_0 += ((b[i] - mean_b_0) * (b[i] - mean_b_0));

        cov_1 += ((a[i+1] - mean_a_0) * (b[i+1] - mean_b_0));
        std_a_1 += ((a[i+1] - mean_a_0) * (a[i+1] - mean_a_0));
        std_b_1 += ((b[i+1] - mean_b_0) * (b[i+1] - mean_b_0));
    }

    // scalar clean-up
    for(; i < n; i++) {
        cov_0 += ((a[i] - mean_a_0) * (b[i] - mean_b_0));
        std_a_0 += ((a[i] - mean_a_0) * (a[i] - mean_a_0));
        std_b_0 += ((b[i] - mean_b_0) * (b[i] - mean_b_0));
    }

    return (1.0 - ((cov_0 + cov_1) / sqrt((std_a_0+std_a_1) * (std_b_0 + std_b_1))));
}


/**
 * @brief 4-unrolled version of function PearsonCorrelation.
 *
 * @param a Vector of length >= n
 * @param b Vector of length >= n
 * @param n Size of the vectors
 * @return Person correlation
 */
inline double PearsonCorrelation_4Unrolled(
    const double* a, const double* b, size_t n) {

    double mean_a_0 = 0.0;
    double mean_b_0 = 0.0;
    double mean_a_1 = 0.0;
    double mean_b_1 = 0.0;
    double mean_a_2 = 0.0;
    double mean_b_2 = 0.0;
    double mean_a_3 = 0.0;
    double mean_b_3 = 0.0;

    size_t i = 0;
    for (; i < n-3; i+=4) {
        mean_a_0 += a[i];
        mean_a_1 += a[i+1];
        mean_a_2 += a[i+2];
        mean_a_3 += a[i+3];

        mean_b_0 += b[i];
        mean_b_1 += b[i+1];
        mean_b_2 += b[i+2];
        mean_b_3 += b[i+3];
    }

    // scalar clean-up
    for (; i < n; i++) {
        mean_a_0 += a[i];
        mean_b_0 += b[i];
    }

    mean_a_0 = (mean_a_0 + mean_a_1 + mean_a_2 + mean_a_3)/n;
    mean_b_0 = (mean_b_0 + mean_b_1 + mean_b_2 + mean_b_3)/n;

    double cov_0 = 0.0;
    double std_a_0 = 0.0;
    double std_b_0 = 0.0;
    double cov_1 = 0.0;
    double std_a_1 = 0.0;
    double std_b_1 = 0.0;
    double cov_2 = 0.0;
    double std_a_2 = 0.0;
    double std_b_2 = 0.0;
    double cov_3 = 0.0;
    double std_a_3 = 0.0;
    double std_b_3 = 0.0;

    i = 0;
    for (; i < n-3; i+=4) {
        cov_0   += ((a[i  ] - mean_a_0) * (b[i  ] - mean_b_0));
        std_a_0 += ((a[i  ] - mean_a_0) * (a[i  ] - mean_a_0));
        std_b_0 += ((b[i  ] - mean_b_0) * (b[i  ] - mean_b_0));
        cov_1   += ((a[i+1] - mean_a_0) * (b[i+1] - mean_b_0));
        std_a_1 += ((a[i+1] - mean_a_0) * (a[i+1] - mean_a_0));
        std_b_1 += ((b[i+1] - mean_b_0) * (b[i+1] - mean_b_0));
        cov_2   += ((a[i+2] - mean_a_0) * (b[i+2] - mean_b_0));
        std_a_2 += ((a[i+2] - mean_a_0) * (a[i+2] - mean_a_0));
        std_b_2 += ((b[i+2] - mean_b_0) * (b[i+2] - mean_b_0));
        cov_3   += ((a[i+3] - mean_a_0) * (b[i+3] - mean_b_0));
        std_a_3 += ((a[i+3] - mean_a_0) * (a[i+3] - mean_a_0));
        std_b_3 += ((b[i+3] - mean_b_0) * (b[i+3] - mean_b_0));
    }

    // scalar clean-up
    for (; i < n; i++) {
        cov_0   += ((a[i  ] - mean_a_0) * (b[i  ] - mean_b_0));
        std_a_0 += ((a[i  ] - mean_a_0) * (a[i  ] - mean_a_0));
        std_b_0 += ((b[i  ] - mean_b_0) * (b[i  ] - mean_b_0));
    }

    return (
        1.0 - ((cov_0 + cov_1 + cov_2 + cov_3) / sqrt(
        (std_a_0 + std_a_1 + std_a_2 + std_a_3) *
        (std_b_0 + std_b_1 + std_b_2 + std_b_3)))
        );
}

/**
 * @brief Single pass version of function PearsonCorrelation.
 *
 *
 * @param a Vector of length >= n
 * @param b Vector of length >= n
 * @param n Size of the vectors
 * @return Person correlation
 */
inline double PearsonCorrelation_Onepass(
    const double* a, const double* b, size_t n) {

    // ref. Bennett et al: "Numerically stable, single-pass, parallel
    // statistics algorithms" (2009), DOI: 10.1109/CLUSTR.2009.5289161
    // and corresponding implementation in boost/math/statistics

    double mean_a = a[0];
    double mean_b = b[0];
    double cov = 0.;
    double std_a = 0.;
    double std_b = 0.;

    for (size_t i = 1; i < n; ++i) {
        double ip1inv = 1. / (i+1);
        double temp_a = a[i] - mean_a ;
        double temp_b = b[i] - mean_b;
        std_a += i * temp_a * temp_a * ip1inv;
        std_b += i * temp_b * temp_b * ip1inv;
        cov += i * temp_a * temp_b * ip1inv;
        mean_a += temp_a * ip1inv;
        mean_b += temp_b * ip1inv;
    }
    return (1.0 - (cov / sqrt(std_a * std_b)));
}

/**
 * @brief 4Unrolled single pass version of function PearsonCorrelation.
 *
 * @param a Vector of length >= n
 * @param b Vector of length >= n
 * @param n Size of the vectors
 * @return Person correlation
 */
inline double PearsonCorrelation_4UnrolledOnepass(
    const double* a, const double* b, size_t n) {

    // ref. Bennett et al: "Numerically stable, single-pass, parallel
    // statistics algorithms" (2009), DOI: 10.1109/CLUSTR.2009.5289161
    // and corresponding implementation in boost/math/statistics

    double mean_a_0 = a[0];
    double mean_b_0 = b[0];
    double cov_0 = 0.;
    double cov_1 = 0.;
    double cov_2 = 0.;
    double cov_3 = 0.;
    double std_a_0 = 0.;
    double std_a_1 = 0.;
    double std_a_2 = 0.;
    double std_a_3 = 0.;
    double std_b_0 = 0.;
    double std_b_1 = 0.;
    double std_b_2 = 0.;
    double std_b_3 = 0.;

    size_t i = 1;
    for (; i < n-3; i+=4) {
        double ip1inv_0 = 1. / (i+1);
        double ip1inv_1 = 1. / (i+2);
        double ip1inv_2 = 1. / (i+3);
        double ip1inv_3 = 1. / (i+4);

        double temp_a_0 = a[i  ] - mean_a_0;
        double temp_b_0 = b[i  ] - mean_b_0;
        std_a_0 +=  i    * temp_a_0 * temp_a_0 * ip1inv_0;
        std_b_0 +=  i    * temp_b_0 * temp_b_0 * ip1inv_0;
        cov_0 += i * temp_a_0 * temp_b_0 * ip1inv_0;
        mean_a_0 += temp_a_0 * ip1inv_0;
        mean_b_0 += temp_b_0 * ip1inv_0;

        double temp_a_1 = a[i+1] - mean_a_0;
        double temp_b_1 = b[i+1] - mean_b_0;
        std_a_1 += (i+1) * temp_a_1 * temp_a_1 * ip1inv_1;
        std_b_1 += (i+1) * temp_b_1 * temp_b_1 * ip1inv_1;
        cov_1 += (i+1) * temp_a_1 * temp_b_1 * ip1inv_1;
        mean_a_0 += temp_a_1 * ip1inv_1;
        mean_b_0 += temp_b_1 * ip1inv_1;

        double temp_a_2 = a[i+2] - mean_a_0;
        double temp_b_2 = b[i+2] - mean_b_0;
        std_a_2 += (i+2) * temp_a_2 * temp_a_2 * ip1inv_2;
        std_b_2 += (i+2) * temp_b_2 * temp_b_2 * ip1inv_2;
        cov_2 += (i+2) * temp_a_2 * temp_b_2 * ip1inv_2;
        mean_a_0 += temp_a_2 * ip1inv_2;
        mean_b_0 += temp_b_2 * ip1inv_2;

        double temp_a_3 = a[i+3] - mean_a_0;
        double temp_b_3 = b[i+3] - mean_b_0;
        std_a_3 += (i+3) * temp_a_3 * temp_a_3 * ip1inv_3;
        std_b_3 += (i+3) * temp_b_3 * temp_b_3 * ip1inv_3;
        cov_3 += (i+3) * temp_a_3 * temp_b_3 * ip1inv_3;
        mean_a_0 += temp_a_3 * ip1inv_3;
        mean_b_0 += temp_b_3 * ip1inv_3;
    }

    for (; i < n; ++i) {
        double ip1inv = 1. / (i+1);
        double temp_a = a[i] - mean_a_0;
        double temp_b = b[i] - mean_b_0;
        std_a_0 += i * temp_a * temp_a * ip1inv;
        std_b_0 += i * temp_b * temp_b * ip1inv;
        cov_0 += i * temp_a * temp_b * ip1inv;
        mean_a_0 += temp_a * ip1inv;
        mean_b_0 += temp_b * ip1inv;
    }


    return (
        1.0 - ((cov_0 + cov_1 + cov_2 + cov_3) / sqrt(
        (std_a_0 + std_a_1 + std_a_2 + std_a_3) *
        (std_b_0 + std_b_1 + std_b_2 + std_b_3)))
        );
}


/**
 * @brief SIMD-vectorized version of Pearson correlation distance
 *
 * @param a Vector of length >= n
 * @param b Vector of length >= n
 * @param n Size of the vectors
 * @return Person correlation
 */
inline double PearsonCorrelation_Vectorized(
    const double* a, const double* b, size_t n) {

    __m256d a_val, b_val, a_accum = _mm256_setzero_pd(),
        b_accum = _mm256_setzero_pd();
    size_t i;

    for (i = 0; i < n - 3; i += 4) {
        a_val = _mm256_loadu_pd(&a[i]);
        b_val = _mm256_loadu_pd(&b[i]);
        a_accum = _mm256_add_pd(a_accum, a_val);
        b_accum = _mm256_add_pd(b_accum, b_val);
    }

    double mean_a = _mm256_reduce_sum_pd(a_val);
    double mean_b = _mm256_reduce_sum_pd(b_val);

    for (; i < n; i++) {
        mean_a += a[i];
        mean_b += b[i];
    }

    mean_a /= n;
    mean_b /= n;

    const __m256d mean_a_vec = _mm256_set1_pd(mean_a);
    const __m256d mean_b_vec = _mm256_set1_pd(mean_b);


    __m256d a_diff, b_diff, cov_accum = _mm256_setzero_pd(),
        std_a_accum = _mm256_setzero_pd(), std_b_accum = _mm256_setzero_pd();

    for (i = 0; i < n - 3; i += 4) {
        a_val = _mm256_loadu_pd(&a[i]);
        a_diff = _mm256_sub_pd(a_val, mean_a_vec);
        b_val = _mm256_loadu_pd(&b[i]);
        b_diff = _mm256_sub_pd(b_val, mean_b_vec);
        cov_accum = _mm256_add_pd(cov_accum, _mm256_mul_pd(a_diff, b_diff));
        std_a_accum = _mm256_add_pd(std_a_accum, _mm256_mul_pd(a_diff, a_diff));
        std_b_accum = _mm256_add_pd(std_b_accum, _mm256_mul_pd(b_diff, b_diff));
    }

    double cov = _mm256_reduce_sum_pd(cov_accum);
    double std_a = _mm256_reduce_sum_pd(std_a_accum);
    double std_b = _mm256_reduce_sum_pd(std_b_accum);

    for (; i < n; i++) {
        cov += ((a[i] - mean_a) * (b[i] - mean_b));
        std_a += ((a[i] - mean_a) * (a[i] - mean_a));
        std_b += ((b[i] - mean_b) * (b[i] - mean_b));
    }

    return (1.0 - (cov / sqrt(std_a * std_b)));
}


#endif
