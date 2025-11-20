#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <numeric>
#include <functional>
#include <fstream>
#include <algorithm>
#include "core/evaluation.h"
#include "core/data_loader.h"
#include "algorithms/local_search.h"
#include "algorithms/simple_local_search.h"
#include "core/point_data.h"
#include "core/json.hpp"
#include "core/stagetimer.h"
#include "algorithms/random_solution.h"
#include "core/TSPProblem.h"
#include "algorithms/multi_start_local_search.h"
#include "algorithms/iterative_local_search.h"

using json = nlohmann::json;

// Helper function to run a solution generation method and print results
void run_and_print_results(
    const std::string& method_name,
    TSPProblem& problem_instance,
    int num_runs,
    const std::function<std::map<std::string, std::vector<int>>()>& generate_solution,
    json& results_json,
    const std::string& instance_name,
    StageTimer& timer
) {
    std::cout << "\n--- Method: " << method_name << " ---" << std::endl;
    double min_score = std::numeric_limits<double>::max();
    double max_score = std::numeric_limits<double>::min();
    double sum_score = 0.0;
    int min_searches = std::numeric_limits<int>::max();
    int max_searches = std::numeric_limits<int>::min();
    double sum_searches = 0.0;
    std::vector<int> best_solution;
    int solutions_count = 0;
    std::map<std::string, std::vector<int>> generate_solution_output;

    for (int i = 0; i < num_runs; ++i) {
        generate_solution_output = generate_solution();
        std::vector<int> solution = generate_solution_output["solution"];
        if (solution.empty()) {
            continue;
        }
        solutions_count++;
        double score = evaluate_solution(solution, problem_instance);
        if (score < min_score) {
            min_score = score;
            best_solution = solution;
        }
        if (score > max_score) {
            max_score = score;
        }
        sum_score += score;

        // If num_searches present calculate stats for it
        if (generate_solution_output.find("num_searches") != generate_solution_output.end()){
            int searches = generate_solution_output["num_searches"][0];
            if (searches < min_searches) {
                min_searches = searches;
            }
            if (searches > max_searches) {
                max_searches = searches;
            }
            sum_searches += searches;    
        }
    }

    if (solutions_count == 0) {
        std::cout << "No solutions were generated for this method." << std::endl;
        return;
    }

    double avg_score = sum_score / solutions_count;
    

    // Normalize the best solution to start with node 0
    if (!best_solution.empty()) {
        auto it = std::find(best_solution.begin(), best_solution.end(), 0);
        if (it != best_solution.end()) {
            std::rotate(best_solution.begin(), it, best_solution.end());
        }
    }

    double avg_searches;

    std::cout << "Min value: " << min_score << std::endl;
    std::cout << "Max value: " << max_score << std::endl;
    std::cout << "Avg value: " << avg_score << std::endl;
    if (generate_solution_output.find("num_searches") != generate_solution_output.end()){
        avg_searches = sum_searches / solutions_count;
        std::cout << "Min searches: " << min_searches << std::endl;
        std::cout << "Max searches: " << max_searches << std::endl;
        std::cout << "Avg searches: " << avg_searches << std::endl;
    }
    std::cout << "Best solution: ";
    for (int id : best_solution) {
        std::cout << id << " ";
    }
    std::cout << std::endl;

    auto avg_runtimes = timer.get_avg_runtimes();
    std::cout << "Average runtimes (ms):" << std::endl;
    for (const auto& pair : avg_runtimes) {
        std::cout << "  - " << pair.first << ": " << pair.second << " ms" << std::endl;
    }

    // Add results to JSON object
    results_json[instance_name][method_name]["min_value"] = min_score;
    results_json[instance_name][method_name]["max_value"] = max_score;
    results_json[instance_name][method_name]["avg_value"] = avg_score;
    results_json[instance_name][method_name]["best_solution"] = best_solution;
    results_json[instance_name][method_name]["avg_runtimes_ms"] = avg_runtimes;

    if (generate_solution_output.find("num_searches") != generate_solution_output.end()){
        results_json[instance_name][method_name]["min_searches"] = min_searches;
        results_json[instance_name][method_name]["max_searches"] = max_searches;
        results_json[instance_name][method_name]["avg_searches"] = avg_searches;
    }
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

    TSPProblem problem_instance = TSPProblem(data);
    const int num_runs = 20;
    const int num_multi_runs = 200; // How many runs the multi start method will perform internally
    const SearchType T = SearchType::STEEPEST;

    // Multi start and iterative are not in a for and manually typed one after another as 
    // the stopping condidtion of Iterative depends on the time of Multi start

    // Multi start
    std::string method_name = "LS_Multi_start";

    StageTimer timer_multi;
    auto generate_solution = [&]() {
        std::vector<int> solution = multi_start_local_search(data, problem_instance, T, num_multi_runs, timer_multi);
        std::map<std::string, std::vector<int>> output = {{"solution", solution}};
        return output;
    };

    run_and_print_results(method_name, problem_instance, num_runs, generate_solution, results_json, instance_name, timer_multi);

    // Iterative
    method_name = "LS_Iterative";
    
    std::map<std::string, double> avg_runtimes = timer_multi.get_avg_runtimes();
    double stopping_condition = avg_runtimes["multi_start"];

    StageTimer timer_iterative;
    auto generate_solution2 = [&]() {
        return iterative_local_search(data, problem_instance, T, stopping_condition, timer_iterative);
    };

    run_and_print_results(method_name, problem_instance, num_runs, generate_solution2, results_json, instance_name, timer_iterative);
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
