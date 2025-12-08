#include "experiment_runner.h"
#include "evaluation.h"
#include <iostream>
#include <limits>
#include <algorithm>

void run_and_print_results(
    const std::string& method_name,
    TSPProblem& problem_instance,
    int num_runs,
    const std::function<std::vector<int>(int, int&)>& generate_solution,
    json& results_json,
    const std::string& instance_name,
    StageTimer& timer
) {
    std::cout << "\n--- Method: " << method_name << " ---" << std::endl;
    double min_score = std::numeric_limits<double>::max();
    double max_score = std::numeric_limits<double>::min();
    double sum_score = 0.0;
    long long sum_iterations = 0;
    std::vector<int> best_solution;
    int solutions_count = 0;

    for (int i = 0; i < num_runs; ++i) {
        int iterations = 0;
        std::vector<int> solution = generate_solution(i, iterations);
        if (solution.empty()) {
            continue;
        }
        solutions_count++;
        sum_iterations += iterations;
        double score = evaluate_solution(solution, problem_instance);
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
    double avg_iterations = (double)sum_iterations / solutions_count;

    // Normalize the best solution to start with node 0
    if (!best_solution.empty()) {
        auto it = std::find(best_solution.begin(), best_solution.end(), 0);
        if (it != best_solution.end()) {
            std::rotate(best_solution.begin(), it, best_solution.end());
        }
    }

    std::cout << "Min value: " << min_score << std::endl;
    std::cout << "Max value: " << max_score << std::endl;
    std::cout << "Avg value: " << avg_score << std::endl;
    std::cout << "Avg iterations: " << avg_iterations << std::endl;
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
    results_json[instance_name][method_name]["avg_iterations"] = avg_iterations;
    results_json[instance_name][method_name]["best_solution"] = best_solution;
    results_json[instance_name][method_name]["avg_runtimes_ms"] = avg_runtimes;
}
