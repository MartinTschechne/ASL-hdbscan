#ifndef TEAM33_HDBSCAN_CONSTRAINT_H
#define TEAM33_HDBSCAN_CONSTRAINT_H

#include <string>

class Constraint {
public:
    inline static std::string MUST_LINK_TAG = "ml";
    inline static std::string CANNOT_LINK_TAG = "cl";

    enum CONSTRAINT_TYPE {
        MUST_LINK,
        CANNOT_LINK
    };

private:
    Constraint::CONSTRAINT_TYPE type_;
    size_t point_a_;
    size_t point_b_;

public:
    Constraint(size_t point_a, size_t point_b, CONSTRAINT_TYPE type);
    size_t GetPointA() const;
    size_t GetPointB() const;
    CONSTRAINT_TYPE GetType() const;
};

#endif