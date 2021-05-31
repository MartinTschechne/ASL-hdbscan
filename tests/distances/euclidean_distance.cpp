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

TEST(distances, euclidean1_4unrolled) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {2.5, 8.5, 2.0, 1.0, 3.0};

    double sim = EuclideanDistance_4Unrolled(a, b, 5);
    ASSERT_NEAR(sim, 7.64852927, 0.0001);

    double sim2 = EuclideanDistance_4Unrolled(b, a, 5);
    ASSERT_NEAR(sim2, sim, 0.0001);
}

TEST(distances, euclidean_similar_vectorized) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {1.0, 2.0, 3.0, 4.0, 5.0};

    double sim = EuclideanDistance_Vectorized(a, b, 5);

    ASSERT_NEAR(sim, 0.0, 0.0001);
}

TEST(distances, euclidean1_vectorized) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {2.5, 8.5, 2.0, 1.0, 3.0};

    double sim = EuclideanDistance_Vectorized(a, b, 5);
    ASSERT_NEAR(sim, 7.64852927, 0.0001);

    double sim2 = EuclideanDistance_Vectorized(b, a, 5);
    ASSERT_NEAR(sim2, sim, 0.0001);
}
