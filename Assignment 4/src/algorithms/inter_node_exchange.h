#ifndef INTER_NODE_EXCHANGE_H
#define INTER_NODE_EXCHANGE_H

#include <vector>
#include <math.h>
#include "../core/point_data.h"

/**
 * @brief Calculates the change in total cost (delta) for an inter-route node exchange.
 *
 * This move involves replacing a node currently in the solution (`node_1`) with a node
 * that is not in the solution (`node_2`). The function calculates the change in
 * tour distance and node costs.
 *
 * @param data The vector of all point data, used to get node costs.
 * @param distance_matrix The pre-calculated distance matrix.
 * @param solution The current solution vector.
 * @param node_1_position The position (index) in the solution of the node to be replaced.
 * @param node_2_id The ID of the node from outside the solution to be inserted.
 * @return The delta (change in cost). A negative value indicates an improvement.
 */
double inter_node_exchange(
    const std::vector<PointData>& data,
    std::vector<std::vector<int>>& distance_matrix,
    std::vector<int>& solution,
    int node_1_position,
    int node_2_id
);

#endif // INTER_NODE_EXCHANGE_H
