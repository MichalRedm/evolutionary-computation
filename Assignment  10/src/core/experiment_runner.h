#ifndef EXPERIMENT_RUNNER_H
#define EXPERIMENT_RUNNER_H

#include <string>
#include <vector>
#include <functional>
#include "TSPProblem.h"
#include "json.hpp"
#include "stagetimer.h"

using json = nlohmann::json;

/**
 * @brief Runs a solution generation method multiple times and prints/records results.
 * 
 * @param method_name Name of the method being run.
 * @param problem_instance The TSP problem instance.
 * @param num_runs Number of times to run the method.
 * @param generate_solution Function that generates a solution (takes run index and output iterations).
 * @param results_json JSON object to store results.
 * @param instance_name Name of the problem instance.
 * @param timer StageTimer to record runtimes.
 */
void run_and_print_results(
    const std::string& method_name,
    TSPProblem& problem_instance,
    int num_runs,
    const std::function<std::vector<int>(int, int&)>& generate_solution,
    json& results_json,
    const std::string& instance_name,
    StageTimer& timer
);

#endif // EXPERIMENT_RUNNER_H
