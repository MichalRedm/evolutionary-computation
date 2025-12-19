#ifndef DESTROY_OPERATOR_H
#define DESTROY_OPERATOR_H

#include <vector>
#include <random>
#include "../core/TSPProblem.h"

/**
 * @brief Destroy operator: Removes subpaths based on edge costs.
 * 
 * @param solution The current solution.
 * @param problem The TSP problem instance.
 * @param rng Random number generator.
 * @return A partial solution with some segments removed.
 */
std::vector<int> destroy_solution(const std::vector<int>& solution, const TSPProblem& problem, std::mt19937& rng);

#endif // DESTROY_OPERATOR_H
