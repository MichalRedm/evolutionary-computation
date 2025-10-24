#ifndef INTRA_NODE_EXCHANGE_H
#define INTRA_NODE_EXCHANGE_H

#include <vector>
#include <math.h>

double intra_node_exchange(
    const std::vector<std::vector<int>>& distance_matrix,
    const std::vector<int>& solution,
    int node_1_position,
    int node_2_position
);

#endif // INTRA_NODE_EXCHANGE_H
