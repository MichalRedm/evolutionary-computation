#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <numeric>
#include <functional>
#include "core/evaluation.h"
#include "core/data_loader.h"
#include "algorithms/greedy_2_regret.h"
#include "algorithms/greedy_with_weighted_sum.h"
#include "core/point_data.h"

// Helper function to run a solution generation method and print results
void run_and_print_results(
    const std::string& method_name,
    const std::vector<PointData>& data,
    const std::vector<std::vector<int>>& distance_matrix,
    int num_runs,
    const std::function<std::tuple<std::vector<int>, int>(int)>& generate_solution
) {
    std::cout << "\n--- Method: " << method_name << " ---" << std::endl;
    double min_score = std::numeric_limits<double>::max();
    double max_score = std::numeric_limits<double>::min();
    double sum_score = 0.0;
    std::vector<int> best_solution;
    int solutions_count = 0;

    for (int i = 0; i < num_runs; ++i) {
        auto [solution, score_int] = generate_solution(i);
        double score = static_cast<double>(score_int);
        if (solution.empty()) {
            continue;
        }
        solutions_count++;
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

    std::cout << "Min value: " << min_score << std::endl;
    std::cout << "Max value: " << max_score << std::endl;
    std::cout << "Avg value: " << sum_score / solutions_count << std::endl;
    std::cout << "Best solution: ";
    for (int id : best_solution) {
        std::cout << id << " ";
    }
    std::cout << std::endl;
}

// Function to process a single instance of the problem
void process_instance(const std::string& filename) {
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
        [&](int i) -> std::tuple<std::vector<int>, int> {
            int start_node_id = i % num_nodes;
            return generate_greedy_2_regret_solution(data, distance_matrix, start_node_id);
        }
    );

    // --- 2. Greedy with Weighted Sum Method ---
    run_and_print_results("Greedy with Weighted Sum Method", data, distance_matrix, num_runs,
        [&](int i) -> std::tuple<std::vector<int>, int> {
            int start_node_id = i % num_nodes;
            return generate_with_weighted_sum_solution(data, distance_matrix, start_node_id);
        }
    );
}

int main() {
    process_instance("../data/TSPA.csv");
    process_instance("../data/TSPB.csv");

    return 0;
}
