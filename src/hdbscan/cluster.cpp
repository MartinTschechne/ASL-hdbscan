#include <hdbscan/cluster.h>

#include <float.h>
#include <stdexcept>

Cluster* CreateCluster(size_t label, Cluster* parent, double birth_level, size_t num_points) {
    Cluster* new_cluster = (Cluster*)malloc(sizeof(Cluster));
    new_cluster->label = label; 
    new_cluster->birth_level = birth_level; 
    new_cluster->death_level = 0.0; 
    new_cluster->num_points = num_points; 
    new_cluster->file_offset = 0; 
    new_cluster->stability = 0.0; 
    new_cluster->propagated_stability = 0.0; 
    new_cluster->propagated_lowest_child_death_level = DBL_MAX;
    new_cluster->num_constraints_satisfied = 0;
    new_cluster->propagated_num_constraints_satisfied = 0;
    new_cluster->parent = parent;
    new_cluster->has_children = false;
    new_cluster->propagated_descendants = new Vector;
    vector_init(new_cluster->propagated_descendants);
    new_cluster->virtual_child_cluster = new std::set<size_t>;

    if(parent != nullptr) {
        parent->has_children = true;
    }

    return new_cluster;
}


void DetachPoints(Cluster* cluster, size_t num_points, double level) {
    if(num_points > cluster->num_points) { // I think assert is hidden by some gtest module
        throw std::invalid_argument("Number of points to detach cannot be greater than the number of points in the cluster");
    }
    cluster->num_points -= num_points;
    cluster->stability += (num_points * (1.0 / level - 1.0 / cluster->birth_level));
    if(cluster->num_points == 0) {
        cluster->death_level = level;
    }
}

void Propagate(Cluster* cluster) {
    if(cluster->parent == nullptr) {
        return;
    }

    //Propagate lowest death level of any descendants:
    if(cluster->propagated_lowest_child_death_level == DBL_MAX) {
        cluster->propagated_lowest_child_death_level = cluster->death_level;
    }
    if(cluster->propagated_lowest_child_death_level < cluster->parent->propagated_lowest_child_death_level) {
        cluster->parent->propagated_lowest_child_death_level = cluster->propagated_lowest_child_death_level;
    }

    //If this cluster has no children, it must propagate itself:
    if(!cluster->has_children) {
        cluster->parent->propagated_num_constraints_satisfied += cluster->num_constraints_satisfied;
        cluster->parent->propagated_stability += cluster->stability;
        vector_push_back(cluster->parent->propagated_descendants, (void*)cluster);
    } else if(cluster->num_constraints_satisfied > cluster->propagated_num_constraints_satisfied) {
        cluster->parent->propagated_num_constraints_satisfied += cluster->num_constraints_satisfied;
        cluster->parent->propagated_stability += cluster->stability;
        vector_push_back(cluster->parent->propagated_descendants, (void*)cluster);
    } else if(cluster->num_constraints_satisfied < cluster->propagated_num_constraints_satisfied) {
        cluster->parent->propagated_num_constraints_satisfied += cluster->propagated_num_constraints_satisfied;
        cluster->parent->propagated_stability += cluster->propagated_stability;
        for(size_t i = 0; i < cluster->propagated_descendants->size; ++i) {
            vector_push_back(cluster->parent->propagated_descendants, vector_get(cluster->propagated_descendants, i));
        } // insert all 
    } else if(cluster->num_constraints_satisfied == cluster->propagated_num_constraints_satisfied) {
        //Chose the parent over descendants if there is a tie in stability:
        if(cluster->stability >= cluster->propagated_stability) {
            cluster->parent->propagated_num_constraints_satisfied += cluster->num_constraints_satisfied;
            cluster->parent->propagated_stability += cluster->stability;
            vector_push_back(cluster->parent->propagated_descendants, cluster);
        } else {
            cluster->parent->propagated_num_constraints_satisfied += cluster->propagated_num_constraints_satisfied;
            cluster->parent->propagated_stability += cluster->propagated_stability;
            for(size_t i = 0; i < cluster->propagated_descendants->size; ++i) {
                vector_push_back(cluster->parent->propagated_descendants, vector_get(cluster->propagated_descendants, i));
            } // insert all 
        }
    }
}

void AddPointsToVirtualChildCluster(Cluster* cluster, const std::set<size_t>& points) {
    cluster->virtual_child_cluster->insert(
        std::begin(points),
        std::end(points)
    );
}

bool VirtualChildClusterContaintsPoint(Cluster* cluster, size_t point) {
    return cluster->virtual_child_cluster->find(point) != cluster->virtual_child_cluster->end();
}

void AddVirtualChildConstraintsSatisfied(Cluster* cluster, size_t num_constraints) {
    cluster->propagated_num_constraints_satisfied += num_constraints;
}

void AddConstraintsSatisfied(Cluster* cluster, size_t num_constraints) {
    cluster->num_constraints_satisfied += num_constraints;
}

void ReleaseVirtualChildCluster(Cluster* cluster) {
    cluster->virtual_child_cluster->clear();
}

void FreeCluster(Cluster* cluster) {
    vector_free(cluster->propagated_descendants);
    delete cluster;
}