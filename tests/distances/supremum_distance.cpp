#include "gtest/gtest.h"
#include <distances/supremum_distance.h>

TEST(distances, supremum_similar) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {1.0, 2.0, 3.0, 4.0, 5.0};

    double sim = SupremumDistance(a, b, 5);

    ASSERT_NEAR(sim, 0.0, 0.0001);
}

TEST(distances, supremum1) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {2.5, 8.5, 2.0, 1.0, 3.0};

    double sim = SupremumDistance(a, b, 5);

    ASSERT_NEAR(sim, 6.5, 0.0001);

    double sim2 = SupremumDistance(b, a, 5);
    ASSERT_NEAR(sim2, sim, 0.0001);
}

TEST(distances, supremum_similar_unrolled) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {1.0, 2.0, 3.0, 4.0, 5.0};

    double sim = SupremumDistanceUnrolled(a, b, 5);

    ASSERT_NEAR(sim, 0.0, 0.0001);
}

TEST(distances, supremum1_unrolled) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {2.5, 8.5, 2.0, 1.0, 3.0};

    double sim = SupremumDistanceUnrolled(a, b, 5);

    ASSERT_NEAR(sim, 6.5, 0.0001);

    double sim2 = SupremumDistanceUnrolled(b, a, 5);
    ASSERT_NEAR(sim2, sim, 0.0001);
}

TEST(distances, supremum_similar_4unrolled) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {1.0, 2.0, 3.0, 4.0, 5.0};

    double sim = SupremumDistance_4Unrolled(a, b, 5);

    ASSERT_NEAR(sim, 0.0, 0.0001);
}

TEST(distances, supremum_4unrolled) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {2.5, 8.5, 2.0, 1.0, 3.0};

    double sim = SupremumDistance_4Unrolled(a, b, 5);

    ASSERT_NEAR(sim, 6.5, 0.0001);

    double sim2 = SupremumDistance_4Unrolled(b, a, 5);
    ASSERT_NEAR(sim2, sim, 0.0001);
}

#ifdef __AVX2__
TEST(distances, supremum_similar_vectorized) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {1.0, 2.0, 3.0, 4.0, 5.0};

    double sim = SupremumDistance_Vectorized(a, b, 5);

    ASSERT_NEAR(sim, 0.0, 0.0001);
}

TEST(distances, supremum_vectorized) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {2.5, 8.5, 2.0, 1.0, 3.0};

    double sim = SupremumDistance_Vectorized(a, b, 5);

    ASSERT_NEAR(sim, 6.5, 0.0001);

    double sim2 = SupremumDistance_Vectorized(b, a, 5);
    ASSERT_NEAR(sim2, sim, 0.0001);
}

TEST(distances, supremum_similar_4unrolled_vectorized) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {1.0, 2.0, 3.0, 4.0, 5.0};

    double sim = SupremumDistance_4Unrolled_Vectorized(a, b, 5);

    ASSERT_NEAR(sim, 0.0, 0.0001);
}

TEST(distances, supremum_4unrolled_vectorized) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {2.5, 8.5, 2.0, 1.0, 3.0};

    double sim = SupremumDistance_4Unrolled_Vectorized(a, b, 5);

    ASSERT_NEAR(sim, 6.5, 0.0001);

    double sim2 = SupremumDistance_4Unrolled_Vectorized(b, a, 5);
    ASSERT_NEAR(sim2, sim, 0.0001);
}

TEST(distances, supremum_4unrolled_vectorized_size_17) {
    double a[17] = {1.0, 2.0, 3.0, 4.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0};
    double b[17] = {2.5, 8.5, 2.0, 1.0, 3.0, 4.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0};

    double sim = SupremumDistance_4Unrolled_Vectorized(a, b, 17);
    ASSERT_NEAR(sim, 6.5, 0.0001);

    double sim2 = SupremumDistance_4Unrolled_Vectorized(b, a, 17);
    ASSERT_NEAR(sim2, sim, 0.0001);
}

TEST(distances, supremum_4unrolled_vectorized_size_21) {
    double a[21] = {1.0, 2.0, 3.0, 4.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0};
    double b[21] = {2.5, 8.5, 2.0, 1.0, 3.0, 4.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0, 8.5, 2.0, 1.0, 3.0};

    double sim = SupremumDistance_4Unrolled_Vectorized(a, b, 21);
    ASSERT_NEAR(sim, 6.5, 0.0001);

    double sim2 = SupremumDistance_4Unrolled_Vectorized(b, a, 21);
    ASSERT_NEAR(sim2, sim, 0.0001);
}

TEST(distances, supremum_4unrolled_vectorized_size_42) {
    double a[42] = {6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0, 2.5, 8.5, 2.0, 1.0, 3.0, 4.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0, 8.5, 2.0, 1.0, 3.0, 2.0, 3.0, 4.0, 5.0, 3.5, 10.0};
    double b[42] = {2.5, 8.5, 2.0, 1.0, 3.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0, 8.5, 2.0, 1.0, 3.0, 2.5, 8.5, 2.0, 1.0, 3.0, 4.0, 5.0, 3.5, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0, 8.5, 2.0, 1.0, 3.0, 3.0, 7.5, 8.5};

    double sim = SupremumDistance_4Unrolled_Vectorized(a, b, 42);
    ASSERT_NEAR(sim, 9.0, 0.0001);

    double sim2 = SupremumDistance_4Unrolled_Vectorized(b, a, 42);
    ASSERT_NEAR(sim2, sim, 0.0001);
}
#endif //__AVX2__
