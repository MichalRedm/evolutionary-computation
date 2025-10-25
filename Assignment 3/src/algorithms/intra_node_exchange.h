#ifndef INTRA_NODE_EXCHANGE_H
#define INTRA_NODE_EXCHANGE_H

#include <vector>
#include <math.h>

/**
 * @brief Calculates the change in total tour cost (delta) for an intra-route node exchange.
 *
 * This move involves swapping the positions of two nodes that are currently in the solution.
 * The function handles three cases: adjacent nodes, non-adjacent nodes, and nodes
 * adjacent at the wrap-around point of the cycle.
 *
 * @param distance_matrix The pre-calculated distance matrix.
 * @param solution The current solution vector.
 * @param node_1_position The position (index) of the first node to be swapped.
 * @param node_2_position The position (index) of the second node to be swapped.
 * @return The delta (change in cost). A negative value indicates an improvement.
 */
double intra_node_exchange(
    const std::vector<std::vector<int>>& distance_matrix,
    const std::vector<int>& solution,
    int node_1_position,
    int node_2_position
);

#endif // INTRA_NODE_EXCHANGE_H
