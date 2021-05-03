#include <gtest/gtest.h>
#include <hdbscan/HDBSCAN_star.h>
#include <distances/euclidian_distance.h>

TEST(HDBSCAN_Star, IO) {
    std::string file_name = "../tests/data/read_test.csv";
    std::string file_name_constraints = "../tests/data/example_constraints.csv";
    auto lines = HDBSCANStar::ReadInDataSet(file_name, ',');

    ASSERT_EQ(lines.size(), 3);
    ASSERT_EQ(lines[0].size(), 2);
    ASSERT_EQ(lines[0][0], 1.0);
    ASSERT_EQ(lines[2][1], 90);

    auto constraints = HDBSCANStar::ReadInConstraints(file_name_constraints);
    ASSERT_EQ(constraints.size(), 10);
    ASSERT_EQ(constraints[3].GetPointA(), 304);
    ASSERT_EQ(constraints[8].GetPointB(), 455);
    ASSERT_EQ(constraints[5].GetType(), Constraint::CONSTRAINT_TYPE::CANNOT_LINK);
}

TEST(HDBSCAN_Star, core_distances) {
    std::string file_name = "../tests/data/example_data_set.csv";
    auto data_set = HDBSCANStar::ReadInDataSet(file_name, ',');
    std::vector<double> core_distances;

    HDBSCANStar::CalculateCoreDistances(data_set, 3, EuclidianDistance, core_distances);
    ASSERT_EQ(core_distances.size(), data_set.size());
    ASSERT_DOUBLE_EQ(core_distances[0], 0.2049413436136352);
}

TEST(HDBSCAN_Star, create_tree) {
    std::string file_name = "../tests/data/example_data_set.csv";
    std::string file_name_constraints = "../tests/data/example_constraints.csv";
    auto data_set = HDBSCANStar::ReadInDataSet(file_name, ',');
    std::vector<double> core_distances;

    HDBSCANStar::CalculateCoreDistances(data_set, 3, EuclidianDistance, core_distances);
    
    ASSERT_NO_THROW(HDBSCANStar::ConstructMST(data_set, core_distances, true, EuclidianDistance));
    UndirectedGraph mst = HDBSCANStar::ConstructMST(data_set, core_distances, true, EuclidianDistance);
    ASSERT_NE(mst.GetNumEdges(), 0);
    mst.QuicksortByEdgeWeight();
    size_t num_points = data_set.size();
    double* point_noise_levels = new double[num_points];
    size_t* point_last_clusters = new size_t[num_points];
    auto constraints = HDBSCANStar::ReadInConstraints(file_name_constraints);

    std::vector<Cluster*> clusters;
    ASSERT_NO_THROW(
        HDBSCANStar::ComputeHierarchyAndClusterTree(mst, 4, true, constraints, 
            "hierarchy.txt", "tree.txt", ',', point_noise_levels, point_last_clusters, 
            "vis.txt", clusters)
    );

    bool inf_stability = HDBSCANStar::PropagateTree(clusters);

    std::vector<size_t> res;
    std::vector<OutlierScore> outlier_scores;
    ASSERT_NO_THROW(HDBSCANStar::FindProminentClusters(clusters, "hierarchy.txt", "flat.txt", ',', num_points, inf_stability, res));
    ASSERT_NO_THROW(HDBSCANStar::CalculateOutlierScores(clusters, point_noise_levels, num_points, point_last_clusters, core_distances.data(), "outlier_score.txt", ',', inf_stability, outlier_scores));

    delete[] point_last_clusters;
    delete[] point_noise_levels;
}