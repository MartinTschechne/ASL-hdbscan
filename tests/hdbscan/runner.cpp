#include <gtest/gtest.h>
#include <hdbscan/HDBSCAN_star_runner.h>
#include <distances/euclidian_distance.h>
#include <filesystem>
#include <fstream>

TEST(HDBSCAN_Runner, full_pipeline) {
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

    RunnerConfig runner_config = {
        500,
        2,
        file_name,
        file_name_constraints,
        "test_output/hierarchy_runner.csv",
        "test_output/tree_runner.csv",
        "test_output/visualization.csv",
        "test_output/partition_runner.csv",
        "test_output/outlier_score_runner.csv",
        min_pts,
        min_cluster_size,
        compact,
        "euclidean"
    };

    HDBSCANRunner(runner_config);

    // check hierarchy 
    std::fstream hierarchy_ours("test_output/hierarchy_runner.csv");
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

    std::fstream tree_ours("test_output/tree_runner.csv");
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
    std::fstream flat_ours("test_output/partition_runner.csv");
    std::fstream flat_ref(flat_ref_file);
    while(std::getline(flat_ref, line)) {
        ASSERT_TRUE(std::getline(flat_ours, other));
        ASSERT_EQ(line, other);
    }

    //Check outlier 
    std::fstream outlier_ours("test_output/outlier_score_runner.csv");
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
}

TEST(HDBSCAN_Runner, full_pipeline_high_dim) {
    std::filesystem::create_directory("test_output");
    std::string file_name = "../data/unit_tests/example_high_dim_10000_32.csv";
    std::string file_name_constraints = "";

    std::string hierarchy_ref_file = "../data/unit_tests/example_high_dim_10000_32_compact_hierarchy.csv";
    std::string tree_ref_file = "../data/unit_tests/example_high_dim_10000_32_tree.csv";
    std::string flat_ref_file = "../data/unit_tests/example_high_dim_10000_32_partition.csv";
    std::string outlier_ref_file = "../data/unit_tests/example_high_dim_10000_32_outlier_scores.csv";


    size_t min_pts = 8;
    size_t min_cluster_size = 8;
    bool compact = true;

    RunnerConfig runner_config = {
        10000,
        32,
        file_name,
        file_name_constraints,
        "test_output/hierarchy_runner_large.csv",
        "test_output/tree_runner_large.csv",
        "test_output/visualization_large.csv",
        "test_output/partition_runner_large.csv",
        "test_output/outlier_score_runner_large.csv",
        min_pts,
        min_cluster_size,
        compact,
        "euclidean"
    };

    HDBSCANRunner(runner_config);

    // check hierarchy 
    std::fstream hierarchy_ours("test_output/hierarchy_runner_large.csv");
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

    std::fstream tree_ours("test_output/tree_runner_large.csv");
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
    std::fstream flat_ours("test_output/partition_runner_large.csv");
    std::fstream flat_ref(flat_ref_file);
    while(std::getline(flat_ref, line)) {
        ASSERT_TRUE(std::getline(flat_ours, other));
        ASSERT_EQ(line, other);
    }

    //Check outlier 
    std::fstream outlier_ours("test_output/outlier_score_runner_large.csv");
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
}