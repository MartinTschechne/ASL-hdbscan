cmake_minimum_required(VERSION 3.13)
project(TEAM33_HDBSCAN)

set(CMAKE_CXX_STANDARD 20)

include(cmake/GetGTest.cmake)

include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/include
)

add_library(
    example
    src/example.cpp
    src/hdbscan/constraint.cpp
    src/hdbscan/outlier_score.cpp
    src/hdbscan/undirected_graph.cpp
    src/hdbscan/cluster.cpp
    src/hdbscan/HDBSCAN_star.cpp
    )

add_executable(
    unit_tests # executable
    tests/main.cpp # dependency
    tests/example_add.cpp # dependency
    tests/hdbscan/constraint.cpp
    tests/distances/cosine_similarity.cpp 
    tests/distances/euclidian_distance.cpp 
    tests/distances/manhattan_distance.cpp 
    tests/distances/pearson_correlation.cpp 
    tests/distances/supremum_distance.cpp 
    tests/hdbscan/outlier_score.cpp
    tests/hdbscan/undirected_graph.cpp
    tests/hdbscan/cluster.cpp
    tests/hdbscan/HDBSCAN_star.cpp
)

add_dependencies(unit_tests GTest)

target_link_libraries(
    unit_tests
    GTest
    example
    pthread
    )

enable_testing()

# add_subdirectory(src)
# add_subdirectory(tests)
