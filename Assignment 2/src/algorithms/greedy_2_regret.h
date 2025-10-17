#ifndef GREEDY_2_REGRET_H
#define GREEDY_2_REGRET_H

#include <vector>
#include <tuple>
#include "../core/point_data.h"

std::tuple<std::vector<int>, int> generate_greedy_2_regret_solution(const std::vector<PointData>& data, const std::vector<std::vector<int>>& distance_matrix, int start_node_id);

#endif // GREEDY_2_REGRET_H
