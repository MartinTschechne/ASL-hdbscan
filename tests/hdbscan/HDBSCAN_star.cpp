#include <gtest/gtest.h>
#include <hdbscan/HDBSCAN_star.h>
#include <distances/euclidian_distance.h>
#include <fstream>
#include <filesystem>

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
    std::filesystem::create_directory("test_output");
    std::string file_name = "../tests/data/example_data_set.csv";
    std::string file_name_constraints = "../tests/data/example_constraints.csv";

    std::string hierarchy_ref_file = "../tests/data/example_results/example_data_set_compact_hierarchy.csv";
    std::string tree_ref_file = "../tests/data/example_results/example_data_set_tree.csv";
    std::string flat_ref_file = "../tests/data/example_results/example_data_set_partition.csv";
    std::string outlier_ref_file = "../tests/data/example_results/example_data_set_outlier_scores.csv";


    size_t min_pts = 8;
    size_t min_cluster_size = 8;
    bool compact = true;


    auto data_set = HDBSCANStar::ReadInDataSet(file_name, ',');
    std::vector<double> core_distances;

    HDBSCANStar::CalculateCoreDistances(data_set, min_pts, EuclidianDistance, core_distances);
    UndirectedGraph mst = HDBSCANStar::ConstructMST(data_set, core_distances, true, EuclidianDistance);
    mst.QuicksortByEdgeWeight();
    size_t num_points = data_set.size();
    double* point_noise_levels = new double[num_points];
    size_t* point_last_clusters = new size_t[num_points];
    auto constraints = HDBSCANStar::ReadInConstraints(file_name_constraints);

    std::vector<Cluster*> clusters;
    ASSERT_NO_THROW(
        HDBSCANStar::ComputeHierarchyAndClusterTree(mst, min_cluster_size, compact, constraints, 
            "test_output/hierarchy.csv", "test_output/tree.csv", ',', point_noise_levels, point_last_clusters, 
            "test_output/visualization.vis", clusters)
    );

    bool inf_stability = HDBSCANStar::PropagateTree(clusters);

    std::vector<size_t> res;
    std::vector<OutlierScore> outlier_scores;
    ASSERT_NO_THROW(HDBSCANStar::FindProminentClusters(clusters, "test_output/hierarchy.csv", "test_output/flat.csv", ',', num_points, inf_stability, res));
    ASSERT_NO_THROW(HDBSCANStar::CalculateOutlierScores(clusters, point_noise_levels, num_points, point_last_clusters, core_distances.data(), "test_output/outlier_score.csv", ',', inf_stability, outlier_scores));

    // check hierarchy 
    std::fstream hierarchy_ours("test_output/hierarchy.csv");
    std::fstream hierarchy_ref(hierarchy_ref_file);

    std::string line, other;
    std::vector<std::string> ours;
    std::vector<std::string> ref;

    while(std::getline(hierarchy_ours, line)) {
        ours.push_back(line);
    }
    while(std::getline(hierarchy_ref, line)) {
        ref.push_back(line);
    }
    for(size_t i = 0; i < ref.size(); ++i) {
        std::stringstream stream_ours(ours[i]); 
        std::stringstream stream_ref(ref[i]);

        size_t count = 0;

        while(std::getline(stream_ref, line, ',')) {
            ASSERT_TRUE(std::getline(stream_ours, other, ',')) << count;
            if(count == 0) {
                ASSERT_NEAR(std::stod(line), std::stod(other), 0.001);
            } else {
                ASSERT_EQ(line, other);
            }

            count++;
        }
    }

    // Check tree
    ours.clear();
    ref.clear();

    std::fstream tree_ours("test_output/tree.csv");
    std::fstream tree_ref(tree_ref_file);
    while(std::getline(tree_ours, line)) {
        ours.push_back(line);
    }
    while(std::getline(tree_ref, line)) {
        ref.push_back(line);
    }
    for(size_t i = 0; i < ref.size(); ++i) {
        std::stringstream stream_ours(ours[i]); 
        std::stringstream stream_ref(ref[i]);

        size_t count = 0;

        while(std::getline(stream_ref, line, ',')) {
            ASSERT_TRUE(std::getline(stream_ours, other, ',')) << count;
            if(count == 6) { // we can't compare number of chars written with the java version
                count++;
                continue;
            }
            if(line == "nan" || line == "NaN") {
                ASSERT_TRUE(other == "nan" || other == "NaN");
            } else {
                ASSERT_NEAR(std::stod(line), std::stod(other), 0.001);
            }
            count++;
        }
    }

    //Check flat 
    std::fstream flat_ours("test_output/flat.csv");
    std::fstream flat_ref(flat_ref_file);
    while(std::getline(flat_ref, line)) {
        ASSERT_TRUE(std::getline(flat_ours, other));
        ASSERT_EQ(line, other);
    }

    //Check outlier 
    std::fstream outlier_ours("test_output/outlier_score.csv");
    std::fstream outlier_ref(outlier_ref_file);

    std::map<size_t, double> scores_ref;
    std::map<size_t, double> scores_ours;
    while(std::getline(outlier_ref, line)) {
        std::stringstream stream(line);    
        double value;
        size_t label;
        char delim;
        stream >> value >> delim >> label;
        scores_ref.insert({label, value});    
    }
    while(std::getline(outlier_ours, line)) {
        std::stringstream stream(line);    
        double value;
        size_t label;
        char delim;
        stream >> value >> delim >> label;
        scores_ours.insert({label, value});    
    }
    for(auto& elem : scores_ref) {
        ASSERT_NEAR(elem.second, scores_ours.at(elem.first), 0.0001);
    }
    for(auto& elem : scores_ours) {
        ASSERT_NEAR(elem.second, scores_ref.at(elem.first), 0.0001);
    }


    delete[] point_last_clusters;
    delete[] point_noise_levels;
}