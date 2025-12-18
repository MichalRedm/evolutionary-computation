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
 * @param iteration_limit Amount of iterations to perform
 * @param use_local_search Whether to apply local search after repair.
 * @return The best solution found.
 */
std::vector<int> large_neighborhood_search(
    TSPProblem& problem_instance,
    std::vector<int> starting_solution,
    int iteration_limit,
    bool use_local_search
);

#endif // LARGE_NEIGHBORHOOD_SEARCH_H
