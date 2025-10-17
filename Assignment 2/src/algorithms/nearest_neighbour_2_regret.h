#ifndef GREEDY_WITH_2_REGRET_H
#define GREEDY_WITH_2_REGRET_H

#include <vector>
#include "../core/point_data.h"

std::vector<int> nearest_neighbour_2_regret(const std::vector<PointData>& data, const std::vector<std::vector<int>>& distance_matrix, int start_node_id);

#endif // GREEDY_WITH_2_REGRET_H
