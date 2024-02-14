#pragma once

#include <cmath>
#include <concepts>
#include <cstdint>
#include <iostream>



namespace tsp {

struct point {
    using coordinate_type = uint32_t;

    coordinate_type x = 0;
    coordinate_type y = 0;

    point(coordinate_type x, coordinate_type y)
    : x(x), y(y) {}

    point() = default;
    point(const point& other) = default;
    point& operator=(const point& other) = default;

    bool operator==(const point& other) const = default;
};

std::ostream& operator<<(std::ostream& os, const point& p) {
    os << "(" << p.x << ", " << p.y << ")";
    return os;
}


namespace detail {

[[nodiscard]] double point_distance_impl(const point& p1, const point& p2) {
    const int64_t x_distance = (int64_t)p1.x - (int64_t)p2.x;
    const int64_t y_distance = (int64_t)p1.y - (int64_t)p2.y;

    return std::sqrt((x_distance * x_distance) + (y_distance * y_distance));
}

} // namespace detail

using distance_type = uint32_t;

[[nodiscard]] inline distance_type point_distance(const point& p1, const point& p2) {
    return static_cast<distance_type>(detail::point_distance_impl(p1, p2));
}

} // namespace tsp
