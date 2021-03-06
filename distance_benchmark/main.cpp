#include <iostream>
#include <fstream>
#include <random>
#include <string>
#include "../src/include/distances/distances.h"
#include "../src/include/benchmark/tsc_x86.h"

#define NUM_RUNS 1
#define CYCLES_REQUIRED 1e8
#define CALIBRATE

typedef double (*fptr)(const double*, const double*, const size_t);

struct func_array {
    fptr func;
    std::string fname;
};

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

func_array* get_distance_functions(std::string f_name) {
    if (f_name == "euclidean") {
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
    } else if (f_name == "cosine") {
        static func_array ds[] = {
                {CosineSimilarity, "Baseline"},
                {CosineSimilarityUnrolled, "2-fold Unrolled"},
                {CosineSimilarity_4Unrolled, "4-fold Unrolled"},
            #ifdef __AVX2__
                {CosineSimilarity_Vectorized, "AVX Vectorized"},
                {CosineSimilarity_Vectorized_FMA, "AVX Vectorized FMA"}
            #endif //__AVX2__
        };
        return ds;
    } else if (f_name == "manhattan") {
        static func_array ds[] = {
                {ManhattanDistance, "Baseline"},
                {ManhattanDistanceUnrolled, "2-fold Unrolled"},
                {ManhattanDistanceUnrolled, "4-fold Unrolled"},
            #ifdef __AVX2__
                {ManhattanDistance_Vectorized, "AVX Vectorized"}
            #endif //__AVX2__
        };
        return ds;
    } else if (f_name == "pearson") {
        static func_array ds[] = {
                {PearsonCorrelation, "Baseline"},
                {PearsonCorrelationUnrolled, "2-fold Unrolled"},
                {PearsonCorrelation_4Unrolled, "4-fold Unrolled"},
                // {PearsonCorrelation_Onepass, "Single Pass"
                // },
                // {PearsonCorrelation_4UnrolledOnepass, "Single Pass 4-fold Unrolled"},
            #ifdef __AVX2__
                {PearsonCorrelation_Vectorized, "AVX Vectorized"},
                {PearsonCorrelation_FMA, "AVX Vectorized FMA"}
            #endif //__AVX2__
        };
        return ds;
    } else if (f_name == "supremum") {
        static func_array ds[] = {
                {SupremumDistance, "Baseline"},
                {SupremumDistanceUnrolled, "2-fold Unrolled"},
                {SupremumDistance_4Unrolled, "4-fold Unrolled"},
            #ifdef __AVX2__
                {SupremumDistance_Vectorized, "AVX Vectorized"},
                {SupremumDistance_4Unrolled_Vectorized, "AVX Vectorized 4-fold Unrolled"}
            #endif //__AVX2__
        };
        return ds;
    } else {
        printf("Function name not found.\n");
        exit(1);
    }
}

int main(int argc, char const *argv[]) {
    if (argc < 4) {
        printf("Too few arguments!\nPlease specify NUM_POINTS, DIM and function name {euclidean, cosine, manhattan, pearson, supremum}.\n");
        exit(1);
    }
    const size_t NUM_POINTS = std::atoi(argv[1]);
    const size_t DIM = std::atoi(argv[2]);

    double* data_a = (double*)malloc(NUM_POINTS*DIM*sizeof(double));
    fill_data(data_a, NUM_POINTS, DIM);
    double* data_b = (double*)malloc(NUM_POINTS*DIM*sizeof(double));
    fill_data(data_b, NUM_POINTS, DIM);

    func_array* d_funcs = get_distance_functions(argv[3]);

    double d, baseline;
    int i, num_runs;
    myInt64 cycles;
    myInt64 start;
    num_runs = NUM_RUNS;
    for (size_t f = 0; f < 4; ++f) {
#ifdef CALIBRATE
        while(num_runs < (1 << 14)) {
            start = start_tsc();
            for (i = 0; i < num_runs; ++i) {
                for (size_t n = 0; n < NUM_POINTS; ++n) {
                    (*d_funcs[f].func)(&data_a[n*DIM], &data_b[n*DIM], DIM);
                }
            }
            cycles = stop_tsc(start);

            if(cycles >= CYCLES_REQUIRED) break;

            num_runs *= 2;
        }
#endif

        start = start_tsc();
        for (i = 0; i < num_runs; ++i) {
            for (size_t n = 0; n < NUM_POINTS; ++n) {
                (*d_funcs[f].func)(&data_a[n*DIM], &data_b[n*DIM], DIM);
            }
        }
        cycles = stop_tsc(start)/(num_runs*NUM_POINTS);
        printf("%s,%li,%li,%lli\n", d_funcs[f].fname.c_str(), NUM_POINTS, DIM, cycles);

    }

    free(data_a);
    free(data_b);
    return 0;
}
