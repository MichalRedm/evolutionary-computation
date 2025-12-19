#ifndef CONSENSUS_BASED_GREEDY_INSERTION_H
#define CONSENSUS_BASED_GREEDY_INSERTION_H

#include <vector>
#include "../../core/TSPProblem.h"

/**
 * @brief Consensus-Based Greedy Insertion Crossover (CBGI).
 *
 * Perserves common edges (consensus) between parents and reconstructs the rest
 * using a greedy insertion heuristic based on edge costs.
 *
 * @param parent1 The first parent solution.
 * @param parent2 The second parent solution.
 * @param problem The TSP problem instance.
 * @return A new offspring solution.
 */
std::vector<int> consensus_based_greedy_insertion(const std::vector<int>& parent1, const std::vector<int>& parent2, const TSPProblem& problem);

#endif // CONSENSUS_BASED_GREEDY_INSERTION_H
