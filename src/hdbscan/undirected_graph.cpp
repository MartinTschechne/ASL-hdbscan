#include <hdbscan/undirected_graph.h>
#include <algorithm>
#include <iostream>

UndirectedGraph::UndirectedGraph(size_t num_vertices, 
    size_t* vertices_A, size_t* vertices_B, double* edge_weights,
    size_t edge_weights_length) :
    num_vertices_{num_vertices},
    edge_weights_length_{edge_weights_length} {
        vertices_A_ = new size_t[edge_weights_length_];
        vertices_B_ = new size_t[edge_weights_length_];
        edge_weights_ = new double[edge_weights_length_];
        edges_.resize(num_vertices);

        for (int i = 0; i < num_vertices_; i++) {
            edges_[i].reserve(1 + edge_weights_length_/num_vertices);
        }


        for (int i = 0; i < edge_weights_length_; i++) {
            edge_weights_[i] = edge_weights[i];
            vertices_A_[i] = vertices_A[i];
            vertices_B_[i] = vertices_B[i];
            size_t vertex_one = vertices_A_[i];
            size_t vertex_two = vertices_B_[i];
            edges_[vertex_one].push_back(vertex_two);
            if (vertex_one != vertex_two) {
                edges_[vertex_two].push_back(vertex_one);
            }     
        }
    }

UndirectedGraph::~UndirectedGraph() {
    delete[] vertices_A_;
    delete[] vertices_B_;
    delete[] edge_weights_;
}

void UndirectedGraph::QuicksortByEdgeWeight() {
    if (edge_weights_length_ <= 1) {
        return;
    }

    long int* start_index_stack = new long int[edge_weights_length_ / 2];
    long int* end_index_stack = new long int[edge_weights_length_ / 2];

    start_index_stack[0] = 0;
    end_index_stack[0] = edge_weights_length_ - 1;
    long int stack_top = 0;

    while (stack_top >= 0) {
        size_t start_index = start_index_stack[stack_top];
        size_t end_index = end_index_stack[stack_top];
        stack_top--;

        size_t pivot_index = this->SelectPivotIndex(start_index, end_index);
        pivot_index = this->Partition(start_index, end_index, pivot_index);

        if (pivot_index > start_index + 1) {
            start_index_stack[stack_top + 1] = start_index;
            end_index_stack[stack_top + 1] = pivot_index - 1;
            stack_top++;
        }
        if (pivot_index < end_index - 1) {
            start_index_stack[stack_top + 1] = pivot_index + 1;
            end_index_stack[stack_top + 1] = end_index;
            stack_top++;
        }
    }
    delete[] start_index_stack;
    delete[] end_index_stack;
    return;
}

void UndirectedGraph::Quicksort(size_t start_index, size_t end_index) {
    if (start_index < end_index) {
        size_t pivot_index = this->SelectPivotIndex(start_index, end_index);
        pivot_index = this->Partition(start_index, end_index, pivot_index);
        this->Quicksort(start_index, pivot_index - 1);
        this->Quicksort(pivot_index + 1, end_index);
    }
}

size_t UndirectedGraph::SelectPivotIndex(size_t start_index, size_t end_index) const {
    if (start_index - end_index <= 1) {
        return start_index;
    }

    double first = edge_weights_[start_index];
    double middle = edge_weights_[start_index + (end_index - start_index) / 2];
    double last = edge_weights_[end_index];

    if (first <= middle) {
        if (middle <= last)
            return start_index + (end_index - start_index) / 2;
        else if (last >= first)
            return end_index;
        else 
            return start_index;
    }
    else {
        if (first <= last)
            return start_index;
        else if (last >= middle)
            return end_index;
        else
            return start_index + (end_index - start_index) / 2;
    }
}

size_t UndirectedGraph::Partition(size_t start_index, size_t end_index, size_t pivot_index) {
    double pivot_value = edge_weights_[pivot_index];
    this->SwapEdges(pivot_index, end_index);
    size_t low_index = start_index;

    for (int i = start_index; i < end_index; i++) {
        if (edge_weights_[i] < pivot_value) {
            this->SwapEdges(i, low_index);
            low_index++;
        }
    }

    this->SwapEdges(low_index, end_index);
    return low_index;
}

void UndirectedGraph::SwapEdges(size_t index_one, size_t index_two) {
    if (index_one == index_two)
        return;
    
    size_t temp_vertex_A = vertices_A_[index_one];
    size_t temp_vertex_B = vertices_B_[index_one];
    double temp_edge_distance = edge_weights_[index_one];

    vertices_A_[index_one] = vertices_A_[index_two];
    vertices_B_[index_one] = vertices_B_[index_two];
    edge_weights_[index_one] = edge_weights_[index_two];

    vertices_A_[index_two] = temp_vertex_A;
    vertices_B_[index_two] = temp_vertex_B;
    edge_weights_[index_two] = temp_edge_distance;
}

size_t UndirectedGraph::GetNumVertices() const {
    return num_vertices_;
}

size_t UndirectedGraph::GetNumEdges() const {
    return edge_weights_length_;
}

size_t UndirectedGraph::GetFirstVertexAtIndex(size_t index) const {
    return vertices_A_[index];
}

size_t UndirectedGraph::GetSecondVertexAtIndex(size_t index) const {
    return vertices_B_[index];
}

double UndirectedGraph::GetEdgeWeightAtIndex(size_t index) const {
    return edge_weights_[index];
}

const std::vector<size_t>& UndirectedGraph::GetEdgeListForVertex(size_t vertex) const {
    return edges_[vertex];
}

void UndirectedGraph::RemoveVertexFromEdgeList(size_t vertex_row, size_t vertex_to_erase) {
    auto it = std::find(edges_[vertex_row].begin(), edges_[vertex_row].end(), vertex_to_erase);
    if(it == edges_[vertex_row].end()) {
        return;
    }
    edges_[vertex_row].erase(it);
}
