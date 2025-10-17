#ifndef GREEDY_WITH_WEIGHTED_SUM_H
#define GREEDY_WITH_WEIGHTED_SUM_H

#include <vector>
#include <tuple>
#include "../core/point_data.h"

std::tuple<std::vector<int>, int> generate_with_weighted_sum_solution(const std::vector<PointData>& data, const std::vector<std::vector<int>>& distance_matrix, int start_node_id);

#endif // GREEDY_WITH_WEIGHTED_SUM_H
