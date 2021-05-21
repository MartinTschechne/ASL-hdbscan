#include <hdbscan/HDBSCAN_star.h>
#include <math.h>
#include <common/bitset.h>

bool PropagateTree(const Vector* const clusters) {
    std::map<int, Cluster*> clusters_to_examine;
    BitSet_t added_to_examination_list = CreateBitset(clusters->size, false);
    bool infinite_stability = false;

    //Find all leaf clusters in the cluster tree
    for(size_t i = 0; i < clusters->size; ++i) {
        Cluster* cluster = (Cluster*)clusters->elements[i];
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