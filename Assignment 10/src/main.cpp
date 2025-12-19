#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <fstream>
#include <iomanip>

#include "core/data_loader.h"
#include "core/point_data.h"
#include "core/json.hpp"
#include "core/stagetimer.h"
#include "core/TSPProblem.h"
#include "core/experiment_runner.h"

#include "algorithms/random_solution.h"
#include "algorithms/hybrid_evolutionary_algorithm.h"
#include "algorithms/crossovers/consensus_based_greedy_insertion.h"
#include "algorithms/crossovers/preservation_crossover.h"

using json = nlohmann::json;

// Function to process a single instance of the problem
void process_instance(const std::string& filename, const std::string& instance_name, json& results_json, int time_limit_ms) {
    std::cout << "=================================================" << std::endl;
    std::cout << "Processing instance: " << filename << std::endl;
    std::cout << "=================================================" << std::endl;

    std::vector<PointData> data;
    if (!load_data(filename, data)) {
        return;
    }

    TSPProblem problem_instance = TSPProblem(data);
    const int num_runs = 10; // Changed to 20 as per assignment

    std::vector<std::vector<int>> random_solutions = {};
    for (int i = 0; i < num_runs; ++i) {
        random_solutions.push_back(generate_random_solution(data));
    }

    StageTimer timer;
    std::string method_name = "Hybrid EA (Pres+CBGI)";
    
    // Config: Preservation (0.5) + CBGI (0.5)
    std::vector<std::pair<CrossoverFunc, double>> crossovers = {
        {preservation_crossover, 0.5},
        {consensus_based_greedy_insertion, 0.5}
    };

    auto generate_solution = [&](int i, int& iterations) {
        timer.start_stage(method_name);
        std::vector<int> starting_solution = random_solutions[i];
        // Uses defaults for mutation (0.3), LNS (0.0), and tournament (0.8)
        std::vector<int> result = hybrid_evolutionary_algorithm(problem_instance, starting_solution, time_limit_ms, 20, iterations, 0.3, 0.0, 0.8, crossovers);
        timer.end_stage();
        return result;
    };
    run_and_print_results(method_name, problem_instance, num_runs, generate_solution, results_json, instance_name, timer);
}

int main(int argc, char* argv[]) {
    std::string json_filename;
    int time_limit_ms = -1;

    // Parse arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--json" && i + 1 < argc) {
            json_filename = argv[++i];
        } else if (arg == "--time" && i + 1 < argc) {
            try {
                time_limit_ms = std::stoi(argv[++i]);
            } catch (...) {
                std::cerr << "Invalid time limit specified." << std::endl;
                return 1;
            }
        }
    }

    if (time_limit_ms <= 0) {
        std::cerr << "Usage: " << argv[0] << " --time <ms> [--json <filename>]" << std::endl;
        std::cerr << "Please specify a positive time limit in milliseconds." << std::endl;
        return 1;
    }

    json results_json;

    process_instance("../data/TSPA.csv", "TSPA", results_json, time_limit_ms);
    process_instance("../data/TSPB.csv", "TSPB", results_json, time_limit_ms);

    if (!json_filename.empty()) {
        std::ofstream o(json_filename);
        o << std::setw(4) << results_json << std::endl;
        std::cout << "\nResults saved to " << json_filename << std::endl;
    }

    return 0;
}
