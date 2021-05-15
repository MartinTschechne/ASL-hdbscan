#include "gtest/gtest.h"
#include <distances/cosine_similarity.h>

TEST(distances, cosine_similarity_similar) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {1.0, 2.0, 3.0, 4.0, 5.0};

    double sim = CosineSimilarity(a, b, 5);

    ASSERT_NEAR(sim, 0.0, 0.0001);
}

TEST(distances, cosine_similarity_ortho) {
    double a[3] = {2.0, -4.2, -3.15};
    double b[3] = {-21.84, -21.155, 14.34};

    double sim = CosineSimilarity(a, b, 5);

    ASSERT_NEAR(sim, 1.0, 0.0001);
}

TEST(distances, cosine_similarity1) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {2.5, 8.5, 2.0, 1.0, 3.0};

    double sim = CosineSimilarity(a, b, 5);

    ASSERT_NEAR(sim, 0.376110322, 0.0001);
}

TEST(distances, cosine_similarity_similar_unrolled) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {1.0, 2.0, 3.0, 4.0, 5.0};

    double sim = CosineSimilarityUnrolled(a, b, 5);

    ASSERT_NEAR(sim, 0.0, 0.0001);
}

TEST(distances, cosine_similarity_ortho_unrolled) {
    double a[3] = {2.0, -4.2, -3.15};
    double b[3] = {-21.84, -21.155, 14.34};

    double sim = CosineSimilarityUnrolled(a, b, 3);

    ASSERT_NEAR(sim, 1.0, 0.0001);
}

TEST(distances, cosine_similarity1_unrolled) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {2.5, 8.5, 2.0, 1.0, 3.0};

    double sim = CosineSimilarityUnrolled(a, b, 5);

    ASSERT_NEAR(sim, 0.376110322, 0.0001);
}
