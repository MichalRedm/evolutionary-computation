#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <numeric>
#include <functional>
#include <fstream>
#include "core/evaluation.h"
#include "core/data_loader.h"
#include "algorithms/greedy_2_regret.h"
#include "algorithms/greedy_with_weighted_sum.h"
#include "algorithms/nearest_neighbour_weighted_sum.h"
#include "algorithms/nearest_neighbour_2_regret.h"
#include "core/point_data.h"
#include "core/json.hpp"

using json = nlohmann::json;

// Helper function to run a solution generation method and print results
void run_and_print_results(
    const std::string& method_name,
    const std::vector<PointData>& data,
    const std::vector<std::vector<int>>& distance_matrix,
    int num_runs,
    const std::function<std::vector<int>(int)>& generate_solution,
    json& results_json,
    const std::string& instance_name
) {
    std::cout << "\n--- Method: " << method_name << " ---" << std::endl;
    double min_score = std::numeric_limits<double>::max();
    double max_score = std::numeric_limits<double>::min();
    double sum_score = 0.0;
    std::vector<int> best_solution;
    int solutions_count = 0;

    for (int i = 0; i < num_runs; ++i) {
        std::vector<int> solution = generate_solution(i);
        if (solution.empty()) {
            continue;
        }
        solutions_count++;
        double score = evaluate_solution(solution, data, distance_matrix);
        if (score < min_score) {
            min_score = score;
            best_solution = solution;
        }
        if (score > max_score) {
            max_score = score;
        }
        sum_score += score;
    }

    if (solutions_count == 0) {
        std::cout << "No solutions were generated for this method." << std::endl;
        return;
    }

    double avg_score = sum_score / solutions_count;

    std::cout << "Min value: " << min_score << std::endl;
    std::cout << "Max value: " << max_score << std::endl;
    std::cout << "Avg value: " << avg_score << std::endl;
    std::cout << "Best solution: ";
    for (int id : best_solution) {
        std::cout << id << " ";
    }
    std::cout << std::endl;

    // Add results to JSON object
    results_json[instance_name][method_name]["min_value"] = min_score;
    results_json[instance_name][method_name]["max_value"] = max_score;
    results_json[instance_name][method_name]["avg_value"] = avg_score;
    results_json[instance_name][method_name]["best_solution"] = best_solution;
}

// Function to process a single instance of the problem
void process_instance(const std::string& filename, const std::string& instance_name, json& results_json) {
    std::cout << "=================================================" << std::endl;
    std::cout << "Processing instance: " << filename << std::endl;
    std::cout << "=================================================" << std::endl;

    std::vector<PointData> data;
    if (!load_data(filename, data)) {
        return;
    }

    auto distance_matrix = calculate_distance_matrix(data);
    const int num_nodes = data.size();
    const int num_runs = 200;

    // --- 1. Greedy 2-Regret Method ---
    run_and_print_results("Greedy 2-Regret Method", data, distance_matrix, num_runs,
        [&](int i) -> std::vector<int> {
            int start_node_id = i % num_nodes;
            return generate_greedy_2_regret_solution(data, distance_matrix, start_node_id);
        },
        results_json, instance_name
    );

    // --- 2. Greedy with Weighted Sum Method ---
    run_and_print_results("Greedy with Weighted Sum Method", data, distance_matrix, num_runs,
        [&](int i) -> std::vector<int> {
            int start_node_id = i % num_nodes;
            return generate_with_weighted_sum_solution(data, distance_matrix, start_node_id);
        },
        results_json, instance_name
    );

    // --- 3. Nearest Neighbour 2-Regret Method ---
    run_and_print_results("Nearest Neighbour 2-Regret Method", data, distance_matrix, num_runs,
        [&](int i) -> std::vector<int> {
            int start_node_id = i % num_nodes;
            return nearest_neighbour_2_regret(data, distance_matrix, start_node_id);
        },
        results_json, instance_name
    );

    // --- 4. Nearest Neighbour with Weighted Sum Method ---
    run_and_print_results("Nearest neighbour with Weighted Sum Method", data, distance_matrix, num_runs,
        [&](int i) -> std::vector<int> {
            int start_node_id = i % num_nodes;
            return nearest_neighbour_weighted_sum(data, distance_matrix, start_node_id);
        },
        results_json, instance_name
    );
}

int main(int argc, char* argv[]) {
    std::string json_filename;
    if (argc == 3 && std::string(argv[1]) == "--json") {
        json_filename = argv[2];
    }

    json results_json;

    process_instance("../data/TSPA.csv", "TSPA", results_json);
    process_instance("../data/TSPB.csv", "TSPB", results_json);

    if (!json_filename.empty()) {
        std::ofstream o(json_filename);
        o << std::setw(4) << results_json << std::endl;
        std::cout << "\nResults saved to " << json_filename << std::endl;
    }

    return 0;
}
