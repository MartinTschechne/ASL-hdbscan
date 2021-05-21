#ifndef TEAM33_HDBSCAN_CLUSTER_H
#define TEAM33_HDBSCAN_CLUSTER_H

#include <cstddef>
#include <set>
#include <common/vector.h>

/**
 * @brief An HDBSCAN* cluster, which will have a birth level, death level, stability, and constraint 
 * satisfaction once fully constructed.

 */
struct Cluster {
    size_t label;
    double birth_level;
    double death_level;
    size_t num_points;
    size_t file_offset;

    double stability;
    double propagated_stability;
    double propagated_lowest_child_death_level;
    size_t num_constraints_satisfied;
    size_t propagated_num_constraints_satisfied;
    std::set<size_t>* virtual_child_cluster;
    Cluster* parent;
    bool has_children;
    Vector* propagated_descendants;
};

/**
 * @brief Construct a new Cluster object
 * 
 * @param label The cluster label, which should be globally unique
 * @param parent The cluster which split to create this cluster
 * @param birth_level The MST edge level at which this cluster first appeared
 * @param num_points The initial number of points in this cluster
 */
Cluster* CreateCluster(size_t label, Cluster* parent, double birth_level, size_t num_points);

/**
 * @brief Removes the specified number of points from this cluster at the given edge level, which will
 * update the stability of this cluster and potentially cause cluster death.  If cluster death
 * occurs, the number of constraints satisfied by the virtual child cluster will also be calculated.
 * 
 * @param num_points The number of points to remove from the cluster
 * @param level The MST edge level at which to remove these points
 */
void DetachPoints(Cluster* cluster, size_t num_points, double level);

/**
 * This cluster will propagate itself to its parent if its number of satisfied constraints is
 * higher than the number of propagated constraints.  Otherwise, this cluster propagates its
 * propagated descendants.  In the case of ties, stability is examined.
 * Additionally, this cluster propagates the lowest death level of any of its descendants to its
 * parent.
 */
void Propagate(Cluster* cluster);

/**
 * @brief Adds points to the virtual child cluster
 * 
 * @param points 
 */
void AddPointsToVirtualChildCluster(Cluster* cluster, const std::set<size_t>& points);

/**
 * @brief Check if the child cluster contains a given point
 * 
 * @param point 
 * @return true 
 * @return false 
 */
bool VirtualChildClusterContaintsPoint(Cluster* cluster, size_t point); 

/**
 * @brief Adds satisfied constraints to the child cluster
 * 
 * @param num_constraints 
 */
void AddVirtualChildConstraintsSatisfied(Cluster* cluster, size_t num_constraints);

/**
 * @brief Add satisfied constraints to the cluster
 * 
 * @param num_constraints 
 */
void AddConstraintsSatisfied(Cluster* cluster, size_t num_constraints);

/**
 * @brief Removes all elements from the virtual child cluster, thereby saving memory.  Only call this method after computing the
 * number of constraints satisfied by the virtual child cluster.
 */
void ReleaseVirtualChildCluster(Cluster* cluster);

/**
 * @brief Freeing the cluster memory
 * 
 * @param cluster 
 */
void FreeCluster(Cluster* cluster);

#endif