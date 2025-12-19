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
#include "algorithms/crossovers/recombination_operator.h"
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
    struct Config {
        std::string name;
        std::vector<std::pair<CrossoverFunc, double>> crossovers;
    };

    std::vector<Config> configurations;
    std::vector<double> probs = {0.0, 0.25, 0.5, 0.75, 1.0};

    // Grid search
    for (double p : probs) {
        double p_preservation = 1.0 - p;

        // Configuration 1: CBGI vs Preservation
        // Note: CBGI corresponds to consensus_based_greedy_insertion
        std::vector<std::pair<CrossoverFunc, double>> crossovers_cbgi;
        if (p > 0) crossovers_cbgi.push_back({consensus_based_greedy_insertion, p});
        if (p_preservation > 0) crossovers_cbgi.push_back({preservation_crossover, p_preservation});
        
        std::string name_cbgi = "CBGI(" + std::to_string(p).substr(0,4) + ") + Pres(" + std::to_string(p_preservation).substr(0,4) + ")";
        if (!crossovers_cbgi.empty()) {
            configurations.push_back({name_cbgi, crossovers_cbgi});
        }

        // Configuration 2: Recombination vs Preservation
        std::vector<std::pair<CrossoverFunc, double>> crossovers_rec;
        if (p > 0) crossovers_rec.push_back({recombination_operator, p});
        if (p_preservation > 0) crossovers_rec.push_back({preservation_crossover, p_preservation});
        
        std::string name_rec = "Rec(" + std::to_string(p).substr(0,4) + ") + Pres(" + std::to_string(p_preservation).substr(0,4) + ")";
        if (!crossovers_rec.empty()) {
            configurations.push_back({name_rec, crossovers_rec});
        }
    }

    for (const auto& config : configurations) {
        auto generate_solution = [&](int i, int& iterations) {
            timer.start_stage(config.name);
            std::vector<int> starting_solution = random_solutions[i];
            // Uses defaults for mutation (0.3) and LNS (0.0)
            std::vector<int> result = hybrid_evolutionary_algorithm(problem_instance, starting_solution, time_limit_ms, 20, iterations, 0.3, 0.0, config.crossovers);
            timer.end_stage();
            return result;
        };
        run_and_print_results(config.name, problem_instance, num_runs, generate_solution, results_json, instance_name, timer);
    }
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
