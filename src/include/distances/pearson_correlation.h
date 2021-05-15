#ifndef TEAM33_DISTANCES_PEARSON_CORRELATION_H
#define TEAM33_DISTANCES_PEARSON_CORRELATION_H

#include <cmath>

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

    return (1.0 - (cov / std::sqrt(std_a * std_b)));
}


/**
 * @brief Unrolled version of function PearsonCorrelation.
 *
 * @param a Vector of length >= n
 * @param b Vector of length >= n
 * @param n Size of the vectors
 * @return Person correlation
 */
inline double PearsonCorrelationUnrolled(const double* a, const double* b, size_t n) {
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

    // sclar clean-up
    for(; i < n; i++) {
        cov_0 += ((a[i] - mean_a_0) * (b[i] - mean_b_0));
        std_a_0 += ((a[i] - mean_a_0) * (a[i] - mean_a_0));
        std_b_0 += ((b[i] - mean_b_0) * (b[i] - mean_b_0));
    }

    return (1.0 - ((cov_0 + cov_1) / std::sqrt((std_a_0+std_a_1) * (std_b_0 + std_b_1))));
}

#endif
