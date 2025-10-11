#ifndef NEAREST_NEIGHBOR_END_H
#define NEAREST_NEIGHBOR_END_H

#include <vector>
#include "point_data.h"

std::vector<int> generate_nearest_neighbor_end_solution(const std::vector<PointData>& data, const std::vector<std::vector<int>>& distance_matrix, int start_node_id);

#endif // NEAREST_NEIGHBOR_END_H
