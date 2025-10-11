#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <limits>
#include <numeric>
#include "assignment1.h"
#include "evaluation.h"

// Function to process a single instance of the problem
void process_instance(const std::string& filename) {
    std::cout << "=================================================" << std::endl;
    std::cout << "Processing instance: " << filename << std::endl;
    std::cout << "=================================================" << std::endl;

    std::ifstream file(filename);  // Open your CSV file
    if (!file.is_open()) {
        std::cerr << "Error: could not open file " << filename << "\n";
        return;
    }

    std::vector<PointData> data;
    std::string line;
    int current_id = 0; 

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string value;
        int temp_x, temp_y, temp_cost;
        int count = 0;
        bool error = false;

        while (std::getline(ss, value, ';')) {  // Use ';' as separator
            try {
                int num = std::stoi(value);
                if (count == 0) {
                    temp_x = num;
                } else if (count == 1) {
                    temp_y = num;
                } else if (count == 2) {
                    temp_cost = num;
                }
                count++;
            } catch (...) {
                error = true;
                break; // Stop processing this row on error
            }
        }

        // Check if exactly 3 values were read and no error occurred
        if (!error && count == 3) {
            // Create a PointData struct and populate it, assigning the current ID
            data.push_back({current_id, temp_x, temp_y, temp_cost});
            
            // Increment the ID counter for the next successful row
            current_id++; 
        }
    }

    file.close();

    if (data.empty()) {
        std::cerr << "Error: No data loaded from " << filename << std::endl;
        return;
    }

    // Calculate the distance matrix once
    auto distance_matrix = calculate_distance_matrix(data);
    const int num_nodes = data.size();
    const int num_runs = 200;

    // --- 1. Random Method ---
    std::cout << "\n--- Method: Random ---" << std::endl;
    double min_random = std::numeric_limits<double>::max();
    double max_random = std::numeric_limits<double>::min();
    double sum_random = 0.0;
    std::vector<int> best_random_solution;

    for (int i = 0; i < num_runs; ++i) {
        std::vector<int> solution = generate_random_solution(data);
        double score = evaluate_solution(solution, data, distance_matrix);
        if (score < min_random) {
            min_random = score;
            best_random_solution = solution;
        }
        if (score > max_random) {
            max_random = score;
        }
        sum_random += score;
    }

    std::cout << "Min value: " << min_random << std::endl;
    std::cout << "Max value: " << max_random << std::endl;
    std::cout << "Avg value: " << sum_random / num_runs << std::endl;
    std::cout << "Best solution: ";
    for (int id : best_random_solution) {
        std::cout << id << " ";
    }
    std::cout << std::endl;

    /*
    // --- 2. Nearest Neighbor (End) Method ---
    std::cout << "\n--- Method: Nearest Neighbor (End) ---" << std::endl;
    double min_nn_end = std::numeric_limits<double>::max();
    double max_nn_end = std::numeric_limits<double>::min();
    double sum_nn_end = 0.0;
    std::vector<int> best_nn_end_solution;

    for (int i = 0; i < num_runs; ++i) {
        int start_node_id = i % num_nodes;
        // The function would need to be modified to accept a starting node
        // std::vector<int> solution = generate_nearest_neighbor_end_solution(data, distance_matrix, start_node_id);
        // double score = evaluate_solution(solution, data, distance_matrix);
        // if (score < min_nn_end) {
        //     min_nn_end = score;
        //     best_nn_end_solution = solution;
        // }
        // if (score > max_nn_end) {
        //     max_nn_end = score;
        // }
        // sum_nn_end += score;
    }

    // std::cout << "Min value: " << min_nn_end << std::endl;
    // std::cout << "Max value: " << max_nn_end << std::endl;
    // std::cout << "Avg value: " << sum_nn_end / num_runs << std::endl;
    // std::cout << "Best solution: ";
    // for (int id : best_nn_end_solution) {
    //     std::cout << id << " ";
    // }
    // std::cout << std::endl;
    */

    /*
    // --- 3. Nearest Neighbor (All Positions) Method ---
    std::cout << "\n--- Method: Nearest Neighbor (All Positions) ---" << std::endl;
    double min_nn_all = std::numeric_limits<double>::max();
    double max_nn_all = std::numeric_limits<double>::min();
    double sum_nn_all = 0.0;
    std::vector<int> best_nn_all_solution;

    for (int i = 0; i < num_runs; ++i) {
        int start_node_id = i % num_nodes;
        // The function would need to be modified to accept a starting node
        // std::vector<int> solution = generate_nearest_neighbor_all_positions_solution(data, distance_matrix, start_node_id);
        // double score = evaluate_solution(solution, data, distance_matrix);
        // if (score < min_nn_all) {
        //     min_nn_all = score;
        //     best_nn_all_solution = solution;
        // }
        // if (score > max_nn_all) {
        //     max_nn_all = score;
        // }
        // sum_nn_all += score;
    }

    // std::cout << "Min value: " << min_nn_all << std::endl;
    // std::cout << "Max value: " << max_nn_all << std::endl;
    // std::cout << "Avg value: " << sum_nn_all / num_runs << std::endl;
    // std::cout << "Best solution: ";
    // for (int id : best_nn_all_solution) {
    //     std::cout << id << " ";
    // }
    // std::cout << std::endl;
    */

    /*
    // --- 4. Greedy Cycle Method ---
    std::cout << "\n--- Method: Greedy Cycle ---" << std::endl;
    double min_greedy = std::numeric_limits<double>::max();
    double max_greedy = std::numeric_limits<double>::min();
    double sum_greedy = 0.0;
    std::vector<int> best_greedy_solution;

    for (int i = 0; i < num_runs; ++i) {
        int start_node_id = i % num_nodes;
        // The function would need to be modified to accept a starting node
        // std::vector<int> solution = generate_greedy_cycle_solution(data, distance_matrix, start_node_id);
        // double score = evaluate_solution(solution, data, distance_matrix);
        // if (score < min_greedy) {
        //     min_greedy = score;
        //     best_greedy_solution = solution;
        // }
        // if (score > max_greedy) {
        //     max_greedy = score;
        // }
        // sum_greedy += score;
    }

    // std::cout << "Min value: " << min_greedy << std::endl;
    // std::cout << "Max value: " << max_greedy << std::endl;
    // std::cout << "Avg value: " << sum_greedy / num_runs << std::endl;
    // std::cout << "Best solution: ";
    // for (int id : best_greedy_solution) {
    //     std::cout << id << " ";
    // }
    // std::cout << std::endl;
    */
}

int main() {
    process_instance("data/TSPA.csv");
    process_instance("data/TSPB.csv");

    return 0;
}