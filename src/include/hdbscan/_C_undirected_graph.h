#ifndef TEAM33_HDBSCAN_UNDIRECTED_GRAPH_C_H
#define TEAM33_HDBSCAN_UNDIRECTED_GRAPH_C_H

#include <cstddef>
#include <common/vector.h>


/**
 * @brief An undirected graph, with weights assigned to each edge. Vertices in
 * the graph are 0 indexed.
 */
typedef struct alignas(32) UndirectedGraph_C {
    size_t num_vertices_;
    size_t* vertices_A_;
    size_t* vertices_B_;
    double* edge_weights_;
    size_t edge_weights_length_;
    vector* edges_;
} UndirectedGraph_C;

/**
 * @brief Constructs a new UndirectedGraph, including creating an edge list
 * for each vertex from the vertex arrays.  For an index i, vertices_A[i]
 * and vertices_B[i] share an edge with weight edge_weights[i].
 *
 * @param num_vertices The number of vertices in the graph (indexed 0 to
 * num_vertices-1)
 * @param vertices_A A vector of vertices corresponding to the array of
 * edges
 * @param vertices_B A vector of vertices corresponding to the array of
 * edges
 * @param edge_weights A vector of edges corresponding to the arrays of
 * vertices
 * @param edge_weights_length The number of edges contained in edge_weights
 */
UndirectedGraph_C* UDG_Create(size_t num_vertices, size_t* vertices_A,
    size_t* vertices_B, double* edge_weights, size_t edge_weights_length);

/**
 * @brief Destructs UndirectedGraph
 *
 * @param udg Undirected graph to destruct
 */
void UDG_Free(UndirectedGraph_C* udg);

/**
 * @brief Quicksorts the graph by edge weight in descending order.
 * This quicksort implementation is iterative and in-place.
 *
 * @param udg Undirected graph to quicksort
 */
void UDG_QuicksortByEdgeWeight(UndirectedGraph_C* udg);

/**
 * @brief Get number of vertices of an undirected graph
 *
 * @param udg Undirected graph to get number of vertices of
 * @return size_t  Number of vertices
 */
size_t UDG_GetNumVertices(const UndirectedGraph_C* udg);

/**
 * @brief Get number of edges of an undirected graph
 *
 * @param udg Undirected graph to get number of edges of
 * @return size_t Number of edges
 */
size_t UDG_GetNumEdges(const UndirectedGraph_C* udg);

/**
 * @brief Get first vertex at given index
 *
 * @param udg Undirected graph to get vertex from
 * @param index The index to get the first vertex from
 * @return int
 */
size_t UDG_GetFirstVertexAtIndex(const UndirectedGraph_C* udg, size_t index);

/**
 * @brief Get second vertex at given index
 *
 * @param udg Undirected graph to get vertex from
 * @param index The index to get the second vertex from
 * @return size_t Index of second vertex
 */
size_t UDG_GetSecondVertexAtIndex(const UndirectedGraph_C* udg, size_t index);

/**
 * @brief Get edge weight at given index
 *
 * @param udg Undirected graph to get edge weight from
 * @param index The index to get the edge weight from
 * @return double
 */
double UDG_GetEdgeWeightAtIndex(const UndirectedGraph_C* udg, size_t index);

/**
 * @brief Gets edge list of a vertex
 *
 * @param udg Undirected graph to get edge list from
 * @param vertex Vertex to get edge list of
 * @return vector* The edge list of the vertex
 */
vector* UDG_GetEdgeListForVertex(const UndirectedGraph_C* udg, size_t vertex);

/**
 * @brief Removes vertex from edge list
 *
 * @param udg Undirected to remove vertext from
 * @param vertex_row Vertex adjacents to erase vertex from
 * @param vertex_to_erase Vertex to erase
 */
void UDG_RemoveVertexFromEdgeList(
    UndirectedGraph_C* udg, size_t vertex_row, size_t vertex_to_erase);

// private:
/**
 * @brief Quicksorts the graph in the interval [startIndex, endIndex] by
 * edge weight.
 *
 * @param udg Undirected graph to quicksort
 * @param startIndex The lowest index to be included in the sort
 * @param endIndex The highest index to be included in the sort
 */
static void UDG_Quicksort(
    UndirectedGraph_C* udg, long int start_index, long int end_index);

/**
 * @brief Returns a pivot index by finding the median of edge weights
 * between the startIndex, endIndex, and middle.
 *
 * @param udg Undirected graph whose edge weights to find pivot in
 * @param start_index The lowest index from which the pivot index should come
 * @param end_index The highest index from which the pivot index should come
 * @return A pivot index
 */
static long int UDG_SelectPivotIndex(
    const UndirectedGraph_C* udg, long int start_index, long int end_index);

/**
 * @brief Partitions the array in the interval [start_index, end_index]
 * around the value at pivotIndex.
 *
 * @param udg Undirected graph whose edge weights to partition
 * @param start_index The lowest index to  partition
 * @param end_index The highest index to partition
 * @param pivot_index The index of the edge weight to partition around
 * @return The index position of the pivot edge weight after the partition
 */
static long int UDG_Partition(
    UndirectedGraph_C* udg, long int start_index, long int end_index,
    long int pivot_index);

/**
 * @brief Swaps the vertices and edge weights between two index locations
 * in the graph.
 *
 * @param udg Undirected graph whose edges to swap
 * @param index_two The second index location
 */
static void UDG_SwapEdges(
    UndirectedGraph_C* udg, long int index_one, long int index_two);

#endif
