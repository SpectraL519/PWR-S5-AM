#pragma once

#include "cycle.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <deque>
#include <functional>
#include <random>
#include <set>
#include <thread>
#include <tuple>



namespace tsp {

namespace detail {

using inversion_type = std::pair<uint32_t, uint32_t>;

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



namespace genetic {

using population_type = std::vector<point_cycle>;
using parents_type = std::pair<std::size_t, std::size_t>;

template <typename Generator>
[[nodiscard]] population_type generate_population(
    const point_cycle& cycle, std::size_t population_size, Generator& gen
) {
    population_type population;

    for (std::size_t i = 0; i < population_size; i++) {
        point_cycle population_member = cycle;
        population_member.shuffle(gen);
        population.push_back(population_member);
    }

    return population;
}

template <typename Generator>
[[nodiscard]] std::vector<parents_type> generate_parents(
    const std::size_t population_size, Generator& gen
) {
    std::uniform_int_distribution<std::size_t> idx_distribution(0, population_size - 1);

    std::vector<parents_type> parents;
    parents.reserve(population_size);
    for (std::size_t i = 0; i < population_size; i++)
        parents.emplace_back(idx_distribution(gen), idx_distribution(gen));

    return parents;
}

using crossover_function = std::function<point_cycle(const point_cycle&, const point_cycle&)>;

[[nodiscard]] point_cycle single_point_crossover(const point_cycle& parent_a, const point_cycle& parent_b) {
    // take the first half of parent a's genes and fill the rest with the parent b's genes
    point_cycle child;
    std::vector<bool> used_genes(parent_a.length(), false);

    for (std::size_t i = 0; i < parent_a.length() / 2; i++) {
        child.add(parent_a.at(i));
        used_genes[parent_a.at(i).key] = true;
    }

    for (uint32_t i = 0; i < parent_b.length(); i++) {
        if (used_genes[parent_b.at(i).key])
            continue;

        child.add(parent_b.at(i));
        used_genes[parent_b.at(i).key] = true;
    }

    return child;
}

[[nodiscard]] point_cycle simple_crossover(const point_cycle& parent_a, const point_cycle& parent_b) {
    point_cycle child;
    for (std::size_t i = 0; i < parent_a.length(); i++)
        child.add(parent_a.at(parent_b.at(i).key));
    return child;
}

void local_search_mutation(
    point_cycle& cycle,
    const std::vector<detail::inversion_type> inversions
) {
    point_distance_type cycle_weight = cycle.weight();

    const uint32_t max_iterations = std::min(std::sqrt(cycle.length()), 100.);
    for (uint32_t i = 0; i < max_iterations; i++) {
        point_distance_type best_candidate_cycle_weight = cycle_weight;
        detail::inversion_type best_inversion = std::make_pair(0u, 0u);

        for (const auto& inv : inversions) {
            const auto weight_diff = detail::invert_weight_diff(cycle.get(), inv.first, inv.second);
            if (weight_diff >= 0)
                continue;

            if (best_candidate_cycle_weight + weight_diff < best_candidate_cycle_weight) {
                best_inversion = inv;
                best_candidate_cycle_weight += weight_diff;
            }
        }

        if (best_candidate_cycle_weight >= cycle_weight)
            return;

        cycle_weight = best_candidate_cycle_weight;
        cycle.invert(best_inversion.first, best_inversion.second);
    }
}

} // namespace genetic



using tsp_result_type = std::pair<point_cycle, point_distance_type>;

template <typename Generator>
[[nodiscard]] tsp_result_type genetic_algorithm(
    const point_cycle& cycle,
    Generator& gen,
    const double max_iter_percentage = 1.,
    const uint32_t population_size = 100u,
    const genetic::crossover_function& crossover = genetic::single_point_crossover,
    const double mutation_prob = 0.1
) {
    static std::uniform_real_distribution<double> probability(0., 1.);

    auto population = genetic::generate_population(cycle, population_size, gen);

    const auto cycle_length = cycle.length();
    point_cycle best_cycle = population[0];
    point_distance_type best_cycle_weight = best_cycle.weight();

    const auto inversions = detail::generate_inversions(cycle_length);
    const auto mutate_genes =
        [&inversions](point_cycle& c) { genetic::local_search_mutation(c, inversions); };

    const std::size_t max_iterations = max_iter_percentage * cycle_length;
    for (std::size_t i = 0; i < max_iterations; i++) {
        const auto parents = genetic::generate_parents(population_size, gen);
        genetic::population_type next_generation;

        for (std::size_t pi = 0; pi < population_size; pi++) {
            auto child_a = crossover(population[parents[pi].first], population[parents[pi].second]);
            auto child_b = crossover(population[parents[pi].second], population[parents[pi].first]);

            if (probability(gen) < mutation_prob)
                mutate_genes(child_a);
            if (probability(gen) < mutation_prob)
                mutate_genes(child_b);

            next_generation.push_back(child_a);
            next_generation.push_back(child_b);
        }

        population = next_generation;

        const auto& best_population_member_it = std::min_element(population.begin(), population.end());
        if (best_population_member_it->weight() < best_cycle_weight) {
            best_cycle = *best_population_member_it;
            best_cycle_weight = best_population_member_it->weight();
        }
    }

    return std::make_pair(best_cycle, best_cycle_weight);
}

namespace detail {

template <typename Generator>
void genetic_algorithm_thread(
    const point_cycle& cycle,
    point_cycle& best_cycle,
    Generator& gen,
    const double max_iter_percentage = 1.,
    const uint32_t population_size = 100u,
    const genetic::crossover_function& crossover = genetic::single_point_crossover,
    const double mutation_prob = 0.1
) {
    static std::uniform_real_distribution<double> probability(0., 1.);

    auto population = genetic::generate_population(cycle, population_size, gen);

    const auto cycle_length = cycle.length();
    best_cycle = population[0];
    point_distance_type best_cycle_weight = best_cycle.weight();

    const auto inversions = detail::generate_inversions(cycle_length);
    const auto mutate_genes =
        [&inversions](point_cycle& c) { genetic::local_search_mutation(c, inversions); };

    const std::size_t max_iterations = max_iter_percentage * cycle_length;
    for (std::size_t i = 0; i < max_iterations; i++) {
        const auto parents = genetic::generate_parents(population_size, gen);
        genetic::population_type next_generation;

        for (std::size_t pi = 0; pi < population_size; pi++) {
            auto child_a = crossover(population[parents[pi].first], population[parents[pi].second]);
            auto child_b = crossover(population[parents[pi].second], population[parents[pi].first]);

            if (probability(gen) < mutation_prob)
                mutate_genes(child_a);
            if (probability(gen) < mutation_prob)
                mutate_genes(child_b);

            next_generation.push_back(child_a);
            next_generation.push_back(child_b);
        }

        population = next_generation;

        const auto& best_population_member_it = std::min_element(population.begin(), population.end());
        if (best_population_member_it->weight() < best_cycle_weight) {
            best_cycle = *best_population_member_it;
            best_cycle_weight = best_population_member_it->weight();
        }
    }
}

} // namespace detail

using tsp_threaded_result_type =
    std::tuple<point_cycle, point_distance_type, point_distance_type>;

template <uint8_t NumThreads, typename Generator>
[[nodiscard]] tsp_threaded_result_type threaded_genetic_algorithm(
    const point_cycle& cycle,
    Generator& gen,
    const double max_iter_percentage = 1.,
    const uint32_t population_size = 100u,
    const genetic::crossover_function& crossover = genetic::single_point_crossover,
    const double mutation_prob = 0.1
) {
    std::vector<point_cycle> thread_best_cycles(NumThreads);
    std::vector<std::thread> threads;

    for (uint8_t i = 0; i < NumThreads; i++)
        threads.push_back(std::thread(
            detail::genetic_algorithm_thread<Generator>,
            std::ref(cycle), std::ref(thread_best_cycles[i]), std::ref(gen),
            max_iter_percentage, population_size, crossover, mutation_prob
        ));

    for (auto& thread : threads)
        thread.join();

    const auto& best_cycle_it = std::min_element(thread_best_cycles.begin(), thread_best_cycles.end());
    point_distance_type avg_cycle_weight = 0.;
    for (auto& cycle : thread_best_cycles)
        avg_cycle_weight += cycle.weight();
    avg_cycle_weight /= NumThreads;

    return std::make_tuple(*best_cycle_it, best_cycle_it->weight(), avg_cycle_weight);
}


} // namespace tsp
