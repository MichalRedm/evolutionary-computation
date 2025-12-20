#ifndef COST_WEIGHTED_EDGE_RECOMBINATION_H
#define COST_WEIGHTED_EDGE_RECOMBINATION_H

#include <vector>
#include "../../core/TSPProblem.h"

/**
 * @brief Cost-Weighted Edge Recombination Crossover (CWER)
 * 
 * Combines the structure of Edge Recombination (preserving adjacency)
 * with a cost-greedy heuristic.
 * 
 * Prio 1: Common Edges
 * Prio 2: Greediest neighbor transition (Node Cost + Edge Weight)
 * Prio 3: Random/Greedy Rescue
 */
std::vector<int> cost_weighted_edge_recombination(const std::vector<int>& parent1, const std::vector<int>& parent2, const TSPProblem& problem);

#endif // COST_WEIGHTED_EDGE_RECOMBINATION_H
