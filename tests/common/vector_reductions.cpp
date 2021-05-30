#include "gtest/gtest.h"
#include <common/vector_reductions.h>
#include <immintrin.h>

TEST(vector_reductions, sum_pd) {
    __m256d v = _mm256_setr_pd(1., 2., 3., 4.);
    ASSERT_EQ(_mm256_reduce_sum_pd(v), 10.);
}

TEST(vector_reductions, sum_epi64) {
    __m256i v = _mm256_setr_epi64x(1, 2, 3, 4);
    ASSERT_EQ(_mm256_reduce_sum_epi64(v), 10);
}

TEST(vector_reductions, max_pd) {
     __m256d v = _mm256_setr_pd(1., 2., -3., -4.);
    ASSERT_EQ(_mm256_reduce_max_pd(v), 2.);
}
