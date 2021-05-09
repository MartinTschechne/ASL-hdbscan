#include <hdbscan/HDBSCAN_star_runner.h>
#include <gflags/gflags.h>

int main(int argc, char** argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    HDBSCANRunner(RunnerConfigFromFlags());
    return 0;
}