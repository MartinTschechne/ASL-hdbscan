#include "gtest/gtest.h"
#include <iostream>
#include <string>

void PrintOptimizationInfo() {
    std::string info = "";
#if defined (OPTIMIZATION_VECTORISE)
    info += "Vectorization enabled.\n";
#elif defined (OPTIMIZATION_UNROLL)
    info += "Unrolling enabled.\n";
#elif defined (OPTIMIZATION_SYMMETRY)
    info += "CalculateCoreDistances makes use of symmetry.\n";
#endif
    if(info == "") {
        std::cout << "No optimizations used!" << std::endl;
    } else {
        std::cout << info << std::endl;
    }
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    PrintOptimizationInfo();

    return RUN_ALL_TESTS();
}
