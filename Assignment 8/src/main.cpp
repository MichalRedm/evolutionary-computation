#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include "core/data_loader.h"
#include "core/point_data.h"
#include "core/TSPProblem.h"
#include "core/stagetimer.h"
#include "core/evaluation.h"

#include "algorithms/random_solution.h"
#include "algorithms/local_search.h"

void process_instance(const std::string& input_filename, const std::string& output_filename) {
    std::cout << "Processing " << input_filename << " -> " << output_filename << "..." << std::endl;

    std::vector<PointData> data;
    if (!load_data(input_filename, data)) {
        std::cerr << "Failed to load data from " << input_filename << std::endl;
        return;
    }

    TSPProblem problem(data);
    StageTimer timer; // Timer is required by local_search signature

    std::ofstream csv_file(output_filename);
    if (!csv_file.is_open()) {
        std::cerr << "Error: Could not open " << output_filename << " for writing." << std::endl;
        return;
    }

    for (int i = 0; i < 1000; ++i) {
        std::vector<int> initial_solution = generate_random_solution(data);
        
        // Use Greedy Local Search
        std::vector<int> improved_solution = local_search(problem, initial_solution, SearchType::GREEDY, timer);
        

        int cost = static_cast<int>(evaluate_solution(improved_solution, problem));
        
        // Write to CSV: node_id_1,node_id_2,...,node_id_N,cost
        for (size_t j = 0; j < improved_solution.size(); ++j) {
            csv_file << improved_solution[j] << ",";
        }
        csv_file << cost << "\n";
    }
    
    csv_file.close();
    std::cout << "Finished " << output_filename << std::endl;
}

int main() {
    process_instance("../data/TSPA.csv", "TSPA_solutions.csv");
    process_instance("../data/TSPB.csv", "TSPB_solutions.csv");

    std::cout << "All solutions generated." << std::endl;

    return 0;
}
