#ifndef GREEDY_WEIGHTED_REGRET_CONSTRUCTOR_H
#define GREEDY_WEIGHTED_REGRET_CONSTRUCTOR_H

#include <vector>
#include "../../core/TSPProblem.h"

/**
 * @brief Greedy Weighted Regret Constructor.
 * Builds or repairs a solution by iteratively inserting nodes based on a weighted 
 * objective combining 2-regret and insertion cost. It supports a Random Candidate List (RCL)
 * strategy to introduce diversity.
 * @param problem The TSP problem instance.
 * @param random_candidate_list_length The number of top candidates to choose from randomly (default 1).
 * @param partial_solution The partial solution to start with (default empty).
 * @return A complete solution with 50% of nodes.
 */
std::vector<int> greedy_weighted_regret_constructor(
    const TSPProblem& problem, 
    int random_candidate_list_length = 1, 
    const std::vector<int>& partial_solution = {}
);

#endif // GREEDY_WEIGHTED_REGRET_CONSTRUCTOR_H