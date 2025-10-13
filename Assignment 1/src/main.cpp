#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <numeric>
#include <functional>
#include "core/evaluation.h"
#include "core/data_loader.h"
#include "algorithms/random_solution.h"
#include "algorithms/nearest_neighbor_end.h"
#include "algorithms/nearest_neighbor_all_positions.h"
#include "algorithms/greedy_cycle.h"
#include "core/point_data.h"

// Helper function to run a solution generation method and print results
void run_and_print_results(
    const std::string& method_name,
    const std::vector<PointData>& data,
    const std::vector<std::vector<int>>& distance_matrix,
    int num_runs,
    const std::function<std::vector<int>(int)>& generate_solution
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

    // --- 1. Random Method ---
    run_and_print_results("Random", data, distance_matrix, num_runs,
        [&](int ) {
            return generate_random_solution(data);
        }
    );

    // --- 2. Nearest Neighbor (End) Method ---
    run_and_print_results("Nearest Neighbor (End)", data, distance_matrix, num_runs,
        [&](int i) -> std::vector<int> {
            int start_node_id = i % num_nodes;
            return generate_nearest_neighbor_end_solution(data, distance_matrix, start_node_id);
        }
    );

    // --- 3. Nearest Neighbor (All Positions) Method ---
    run_and_print_results("Nearest Neighbor (All Positions)", data, distance_matrix, num_runs,
        [&](int i) -> std::vector<int> {
            int start_node_id = i % num_nodes;
            return generate_nearest_neighbor_all_positions_solution(data, distance_matrix, start_node_id);
        }
    );
    
    // --- 4. Greedy Cycle Method ---
    run_and_print_results("Greedy Cycle", data, distance_matrix, num_runs,
        [&](int i) {
            int start_node_id = i % num_nodes;
            return generate_greedy_cycle_solution(data, distance_matrix, start_node_id);
        }
    );
}

int main() {
    process_instance("../data/TSPA.csv");
    process_instance("../data/TSPB.csv");

    return 0;
}
