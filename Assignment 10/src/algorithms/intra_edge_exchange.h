#ifndef INTRA_EDGE_EXCHANGE_H
#define INTRA_EDGE_EXCHANGE_H

#include <vector>
#include "../core/TSPProblem.h"

/**
 * @brief Calculates the change in total tour cost (delta) for a 2-edge exchange (2-opt) move.
 *
 * This move breaks the edges (i, i+1) and (j, j+1) and creates new edges (i, j) and (i+1, j+1).
 * This effectively reverses the path segment between node i+1 and node j.
 *
 * @param problem_instance The TSPProblem instance containing distance information.
 * @param pos1 The position of the first node of the first edge to break.
 * @param pos2 The position of the first node of the second edge to break.
 * @return The delta (change in cost). A negative value indicates an improvement.
 */
double intra_edge_exchange(
    TSPProblem& problem_instance,
    const std::vector<int>& solution,
    int pos1,
    int pos2
);

/**
 * @brief Applies a 2-edge exchange (2-opt) move to the solution vector.
 *
 * This move corresponds to the delta calculated in intra_edge_exchange.
 * It reverses the segment of the solution between pos1+1 and pos2 (inclusive).
 *
 * @param solution The solution vector (will be modified in-place).
 * @param pos1 The position of the first node of the first broken edge.
 * @param pos2 The position of the first node of the second broken edge.
 */
void apply_intra_edge_exchange(std::vector<int>& solution, int pos1, int pos2);

#endif // INTRA_EDGE_EXCHANGE_H
