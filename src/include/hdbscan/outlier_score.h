#ifndef TEAM33_HDBSCAN_OUTLIER_SCORE_H
#define TEAM33_HDBSCAN_OUTLIER_SCORE_H

#include <cstddef>

/**
 * @brief Simple storage class that keeps the outlier score, core distance, and id (index) for a single point.
 * OutlierScores are sorted in ascending order by outlier score, with core distances used to break
 * outlier score ties, and ids used to break core distance ties.
 */
class OutlierScore {
private:
    double score_;
    double core_distance_;
    size_t id_;

public:
    /**
     * Creates a new OutlierScore for a given point.
	 * @param score The outlier score of the point
	 * @param coreDistance The point's core distance
	 * @param id The id (index) of the point
     */
    OutlierScore(double score, double core_distance, size_t id);

    /**
     * @brief Creates a order relation for objects of this class
     * 
     * @param other 
     * @return int 
     */
    int CompareTo(const OutlierScore& other) const;

    /**
     * @brief Creates a order relation for objects of this class
     * 
     * @param other 
     * @return int 
     */
    int CompareTo(const OutlierScore* const other) const;

    /**
     * @brief Get the score value
     * 
     * @return double 
     */
    double GetScore() const;

    /**
     * @brief Get the point id/index
     * 
     * @return size_t 
     */
    size_t GetId() const;
};

#endif