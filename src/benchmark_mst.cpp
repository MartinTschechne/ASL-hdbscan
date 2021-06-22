#include <benchmark/benchmark_mst.h>
#include <cassert>
#include <hdbscan/HDBSCAN_star.h>
#include <filesystem>

DEFINE_bool(disable_nobitset, false, "Whether or not to disable no bitset functions (they take more memory)");

void RunBenchmarking(RunnerConfig config) {
    DistanceCalculator dist_fun = GetDistanceCalculator(config.dist_function, config.optimization_level);
    assert(config.hierarchy_file != "");
    assert(config.tree_file != "");
    assert(config.partition_file != "");
    assert(config.visualization_file != "");
    assert(config.outlier_score_file != "");
    assert(config.num_points > 0 && config.num_dimensions > 0);

    std::filesystem::create_directory("mst_timings");

    size_t num_points = config.num_points;
    size_t num_dimensions = config.num_dimensions;
    double* data_set = ReadInDataSet(config.points_file, ',', num_points, num_dimensions);

    Vector* constraints = nullptr;
    if(config.constraints != "") {
        constraints = ReadInConstraints(config.constraints);
    }

    double** distance_matrix = nullptr;
    CalculateCoreDistances_t calculate_core_distances_f = GetCalculateCoreDistancesFunction("symmetry");
    double* core_distances = calculate_core_distances_f(data_set, config.num_neighbors, dist_fun, num_points, num_dimensions, distance_matrix); "\n";

    UndirectedGraph_C* mst = ConstructMST(data_set, core_distances, true, dist_fun, num_points, num_dimensions, distance_matrix);
    UDG_Free(mst);
    mst = ConstructMST_Unrolled_Bitset(data_set, core_distances, true, dist_fun, num_points, num_dimensions, distance_matrix);
    UDG_Free(mst);
    mst = ConstructMST_Bitset_NoCalc(data_set, core_distances, true, dist_fun, num_points, num_dimensions, distance_matrix);
    UDG_Free(mst);
    mst = ConstructMST_Unrolled_Bitset_NoCalc(data_set, core_distances, true, dist_fun, num_points, num_dimensions, distance_matrix);
    UDG_Free(mst);
    #ifdef __AVX2__
    mst = ConstructMST_Bitset_NoCalc_AVX(data_set, core_distances, true, dist_fun, num_points, num_dimensions, distance_matrix);
    UDG_Free(mst);
    mst = ConstructMST_Bitset_NoCalc_AVX_Unrolled_2(data_set, core_distances, true, dist_fun, num_points, num_dimensions, distance_matrix);
    UDG_Free(mst);
    mst = ConstructMST_Bitset_NoCalc_AVX_Unrolled_4(data_set, core_distances, true, dist_fun, num_points, num_dimensions, distance_matrix);
    UDG_Free(mst);
    #endif //__AVX2__

    if(!FLAGS_disable_nobitset) {
        mst = ConstructMST_Unrolled_NoBitset_CalcDistances(data_set, core_distances, true, dist_fun, num_points, num_dimensions, distance_matrix);
        UDG_Free(mst);
        mst = ConstructMST_Unrolled_NoBitset_NoCalcDistances(data_set, core_distances, true, dist_fun, num_points, num_dimensions, distance_matrix);
        UDG_Free(mst);
        #ifdef __AVX2__
        mst = ConstructMST_NoBitset_NoCalcDistances_AVX256(data_set, core_distances, true, dist_fun, num_points, num_dimensions, distance_matrix);
        UDG_Free(mst);
        #endif //__AVX2__
    }
}

int main(int argc, char** argv) {
    if (argc == 0) {
        gflags::ShowUsageWithFlags(argv[0]);
        exit(1);
    }
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    RunBenchmarking(RunnerConfigFromFlags());
    return 0;
}
