#pragma once

#include "cycle.hpp"

#include <iostream>
#include <algorithm>
#include <cmath>
#include <random>



namespace tsp {

namespace detail {

[[nodiscard]] point_distance_type invert_weight(
    const point_cycle_type& cycle,
    const point_distance_type current_weight,
    uint32_t i, uint32_t j
) {
    const std::size_t cycle_length = cycle.size();

    const auto prev_i = (i == 0) ? (cycle_length - 1) : (i - 1);
    const auto post_j = (j + 1) % cycle_length;

    return current_weight - point_distance(cycle[prev_i], cycle[i])
                          - point_distance(cycle[j], cycle[post_j])
                          + point_distance(cycle[prev_i], cycle[j])
                          + point_distance(cycle[i], cycle[post_j]);
}

} // namespace detail



using local_search_output_type = std::tuple<point_cycle, point_distance_type, std::size_t>;

[[nodiscard]] local_search_output_type local_search(const point_cycle& cycle) {
    const auto cycle_length = cycle.length();
    point_cycle best_found_cycle(cycle);
    point_distance_type best_found_cycle_weight = best_found_cycle.weight();

    std::size_t improvements = 0;

    while (true) {
        point_distance_type tmp_cycle_weight = best_found_cycle_weight;
        uint32_t best_found_i = 0, best_found_j = 0;

        for (uint32_t i = 0; i < cycle_length; i++) {
            const uint32_t max_j = (uint32_t)(cycle_length - (i == 0));
            for (uint32_t j = i + 1; j < max_j; j++) {
                const point_distance_type iw = detail::invert_weight(
                    best_found_cycle.get(), best_found_cycle_weight, i, j);

                if (iw < tmp_cycle_weight) {
                    tmp_cycle_weight = iw;
                    best_found_i = i;
                    best_found_j = j;
                }
            }
        }

        if (not (tmp_cycle_weight < best_found_cycle_weight))
            return std::make_tuple(best_found_cycle, best_found_cycle_weight, improvements);

        improvements++;
        best_found_cycle_weight = tmp_cycle_weight;

        const uint32_t max_k = (best_found_j - best_found_i + 1) / 2;
        for (uint32_t k = 0; k < max_k; k++)
            best_found_cycle.invert(best_found_i + k, best_found_j - k);
    }
}

template <typename RandomEngine>
[[nodiscard]] local_search_output_type local_search_random_neighbourhood(
    const point_cycle& cycle, RandomEngine& engine
) {
    const auto cycle_length = cycle.length();
    point_cycle best_found_cycle(cycle);
    point_distance_type best_found_cycle_weight = best_found_cycle.weight();

    std::size_t improvements = 0;

    std::vector<std::pair<uint32_t, uint32_t>> inversions;
    for (uint32_t i = 0; i < cycle_length; i++)
        for (uint32_t j = i + 1; j < cycle_length - (i == 0); j++)
            inversions.emplace_back(i, j);

    while (true) {
        point_distance_type tmp_cycle_weight = best_found_cycle_weight;
        uint32_t best_found_i = 0, best_found_j = 0;

        std::shuffle(inversions.begin(), inversions.end(), engine);

        for (uint32_t n = 0; n < cycle_length; n++) {
            const auto [i, j] = inversions[n];

            const auto iw = detail::invert_weight(
                best_found_cycle.get(), best_found_cycle_weight, i, j);

            if (iw < tmp_cycle_weight) {
                tmp_cycle_weight = iw;
                best_found_i = i;
                best_found_j = j;
            }
        }

        if (not (tmp_cycle_weight < best_found_cycle_weight))
            return std::make_tuple(best_found_cycle, best_found_cycle_weight, improvements);

        improvements++;
        best_found_cycle_weight = tmp_cycle_weight;

        const uint32_t max_k = (best_found_j - best_found_i + 1) / 2;
        for (uint32_t k = 0; k < max_k; k++)
            best_found_cycle.invert(best_found_i + k, best_found_j - k);
    }
}

} // namespace tsp
