cmake_minimum_required(VERSION 3.13)
project(TEAM33_HDBSCAN)

set(CMAKE_CXX_STANDARD 20)

include(cmake/GetGTest.cmake)
include(cmake/GetGFlags.cmake)
include(cmake/GetLikwid.cmake)

include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/include
    ${gflags_BINARY_DIR}/include
    ${likwid_BINARY_DIR}/include
)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


set(
    HDB_SOURCE_FILES
    src/hdbscan/outlier_score.cpp
    src/hdbscan/undirected_graph.cpp
    src/hdbscan/_C_undirected_graph.cpp
    src/hdbscan/cluster.cpp
    src/hdbscan/HDBSCAN_io.cpp
    src/hdbscan/runner.cpp
    src/hdbscan/calculate_core_distances.cpp
    src/hdbscan/construct_mst.cpp
    src/hdbscan/hierarchy_cluster.cpp
    src/hdbscan/propagate_tree.cpp
    src/hdbscan/prominent_clusters.cpp
    src/common/list.cpp
    src/common/vector.cpp
    src/common/set.cpp
    src/common/ordered_set.cpp
    src/common/map.cpp
)

add_library(
    hdb_lib
    ${HDB_SOURCE_FILES}
)
set_target_properties(
    hdb_lib
    PROPERTIES COMPILE_FLAGS "-O3 -march=native"
)

add_executable(
    unit_tests # executable
    tests/main.cpp # dependency
    tests/hdbscan/constraint.cpp
    tests/distances/cosine_similarity.cpp
    tests/distances/euclidean_distance.cpp
    tests/distances/manhattan_distance.cpp
    tests/distances/pearson_correlation.cpp
    tests/distances/supremum_distance.cpp
    tests/hdbscan/undirected_graph.cpp
    tests/hdbscan/_C_undirected_graph.cpp
    tests/hdbscan/cluster.cpp
    tests/hdbscan/HDBSCAN_star.cpp
    tests/hdbscan/runner.cpp
    tests/common/vector_reductions.cpp
    tests/common/list.cpp
    tests/common/vector.cpp
    tests/common/bitset.cpp
    tests/common/set.cpp
    tests/common/quicksort.cpp
    tests/common/ordered_set.cpp
    tests/common/map.cpp
)
target_link_libraries(
    unit_tests
    gtest_main
    hdb_lib
    pthread
    gflags_nothreads_static
    stdc++fs
    likwid
)
set_target_properties(
    unit_tests
    PROPERTIES COMPILE_FLAGS "-O3 -march=native -DLIKWID_PERFMON"
)

function(build_hdbscan exec_name compiler_flags output_directory)
    add_executable(
        ${exec_name}
        src/main.cpp
    )
    add_library(
        ${exec_name}_lib
        ${HDB_SOURCE_FILES}
    )
    target_link_libraries(
        ${exec_name}
        gflags_nothreads_static
        ${exec_name}_lib
        pthread
        likwid
    )
    set_target_properties(
        ${exec_name}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${output_directory}"
    )
    set_target_properties(
        ${exec_name}
        ${exec_name}_lib
        PROPERTIES COMPILE_FLAGS "${compiler_flags} -fno-tree-vectorize -march=native -DLIKWID_PERFMON")
endfunction()

function(build_hdbscan_benchmarking compiler_flags)
    set(NAME_APPEND "")
    string(REPLACE " " "_" NAME_APPEND ${compiler_flags})
    build_hdbscan("hdbscan${NAME_APPEND}" ${compiler_flags} "benchmarking")
endfunction()

build_hdbscan("hdbscan" "-O3" "")

build_hdbscan_benchmarking("-O0")
build_hdbscan_benchmarking("-O3")

enable_testing()

add_executable(
    distance_benchmark
    distance_benchmark/main.cpp
)

set_target_properties(
    distance_benchmark
    PROPERTIES COMPILE_FLAGS "-O3 -fno-tree-vectorize -march=native -DLIKWID_PERFMON"
)

add_library(
    hdb_lib_mst_benchmarking
    ${HDB_SOURCE_FILES}
)
set_target_properties(
    hdb_lib_mst_benchmarking
    PROPERTIES COMPILE_FLAGS "-O3 -fno-tree-vectorize -march=native -DLIKWID_PERFMON"
)

add_executable(
    mst_benchmark
    src/benchmark_mst.cpp
)
target_link_libraries(
    mst_benchmark
    gflags_nothreads_static
    hdb_lib_mst_benchmarking
    stdc++fs
    likwid
)
set_target_properties(
    mst_benchmark
    PROPERTIES COMPILE_FLAGS "-O3 -fno-tree-vectorize -march=native -DLIKWID_PERFMON"
)
add_executable(
    cd_benchmark
    src/benchmark_cd.cpp
)
target_link_libraries(
    cd_benchmark
    gflags_nothreads_static
    hdb_lib_mst_benchmarking
    stdc++fs
    likwid
)
set_target_properties(
    cd_benchmark
    PROPERTIES COMPILE_FLAGS "-O3 -fno-tree-vectorize -march=native -DLIKWID_PERFMON"
)

# add_subdirectory(src)
# add_subdirectory(tests)
