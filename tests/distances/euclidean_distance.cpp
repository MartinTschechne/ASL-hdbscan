#include "gtest/gtest.h"
#include <distances/euclidean_distance.h>

TEST(distances, euclidean_similar) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {1.0, 2.0, 3.0, 4.0, 5.0};

    double sim = EuclideanDistance(a, b, 5);

    ASSERT_NEAR(sim, 0.0, 0.0001);
}

TEST(distances, euclidean1) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {2.5, 8.5, 2.0, 1.0, 3.0};

    double sim = EuclideanDistance(a, b, 5);

    ASSERT_NEAR(sim, 7.64852927, 0.0001);

    double sim2 = EuclideanDistance(b, a, 5);
    ASSERT_NEAR(sim2, sim, 0.0001);
}

TEST(distances, euclidean_similar_unrolled) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {1.0, 2.0, 3.0, 4.0, 5.0};

    double sim = EuclideanDistanceUnrolled(a, b, 5);

    ASSERT_NEAR(sim, 0.0, 0.0001);
}

TEST(distances, euclidean1_unrolled) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {2.5, 8.5, 2.0, 1.0, 3.0};

    double sim = EuclideanDistanceUnrolled(a, b, 5);
    ASSERT_NEAR(sim, 7.64852927, 0.0001);

    double sim2 = EuclideanDistanceUnrolled(b, a, 5);
    ASSERT_NEAR(sim2, sim, 0.0001);
}

TEST(distances, euclidean_unrolled_length) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {2.5, 8.5, 2.0, 1.0, 3.0};

    double sim = EuclideanDistanceUnrolled(a, b, 4);
    ASSERT_NEAR(sim, EuclideanDistance(a, b, 4), 0.0001);

    sim = EuclideanDistanceUnrolled(a, b, 3);
    ASSERT_NEAR(sim, EuclideanDistance(a, b, 3), 0.0001);

    sim = EuclideanDistanceUnrolled(a, b, 2);
    ASSERT_NEAR(sim, EuclideanDistance(a, b, 2), 0.0001);

}

TEST(distances, euclidean_similar_4unrolled) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {1.0, 2.0, 3.0, 4.0, 5.0};

    double sim = EuclideanDistance_4Unrolled(a, b, 5);

    ASSERT_NEAR(sim, 0.0, 0.0001);
}

TEST(distances, euclidean_4unrolled) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {2.5, 8.5, 2.0, 1.0, 3.0};

    double sim = EuclideanDistance_4Unrolled(a, b, 5);
    ASSERT_NEAR(sim, 7.64852927, 0.0001);

    double sim2 = EuclideanDistance_4Unrolled(b, a, 5);
    ASSERT_NEAR(sim2, sim, 0.0001);
}

#ifdef __AVX2__
TEST(distances, euclidean_similar_vectorized) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {1.0, 2.0, 3.0, 4.0, 5.0};

    double sim = EuclideanDistance_Vectorized(a, b, 5);

    ASSERT_NEAR(sim, 0.0, 0.0001);
}

TEST(distances, euclidean_vectorized) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {2.5, 8.5, 2.0, 1.0, 3.0};

    double sim = EuclideanDistance_Vectorized(a, b, 5);
    ASSERT_NEAR(sim, 7.64852927, 0.0001);

    double sim2 = EuclideanDistance_Vectorized(b, a, 5);
    ASSERT_NEAR(sim2, sim, 0.0001);
}

TEST(distances, euclidean_similar_fma) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {1.0, 2.0, 3.0, 4.0, 5.0};

    double sim = EuclideanDistance_FMA(a, b, 5);

    ASSERT_NEAR(sim, 0.0, 0.0001);
}

TEST(distances, euclidean_fma) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {2.5, 8.5, 2.0, 1.0, 3.0};

    double sim = EuclideanDistance_FMA(a, b, 5);
    ASSERT_NEAR(sim, 7.64852927, 0.0001);

    double sim2 = EuclideanDistance_FMA(b, a, 5);
    ASSERT_NEAR(sim2, sim, 0.0001);
}

TEST(distances, euclidean_fma_size_17) {
    double a[17] = {1.0, 2.0, 3.0, 4.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0};
    double b[17] = {2.5, 8.5, 2.0, 1.0, 3.0, 4.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0};

    double sim = EuclideanDistance_FMA(a, b, 17);
    ASSERT_NEAR(sim, 15.508062419270823, 0.0001);

    double sim2 = EuclideanDistance_FMA(b, a, 17);
    ASSERT_NEAR(sim2, sim, 0.0001);
}

TEST(distances, euclidean_fma_size_21) {
    double a[21] = {1.0, 2.0, 3.0, 4.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0};
    double b[21] = {2.5, 8.5, 2.0, 1.0, 3.0, 4.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0, 8.5, 2.0, 1.0, 3.0};

    double sim = EuclideanDistance_FMA(a, b, 21);
    ASSERT_NEAR(sim, 18.048545647780045, 0.0001);

    double sim2 = EuclideanDistance_FMA(b, a, 21);
    ASSERT_NEAR(sim2, sim, 0.0001);
}

TEST(distances, euclidean_fma_size_42) {
    double a[42] = {6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0, 2.5, 8.5, 2.0, 1.0, 3.0, 4.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0, 8.5, 2.0, 1.0, 3.0, 2.0, 3.0, 4.0, 5.0, 3.5, 10.0};
    double b[42] = {2.5, 8.5, 2.0, 1.0, 3.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0, 8.5, 2.0, 1.0, 3.0, 2.5, 8.5, 2.0, 1.0, 3.0, 4.0, 5.0, 3.5, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0, 8.5, 2.0, 1.0, 3.0, 3.0, 7.5, 8.5};

    double sim = EuclideanDistance_FMA(a, b, 42);
    ASSERT_NEAR(sim, 22.847319317591726, 0.0001);

    double sim2 = EuclideanDistance_FMA(b, a, 42);
    ASSERT_NEAR(sim2, sim, 0.0001);
}
#endif //__AVX2__
