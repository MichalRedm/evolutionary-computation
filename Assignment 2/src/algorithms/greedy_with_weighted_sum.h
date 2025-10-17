#ifndef NEAREST_NEIGHBOUR_WITH_WEIGHTED_SUM_H
#define NEAREST_NEIGHBOUR_WITH_WEIGHTED_SUM_H

#include <vector>
#include "../core/point_data.h"

std::vector<int> generate_with_weighted_sum_solution(const std::vector<PointData>& data, const std::vector<std::vector<int>>& distance_matrix, int start_node_id);

#endif // NEAREST_NEIGHBOUR_WITH_WEIGHTED_SUM_H
