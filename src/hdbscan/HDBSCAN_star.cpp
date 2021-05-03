#include <hdbscan/HDBSCAN_star.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <sstream>
#include <string>

std::vector<std::vector<double>> HDBSCANStar::ReadInDataSet(std::string const& file_name, const char delimiter) {
    std::vector<std::vector<double>> result;
    std::ifstream file(file_name);
    std::string line;
    int num_attributes = -1;
    size_t line_count = 0;

    while(std::getline(file, line)) {
        std::string value;
        std::vector<double> row;
        std::stringstream stream(line);
        try {
            std::string value;
            while(std::getline(stream, value, delimiter)) {
                row.push_back(std::stod(value));
            }
            
            if(num_attributes == -1) {
                num_attributes = row.size();
            }
            if(row.size() != num_attributes) {
                throw std::runtime_error("Line " + std::to_string(line_count) + " has a different number of attributes than the first line");
            }

            result.push_back(row);
        } catch(const std::exception& e) {
            std::cout << "Failed to parse line " << line_count << ". " << e.what() << std::endl;
        }

        line_count++;
    }

    return result;
}

std::vector<Constraint> HDBSCANStar::ReadInConstraints(std::string const& file_name) {
    std::vector<Constraint> result;
    std::ifstream file(file_name);
    std::string line;
    int num_attributes = -1;
    size_t line_count = 0;

    while(std::getline(file, line)) {
        std::stringstream stream(line);
        
        size_t point_a;
        size_t point_b;
        std::string link_type;

        stream >> point_a;
        stream.ignore();
        stream >> point_b;
        stream.ignore();
        stream >> link_type;

        Constraint::CONSTRAINT_TYPE contraint_type = 
            link_type == Constraint::MUST_LINK_TAG 
            ? Constraint::CONSTRAINT_TYPE::MUST_LINK 
            : Constraint::CONSTRAINT_TYPE::CANNOT_LINK;

        result.push_back(Constraint(point_a, point_b, contraint_type));

        line_count++;
    }

    return result;
}

void HDBSCANStar::CalculateCoreDistances(const std::vector<std::vector<double>>& data_set, size_t k,
    DistanceCalculator distance_function, std::vector<double>& result) {
    
    size_t num_neighbors = k - 1;
    const double DOUBLE_MAX = std::numeric_limits<double>::max();
    result.clear();
    result.resize(data_set.size());

    if(k == 1) {
        for(size_t point = 0; point < data_set.size(); ++point) {
            result[point] = 0;
        }
        return;
    }

    double* knn_distances = new double[num_neighbors];
    for(size_t point = 0; point < data_set.size(); ++point) {
        for(size_t i = 0; i < num_neighbors; ++i) {
            knn_distances[i] = DOUBLE_MAX;
        }

        // TODO use symmetry
        for(size_t neighbor = 0; neighbor < data_set.size(); ++neighbor) {
            if(point == neighbor) {
                continue;
            }

            double distance = distance_function(data_set[point].data(), data_set[neighbor].data(), data_set[0].size());
            
            //Check at which position in the nearest distances the current distance would fit:
            size_t neighbor_index = num_neighbors;
            while(neighbor_index >= 1 && distance < knn_distances[neighbor_index-1]) {
                neighbor_index--;
            }

            //Shift elements in the array to make room for the current distance:
            if(neighbor_index < num_neighbors) {
                for(size_t shift_index = num_neighbors - 1; shift_index > neighbor_index; shift_index--) {
                    knn_distances[shift_index] = knn_distances[shift_index - 1];
                }
                knn_distances[neighbor_index] = distance;
            }
        }
        result[point] = knn_distances[num_neighbors-1];
    }
    delete[] knn_distances;
}

UndirectedGraph HDBSCANStar::ConstructMST(const std::vector<std::vector<double>>& data_set,
        const std::vector<double>& core_distances, bool self_edges,
        DistanceCalculator distance_function) {
    
    size_t self_edge_capacity = self_edges ? data_set.size() : 0;
    const double DOUBLE_MAX = std::numeric_limits<double>::max();
    const size_t SIZE_T_MAX = std::numeric_limits<size_t>::max();

    //One bit is set (true) for each attached point, or unset (false) for unattached points:
    std::vector<bool> attached_points(data_set.size()); // bool vector is specialized to act like bitset 

    //Each point has a current neighbor point in the tree, and a current nearest distance:
    size_t* nearest_mrd_neighbors = new size_t[data_set.size()-1 + self_edge_capacity];
    double* nearest_mrd_distances = new double[data_set.size()-1 + self_edge_capacity];

    for(size_t i = 0; i < data_set.size()-1; ++i) {
        nearest_mrd_distances[i] = DOUBLE_MAX;
    }

    //The MST is expanded starting with the last point in the data set:
    size_t current_point = data_set.size() - 1;
    size_t num_attached_points = 1;
    attached_points[current_point] = true;

    //Continue attaching points to the MST until all points are attached:
    while(num_attached_points < data_set.size()) {
        size_t nearest_mrd_point = SIZE_T_MAX;
        double nearest_mrd_distance = DOUBLE_MAX;

        //Iterate through all unattached points, updating distances using the current point:
        for(size_t neighbor = 0; neighbor < data_set.size(); ++neighbor) {
            if(current_point == neighbor) {
                continue;
            }
            if(attached_points[neighbor]) { // point already attached
                continue;
            }

            double distance = distance_function(data_set[current_point].data(), data_set[neighbor].data(), data_set[0].size());
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
        attached_points[nearest_mrd_point] = true;
        ++num_attached_points;
        current_point = nearest_mrd_point;
    }

    //Create an array for vertices in the tree that each point attached to:
    size_t* other_vertex_indices = new size_t[data_set.size()-1 + self_edge_capacity];
    for(size_t i = 0; i < data_set.size(); ++i) {
        other_vertex_indices[i] = i;
    }

    //If necessary, attach self edges:
    if(self_edges) {
        for(size_t i = data_set.size() -1; i < data_set.size()*2 - 1; ++i) {
            size_t vertex = i - (data_set.size() - 1);
            nearest_mrd_neighbors[i] = vertex;
            other_vertex_indices[i] = vertex;
            nearest_mrd_distances[i] = core_distances[vertex];
        }
    }

    return UndirectedGraph(data_set.size(), nearest_mrd_neighbors, other_vertex_indices, nearest_mrd_distances, data_set.size()-1 + self_edge_capacity);
}

void HDBSCANStar::ComputeHierarchyAndClusterTree(
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
            auto examined_cluster_label_it = --affected_cluster_labels.end();
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

            std::set<size_t> first_child_cluster;
            std::list<size_t> unexplored_first_child_cluster_points;
            size_t num_child_clusters;

            /*
            * Check if the cluster has split or shrunk by exploring the graph from each affected
            * vertex.  If there are two or more valid child clusters (each has >= minClusterSize
            * points), the cluster has split.
            * Note that firstChildCluster will only be fully explored if there is a cluster
            * split, otherwise, only spurious components are fully explored, in order to label 
            * them noise.
            */
           while(!examined_vertices.empty()) {
                std::set<size_t> constructing_sub_cluster;
                std::list<size_t> unsexplored_sub_cluster_points;
                bool any_edges = false;
                bool incremented_child_count = false;

                std::set<size_t>::iterator root_vertex = --examined_vertices.end();
                constructing_sub_cluster.insert(*root_vertex);
                unsexplored_sub_cluster_points.push_back(*root_vertex);
                examined_vertices.erase(root_vertex);

                //Explore this potential child cluster as long as there are unexplored points:
                while(!unexplored_first_child_cluster_points.empty()) {
                    size_t vertex_to_explore = unexplored_first_child_cluster_points.front();
                    unexplored_first_child_cluster_points.pop_front();

                    for(size_t neighbor : mst.GetEdgeListForVertex(vertex_to_explore)) {
                        any_edges = true;
                        if(constructing_sub_cluster.find(neighbor) == constructing_sub_cluster.end()) {
                            constructing_sub_cluster.insert(neighbor);
                            unsexplored_sub_cluster_points.push_back(neighbor);
                            examined_vertices.erase(neighbor);
                        }
                    }

                    //Check if this potential child cluster is a valid cluster:
                    if(!incremented_child_count && constructing_sub_cluster.size() >= min_cluster_size && any_edges) {
                        incremented_child_count = true;
                        num_child_clusters++;

                        //If this is the first valid child cluster, stop exploring it:
                        if(first_child_cluster.empty()) {
                            first_child_cluster = constructing_sub_cluster;
                            unexplored_first_child_cluster_points = unsexplored_sub_cluster_points;
                            break;
                        }
                    }
                }

                //If there could be a split, and this child cluster is valid:
                if(num_child_clusters >= 2 && constructing_sub_cluster.size() >= min_cluster_size && any_edges) {
                    //Check this child cluster is not equal to the unexplored first child cluster:
                    size_t first_child_cluster_member = *(--first_child_cluster.end());
                    if(constructing_sub_cluster.find(first_child_cluster_member) != constructing_sub_cluster.end()) {
                        num_child_clusters--;
                    } else {
                        //Otherwise, create a new cluster:
                        Cluster* new_cluster = CreateNewCluster(constructing_sub_cluster, current_cluster_labels, clusters[examined_cluster_label], next_cluster_label, current_edge_weight);
                        new_clusters.push_back(new_cluster);
                        clusters.push_back(new_cluster);
                        next_cluster_label++;
                    }
                } else if(constructing_sub_cluster.size() < min_cluster_size || !any_edges) { //If this child cluster is not valid cluster, assign it to noise:
                    CreateNewCluster(constructing_sub_cluster, current_cluster_labels, clusters[examined_cluster_label], 0, current_edge_weight);

                    for(size_t point : constructing_sub_cluster) {
                        point_noise_levels[point] = current_edge_weight;
                        point_last_clusters[point] = examined_cluster_label;
                    }
                } 
           }
           //Finish exploring and cluster the first child cluster if there was a split and it was not already clustered:
           if(num_child_clusters >= 2 && current_cluster_labels[*first_child_cluster.begin()] == examined_cluster_label) {
               while(!unexplored_first_child_cluster_points.empty()) {
                   size_t vertex_to_explore = unexplored_first_child_cluster_points.front();
                   unexplored_first_child_cluster_points.pop_front();

                   for(size_t neighbor : mst.GetEdgeListForVertex(vertex_to_explore)) {
                       if(first_child_cluster.find(neighbor) == first_child_cluster.end()) {
                           first_child_cluster.insert(neighbor);
                           unexplored_first_child_cluster_points.push_back(neighbor);
                       }
                   }
               }

               Cluster* new_cluster = CreateNewCluster(first_child_cluster, current_cluster_labels, clusters[examined_cluster_label], next_cluster_label, current_edge_weight);
               new_clusters.push_back(new_cluster);
               clusters.push_back(new_cluster);
               next_cluster_label++;
           }
        }

        //Write out the current level of the hierarchy:
        if(!compact_hierarchy || next_level_significant || !new_clusters.empty()) {
            size_t output_lenght = 0;
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

        for(size_t i = 0; i < current_cluster_labels_length-1; ++i) {
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

void HDBSCANStar::FindProminentClusters(const std::vector<Cluster*>& clusters,
        const std::string& hierarchy_file, const std::string& flat_output_file,
        const char delimiter, size_t num_points, bool infinite_stability,
        std::vector<size_t>& result) {
    
    //Take the list of propagated clusters from the root cluster:
    const std::vector<Cluster*>& solution = clusters[1]->GetPropagatedDescendants();

    std::fstream reader(hierarchy_file);
    size_t* flat_partioning = new size_t[num_points];

    size_t current_offset = 0;

    //Store all the file offsets at which to find the birth points for the flat clustering:
    std::map<size_t, std::vector<size_t>*> significant_file_offsets;
    for(const Cluster* cluster : solution) {
        std::vector<size_t>* cluster_list = nullptr;
        auto entry = significant_file_offsets.find(cluster->GetFileOffset());
        if(entry == significant_file_offsets.end()) {
            cluster_list = new std::vector<size_t>;
            significant_file_offsets.insert({cluster->GetFileOffset(), cluster_list});
        } else {
            cluster_list = entry->second;
        }

        cluster_list->push_back(cluster->GetLabel());
    }

    //Go through the hierarchy file, setting labels for the flat clustering:
    for(auto it = significant_file_offsets.begin(); it != significant_file_offsets.end(); ) {
        std::vector<size_t>* cluster_list = it->second;
        size_t offset = it->first;

        reader.seekg(offset);
        std::string line;
        std::getline(reader, line);

        std::stringstream stream(line);
        std::string part;
        size_t i = 0;
        while(std::getline(stream, part, delimiter)) {
            if(i == 0) continue;
            size_t label = std::stoul(part);

            if(std::find(cluster_list->begin(), cluster_list->end(), label) != cluster_list->end()) {
                flat_partioning[i-1] = label;
            }

            ++i;
        }
        it = significant_file_offsets.erase(it);
    }

    //Output the flat clustering result:
    std::ofstream writer(flat_output_file);
    if(infinite_stability) {
        writer << warning_message << "\n";
    }

    for(size_t i = 0; i < num_points-1; ++i) {
        writer << flat_partioning[i] << delimiter;
    }

    writer << flat_partioning[num_points-1] << "\n";
    delete[] flat_partioning;
}

void HDBSCANStar::CalculateOutlierScores(
        const std::vector<Cluster*>& clusters, double* point_noise_levels, size_t point_noise_levels_length,
        size_t* point_last_clusters, double* core_distances,
        const std::string& outlier_scores_outputFile, const char delimiter,
        bool infinite_stability, std::vector<OutlierScore>& result) {
    
    result.clear();
    result.reserve(point_noise_levels_length);
    
    //Iterate through each point, calculating its outlier score:
    for(size_t i = 0; i < point_noise_levels_length; ++i) {
        double eps_max = clusters[point_last_clusters[i]]->GetPropagatedLowestChildDeathLevel();
        double eps = point_noise_levels[i];

        double score = 0.0;
        if(eps != 0) {
            score = 1 - (eps_max / eps);
        }

        result.push_back(OutlierScore(score, core_distances[i], i));
    }

    //Sort the outlier scores:
    std::sort(result.begin(), result.end(), [](const OutlierScore& a, const OutlierScore& b)->bool { return a.GetScore() > b.GetScore();});

    //Output the outlier scores:
    std::ofstream writer(outlier_scores_outputFile);
    if(infinite_stability) {
        writer << warning_message << "\n";
    }

    for(const OutlierScore& score : result) {
        writer << score.GetScore() << delimiter << score.GetId() << "\n";
    }
}

void HDBSCANStar::CalculateNumConstraintsSatisfied(
        const std::set<size_t>& new_cluster_labels, const std::vector<Cluster*>& clusters, const std::vector<Constraint>& constraints, size_t* cluster_labels) {
    
    if(constraints.empty()) {
        return;
    }

    std::vector<Cluster*> parents;
    for(size_t label : new_cluster_labels) {
        Cluster* parent = clusters[label]->GetParent();
        if(parent != nullptr && std::find(parents.begin(), parents.end(), parent) != parents.end()) {
            parents.push_back(parent);
        }
    }

    for(const Constraint& constraint : constraints) {
        size_t label_a = cluster_labels[constraint.GetPointA()];
        size_t label_b = cluster_labels[constraint.GetPointB()];

        if(constraint.GetType() == Constraint::CONSTRAINT_TYPE::MUST_LINK && label_a == label_b) {
            if(new_cluster_labels.find(label_a) != new_cluster_labels.end()) {
                clusters[label_a]->AddConstraintsSatisfied(2);
            }
        } else if(constraint.GetType() == Constraint::CONSTRAINT_TYPE::CANNOT_LINK && (label_a != label_b || label_a == 0)) {
            if(label_a != 0 && new_cluster_labels.find(label_a) != new_cluster_labels.end()) {
                clusters[label_a]->AddConstraintsSatisfied(1);
            }
            if(label_b != 0 && new_cluster_labels.find(label_b) != new_cluster_labels.end()) {
                clusters[label_b]->AddConstraintsSatisfied(1);
            }

            if(label_a == 0) {
                for(Cluster* parent : parents) {
                    if(parent->VirtualChildClusterContaintsPoint(constraint.GetPointA())) {
                        parent->AddVirtualChildConstraintsSatisfied(1);
                        break;
                    }
                }
            }

            if(label_b == 0) {
                for(Cluster* parent : parents) {
                    if(parent->VirtualChildClusterContaintsPoint(constraint.GetPointB())) {
                        parent->AddVirtualChildConstraintsSatisfied(1);
                        break;
                    }
                }
            }
        } 
    }
}

Cluster* HDBSCANStar::CreateNewCluster(const std::set<size_t>& points,
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

bool HDBSCANStar::PropagateTree(const std::vector<Cluster*>& clusters) {
    std::map<int, Cluster*> clusters_to_examine;
    std::vector<bool> added_to_examination_list(clusters.size(), false);
    bool infinite_stability = false;

    //Find all leaf clusters in the cluster tree
    for(Cluster* cluster : clusters) {
        if(cluster != nullptr && !cluster->HasChildren()) {
            clusters_to_examine.insert({cluster->GetLabel(), cluster});
            added_to_examination_list[cluster->GetLabel()];
        }
    }

    //Iterate through every cluster, propagating stability from children to parents
    while(!clusters_to_examine.empty()) {
        auto cluster_last_it = --clusters_to_examine.end();
        Cluster* current_cluster = cluster_last_it->second;
        clusters_to_examine.erase(cluster_last_it);

        current_cluster->Propagate();
        if(current_cluster->GetStability() == std::numeric_limits<double>::infinity()) {
            infinite_stability = true;
        }

        if(current_cluster->GetParent() != nullptr) {
            Cluster* parent = current_cluster->GetParent();

            if(!added_to_examination_list[parent->GetLabel()]) {
                clusters_to_examine.insert({parent->GetLabel(), parent});
                added_to_examination_list[parent->GetLabel()] = true;
            }
        }
    }

    if(infinite_stability) {
        std::cout << warning_message << std::endl;
    }

    return infinite_stability;
}