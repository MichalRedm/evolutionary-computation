#include "nearest_neighbor_all_positions.h"
#include "point_data.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <limits>

std::vector<int> generate_nearest_neighbor_all_positions_solution(const std::vector<PointData>& data, const std::vector<std::vector<int>>& distance_matrix, int start_node_id) {
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

    // Find the best second node to form the initial 2-node solution
    int best_second_node = -1;
    double min_initial_objective = std::numeric_limits<double>::max();

    for (int i = 0; i < total_nodes; ++i) {
        if (!visited[i]) {
            // Objective for a 2-node solution: dist(A,B) + cost(A) + cost(B)
            double current_objective = distance_matrix[start_node_id][i] + data[start_node_id].cost + data[i].cost;
            if (current_objective < min_initial_objective) {
                min_initial_objective = current_objective;
                best_second_node = i;
            }
        }
    }

    if (best_second_node != -1) {
        solution.push_back(best_second_node);
        visited[best_second_node] = true;
    } else {
        // This case should not be reached if total_nodes > 1
        return solution;
    }

    // Iteratively insert the node that causes the smallest increase in the objective function
    while (solution.size() < num_to_select) {
        int best_node_to_insert = -1;
        int best_insertion_idx = -1;
        double min_cost_change = std::numeric_limits<double>::max();

        // Iterate through all unvisited nodes
        for (int k = 0; k < total_nodes; ++k) {
            if (!visited[k]) {
                // Iterate through all possible insertion positions in the current cycle
                for (size_t i = 0; i < solution.size(); ++i) {
                    int current_node_id = solution[i];
                    int next_node_id = solution[(i + 1) % solution.size()];

                    // Cost change = (dist(i,k) + dist(k,j) - dist(i,j)) + cost(k)
                    // if we insert at the end we are not taking the edge that would make a cycle into account
                    double cost_change;

                    if (i == solution.size() - 1)
                    {
                        cost_change = distance_matrix[current_node_id][k] + data[k].cost;
                    }
                    else{
                        cost_change = distance_matrix[current_node_id][k] + distance_matrix[k][next_node_id] - distance_matrix[current_node_id][next_node_id] + data[k].cost;
                    }

                    if (cost_change < min_cost_change) {
                        min_cost_change = cost_change;
                        best_node_to_insert = k;
                        best_insertion_idx = i + 1;
                    }
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
