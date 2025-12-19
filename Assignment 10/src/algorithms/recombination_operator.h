#ifndef RECOMBINATION_OPERATOR_H
#define RECOMBINATION_OPERATOR_H

#include <vector>
#include "../core/TSPProblem.h"

/**
 * @brief Operator 1: Recombination operator.
 *
 * Locates common nodes and edges in the offspring compared to parents.
 * Fills the rest of the solution at random (up to 50% of total nodes).
 * Connects subpaths at random to form a single cycle.
 *
 * @param parent1 The first parent solution.
 * @param parent2 The second parent solution.
 * @param problem The TSP problem instance.
 * @return A new offspring solution.
 */
std::vector<int> recombination_operator(const std::vector<int>& parent1, const std::vector<int>& parent2, const TSPProblem& problem);

#endif // RECOMBINATION_OPERATOR_H
