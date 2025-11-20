#include <vector>
#include <algorithm>
#include <random>
#include <stdexcept>
#include <limits>
#include <set>
#include <map>
#include "random_solution.h"
#include "../core/stagetimer.h"
#include "../core/evaluation.h"
#include "inter_node_exchange.h"
#include "intra_edge_exchange.h"
#include "neighborhood_utils.h"
#include "local_search.h"
#include "simple_local_search.h"

/**
 * @brief Performs a multiple start local search
 *
 * This functions runs local search started from a randomly generated solution
 * multiple times and returns the best found solution.
 *
 * @param data The data describind our TSPProblem instance
 * @param problem_instance The TSPProblem instance containing points and distance matrix.
 * @param T The search type (STEEPEST or GREEDY).
 * @param num_runs The amount of times we will run local search.
 * @param timer A StageTimer object to record performance metrics.
 * @return The best found solution.
 */
std::vector<int> multi_start_local_search(
    std::vector<PointData> data,
    TSPProblem& problem_instance,
    SearchType T,
    int num_runs,
    StageTimer& timer
){
    std::vector<int> best_solution;
    double best_score = std::numeric_limits<double>::max();

    StageTimer dummy_timer;

    std::vector<std::vector<int>> random_solutions = {};
    for (int i = 0; i < num_runs; ++i) {
        random_solutions.push_back(generate_random_solution(data));
    }
    
    timer.start_stage("multi_start");

    // Make num_runs amount of attempts and pick best solution
    for (int i = 0; i < num_runs; ++i){
        // Create a single solution
        std::vector<int> solution = simple_local_search(problem_instance, random_solutions[i], T, dummy_timer);

        // Evaluate the solution
        double score = evaluate_solution(solution, problem_instance);
        if (best_score > score){
            best_score = score;
            best_solution = solution;
        }
    }

    timer.end_stage();

    return best_solution;

}