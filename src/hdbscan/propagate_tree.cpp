#include <hdbscan/HDBSCAN_star.h>
#include <math.h>

bool PropagateTree(const std::vector<Cluster*>& clusters) {
    std::map<int, Cluster*> clusters_to_examine;
    std::vector<bool> added_to_examination_list(clusters.size(), false);
    bool infinite_stability = false;

    //Find all leaf clusters in the cluster tree
    for(Cluster* cluster : clusters) {
        if(cluster != nullptr && !cluster->has_children) {
            clusters_to_examine.insert({cluster->label, cluster});
            added_to_examination_list[cluster->label];
        }
    }

    //Iterate through every cluster, propagating stability from children to parents
    while(!clusters_to_examine.empty()) {
        auto cluster_last_it = --clusters_to_examine.end();
        Cluster* current_cluster = cluster_last_it->second;
        clusters_to_examine.erase(cluster_last_it);

        Propagate(current_cluster);
        if(current_cluster->stability == INFINITY) {
            infinite_stability = true;
        }

        if(current_cluster->parent != nullptr) {
            Cluster* parent = current_cluster->parent;

            if(!added_to_examination_list[parent->label]) {
                clusters_to_examine.insert({parent->label, parent});
                added_to_examination_list[parent->label] = true;
            }
        }
    }

    if(infinite_stability) {
        std::cout << warning_message << std::endl;
    }

    return infinite_stability;
}