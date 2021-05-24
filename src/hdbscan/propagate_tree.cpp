#include <hdbscan/HDBSCAN_star.h>
#include <math.h>
#include <common/bitset.h>
#include <common/map.h>

bool PropagateTree(const Vector* const clusters) {
    Map* clusters_to_examine = Map_create();
    BitSet_t added_to_examination_list = CreateBitset(clusters->size, false);
    bool infinite_stability = false;

    //Find all leaf clusters in the cluster tree
    for(size_t i = 0; i < clusters->size; ++i) {
        Cluster* cluster = (Cluster*)clusters->elements[i];
        if(cluster != nullptr && !cluster->has_children) {
            Map_insert(clusters_to_examine, cluster->label, (void*)cluster);
            added_to_examination_list[cluster->label];
        }
    }

    //Iterate through every cluster, propagating stability from children to parents
    while(!Map_empty(clusters_to_examine)) {
        size_t cluster_last_it = Map_prev(clusters_to_examine, Map_end(clusters_to_examine));
        Cluster* current_cluster = (Cluster*)Map_get_idx(clusters_to_examine, cluster_last_it);
        Map_erase(clusters_to_examine, Map_key_from_idx(clusters_to_examine, cluster_last_it));

        Propagate(current_cluster);
        if(current_cluster->stability == INFINITY) {
            infinite_stability = true;
        }

        if(current_cluster->parent != nullptr) {
            Cluster* parent = current_cluster->parent;

            if(!added_to_examination_list[parent->label]) {
                Map_insert(clusters_to_examine, parent->label, (void*)parent);
                added_to_examination_list[parent->label] = true;
            }
        }
    }

    if(infinite_stability) {
        std::cout << warning_message << std::endl;
    }

    return infinite_stability;
}