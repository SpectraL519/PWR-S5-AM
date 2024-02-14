#pragma once

#include "point.hpp"

#include <concepts>
#include <cstdint>
#include <fstream>
#include <optional>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <vector>



namespace tsp {

class complete_graph {
public:
    using vertex_coordinate_type = typename point::coordinate_type;
    using vertex_type = point;

    complete_graph() = default;
    ~complete_graph() = default;

    [[nodiscard]] inline std::size_t num_vertices() const {
        return this->_vertices.size();
    }

    [[nodiscard]] inline const std::vector<vertex_type>& vertices() const {
        return this->_vertices;
    }

    [[nodiscard]] inline const vertex_type& at(std::size_t idx) const {
        return this->_vertices.at(idx);
    }

    inline void add_vertex(const vertex_type& vertex) {
        this->_vertices.push_back(vertex);
    }

    inline const vertex_type& add_vertex(
        const vertex_coordinate_type x, const vertex_coordinate_type y
    ) {
        return this->_vertices.emplace_back(x, y);
    }

private:
    std::vector<vertex_type> _vertices;
};



namespace detail {

std::vector<std::string> split(const std::string& str, char delimiter) {
    auto substr_list = str | std::views::split(delimiter);

    std::vector<std::string> split;
    split.reserve(std::ranges::distance(substr_list));

    for (auto&& substr : substr_list)
        split.emplace_back(std::ranges::begin(substr), std::ranges::end(substr));

    return split;
}

std::optional<point> read_point(const std::string& line) {
    using point_coordinate_type = typename point::coordinate_type;
    static constexpr uint8_t point_line_size = 3;

    const auto split_line = split(line, ' ');
    if (split_line.size() != point_line_size)
        return std::nullopt;

    point_coordinate_type x;
    std::istringstream x_ss(split_line.at(1));
    if (not (x_ss >> x))
        return std::nullopt;

    point_coordinate_type y;
    std::istringstream y_ss(split_line.at(2));
    if (not (y_ss >> y))
        return std::nullopt;

    return point{x, y};
}

} // namespace detail

complete_graph from_file(const std::string& file_path) {
    std::ifstream file(file_path);

    if (not file.is_open())
        throw std::invalid_argument("Could not open: " + file_path + "!");

    complete_graph graph;
    std::string line;

    while (std::getline(file, line)) {
        const auto point_opt = detail::read_point(line);
        if (point_opt)
            graph.add_vertex(point_opt.value());
    }

    return graph;
}

} // namespace tsp
