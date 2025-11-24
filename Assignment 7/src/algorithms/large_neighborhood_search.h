#ifndef LARGE_NEIGHBORHOOD_SEARCH_H
#define LARGE_NEIGHBORHOOD_SEARCH_H

#include "../core/TSPProblem.h"
#include "../core/stagetimer.h"
#include <vector>

/**
 * @brief Performs Large Neighborhood Search (LNS) to improve a solution.
 *
 * @param problem_instance The TSPProblem instance.
 * @param starting_solution The initial solution.
 * @param time_limit_ms Time limit in milliseconds.
 * @param use_local_search Whether to apply local search after repair.
 * @param iterations_out Output parameter to return the number of iterations performed.
 * @return The best solution found.
 */
std::vector<int> large_neighborhood_search(
    TSPProblem& problem_instance,
    std::vector<int> starting_solution,
    int time_limit_ms,
    bool use_local_search,
    int& iterations_out
);

#endif // LARGE_NEIGHBORHOOD_SEARCH_H
