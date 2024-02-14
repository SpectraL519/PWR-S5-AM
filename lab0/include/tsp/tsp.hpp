#pragma once

#include "graph.hpp"
#include "point.hpp"

#include <algorithm>
#include <fstream>
#include <limits>
#include <stdexcept>



namespace tsp {

using weight_type = distance_type;

constexpr weight_type max_weight = std::numeric_limits<weight_type>::max();
constexpr distance_type max_distance = max_weight;


[[nodiscard]] weight_type cycle_weight(const std::vector<point>& cycle) {
    weight_type weight = 0;

    for (std::size_t i = 1; i < cycle.size(); i++)
        weight += point_distance(cycle.at(i), cycle.at(i - 1));
    weight += point_distance(cycle.at(0), cycle.at(cycle.size() - 1));

    return weight;
}


class cycle {
public:
    using vertex_coordinate_type = typename point::coordinate_type;
    using vertex_type = point;

    cycle() = default;
    ~cycle() = default;


    [[nodiscard]] inline std::size_t size() const {
        return this->_cycle.size();
    }

    void start(const vertex_type& start_vertex) {
        if (this->size() != 0)
            throw std::logic_error("Cycle is already started!");

        this->_cycle.push_back(start_vertex);
    }

    [[nodiscard]] inline bool has(const vertex_type& vertex) const {
        return std::find(
            this->_cycle.begin(),
            this->_cycle.end(),
            vertex
        ) != this->_cycle.end();
    }

    [[nodiscard]] inline const std::vector<vertex_type>& get() const {
        return this->_cycle;
    }

    inline void insert_after(
        const vertex_type& vertex, const vertex_type& after, const weight_type edge_weight
    ) {
        auto after_it = std::find(this->_cycle.begin(), this->_cycle.end(), after);

        if (after_it == this->_cycle.end())
            this->_cycle.push_back(vertex);
        else
            this->_cycle.insert(after_it + 1, vertex);

        this->_mst_weight += edge_weight;
    }

    [[nodiscard]] inline weight_type weight() const {
        return cycle_weight(this->_cycle);
    }

    [[nodiscard]] inline weight_type mst_weight() const {
        return this->_mst_weight;
    }

    void save_to_file(const std::string& file_path) const {
        std::ofstream file(file_path);

        if (not file.is_open())
            throw std::invalid_argument("Could not open: " + file_path + "!");

        file << "x,y" << std::endl;
        for (const auto& vertex : this->_cycle)
            file << vertex.x << "," << vertex.y << std::endl;
    }

private:
    std::vector<vertex_type> _cycle;
    weight_type _mst_weight = 0;
};


cycle graph_cycle(const complete_graph& graph) {
    using cycle_vertex_type = typename cycle::vertex_type;

    cycle cycle;
    cycle.start(graph.at(0));

    while (cycle.size() < graph.num_vertices()) {
        distance_type min_distance = max_distance;
        cycle_vertex_type src, dst;

        for (const auto& mst_vertex : cycle.get()) {
            for (const auto& vertex : graph.vertices()) {
                if (vertex == mst_vertex or cycle.has(vertex))
                    continue;

                const auto distance = point_distance(vertex, mst_vertex);

                if (distance < min_distance) {
                    src = mst_vertex;
                    dst = vertex;
                    min_distance = distance;
                }
            }
        }

        cycle.insert_after(dst, src, min_distance);
    }

    return cycle;
}


} // namespace tsp
