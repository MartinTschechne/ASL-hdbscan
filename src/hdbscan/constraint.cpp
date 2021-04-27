#include <hdbscan/constraint.h>

Constraint::Constraint(size_t point_a, size_t point_b, CONSTRAINT_TYPE type) :
    point_a_(point_a),
    point_b_(point_b),
    type_(type) {}

size_t Constraint::GetPointA() const {
    return point_a_;
}
size_t Constraint::GetPointB() const {
    return point_b_;
}
Constraint::CONSTRAINT_TYPE Constraint::GetType() const {
    return type_;
}