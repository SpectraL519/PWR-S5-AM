#pragma once

#include "graph.hpp"
#include "point.hpp"

#include <optional>
#include <random>
#include <stack>



namespace tsp {

class point_cycle {
public:
    using type = std::vector<point>;

    point_cycle() = default;

    point_cycle(uint32_t size) {
        this->_cycle.reserve(size);
    }

    point_cycle(const type& points) {
        std::copy(points.begin(), points.end(), std::back_inserter(this->_cycle));
    }

    point_cycle(const point_cycle& other) {
        std::copy(other._cycle.begin(), other._cycle.end(), std::back_inserter(this->_cycle));
    }

    point_cycle& operator= (const point_cycle& other) {
        this->_cycle = other._cycle;
        this->_weight = other._weight;
        return *this;
    }

    ~point_cycle() = default;

    [[nodiscard]] inline bool operator==(const point_cycle& other) const {
        return this->_cycle == other._cycle;
    }

    [[nodiscard]] inline bool operator<(point_cycle& other) {
        return this->weight() < other.weight();
    }

    [[nodiscard]] inline const type& get() const {
        return this->_cycle;
    }

    [[nodiscard]] inline std::size_t length() const {
        return this->_cycle.size();
    }

    inline void add(const point& p) {
        this->_cycle.push_back(p);
    }

    [[nodiscard]] inline const point& at(uint32_t i) const {
        return this->_cycle.at(i);
    }

    [[nodiscard]] static point_distance_type weight(const type& cycle) {
        point_distance_type weight = 0;
        const auto cycle_length = cycle.size();

        for (uint32_t i = 1; i < cycle_length; i++)
            weight += point_distance(cycle[i], cycle[i - 1]);
        weight += point_distance(cycle.front(), cycle.back());

        return weight;
    }

    [[nodiscard]] inline point_distance_type weight() {
        if (not this->_weight)
            this->_weight.emplace(weight(this->_cycle));
        return this->_weight.value();
    }

    void set_weight(const point_coordinate_type weight) {
        this->_weight.emplace(weight);
    }

    void reset_weight() {
        this->_weight = std::nullopt;
    }

    template <typename Generator>
    inline void shuffle(Generator& gen) {
        std::shuffle(this->_cycle.begin(), this->_cycle.end(), gen);
        this->_weight = std::nullopt;
    }

    inline void swap(uint32_t i, uint32_t j) {
        std::swap(this->_cycle.at(i), this->_cycle.at(j));
        this->_weight = std::nullopt;
    }

    void invert(uint32_t i, uint32_t j) {
        const uint32_t max_k = (j - i + 1) / 2;
        for (uint32_t k = 0; k < max_k; k++)
            std::swap(this->_cycle.at(i + k), this->_cycle.at(j - k));
        this->_weight = std::nullopt;
    }

private:
    type _cycle;
    std::optional<point_distance_type> _weight;
};

using point_cycle_type = typename point_cycle::type;



class point_cycle_builder {
public:
    template <std::unsigned_integral Vertex>
    [[nodiscard]] static point_cycle from_mst(
        const graph<Vertex>& mst,
        const std::vector<point>& points,
        const Vertex start_vertex
    ) {
        const auto num_vertices = mst.num_vertices();

        point_cycle cycle;

        std::vector<bool> visited(num_vertices, false);

        std::stack<Vertex> stack;
        stack.push(start_vertex);

        while (!stack.empty()) {
            const auto vertex = stack.top();
            stack.pop();

            if (visited.at(vertex))
                continue;

            visited.at(vertex) = true;
            cycle.add(points.at(vertex));

            for (const auto& edge : mst.adjacent(vertex))
                if (not visited.at(edge.destination))
                    stack.push(edge.destination);

        }

        return cycle;
    }
};

} // namespace tsp
