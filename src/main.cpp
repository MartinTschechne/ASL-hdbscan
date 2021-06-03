#include <hdbscan/HDBSCAN_star_runner.h>
#include <gflags/gflags.h>
#include <iostream>
#include <fstream>

#include <benchmark/tsc_x86.h>

int main(int argc, char** argv) {
    if (argc == 1) {
        gflags::ShowUsageWithFlags(argv[0]);
        exit(1);
    }
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    long int start = start_tsc();
    HDBSCANRunner(RunnerConfigFromFlags());
    long int cycles = stop_tsc(start);

    std::ofstream benchmark_runner;
    benchmark_runner.open("measurements_runner.txt", std::ios_base::app);
    benchmark_runner << "total," << cycles << "\n";
    benchmark_runner.close();

    return 0;
}
