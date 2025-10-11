#ifndef ASSIGNMENT1_H
#define ASSIGNMENT1_H

#include <vector>

struct PointData {
    int id;
    int x;
    int y;
    int cost;
};

std::vector<int> generate_random_solution(const std::vector<PointData>& data);
std::vector<int> generate_nearest_neighbor_end_solution(const std::vector<PointData>& data, const std::vector<std::vector<int>>& distance_matrix, int start_node_id);
std::vector<int> generate_nearest_neighbor_all_positions_solution(const std::vector<PointData>& data, const std::vector<std::vector<int>>& distance_matrix, int start_node_id);
std::vector<int> generate_greedy_cycle_solution(const std::vector<PointData>& data, const std::vector<std::vector<int>>& distance_matrix, int start_node_id);

#endif // ASSIGNMENT1_H
