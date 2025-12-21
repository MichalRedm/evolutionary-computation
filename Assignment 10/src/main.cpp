#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "core/data_loader.h"
#include "core/point_data.h"
#include "core/json.hpp"
#include "core/stagetimer.h"
#include "core/TSPProblem.h"
#include "core/experiment_runner.h"

#include "algorithms/constructors/random_solution.h"
#include "algorithms/constructors/greedy_weighted_regret_constructor.h"
#include "algorithms/hybrid_evolutionary_algorithm.h"
#include "algorithms/crossovers/preservation_crossover.h"
#include "algorithms/crossovers/recombination_operator.h"
#include "algorithms/crossovers/greedy_edge_crossover.h"

#include <map>

using json = nlohmann::json;

// Generic Grid Search Structure
struct GridDimension {
    std::string name;
    std::vector<double> values;
};

// Recursive helper to generate all combinations
void generate_grid_configurations(
    const std::vector<GridDimension>& dimensions,
    size_t dim_index,
    std::map<std::string, double>& current_config,
    std::vector<std::map<std::string, double>>& all_configs
) {
    if (dim_index == dimensions.size()) {
        all_configs.push_back(current_config);
        return;
    }

    const auto& dim = dimensions[dim_index];
    for (double val : dim.values) {
        current_config[dim.name] = val;
        generate_grid_configurations(dimensions, dim_index + 1, current_config, all_configs);
    }
}

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
    const int num_runs = 20; // Changed to 20 as per assignment

    StageTimer timer;
    
    // Define the grid dimensions
    // To add a new parameter, simply add a new GridDimension here!
    std::vector<GridDimension> grid_dimensions = {
        {"mutation_probability", {0.6}},
        {"lns_probability", {0.0}},
        {"tournament_probability", {0.0}},
        {"adaptive_learning_rate", {0.03}},
        {"adaptive_min_weight", {0.1}},
        {"use_adaptive_crossover", {0.0}},
        {"mutation_strength", {10.0}},
        {"use_adaptive_mutation", {0.0}},
        {"stagnation_step", {100.0}},
        {"k_candidates", {-1.0}},
        {"max_stagnation_iterations", {-1.0}},
        {"initial_solution_builder", {0.0}}, // 0: random, 1: greedy_weighted_regret
        {"regret_k_candidates", {3.0}}     // for greedy regret
    };

    // Generate all configurations recursively
    std::vector<std::map<std::string, double>> configurations;
    std::map<std::string, double> current_config;
    generate_grid_configurations(grid_dimensions, 0, current_config, configurations);

    // Fixed crossover configuration for this grid search
    std::vector<std::pair<CrossoverFunc, double>> crossovers = {
        {preservation_crossover, 0.3},
        {recombination_operator, 0.3},
        {greedy_edge_crossover, 0.4},
        // {cost_priority_crossover, 0.25},
        // {consensus_based_greedy_insertion, 0.25}
    };

    // Iterate over all generated configurations
    for (const auto& config : configurations) {
        std::stringstream ss;
        ss << "HEA";
        
        // Build method name mainly from parameters that have multiple values
        for (const auto& dim : grid_dimensions) {
             if (dim.values.size() > 1) {
                 ss << " " << dim.name << "=" << config.at(dim.name);
             }
        }
        // If no varying parameters, just add mutation prob as a baseline identifier
        if (ss.str() == "HEA") {
            ss << " Mut=" << config.at("mutation_probability");
        }

        std::string method_name = ss.str();

        auto generate_solution = [&](int i, int& iterations) {
            timer.start_stage(method_name);
            
            // Extract parameters from map
            double mut_prob = config.at("mutation_probability");
            double lns_prob = config.at("lns_probability");
            double tourn_prob = config.at("tournament_probability");
            bool use_adaptive = (config.at("use_adaptive_crossover") > 0.5);
            double lr = config.at("adaptive_learning_rate");
            double min_w = config.at("adaptive_min_weight");
            int mut_str = (int)config.at("mutation_strength");
            bool use_adaptive_mut = (config.at("use_adaptive_mutation") > 0.5);
            int stag_step = (int)config.at("stagnation_step");
            int k = (int)config.at("k_candidates");
            int max_stag_iter = (int)config.at("max_stagnation_iterations");
            
            int builder_type = (int)config.at("initial_solution_builder");
            int regret_k = (int)config.at("regret_k_candidates");

            SolutionConstructor constructor;
            if (builder_type == 1) {
                // Greedy Weighted Regret
                constructor = [regret_k](const TSPProblem& p) {
                    return greedy_weighted_regret_constructor(p, regret_k, {});
                };
            } else {
                // Random (Default)
                constructor = [](const TSPProblem& p) {
                    return generate_random_solution(p.get_points());
                };
            }

            std::vector<int> result = hybrid_evolutionary_algorithm(
                problem_instance, 
                constructor, 
                time_limit_ms, 
                20, // population_size
                iterations, 
                mut_prob, 
                lns_prob, 
                tourn_prob,
                crossovers, 
                use_adaptive,
                lr,
                min_w,
                mut_str,
                use_adaptive_mut,
                stag_step,
                k,
                max_stag_iter
            );
            timer.end_stage();
            return result;
        };
        run_and_print_results(method_name, problem_instance, num_runs, generate_solution, results_json, instance_name, timer);
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
