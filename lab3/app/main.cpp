#include <tsp/cycle.hpp>
#include <tsp/utility.hpp>
#include <tsp/tsp.hpp>

#include <argument_parser.hpp>

#include <chrono>
#include <iostream>
#include <limits>
#include <random>
#include <fstream>



namespace {
constexpr uint8_t num_threads = 4u;
} // namespace




int main (int argc, char* argv[]) {
    ap::argument_parser parser;
    parser.program_description("Traveling Salesman Problem - Genetic Algorithm")
          .default_optional_arguments({ap::default_argument::optional::help});

    parser.add_optional_argument("file", "f")
          .required()
          .help("Path to a .tsp data file");
    parser.add_optional_argument("save", "s")
          .help("Path to an output file for tsp results");
    parser.add_optional_argument("best", "b")
          .help("Path to an output file for the best found cycle");
    parser.add_flag("experimental", "e")
          .help("enables saving additional data");

    parser.add_optional_argument<double>("max-iter", "mi")
          .default_value(0.3)
          .help("maximum iterations (percentage of graph size)");
    parser.add_optional_argument<std::size_t>("population-size", "ps")
          .default_value(100u);
    parser.add_flag<true>("sp-crossover", "spc")
          .help("use the single point crossover method flag");
    parser.add_optional_argument<double>("mutation-prob", "mp")
          .default_value(0.1)
          .help("probability of gene mutation");

    try {
        parser.parse_args(argc, argv);
    }
    catch (const ap::argument_parser_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << parser;
        std::exit(EXIT_FAILURE);
    }

    const auto input_file_name = parser.value("file");
    const auto points = tsp::utility::read_points(input_file_name);
    const auto graph = tsp::utility::problem_graph(points);

    uint32_t seed = (uint32_t)std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 generator(seed);

    tsp::point_cycle random_cycle(points);
    random_cycle.shuffle(generator);

    const auto max_iter = parser.value<double>("max-iter");
    const auto population_size = parser.value<std::size_t>("population-size");
    const auto use_sp_crossover = parser.value<bool>("sp-crossover");
    const auto crossover = use_sp_crossover ? tsp::genetic::single_point_crossover
                                            : tsp::genetic::simple_crossover;
    const auto mutation_prob = parser.value<double>("mutation-prob");

    const auto [best_cycle, min_cycle_weight, avg_cycle_weight] =
        tsp::threaded_genetic_algorithm<num_threads>(
            random_cycle, generator, max_iter, population_size, crossover, mutation_prob);

    std::cout << "min cycle weight = " << min_cycle_weight << std::endl;
    std::cout << "avg cycle weight = " << avg_cycle_weight << std::endl;

    if (parser.has_value("save")) {
        std::ofstream file;
        file.open(parser.value("save"), std::ios_base::app);

        file << input_file_name << "," << graph.num_vertices() << ","
             << avg_cycle_weight << "," << min_cycle_weight;

        if (parser.value<bool>("experimental"))
            file << std::boolalpha << "," << max_iter << "," << population_size
                 << "," << use_sp_crossover << "," << mutation_prob;

        file << std::endl;
    }

    if (parser.has_value("best"))
        tsp::utility::save_cycle(best_cycle.get(), parser.value("best"));

    return 0;
}

// -mi 50 -hc -ps 100 -mp 0.2
