#ifndef LOCAL_SEARCH_H
#define LOCAL_SEARCH_H

#include <vector>
#include "../core/point_data.h"

enum class SearchType {
    STEEPEST,
    GREEDY
};

enum class IntraMoveType {
    NODES_EXCHANGE,
    EDGES_EXCHANGE
};

enum class StartingSolutionType {
    RANDOM,
    GREEDY
};

std::vector<int> local_search(
    const std::vector<PointData>& data,
    std::vector<std::vector<int>>& distance_matrix,
    SearchType T,
    IntraMoveType N,
    StartingSolutionType S,
    StageTimer& timer,
    int greedy_start_node_id
);

#endif // LOCAL_SEARCH_H
