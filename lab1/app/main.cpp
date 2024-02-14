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

    parser
        .add_argument("-f", "--file")
        .required()
        .help("Path to a .tsp data file");
    parser
        .add_argument("-m", "--mode")
        .action([] (const std::string& value) {
            static const std::vector <std::string> choices = { "rst", "rpc", "rnf" };
            if (std::find(choices.begin(), choices.end(), value) != choices.end())
                return value;
            return std::string("rst");
        })
        .default_value("rst")
        .help("program mode: [rst - random start vertex] "
              "[rpc - random permutation cycle] [f - random neighbourhood (fast)]");
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
    const auto [mst, mst_weight] = graph.mst();

    std::cout << "mst generated\n";

    uint32_t num_iterations = 1;
    uint32_t seed = (uint32_t)std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 generator(seed);
    std::uniform_int_distribution<uint32_t> uniform(0, graph.num_vertices() - 1);

    tsp::point_distance_type avg_cycle_weight = 0;
    std::size_t avg_improvements = 0;

    tsp::point_distance_type min_cycle_weight =
        std::numeric_limits<tsp::point_distance_type>::max();
    tsp::point_cycle best_cycle;

    const auto mode = parser.get("mode");

    if (mode == "rst") {
        num_iterations = std::min((uint32_t)std::sqrt(graph.num_vertices()), (uint32_t)100);

        for (uint32_t i = 0; i < num_iterations; i++) {
            if ((i + 1) % 10 == 0)
                std::cout << "it: " << (i + 1) << "\n";

            const auto start_vertex = uniform(generator);

            const auto cycle =
                tsp::point_cycle_builder::from_mst<uint32_t>(mst, points, start_vertex);
            const auto [ls_cycle, ls_cycle_weight, ls_improvements] = tsp::local_search(cycle);

            avg_cycle_weight += ls_cycle_weight;
            avg_improvements += ls_improvements;

            if (ls_cycle_weight < min_cycle_weight) {
                min_cycle_weight = ls_cycle_weight;
                best_cycle = ls_cycle;
            }
        }
    }
    else if (mode == "rpc") {
        num_iterations = graph.num_vertices() < 1000 ? graph.num_vertices() : 100;
        tsp::point_cycle random_cycle(points);

        for (uint32_t i = 0; i < num_iterations; i++) {
            if ((i + 1) % 10 == 0)
                std::cout << "it: " << (i + 1) << "\n";

            random_cycle.shuffle(generator);

            const auto [ls_cycle, ls_cycle_weight, ls_improvements] = tsp::local_search(random_cycle);

            avg_cycle_weight += ls_cycle_weight;
            avg_improvements += ls_improvements;

            if (ls_cycle_weight < min_cycle_weight) {
                min_cycle_weight = ls_cycle_weight;
                best_cycle = ls_cycle;
            }
        }
    }
    else {
        num_iterations = graph.num_vertices() < 1000 ? graph.num_vertices() : 100;
        std::default_random_engine engine{seed};

        for (uint32_t i = 0; i < num_iterations; i++) {
            if ((i + 1) % 10 == 0)
                std::cout << "it: " << (i + 1) << "\n";

            const auto start_vertex = uniform(generator);

            const auto cycle =
                tsp::point_cycle_builder::from_mst<uint32_t>(mst, points, start_vertex);
            const auto [ls_cycle, ls_cycle_weight, ls_improvements] =
                tsp::local_search_random_neighbourhood(cycle, engine);

            avg_cycle_weight += ls_cycle_weight;
            avg_improvements += ls_improvements;

            if (ls_cycle_weight < min_cycle_weight) {
                min_cycle_weight = ls_cycle_weight;
                best_cycle = ls_cycle;
            }
        }
    }

    avg_cycle_weight /= num_iterations;
    avg_improvements /= num_iterations;

    std::cout << "mst weight = " << mst_weight << std::endl
              << "avg cycle weight = " << avg_cycle_weight << std::endl
              << "min cycle weight = " << min_cycle_weight << std::endl
              << "avg #improvements = " << avg_improvements << std::endl;

    if (parser.present("save")) {
        std::ofstream file;
        file.open(parser.get("save"), std::ios_base::app);

        file << input_file_name << "," << graph.num_vertices() << "," << mst_weight << ","
             << avg_cycle_weight << "," << min_cycle_weight << "," << avg_improvements << std::endl;
    }

    if (parser.present("best")) {
        std::cout << "saving best found cycle\n";
        tsp::utility::save_cycle(best_cycle.get(), parser.get("best"));
    }

    return 0;
}
