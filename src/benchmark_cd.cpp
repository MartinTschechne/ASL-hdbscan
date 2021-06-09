#include <benchmark/benchmark_cd.h>
#include <cassert>
#include <hdbscan/HDBSCAN_star.h>
#include <filesystem>
#include <distances/euclidean_distance.h>
#include <random>
#include <common/memory.h>
#include <benchmark/tsc_x86.h>

DEFINE_int64(num_runs, 2, "How often to run the function");
DEFINE_int64(num_points, 10, "How many points to include in the dataset");
DEFINE_int64(num_dims, 10, "How many dimensions the points should have");

#define CYCLES_REQUIRED 1e8
#define CALIBRATE

struct func_array {
    DistanceCalculator func;
    std::string fname;
};

func_array* get_distance_functions() {
    static func_array ds[] = {
            {EuclideanDistance, "Baseline"},
            {EuclideanDistanceUnrolled, "2-fold Unrolled"},
            {EuclideanDistance_4Unrolled, "4-fold Unrolled"},
        #ifdef __AVX2__
            {EuclideanDistance_Vectorized, "AVX Vectorized"},
            {EuclideanDistance_FMA, "AVX Vectorized FMA"}
        #endif //__AVX2__
    };
    return ds;
}



void fill_data(double* data, const size_t num_points, const size_t dim) {
    std::mt19937 gen;
    gen.seed(42);
    std::normal_distribution<> d{5,2};
    for (size_t i = 0; i < num_points; i++) {
      for (size_t j = 0; j < dim; j++) {
        data[i*dim + j] = d(gen);
      }
    }
}

void RunBenchmarking() {
    #ifdef __AVX2__
    size_t num_fun = 5;
    #else
    size_t num_fun = 3;
    #endif

    double* dataset = CreateAlignedDouble1D(FLAGS_num_dims*FLAGS_num_points);
    fill_data(dataset, FLAGS_num_points, FLAGS_num_dims);

    auto functions = get_distance_functions();
    double** dist_mat = CreateAlignedDouble2D(FLAGS_num_points, FLAGS_num_points);
    double* result;
    uint64_t num_runs = FLAGS_num_runs;
    uint64_t start, cycles;
    for(size_t f = 0; f < num_fun; ++f) {
        auto core_dist_fun = functions[f].func;
        num_runs = FLAGS_num_runs;
#ifdef CALIBRATE
        cycles = 0;
        while(num_runs < (1 << 14)) {
            start = start_tsc();
            for (size_t i = 0; i < num_runs; ++i) {
                result = CalculateCoreDistancesSymmetry(dataset, 5, core_dist_fun, FLAGS_num_points, FLAGS_num_dims, dist_mat);
                free(result);

            }
            cycles = stop_tsc(start);

            if(cycles >= CYCLES_REQUIRED) break;

            num_runs *= 2;
        }
#endif
        cycles = 0;
        for (size_t i = 0; i < FLAGS_num_runs; ++i) {
            start = start_tsc();
            result = CalculateCoreDistancesSymmetry(dataset, 5, core_dist_fun, FLAGS_num_points, FLAGS_num_dims, dist_mat);
            cycles += stop_tsc(start);
            free(result);
        }
        cycles /= (FLAGS_num_runs*FLAGS_num_points);
        printf("%s,%li,%li,%lu\n", functions[f].fname.c_str(), FLAGS_num_points, FLAGS_num_dims, cycles);
    }
    num_runs = FLAGS_num_runs;
#ifdef __AVX2__
#ifdef CALIBRATE
    cycles = 0;
    while(num_runs < (1 << 14)) {
            start = start_tsc();
            for (size_t i = 0; i < num_runs; ++i) {
                result = CalculateCoreDistancesSymmetry_blocked(dataset, 5, EuclideanDistance_FMA, FLAGS_num_points, FLAGS_num_dims, dist_mat);
                free(result);
            }
            cycles = stop_tsc(start);

            if(cycles >= CYCLES_REQUIRED) break;

            num_runs *= 2;
        }
#endif
        cycles = 0;
        for (size_t i = 0; i < FLAGS_num_runs; ++i) {
            start = start_tsc();
            result = CalculateCoreDistancesSymmetry_blocked(dataset, 5, EuclideanDistance_FMA, FLAGS_num_points, FLAGS_num_dims, dist_mat);
            cycles += stop_tsc(start);
            free(result);
        }
        cycles /= (FLAGS_num_runs*FLAGS_num_points);
        printf("%s,%li,%li,%lu\n", "Symmetry+Blocking", FLAGS_num_points, FLAGS_num_dims, cycles);

        num_runs = FLAGS_num_runs;
#ifdef CALIBRATE
    cycles = 0;
    while(num_runs < (1 << 14)) {
            start = start_tsc();
            for (size_t i = 0; i < num_runs; ++i) {
                result = CalculateCoreDistancesBlocked_Euclidean(dataset, 5, nullptr, FLAGS_num_points, FLAGS_num_dims, dist_mat);
                free(result);
            }
            cycles += stop_tsc(start);

            if(cycles >= CYCLES_REQUIRED) break;

            num_runs *= 2;
        }
#endif
        cycles = 0;
        for (size_t i = 0; i < FLAGS_num_runs; ++i) {
            start = start_tsc();
            result = CalculateCoreDistancesBlocked_Euclidean(dataset, 5, nullptr, FLAGS_num_points, FLAGS_num_dims, dist_mat);
            cycles += stop_tsc(start);
            free(result);
        }
        cycles /= (FLAGS_num_runs*FLAGS_num_points);
        printf("%s,%li,%li,%lu\n", "Blocking Optimized", FLAGS_num_points, FLAGS_num_dims, cycles);
#endif //__AVX2__
}


int main(int argc, char** argv) {
    if (argc == 0) {
        gflags::ShowUsageWithFlags(argv[0]);
        exit(1);
    }
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    RunBenchmarking();
    return 0;
}
