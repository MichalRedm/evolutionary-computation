#ifndef INTER_NODE_EXCHANGE_H
#define INTER_NODE_EXCHANGE_H

#include <vector>
#include <math.h>
#include "../core/point_data.h"

double inter_node_exchange(
    const std::vector<PointData>& data,
    std::vector<std::vector<int>>& distance_matrix,
    std::vector<int>& solution,
    int node_1_position,
    int node_2_id
);

#endif // INTER_NODE_EXCHANGE_H
