#ifndef REPAIR_OPERATOR_H
#define REPAIR_OPERATOR_H

#include <vector>
#include "../core/TSPProblem.h"

/**
 * @brief Repair operator: Uses greedy weighted sum heuristic to rebuild the solution.
 * 
 * @param partial_solution The partial solution to be repaired.
 * @param problem The TSP problem instance.
 * @return A complete solution with 50% of nodes.
 */
std::vector<int> repair_solution(const std::vector<int>& partial_solution, const TSPProblem& problem);

#endif // REPAIR_OPERATOR_H
