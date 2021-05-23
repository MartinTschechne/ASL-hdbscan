#include <hdbscan/HDBSCAN_star.h>
#include <math.h>
#include <common/list.h>

void ComputeHierarchyAndClusterTree(
        UndirectedGraph_C* mst, size_t min_cluster_size, bool compact_hierarchy,
        const Vector* const constraints, std::string hierarchy_output_file,
        std::string tree_output_file, const char delimiter,
        double* point_noise_levels, size_t* point_last_clusters,
        std::string visualization_output_file, Vector* result) {

    std::ofstream hierarchy_writer(hierarchy_output_file);
    std::ofstream tree_writer(tree_output_file);

    unsigned long long hierarchy_chars_written = 0;
    size_t line_count = 0; //Indicates the number of lines written into hierarchyFile.

    //The current edge being removed from the MST:
    size_t current_edge_index = UDG_GetNumEdges(mst);
    size_t next_cluster_label = 2;
    bool next_level_significant = true;

    //The previous and current cluster numbers of each point in the data set:
    size_t current_cluster_labels_length = UDG_GetNumVertices(mst);
    size_t* previous_cluster_labels = (size_t*)malloc(current_cluster_labels_length * sizeof(size_t));
    size_t* current_cluster_labels = (size_t*)malloc(current_cluster_labels_length * sizeof(size_t));
    for(size_t i = 0; i < current_cluster_labels_length; ++i) {
        current_cluster_labels[i] = 1;
        previous_cluster_labels[i] = 1;
    }
    //A list of clusters in the cluster tree, with the 0th cluster (noise) null:
    Vector* clusters = result;
    vector_clear(clusters);
    vector_push_back(clusters, nullptr);
    vector_push_back(clusters, (void*)CreateCluster(1, nullptr, SNAN, UDG_GetNumVertices(mst)));

    //Calculate number of constraints satisfied for cluster 1:
    OrderedSet* cluster_one = OS_create();
    OS_insert(cluster_one, 1);
    CalculateNumConstraintsSatisfied(cluster_one, clusters, constraints, current_cluster_labels);

    //Sets for the clusters and vertices that are affected by the edge(s) being removed:
    OrderedSet* affected_cluster_labels = OS_create();
    OrderedSet* affected_vertices = OS_create();

    while(current_edge_index > 0) {
        double current_edge_weight = UDG_GetEdgeWeightAtIndex(mst, current_edge_index - 1);
        Vector* new_clusters = (Vector*)malloc(sizeof(Vector));
        vector_init(new_clusters);

        //Remove all edges tied with the current edge weight, and store relevant clusters and vertices:
        while(current_edge_index > 0 && UDG_GetEdgeWeightAtIndex(mst, current_edge_index - 1) == current_edge_weight) {
            size_t first_vertex = UDG_GetFirstVertexAtIndex(mst, current_edge_index - 1);
            size_t second_vertex = UDG_GetSecondVertexAtIndex(mst, current_edge_index - 1);
            UDG_RemoveVertexFromEdgeList(mst, first_vertex, second_vertex);
            UDG_RemoveVertexFromEdgeList(mst, second_vertex, first_vertex);

            if(current_cluster_labels[first_vertex] == 0) {
                --current_edge_index;
                continue;
            }
            
            OS_insert(affected_vertices, first_vertex);
            OS_insert(affected_vertices, second_vertex);
            OS_insert(affected_cluster_labels, current_cluster_labels[first_vertex]);
            --current_edge_index;
        }

        if(OS_empty(affected_cluster_labels)) {
            continue;
        }

        //Check each cluster affected for a possible split:
        while(!OS_empty(affected_cluster_labels)) {
            size_t examined_cluster_label_it = OS_prev(affected_cluster_labels, OS_end(affected_cluster_labels));
            size_t examined_cluster_label = OS_get(affected_cluster_labels, examined_cluster_label_it);
            OS_erase(affected_cluster_labels, examined_cluster_label); // @Beat is this right?
            OrderedSet* examined_vertices = OS_create();

            //Get all affected vertices that are members of the cluster currently being examined:
            // @Beat is this the correct way to iterate when we erase elements? 
            // In the C++ impl erase returns the next valid iterator
            for(size_t it = OS_begin(affected_vertices); it < OS_end(affected_vertices); ) {
                size_t value = OS_get(affected_vertices, it);
                if(current_cluster_labels[value] == examined_cluster_label) {
                    OS_insert(examined_vertices, value);
                    it = OS_erase(affected_vertices, value);
                }
            }

            OrderedSet* first_child_cluster = nullptr;
            list* unexplored_first_child_cluster_points = nullptr;
            size_t num_child_clusters = 0;

            /*
            * Check if the cluster has split or shrunk by exploring the graph from each affected
            * vertex.  If there are two or more valid child clusters (each has >= minClusterSize
            * points), the cluster has split.
            * Note that firstChildCluster will only be fully explored if there is a cluster
            * split, otherwise, only spurious components are fully explored, in order to label
            * them noise.
            */
           while(!OS_empty(examined_vertices)) {
                OrderedSet* constructing_sub_cluster = OS_create();
                list* unexplored_sub_cluster_points = list_create();
                bool any_edges = false;
                bool incremented_child_count = false;

                size_t root_vertex_index = OS_prev(examined_vertices, OS_end(examined_vertices));
                size_t root_vertex = OS_get(examined_vertices, root_vertex_index);
                OS_insert(constructing_sub_cluster, root_vertex);
                list_push_back(unexplored_sub_cluster_points, root_vertex);
                OS_erase(examined_vertices, root_vertex);

                //Explore this potential child cluster as long as there are unexplored points:
                while(!list_empty(unexplored_sub_cluster_points)) {
                    size_t vertex_to_explore = list_front(unexplored_sub_cluster_points);
                    list_pop_front(unexplored_sub_cluster_points);
                    vector* edge_list = UDG_GetEdgeListForVertex(mst, vertex_to_explore);
                    for(size_t neighbor_it = 0; neighbor_it < edge_list->size; ++neighbor_it) {
                        size_t neighbor = *((size_t*)edge_list->elements[neighbor_it]);
                        any_edges = true;
                        if(!OS_contains(constructing_sub_cluster, neighbor)) {
                            OS_insert(constructing_sub_cluster, neighbor);
                            OS_erase(examined_vertices, neighbor); // Here we want to delete by value 
                            list_push_back(unexplored_sub_cluster_points, neighbor);
                        }
                    }

                    //Check if this potential child cluster is a valid cluster:
                    if(!incremented_child_count && constructing_sub_cluster->size >= min_cluster_size && any_edges) {
                        incremented_child_count = true;
                        num_child_clusters++;

                        //If this is the first valid child cluster, stop exploring it:
                        if(first_child_cluster == nullptr) {
                            first_child_cluster = constructing_sub_cluster;
                            unexplored_first_child_cluster_points = unexplored_sub_cluster_points;
                            break;
                        }
                    }
                }

                //If there could be a split, and this child cluster is valid:
                if((num_child_clusters >= 2) && (constructing_sub_cluster->size >= min_cluster_size) && any_edges) {
                    //Check this child cluster is not equal to the unexplored first child cluster:
                    size_t first_child_cluster_member = OS_get(first_child_cluster, OS_prev(first_child_cluster, OS_end(first_child_cluster)));
                    if(OS_contains(constructing_sub_cluster, first_child_cluster_member)) {
                        num_child_clusters--;
                    } else {
                        //Otherwise, create a new cluster:
                        Cluster* new_cluster = CreateNewCluster(constructing_sub_cluster, current_cluster_labels, (Cluster*)(clusters->elements[examined_cluster_label]), next_cluster_label, current_edge_weight);
                        vector_push_back(new_clusters, (void*)new_cluster);
                        vector_push_back(clusters, (void*)new_cluster);
                        next_cluster_label++;
                    }
                } else if(constructing_sub_cluster->size < min_cluster_size || !any_edges) { //If this child cluster is not valid cluster, assign it to noise:
                    CreateNewCluster(constructing_sub_cluster, current_cluster_labels, (Cluster*)(clusters->elements[examined_cluster_label]), 0, current_edge_weight);

                    for(size_t i = OS_begin(constructing_sub_cluster); i < OS_end(constructing_sub_cluster); i = OS_next(constructing_sub_cluster, i)) {
                        size_t point = OS_get(constructing_sub_cluster, i);
                        point_noise_levels[point] = current_edge_weight;
                        point_last_clusters[point] = examined_cluster_label;
                    }
                }

                if(first_child_cluster != constructing_sub_cluster) {
                    OS_free(constructing_sub_cluster);
                    list_free(unexplored_sub_cluster_points);
                }
           }
           //Finish exploring and cluster the first child cluster if there was a split and it was not already clustered:
           if((num_child_clusters >= 2) && (current_cluster_labels[OS_get(first_child_cluster, OS_begin(first_child_cluster))] == examined_cluster_label)) {
                while(!list_empty(unexplored_first_child_cluster_points)) {
                    size_t vertex_to_explore = list_front(unexplored_first_child_cluster_points);
                    list_pop_front(unexplored_first_child_cluster_points);

                    vector* edge_list = UDG_GetEdgeListForVertex(mst, vertex_to_explore);
                    for(size_t neighbor_it = 0; neighbor_it < edge_list->size; ++neighbor_it) {
                        size_t neighbor = *((size_t*)edge_list->elements[neighbor_it]);
                        if(!OS_contains(first_child_cluster, neighbor)) {
                            OS_insert(first_child_cluster, neighbor);
                            list_push_back(unexplored_first_child_cluster_points, neighbor);
                        }
                    }
                }

                Cluster* new_cluster = CreateNewCluster(first_child_cluster, current_cluster_labels, (Cluster*)(clusters->elements[examined_cluster_label]), next_cluster_label, current_edge_weight);
                vector_push_back(new_clusters, (void*)new_cluster);
                vector_push_back(clusters, (void*)new_cluster);
                next_cluster_label++;
           }

            OS_free(examined_vertices);
            if(first_child_cluster != nullptr) {
                OS_free(first_child_cluster);
            }
            list_free(unexplored_first_child_cluster_points);
        }

        //Write out the current level of the hierarchy:
        if(!compact_hierarchy || next_level_significant || new_clusters->size != 0) {
            std::stringstream output;
            output << current_edge_weight << delimiter;

            for(size_t i = 0; i < current_cluster_labels_length - 1; ++i) {
                output << previous_cluster_labels[i] << delimiter;
            }
            output << previous_cluster_labels[current_cluster_labels_length - 1] << std::endl;

            std::string content = output.str();
            hierarchy_writer << content;

            hierarchy_chars_written += content.size();
            line_count++;
        }

        //Assign file offsets and calculate the number of constraints satisfied:
        OrderedSet* new_cluster_labels = OS_create();
        for(size_t i = 0; i < new_clusters->size; ++i) {
            Cluster* new_cluster = (Cluster*)new_clusters->elements[i];
            new_cluster->file_offset = hierarchy_chars_written;
            OS_insert(new_cluster_labels, new_cluster->label);
        }
        if(!OS_empty(new_cluster_labels)) {
            CalculateNumConstraintsSatisfied(new_cluster_labels, clusters, constraints, current_cluster_labels);
        }

        OS_free(new_cluster_labels);

        for(size_t i = 0; i < current_cluster_labels_length; ++i) { // TODO we can do a pointer swap here to speed things up
            previous_cluster_labels[i] = current_cluster_labels[i];
        }

        next_level_significant = new_clusters->size != 0;

        vector_free(new_clusters);
    }

    //Write out the final level of the hierarchy (all points noise):
    hierarchy_writer << "0" << delimiter;
    for(size_t i = 0; i < current_cluster_labels_length-1; ++i) {
        hierarchy_writer << "0" << delimiter;
    }
    hierarchy_writer << "0\n";
    line_count++;

    //Write out the cluster tree:
    for(size_t i = 0; i < clusters->size; ++i) {
        Cluster* cluster = (Cluster*)clusters->elements[i];
        if(cluster == nullptr) {
            continue;
        }

        tree_writer << cluster->label << delimiter;
        tree_writer << cluster->birth_level << delimiter;
        tree_writer << cluster->death_level << delimiter;
        tree_writer << cluster->stability << delimiter;

        if(!vector_empty(constraints)) {
            tree_writer << 0.5*cluster->num_constraints_satisfied / constraints->size << delimiter;
            tree_writer << 0.5 * cluster->propagated_num_constraints_satisfied / constraints->size << delimiter;
        } else {
            tree_writer << 0 << delimiter << 0 << delimiter;
        }

        tree_writer << cluster->file_offset << delimiter;
        if(cluster->parent != nullptr) {
            tree_writer << cluster->parent->label << "\n";
        } else {
            tree_writer << "0\n";
        }
    }

    std::fstream visualization_writer(visualization_output_file);
    if(!compact_hierarchy) {
        visualization_writer << "1\n";
    } else {
        visualization_writer << "0\n";
    }
    visualization_writer << line_count;

    OS_free(affected_cluster_labels);
    OS_free(affected_vertices);
    free(previous_cluster_labels);
    free(current_cluster_labels);
}

void CalculateNumConstraintsSatisfied(
        const OrderedSet* const new_cluster_labels, const Vector* const clusters, const Vector* const constraints, size_t* cluster_labels) {

    if(vector_empty(constraints)) {
        return;
    }

    Vector* parents = (Vector*)malloc(sizeof(Vector));
    vector_init(parents);

    for(size_t i = OS_begin(new_cluster_labels); i < OS_end(new_cluster_labels); i = OS_next(new_cluster_labels, i)) {
        size_t label = OS_get(new_cluster_labels, i);
        Cluster* parent = ((Cluster*)vector_get(clusters, label))->parent;
        if(parent != nullptr && !vector_contains(parents, parent)) {
            vector_push_back(parents, (void*)parent);
        }
    }

    for(size_t i = 0; i < constraints->size; ++i) {
        const Constraint& constraint = *((Constraint*)constraints->elements[i]);
        size_t label_a = cluster_labels[constraint.point_a];
        size_t label_b = cluster_labels[constraint.point_b];

        if(constraint.type == Constraint::CONSTRAINT_TYPE::MUST_LINK && label_a == label_b) {
            if(OS_contains(new_cluster_labels, label_a)) {
                AddConstraintsSatisfied((Cluster*)vector_get(clusters, label_a), 2);
            }
        } else if(constraint.type == Constraint::CONSTRAINT_TYPE::CANNOT_LINK && (label_a != label_b || label_a == 0)) {
            if(label_a != 0 && OS_contains(new_cluster_labels, label_a)) {
                AddConstraintsSatisfied((Cluster*)vector_get(clusters, label_a), 1);
            }
            if(label_b != 0 && OS_contains(new_cluster_labels, label_b)) {
                AddConstraintsSatisfied((Cluster*)vector_get(clusters, label_b), 1);
            }

            if(label_a == 0) {
                for(size_t i = 0; i < parents->size; ++i) {
                    Cluster* parent = (Cluster*)parents->elements[i];
                    if(VirtualChildClusterContaintsPoint(parent, constraint.point_a)) {
                        AddVirtualChildConstraintsSatisfied(parent, 1);
                        break;
                    }
                }
            }
            if(label_b == 0) {
                for(size_t i = 0; i < parents->size; ++i) {
                    Cluster* parent = (Cluster*)parents->elements[i];
                    if(VirtualChildClusterContaintsPoint(parent, constraint.point_b)) {
                        AddVirtualChildConstraintsSatisfied(parent, 1);
                        break;
                    }
                }
            }
        }
    }
}

Cluster* CreateNewCluster(const OrderedSet* const points,
        size_t* cluster_labels, Cluster* parent_cluster, size_t cluster_label,
        double edge_weight) {
    for(size_t i = OS_begin(points); i < OS_end(points); i = OS_next(points, i)) {
        cluster_labels[OS_get(points, i)] = cluster_label;
    }
    DetachPoints(parent_cluster, points->size, edge_weight);

    if(cluster_label != 0) {
        return CreateCluster(cluster_label, parent_cluster, edge_weight, points->size);
    }

    AddPointsToVirtualChildCluster(parent_cluster, points);
    return nullptr;
}
