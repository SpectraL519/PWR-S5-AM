#include <tsp/graph.hpp>
#include <tsp/point.hpp>
#include <tsp/tsp.hpp>

#include <argparse.hpp>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>



int main (int argc, char* argv[]) {
    argparse::ArgumentParser parser("Traveling Salesman Problem - MST algorithm");

    parser
        .add_argument("-f", "--file")
        .required()
        .help("Path to a .tsp data file");
    parser
        .add_argument("-s", "--save")
        .help("Path to an output file for tsp results");
    parser
        .add_argument("-p", "--permutations")
        .default_value(false)
        .implicit_value(true)
        .help("Enables checking permutations");

    try {
        parser.parse_args(argc, argv);
    }
    catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << parser;
        std::exit(1);
    }

    const auto graph = tsp::from_file(parser.get("file"));
    const auto cycle = tsp::graph_cycle(graph);

    std::cout << "mst weight = " << cycle.mst_weight() << std::endl;
    std::cout << "cycle weight = " << cycle.weight() << std::endl;

    if (parser.present("save"))
        cycle.save_to_file(parser.get("save"));

    if (parser.get<bool>("permutations")) {
        std::cout << std::endl;

        std::vector<tsp::point> random_cycle(graph.num_vertices());
        std::copy(
            graph.vertices().begin(),
            graph.vertices().end(),
            random_cycle.begin()
        );

        constexpr uint16_t num_permutations = 1000;
        std::vector<tsp::weight_type> permutation_cycle_weights(num_permutations);
        tsp::weight_type min_weight = tsp::max_weight;

        std::random_device rd;
        std::mt19937 gen{rd()};

        for (uint16_t i = 0; i < num_permutations; i++) {
            std::shuffle(random_cycle.begin(), random_cycle.end(), gen);

            const auto weight = tsp::cycle_weight(random_cycle);
            permutation_cycle_weights.at(i) = weight;

            if (weight < min_weight)
                min_weight = weight;
        }

        std::vector<tsp::weight_type> min_weights_interval_10;
        tsp::weight_type min_weight_interval_10 = tsp::max_weight;

        std::vector<tsp::weight_type> min_weights_interval_50;
        tsp::weight_type min_weight_interval_50 = tsp::max_weight;

        for (const auto weight : permutation_cycle_weights) {
            static uint16_t iteration = 0;
            iteration++;

            if (weight < min_weight_interval_10)
                min_weight_interval_10 = weight;

            if (weight < min_weight_interval_50)
                min_weight_interval_50 = weight;

            if (iteration % 10 == 0) {
                min_weights_interval_10.push_back(min_weight_interval_10);
                min_weight_interval_10 = tsp::max_weight;
            }

            if (iteration % 50 == 0) {
                min_weights_interval_50.push_back(min_weight_interval_50);
                min_weight_interval_50 = tsp::max_weight;
            }
        }

        const auto avg_weight_interval_10 =
            std::accumulate(min_weights_interval_10.begin(), min_weights_interval_10.end(), 0) /
            min_weights_interval_10.size();

        const auto avg_weight_interval_50 =
            std::accumulate(min_weights_interval_50.begin(), min_weights_interval_50.end(), 0) /
            min_weights_interval_50.size();

        std::cout << "min weight = " << min_weight << std::endl;
        std::cout << "avg min weight (interval = 10) = "
                  << avg_weight_interval_10 << std::endl;
        std::cout << "avg min weight (interval = 50) = "
                  << avg_weight_interval_50 << std::endl;
    }

    return 0;
}
