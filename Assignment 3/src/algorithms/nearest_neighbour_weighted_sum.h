#ifndef GREEDY_WITH_WEIGHTED_SUM_H
#define GREEDY_WITH_WEIGHTED_SUM_H

#include <vector>
#include "../core/point_data.h"

std::vector<int> nearest_neighbour_weighted_sum(const std::vector<PointData>& data, const std::vector<std::vector<int>>& distance_matrix, int start_node_id);

#endif // GREEDY_WITH_WEIGHTED_SUM_H
