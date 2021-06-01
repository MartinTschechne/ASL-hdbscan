#include <gtest/gtest.h>
#include <hdbscan/HDBSCAN_star.h>
#include <distances/euclidean_distance.h>
#include <distances/supremum_distance.h>
#include <fstream>
#include <filesystem>

TEST(HDBSCAN_Star, IO) {
    std::string file_name = "../tests/data/read_test.csv";
    std::string file_name_constraints = "../tests/data/example_constraints.csv";
    size_t num_pts = 3;
    size_t dim = 2;
    auto lines = ReadInDataSet(file_name, ',', num_pts, dim);

    ASSERT_EQ(lines[0][0], 1.0);
    ASSERT_EQ(lines[2][1], 90);

    auto constraints = ReadInConstraints(file_name_constraints);
    ASSERT_EQ(constraints->size, 10);
    ASSERT_EQ(((Constraint*)constraints->elements[3])->point_a, 304);
    ASSERT_EQ(((Constraint*)constraints->elements[8])->point_b, 455);
    ASSERT_EQ(((Constraint*)constraints->elements[5])->type, Constraint::CONSTRAINT_TYPE::CANNOT_LINK);

    FreeDataset(lines, num_pts);
}

TEST(HDBSCAN_Star, core_distances) {
    std::string file_name = "../tests/data/example_data_set.csv";
    size_t num_pts = 500;
    size_t dim = 2;
    auto data_set = ReadInDataSet(file_name, ',', num_pts, dim);

    double** d = nullptr;
    double* core_distances = CalculateCoreDistancesNoOptimization(data_set, 3, EuclideanDistance, num_pts, dim, d);
    ASSERT_DOUBLE_EQ(core_distances[0], 0.2049413436136352);

    FreeDataset(data_set, num_pts);
    delete[] core_distances;
}

TEST(HDBSCAN_Star, core_distances_symmetry) {
    std::string file_name = "../tests/data/example_data_set.csv";
    size_t num_pts = 500;
    size_t dim = 2;
    auto data_set = ReadInDataSet(file_name, ',', num_pts, dim);

    double** d = nullptr;
    double* core_distances_symmetry = CalculateCoreDistancesSymmetry(data_set, 3, EuclideanDistance, num_pts, dim, d);
    double* core_distances = CalculateCoreDistancesNoOptimization(data_set, 3, EuclideanDistance, num_pts, dim, d);

    ASSERT_DOUBLE_EQ(core_distances_symmetry[0], 0.2049413436136352);

    double sim = SupremumDistance(core_distances, core_distances_symmetry, num_pts);
    ASSERT_NEAR(sim, 0.0, 0.0001);

    FreeDataset(data_set, num_pts);
    delete[] core_distances;
    delete[] core_distances_symmetry;
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
    size_t num_pts = 500;
    size_t dim = 2;
    bool compact = true;

    auto data_set = ReadInDataSet(file_name, ',', num_pts, dim);

    double** distance_matrix;
    CalculateCoreDistances_t calculate_core_distances_f = GetCalculateCoreDistancesFunction("no_optimization");
    double* core_distances = calculate_core_distances_f(data_set, 8, EuclideanDistance, num_pts, dim, distance_matrix);
    UndirectedGraph_C* mst = ConstructMST(data_set, core_distances, true, EuclideanDistance, num_pts, dim, distance_matrix);
    UDG_QuicksortByEdgeWeight(mst);

    double* point_noise_levels = new double[num_pts];
    size_t* point_last_clusters = new size_t[num_pts];
    auto constraints = ReadInConstraints(file_name_constraints);

    Vector* clusters = new Vector;
    vector_init(clusters);
    ASSERT_NO_THROW(
        ComputeHierarchyAndClusterTree(mst, min_cluster_size, compact, constraints,
            "test_output/hierarchy.csv", "test_output/tree.csv", ',', point_noise_levels, point_last_clusters,
            "test_output/visualization.vis", clusters)
    );

    UDG_Free(mst);

    bool inf_stability = PropagateTree(clusters);

    vector* res = vector_create();
    vector* outlier_scores = vector_create();
    ASSERT_NO_THROW(FindProminentClusters(clusters, "test_output/hierarchy.csv", "test_output/flat.csv", ',', num_pts, inf_stability, res));
    ASSERT_NO_THROW(CalculateOutlierScores(clusters, point_noise_levels, num_pts, point_last_clusters, core_distances, "test_output/outlier_score.csv", ',', inf_stability, outlier_scores));

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

    FreeDataset(data_set, num_pts);
    delete[] core_distances;
}
