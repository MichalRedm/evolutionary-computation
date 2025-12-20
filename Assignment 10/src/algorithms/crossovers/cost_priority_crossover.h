#ifndef COST_PRIORITY_CROSSOVER_H
#define COST_PRIORITY_CROSSOVER_H

#include <vector>
#include "../../core/TSPProblem.h"

/**
 * @brief Cost Priority Crossover
 * 
 * Selects nodes based on their cost (cheapest first) from both parents,
 * then constructs the offspring by preserving the relative order from parents.
 * 
 * @param parent1 First parent solution
 * @param parent2 Second parent solution
 * @param problem TSP Problem instance
 * @return Offspring solution
 */
std::vector<int> cost_priority_crossover(const std::vector<int>& parent1, const std::vector<int>& parent2, const TSPProblem& problem);

#endif // COST_PRIORITY_CROSSOVER_H
