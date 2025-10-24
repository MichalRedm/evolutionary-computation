#include "nearest_neighbour_weighted_sum.h"

#include <vector>
#include <cmath>
#include <algorithm>
#include <limits>

#include <iostream>

std::vector<int> nearest_neighbour_weighted_sum(const std::vector<PointData>& data, const std::vector<std::vector<int>>& distance_matrix, int start_node_id) {
    int total_nodes = data.size();
    int num_to_select = static_cast<int>(ceil(static_cast<double>(total_nodes) / 2.0));

    if (num_to_select <= 0) {
        return {};
    }

    std::vector<int> solution;
    solution.push_back(start_node_id);

    std::vector<bool> visited(total_nodes, false);
    visited[start_node_id] = true;

    // Iteratively insert nodes based on the 2-regret heuristic weighted with equal weight with basic greedy
    while (solution.size() < num_to_select) {
        int best_node_to_insert = -1;
        int best_insertion_idx = -1;
        double best_weighted_objective = -std::numeric_limits<double>::infinity();

        // Iterate through all unvisited nodes to find the one with the best objective function
        for (int k = 0; k < total_nodes; ++k) {
            if (!visited[k]) {
                double best_cost = std::numeric_limits<double>::max();
                double second_best_cost = std::numeric_limits<double>::max();
                int current_best_insertion_idx = -1;

                // Find the best and second-best insertion costs for node k
                for (int i = -1; i < static_cast<int>(solution.size()); ++i) {
                    double cost_change;
                    
                    if (i == -1) {
                        // If we are inserting at the beginning
                        int next = solution.front();
                        cost_change = distance_matrix[k][next] + data[k].cost;
                    } 
                    else if (i == static_cast<int>(solution.size()) - 1) {
                        // If we are inserting at the end
                        int prev = solution.back();
                        cost_change = distance_matrix[prev][k] + data[k].cost;
                    } 
                    else {
                        int prev = solution[i];
                        int next = solution[i + 1];
                        cost_change = distance_matrix[prev][k] + distance_matrix[k][next] - distance_matrix[prev][next] + data[k].cost;
                    }

                    if (cost_change < best_cost) {
                        second_best_cost = best_cost;
                        best_cost = cost_change;
                        current_best_insertion_idx = i + 1;
                    } else if (cost_change < second_best_cost) {
                        second_best_cost = cost_change;
                    }
                }

                double regret = second_best_cost - best_cost;
                double weighted_objective = regret - best_cost;

                if (weighted_objective > best_weighted_objective) {
                    best_weighted_objective = weighted_objective;
                    best_node_to_insert = k;
                    best_insertion_idx = current_best_insertion_idx;
                }
            }
        }

        if (best_node_to_insert != -1) {
            solution.insert(solution.begin() + best_insertion_idx, best_node_to_insert);
            visited[best_node_to_insert] = true;
        } else {
            // No more unvisited nodes to insert
            break;
        }
    }

    return solution;
}
