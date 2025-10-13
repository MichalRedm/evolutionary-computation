#ifndef NEAREST_NEIGHBOR_ALL_POSITIONS_H
#define NEAREST_NEIGHBOR_ALL_POSITIONS_H

#include <vector>
#include "../core/point_data.h"

std::vector<int> generate_nearest_neighbor_all_positions_solution(const std::vector<PointData>& data, const std::vector<std::vector<int>>& distance_matrix, int start_node_id);

#endif // NEAREST_NEIGHBOR_ALL_POSITIONS_H
