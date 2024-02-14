#include <tsp/cycle.hpp>
#include <tsp/utility.hpp>
#include <tsp/tsp.hpp>

#include <argparse.hpp>

#include <chrono>
#include <iostream>
#include <limits>
#include <random>
#include <fstream>



int main (int argc, char* argv[]) {
    argparse::ArgumentParser parser("Traveling Salesman Problem - MST algorithm");

    // general args
    parser
        .add_argument("-f", "--file")
        .required()
        .help("Path to a .tsp data file");
    parser
        .add_argument("-m", "--mode")
        .action([] (const std::string& value) {
            static const std::vector <std::string> choices = { "sa", "ts" };
            if (std::find(choices.begin(), choices.end(), value) != choices.end())
                return value;
            return std::string("sa");
        })
        .default_value("sa")
        .help("program mode: [sa - simulated anealing] [ts - taboo search]");
    parser
        .add_argument("-e", "--experimental")
        .default_value(false)
        .implicit_value(true)
        .help("enables saving additional data");

    // simulate annealing args
    parser
        .add_argument("-sit", "--sa-init-temp")
        .scan<'g', double>()
        .default_value(100.)
        .help("simulated annealing: initial temperature");
    parser
        .add_argument("-sa", "--sa-alpha")
        .scan<'g', double>()
        .default_value(0.99)
        .help("simulated annealing: alpha");
    parser
        .add_argument("-smt", "--sa-min-temp")
        .scan<'g', double>()
        .default_value(0.1)
        .help("simulated annealing: minimum temperature");
    parser
        .add_argument("-smi", "--sa-max-iter")
        .scan<'g', double>()
        .default_value(0.3)
        .help("simulated annealing: maximum iterations (percentage of graph size)");

    // taboo search args
    parser
        .add_argument("-tls", "--ts-list-size")
        .scan<'u', std::size_t>()
        .default_value(std::size_t{100})
        .help("taboo search: maximum taboo list size");
    parser
        .add_argument("-tmi", "--ts-max-iter")
        .scan<'u', std::size_t>()
        .default_value(std::size_t{5})
        .help("taboo search: maximum iterations (percentage of graph size)");

    parser
        .add_argument("-s", "--save")
        .help("Path to an output file for tsp results");
    parser
        .add_argument("-b", "--best")
        .help("Path to an output file for the best found cycle");

    try {
        parser.parse_args(argc, argv);
    }
    catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << parser;
        std::exit(1);
    }

    const auto input_file_name = parser.get("file");
    const auto points = tsp::utility::read_points(input_file_name);
    const auto graph = tsp::utility::problem_graph(points);

    // uint32_t num_iterations = std::min((uint32_t)std::sqrt(graph.num_vertices()), (uint32_t)100);
    constexpr uint32_t num_iterations = 10;
    constexpr double percentage_step = 0.1;

    uint32_t seed = (uint32_t)std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 generator(seed);
    std::default_random_engine engine{seed};
    std::uniform_int_distribution<uint32_t> uniform(0, graph.num_vertices() - 1);

    tsp::point_distance_type avg_cycle_weight = 0;
    tsp::point_distance_type min_cycle_weight =
        std::numeric_limits<tsp::point_distance_type>::max();
    tsp::point_cycle best_cycle;

    if (parser.get("mode") == "sa") {
        const auto init_temperature = parser.get<double>("sit");
        const auto alpha            = parser.get<double>("sa");
        const auto min_temperature  = parser.get<double>("smt");
        const auto max_iter         = parser.get<double>("smi");

        tsp::point_cycle random_cycle(points);

        double percentage = 0.0;
        for (uint32_t i = 0; i < num_iterations; i++) {
            if ((double)(i + 1) / num_iterations > percentage) {
                std::cout << "[ " << (int)(percentage * 100) << "% ]\n";
                percentage += percentage_step;
            }

            random_cycle.shuffle(generator);

            const auto [sa_cycle, sa_cycle_weight] =
                tsp::simulated_annealing(
                    random_cycle, engine,
                    init_temperature, alpha, min_temperature, max_iter
                );

            avg_cycle_weight += sa_cycle_weight;
            if (sa_cycle_weight < min_cycle_weight) {
                min_cycle_weight = sa_cycle_weight;
                best_cycle = sa_cycle;
            }
        }

        avg_cycle_weight /= num_iterations;

        std::cout << std::endl
                << "avg cycle weight = " << avg_cycle_weight << std::endl
                << "min cycle weight = " << min_cycle_weight << std::endl;

        if (parser.present("save")) {
            std::cout << "\nsaving results\n";
            std::ofstream file;
            file.open(parser.get("save"), std::ios_base::app);

            file << input_file_name << "," << graph.num_vertices() << ","
                << avg_cycle_weight << "," << min_cycle_weight;

            if (parser.get<bool>("experimental")) {
                file << "," << init_temperature
                     << "," << alpha
                     << "," << min_temperature
                     << "," << max_iter;
            }

            file << std::endl;
        }

        if (parser.present("best")) {
            std::cout << "\nsaving best found cycle\n";
            tsp::utility::save_cycle(best_cycle.get(), parser.get("best"));
        }
    }
    else {
        const auto max_taboo_list_size = parser.get<std::size_t>("tls");
        const auto max_iter            = parser.get<std::size_t>("tmi");

        tsp::point_cycle random_cycle(points);

        double percentage = 0.0;
        for (uint32_t i = 0; i < num_iterations; i++) {
            if ((double)(i + 1) / num_iterations > percentage) {
                std::cout << "[ " << (int)(percentage * 100) << "% ]\n";
                percentage += percentage_step;
            }

            random_cycle.shuffle(generator);

            const auto [ts_cycle, ts_cycle_weight] =
                tsp::taboo_search(random_cycle, engine, max_taboo_list_size, max_iter);

            avg_cycle_weight += ts_cycle_weight;
            if (ts_cycle_weight < min_cycle_weight) {
                min_cycle_weight = ts_cycle_weight;
                best_cycle = ts_cycle;
            }
        }

        avg_cycle_weight /= num_iterations;

        std::cout << std::endl
                << "avg cycle weight = " << avg_cycle_weight << std::endl
                << "min cycle weight = " << min_cycle_weight << std::endl;

        if (parser.present("save")) {
            std::cout << "\nsaving results\n";
            std::ofstream file;
            file.open(parser.get("save"), std::ios_base::app);

            file << input_file_name << "," << graph.num_vertices() << ","
                << avg_cycle_weight << "," << min_cycle_weight;

            if (parser.get<bool>("experimental")) {
                file << "," << max_taboo_list_size
                     << "," << max_iter;
            }

            file << std::endl;
        }

        if (parser.present("best")) {
            std::cout << "\nsaving best found cycle\n";
            tsp::utility::save_cycle(best_cycle.get(), parser.get("best"));
        }
    }

    return 0;
}
