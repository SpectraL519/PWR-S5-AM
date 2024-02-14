#pragma once

#include "cycle.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <deque>
#include <iostream>
#include <random>



namespace tsp {

namespace detail {

using inversion_type = std::pair<point_coordinate_type, point_coordinate_type>;

[[nodiscard]] std::vector<inversion_type> generate_inversions(const std::size_t cycle_length) {
    std::vector<inversion_type> inversions;
    for (uint32_t i = 0; i < cycle_length; i++)
        for (uint32_t j = i + 1; j < cycle_length - (i == 0); j++)
            inversions.emplace_back(i, j);

    return inversions;
}

[[nodiscard]] point_distance_type invert_weight_diff(
    const point_cycle_type& cycle, uint32_t i, uint32_t j
) {
    const std::size_t cycle_length = cycle.size();

    const auto prev_i = (i == 0) ? (cycle_length - 1) : (i - 1);
    const auto post_j = (j + 1) % cycle_length;

    return point_distance(cycle[i], cycle[post_j])
           + point_distance(cycle[j], cycle[prev_i])
           - point_distance(cycle[prev_i], cycle[i])
           - point_distance(cycle[post_j], cycle[j]);
}

[[nodiscard]] point_distance_type invert_weight(
    const point_cycle_type& cycle,
    const point_distance_type current_weight,
    uint32_t i, uint32_t j
) {
    return current_weight + invert_weight_diff(cycle, i, j);
}

} // namespace detail



using tsp_result_type = std::pair<point_cycle, point_distance_type>;

template <typename RandomEngine>
[[nodiscard]] tsp_result_type simulated_annealing(
    const point_cycle& cycle,
    RandomEngine& engine,
    const double init_temperature,
    const double alpha,
    const double min_temperature,
    const double max_iter_percentage
) {
    const auto cycle_length = cycle.length();

    auto inversions = detail::generate_inversions(cycle_length);

    // random generators
    std::uniform_real_distribution<> probability(0., 1.);
    std::uniform_int_distribution<std::size_t> inversion_idx(0, inversions.size() - 1);

    point_cycle curr_cycle(cycle);
    point_distance_type curr_cycle_weight = curr_cycle.weight();
    point_cycle best_cycle = curr_cycle;
    point_distance_type best_cycle_weight = curr_cycle_weight;

    double temperature = init_temperature;
    const std::size_t max_iterations = max_iter_percentage * cycle_length;

    // simulated anealing algorithm
    while (temperature > min_temperature) {
        std::size_t updates = 0;
        while (updates < max_iterations) {
            const auto& [i, j] = inversions[inversion_idx(engine)];

            const point_distance_type weight_diff =
                detail::invert_weight_diff(curr_cycle.get(), i, j);

            if (
                weight_diff < 0 or
                probability(engine) < std::exp((double)(-weight_diff) / temperature)
            ) {
                curr_cycle.invert(i, j);
                curr_cycle_weight += weight_diff;
                updates++;

                if (curr_cycle_weight < best_cycle_weight) {
                    best_cycle_weight = curr_cycle_weight;
                    best_cycle = curr_cycle;
                }
            }
        }

        temperature *= alpha;
    }

    return std::make_pair(best_cycle, best_cycle_weight);
}


template <typename RandomEngine>
[[nodiscard]] tsp_result_type taboo_search(
    const point_cycle& cycle,
    RandomEngine& engine,
    const std::size_t max_taboo_list_size,
    const std::size_t max_no_update_iterations
) {
    const auto cycle_length = cycle.length();
    point_cycle best_cycle(cycle);
    point_distance_type best_cycle_weight = best_cycle.weight();

    auto inversions = detail::generate_inversions(cycle_length);

    std::deque<detail::inversion_type> taboo_list;
    const auto taboo_list_contains =
        [&taboo_list] (const detail::inversion_type& inv) {
            return std::find(taboo_list.begin(), taboo_list.end(), inv) != taboo_list.end();
        };

    std::size_t no_update_it = 0;

    // taboo search algorithm
    while (no_update_it++ < max_no_update_iterations) {
        std::shuffle(inversions.begin(), inversions.end(), engine);

        detail::inversion_type best_inversion;
        point_distance_type best_candidate_weight = std::numeric_limits<point_distance_type>::max();

        point_distance_type tmp_weight = best_cycle_weight;

        for (const auto& inv : inversions) {
            const auto weight_diff = detail::invert_weight_diff(best_cycle.get(), inv.first, inv.second);
            if (weight_diff >= 0 and taboo_list_contains(inv))
                continue;

            if (tmp_weight + weight_diff < best_candidate_weight) {
                best_inversion = inv;
                best_candidate_weight = tmp_weight + weight_diff;
            }
        }

        taboo_list.push_back(best_inversion);
        if (taboo_list.size() > max_taboo_list_size)
            taboo_list.pop_front();

        if (best_candidate_weight < best_cycle_weight) {
            best_cycle.invert(best_inversion.first, best_inversion.second);
            best_cycle_weight = best_candidate_weight;
            no_update_it = 0;
        }
    }

    return std::make_pair(best_cycle, best_cycle_weight);
}

} // namespace tsp
