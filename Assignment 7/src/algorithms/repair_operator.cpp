#include "repair_operator.h"
#include <cmath>
#include <limits>

std::vector<int> repair_solution(const std::vector<int>& partial_solution, const TSPProblem& problem) {
    int total_nodes = problem.get_num_points();
    int num_to_select = static_cast<int>(ceil(static_cast<double>(total_nodes) / 2.0));

    if (num_to_select <= 0) {
        return {};
    }

    std::vector<int> solution;
    std::vector<bool> visited(total_nodes, false);

    if (partial_solution.empty()) {
        // Should not happen in LNS repair usually, but handle it
        solution.push_back(0); // Default start
        visited[0] = true;
    } else {
        solution = partial_solution;
        for (int node : solution) {
            visited[node] = true;
        }
    }

    // Iteratively insert nodes based on the 2-regret heuristic weighted with equal weight with basic greedy
    while ((int)solution.size() < num_to_select) {
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
                for (size_t i = 0; i < solution.size(); ++i) {
                    int current_node_id = solution[i];
                    int next_node_id = solution[(i + 1) % solution.size()];

                    // Cost change = (dist(i,k) + dist(k,j) - dist(i,j)) + cost(k)
                    double cost_change = problem.get_distance(current_node_id, k) + 
                                         problem.get_distance(k, next_node_id) - 
                                         problem.get_distance(current_node_id, next_node_id) + 
                                         problem.get_point(k).cost;

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
