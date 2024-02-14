#pragma once

#include "graph.hpp"
#include "point.hpp"
#include "cycle.hpp"

#include <algorithm>
#include <fstream>
#include <optional>
#include <ranges>
#include <sstream>
#include <stdexcept>



namespace tsp::utility {

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

std::vector<point> read_points(const std::string& file_path) {
    std::ifstream file(file_path);

    if (not file.is_open())
        throw std::invalid_argument("Could not open: " + file_path + "!");

    std::vector<point> points;
    std::string line;

    while (std::getline(file, line)) {
        const auto point_opt = read_point(line);
        if (point_opt)
            points.push_back(point_opt.value());
    }

    return points;
}

graph<uint32_t> problem_graph(const std::vector<point>& points) {
    const auto num_points = points.size();
    graph<uint32_t> graph((uint32_t)num_points);

    for (uint32_t p = 0; p < num_points; p++)
        for (uint32_t q = 0; q < p; q++)
            graph.add_edge(p, q, point_distance(points.at(p), points.at(q)));

    return graph;
}

void save_cycle(const point_cycle_type& cycle, const std::string& file_path) {
        std::ofstream file(file_path);

        if (not file.is_open())
            throw std::invalid_argument("Could not open: " + file_path + "!");

        file << "x,y" << std::endl;
        for (const auto& vertex : cycle)
            file << vertex.x << "," << vertex.y << std::endl;
    }

} // namespace utility
