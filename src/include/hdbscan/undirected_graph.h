#ifndef TEAM33_HDBSCAN_UNDIRECTED_GRAPH_H
#define TEAM33_HDBSCAN_UNDIRECTED_GRAPH_H

#include <cstddef>
#include <memory>

/**
 * @brief An undirected graph, with weights assigned to each edge. Vertices in
 * the graph are 0 indexed.
 */
class UndirectedGraph {
private:
    size_t num_vertices_;
    size_t* vertices_A_;
    size_t* vertices_B_;
    double* edge_weights_;
    size_t** edges_;
    // std::unique_ptr<size_t[]> vertices_A_;
    // std::unique_ptr<size_t[]> vertices_B_;
    // std::unique_ptr<double[]> edge_weights_;
    // std::unique_ptr<std::unique_ptr<size_t[]>[]> edges_;
    size_t edge_weights_length_;
    
public:
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
    UndirectedGraph(size_t num_vertices, size_t* vertices_A, size_t* vertices_B, double* edge_weights, size_t edge_weights_length);

    /**
     * @brief Destructs UndirectedGraph
     */
    ~UndirectedGraph();

    /**
	 * @brief Quicksorts the graph by edge weight in descending order.  This 
     * quicksort
     * implementation is iterative and in-place.
	 */
    void QuicksortByEdgeWeight();

    /**
     * @brief Get number of vertices
     * 
     * @return size_t  
     */
    size_t GetNumVertices() const;

    /**
     * @brief Get number of edges
     * 
     * @return int
     */
    size_t GetNumEdges() const;

    /**
     * @brief Get first vertex at given index
     * 
     * @param index The index to get the first vertex from
     * @return int
     */
    size_t GetFirstVertexAtIndex(size_t index) const;

    /**
     * @brief Get second vertex at given index
     * 
     * @param index The index to get the second vertex from
     * @return int
     */
    size_t GetSecondVertexAtIndex(size_t index) const;

    /**
     * @brief Get edge weight at given index
     * 
     * @param index The index to get the edge weight from 
     * @return double
     */
    double GetEdgeWeightAtIndex(size_t index) const;

private:
    /**
	 * @brief Quicksorts the graph in the interval [startIndex, endIndex] by 
     * edge weight.
     * 
	 * @param startIndex The lowest index to be included in the sort
	 * @param endIndex The highest index to be included in the sort
	 */
	void Quicksort(size_t start_index, size_t end_index);

    /**
	 * @brief Returns a pivot index by finding the median of edge weights 
     * between the startIndex, endIndex, and middle.
     * 
	 * @param start_index The lowest index from which the pivot index should come
	 * @param end_index The highest index from which the pivot index should come
	 * @return A pivot index
	 */
	size_t SelectPivotIndex(size_t start_index, size_t end_index) const;

    /**
	 * @brief Partitions the array in the interval [start_index, end_index] 
     * around the value at pivotIndex.
     * 
	 * @param start_index The lowest index to  partition
	 * @param end_index The highest index to partition
	 * @param pivot_index The index of the edge weight to partition around
	 * @return The index position of the pivot edge weight after the partition
	 */
	size_t Partition(size_t start_index, size_t end_index, size_t pivot_index);

    /**
	 * @brief Swaps the vertices and edge weights between two index locations 
     * in the graph.
     * 
	 * @param index_one The first index location
	 * @param index_two The second index location
	 */
	void SwapEdges(size_t index_one, size_t index_two);
};

#endif
