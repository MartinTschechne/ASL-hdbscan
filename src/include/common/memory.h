#ifndef TEAM33_COMMON_MEMORY_H
#define TEAM33_COMMON_MEMORY_H

#include <malloc.h>

inline size_t* CreateAlignedSizeT1D(size_t size) {
    return (size_t*)memalign(64, size*sizeof(size_t));
}

inline double* CreateAlignedDouble1D(size_t size) {
    return (double*)memalign(64, size*sizeof(double));
}

inline double** CreateAlignedDouble2D(size_t dim_a, size_t dim_b) {
    double** result = (double**)malloc(dim_a*sizeof(double*));
    for(size_t i = 0; i < dim_a; ++i) {
        result[i] = CreateAlignedDouble1D(dim_b);
    }

    return result;
}

#endif