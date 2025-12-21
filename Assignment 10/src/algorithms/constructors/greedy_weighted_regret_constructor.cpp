#include "greedy_weighted_regret_constructor.h"
#include <cmath>
#include <limits>
#include <algorithm>
#include <random>

// Internal helper struct to manage candidates
struct InsertionCandidate {
    int node_id;
    int insertion_idx;
    double objective;
};

std::vector<int> greedy_weighted_regret_constructor(
    const TSPProblem& problem, 
    int random_candidate_list_length, 
    const std::vector<int>& partial_solution
) {
    int total_nodes = problem.get_num_points();
    int num_to_select = static_cast<int>(ceil(static_cast<double>(total_nodes) / 2.0));

    if (num_to_select <= 0) {
        return {};
    }

    // Ensure safe RCL length
    if (random_candidate_list_length < 1) {
        random_candidate_list_length = 1;
    }

    // Initialize RNG (Static to seed once and reuse for performance)
    static std::mt19937 gen(std::random_device{}());

    std::vector<int> solution;
    std::vector<bool> visited(total_nodes, false);

    // Initialize solution state
    if (partial_solution.empty()) {
        solution.push_back(0); // Default start
        visited[0] = true;
    } else {
        solution = partial_solution;
        for (int node : solution) {
            if (node >= 0 && node < total_nodes) {
                visited[node] = true;
            }
        }
    }

    // Pre-allocate candidates vector to avoid re-allocations in the loop
    std::vector<InsertionCandidate> candidates;
    candidates.reserve(total_nodes);

    // Iteratively insert nodes
    while ((int)solution.size() < num_to_select) {
        candidates.clear();

        // 1. Evaluate all unvisited nodes
        for (int k = 0; k < total_nodes; ++k) {
            if (visited[k]) continue;

            double best_cost = std::numeric_limits<double>::max();
            double second_best_cost = std::numeric_limits<double>::max();
            int best_insertion_idx_for_k = -1;

            // Find best and second-best insertion positions for node k
            for (size_t i = 0; i < solution.size(); ++i) {
                int current_node_id = solution[i];
                int next_node_id = solution[(i + 1) % solution.size()];

                double dist_ik = problem.get_distance(current_node_id, k);
                double dist_kj = problem.get_distance(k, next_node_id);
                double dist_ij = problem.get_distance(current_node_id, next_node_id);
                double node_cost = problem.get_point(k).cost;

                // Cost change = (dist(i,k) + dist(k,j) - dist(i,j)) + cost(k)
                double cost_change = dist_ik + dist_kj - dist_ij + node_cost;

                if (cost_change < best_cost) {
                    second_best_cost = best_cost;
                    best_cost = cost_change;
                    best_insertion_idx_for_k = i + 1;
                } else if (cost_change < second_best_cost) {
                    second_best_cost = cost_change;
                }
            }

            // Calculate objective: Regret - Cost
            // Regret = second_best - best
            // Weighted Objective = (second_best - best) - best
            double regret = second_best_cost - best_cost;
            double weighted_objective = regret - best_cost;

            candidates.push_back({k, best_insertion_idx_for_k, weighted_objective});
        }

        if (candidates.empty()) {
            break;
        }

        // 2. Select from Random Candidate List (RCL)
        int current_rcl_len = std::min((int)candidates.size(), random_candidate_list_length);

        if (current_rcl_len > 1) {
            // Partially sort only the top 'current_rcl_len' candidates to the front
            std::partial_sort(
                candidates.begin(), 
                candidates.begin() + current_rcl_len, 
                candidates.end(),
                [](const InsertionCandidate& a, const InsertionCandidate& b) {
                    return a.objective > b.objective; // Descending order
                }
            );

            // Randomly pick one from the top candidates
            std::uniform_int_distribution<> dist(0, current_rcl_len - 1);
            int selected_idx = dist(gen);
            const auto& selected = candidates[selected_idx];
            
            solution.insert(solution.begin() + selected.insertion_idx, selected.node_id);
            visited[selected.node_id] = true;
        } else {
            // Optimization for deterministic case (greedy)
            // Just find the max element, no need to sort
            auto best_it = std::max_element(
                candidates.begin(), 
                candidates.end(),
                [](const InsertionCandidate& a, const InsertionCandidate& b) {
                    return a.objective < b.objective;
                }
            );

            if (best_it != candidates.end()) {
                solution.insert(solution.begin() + best_it->insertion_idx, best_it->node_id);
                visited[best_it->node_id] = true;
            }
        }
    }

    return solution;
}
