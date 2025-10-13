#include "nearest_neighbor_end.h"

#include <vector>
#include <cmath>
#include <algorithm>
#include <limits>

std::vector<int> generate_nearest_neighbor_end_solution(const std::vector<PointData>& data, const std::vector<std::vector<int>>& distance_matrix, int start_node_id) {
    int total_nodes = data.size();
    int num_to_select = static_cast<int>(ceil(static_cast<double>(total_nodes) / 2.0));

    if (num_to_select <= 0) {
        return {};
    }
    if (num_to_select == 1) {
        return {start_node_id};
    }

    std::vector<int> solution;
    solution.push_back(start_node_id);

    std::vector<bool> visited(total_nodes, false);
    visited[start_node_id] = true;

    // Iteratively append the node that causes the smallest increase in the objective function
    while (solution.size() < num_to_select) {
        int best_node_to_insert = -1;
        double min_cost_change = std::numeric_limits<double>::max();

        // Iterate through all unvisited nodes
        for (int k = 0; k < total_nodes; ++k) {
            if (!visited[k]) {
                int current_node_id = solution[solution.size()-1];

                // Cost change = dist(i,k) + cost(k)
                double cost_change = distance_matrix[current_node_id][k] + data[k].cost;

                if (cost_change < min_cost_change) {
                    min_cost_change = cost_change;
                    best_node_to_insert = k;
                }
            }
        }

        if (best_node_to_insert != -1) {
            solution.push_back(best_node_to_insert);
            visited[best_node_to_insert] = true;
        } else {
            // No more unvisited nodes to insert
            break;
        }
    }

    return solution;
}
