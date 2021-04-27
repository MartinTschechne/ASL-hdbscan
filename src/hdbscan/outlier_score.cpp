#include <hdbscan/outlier_score.h>

OutlierScore::OutlierScore(double score, double core_distance, size_t id) : 
    score_(score), 
    core_distance_(core_distance_), 
    id_(id) {}

size_t OutlierScore::GetId() const {
    return id_;
}

double OutlierScore::GetScore() const {
    return score_;
}

int OutlierScore::CompareTo(const OutlierScore* const other) const {
    if(this->score_ > other->score_) {
        return 1;
    }
    if(this->score_ < other->score_) {
        return -1;
    }

    if(this->core_distance_ > other->core_distance_) {
        return 1;
    }

    if(this->core_distance_ < other->core_distance_) {
        return -1;
    }

    return this->id_ - other->id_;
}

int OutlierScore::CompareTo(const OutlierScore& other) const {
    return this->CompareTo(&other);
}