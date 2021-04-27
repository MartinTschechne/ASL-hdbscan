#include "gtest/gtest.h"
#include <hdbscan/undirected_graph.h>
#include <algorithm>

TEST(undirected_graph, instantiation_getters) {
    size_t num_vertices{4};
    size_t vertices_A[6] = {0, 0, 0, 1, 1, 2};
    size_t vertices_B[6] = {1, 2, 3, 2, 3, 3};
    double edge_weights[6] = {0.5, 1.75, 0.75, 1.25, 1.0, 0.25};
    size_t edge_weights_length{6};
    UndirectedGraph undirected_graph(num_vertices, vertices_A,
        vertices_B, edge_weights, edge_weights_length);

    ASSERT_EQ(undirected_graph.GetNumVertices(), num_vertices);
    ASSERT_EQ(undirected_graph.GetNumEdges(), edge_weights_length);
    ASSERT_EQ(undirected_graph.GetFirstVertexAtIndex(3), 1); 
    ASSERT_EQ(undirected_graph.GetSecondVertexAtIndex(3), 2); 
    ASSERT_EQ(undirected_graph.GetEdgeWeightAtIndex(3), 1.25);  
    ASSERT_EQ(undirected_graph.GetFirstVertexAtIndex(5), 2); 
    ASSERT_EQ(undirected_graph.GetSecondVertexAtIndex(5), 3); 
    ASSERT_EQ(undirected_graph.GetEdgeWeightAtIndex(5), 0.25);  
}

TEST(undirected_graph, QuickSortByEdgeWeight) {
    size_t num_vertices{4};
    size_t vertices_A[6] = {0, 0, 0, 1, 1, 2};
    size_t vertices_B[6] = {1, 2, 3, 2, 3, 3};
    double edge_weights[6] = {0.5, 1.75, 0.75, 1.25, 1.0, 0.25};
    size_t edge_weights_length{6};

    UndirectedGraph undirected_graph(num_vertices, vertices_A,
        vertices_B, edge_weights, edge_weights_length);
    undirected_graph.QuicksortByEdgeWeight();

    std::sort(edge_weights, edge_weights + edge_weights_length);

    for (int i = 0; i < edge_weights_length; i++) {
        ASSERT_EQ(undirected_graph.GetEdgeWeightAtIndex(i), edge_weights[i]);
    }
}
