#pragma once

#include "point.hpp"

#include <algorithm>
#include <concepts>
#include <limits>
#include <ranges>
#include <vector>
#include <queue>



namespace tsp {

namespace detail {

template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

} // namespace detail



template <std::unsigned_integral Vertex>
class graph {
public:
    using vertex_type = Vertex;
    using weight_type = point_distance_type;

    struct edge_type {
        edge_type(vertex_type src, vertex_type dst, weight_type w)
            : source(src), destination(dst), weight(w)
        {}

        edge_type() = default;
        edge_type(const edge_type& other) = default;
        edge_type& operator=(const edge_type& other) = default;

        vertex_type source;
        vertex_type destination;
        weight_type weight;

        [[nodiscard]] inline edge_type reverse() const {
            return edge_type{destination, source, weight};
        }

        struct comp {
            bool operator() (const edge_type& lhs, const edge_type& rhs) const {
                return lhs.weight > rhs.weight;
            }
        };
    };

    using adjacent_type = std::vector<edge_type>;

    graph(Vertex num_vertices) : _adjacency_list(num_vertices) {}

    ~graph() = default;

    [[nodiscard]] inline Vertex num_vertices() const {
        return (Vertex)this->_adjacency_list.size();
    }

    [[nodiscard]] inline const std::vector<vertex_type>& vertices() const {
        return this->_adjacency_list;
    }

    [[nodiscard]] inline const adjacent_type& adjacent(vertex_type vertex) const {
        return this->_adjacency_list.at(vertex);
    }

    void add_edge(const vertex_type& u, const vertex_type& v, const weight_type& w) {
        this->_adjacency_list.at(u).emplace_back(u, v, w);
        this->_adjacency_list.at(v).emplace_back(v, u, w);
    }

    void add_edge(const edge_type& edge) {
        this->_adjacency_list.at(edge.source).push_back(edge);
        this->_adjacency_list.at(edge.destination).push_back(edge.reverse());
    }

    // returns a mst with complete graph's vertex indices as vertices
    std::pair<graph<vertex_type>, weight_type> mst() const {
        const auto num_vertices = this->num_vertices();
        constexpr vertex_type start_vertex = 0;

        std::vector<bool> visited(num_vertices, false);
        visited.at(start_vertex) = true;
        uint32_t num_vertices_in_mst = 1; // start vertex

        std::priority_queue<edge_type, adjacent_type, typename edge_type::comp> edges(
            this->adjacent(start_vertex).begin(),
            this->adjacent(start_vertex).end()
        );

        graph<vertex_type> mst(num_vertices);
        weight_type mst_weight = 0;

        while (num_vertices_in_mst < num_vertices) {
            const auto min_edge = edges.top();
            edges.pop();

            const auto vertex = min_edge.destination;
            if (not visited.at(vertex)) {
                mst.add_edge(min_edge);
                mst_weight += min_edge.weight;

                visited.at(vertex) = true;
                num_vertices_in_mst++;
            }

            for (const auto& edge : this->adjacent(vertex))
                if (not visited.at(edge.destination))
                    edges.push(edge);
        }

        return std::make_pair(mst, mst_weight);
    }

private:
    std::vector<adjacent_type> _adjacency_list;
};

} // namespace tsp
