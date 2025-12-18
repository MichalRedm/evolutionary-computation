#ifndef PRESERVATION_CROSSOVER_H
#define PRESERVATION_CROSSOVER_H

#include <vector>
#include "../core/TSPProblem.h"

/**
 * @brief Operator 2: Preservation crossover operator.
 *
 * Chooses one parent as starting solution, removes nodes not in the other parent,
 * and repairs the solution using the heuristic repair method.
 *
 * @param parent1 The first parent solution (starting solution/donor of order).
 * @param parent2 The second parent solution (filter).
 * @param problem The TSP problem instance.
 * @return A new offspring solution.
 */
std::vector<int> preservation_crossover(const std::vector<int>& parent1, const std::vector<int>& parent2, const TSPProblem& problem);

#endif // PRESERVATION_CROSSOVER_H
