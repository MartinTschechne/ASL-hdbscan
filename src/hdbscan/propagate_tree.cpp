#include <hdbscan/HDBSCAN_star.h>

bool PropagateTree(const std::vector<Cluster*>& clusters) {
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