#include "gtest/gtest.h"
#include <hdbscan/constraint.h>

TEST(contraint, constraint_basic) {
    size_t point_a = 234;
    size_t point_b = 2389;
    Constraint a(point_a, point_b, Constraint::CONSTRAINT_TYPE::CANNOT_LINK);
    Constraint b(point_a, point_b, Constraint::CONSTRAINT_TYPE::MUST_LINK);

    ASSERT_EQ(a.GetPointA(), point_a);
    ASSERT_EQ(a.GetPointB(), point_b);
    ASSERT_EQ(a.GetType(), Constraint::CONSTRAINT_TYPE::CANNOT_LINK);
    ASSERT_EQ(b.GetType(), Constraint::CONSTRAINT_TYPE::MUST_LINK);
}