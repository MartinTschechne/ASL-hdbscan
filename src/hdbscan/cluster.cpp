#include <hdbscan/cluster.h>

#include <limits>
#include <stdexcept>

Cluster::Cluster(size_t label, Cluster* parent, double birth_level, size_t num_points) : 
    label_(label), 
    parent_(parent),
    birth_level_(birth_level),
    num_points_(num_points),
    death_level_(0.0),
    file_offset_(0),
    stability_(0.0),
    propagated_stability_(0.0),
    propagated_lowest_child_death_level_(std::numeric_limits<double>::max()),
    num_constraints_satisfied_(0),
    propagated_num_constraints_satisfied_(0),
    has_children_(false) {
    
    if(parent_ != nullptr) {
        parent_->has_children_ = true;
    }
}


void Cluster::DetachPoints(size_t num_points, double level) {
    if(num_points <= this->num_points_) { // I think assert is hidden by some gtest module
        throw std::invalid_argument("Number of points to detach cannot be greater than the number of points in the cluster");
    }
    this->num_points_ -= num_points;
    this->stability_ += (num_points * (1.0 / level - 1.0 / this->birth_level_));
    if(this->num_points_ == 0) {
        this->death_level_ = level;
    }
}

void Cluster::Propagate() {
    if(parent_ == nullptr) {
        return;
    }

    //Propagate lowest death level of any descendants:
    if(propagated_lowest_child_death_level_ == std::numeric_limits<double>::max()) {
        propagated_lowest_child_death_level_ = death_level_;
    }
    if(propagated_lowest_child_death_level_ < parent_->propagated_lowest_child_death_level_) {
        parent_->propagated_lowest_child_death_level_ = propagated_lowest_child_death_level_;
    }

    //If this cluster has no children, it must propagate itself:
    if(!has_children_) {
        parent_->propagated_num_constraints_satisfied_ += num_constraints_satisfied_;
        parent_->propagated_stability_ += stability_;
        parent_->propagated_descendants_.push_back(this);
    } else if(num_constraints_satisfied_ > propagated_num_constraints_satisfied_) {
        parent_->propagated_num_constraints_satisfied_ += num_constraints_satisfied_;
        parent_->propagated_stability_ += stability_;
        parent_->propagated_descendants_.push_back(this);
    } else if(num_constraints_satisfied_ < propagated_num_constraints_satisfied_) {
        parent_->propagated_num_constraints_satisfied_ += propagated_num_constraints_satisfied_;
        parent_->propagated_stability_ += propagated_stability_;
        parent_->propagated_descendants_.insert(
            std::end(parent_->propagated_descendants_), 
            std::begin(propagated_descendants_),
            std::end(propagated_descendants_)
        ); // insert all 
    } else if(num_constraints_satisfied_ == propagated_num_constraints_satisfied_) {
        //Chose the parent over descendants if there is a tie in stability:
        if(stability_ >= propagated_stability_) {
            parent_->propagated_num_constraints_satisfied_ += num_constraints_satisfied_;
            parent_->propagated_stability_ += stability_;
            parent_->propagated_descendants_.push_back(this);
        } else {
            parent_->propagated_num_constraints_satisfied_ += propagated_num_constraints_satisfied_;
            parent_->propagated_stability_ += propagated_stability_;
            parent_->propagated_descendants_.insert(
            std::end(parent_->propagated_descendants_), 
            std::begin(propagated_descendants_),
            std::end(propagated_descendants_)
        ); // insert all 
        }
    }
}

void Cluster::AddPointsToVirtualChildCluster(const std::set<size_t>& points) {
    virtual_child_cluster_.insert(
        std::begin(points),
        std::end(points)
    );
}

bool Cluster::VirtualChildClusterContaintsPoint(size_t point) const {
    return virtual_child_cluster_.find(point) != virtual_child_cluster_.end();
}

void Cluster::AddVirtualChildConstraintsSatisfied(size_t num_constraints) {
    propagated_num_constraints_satisfied_ += num_constraints;
}

void Cluster::addConstraintsSatisfied(size_t num_constraints) {
    num_constraints_satisfied_ += num_constraints;
}

void Cluster::releaseVirtualChildCluster() {
    virtual_child_cluster_.clear();
}

size_t Cluster::GetLabel() const {
    return label_;
}

Cluster* Cluster::GetParent() const {
    return parent_;
}

double Cluster::GetBirthLevel() const {
    return birth_level_;
}

double Cluster::GetDeathLevel() const {
    return death_level_;
}

double Cluster::GetFileOffset() const {
    return file_offset_;
}

void Cluster::SetFileOffset(long offset) {
    file_offset_ = offset;
}

double Cluster::GetStability() {
    return stability_;
}

double Cluster::GetPropagatedLowestChildDeathLevel() const {
    return propagated_lowest_child_death_level_;
}

size_t Cluster::GetNumConstraintsSatisfied() const {
    return num_constraints_satisfied_;
}

size_t Cluster::GetPropagatedNumConstraintsSatisfied() const {
    return propagated_num_constraints_satisfied_;
}

bool Cluster::HasChildren() const {
    return has_children_;
}
