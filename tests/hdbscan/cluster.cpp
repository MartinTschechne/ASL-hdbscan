#include "gtest/gtest.h"
#include <hdbscan/cluster.h>

TEST(cluster, basic) {
    Cluster cluster(120, nullptr, 1.0, 10);

    std::set<size_t> points;
    points.insert(12);
    points.insert(3423);
    points.insert(234);

    ASSERT_EQ(cluster.GetLabel(), 120);
    ASSERT_ANY_THROW(cluster.DetachPoints(12, 2.0));

    cluster.AddConstraintsSatisfied(3);
    ASSERT_EQ(cluster.GetNumConstraintsSatisfied(), 3);

    cluster.AddPointsToVirtualChildCluster(points);
    ASSERT_TRUE(cluster.VirtualChildClusterContaintsPoint(234));
    ASSERT_FALSE(cluster.VirtualChildClusterContaintsPoint(90));
    cluster.ReleaseVirtualChildCluster();
    ASSERT_FALSE(cluster.VirtualChildClusterContaintsPoint(234));
}

TEST(cluster, advanced) {
    Cluster parent(111, nullptr, 2.0, 15);
    Cluster cluster(120, &parent, 1.0, 10);

    ASSERT_TRUE(parent.HasChildren());

    cluster.DetachPoints(4, 2.0);
    cluster.AddConstraintsSatisfied(5);
    cluster.Propagate();

    ASSERT_EQ(parent.GetPropagatedNumConstraintsSatisfied(), 5);
    ASSERT_EQ(parent.GetPropagatedDescendants().size(), 1);
}