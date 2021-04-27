#ifndef TEAM33_DISTANCES_COSINE_SIMILARITY_H
#define TEAM33_DISTANCES_COSINE_SIMILARITY_H

#include <cmath>

/**
 * @brief Computes to Cosine Similarity between arrays a and b.
 * 
 * @param a Vector of length >= n
 * @param b Vector of length >= n
 * @param n Size of the vectors
 * @return double 
 */
inline double CosineSimilarity(double* a, double* b, size_t n) {
    double dot_product = 0.0;
    double mag_a = 0.0;
    double mag_b = 0.0;

    for(size_t i = 0; i < n; ++i) {
        dot_product += a[i] * b[i];
        mag_a += a[i]*a[i];
        mag_b += b[i]*b[i];
    }

    return 1.0 - (dot_product / std::sqrt(mag_a * mag_b));
}

#endif 