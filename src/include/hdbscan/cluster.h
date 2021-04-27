#ifndef TEAM33_HDBSCAN_CLUSTER_H
#define TEAM33_HDBSCAN_CLUSTER_H

#include <cstddef>
#include <set>
#include <vector>

/**
 * @brief An HDBSCAN* cluster, which will have a birth level, death level, stability, and constraint 
 * satisfaction once fully constructed.

 */
class Cluster {
private:
    size_t label_;
    double birth_level_;
    double death_level_;
    size_t num_points_;
    long file_offset_;

    double stability_;
    double propagated_stability_;
    double propagated_lowest_child_death_level_;
    size_t num_constraints_satisfied_;
    size_t propagated_num_constraints_satisfied_;
    std::set<size_t> virtual_child_cluster_;
    Cluster* parent_;
    bool has_children_;
    std::vector<Cluster*> propagated_descendants_;

public:
    /**
     * @brief Construct a new Cluster object
     * 
     * @param label The cluster label, which should be globally unique
     * @param parent The cluster which split to create this cluster
     * @param birth_level The MST edge level at which this cluster first appeared
     * @param num_points The initial number of points in this cluster
     */
    Cluster(size_t label, Cluster* parent, double birth_level, size_t num_points);

    /**
     * @brief Removes the specified number of points from this cluster at the given edge level, which will
	 * update the stability of this cluster and potentially cause cluster death.  If cluster death
	 * occurs, the number of constraints satisfied by the virtual child cluster will also be calculated.
     * 
     * @param num_points The number of points to remove from the cluster
     * @param level The MST edge level at which to remove these points
     */
    void DetachPoints(size_t num_points, double level);

    /**
	 * This cluster will propagate itself to its parent if its number of satisfied constraints is
	 * higher than the number of propagated constraints.  Otherwise, this cluster propagates its
	 * propagated descendants.  In the case of ties, stability is examined.
	 * Additionally, this cluster propagates the lowest death level of any of its descendants to its
	 * parent.
	 */
    void Propagate();

    void AddPointsToVirtualChildCluster(const std::set<size_t>& points);

    bool VirtualChildClusterContaintsPoint(size_t point) const; 

    void AddVirtualChildConstraintsSatisfied(size_t num_constraints);

    void addConstraintsSatisfied(size_t num_constraints);

    /**
     * @brief Removes all elements from the virtual child cluster, thereby saving memory.  Only call this method after computing the
	 * number of constraints satisfied by the virtual child cluster.
     */
    void releaseVirtualChildCluster();

    size_t GetLabel() const;

    Cluster* GetParent() const;

    double GetBirthLevel() const;

    double GetDeathLevel() const;

    double GetFileOffset() const;

    void SetFileOffset(long offset);

    double GetStability();

    double GetPropagatedLowestChildDeathLevel() const;

    size_t GetNumConstraintsSatisfied() const;
    
    size_t GetPropagatedNumConstraintsSatisfied() const;

    bool HasChildren() const;

    const std::vector<Cluster*>& GetPropagatedDescendants() const;
};

#endif