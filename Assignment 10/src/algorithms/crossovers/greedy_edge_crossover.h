#ifndef GREEDY_EDGE_CROSSOVER_H
#define GREEDY_EDGE_CROSSOVER_H

#include <vector>
#include "../../core/TSPProblem.h"

/**
 * @brief Greedy Edge Crossover
 * 
 * Constructs an offspring by greedily selecting the shortest edge available 
 * from either parent's adjacency list, falling back to nearest global neighbor if needed.
 * 
 * @param parent1 First parent solution
 * @param parent2 Second parent solution
 * @param problem TSP Problem instance
 * @return Offspring solution
 */
std::vector<int> greedy_edge_crossover(const std::vector<int>& parent1, const std::vector<int>& parent2, const TSPProblem& problem);

#endif // GREEDY_EDGE_CROSSOVER_H
