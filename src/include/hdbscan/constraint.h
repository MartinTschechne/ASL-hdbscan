#ifndef TEAM33_HDBSCAN_CONSTRAINT_H
#define TEAM33_HDBSCAN_CONSTRAINT_H

#include <string>

struct Constraint {
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

    size_t point_a;
    size_t point_b;
    Constraint::CONSTRAINT_TYPE type;
};
#endif