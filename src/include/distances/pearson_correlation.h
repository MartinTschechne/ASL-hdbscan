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

#endif