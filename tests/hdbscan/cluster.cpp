#include "gtest/gtest.h"
#include <hdbscan/cluster.h>

TEST(cluster, basic) {
    Cluster* cluster = CreateCluster(120, nullptr, 1.0, 10);

    set* points = set_create();
    set_insert(points, 12);
    set_insert(points, 3423);
    set_insert(points, 234);

    ASSERT_EQ(cluster->label, 120);
    ASSERT_ANY_THROW(DetachPoints(cluster, 12, 2.0));

    AddConstraintsSatisfied(cluster, 3);
    ASSERT_EQ(cluster->num_constraints_satisfied, 3);

    AddPointsToVirtualChildCluster(cluster, points);
    ASSERT_TRUE(VirtualChildClusterContaintsPoint(cluster, 234));
    ASSERT_FALSE(VirtualChildClusterContaintsPoint(cluster, 90));
    ReleaseVirtualChildCluster(cluster);
    ASSERT_FALSE(VirtualChildClusterContaintsPoint(cluster, 234));

    FreeCluster(cluster);
}

TEST(cluster, advanced) {
    Cluster* parent = CreateCluster(111, nullptr, 2.0, 15);
    Cluster* cluster = CreateCluster(120, parent, 1.0, 10);

    ASSERT_TRUE(parent->has_children);

    DetachPoints(cluster, 4, 2.0);
    AddConstraintsSatisfied(cluster, 5);
    Propagate(cluster);

    ASSERT_EQ(parent->propagated_num_constraints_satisfied, 5);
    ASSERT_EQ(parent->propagated_descendants->size, 1);

    FreeCluster(cluster);
    delete parent;
}