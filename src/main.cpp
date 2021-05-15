#include <hdbscan/HDBSCAN_star_runner.h>
#include <gflags/gflags.h>

int main(int argc, char** argv) {
    if (argc == 1) {
        gflags::ShowUsageWithFlags(argv[0]);
        exit(1);
    }
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    HDBSCANRunner(RunnerConfigFromFlags());
    return 0;
}