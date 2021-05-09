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
    size_t file_offset_;

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

    /**
     * @brief Adds points to the virtual child cluster
     * 
     * @param points 
     */
    void AddPointsToVirtualChildCluster(const std::set<size_t>& points);

    /**
     * @brief Check if the child cluster contains a given point
     * 
     * @param point 
     * @return true 
     * @return false 
     */
    bool VirtualChildClusterContaintsPoint(size_t point) const; 

    /**
     * @brief Adds satisfied constraints to the child cluster
     * 
     * @param num_constraints 
     */
    void AddVirtualChildConstraintsSatisfied(size_t num_constraints);

    /**
     * @brief Add satisfied constraints to the cluster
     * 
     * @param num_constraints 
     */
    void AddConstraintsSatisfied(size_t num_constraints);

    /**
     * @brief Removes all elements from the virtual child cluster, thereby saving memory.  Only call this method after computing the
	 * number of constraints satisfied by the virtual child cluster.
     */
    void ReleaseVirtualChildCluster();

    /**
     * @brief Get the cluster label
     * 
     * @return size_t 
     */
    size_t GetLabel() const;

    /**
     * @brief Get the Parent cluster
     * 
     * @return Cluster* 
     */
    Cluster* GetParent() const;

    /**
     * @brief Get the Birth Level
     * 
     * @return double 
     */
    double GetBirthLevel() const;

    /**
     * @brief Get the Death Level
     * 
     * @return double 
     */
    double GetDeathLevel() const;

    /**
     * @brief Get the File Offset
     * 
     * @return size_t 
     */
    size_t GetFileOffset() const;

    /**
     * @brief Set the File Offset
     * 
     * @param offset 
     */
    void SetFileOffset(size_t offset);

    /**
     * @brief Get the Stability
     * 
     * @return double 
     */
    double GetStability();

    /**
     * @brief Get the Propagated Lowest Child Death Level
     * 
     * @return double 
     */
    double GetPropagatedLowestChildDeathLevel() const;

    /**
     * @brief Get the Num Constraints Satisfied
     * 
     * @return size_t 
     */
    size_t GetNumConstraintsSatisfied() const;
    
    /**
     * @brief Get the Propagated Num Constraints Satisfied
     * 
     * @return size_t 
     */
    size_t GetPropagatedNumConstraintsSatisfied() const;

    /**
     * @brief Returns true if the cluster has children
     * 
     * @return true 
     * @return false 
     */
    bool HasChildren() const;
    
    /**
     * @brief Get the Propagated Descendants
     * 
     * @return const std::vector<Cluster*>& 
     */
    const std::vector<Cluster*>& GetPropagatedDescendants() const;
};

#endif