#include <hdbscan/HDBSCAN_star.h>

void ComputeHierarchyAndClusterTree(
        UndirectedGraph& mst, size_t min_cluster_size, bool compact_hierarchy,
        std::vector<Constraint> constraints, std::string hierarchy_output_file,
        std::string tree_output_file, const char delimiter,
        double* point_noise_levels, size_t* point_last_clusters,
        std::string visualization_output_file, std::vector<Cluster*>& result) {

    std::ofstream hierarchy_writer(hierarchy_output_file);
    std::ofstream tree_writer(tree_output_file);

    unsigned long long hierarchy_chars_written = 0;
    size_t line_count = 0; //Indicates the number of lines written into hierarchyFile.

    //The current edge being removed from the MST:
    size_t current_edge_index = mst.GetNumEdges();
    size_t next_cluster_label = 2;
    bool next_level_significant = true;

    //The previous and current cluster numbers of each point in the data set:
    size_t current_cluster_labels_length = mst.GetNumVertices();
    size_t* previous_cluster_labels = new size_t[current_cluster_labels_length];
    size_t* current_cluster_labels = new size_t[current_cluster_labels_length];
    for(size_t i = 0; i < current_cluster_labels_length; ++i) {
        current_cluster_labels[i] = 1;
        previous_cluster_labels[i] = 1;
    }
    //A list of clusters in the cluster tree, with the 0th cluster (noise) null:
    std::vector<Cluster*>& clusters = result;
    clusters.clear();
    clusters.push_back(nullptr);
    clusters.push_back(new Cluster(1, nullptr, std::numeric_limits<double>::quiet_NaN(), mst.GetNumVertices()));

    //Calculate number of constraints satisfied for cluster 1:
    std::set<size_t> cluster_one;
    cluster_one.insert(1);
    CalculateNumConstraintsSatisfied(cluster_one, clusters, constraints, current_cluster_labels);

    //Sets for the clusters and vertices that are affected by the edge(s) being removed:
    std::set<size_t> affected_cluster_labels;
    std::set<size_t> affected_vertices;

    while(current_edge_index > 0) {
        double current_edge_weight = mst.GetEdgeWeightAtIndex(current_edge_index - 1);
        std::vector<Cluster*> new_clusters;

        //Remove all edges tied with the current edge weight, and store relevant clusters and vertices:
        while(current_edge_index > 0 && mst.GetEdgeWeightAtIndex(current_edge_index - 1) == current_edge_weight) {
            size_t first_vertex = mst.GetFirstVertexAtIndex(current_edge_index - 1);
            size_t second_vertex = mst.GetSecondVertexAtIndex(current_edge_index - 1);
            mst.RemoveVertexFromEdgeList(first_vertex, second_vertex);
            mst.RemoveVertexFromEdgeList(second_vertex, first_vertex);

            if(current_cluster_labels[first_vertex] == 0) {
                --current_edge_index;
                continue;
            }

            affected_vertices.insert(first_vertex);
            affected_vertices.insert(second_vertex);
            affected_cluster_labels.insert(current_cluster_labels[first_vertex]);
            --current_edge_index;
        }

        if(affected_cluster_labels.empty()) {
            continue;
        }

        //Check each cluster affected for a possible split:
        while(!affected_cluster_labels.empty()) {
            std::set<size_t>::iterator examined_cluster_label_it = --affected_cluster_labels.end();
            size_t examined_cluster_label = *examined_cluster_label_it;
            affected_cluster_labels.erase(examined_cluster_label_it);
            std::set<size_t> examined_vertices;

            //Get all affected vertices that are members of the cluster currently being examined:
            for(std::set<size_t>::iterator it = affected_vertices.begin(); it != affected_vertices.end(); ) {
                if(current_cluster_labels[*it] == examined_cluster_label) {
                    examined_vertices.insert(*it);
                    it = affected_vertices.erase(it);
                    continue;
                }
                ++it;
            }

            std::set<size_t>* first_child_cluster = nullptr;
            std::list<size_t>* unexplored_first_child_cluster_points = nullptr;
            size_t num_child_clusters = 0;

            /*
            * Check if the cluster has split or shrunk by exploring the graph from each affected
            * vertex.  If there are two or more valid child clusters (each has >= minClusterSize
            * points), the cluster has split.
            * Note that firstChildCluster will only be fully explored if there is a cluster
            * split, otherwise, only spurious components are fully explored, in order to label
            * them noise.
            */
           while(!examined_vertices.empty()) {
                std::set<size_t>* constructing_sub_cluster = new std::set<size_t>;
                std::list<size_t>* unexplored_sub_cluster_points = new std::list<size_t>;
                bool any_edges = false;
                bool incremented_child_count = false;

                std::set<size_t>::iterator root_vertex = --examined_vertices.end();
                constructing_sub_cluster->insert(*root_vertex);
                unexplored_sub_cluster_points->push_back(*root_vertex);
                examined_vertices.erase(root_vertex);

                //Explore this potential child cluster as long as there are unexplored points:
                while(!unexplored_sub_cluster_points->empty()) {
                    size_t vertex_to_explore = unexplored_sub_cluster_points->front();
                    unexplored_sub_cluster_points->pop_front();
                    for(size_t neighbor : mst.GetEdgeListForVertex(vertex_to_explore)) {
                        any_edges = true;
                        if(constructing_sub_cluster->find(neighbor) == constructing_sub_cluster->end()) {
                            constructing_sub_cluster->insert(neighbor);
                            unexplored_sub_cluster_points->push_back(neighbor);
                            examined_vertices.erase(neighbor);
                        }
                    }

                    //Check if this potential child cluster is a valid cluster:
                    if(!incremented_child_count && constructing_sub_cluster->size() >= min_cluster_size && any_edges) {
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
                if((num_child_clusters >= 2) && (constructing_sub_cluster->size() >= min_cluster_size) && any_edges) {
                    //Check this child cluster is not equal to the unexplored first child cluster:
                    size_t first_child_cluster_member = *(--first_child_cluster->end());
                    if(constructing_sub_cluster->find(first_child_cluster_member) != constructing_sub_cluster->end()) {
                        num_child_clusters--;
                    } else {
                        //Otherwise, create a new cluster:
                        Cluster* new_cluster = CreateNewCluster(*constructing_sub_cluster, current_cluster_labels, clusters[examined_cluster_label], next_cluster_label, current_edge_weight);
                        new_clusters.push_back(new_cluster);
                        clusters.push_back(new_cluster);
                        next_cluster_label++;
                    }
                } else if(constructing_sub_cluster->size() < min_cluster_size || !any_edges) { //If this child cluster is not valid cluster, assign it to noise:
                    CreateNewCluster(*constructing_sub_cluster, current_cluster_labels, clusters[examined_cluster_label], 0, current_edge_weight);

                    for(size_t point : *constructing_sub_cluster) {
                        point_noise_levels[point] = current_edge_weight;
                        point_last_clusters[point] = examined_cluster_label;
                    }
                }

                if(first_child_cluster != constructing_sub_cluster) {
                    delete constructing_sub_cluster;
                    delete unexplored_sub_cluster_points;
                }
           }
           //Finish exploring and cluster the first child cluster if there was a split and it was not already clustered:
           if((num_child_clusters >= 2) && (current_cluster_labels[*first_child_cluster->begin()] == examined_cluster_label)) {
               while(!unexplored_first_child_cluster_points->empty()) {
                   size_t vertex_to_explore = unexplored_first_child_cluster_points->front();
                   unexplored_first_child_cluster_points->pop_front();

                   for(size_t neighbor : mst.GetEdgeListForVertex(vertex_to_explore)) {
                       if(first_child_cluster->find(neighbor) == first_child_cluster->end()) {
                           first_child_cluster->insert(neighbor);
                           unexplored_first_child_cluster_points->push_back(neighbor);
                       }
                   }
               }

               Cluster* new_cluster = CreateNewCluster(*first_child_cluster, current_cluster_labels, clusters[examined_cluster_label], next_cluster_label, current_edge_weight);
               new_clusters.push_back(new_cluster);
               clusters.push_back(new_cluster);
               next_cluster_label++;
           }

            delete first_child_cluster;
            delete unexplored_first_child_cluster_points;
        }

        //Write out the current level of the hierarchy:
        if(!compact_hierarchy || next_level_significant || !new_clusters.empty()) {
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
        std::set<size_t> new_cluster_labels;
        for(Cluster* new_cluster : new_clusters) {
            new_cluster->SetFileOffset(hierarchy_chars_written);
            new_cluster_labels.insert(new_cluster->GetLabel());
        }
        if(!new_cluster_labels.empty()) {
            CalculateNumConstraintsSatisfied(new_cluster_labels, clusters, constraints, current_cluster_labels);
        }

        for(size_t i = 0; i < current_cluster_labels_length; ++i) { // TODO we can do a pointer swap here to speed things up
            previous_cluster_labels[i] = current_cluster_labels[i];
        }

        next_level_significant = !new_clusters.empty();
    }

    //Write out the final level of the hierarchy (all points noise):
    hierarchy_writer << "0" << delimiter;
    for(size_t i = 0; i < current_cluster_labels_length-1; ++i) {
        hierarchy_writer << "0" << delimiter;
    }
    hierarchy_writer << "0\n";
    line_count++;

    //Write out the cluster tree:
    for(Cluster* cluster : clusters) {
        if(cluster == nullptr) {
            continue;
        }

        tree_writer << cluster->GetLabel() << delimiter;
        tree_writer << cluster->GetBirthLevel() << delimiter;
        tree_writer << cluster->GetDeathLevel() << delimiter;
        tree_writer << cluster->GetStability() << delimiter;

        if(!constraints.empty()) {
            tree_writer << 0.5*cluster->GetNumConstraintsSatisfied() / constraints.size() << delimiter;
            tree_writer << 0.5 * cluster->GetPropagatedNumConstraintsSatisfied() / constraints.size() << delimiter;
        } else {
            tree_writer << 0 << delimiter << 0 << delimiter;
        }

        tree_writer << cluster->GetFileOffset() << delimiter;
        if(cluster->GetParent() != nullptr) {
            tree_writer << cluster->GetParent()->GetLabel() << "\n";
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


    delete[] previous_cluster_labels;
    delete[] current_cluster_labels;
}

void CalculateNumConstraintsSatisfied(
        const std::set<size_t>& new_cluster_labels, const std::vector<Cluster*>& clusters, const std::vector<Constraint>& constraints, size_t* cluster_labels) {

    if(constraints.empty()) {
        return;
    }

    std::vector<Cluster*> parents;
    for(size_t label : new_cluster_labels) {
        Cluster* parent = clusters[label]->GetParent();
        if(parent != nullptr && std::find(parents.begin(), parents.end(), parent) == parents.end()) {
            parents.push_back(parent);
        }
    }

    for(const Constraint& constraint : constraints) {
        size_t label_a = cluster_labels[constraint.point_a];
        size_t label_b = cluster_labels[constraint.point_b];

        if(constraint.type == Constraint::CONSTRAINT_TYPE::MUST_LINK && label_a == label_b) {
            if(new_cluster_labels.find(label_a) != new_cluster_labels.end()) {
                clusters[label_a]->AddConstraintsSatisfied(2);
            }
        } else if(constraint.type == Constraint::CONSTRAINT_TYPE::CANNOT_LINK && (label_a != label_b || label_a == 0)) {
            if(label_a != 0 && new_cluster_labels.find(label_a) != new_cluster_labels.end()) {
                clusters[label_a]->AddConstraintsSatisfied(1);
            }
            if(label_b != 0 && new_cluster_labels.find(label_b) != new_cluster_labels.end()) {
                clusters[label_b]->AddConstraintsSatisfied(1);
            }

            if(label_a == 0) {
                for(Cluster* parent : parents) {
                    if(parent->VirtualChildClusterContaintsPoint(constraint.point_a)) {
                        parent->AddVirtualChildConstraintsSatisfied(1);
                        break;
                    }
                }
            }

            if(label_b == 0) {
                for(Cluster* parent : parents) {
                    if(parent->VirtualChildClusterContaintsPoint(constraint.point_b)) {
                        parent->AddVirtualChildConstraintsSatisfied(1);
                        break;
                    }
                }
            }
        }
    }
}

Cluster* CreateNewCluster(const std::set<size_t>& points,
        size_t* cluster_labels, Cluster* parent_cluster, size_t cluster_label,
        double edge_weight) {
    for(size_t point : points) {
        cluster_labels[point] = cluster_label;
    }
    parent_cluster->DetachPoints(points.size(), edge_weight);

    if(cluster_label != 0) {
        return new Cluster(cluster_label, parent_cluster, edge_weight, points.size());
    }

    parent_cluster->AddPointsToVirtualChildCluster(points);
    return nullptr;
}
