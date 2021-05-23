#ifndef TEAM33_DISTANCES_MANHATTAN_DISTANCE_H
#define TEAM33_DISTANCES_MANHATTAN_DISTANCE_H

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

#endif
