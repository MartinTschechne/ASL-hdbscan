#ifndef TEAM33_HDBSCAN_CONSTRAINT_H
#define TEAM33_HDBSCAN_CONSTRAINT_H

#include <string>

class Constraint {
public:
    inline static std::string MUST_LINK_TAG = "ml";
    inline static std::string CANNOT_LINK_TAG = "cl";

    /**
     * @brief COntraint types
     * 
     */
    enum CONSTRAINT_TYPE {
        MUST_LINK,
        CANNOT_LINK
    };

private:
    Constraint::CONSTRAINT_TYPE type_;
    size_t point_a_;
    size_t point_b_;

public:
    /**
     * @brief Construct a new Constraint object
     * 
     * @param point_a 
     * @param point_b 
     * @param type 
     */
    Constraint(size_t point_a, size_t point_b, CONSTRAINT_TYPE type);

    /**
     * @brief Get the Point A
     * 
     * @return size_t 
     */
    size_t GetPointA() const;

    /**
     * @brief Get the Point B
     * 
     * @return size_t 
     */
    size_t GetPointB() const;

    /**
     * @brief Get the Type object
     * 
     * @return CONSTRAINT_TYPE 
     */
    CONSTRAINT_TYPE GetType() const;
};

#endif