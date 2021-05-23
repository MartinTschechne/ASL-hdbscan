#include "gtest/gtest.h"
#include <hdbscan/_C_undirected_graph.h>
#include <algorithm>

TEST(undirected_graph_C, instantiation_getters) {
    size_t num_vertices{4};
    size_t edge_weights_length{6};
    size_t vertices_A[6] = {0, 0, 0, 1, 1, 2};
    size_t vertices_B[6] = {1, 2, 3, 2, 3, 3};
    double edge_weights[6] = {0.5, 1.75, 0.75, 1.25, 1.0, 0.25};
    
    UndirectedGraph_C* udg = UDG_Create(num_vertices, vertices_A,
        vertices_B, edge_weights, edge_weights_length);

    ASSERT_EQ(UDG_GetNumVertices(udg), num_vertices);
    ASSERT_EQ(UDG_GetNumEdges(udg), edge_weights_length);
    ASSERT_EQ(UDG_GetFirstVertexAtIndex(udg, 3), 1); 
    ASSERT_EQ(UDG_GetSecondVertexAtIndex(udg, 3), 2); 
    ASSERT_EQ(UDG_GetEdgeWeightAtIndex(udg, 3), 1.25);  
    ASSERT_EQ(UDG_GetFirstVertexAtIndex(udg, 5), 2); 
    ASSERT_EQ(UDG_GetSecondVertexAtIndex(udg, 5), 3); 
    ASSERT_EQ(UDG_GetEdgeWeightAtIndex(udg, 5), 0.25);  

    UDG_Free(udg);
}

TEST(undirected_graph_C, QuickSortByEdgeWeight) {
    size_t num_vertices{4};
    size_t edge_weights_length{6};
    size_t vertices_A[6] = {0, 0, 0, 1, 1, 2};
    size_t vertices_B[6] = {1, 2, 3, 2, 3, 3};
    double edge_weights[6] = {0.5, 1.75, 0.75, 1.25, 1.0, 0.25};

    UndirectedGraph_C* udg = UDG_Create(num_vertices, vertices_A,
        vertices_B, edge_weights, edge_weights_length);

    UDG_QuicksortByEdgeWeight(udg);

    std::sort(edge_weights, edge_weights + edge_weights_length);

    for (int i = 0; i < edge_weights_length; i++) {
        ASSERT_EQ(UDG_GetEdgeWeightAtIndex(udg, i), edge_weights[i]);
    }
    UDG_Free(udg);
}

TEST(undirected_graph_C, UDG_RemoveVertexFromEdgeList) {
    size_t num_vertices{4};
    size_t edge_weights_length{6};
    size_t vertices_A[6] = {0, 0, 0, 1, 1, 2};
    size_t vertices_B[6] = {1, 2, 3, 2, 3, 3};
    double edge_weights[6] = {0.5, 1.75, 0.75, 1.25, 1.0, 0.25};

    UndirectedGraph_C* udg = UDG_Create(num_vertices, vertices_A,
        vertices_B, edge_weights, edge_weights_length);

    vector* edgelist = UDG_GetEdgeListForVertex(udg, 0);

    vector* edge_list_1 = UDG_GetEdgeListForVertex(udg, 1);
    ASSERT_EQ(edge_list_1->size, 3);

    UDG_RemoveVertexFromEdgeList(udg, 1, 2);
    UDG_RemoveVertexFromEdgeList(udg, 1, 0);
    UDG_RemoveVertexFromEdgeList(udg, 1, 1);
    UDG_RemoveVertexFromEdgeList(udg, 1, 3);

    edge_list_1 = UDG_GetEdgeListForVertex(udg, 1);
    ASSERT_EQ(edge_list_1->size, 0);
    UDG_Free(udg);
}
