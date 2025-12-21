#ifndef ASSYMETRIC_REPAIR_CROSSOVER_H
#define ASSYMETRIC_REPAIR_CROSSOVER_H

#include <vector>
#include "../../core/TSPProblem.h"

/**
 * @brief Operator 2: Assymetric Repair Crossover.
 *
 * Chooses one parent as starting solution, removes nodes not in the other parent,
 * and repairs the solution using the heuristic repair method.
 *
 * @param parent1 The first parent solution (starting solution/donor of order).
 * @param parent2 The second parent solution (filter).
 * @param problem The TSP problem instance.
 * @return A new offspring solution.
 */
std::vector<int> assymetric_repair_crossover(const std::vector<int>& parent1, const std::vector<int>& parent2, const TSPProblem& problem);

#endif // ASSYMETRIC_REPAIR_CROSSOVER_H
