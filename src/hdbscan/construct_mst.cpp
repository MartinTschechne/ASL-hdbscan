#include <hdbscan/HDBSCAN_star.h>
#include <float.h>
#include <stdint.h>
#include <common/bitset.h>
#include <common/memory.h>
#include <common/vector_reductions.h>
#include <cmath>
#include <fstream>
#include <benchmark/tsc_x86.h>
#include <immintrin.h>

ConstructMST_t GetConstructMSTFunction(const std::string& optimization_level) {
    if(optimization_level == "bitset_nocalc") {
        return ConstructMST_Bitset_NoCalc;
    }
    else if(optimization_level == "bitset_unroll") {
        return ConstructMST_Unrolled_Bitset;
    }
    else if(optimization_level == "bitset_unroll_nocalc") {
        return ConstructMST_Unrolled_Bitset_NoCalc;
    }
    #ifdef __AVX2__
    else if(optimization_level == "bitset_nocalc_avx") {
        return ConstructMST_Bitset_NoCalc_AVX;
    }
    else if(optimization_level == "bitset_nocalc_avx_unroll_2") {
        return ConstructMST_Bitset_NoCalc_AVX_Unrolled_2;
    }
    else if(optimization_level == "bitset_nocalc_avx_unroll_4") {
        return ConstructMST_Bitset_NoCalc_AVX_Unrolled_4;
    }
    #endif //__AVX2__
    else if(optimization_level == "nobitset_unroll") {
        return ConstructMST_Unrolled_NoBitset_CalcDistances;
    }
    else if(optimization_level == "nobitset_unroll_nocalc") {
        return ConstructMST_Unrolled_NoBitset_NoCalcDistances;
    }
    #ifdef __AVX2__
    else if(optimization_level == "nobitset_nocalc_avx256") {
        return ConstructMST_NoBitset_NoCalcDistances_AVX256;
    }
    #endif //__AVX2__
    #ifdef __AVX512__
    else if(optimization_level == "nobitset_nocalc_avx512") {
        return ConstructMST_NoBitset_NoCalcDistances_AVX512;
    }
    #endif //__AVX512__
    else if(optimization_level != "no_optimization"){
        std::cout << "\n\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n";
        std::cout << "Optimization " << optimization_level << " not supported for GetConstructMSTFunction. Fallback to unoptimized version\n\n";
        std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n" << std::endl;
    }
    return ConstructMST;

}

UndirectedGraph_C* ConstructMST(const double* data_set,
    const double* core_distances, bool self_edges,
    DistanceCalculator distance_function, size_t n_pts, size_t point_dimension, double** distance_matrix) {

    std::ofstream timing("mst_timings/base.csv");

    unsigned long long start = start_tsc();
    size_t self_edge_capacity = self_edges ? n_pts : 0;
    const double DOUBLE_MAX = DBL_MAX;
    const size_t SIZE_T_MAX = SIZE_MAX;

    //One bit is set (true) for each attached point, or unset (false) for unattached points:
    BitSet_t attached_points = CreateBitset(n_pts, false);

    //Each point has a current neighbor point in the tree, and a current nearest distance:
    size_t* nearest_mrd_neighbors = (size_t*)calloc(n_pts-1 + self_edge_capacity, sizeof(size_t));
    double* nearest_mrd_distances = (double*)calloc(n_pts-1 + self_edge_capacity, sizeof(double));

    for(size_t i = 0; i < n_pts-1; ++i) {
        nearest_mrd_distances[i] = DOUBLE_MAX;
    }

    //The MST is expanded starting with the last point in the data set:
    size_t current_point = n_pts - 1;
    size_t num_attached_points = 1;
    SetBit(attached_points, current_point, true);
    unsigned long long end = stop_tsc(start);

    timing << "Setup," << end << "\n";

    start = start_tsc();
    //Continue attaching points to the MST until all points are attached:
    for(num_attached_points = 1; num_attached_points < n_pts; ++num_attached_points) {
        size_t nearest_mrd_point = SIZE_T_MAX;
        double nearest_mrd_distance = DOUBLE_MAX;

        //Iterate through all unattached points, updating distances using the current point:
        for(size_t neighbor = 0; neighbor < n_pts; ++neighbor) {
            if(current_point == neighbor) {
                continue;
            }
            if(GetBit(attached_points, neighbor)) { // point already attached
                continue;
            }

            double distance = distance_function(&data_set[current_point*point_dimension], &data_set[neighbor*point_dimension], point_dimension);
            double mutual_reachability_distance = distance;

            if(core_distances[current_point] > mutual_reachability_distance) {
                mutual_reachability_distance = core_distances[current_point];
            }
            if(core_distances[neighbor] > mutual_reachability_distance) {
                mutual_reachability_distance = core_distances[neighbor];
            }

            if(mutual_reachability_distance < nearest_mrd_distances[neighbor]) {
                nearest_mrd_distances[neighbor] = mutual_reachability_distance;
                nearest_mrd_neighbors[neighbor] = current_point;
            }

            //Check if the unattached point being updated is the closest to the tree:
            if(nearest_mrd_distances[neighbor] <= nearest_mrd_distance) {
                nearest_mrd_distance = nearest_mrd_distances[neighbor];
                nearest_mrd_point = neighbor;
            }
        }
        //Attach the closest point found in this iteration to the tree:
        SetBit(attached_points, nearest_mrd_point, true);
        current_point = nearest_mrd_point;
    }
    end = stop_tsc(start);
    timing << "MST Compute," << end << "\n";

    //Create an array for vertices in the tree that each point attached to:
    size_t* other_vertex_indices = (size_t*)calloc(n_pts-1 + self_edge_capacity, sizeof(size_t));
    for(size_t i = 0; i < n_pts; ++i) {
        other_vertex_indices[i] = i;
    }

    //If necessary, attach self edges:
    if(self_edges) {
        for(size_t i = n_pts - 1; i < n_pts*2 - 1; ++i) {
            size_t vertex = i - (n_pts - 1);
            nearest_mrd_neighbors[i] = vertex;
            other_vertex_indices[i] = vertex;
            nearest_mrd_distances[i] = core_distances[vertex];
        }
    }

    start = start_tsc();
    UndirectedGraph_C* result = UDG_Create_Fast(n_pts, nearest_mrd_neighbors, other_vertex_indices, nearest_mrd_distances, n_pts-1 + self_edge_capacity);
    end = stop_tsc(start);
    timing << "Construct graph," << end;
    return result;
}

UndirectedGraph_C* ConstructMST_Unrolled_Bitset(const double* data_set,
    const double* core_distances, bool self_edges,
    DistanceCalculator distance_function, size_t n_pts, size_t point_dimension, double** distance_matrix) {

    std::ofstream timing("mst_timings/bitset_unroll.csv");

    unsigned long long start = start_tsc();
    size_t self_edge_capacity = self_edges ? n_pts : 0;
    const double DOUBLE_MAX = DBL_MAX;
    const size_t SIZE_T_MAX = SIZE_MAX;

    //One bit is set (true) for each attached point, or unset (false) for unattached points:
    BitSet_t attached_points = CreateBitset(n_pts, false);

    //Each point has a current neighbor point in the tree, and a current nearest distance:
    size_t* nearest_mrd_neighbors = (size_t*)calloc(n_pts-1 + self_edge_capacity, sizeof(size_t));
    double* nearest_mrd_distances = (double*)calloc(n_pts-1 + self_edge_capacity, sizeof(double));

    for(size_t i = 0; i < n_pts-1; ++i) {
        nearest_mrd_distances[i] = DOUBLE_MAX;
    }

    //The MST is expanded starting with the last point in the data set:
    size_t current_point = n_pts - 1;
    size_t num_attached_points = 1;
    SetBit(attached_points, current_point, true);
    unsigned long long end = stop_tsc(start);

    timing << "Setup," << end << "\n";

    start = start_tsc();
    //Continue attaching points to the MST until all points are attached:
    for(num_attached_points = 1; num_attached_points < n_pts; ++num_attached_points) {
        size_t nearest_mrd_point0 = 0;
        double nearest_mrd_distance0 = DOUBLE_MAX;
        size_t nearest_mrd_point1 = 0;
        double nearest_mrd_distance1 = DOUBLE_MAX;
        size_t nearest_mrd_point2 = 0;
        double nearest_mrd_distance2 = DOUBLE_MAX;
        size_t nearest_mrd_point3 = 0;
        double nearest_mrd_distance3 = DOUBLE_MAX;

        //Iterate through all unattached points, updating distances using the current point:
        size_t neighbor = 0;
        for(; neighbor < n_pts - 3; neighbor += 4) {
            bool flag0 = current_point == (neighbor) || GetBit(attached_points, neighbor);
            bool flag1 = current_point == (neighbor + 1) || GetBit(attached_points, neighbor + 1);
            bool flag2 = current_point == (neighbor + 2) || GetBit(attached_points, neighbor + 2);
            bool flag3 = current_point == (neighbor + 3) || GetBit(attached_points, neighbor + 3);

            if(flag0 && flag1 && flag2 && flag3) {
                continue;
            }

            const double* current_point_ptr = &data_set[current_point*point_dimension];
            double core_distance = core_distances[current_point];
            double cdn0 = core_distances[neighbor];
            double cdn1 = core_distances[neighbor + 1];
            double cdn2 = core_distances[neighbor + 2];
            double cdn3 = core_distances[neighbor + 3];
            double nmrd0 = nearest_mrd_distances[neighbor];
            double nmrd1 = nearest_mrd_distances[neighbor + 1];
            double nmrd2 = nearest_mrd_distances[neighbor + 2];
            double nmrd3 = nearest_mrd_distances[neighbor + 3];
            double nmrdn0 = nearest_mrd_neighbors[neighbor];
            double nmrdn1 = nearest_mrd_neighbors[neighbor + 1];
            double nmrdn2 = nearest_mrd_neighbors[neighbor + 2];
            double nmrdn3 = nearest_mrd_neighbors[neighbor + 3];


            double mrd0 = flag0 ? 0.0 : distance_function(current_point_ptr, &data_set[neighbor * point_dimension], point_dimension);
            double mrd1 = flag1 ? 0.0 : distance_function(current_point_ptr, &data_set[(neighbor + 1)*point_dimension], point_dimension);
            double mrd2 = flag2 ? 0.0 : distance_function(current_point_ptr, &data_set[(neighbor + 2)*point_dimension], point_dimension);
            double mrd3 = flag3 ? 0.0 : distance_function(current_point_ptr, &data_set[(neighbor + 3)*point_dimension], point_dimension);

            mrd0 = fmax(mrd0, core_distance);
            mrd0 = fmax(mrd0, cdn0);

            mrd1 = fmax(mrd1, core_distance);
            mrd1 = fmax(mrd1, cdn1);
            mrd2 = fmax(mrd2, core_distance);
            mrd2 = fmax(mrd2, cdn2);
            mrd3 = fmax(mrd3, core_distance);
            mrd3 = fmax(mrd3, cdn3);


            if(!flag0 && mrd0 < nmrd0) {
                nmrd0 = mrd0;
                nmrdn0 = current_point;
                nearest_mrd_distances[neighbor] = mrd0;
                nearest_mrd_neighbors[neighbor] = current_point;
            }
            if(!flag1 && mrd1 < nmrd1) {
                nmrd1 = mrd1;
                nmrdn1 = current_point;
                nearest_mrd_distances[neighbor + 1] = mrd1;
                nearest_mrd_neighbors[neighbor + 1] = current_point;
            }
            if(!flag2 && mrd2 < nmrd2) {
                nmrd2 = mrd2;
                nmrdn2 = current_point;
                nearest_mrd_distances[neighbor + 2] = mrd2;
                nearest_mrd_neighbors[neighbor + 2] = current_point;
            }
            if(!flag3 && mrd3 < nmrd3) {
                nmrd3 = mrd3;
                nmrdn3 = current_point;
                nearest_mrd_distances[neighbor + 3] = mrd3;
                nearest_mrd_neighbors[neighbor + 3] = current_point;
            }

            nmrd0 = flag0 ? DBL_MAX : nmrd0;
            nmrd1 = flag1 ? DBL_MAX : nmrd1;
            nmrd2 = flag2 ? DBL_MAX : nmrd2;
            nmrd3 = flag3 ? DBL_MAX : nmrd3;

            if(nmrd0 <= nearest_mrd_distance0) {
                nearest_mrd_distance0 = nmrd0;
                nearest_mrd_point0 = neighbor;
            }
            if(nmrd1 <= nearest_mrd_distance1) {
                nearest_mrd_distance1 = nmrd1;
                nearest_mrd_point1 = neighbor + 1;
            }
            if(nmrd2 <= nearest_mrd_distance2) {
                nearest_mrd_distance2 = nmrd2;
                nearest_mrd_point2 = neighbor + 2;
            }
            if(nmrd3 <= nearest_mrd_distance3) {
                nearest_mrd_distance3 = nmrd3;
                nearest_mrd_point3 = neighbor + 3;
            }
        }
        for(; neighbor < n_pts; ++neighbor) {
            if(current_point == neighbor) {
                continue;
            }
            if(GetBit(attached_points, neighbor)) { // point already attached
                continue;
            }

            double distance = distance_function(&data_set[current_point*point_dimension], &data_set[neighbor*point_dimension], point_dimension);
            double mutual_reachability_distance = distance;

            if(core_distances[current_point] > mutual_reachability_distance) {
                mutual_reachability_distance = core_distances[current_point];
            }
            if(core_distances[neighbor] > mutual_reachability_distance) {
                mutual_reachability_distance = core_distances[neighbor];
            }

            if(mutual_reachability_distance < nearest_mrd_distances[neighbor]) {
                nearest_mrd_distances[neighbor] = mutual_reachability_distance;
                nearest_mrd_neighbors[neighbor] = current_point;
            }

            //Check if the unattached point being updated is the closest to the tree:
            if(nearest_mrd_distances[neighbor] <= nearest_mrd_distance3) {
                nearest_mrd_distance3 = nearest_mrd_distances[neighbor];
                nearest_mrd_point3 = neighbor;
            }
        }

        double nearest_mrd_distance = fmin(fmin(fmin(nearest_mrd_distance0, nearest_mrd_distance1), nearest_mrd_distance2), nearest_mrd_distance3);
        current_point = 0;
        if(nearest_mrd_distance0 == nearest_mrd_distance) {
            current_point = nearest_mrd_point0 > current_point ? nearest_mrd_point0 : current_point;
        }
        if(nearest_mrd_distance1 == nearest_mrd_distance) {
            current_point = nearest_mrd_point1 > current_point ? nearest_mrd_point1 : current_point;
        }
        if(nearest_mrd_distance2 == nearest_mrd_distance) {
            current_point = nearest_mrd_point2 > current_point ? nearest_mrd_point2 : current_point;
        }
        if(nearest_mrd_distance3 == nearest_mrd_distance) {
            current_point = nearest_mrd_point3 > current_point ? nearest_mrd_point3 : current_point;
        }

        //Attach the closest point found in this iteration to the tree:
        SetBit(attached_points, current_point, true);
    }
    end = stop_tsc(start);
    timing << "MST Compute," << end << "\n";

    //Create an array for vertices in the tree that each point attached to:
    size_t* other_vertex_indices = (size_t*)calloc(n_pts-1 + self_edge_capacity, sizeof(size_t));
    for(size_t i = 0; i < n_pts; ++i) {
        other_vertex_indices[i] = i;
    }

    //If necessary, attach self edges:
    if(self_edges) {
        for(size_t i = n_pts - 1; i < n_pts*2 - 1; ++i) {
            size_t vertex = i - (n_pts - 1);
            nearest_mrd_neighbors[i] = vertex;
            other_vertex_indices[i] = vertex;
            nearest_mrd_distances[i] = core_distances[vertex];
        }
    }

    start = start_tsc();
    UndirectedGraph_C* result = UDG_Create_Fast(n_pts, nearest_mrd_neighbors, other_vertex_indices, nearest_mrd_distances, n_pts-1 + self_edge_capacity);
    end = stop_tsc(start);
    timing << "Construct graph," << end;
    return result;
}

UndirectedGraph_C* ConstructMST_Bitset_NoCalc(const double* data_set,
    const double* core_distances, bool self_edges,
    DistanceCalculator distance_function, size_t n_pts, size_t point_dimension, double** distance_matrix) {

    std::ofstream timing("mst_timings/bitset_nocalc.csv");

    unsigned long long start = start_tsc();
    size_t self_edge_capacity = self_edges ? n_pts : 0;
    const double DOUBLE_MAX = DBL_MAX;
    const size_t SIZE_T_MAX = SIZE_MAX;

    //One bit is set (true) for each attached point, or unset (false) for unattached points:
    BitSet_t attached_points = CreateBitset(n_pts, false);

    //Each point has a current neighbor point in the tree, and a current nearest distance:
    size_t* nearest_mrd_neighbors = (size_t*)calloc(n_pts-1 + self_edge_capacity, sizeof(size_t));
    double* nearest_mrd_distances = (double*)calloc(n_pts-1 + self_edge_capacity, sizeof(double));

    for(size_t i = 0; i < n_pts-1; ++i) {
        nearest_mrd_distances[i] = DOUBLE_MAX;
    }

    //The MST is expanded starting with the last point in the data set:
    size_t current_point = n_pts - 1;
    size_t num_attached_points = 1;
    SetBit(attached_points, current_point, true);
    unsigned long long end = stop_tsc(start);

    timing << "Setup," << end << "\n";

    start = start_tsc();
    //Continue attaching points to the MST until all points are attached:
    for(num_attached_points = 1; num_attached_points < n_pts; ++num_attached_points) {
        size_t nearest_mrd_point = SIZE_T_MAX;
        double nearest_mrd_distance = DOUBLE_MAX;

        //Iterate through all unattached points, updating distances using the current point:
        for(size_t neighbor = 0; neighbor < n_pts; ++neighbor) {
            if(current_point == neighbor) {
                continue;
            }
            if(GetBit(attached_points, neighbor)) { // point already attached
                continue;
            }

            double distance = distance_matrix[current_point][neighbor];
            double mutual_reachability_distance = distance;

            if(core_distances[current_point] > mutual_reachability_distance) {
                mutual_reachability_distance = core_distances[current_point];
            }
            if(core_distances[neighbor] > mutual_reachability_distance) {
                mutual_reachability_distance = core_distances[neighbor];
            }

            if(mutual_reachability_distance < nearest_mrd_distances[neighbor]) {
                nearest_mrd_distances[neighbor] = mutual_reachability_distance;
                nearest_mrd_neighbors[neighbor] = current_point;
            }

            //Check if the unattached point being updated is the closest to the tree:
            if(nearest_mrd_distances[neighbor] <= nearest_mrd_distance) {
                nearest_mrd_distance = nearest_mrd_distances[neighbor];
                nearest_mrd_point = neighbor;
            }
        }
        //Attach the closest point found in this iteration to the tree:
        SetBit(attached_points, nearest_mrd_point, true);
        current_point = nearest_mrd_point;
    }
    end = stop_tsc(start);
    timing << "MST Compute," << end << "\n";

    //Create an array for vertices in the tree that each point attached to:
    size_t* other_vertex_indices = (size_t*)calloc(n_pts-1 + self_edge_capacity, sizeof(size_t));
    for(size_t i = 0; i < n_pts; ++i) {
        other_vertex_indices[i] = i;
    }

    //If necessary, attach self edges:
    if(self_edges) {
        for(size_t i = n_pts - 1; i < n_pts*2 - 1; ++i) {
            size_t vertex = i - (n_pts - 1);
            nearest_mrd_neighbors[i] = vertex;
            other_vertex_indices[i] = vertex;
            nearest_mrd_distances[i] = core_distances[vertex];
        }
    }

    start = start_tsc();
    UndirectedGraph_C* result = UDG_Create_Fast(n_pts, nearest_mrd_neighbors, other_vertex_indices, nearest_mrd_distances, n_pts-1 + self_edge_capacity);
    end = stop_tsc(start);
    timing << "Construct graph," << end;
    return result;
}

UndirectedGraph_C* ConstructMST_Unrolled_Bitset_NoCalc(const double* data_set,
    const double* core_distances, bool self_edges,
    DistanceCalculator distance_function, size_t n_pts, size_t point_dimension, double** distance_matrix) {

    std::ofstream timing("mst_timings/bitset_nocalc_unroll4.csv");

    unsigned long long start = start_tsc();
    size_t self_edge_capacity = self_edges ? n_pts : 0;
    const double DOUBLE_MAX = DBL_MAX;
    const size_t SIZE_T_MAX = SIZE_MAX;

    //One bit is set (true) for each attached point, or unset (false) for unattached points:
    BitSet_t attached_points = CreateBitset(n_pts, false);

    //Each point has a current neighbor point in the tree, and a current nearest distance:
    size_t* nearest_mrd_neighbors = (size_t*)calloc(n_pts-1 + self_edge_capacity, sizeof(size_t));
    double* nearest_mrd_distances = (double*)calloc(n_pts-1 + self_edge_capacity, sizeof(double));

    for(size_t i = 0; i < n_pts-1; ++i) {
        nearest_mrd_distances[i] = DOUBLE_MAX;
    }

    //The MST is expanded starting with the last point in the data set:
    size_t current_point = n_pts - 1;
    size_t num_attached_points = 1;
    SetBit(attached_points, current_point, true);
    unsigned long long end = stop_tsc(start);

    timing << "Setup," << end << "\n";

    start = start_tsc();
    //Continue attaching points to the MST until all points are attached:
    for(num_attached_points = 1; num_attached_points < n_pts; ++num_attached_points) {
        size_t nearest_mrd_point0 = 0;
        double nearest_mrd_distance0 = DOUBLE_MAX;
        size_t nearest_mrd_point1 = 0;
        double nearest_mrd_distance1 = DOUBLE_MAX;
        size_t nearest_mrd_point2 = 0;
        double nearest_mrd_distance2 = DOUBLE_MAX;
        size_t nearest_mrd_point3 = 0;
        double nearest_mrd_distance3 = DOUBLE_MAX;

        //Iterate through all unattached points, updating distances using the current point:
        size_t neighbor = 0;
        for(; neighbor < n_pts - 3; neighbor += 4) {
            bool flag0 = current_point == (neighbor) || GetBit(attached_points, neighbor);
            bool flag1 = current_point == (neighbor + 1) || GetBit(attached_points, neighbor + 1);
            bool flag2 = current_point == (neighbor + 2) || GetBit(attached_points, neighbor + 2);
            bool flag3 = current_point == (neighbor + 3) || GetBit(attached_points, neighbor + 3);

            if(flag0 && flag1 && flag2 && flag3) {
                continue;
            }

            const double* current_point_ptr = &data_set[current_point*point_dimension];
            double core_distance = core_distances[current_point];
            double cdn0 = core_distances[neighbor];
            double cdn1 = core_distances[neighbor + 1];
            double cdn2 = core_distances[neighbor + 2];
            double cdn3 = core_distances[neighbor + 3];
            double nmrd0 = nearest_mrd_distances[neighbor];
            double nmrd1 = nearest_mrd_distances[neighbor + 1];
            double nmrd2 = nearest_mrd_distances[neighbor + 2];
            double nmrd3 = nearest_mrd_distances[neighbor + 3];
            double nmrdn0 = nearest_mrd_neighbors[neighbor];
            double nmrdn1 = nearest_mrd_neighbors[neighbor + 1];
            double nmrdn2 = nearest_mrd_neighbors[neighbor + 2];
            double nmrdn3 = nearest_mrd_neighbors[neighbor + 3];


            double mrd0 = distance_matrix[current_point][neighbor];
            double mrd1 = distance_matrix[current_point][neighbor + 1];
            double mrd2 = distance_matrix[current_point][neighbor + 2];
            double mrd3 = distance_matrix[current_point][neighbor + 3];

            mrd0 = fmax(mrd0, core_distance);
            mrd0 = fmax(mrd0, cdn0);

            mrd1 = fmax(mrd1, core_distance);
            mrd1 = fmax(mrd1, cdn1);
            mrd2 = fmax(mrd2, core_distance);
            mrd2 = fmax(mrd2, cdn2);
            mrd3 = fmax(mrd3, core_distance);
            mrd3 = fmax(mrd3, cdn3);


            if(!flag0 && mrd0 < nmrd0) {
                nmrd0 = mrd0;
                nmrdn0 = current_point;
                nearest_mrd_distances[neighbor] = mrd0;
                nearest_mrd_neighbors[neighbor] = current_point;
            }
            if(!flag1 && mrd1 < nmrd1) {
                nmrd1 = mrd1;
                nmrdn1 = current_point;
                nearest_mrd_distances[neighbor + 1] = mrd1;
                nearest_mrd_neighbors[neighbor + 1] = current_point;
            }
            if(!flag2 && mrd2 < nmrd2) {
                nmrd2 = mrd2;
                nmrdn2 = current_point;
                nearest_mrd_distances[neighbor + 2] = mrd2;
                nearest_mrd_neighbors[neighbor + 2] = current_point;
            }
            if(!flag3 && mrd3 < nmrd3) {
                nmrd3 = mrd3;
                nmrdn3 = current_point;
                nearest_mrd_distances[neighbor + 3] = mrd3;
                nearest_mrd_neighbors[neighbor + 3] = current_point;
            }

            nmrd0 = flag0 ? DBL_MAX : nmrd0;
            nmrd1 = flag1 ? DBL_MAX : nmrd1;
            nmrd2 = flag2 ? DBL_MAX : nmrd2;
            nmrd3 = flag3 ? DBL_MAX : nmrd3;

            if(nmrd0 <= nearest_mrd_distance0) {
                nearest_mrd_distance0 = nmrd0;
                nearest_mrd_point0 = neighbor;
            }
            if(nmrd1 <= nearest_mrd_distance1) {
                nearest_mrd_distance1 = nmrd1;
                nearest_mrd_point1 = neighbor + 1;
            }
            if(nmrd2 <= nearest_mrd_distance2) {
                nearest_mrd_distance2 = nmrd2;
                nearest_mrd_point2 = neighbor + 2;
            }
            if(nmrd3 <= nearest_mrd_distance3) {
                nearest_mrd_distance3 = nmrd3;
                nearest_mrd_point3 = neighbor + 3;
            }
        }
        for(; neighbor < n_pts; ++neighbor) {
            if(current_point == neighbor) {
                continue;
            }
            if(GetBit(attached_points, neighbor)) { // point already attached
                continue;
            }

            double distance = distance_matrix[current_point][neighbor];
            double mutual_reachability_distance = distance;

            if(core_distances[current_point] > mutual_reachability_distance) {
                mutual_reachability_distance = core_distances[current_point];
            }
            if(core_distances[neighbor] > mutual_reachability_distance) {
                mutual_reachability_distance = core_distances[neighbor];
            }

            if(mutual_reachability_distance < nearest_mrd_distances[neighbor]) {
                nearest_mrd_distances[neighbor] = mutual_reachability_distance;
                nearest_mrd_neighbors[neighbor] = current_point;
            }

            //Check if the unattached point being updated is the closest to the tree:
            if(nearest_mrd_distances[neighbor] <= nearest_mrd_distance3) {
                nearest_mrd_distance3 = nearest_mrd_distances[neighbor];
                nearest_mrd_point3 = neighbor;
            }
        }

        double nearest_mrd_distance = fmin(fmin(fmin(nearest_mrd_distance0, nearest_mrd_distance1), nearest_mrd_distance2), nearest_mrd_distance3);
        current_point = 0;
        if(nearest_mrd_distance0 == nearest_mrd_distance) {
            current_point = nearest_mrd_point0 > current_point ? nearest_mrd_point0 : current_point;
        }
        if(nearest_mrd_distance1 == nearest_mrd_distance) {
            current_point = nearest_mrd_point1 > current_point ? nearest_mrd_point1 : current_point;
        }
        if(nearest_mrd_distance2 == nearest_mrd_distance) {
            current_point = nearest_mrd_point2 > current_point ? nearest_mrd_point2 : current_point;
        }
        if(nearest_mrd_distance3 == nearest_mrd_distance) {
            current_point = nearest_mrd_point3 > current_point ? nearest_mrd_point3 : current_point;
        }

        //Attach the closest point found in this iteration to the tree:
        SetBit(attached_points, current_point, true);
    }
    end = stop_tsc(start);
    timing << "MST Compute," << end << "\n";

    //Create an array for vertices in the tree that each point attached to:
    size_t* other_vertex_indices = (size_t*)calloc(n_pts-1 + self_edge_capacity, sizeof(size_t));
    for(size_t i = 0; i < n_pts; ++i) {
        other_vertex_indices[i] = i;
    }

    //If necessary, attach self edges:
    if(self_edges) {
        for(size_t i = n_pts - 1; i < n_pts*2 - 1; ++i) {
            size_t vertex = i - (n_pts - 1);
            nearest_mrd_neighbors[i] = vertex;
            other_vertex_indices[i] = vertex;
            nearest_mrd_distances[i] = core_distances[vertex];
        }
    }

    start = start_tsc();
    UndirectedGraph_C* result = UDG_Create_Fast(n_pts, nearest_mrd_neighbors, other_vertex_indices, nearest_mrd_distances, n_pts-1 + self_edge_capacity);
    end = stop_tsc(start);
    timing << "Construct graph," << end;
    return result;
}

#ifdef __AVX2__
UndirectedGraph_C* ConstructMST_Bitset_NoCalc_AVX(const double* data_set,
    const double* core_distances, bool self_edges,
    DistanceCalculator distance_function, size_t n_pts, size_t point_dimension, double** distance_matrix) {

    std::ofstream timing("mst_timings/bitset_nocalc_avx.csv");
    size_t all_int = 0xffffffffffffffff;

    unsigned long long start = start_tsc();
    size_t self_edge_capacity = self_edges ? n_pts : 0;
    const double DOUBLE_MAX = DBL_MAX;
    const size_t SIZE_T_MAX = SIZE_MAX;

    //One bit is set (true) for each attached point, or unset (false) for unattached points:
    size_t* attached_points = CreateAlignedSizeT1D(n_pts);
    size_t i = 0;
    for(; i < n_pts - 3; i += 4) {
        _mm256_store_si256((__m256i*)(attached_points+i), _mm256_setzero_si256());
    }
    for(; i < n_pts; ++i) {
        attached_points[i] = 0;
    }

    //Each point has a current neighbor point in the tree, and a current nearest distance:
    size_t* nearest_mrd_neighbors = CreateAlignedSizeT1D(n_pts-1 + self_edge_capacity);
    double* nearest_mrd_distances = CreateAlignedDouble1D(n_pts-1 + self_edge_capacity);

    for(size_t i = 0; i < n_pts-1; ++i) {
        nearest_mrd_distances[i] = DOUBLE_MAX;
    }

    //The MST is expanded starting with the last point in the data set:
    size_t current_point = n_pts - 1;
    size_t num_attached_points = 1;
    attached_points[current_point] = all_int;
    unsigned long long end = stop_tsc(start);

    timing << "Setup," << end << "\n";

    start = start_tsc();
    __m256d dbl_max = _mm256_set1_pd(DBL_MAX);
    __m256i all_set_mask = _mm256_set1_epi64x(all_int);
    //Continue attaching points to the MST until all points are attached:
    for(num_attached_points = 1; num_attached_points < n_pts; ++num_attached_points) {
        __m256i nearest_mrd_point = _mm256_setzero_si256();
        __m256d nearest_mrd_distance = _mm256_set1_pd(DOUBLE_MAX);
        __m256i current_point_intr = _mm256_set1_epi64x(current_point);
        __m256d core_distance_current = _mm256_set1_pd(core_distances[current_point]);

        //Iterate through all unattached points, updating distances using the current point:
        size_t neighbor = 0;
        for(; neighbor < n_pts - 4; neighbor += 4) {
            __m256i attached_mask = _mm256_load_si256((__m256i*)(attached_points + neighbor));
            if(vec_equal(attached_mask, all_set_mask)) {
                continue;
            }
            __m256i possible_indices = _mm256_set_epi64x(neighbor + 3, neighbor + 2, neighbor + 1, neighbor);
            __m256i current_point_mask = _mm256_cmpeq_epi64(current_point_intr, possible_indices);
            __m256d nearest_mrd = _mm256_load_pd(nearest_mrd_distances + neighbor);
            __m256i nearest_neighbors = _mm256_load_si256(((__m256i *)(nearest_mrd_neighbors + neighbor)));
            __m256d core_distances_neighbor = _mm256_load_pd(core_distances + neighbor);

            __m256d mrd = _mm256_load_pd(distance_matrix[current_point] + neighbor);
            mrd = _mm256_max_pd(core_distance_current, mrd);
            mrd = _mm256_max_pd(core_distances_neighbor, mrd);

            mrd = _mm256_blendv_pd(mrd, dbl_max, (__m256d)_mm256_or_si256(attached_mask, current_point_mask));

            __m256d lt_mask = _mm256_cmp_pd(mrd, nearest_mrd, _CMP_LT_OQ);
            nearest_mrd = _mm256_blendv_pd(nearest_mrd, mrd, lt_mask);
            nearest_neighbors = _mm256_blendv_epi8(nearest_neighbors, current_point_intr, (__m256i)lt_mask);

            _mm256_store_pd(nearest_mrd_distances + neighbor, nearest_mrd);
            _mm256_store_si256((__m256i *)(nearest_mrd_neighbors + neighbor), nearest_neighbors);

            __m256d max_mask = _mm256_cmp_pd(mrd, dbl_max, _CMP_NEQ_OQ);
            __m256d le_mask = _mm256_cmp_pd(nearest_mrd, nearest_mrd_distance, _CMP_LE_OQ);
            le_mask = _mm256_and_pd(max_mask, le_mask);

            nearest_mrd_distance = _mm256_blendv_pd(nearest_mrd_distance, nearest_mrd, le_mask);
            nearest_mrd_point = _mm256_blendv_epi8(nearest_mrd_point, possible_indices, (__m256i)le_mask);
        }
        double nearest_mrd_distance_arr[4];
        size_t nearest_mrd_point_arr[4];

        _mm256_store_pd(nearest_mrd_distance_arr, nearest_mrd_distance);
        _mm256_store_si256((__m256i*)nearest_mrd_point_arr, nearest_mrd_point);

        for(; neighbor < n_pts - 1; ++neighbor) {
            if(current_point == neighbor) {
                continue;
            }
            if(attached_points[neighbor] == all_int) { // point already attached
                continue;
            }

            double distance = distance_matrix[current_point][neighbor];
            double mutual_reachability_distance = distance;

            if(core_distances[current_point] > mutual_reachability_distance) {
                mutual_reachability_distance = core_distances[current_point];
            }
            if(core_distances[neighbor] > mutual_reachability_distance) {
                mutual_reachability_distance = core_distances[neighbor];
            }

            if(mutual_reachability_distance < nearest_mrd_distances[neighbor]) {
                nearest_mrd_distances[neighbor] = mutual_reachability_distance;
                nearest_mrd_neighbors[neighbor] = current_point;
            }

            //Check if the unattached point being updated is the closest to the tree:
            if(nearest_mrd_distances[neighbor] <= nearest_mrd_distance_arr[3]) {
                nearest_mrd_distance_arr[3] = nearest_mrd_distances[neighbor];
                nearest_mrd_point_arr[3] = neighbor;
            }
        }

        double nearest_mrd_distance_final = fmin(fmin(fmin(nearest_mrd_distance_arr[0], nearest_mrd_distance_arr[1]), nearest_mrd_distance_arr[2]), nearest_mrd_distance_arr[3]);
        current_point = 0;
        if(nearest_mrd_distance_arr[0] == nearest_mrd_distance_final) {
            current_point = nearest_mrd_point_arr[0] > current_point ? nearest_mrd_point_arr[0] : current_point;
        }
        if(nearest_mrd_distance_arr[1] == nearest_mrd_distance_final) {
            current_point = nearest_mrd_point_arr[1] > current_point ? nearest_mrd_point_arr[1] : current_point;
        }
        if(nearest_mrd_distance_arr[2] == nearest_mrd_distance_final) {
            current_point = nearest_mrd_point_arr[2] > current_point ? nearest_mrd_point_arr[2] : current_point;
        }
        if(nearest_mrd_distance_arr[3] == nearest_mrd_distance_final) {
            current_point = nearest_mrd_point_arr[3] > current_point ? nearest_mrd_point_arr[3] : current_point;
        }
        //Attach the closest point found in this iteration to the tree:
        attached_points[current_point] = all_int;
    }
    end = stop_tsc(start);
    timing << "MST Compute," << end << "\n";
    free(attached_points);

    //Create an array for vertices in the tree that each point attached to:
    size_t* other_vertex_indices = (size_t*)calloc(n_pts-1 + self_edge_capacity, sizeof(size_t));
    for(size_t i = 0; i < n_pts; ++i) {
        other_vertex_indices[i] = i;
    }

    //If necessary, attach self edges:
    if(self_edges) {
        for(size_t i = n_pts - 1; i < n_pts*2 - 1; ++i) {
            size_t vertex = i - (n_pts - 1);
            nearest_mrd_neighbors[i] = vertex;
            other_vertex_indices[i] = vertex;
            nearest_mrd_distances[i] = core_distances[vertex];
        }
    }

    start = start_tsc();
    UndirectedGraph_C* result = UDG_Create_Fast(n_pts, nearest_mrd_neighbors, other_vertex_indices, nearest_mrd_distances, n_pts-1 + self_edge_capacity);
    end = stop_tsc(start);
    timing << "Construct graph," << end;
    return result;
}


UndirectedGraph_C* ConstructMST_Bitset_NoCalc_AVX_Unrolled_2(const double* data_set,
    const double* core_distances, bool self_edges,
    DistanceCalculator distance_function, size_t n_pts, size_t point_dimension, double** distance_matrix) {

    std::ofstream timing("mst_timings/bitset_nocalc_avx_unroll2.csv");

    unsigned long long start = start_tsc();
    size_t self_edge_capacity = self_edges ? n_pts : 0;
    const double DOUBLE_MAX = DBL_MAX;
    const size_t SIZE_T_MAX = SIZE_MAX;
    size_t all_int = 0xffffffffffffffff;

    //One bit is set (true) for each attached point, or unset (false) for unattached points:
    size_t* attached_points = CreateAlignedSizeT1D(n_pts);
    size_t i = 0;
    for(; i < n_pts - 3; i += 4) {
        _mm256_store_si256((__m256i*)(attached_points+i), _mm256_setzero_si256());
    }
    for(; i < n_pts; ++i) {
        attached_points[i] = 0;
    }

    //Each point has a current neighbor point in the tree, and a current nearest distance:
    size_t* nearest_mrd_neighbors = CreateAlignedSizeT1D(n_pts-1 + self_edge_capacity);
    double* nearest_mrd_distances = CreateAlignedDouble1D(n_pts-1 + self_edge_capacity);

    for(size_t i = 0; i < n_pts-1; ++i) {
        nearest_mrd_distances[i] = DOUBLE_MAX;
    }

    //The MST is expanded starting with the last point in the data set:
    size_t current_point = n_pts - 1;
    size_t num_attached_points = 1;
    attached_points[current_point] = all_int;
    unsigned long long end = stop_tsc(start);

    timing << "Setup," << end << "\n";

    start = start_tsc();
    __m256d dbl_max = _mm256_set1_pd(DBL_MAX);
    __m256i all_set_mask = _mm256_set1_epi64x(all_int);
    //Continue attaching points to the MST until all points are attached:
    for(num_attached_points = 1; num_attached_points < n_pts; ++num_attached_points) {
        __m256i nearest_mrd_point0 = _mm256_setzero_si256();
        __m256i nearest_mrd_point1 = _mm256_setzero_si256();

        __m256d nearest_mrd_distance0 = _mm256_set1_pd(DOUBLE_MAX);
        __m256d nearest_mrd_distance1 = _mm256_set1_pd(DOUBLE_MAX);

        __m256i current_point_intr = _mm256_set1_epi64x(current_point);
        __m256d core_distance_current = _mm256_set1_pd(core_distances[current_point]);

        //Iterate through all unattached points, updating distances using the current point:
        size_t neighbor = 0;
        for(; neighbor < n_pts - 8; neighbor += 8) {
            __m256i attached_mask0 = _mm256_load_si256((__m256i*)(attached_points + neighbor));
            __m256i attached_mask1 = _mm256_load_si256((__m256i*)(attached_points + neighbor + 4));
            if(vec_equal(attached_mask0, all_set_mask) && vec_equal(attached_mask1, all_set_mask)) {
                continue;
            }
            __m256i possible_indices0 = _mm256_set_epi64x(neighbor + 3, neighbor + 2, neighbor + 1, neighbor);
            __m256i possible_indices1 = _mm256_set_epi64x(neighbor + 7, neighbor + 6, neighbor + 5, neighbor + 4);

            __m256i current_point_mask0 = _mm256_cmpeq_epi64(current_point_intr, possible_indices0);
            __m256i current_point_mask1 = _mm256_cmpeq_epi64(current_point_intr, possible_indices1);
            __m256d nearest_mrd0 = _mm256_load_pd(nearest_mrd_distances + neighbor);
            __m256d nearest_mrd1 = _mm256_load_pd(nearest_mrd_distances + neighbor + 4);

            __m256i nearest_neighbors0 = _mm256_load_si256(((__m256i *)(nearest_mrd_neighbors + neighbor)));
            __m256i nearest_neighbors1 = _mm256_load_si256(((__m256i *)(nearest_mrd_neighbors + neighbor + 4)));
            __m256d core_distances_neighbor0 = _mm256_load_pd(core_distances + neighbor);
            __m256d core_distances_neighbor1 = _mm256_load_pd(core_distances + neighbor + 4);

            __m256d mrd0 = _mm256_load_pd(distance_matrix[current_point] + neighbor);
            __m256d mrd1 = _mm256_load_pd(distance_matrix[current_point] + neighbor + 4);
            mrd0 = _mm256_max_pd(core_distance_current, mrd0);
            mrd1 = _mm256_max_pd(core_distance_current, mrd1);

            mrd0 = _mm256_max_pd(core_distances_neighbor0, mrd0);
            mrd1 = _mm256_max_pd(core_distances_neighbor1, mrd1);

            mrd0 = _mm256_blendv_pd(mrd0, dbl_max, (__m256d)_mm256_or_si256(attached_mask0, current_point_mask0));
            mrd1 = _mm256_blendv_pd(mrd1, dbl_max, (__m256d)_mm256_or_si256(attached_mask1, current_point_mask1));

            __m256d lt_mask0 = _mm256_cmp_pd(mrd0, nearest_mrd0, _CMP_LT_OQ);
            __m256d lt_mask1 = _mm256_cmp_pd(mrd1, nearest_mrd1, _CMP_LT_OQ);

            nearest_mrd0 = _mm256_blendv_pd(nearest_mrd0, mrd0, lt_mask0);
            nearest_mrd1 = _mm256_blendv_pd(nearest_mrd1, mrd1, lt_mask1);

            nearest_neighbors0 = _mm256_blendv_epi8(nearest_neighbors0, current_point_intr, (__m256i)lt_mask0);
            nearest_neighbors1 = _mm256_blendv_epi8(nearest_neighbors1, current_point_intr, (__m256i)lt_mask1);

            _mm256_store_pd(nearest_mrd_distances + neighbor, nearest_mrd0);
            _mm256_store_pd(nearest_mrd_distances + neighbor + 4, nearest_mrd1);
            _mm256_store_si256((__m256i *)(nearest_mrd_neighbors + neighbor), nearest_neighbors0);
            _mm256_store_si256((__m256i *)(nearest_mrd_neighbors + neighbor + 4), nearest_neighbors1);

            __m256d max_mask0 = _mm256_cmp_pd(mrd0, dbl_max, _CMP_NEQ_OQ);
            __m256d max_mask1 = _mm256_cmp_pd(mrd1, dbl_max, _CMP_NEQ_OQ);

            __m256d le_mask0 = _mm256_cmp_pd(nearest_mrd0, nearest_mrd_distance0, _CMP_LE_OQ);
            __m256d le_mask1 = _mm256_cmp_pd(nearest_mrd1, nearest_mrd_distance1, _CMP_LE_OQ);

            le_mask0 = _mm256_and_pd(max_mask0, le_mask0);
            le_mask1 = _mm256_and_pd(max_mask1, le_mask1);

            nearest_mrd_distance0 = _mm256_blendv_pd(nearest_mrd_distance0, nearest_mrd0, le_mask0);
            nearest_mrd_distance1 = _mm256_blendv_pd(nearest_mrd_distance1, nearest_mrd1, le_mask1);

            nearest_mrd_point0 = _mm256_blendv_epi8(nearest_mrd_point0, possible_indices0, (__m256i)le_mask0);
            nearest_mrd_point1 = _mm256_blendv_epi8(nearest_mrd_point1, possible_indices1, (__m256i)le_mask1);
        }

        double nearest_mrd_distance_arr[4];
        size_t nearest_mrd_point_arr[4];
        __m256d nearest_mrd_distance = _mm256_min_pd(nearest_mrd_distance0, nearest_mrd_distance1);
        __m256d eq_mask0 = _mm256_cmp_pd(nearest_mrd_distance, nearest_mrd_distance0, _CMP_EQ_OQ);
        __m256d eq_mask1 = _mm256_cmp_pd(nearest_mrd_distance, nearest_mrd_distance1, _CMP_EQ_OQ);
        nearest_mrd_point0 = _mm256_blendv_epi8(_mm256_setzero_si256(), nearest_mrd_point0, (__m256i)eq_mask0);
        nearest_mrd_point1 = _mm256_blendv_epi8(_mm256_setzero_si256(), nearest_mrd_point1, (__m256i)eq_mask1);
        __m256i gt_mask = _mm256_cmpgt_epi64(nearest_mrd_point0, nearest_mrd_point1);
        nearest_mrd_point0 = _mm256_blendv_epi8(nearest_mrd_point1, nearest_mrd_point0, gt_mask);

        _mm256_store_pd(nearest_mrd_distance_arr, nearest_mrd_distance);
        _mm256_store_si256((__m256i*)nearest_mrd_point_arr, nearest_mrd_point0);

        for(; neighbor < n_pts - 1; ++neighbor) {
            if(current_point == neighbor) {
                continue;
            }
            if(attached_points[neighbor] == all_int) { // point already attached
                continue;
            }

            double distance = distance_matrix[current_point][neighbor];
            double mutual_reachability_distance = distance;

            if(core_distances[current_point] > mutual_reachability_distance) {
                mutual_reachability_distance = core_distances[current_point];
            }
            if(core_distances[neighbor] > mutual_reachability_distance) {
                mutual_reachability_distance = core_distances[neighbor];
            }

            if(mutual_reachability_distance < nearest_mrd_distances[neighbor]) {
                nearest_mrd_distances[neighbor] = mutual_reachability_distance;
                nearest_mrd_neighbors[neighbor] = current_point;
            }

            //Check if the unattached point being updated is the closest to the tree:
            if(nearest_mrd_distances[neighbor] <= nearest_mrd_distance_arr[3]) {
                nearest_mrd_distance_arr[3] = nearest_mrd_distances[neighbor];
                nearest_mrd_point_arr[3] = neighbor;
            }
        }

        double nearest_mrd_distance_final = fmin(fmin(fmin(nearest_mrd_distance_arr[0], nearest_mrd_distance_arr[1]), nearest_mrd_distance_arr[2]), nearest_mrd_distance_arr[3]);
        current_point = 0;
        if(nearest_mrd_distance_arr[0] == nearest_mrd_distance_final) {
            current_point = nearest_mrd_point_arr[0] > current_point ? nearest_mrd_point_arr[0] : current_point;
        }
        if(nearest_mrd_distance_arr[1] == nearest_mrd_distance_final) {
            current_point = nearest_mrd_point_arr[1] > current_point ? nearest_mrd_point_arr[1] : current_point;
        }
        if(nearest_mrd_distance_arr[2] == nearest_mrd_distance_final) {
            current_point = nearest_mrd_point_arr[2] > current_point ? nearest_mrd_point_arr[2] : current_point;
        }
        if(nearest_mrd_distance_arr[3] == nearest_mrd_distance_final) {
            current_point = nearest_mrd_point_arr[3] > current_point ? nearest_mrd_point_arr[3] : current_point;
        }
        //Attach the closest point found in this iteration to the tree:
        attached_points[current_point] = all_int;
    }
    end = stop_tsc(start);
    timing << "MST Compute," << end << "\n";
    free(attached_points);

    //Create an array for vertices in the tree that each point attached to:
    size_t* other_vertex_indices = (size_t*)calloc(n_pts-1 + self_edge_capacity, sizeof(size_t));
    for(size_t i = 0; i < n_pts; ++i) {
        other_vertex_indices[i] = i;
    }

    //If necessary, attach self edges:
    if(self_edges) {
        for(size_t i = n_pts - 1; i < n_pts*2 - 1; ++i) {
            size_t vertex = i - (n_pts - 1);
            nearest_mrd_neighbors[i] = vertex;
            other_vertex_indices[i] = vertex;
            nearest_mrd_distances[i] = core_distances[vertex];
        }
    }

    start = start_tsc();
    UndirectedGraph_C* result = UDG_Create_Fast(n_pts, nearest_mrd_neighbors, other_vertex_indices, nearest_mrd_distances, n_pts-1 + self_edge_capacity);
    end = stop_tsc(start);
    timing << "Construct graph," << end;
    return result;
}


UndirectedGraph_C* ConstructMST_Bitset_NoCalc_AVX_Unrolled_4(const double* data_set,
    const double* core_distances, bool self_edges,
    DistanceCalculator distance_function, size_t n_pts, size_t point_dimension, double** distance_matrix) {

    std::ofstream timing("mst_timings/bitset_nocalc_avx_unroll4.csv");

    unsigned long long start = start_tsc();
    size_t self_edge_capacity = self_edges ? n_pts : 0;
    const double DOUBLE_MAX = DBL_MAX;
    const size_t SIZE_T_MAX = SIZE_MAX;
    size_t all_int = 0xffffffffffffffff;

    size_t* attached_points = CreateAlignedSizeT1D(n_pts);
    size_t i = 0;
    for(; i < n_pts - 3; i += 4) {
        _mm256_store_si256((__m256i*)(attached_points+i), _mm256_setzero_si256());
    }
    for(; i < n_pts; ++i) {
        attached_points[i] = 0;
    }

    //Each point has a current neighbor point in the tree, and a current nearest distance:
    size_t* nearest_mrd_neighbors = CreateAlignedSizeT1D(n_pts-1 + self_edge_capacity);
    double* nearest_mrd_distances = CreateAlignedDouble1D(n_pts-1 + self_edge_capacity);

    for(size_t i = 0; i < n_pts-1; ++i) {
        nearest_mrd_distances[i] = DOUBLE_MAX;
    }

    //The MST is expanded starting with the last point in the data set:
    size_t current_point = n_pts - 1;
    size_t num_attached_points = 1;
    attached_points[current_point] = all_int;
    unsigned long long end = stop_tsc(start);

    timing << "Setup," << end << "\n";

    start = start_tsc();
    __m256d dbl_max = _mm256_set1_pd(DBL_MAX);
    __m256i all_set_mask = _mm256_set1_epi64x(all_int);
    //Continue attaching points to the MST until all points are attached:
    for(num_attached_points = 1; num_attached_points < n_pts; ++num_attached_points) {
        __m256i nearest_mrd_point0 = _mm256_setzero_si256();
        __m256i nearest_mrd_point1 = _mm256_setzero_si256();
        __m256i nearest_mrd_point2 = _mm256_setzero_si256();
        __m256i nearest_mrd_point3 = _mm256_setzero_si256();

        __m256d nearest_mrd_distance0 = _mm256_set1_pd(DOUBLE_MAX);
        __m256d nearest_mrd_distance1 = _mm256_set1_pd(DOUBLE_MAX);
        __m256d nearest_mrd_distance2 = _mm256_set1_pd(DOUBLE_MAX);
        __m256d nearest_mrd_distance3 = _mm256_set1_pd(DOUBLE_MAX);

        __m256i current_point_intr = _mm256_set1_epi64x(current_point);
        __m256d core_distance_current = _mm256_set1_pd(core_distances[current_point]);

        //Iterate through all unattached points, updating distances using the current point:
        size_t neighbor = 0;
        for(; neighbor < n_pts - 16; neighbor += 16) {
            __m256i attached_mask0 = _mm256_load_si256((__m256i*)(attached_points + neighbor));
            __m256i attached_mask1 = _mm256_load_si256((__m256i*)(attached_points + neighbor + 4));
            __m256i attached_mask2 = _mm256_load_si256((__m256i*)(attached_points + neighbor + 8));
            __m256i attached_mask3 = _mm256_load_si256((__m256i*)(attached_points + neighbor + 12));

            if(vec_equal(attached_mask0, all_set_mask) && vec_equal(attached_mask1, all_set_mask) && vec_equal(attached_mask2, all_set_mask) && vec_equal(attached_mask3, all_set_mask)) {
                continue;
            }

            __m256i possible_indices0 = _mm256_set_epi64x(neighbor + 3, neighbor + 2, neighbor + 1, neighbor);
            __m256i possible_indices1 = _mm256_set_epi64x(neighbor + 7, neighbor + 6, neighbor + 5, neighbor + 4);
            __m256i possible_indices2 = _mm256_set_epi64x(neighbor + 11, neighbor + 10, neighbor + 9, neighbor + 8);
            __m256i possible_indices3 = _mm256_set_epi64x(neighbor + 15, neighbor + 14, neighbor + 13, neighbor + 12);

            __m256i current_point_mask0 = _mm256_cmpeq_epi64(current_point_intr, possible_indices0);
            __m256i current_point_mask1 = _mm256_cmpeq_epi64(current_point_intr, possible_indices1);
            __m256i current_point_mask2 = _mm256_cmpeq_epi64(current_point_intr, possible_indices2);
            __m256i current_point_mask3 = _mm256_cmpeq_epi64(current_point_intr, possible_indices3);

            __m256d nearest_mrd0 = _mm256_load_pd(nearest_mrd_distances + neighbor);
            __m256d nearest_mrd1 = _mm256_load_pd(nearest_mrd_distances + neighbor + 4);
            __m256d nearest_mrd2 = _mm256_load_pd(nearest_mrd_distances + neighbor + 8);
            __m256d nearest_mrd3 = _mm256_load_pd(nearest_mrd_distances + neighbor + 12);

            __m256i nearest_neighbors0 = _mm256_load_si256(((__m256i *)(nearest_mrd_neighbors + neighbor)));
            __m256i nearest_neighbors1 = _mm256_load_si256(((__m256i *)(nearest_mrd_neighbors + neighbor + 4)));
            __m256i nearest_neighbors2 = _mm256_load_si256(((__m256i *)(nearest_mrd_neighbors + neighbor + 8)));
            __m256i nearest_neighbors3 = _mm256_load_si256(((__m256i *)(nearest_mrd_neighbors + neighbor + 12)));
            __m256d core_distances_neighbor0 = _mm256_load_pd(core_distances + neighbor);
            __m256d core_distances_neighbor1 = _mm256_load_pd(core_distances + neighbor + 4);
            __m256d core_distances_neighbor2 = _mm256_load_pd(core_distances + neighbor + 8);
            __m256d core_distances_neighbor3 = _mm256_load_pd(core_distances + neighbor + 12);

            __m256d mrd0 = _mm256_load_pd(distance_matrix[current_point] + neighbor);
            __m256d mrd1 = _mm256_load_pd(distance_matrix[current_point] + neighbor + 4);
            __m256d mrd2 = _mm256_load_pd(distance_matrix[current_point] + neighbor + 8);
            __m256d mrd3 = _mm256_load_pd(distance_matrix[current_point] + neighbor + 12);

            mrd0 = _mm256_max_pd(core_distance_current, mrd0);
            mrd1 = _mm256_max_pd(core_distance_current, mrd1);
            mrd2 = _mm256_max_pd(core_distance_current, mrd2);
            mrd3 = _mm256_max_pd(core_distance_current, mrd3);

            mrd0 = _mm256_max_pd(core_distances_neighbor0, mrd0);
            mrd1 = _mm256_max_pd(core_distances_neighbor1, mrd1);
            mrd2 = _mm256_max_pd(core_distances_neighbor2, mrd2);
            mrd3 = _mm256_max_pd(core_distances_neighbor3, mrd3);

            mrd0 = _mm256_blendv_pd(mrd0, dbl_max, (__m256d)_mm256_or_si256(attached_mask0, current_point_mask0));
            mrd1 = _mm256_blendv_pd(mrd1, dbl_max, (__m256d)_mm256_or_si256(attached_mask1, current_point_mask1));
            mrd2 = _mm256_blendv_pd(mrd2, dbl_max, (__m256d)_mm256_or_si256(attached_mask2, current_point_mask2));
            mrd3 = _mm256_blendv_pd(mrd3, dbl_max, (__m256d)_mm256_or_si256(attached_mask3, current_point_mask3));

            __m256d lt_mask0 = _mm256_cmp_pd(mrd0, nearest_mrd0, _CMP_LT_OQ);
            __m256d lt_mask1 = _mm256_cmp_pd(mrd1, nearest_mrd1, _CMP_LT_OQ);
            __m256d lt_mask2 = _mm256_cmp_pd(mrd2, nearest_mrd2, _CMP_LT_OQ);
            __m256d lt_mask3 = _mm256_cmp_pd(mrd3, nearest_mrd3, _CMP_LT_OQ);

            nearest_mrd0 = _mm256_blendv_pd(nearest_mrd0, mrd0, lt_mask0);
            nearest_mrd1 = _mm256_blendv_pd(nearest_mrd1, mrd1, lt_mask1);
            nearest_mrd2 = _mm256_blendv_pd(nearest_mrd2, mrd2, lt_mask2);
            nearest_mrd3 = _mm256_blendv_pd(nearest_mrd3, mrd3, lt_mask3);

            nearest_neighbors0 = _mm256_blendv_epi8(nearest_neighbors0, current_point_intr, (__m256i)lt_mask0);
            nearest_neighbors1 = _mm256_blendv_epi8(nearest_neighbors1, current_point_intr, (__m256i)lt_mask1);
            nearest_neighbors2 = _mm256_blendv_epi8(nearest_neighbors2, current_point_intr, (__m256i)lt_mask2);
            nearest_neighbors3 = _mm256_blendv_epi8(nearest_neighbors3, current_point_intr, (__m256i)lt_mask3);

            _mm256_store_pd(nearest_mrd_distances + neighbor, nearest_mrd0);
            _mm256_store_pd(nearest_mrd_distances + neighbor + 4, nearest_mrd1);
            _mm256_store_pd(nearest_mrd_distances + neighbor + 8, nearest_mrd2);
            _mm256_store_pd(nearest_mrd_distances + neighbor + 12, nearest_mrd3);
            _mm256_store_si256((__m256i *)(nearest_mrd_neighbors + neighbor), nearest_neighbors0);
            _mm256_store_si256((__m256i *)(nearest_mrd_neighbors + neighbor + 4), nearest_neighbors1);
            _mm256_store_si256((__m256i *)(nearest_mrd_neighbors + neighbor + 8), nearest_neighbors2);
            _mm256_store_si256((__m256i *)(nearest_mrd_neighbors + neighbor + 12), nearest_neighbors3);

            __m256d max_mask0 = _mm256_cmp_pd(mrd0, dbl_max, _CMP_NEQ_OQ);
            __m256d max_mask1 = _mm256_cmp_pd(mrd1, dbl_max, _CMP_NEQ_OQ);
            __m256d max_mask2 = _mm256_cmp_pd(mrd2, dbl_max, _CMP_NEQ_OQ);
            __m256d max_mask3 = _mm256_cmp_pd(mrd3, dbl_max, _CMP_NEQ_OQ);

            __m256d le_mask0 = _mm256_cmp_pd(nearest_mrd0, nearest_mrd_distance0, _CMP_LE_OQ);
            __m256d le_mask1 = _mm256_cmp_pd(nearest_mrd1, nearest_mrd_distance1, _CMP_LE_OQ);
            __m256d le_mask2 = _mm256_cmp_pd(nearest_mrd2, nearest_mrd_distance2, _CMP_LE_OQ);
            __m256d le_mask3 = _mm256_cmp_pd(nearest_mrd3, nearest_mrd_distance3, _CMP_LE_OQ);

            le_mask0 = _mm256_and_pd(max_mask0, le_mask0);
            le_mask1 = _mm256_and_pd(max_mask1, le_mask1);
            le_mask2 = _mm256_and_pd(max_mask2, le_mask2);
            le_mask3 = _mm256_and_pd(max_mask3, le_mask3);

            nearest_mrd_distance0 = _mm256_blendv_pd(nearest_mrd_distance0, nearest_mrd0, le_mask0);
            nearest_mrd_distance1 = _mm256_blendv_pd(nearest_mrd_distance1, nearest_mrd1, le_mask1);
            nearest_mrd_distance2 = _mm256_blendv_pd(nearest_mrd_distance2, nearest_mrd2, le_mask2);
            nearest_mrd_distance3 = _mm256_blendv_pd(nearest_mrd_distance3, nearest_mrd3, le_mask3);

            nearest_mrd_point0 = _mm256_blendv_epi8(nearest_mrd_point0, possible_indices0, (__m256i)le_mask0);
            nearest_mrd_point1 = _mm256_blendv_epi8(nearest_mrd_point1, possible_indices1, (__m256i)le_mask1);
            nearest_mrd_point2 = _mm256_blendv_epi8(nearest_mrd_point2, possible_indices2, (__m256i)le_mask2);
            nearest_mrd_point3 = _mm256_blendv_epi8(nearest_mrd_point3, possible_indices3, (__m256i)le_mask3);
        }
        double nearest_mrd_distance_arr[4];
        size_t nearest_mrd_point_arr[4];

        // Reduce to two distances and points
        __m256d nearest_mrd_distance_r0 = _mm256_min_pd(nearest_mrd_distance0, nearest_mrd_distance2);
        __m256d eq_mask0 = _mm256_cmp_pd(nearest_mrd_distance_r0, nearest_mrd_distance0, _CMP_EQ_OQ);
        __m256d eq_mask1 = _mm256_cmp_pd(nearest_mrd_distance_r0, nearest_mrd_distance2, _CMP_EQ_OQ);
        nearest_mrd_point0 = _mm256_blendv_epi8(_mm256_setzero_si256(), nearest_mrd_point0, (__m256i)eq_mask0);
        nearest_mrd_point2 = _mm256_blendv_epi8(_mm256_setzero_si256(), nearest_mrd_point2, (__m256i)eq_mask1);
        __m256i gt_mask = _mm256_cmpgt_epi64(nearest_mrd_point0, nearest_mrd_point2);
        nearest_mrd_point0 = _mm256_blendv_epi8(nearest_mrd_point2, nearest_mrd_point0, gt_mask);
        nearest_mrd_distance0 = nearest_mrd_distance_r0;

        __m256d nearest_mrd_distance_r1 = _mm256_min_pd(nearest_mrd_distance1, nearest_mrd_distance3);
        eq_mask0 = _mm256_cmp_pd(nearest_mrd_distance_r1, nearest_mrd_distance1, _CMP_EQ_OQ);
        eq_mask1 = _mm256_cmp_pd(nearest_mrd_distance_r1, nearest_mrd_distance3, _CMP_EQ_OQ);
        nearest_mrd_point1 = _mm256_blendv_epi8(_mm256_setzero_si256(), nearest_mrd_point1, (__m256i)eq_mask0);
        nearest_mrd_point3 = _mm256_blendv_epi8(_mm256_setzero_si256(), nearest_mrd_point3, (__m256i)eq_mask1);
        gt_mask = _mm256_cmpgt_epi64(nearest_mrd_point1, nearest_mrd_point3);
        nearest_mrd_point1 = _mm256_blendv_epi8(nearest_mrd_point3, nearest_mrd_point1, gt_mask);
        nearest_mrd_distance1 = nearest_mrd_distance_r1;

        // for(; neighbor < n_pts - 8; neighbor += 8) {
        //     int mask0 = GetMask(attached_points, neighbor) & 0xf;
        //     int mask1 = GetMask(attached_points, neighbor + 4) & 0xf;
        //     if(mask0 + mask1 == 30) {
        //         continue;
        //     }
        //     __m256i possible_indices0 = _mm256_set_epi64x(neighbor + 3, neighbor + 2, neighbor + 1, neighbor);
        //     __m256i possible_indices1 = _mm256_set_epi64x(neighbor + 7, neighbor + 6, neighbor + 5, neighbor + 4);

        //     __m256i current_point_mask0 = _mm256_cmpeq_epi64(current_point_intr, possible_indices0);
        //     __m256i current_point_mask1 = _mm256_cmpeq_epi64(current_point_intr, possible_indices1);

        //     __m256i attached_mask0 = _mm256_set_epi64x( // AVX512 has much nices ways :(
        //         mask0 & 8 ? all_int : 0,
        //         mask0 & 4 ? all_int : 0,
        //         mask0 & 2 ? all_int : 0,
        //         mask0 & 1 ? all_int : 0
        //     );
        //     __m256i attached_mask1 = _mm256_set_epi64x( // AVX512 has much nices ways :(
        //         mask1 & 8 ? all_int : 0,
        //         mask1 & 4 ? all_int : 0,
        //         mask1 & 2 ? all_int : 0,
        //         mask1 & 1 ? all_int : 0
        //     );
        //     __m256d nearest_mrd0 = _mm256_load_pd(nearest_mrd_distances + neighbor);
        //     __m256d nearest_mrd1 = _mm256_load_pd(nearest_mrd_distances + neighbor + 4);

        //     __m256i nearest_neighbors0 = _mm256_load_si256(((__m256i *)(nearest_mrd_neighbors + neighbor)));
        //     __m256i nearest_neighbors1 = _mm256_load_si256(((__m256i *)(nearest_mrd_neighbors + neighbor + 4)));
        //     __m256d core_distances_neighbor0 = _mm256_load_pd(core_distances + neighbor);
        //     __m256d core_distances_neighbor1 = _mm256_load_pd(core_distances + neighbor + 4);

        //     __m256d mrd0 = _mm256_load_pd(distance_matrix[current_point] + neighbor);
        //     __m256d mrd1 = _mm256_load_pd(distance_matrix[current_point] + neighbor + 4);
        //     mrd0 = _mm256_max_pd(core_distance_current, mrd0);
        //     mrd1 = _mm256_max_pd(core_distance_current, mrd1);

        //     mrd0 = _mm256_max_pd(core_distances_neighbor0, mrd0);
        //     mrd1 = _mm256_max_pd(core_distances_neighbor1, mrd1);

        //     mrd0 = _mm256_blendv_pd(mrd0, dbl_max, (__m256d)_mm256_or_si256(attached_mask0, current_point_mask0));
        //     mrd1 = _mm256_blendv_pd(mrd1, dbl_max, (__m256d)_mm256_or_si256(attached_mask1, current_point_mask1));

        //     __m256d lt_mask0 = _mm256_cmp_pd(mrd0, nearest_mrd0, _CMP_LT_OQ);
        //     __m256d lt_mask1 = _mm256_cmp_pd(mrd1, nearest_mrd1, _CMP_LT_OQ);

        //     nearest_mrd0 = _mm256_blendv_pd(nearest_mrd0, mrd0, lt_mask0);
        //     nearest_mrd1 = _mm256_blendv_pd(nearest_mrd1, mrd1, lt_mask1);

        //     nearest_neighbors0 = _mm256_blendv_epi8(nearest_neighbors0, current_point_intr, (__m256i)lt_mask0);
        //     nearest_neighbors1 = _mm256_blendv_epi8(nearest_neighbors1, current_point_intr, (__m256i)lt_mask1);

        //     _mm256_store_pd(nearest_mrd_distances + neighbor, nearest_mrd0);
        //     _mm256_store_pd(nearest_mrd_distances + neighbor + 4, nearest_mrd1);
        //     _mm256_store_si256((__m256i *)(nearest_mrd_neighbors + neighbor), nearest_neighbors0);
        //     _mm256_store_si256((__m256i *)(nearest_mrd_neighbors + neighbor + 4), nearest_neighbors1);

        //     __m256d max_mask0 = _mm256_cmp_pd(mrd0, dbl_max, _CMP_NEQ_OQ);
        //     __m256d max_mask1 = _mm256_cmp_pd(mrd1, dbl_max, _CMP_NEQ_OQ);

        //     __m256d le_mask0 = _mm256_cmp_pd(nearest_mrd0, nearest_mrd_distance0, _CMP_LE_OQ);
        //     __m256d le_mask1 = _mm256_cmp_pd(nearest_mrd1, nearest_mrd_distance1, _CMP_LE_OQ);

        //     le_mask0 = _mm256_and_pd(max_mask0, le_mask0);
        //     le_mask1 = _mm256_and_pd(max_mask1, le_mask1);

        //     nearest_mrd_distance0 = _mm256_blendv_pd(nearest_mrd_distance0, nearest_mrd0, le_mask0);
        //     nearest_mrd_distance1 = _mm256_blendv_pd(nearest_mrd_distance1, nearest_mrd1, le_mask1);

        //     nearest_mrd_point0 = _mm256_blendv_epi8(nearest_mrd_point0, possible_indices0, (__m256i)le_mask0);
        //     nearest_mrd_point1 = _mm256_blendv_epi8(nearest_mrd_point1, possible_indices1, (__m256i)le_mask1);
        // }

        __m256d nearest_mrd_distance = _mm256_min_pd(nearest_mrd_distance0, nearest_mrd_distance1);
        eq_mask0 = _mm256_cmp_pd(nearest_mrd_distance, nearest_mrd_distance0, _CMP_EQ_OQ);
        eq_mask1 = _mm256_cmp_pd(nearest_mrd_distance, nearest_mrd_distance1, _CMP_EQ_OQ);
        nearest_mrd_point0 = _mm256_blendv_epi8(_mm256_setzero_si256(), nearest_mrd_point0, (__m256i)eq_mask0);
        nearest_mrd_point1 = _mm256_blendv_epi8(_mm256_setzero_si256(), nearest_mrd_point1, (__m256i)eq_mask1);
        gt_mask = _mm256_cmpgt_epi64(nearest_mrd_point0, nearest_mrd_point1);
        nearest_mrd_point0 = _mm256_blendv_epi8(nearest_mrd_point1, nearest_mrd_point0, gt_mask);

        _mm256_store_pd(nearest_mrd_distance_arr, nearest_mrd_distance);
        _mm256_store_si256((__m256i*)nearest_mrd_point_arr, nearest_mrd_point0);

        for(; neighbor < n_pts - 1; ++neighbor) {
            if(current_point == neighbor) {
                continue;
            }
            if(attached_points[neighbor] == all_int) { // point already attached
                continue;
            }

            double distance = distance_matrix[current_point][neighbor];
            double mutual_reachability_distance = distance;

            if(core_distances[current_point] > mutual_reachability_distance) {
                mutual_reachability_distance = core_distances[current_point];
            }
            if(core_distances[neighbor] > mutual_reachability_distance) {
                mutual_reachability_distance = core_distances[neighbor];
            }

            if(mutual_reachability_distance < nearest_mrd_distances[neighbor]) {
                nearest_mrd_distances[neighbor] = mutual_reachability_distance;
                nearest_mrd_neighbors[neighbor] = current_point;
            }

            //Check if the unattached point being updated is the closest to the tree:
            if(nearest_mrd_distances[neighbor] <= nearest_mrd_distance_arr[3]) {
                nearest_mrd_distance_arr[3] = nearest_mrd_distances[neighbor];
                nearest_mrd_point_arr[3] = neighbor;
            }
        }

        double nearest_mrd_distance_final = fmin(fmin(fmin(nearest_mrd_distance_arr[0], nearest_mrd_distance_arr[1]), nearest_mrd_distance_arr[2]), nearest_mrd_distance_arr[3]);
        current_point = 0;
        if(nearest_mrd_distance_arr[0] == nearest_mrd_distance_final) {
            current_point = nearest_mrd_point_arr[0] > current_point ? nearest_mrd_point_arr[0] : current_point;
        }
        if(nearest_mrd_distance_arr[1] == nearest_mrd_distance_final) {
            current_point = nearest_mrd_point_arr[1] > current_point ? nearest_mrd_point_arr[1] : current_point;
        }
        if(nearest_mrd_distance_arr[2] == nearest_mrd_distance_final) {
            current_point = nearest_mrd_point_arr[2] > current_point ? nearest_mrd_point_arr[2] : current_point;
        }
        if(nearest_mrd_distance_arr[3] == nearest_mrd_distance_final) {
            current_point = nearest_mrd_point_arr[3] > current_point ? nearest_mrd_point_arr[3] : current_point;
        }
        //Attach the closest point found in this iteration to the tree:
        attached_points[current_point] = all_int;
    }
    end = stop_tsc(start);
    timing << "MST Compute," << end << "\n";
    free(attached_points);

    //Create an array for vertices in the tree that each point attached to:
    size_t* other_vertex_indices = (size_t*)calloc(n_pts-1 + self_edge_capacity, sizeof(size_t));
    for(size_t i = 0; i < n_pts; ++i) {
        other_vertex_indices[i] = i;
    }

    //If necessary, attach self edges:
    if(self_edges) {
        for(size_t i = n_pts - 1; i < n_pts*2 - 1; ++i) {
            size_t vertex = i - (n_pts - 1);
            nearest_mrd_neighbors[i] = vertex;
            other_vertex_indices[i] = vertex;
            nearest_mrd_distances[i] = core_distances[vertex];
        }
    }

    start = start_tsc();
    UndirectedGraph_C* result = UDG_Create_Fast(n_pts, nearest_mrd_neighbors, other_vertex_indices, nearest_mrd_distances, n_pts-1 + self_edge_capacity);
    end = stop_tsc(start);
    timing << "Construct graph," << end;
    return result;
}
#endif //__AVX2__


UndirectedGraph_C* ConstructMST_Unrolled_NoBitset_CalcDistances(const double* data_set,
    const double* core_distances, bool self_edges,
    DistanceCalculator distance_function, size_t n_pts, size_t point_dimension, double** distance_matrix) {

    std::ofstream timing("mst_timings/nobitset.csv");

    unsigned long long start = start_tsc();
    size_t self_edge_capacity = self_edges ? n_pts : 0;
    const double DOUBLE_MAX = DBL_MAX;
    const size_t SIZE_T_MAX = SIZE_MAX;

    //One bit is set (true) for each attached point, or unset (false) for unattached points:
    BitSet_t attached_points = CreateBitset(n_pts, false);

    //Each point has a current neighbor point in the tree, and a current nearest distance:
    size_t* nearest_mrd_neighbors = (size_t*)malloc((n_pts-1 + self_edge_capacity)*sizeof(size_t));
    double* nearest_mrd_distances = (double*)malloc((n_pts-1 + self_edge_capacity)*sizeof(double));

    for(size_t i = 0; i < n_pts-1; ++i) {
        nearest_mrd_distances[i] = DOUBLE_MAX;
    }

    //The MST is expanded starting with the last point in the data set:
    size_t current_point = n_pts - 1;
    size_t num_attached_points = 1;
    unsigned long long end = stop_tsc(start);

    timing << "Setup," << end << "\n";

    start = start_tsc();
    // Calculate MRD
    double** mrd_matrix = (double**)malloc((n_pts)*sizeof(double*));
    for(size_t i = 0; i < n_pts; ++i) {
        mrd_matrix[i] = (double*)malloc(n_pts*sizeof(double));
    }

    for(size_t pt_x = 0; pt_x < n_pts; ++pt_x) {
        const double* x = &data_set[pt_x*point_dimension];
        const double cd_x = core_distances[pt_x];

        mrd_matrix[pt_x][pt_x] = DBL_MAX;
        size_t pt_y = pt_x + 1;
        for(; pt_y < n_pts - 3; pt_y += 4) {
            double d0 = distance_function(x, &data_set[pt_y*point_dimension], point_dimension);
            double cd_y0 = core_distances[pt_y];
            double d1 = distance_function(x, &data_set[(pt_y + 1)*point_dimension], point_dimension);
            double cd_y1 = core_distances[pt_y + 1];
            double d2 = distance_function(x, &data_set[(pt_y + 2)*point_dimension], point_dimension);
            double cd_y2 = core_distances[pt_y + 2];
            double d3 = distance_function(x, &data_set[(pt_y + 3)*point_dimension], point_dimension);
            double cd_y3 = core_distances[pt_y + 3];

            double mrd0 = d0;
            double mrd1 = d1;
            double mrd2 = d2;
            double mrd3 = d3;

            mrd0 = fmax(mrd0, cd_x);
            mrd1 = fmax(mrd1, cd_x);
            mrd2 = fmax(mrd2, cd_x);
            mrd3 = fmax(mrd3, cd_x);

            mrd0 = fmax(mrd0, cd_y0);
            mrd1 = fmax(mrd1, cd_y1);
            mrd2 = fmax(mrd2, cd_y2);
            mrd3 = fmax(mrd3, cd_y3);

            mrd_matrix[pt_x][pt_y] = mrd0;
            mrd_matrix[pt_y][pt_x] = mrd0;
            mrd_matrix[pt_x][pt_y + 1] = mrd1;
            mrd_matrix[pt_y + 1][pt_x] = mrd1;
            mrd_matrix[pt_x][pt_y + 2] = mrd2;
            mrd_matrix[pt_y + 2][pt_x] = mrd2;
            mrd_matrix[pt_x][pt_y + 3] = mrd3;
            mrd_matrix[pt_y + 3][pt_x] = mrd3;
        }
        for(; pt_y < n_pts; ++pt_y) {
            double d0 = distance_function(x, &data_set[pt_y*point_dimension], point_dimension);
            double cd_y0 = core_distances[pt_y];

            double mrd = d0;
            if(cd_x > mrd) {
                mrd = cd_x;
            }
            if(cd_y0 > mrd) {
                mrd = cd_y0;
            }
            mrd_matrix[pt_x][pt_y] = mrd;
            mrd_matrix[pt_y][pt_x] = mrd;
        }
    }
    end = stop_tsc(start);
    timing << "MRD computation," << end << "\n";

    start = start_tsc();
    for(; num_attached_points < n_pts; ++num_attached_points) {
        // find index of minimal mrd
        double* row = mrd_matrix[current_point];
        size_t min_index0 = 0;
        double min_value0 = DBL_MAX;
        size_t min_index1 = 0;
        double min_value1 = DBL_MAX;
        size_t min_index2 = 0;
        double min_value2 = DBL_MAX;
        size_t min_index3 = 0;
        double min_value3 = DBL_MAX;
        size_t j = 0;
        for(; j < n_pts-4; j += 4) {
            mrd_matrix[j][current_point] = DBL_MAX;
            mrd_matrix[j+1][current_point] = DBL_MAX;
            mrd_matrix[j+2][current_point] = DBL_MAX;
            mrd_matrix[j+3][current_point] = DBL_MAX;

            double mrd0 = row[j];
            double mrd1 = row[j + 1];
            double mrd2 = row[j + 2];
            double mrd3 = row[j + 3];

            if(mrd0 < nearest_mrd_distances[j]) {
                nearest_mrd_distances[j] = mrd0;
                nearest_mrd_neighbors[j] = current_point;
            }
            if(mrd1 < nearest_mrd_distances[j + 1]) {
                nearest_mrd_distances[j + 1] = mrd1;
                nearest_mrd_neighbors[j + 1] = current_point;
            }
            if(mrd2 < nearest_mrd_distances[j + 2]) {
                nearest_mrd_distances[j + 2] = mrd2;
                nearest_mrd_neighbors[j + 2] = current_point;
            }
            if(mrd3 < nearest_mrd_distances[j + 3]) {
                nearest_mrd_distances[j + 3] = mrd3;
                nearest_mrd_neighbors[j + 3] = current_point;
            }

            if(mrd0 != DBL_MAX && nearest_mrd_distances[j] <= min_value0) {
                min_index0 = j;
                min_value0 = nearest_mrd_distances[j] ;
            }
            if(mrd1 != DBL_MAX && nearest_mrd_distances[j + 1] <= min_value1) {
                min_index1 = j + 1;
                min_value1 = nearest_mrd_distances[j + 1];
            }
            if(mrd2 != DBL_MAX && nearest_mrd_distances[j + 2] <= min_value2) {
                min_index2 = j + 2;
                min_value2 = nearest_mrd_distances[j + 2] ;
            }
            if(mrd3 != DBL_MAX && nearest_mrd_distances[j + 3] <= min_value3) {
                min_index3 = j + 3;
                min_value3 = nearest_mrd_distances[j + 3] ;
            }
        }
        for(; j < n_pts-1; ++j) {
            mrd_matrix[j][current_point] = DBL_MAX;
            double mrd3 = row[j];
            if(mrd3 < nearest_mrd_distances[j]) {
                nearest_mrd_distances[j] = mrd3;
                nearest_mrd_neighbors[j] = current_point;
            }
            if(mrd3 != DBL_MAX && nearest_mrd_distances[j] <= min_value3) {
                min_index3 = j;
                min_value3 = nearest_mrd_distances[j] ;
            }
        }

        double min_value = fmin(fmin(fmin(min_value0, min_value1), min_value2), min_value3);
        current_point = 0;
        if(min_value0 == min_value) {
            current_point = min_index0;
        }
        if(min_value1 == min_value) {
            current_point = min_index1 > current_point ? min_index1 : current_point;
        }
        if(min_value2 == min_value) {
            current_point = min_index2 > current_point ? min_index2 : current_point;
        }
        if(min_value3 == min_value) {
            current_point = min_index3 > current_point ? min_index3 : current_point;
        }
    }
    end = stop_tsc(start);
    timing << "MST Compute," << end << "\n";
    for(size_t i = 0; i < n_pts; ++i) {
        free(mrd_matrix[i]);
    }
    free(mrd_matrix);

    //Create an array for vertices in the tree that each point attached to:
    size_t* other_vertex_indices = (size_t*)calloc(n_pts-1 + self_edge_capacity, sizeof(size_t));
    for(size_t i = 0; i < n_pts; ++i) {
        other_vertex_indices[i] = i;
    }

    //If necessary, attach self edges:
    if(self_edges) {
        for(size_t i = n_pts - 1; i < n_pts*2 - 1; ++i) {
            size_t vertex = i - (n_pts - 1);
            nearest_mrd_neighbors[i] = vertex;
            other_vertex_indices[i] = vertex;
            nearest_mrd_distances[i] = core_distances[vertex];
        }
    }

    start = start_tsc();
    UndirectedGraph_C* result = UDG_Create_Fast(n_pts, nearest_mrd_neighbors, other_vertex_indices, nearest_mrd_distances, n_pts-1 + self_edge_capacity);
    end = stop_tsc(start);
    timing << "Construct graph," << end;
    return result;
}

UndirectedGraph_C* ConstructMST_Unrolled_NoBitset_NoCalcDistances(const double* data_set,
    const double* core_distances, bool self_edges,
    DistanceCalculator distance_function, size_t n_pts, size_t point_dimension, double** distance_matrix) {

    std::ofstream timing("mst_timings/nobitset_nocalc.csv");

    unsigned long long start = start_tsc();
    size_t self_edge_capacity = self_edges ? n_pts : 0;
    const double DOUBLE_MAX = DBL_MAX;
    const size_t SIZE_T_MAX = SIZE_MAX;

    //One bit is set (true) for each attached point, or unset (false) for unattached points:
    BitSet_t attached_points = CreateBitset(n_pts, false);

    //Each point has a current neighbor point in the tree, and a current nearest distance:
    size_t* nearest_mrd_neighbors = (size_t*)malloc((n_pts-1 + self_edge_capacity)*sizeof(size_t));
    double* nearest_mrd_distances = (double*)malloc((n_pts-1 + self_edge_capacity)*sizeof(double));

    for(size_t i = 0; i < n_pts-1; ++i) {
        nearest_mrd_distances[i] = DOUBLE_MAX;
    }

    //The MST is expanded starting with the last point in the data set:
    size_t current_point = n_pts - 1;
    size_t num_attached_points = 1;
    unsigned long long end = stop_tsc(start);

    timing << "Setup," << end << "\n";

    start = start_tsc();
    // Calculate MRD
    double** mrd_matrix = (double**)malloc((n_pts)*sizeof(double*));
    for(size_t i = 0; i < n_pts; ++i) {
        mrd_matrix[i] = (double*)malloc(n_pts*sizeof(double));
    }

    for(size_t pt_x = 0; pt_x < n_pts; ++pt_x) {
        const double* x = distance_matrix[pt_x];
        const double cd_x = core_distances[pt_x];

        mrd_matrix[pt_x][pt_x] = DBL_MAX;
        size_t pt_y = pt_x + 1;
        for(; pt_y < n_pts - 3; pt_y += 4) {
            double d0 = x[pt_y];
            double cd_y0 = core_distances[pt_y];
            double d1 = x[pt_y + 1];
            double cd_y1 = core_distances[pt_y + 1];
            double d2 = x[pt_y + 2];
            double cd_y2 = core_distances[pt_y + 2];
            double d3 = x[pt_y + 3];
            double cd_y3 = core_distances[pt_y + 3];

            double mrd0 = d0;
            double mrd1 = d1;
            double mrd2 = d2;
            double mrd3 = d3;

            mrd0 = fmax(mrd0, cd_x);
            mrd1 = fmax(mrd1, cd_x);
            mrd2 = fmax(mrd2, cd_x);
            mrd3 = fmax(mrd3, cd_x);

            mrd0 = fmax(mrd0, cd_y0);
            mrd1 = fmax(mrd1, cd_y1);
            mrd2 = fmax(mrd2, cd_y2);
            mrd3 = fmax(mrd3, cd_y3);

            mrd_matrix[pt_x][pt_y] = mrd0;
            mrd_matrix[pt_y][pt_x] = mrd0;
            mrd_matrix[pt_x][pt_y + 1] = mrd1;
            mrd_matrix[pt_y + 1][pt_x] = mrd1;
            mrd_matrix[pt_x][pt_y + 2] = mrd2;
            mrd_matrix[pt_y + 2][pt_x] = mrd2;
            mrd_matrix[pt_x][pt_y + 3] = mrd3;
            mrd_matrix[pt_y + 3][pt_x] = mrd3;
        }
        for(; pt_y < n_pts; ++pt_y) {
            double d0 = distance_matrix[pt_x][pt_y];
            double cd_y0 = core_distances[pt_y];

            double mrd = d0;
            if(cd_x > mrd) {
                mrd = cd_x;
            }
            if(cd_y0 > mrd) {
                mrd = cd_y0;
            }
            mrd_matrix[pt_x][pt_y] = mrd;
            mrd_matrix[pt_y][pt_x] = mrd;
        }
    }
    end = stop_tsc(start);
    timing << "MRD computation," << end << "\n";

    start = start_tsc();
    for(; num_attached_points < n_pts; ++num_attached_points) {
        // find index of minimal mrd
        double* row = mrd_matrix[current_point];
        size_t min_index0 = 0;
        double min_value0 = DBL_MAX;
        size_t min_index1 = 0;
        double min_value1 = DBL_MAX;
        size_t min_index2 = 0;
        double min_value2 = DBL_MAX;
        size_t min_index3 = 0;
        double min_value3 = DBL_MAX;
        size_t j = 0;
        for(; j < n_pts-4; j += 4) {
            mrd_matrix[j][current_point] = DBL_MAX;
            mrd_matrix[j+1][current_point] = DBL_MAX;
            mrd_matrix[j+2][current_point] = DBL_MAX;
            mrd_matrix[j+3][current_point] = DBL_MAX;

            double mrd0 = row[j];
            double mrd1 = row[j + 1];
            double mrd2 = row[j + 2];
            double mrd3 = row[j + 3];

            if(mrd0 < nearest_mrd_distances[j]) {
                nearest_mrd_distances[j] = mrd0;
                nearest_mrd_neighbors[j] = current_point;
            }
            if(mrd1 < nearest_mrd_distances[j + 1]) {
                nearest_mrd_distances[j + 1] = mrd1;
                nearest_mrd_neighbors[j + 1] = current_point;
            }
            if(mrd2 < nearest_mrd_distances[j + 2]) {
                nearest_mrd_distances[j + 2] = mrd2;
                nearest_mrd_neighbors[j + 2] = current_point;
            }
            if(mrd3 < nearest_mrd_distances[j + 3]) {
                nearest_mrd_distances[j + 3] = mrd3;
                nearest_mrd_neighbors[j + 3] = current_point;
            }

            if(mrd0 != DBL_MAX && nearest_mrd_distances[j] <= min_value0) {
                min_index0 = j;
                min_value0 = nearest_mrd_distances[j] ;
            }
            if(mrd1 != DBL_MAX && nearest_mrd_distances[j + 1] <= min_value1) {
                min_index1 = j + 1;
                min_value1 = nearest_mrd_distances[j + 1];
            }
            if(mrd2 != DBL_MAX && nearest_mrd_distances[j + 2] <= min_value2) {
                min_index2 = j + 2;
                min_value2 = nearest_mrd_distances[j + 2] ;
            }
            if(mrd3 != DBL_MAX && nearest_mrd_distances[j + 3] <= min_value3) {
                min_index3 = j + 3;
                min_value3 = nearest_mrd_distances[j + 3] ;
            }
        }
        for(; j < n_pts-1; ++j) {
            mrd_matrix[j][current_point] = DBL_MAX;
            double mrd3 = row[j];
            if(mrd3 < nearest_mrd_distances[j]) {
                nearest_mrd_distances[j] = mrd3;
                nearest_mrd_neighbors[j] = current_point;
            }
            if(mrd3 != DBL_MAX && nearest_mrd_distances[j] <= min_value3) {
                min_index3 = j;
                min_value3 = nearest_mrd_distances[j] ;
            }
        }

        double min_value = fmin(fmin(fmin(min_value0, min_value1), min_value2), min_value3);
        current_point = 0;
        if(min_value0 == min_value) {
            current_point = min_index0;
        }
        if(min_value1 == min_value) {
            current_point = min_index1 > current_point ? min_index1 : current_point;
        }
        if(min_value2 == min_value) {
            current_point = min_index2 > current_point ? min_index2 : current_point;
        }
        if(min_value3 == min_value) {
            current_point = min_index3 > current_point ? min_index3 : current_point;
        }
    }
    end = stop_tsc(start);
    timing << "MST Compute," << end << "\n";
    for(size_t i = 0; i < n_pts; ++i) {
        free(mrd_matrix[i]);
    }
    free(mrd_matrix);

    //Create an array for vertices in the tree that each point attached to:
    size_t* other_vertex_indices = (size_t*)calloc(n_pts-1 + self_edge_capacity, sizeof(size_t));
    for(size_t i = 0; i < n_pts; ++i) {
        other_vertex_indices[i] = i;
    }

    //If necessary, attach self edges:
    if(self_edges) {
        for(size_t i = n_pts - 1; i < n_pts*2 - 1; ++i) {
            size_t vertex = i - (n_pts - 1);
            nearest_mrd_neighbors[i] = vertex;
            other_vertex_indices[i] = vertex;
            nearest_mrd_distances[i] = core_distances[vertex];
        }
    }

    start = start_tsc();
    UndirectedGraph_C* result = UDG_Create_Fast(n_pts, nearest_mrd_neighbors, other_vertex_indices, nearest_mrd_distances, n_pts-1 + self_edge_capacity);
    end = stop_tsc(start);
    timing << "Construct graph," << end;
    return result;
}


#ifdef __AVX2__
UndirectedGraph_C* ConstructMST_NoBitset_NoCalcDistances_AVX256(const double* data_set,
    const double* core_distances, bool self_edges,
    DistanceCalculator distance_function, size_t n_pts, size_t point_dimension, double** distance_matrix) {

    std::ofstream timing("mst_timings/nobitset_nocalc_avx.csv");

    unsigned long long start = start_tsc();
    size_t self_edge_capacity = self_edges ? n_pts : 0;
    const double DOUBLE_MAX = DBL_MAX;
    const size_t SIZE_T_MAX = SIZE_MAX;

    //One bit is set (true) for each attached point, or unset (false) for unattached points:
    BitSet_t attached_points = CreateBitset(n_pts, false);

    //Each point has a current neighbor point in the tree, and a current nearest distance:
    size_t* nearest_mrd_neighbors = CreateAlignedSizeT1D((n_pts-1 + self_edge_capacity));
    double* nearest_mrd_distances = CreateAlignedDouble1D((n_pts-1 + self_edge_capacity));

    for(size_t i = 0; i < n_pts-1; ++i) {
        nearest_mrd_distances[i] = DOUBLE_MAX;
    }

    //The MST is expanded starting with the last point in the data set:
    size_t current_point = n_pts - 1;
    size_t num_attached_points = 1;
    unsigned long long end = stop_tsc(start);

    timing << "Setup," << end << "\n";

    start = start_tsc();
    // Calculate MRD
    double** mrd_matrix = CreateAlignedDouble2D(n_pts, n_pts);

    for(size_t pt_x = 0; pt_x < n_pts; ++pt_x) {
        double* x = distance_matrix[pt_x];
        double cd_x = core_distances[pt_x];
        double* mrd_row = mrd_matrix[pt_x];

        __m256d cd_x_intr = _mm256_set1_pd(cd_x);

        mrd_matrix[pt_x][pt_x] = DBL_MAX;
        size_t pt_y = pt_x + 1;
        size_t offset = (4 - (pt_y % 4)) % 4;
        for(size_t k = 0; k < offset && pt_y < n_pts; ++pt_y, ++k) {
            double d0 = distance_matrix[pt_x][pt_y];
            double cd_y0 = core_distances[pt_y];

            double mrd = d0;
            if(cd_x > mrd) {
                mrd = cd_x;
            }
            if(cd_y0 > mrd) {
                mrd = cd_y0;
            }
            mrd_matrix[pt_x][pt_y] = mrd;
            mrd_matrix[pt_y][pt_x] = mrd;
        }
        for(; pt_y < n_pts - 3; pt_y += 4) {
            __m256d d = _mm256_load_pd(x + pt_y);
            __m256d cd_y = _mm256_load_pd(core_distances + pt_y);

            __m256d mrd = _mm256_max_pd(d, cd_x_intr);
            mrd = _mm256_max_pd(mrd, cd_y);

            _mm256_store_pd(mrd_row + pt_y, mrd);

            mrd_matrix[pt_y][pt_x] = mrd_matrix[pt_x][pt_y] ;
            mrd_matrix[pt_y + 1][pt_x] = mrd_matrix[pt_x][pt_y + 1];
            mrd_matrix[pt_y + 2][pt_x] = mrd_matrix[pt_x][pt_y + 2];
            mrd_matrix[pt_y + 3][pt_x] = mrd_matrix[pt_x][pt_y + 3];
        }
        for(; pt_y < n_pts; ++pt_y) {
            double d0 = distance_matrix[pt_x][pt_y];
            double cd_y0 = core_distances[pt_y];

            double mrd = d0;
            if(cd_x > mrd) {
                mrd = cd_x;
            }
            if(cd_y0 > mrd) {
                mrd = cd_y0;
            }
            mrd_matrix[pt_x][pt_y] = mrd;
            mrd_matrix[pt_y][pt_x] = mrd;
        }
    }
    end = stop_tsc(start);
    timing << "MRD computation," << end << "\n";


    __m256d dbl_max = _mm256_set1_pd(DBL_MAX);
    start = start_tsc();
    for(; num_attached_points < n_pts; ++num_attached_points) {
        // find index of minimal mrd
        double* row = mrd_matrix[current_point];
        __m256i_u min_index = _mm256_setzero_si256();
        __m256d min_value = _mm256_set1_pd(DBL_MAX);

        size_t j = 0;
        __m256i current_point_intr = _mm256_set1_epi64x((uint64_t)current_point);
        for(; j < n_pts-4; j += 4) {
            mrd_matrix[j][current_point] = DBL_MAX;
            mrd_matrix[j+1][current_point] = DBL_MAX;
            mrd_matrix[j+2][current_point] = DBL_MAX;
            mrd_matrix[j+3][current_point] = DBL_MAX;

            __m256d mrd = _mm256_load_pd(row + j);
            __m256d nearest_mrd = _mm256_load_pd(nearest_mrd_distances + j);
            __m256i nearest_neighbors = _mm256_load_si256(((__m256i *)(nearest_mrd_neighbors + j)));
            __m256i possible_indices = _mm256_set_epi64x(j+3, j+2, j+1, j);

            __m256d lt_mask = _mm256_cmp_pd(mrd, nearest_mrd, _CMP_LT_OQ);
            nearest_mrd = _mm256_blendv_pd(nearest_mrd, mrd, lt_mask);
            nearest_neighbors = _mm256_blendv_epi8(nearest_neighbors, current_point_intr, (__m256i)lt_mask);

            _mm256_store_pd(nearest_mrd_distances + j, nearest_mrd);
            _mm256_store_si256((__m256i *)(nearest_mrd_neighbors + j), nearest_neighbors);

            __m256d max_mask = _mm256_cmp_pd(mrd, dbl_max, _CMP_NEQ_OQ);
            __m256d le_mask = _mm256_cmp_pd(nearest_mrd, min_value, _CMP_LE_OQ);
            le_mask = _mm256_and_pd(max_mask, le_mask);

            min_value = _mm256_blendv_pd(min_value, nearest_mrd, le_mask);
            min_index = _mm256_blendv_epi8(min_index, possible_indices, (__m256i)le_mask);
        }

        double min_values[4];
        uint64_t min_indices[4];
        _mm256_store_pd(min_values, min_value);
        _mm256_store_si256((__m256i*)min_indices, min_index);
        for(; j < n_pts-1; ++j) {
            mrd_matrix[j][current_point] = DBL_MAX;
            double mrd3 = row[j];
            if(mrd3 < nearest_mrd_distances[j]) {
                nearest_mrd_distances[j] = mrd3;
                nearest_mrd_neighbors[j] = current_point;
            }
            if(mrd3 != DBL_MAX && nearest_mrd_distances[j] <= min_values[3]) {
                min_indices[3] = j;
                min_values[3] = nearest_mrd_distances[j] ;
            }
        }

        double min_value_final = fmin(fmin(fmin(min_values[0], min_values[1]), min_values[2]), min_values[3]);
        current_point = 0;
        if(min_values[0] == min_value_final) {
            current_point = min_indices[0];
        }
        if(min_values[1] == min_value_final) {
            current_point = min_indices[1] > current_point ? min_indices[1] : current_point;
        }
        if(min_values[2] == min_value_final) {
            current_point = min_indices[2] > current_point ? min_indices[2] : current_point;
        }
        if(min_values[3] == min_value_final) {
            current_point = min_indices[3] > current_point ? min_indices[3] : current_point;
        }
    }
    end = stop_tsc(start);
    timing << "MST Compute," << end << "\n";
    for(size_t i = 0; i < n_pts; ++i) {
        free(mrd_matrix[i]);
    }
    free(mrd_matrix);

    //Create an array for vertices in the tree that each point attached to:
    size_t* other_vertex_indices = (size_t*)calloc(n_pts-1 + self_edge_capacity, sizeof(size_t));
    for(size_t i = 0; i < n_pts; ++i) {
        other_vertex_indices[i] = i;
    }

    //If necessary, attach self edges:
    if(self_edges) {
        for(size_t i = n_pts - 1; i < n_pts*2 - 1; ++i) {
            size_t vertex = i - (n_pts - 1);
            nearest_mrd_neighbors[i] = vertex;
            other_vertex_indices[i] = vertex;
            nearest_mrd_distances[i] = core_distances[vertex];
        }
    }

    start = start_tsc();
    UndirectedGraph_C* result = UDG_Create_Fast(n_pts, nearest_mrd_neighbors, other_vertex_indices, nearest_mrd_distances, n_pts-1 + self_edge_capacity);
    end = stop_tsc(start);
    timing << "Construct graph," << end;
    return result;
}
#endif //__AVX2__


#ifdef __AVX512__
UndirectedGraph_C* ConstructMST_NoBitset_NoCalcDistances_AVX512(const double* data_set,
    const double* core_distances, bool self_edges,
    DistanceCalculator distance_function, size_t n_pts, size_t point_dimension, double** distance_matrix) {

    std::ofstream timing("mst_timings/nobitset_nocalcdistances_avx512.csv");

    unsigned long long start = start_tsc();
    size_t self_edge_capacity = self_edges ? n_pts : 0;
    const double DOUBLE_MAX = DBL_MAX;
    const size_t SIZE_T_MAX = SIZE_MAX;

    //One bit is set (true) for each attached point, or unset (false) for unattached points:
    BitSet_t attached_points = CreateBitset(n_pts, false);

    //Each point has a current neighbor point in the tree, and a current nearest distance:
    size_t* nearest_mrd_neighbors = CreateAlignedSizeT1D((n_pts-1 + self_edge_capacity));
    double* nearest_mrd_distances = CreateAlignedDouble1D((n_pts-1 + self_edge_capacity));

    for(size_t i = 0; i < n_pts-1; ++i) {
        nearest_mrd_distances[i] = DOUBLE_MAX;
    }

    //The MST is expanded starting with the last point in the data set:
    size_t current_point = n_pts - 1;
    size_t num_attached_points = 1;
    unsigned long long end = stop_tsc(start);

    timing << "Setup," << end << "\n";

    start = start_tsc();
    // Calculate MRD
    double** mrd_matrix = CreateAlignedDouble2D(n_pts, n_pts);

    for(size_t pt_x = 0; pt_x < n_pts; ++pt_x) {
        double* x = distance_matrix[pt_x];
        double cd_x = core_distances[pt_x];
        double* mrd_row = mrd_matrix[pt_x];

        __m512d cd_x_intr = _mm512_set1_pd(cd_x);

        mrd_matrix[pt_x][pt_x] = DBL_MAX;
        size_t pt_y = pt_x + 1;
        size_t offset = (8 - (pt_y % 8)) % 8;
        for(size_t k = 0; k < offset && pt_y < n_pts; ++pt_y, ++k) {
            double d0 = distance_matrix[pt_x][pt_y];
            double cd_y0 = core_distances[pt_y];

            double mrd = d0;
            if(cd_x > mrd) {
                mrd = cd_x;
            }
            if(cd_y0 > mrd) {
                mrd = cd_y0;
            }
            mrd_matrix[pt_x][pt_y] = mrd;
            mrd_matrix[pt_y][pt_x] = mrd;
        }
        for(; pt_y < n_pts - 7; pt_y += 8) {
            __m512d d = _mm512_load_pd(x + pt_y);
            __m512d cd_y = _mm512_load_pd(core_distances + pt_y);

            __m512d mrd = _mm512_max_pd(d, cd_x_intr);
            mrd = _mm512_max_pd(mrd, cd_y);

            _mm512_store_pd(mrd_row + pt_y, mrd);

            mrd_matrix[pt_y][pt_x] = mrd_matrix[pt_x][pt_y] ;
            mrd_matrix[pt_y + 1][pt_x] = mrd_matrix[pt_x][pt_y + 1];
            mrd_matrix[pt_y + 2][pt_x] = mrd_matrix[pt_x][pt_y + 2];
            mrd_matrix[pt_y + 3][pt_x] = mrd_matrix[pt_x][pt_y + 3];
        }
        for(; pt_y < n_pts; ++pt_y) {
            double d0 = distance_matrix[pt_x][pt_y];
            double cd_y0 = core_distances[pt_y];

            double mrd = d0;
            if(cd_x > mrd) {
                mrd = cd_x;
            }
            if(cd_y0 > mrd) {
                mrd = cd_y0;
            }
            mrd_matrix[pt_x][pt_y] = mrd;
            mrd_matrix[pt_y][pt_x] = mrd;
        }
    }
    end = stop_tsc(start);
    timing << "MRD computation," << end << "\n";


    __m512d dbl_max = _mm512_set1_pd(DBL_MAX);
    start = start_tsc();
    for(; num_attached_points < n_pts; ++num_attached_points) {
        // find index of minimal mrd
        double* row = mrd_matrix[current_point];
        __m512i_u min_index = _mm512_setzero_si512();
        __m512d min_value = _mm512_set1_pd(DBL_MAX);

        size_t j = 0;
        __m512i current_point_intr = _mm512_set1_epi64((uint64_t)current_point);
        for(; j < n_pts-8; j += 8) {
            mrd_matrix[j][current_point] = DBL_MAX;
            mrd_matrix[j+1][current_point] = DBL_MAX;
            mrd_matrix[j+2][current_point] = DBL_MAX;
            mrd_matrix[j+3][current_point] = DBL_MAX;
            mrd_matrix[j+4][current_point] = DBL_MAX;
            mrd_matrix[j+5][current_point] = DBL_MAX;
            mrd_matrix[j+6][current_point] = DBL_MAX;
            mrd_matrix[j+7][current_point] = DBL_MAX;

            __m512d mrd = _mm512_load_pd(row + j);
            __m512d nearest_mrd = _mm512_load_pd(nearest_mrd_distances + j);
            __m512i nearest_neighbors = _mm512_load_si512(((__m512i *)(nearest_mrd_neighbors + j)));
            __m512i possible_indices = _mm512_set_epi64(j+7, j+6, j+5, j+4, j+3, j+2, j+1, j);

            __mmask8 lt_mask = _mm512_cmplt_pd_mask(mrd, nearest_mrd);
            nearest_mrd = _mm512_mask_blend_pd(lt_mask, nearest_mrd, mrd);
            nearest_neighbors = _mm512_mask_blend_epi64(lt_mask, nearest_neighbors, current_point_intr);

            _mm512_store_pd(nearest_mrd_distances + j, nearest_mrd);
            _mm512_store_si512((__m512i *)(nearest_mrd_neighbors + j), nearest_neighbors);

            __mmask8 max_mask = _mm512_cmpneq_pd_mask(mrd, dbl_max);
            __mmask8 le_mask = _mm512_cmple_pd_mask(nearest_mrd, min_value);
            le_mask = max_mask & le_mask;

            min_value = _mm512_mask_blend_pd(le_mask, min_value, nearest_mrd);
            min_index = _mm512_mask_blend_epi64(le_mask, min_index, possible_indices);
        }

        double min_values[4];
        uint64_t min_indices[4];
        _mm512_store_pd(min_values, min_value);
        _mm512_store_si512((__m512i*)min_indices, min_index);
        for(; j < n_pts-1; ++j) {
            mrd_matrix[j][current_point] = DBL_MAX;
            double mrd3 = row[j];
            if(mrd3 < nearest_mrd_distances[j]) {
                nearest_mrd_distances[j] = mrd3;
                nearest_mrd_neighbors[j] = current_point;
            }
            if(mrd3 != DBL_MAX && nearest_mrd_distances[j] <= min_values[3]) {
                min_indices[3] = j;
                min_values[3] = nearest_mrd_distances[j] ;
            }
        }

        double min_value_final = fmin(fmin(fmin(min_values[0], min_values[1]), min_values[2]), min_values[3]);
        current_point = 0;
        if(min_values[0] == min_value_final) {
            current_point = min_indices[0];
        }
        if(min_values[1] == min_value_final) {
            current_point = min_indices[1] > current_point ? min_indices[1] : current_point;
        }
        if(min_values[2] == min_value_final) {
            current_point = min_indices[2] > current_point ? min_indices[2] : current_point;
        }
        if(min_values[3] == min_value_final) {
            current_point = min_indices[3] > current_point ? min_indices[3] : current_point;
        }
    }
    end = stop_tsc(start);
    timing << "MST Compute," << end << "\n";
    for(size_t i = 0; i < n_pts; ++i) {
        free(mrd_matrix[i]);
    }
    free(mrd_matrix);

    //Create an array for vertices in the tree that each point attached to:
    size_t* other_vertex_indices = (size_t*)calloc(n_pts-1 + self_edge_capacity, sizeof(size_t));
    for(size_t i = 0; i < n_pts; ++i) {
        other_vertex_indices[i] = i;
    }

    //If necessary, attach self edges:
    if(self_edges) {
        for(size_t i = n_pts - 1; i < n_pts*2 - 1; ++i) {
            size_t vertex = i - (n_pts - 1);
            nearest_mrd_neighbors[i] = vertex;
            other_vertex_indices[i] = vertex;
            nearest_mrd_distances[i] = core_distances[vertex];
        }
    }

    start = start_tsc();
    UndirectedGraph_C* result = UDG_Create_Fast(n_pts, nearest_mrd_neighbors, other_vertex_indices, nearest_mrd_distances, n_pts-1 + self_edge_capacity);
    end = stop_tsc(start);
    timing << "Construct graph," << end;
    return result;
}
#endif //__AVX512__
