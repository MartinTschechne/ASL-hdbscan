#include <hdbscan/_C_undirected_graph.h>
#include <cstdlib>
#include <cstdio>

UndirectedGraph_C* UDG_Create(size_t num_vertices, size_t* vertices_A,
    size_t* vertices_B, double* edge_weights, size_t edge_weights_length) {
    UndirectedGraph_C* udg = (UndirectedGraph_C*)malloc(sizeof(*udg));
    udg->num_vertices_ = num_vertices;
    udg->edge_weights_length_ = edge_weights_length;
    udg->vertices_A_ = (size_t*)malloc(
        udg->edge_weights_length_ * sizeof(*udg->vertices_A_));
    udg->vertices_B_ = (size_t*)malloc(
        udg->edge_weights_length_ * sizeof(*udg->vertices_B_));
    udg->edge_weights_ = (double*)malloc(
        udg->edge_weights_length_ * sizeof(*udg->edge_weights_));
    udg->edges_ = (vector*)malloc(sizeof(*udg->edges_));
    vector_init(udg->edges_);
    for(size_t i = 0; i < num_vertices; ++i) {
       vector_push_back(udg->edges_, (void*)vector_create());
    }

    for (int i = 0; i < udg->num_vertices_; i++) {
        vector* vertex_adjacents = vector_create();
        vector_push_back(udg->edges_, vertex_adjacents);
    }

    for (int i = 0; i < udg->edge_weights_length_; i++) {
        udg->edge_weights_[i] = edge_weights[i];
        udg->vertices_A_[i] = vertices_A[i];
        udg->vertices_B_[i] = vertices_B[i];

        size_t* vertex_one = (size_t*)malloc(sizeof(size_t));
        *vertex_one = vertices_A[i];
        size_t* vertex_two = (size_t*)malloc(sizeof(size_t));
        *vertex_two = vertices_B[i];

        vector_push_back((vector*)udg->edges_->elements[*vertex_one], (void*)vertex_two);
        if(*vertex_one != *vertex_two) {
            vector_push_back((vector*)udg->edges_->elements[*vertex_two], (void*)vertex_one);
        }
    }
    return udg;
}

UndirectedGraph_C* UDG_Create_Fast(size_t num_vertices, size_t* vertices_A,
    size_t* vertices_B, double* edge_weights, size_t edge_weights_length) {
    UndirectedGraph_C* udg = (UndirectedGraph_C*)malloc(sizeof(*udg));
    udg->num_vertices_ = num_vertices;
    udg->edge_weights_length_ = edge_weights_length;
    udg->vertices_A_ = vertices_A;
    udg->vertices_B_ = vertices_B;
    udg->edge_weights_ = edge_weights;
    udg->edges_ = vector_create(num_vertices);
    for(size_t i = 0; i < num_vertices; ++i) {
       vector_push_back(udg->edges_, (void*)vector_create(10));
    }

    for (int i = 0; i < udg->edge_weights_length_; i++) {
        size_t* vertex_one = (size_t*)malloc(sizeof(size_t));
        *vertex_one = vertices_A[i];
        size_t* vertex_two = (size_t*)malloc(sizeof(size_t));
        *vertex_two = vertices_B[i];

        vector_push_back((vector*)udg->edges_->elements[*vertex_one], (void*)vertex_two);
        if(*vertex_one != *vertex_two) {
            vector_push_back((vector*)udg->edges_->elements[*vertex_two], (void*)vertex_one);
        }
    }
    return udg;
}

void UDG_Free(UndirectedGraph_C* udg) {
    free(udg->vertices_A_);
    free(udg->vertices_B_);
    free(udg->edge_weights_);
    free(udg->edges_);
    free(udg);
}

void UDG_QuicksortByEdgeWeight(UndirectedGraph_C* udg) {
    if (udg->edge_weights_length_ <= 1) {
        return;
    }

    long int*  start_index_stack = (long int*)malloc(
        (size_t)(udg->edge_weights_length_ / 2) * sizeof(*start_index_stack));
    long int* end_index_stack = (long int*)malloc(
        (size_t)(udg->edge_weights_length_ / 2) * sizeof(*end_index_stack));

    start_index_stack[0] = 0;
    end_index_stack[0] = (long int)udg->edge_weights_length_ - 1;
    long int stack_top = 0;

    while (stack_top >= 0) {
        long int start_index = start_index_stack[stack_top];
        long int end_index = end_index_stack[stack_top];
        stack_top--;

        long int pivot_index = UDG_SelectPivotIndex(
            udg, start_index, end_index);
        pivot_index = UDG_Partition(
            udg, start_index, end_index, pivot_index);

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
    free(start_index_stack);
    free(end_index_stack);
    return;
}

static void UDG_Quicksort(
    UndirectedGraph_C* udg, long int start_index, long int end_index) {
    if (start_index < end_index) {
        long int pivot_index = UDG_SelectPivotIndex(
            udg, start_index, end_index);
        pivot_index = UDG_Partition(
            udg, start_index, end_index, pivot_index);
        UDG_Quicksort(udg, start_index, pivot_index - 1);
        UDG_Quicksort(udg, pivot_index + 1, end_index);
    }
}

static long int UDG_SelectPivotIndex(
    const UndirectedGraph_C* udg, long int start_index, long int end_index) {
    if (start_index - end_index <= 1) {
        return start_index;
    }
    long int middle_index = (long int)(
        start_index + (end_index - start_index) / 2);
    double first = udg->edge_weights_[start_index];
    double middle = udg->edge_weights_[middle_index];
    double last = udg->edge_weights_[end_index];

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

static long int UDG_Partition(
    UndirectedGraph_C* udg, long int start_index, long int end_index,
    long int pivot_index) {
    double pivot_value = udg->edge_weights_[pivot_index];
    UDG_SwapEdges(udg, pivot_index, end_index);
    long int low_index = start_index;

    for (long int i = start_index; i < end_index; i++) {
        if (udg->edge_weights_[i] < pivot_value) {
            UDG_SwapEdges(udg, i, low_index);
            low_index++;
        }
    }

    UDG_SwapEdges(udg, low_index, end_index);
    return low_index;
}

static void UDG_SwapEdges(
    UndirectedGraph_C* udg, long int index_one, long int index_two) {
    if (index_one == index_two)
        return;

    size_t temp_vertex_A = udg->vertices_A_[index_one];
    size_t temp_vertex_B = udg->vertices_B_[index_one];
    double temp_edge_distance = udg->edge_weights_[index_one];

    udg->vertices_A_[index_one] = udg->vertices_A_[index_two];
    udg->vertices_B_[index_one] = udg->vertices_B_[index_two];
    udg->edge_weights_[index_one] = udg->edge_weights_[index_two];

    udg->vertices_A_[index_two] = temp_vertex_A;
    udg->vertices_B_[index_two] = temp_vertex_B;
    udg->edge_weights_[index_two] = temp_edge_distance;
}

size_t UDG_GetNumVertices(const UndirectedGraph_C* udg) {
    return udg->num_vertices_;
}

size_t UDG_GetNumEdges(const UndirectedGraph_C* udg) {
    return udg->edge_weights_length_;
}

size_t UDG_GetFirstVertexAtIndex(const UndirectedGraph_C* udg, size_t index) {
    return udg->vertices_A_[index];
}

size_t UDG_GetSecondVertexAtIndex(const UndirectedGraph_C* udg, size_t index) {
    return udg->vertices_B_[index];
}

double UDG_GetEdgeWeightAtIndex(const UndirectedGraph_C* udg, size_t index) {
    return udg->edge_weights_[index];
}

vector* UDG_GetEdgeListForVertex(const UndirectedGraph_C* udg, size_t vertex) {
    return (vector*)vector_get(udg->edges_, vertex);
}

void UDG_RemoveVertexFromEdgeList(
    UndirectedGraph_C* udg, size_t vertex_row, size_t vertex_to_erase) {
    vector* vertex_adjacents = UDG_GetEdgeListForVertex(udg, vertex_row);
    bool found = false;
    long int length = (long int)vector_size(vertex_adjacents);
    long int i = 0;
    for (; i < length; i++) {
        if (*(size_t*)vector_get(vertex_adjacents, i) == vertex_to_erase) {
            found = true;
            break;
        }
    }
    if (found) {
        vector_erase(vertex_adjacents, i);
    }
}
