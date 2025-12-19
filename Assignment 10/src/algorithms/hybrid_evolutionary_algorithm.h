#ifndef HYBRID_EVOLUTIONARY_ALGORITHM_H
#define HYBRID_EVOLUTIONARY_ALGORITHM_H

#include <vector>
#include <functional>
#include <utility>
#include "../core/TSPProblem.h"

using CrossoverFunc = std::function<std::vector<int>(const std::vector<int>&, const std::vector<int>&, const TSPProblem&)>;

/**
 * @brief Implements a hybrid evolutionary algorithm for the TSP problem.
 * 
 * This algorithm uses:
 * - An elite population of solutions
 * - Two recombination operators (randomly selected)
 * - Optional local search on offspring
 * - Steady-state replacement strategy
 * 
 * @param problem The TSP problem instance
 * @param initial_solution Initial solution (used as random seed basis)
 * @param time_limit_ms Time limit in milliseconds
 * @param use_ls Whether to apply local search to offspring (always applied to initial population)
 * @param population_size Size of the elite population
 * @param iterations Output parameter for number of iterations performed
 * @param crossovers List of crossover operators and their probabilities. If empty, defaults to 50/50 mix of recombination and preservation.
 * @return The best solution found
 */
std::vector<int> hybrid_evolutionary_algorithm(const TSPProblem& problem, 
                                               const std::vector<int>& initial_solution, 
                                               int time_limit_ms, 
                                               
                                               int population_size,
                                               int& iterations,
                                               const std::vector<std::pair<CrossoverFunc, double>>& crossovers = {});

#endif // HYBRID_EVOLUTIONARY_ALGORITHM_H