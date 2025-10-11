#ifndef GREEDY_CYCLE_H
#define GREEDY_CYCLE_H

#include <vector>
#include "point_data.h"

std::vector<int> generate_greedy_cycle_solution(const std::vector<PointData>& data, const std::vector<std::vector<int>>& distance_matrix, int start_node_id);

#endif // GREEDY_CYCLE_H
