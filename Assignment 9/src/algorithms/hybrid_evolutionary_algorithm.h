#ifndef HYBRID_EVOLUTIONARY_ALGORITHM_H
#define HYBRID_EVOLUTIONARY_ALGORITHM_H

#include <vector>
#include "../core/TSPProblem.h"

/**
 * @brief Hybrid Evolutionary Algorithm.
 *
 * Currently a placeholder that runs for a specified time and returns a random solution.
 * Replaces Large Neighborhood Search.
 *
 * @param problem The TSP problem instance.
 * @param initial_solution The starting solution.
 * @param time_limit_ms Time limit in milliseconds.
 * @param use_ls Whether to use local search (unused in placeholder).
 * @param population_size The size of the population.
 * @param iterations Output parameter for number of iterations.
 * @return The best solution found (random in this placeholder).
 */
std::vector<int> hybrid_evolutionary_algorithm(const TSPProblem& problem, 
                                               const std::vector<int>& initial_solution, 
                                               int time_limit_ms, 
                                               bool use_ls, 
                                               int population_size,
                                               int& iterations);

#endif // HYBRID_EVOLUTIONARY_ALGORITHM_H
