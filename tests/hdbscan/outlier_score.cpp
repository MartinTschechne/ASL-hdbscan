#include "gtest/gtest.h"
#include <hdbscan/outlier_score.h>

TEST(outlier_score, basic_functionality) {
    double score = 10.0;
    double dist = 3.74;
    size_t id = 9234;
    OutlierScore outlier_score(score, dist, id);

    ASSERT_EQ(outlier_score.GetScore(), score);
    ASSERT_EQ(outlier_score.GetId(), id);
    ASSERT_EQ(outlier_score.CompareTo(outlier_score), 0);
    ASSERT_EQ(outlier_score.CompareTo(&outlier_score), 0);
}

TEST(outlier_score, order) {
    double score = 10.0;
    double dist = 3.74;
    size_t id = 9234;
    OutlierScore outlier_score(score, dist, id);

    ASSERT_EQ(outlier_score.CompareTo(OutlierScore(score - 1.0, dist, id)), 1);
    ASSERT_EQ(outlier_score.CompareTo(OutlierScore(score + 1.0, dist, id)), -1);

    ASSERT_EQ(outlier_score.CompareTo(OutlierScore(score, dist - 1.0, id)), 1);
    ASSERT_EQ(outlier_score.CompareTo(OutlierScore(score, dist + 1.0, id)), -1);

    ASSERT_EQ(outlier_score.CompareTo(OutlierScore(score, dist, id-3)), 3);
    ASSERT_EQ(outlier_score.CompareTo(OutlierScore(score, dist, id+3)), -3);
}