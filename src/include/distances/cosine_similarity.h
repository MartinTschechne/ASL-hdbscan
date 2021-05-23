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

#endif
