#include "greedy_edge_crossover.h"
#include <vector>
#include <set>
#include <cmath>
#include <limits>
#include <random>

std::vector<int> greedy_edge_crossover(const std::vector<int>& parent1, const std::vector<int>& parent2, const TSPProblem& problem) {
    int total_nodes = problem.get_num_points();
    int target_size = static_cast<int>(ceil(total_nodes / 2.0));

    // 1. Build adjacency maps for both parents to allow quick lookups
    // We map: node -> list of neighbors in that parent
    std::vector<std::vector<int>> adj_p1(total_nodes);
    std::vector<std::vector<int>> adj_p2(total_nodes);

    auto build_adj = [&](const std::vector<int>& p, std::vector<std::vector<int>>& adj) {
        if (p.empty()) return;
        for (size_t i = 0; i < p.size(); ++i) {
            int u = p[i];
            int v = p[(i + 1) % p.size()];
            adj[u].push_back(v);
            adj[v].push_back(u);
        }
    };

    build_adj(parent1, adj_p1);
    build_adj(parent2, adj_p2);

    // 2. Identify the pool of available nodes (Union of P1 and P2)
    std::set<int> available_pool;
    available_pool.insert(parent1.begin(), parent1.end());
    available_pool.insert(parent2.begin(), parent2.end());

    std::vector<int> offspring;
    std::set<int> visited;
    offspring.reserve(target_size);

    // 3. Start from a random node present in Parent 1
    std::random_device rd;
    std::mt19937 g(rd());
    
    int current_node = -1;
    if (!parent1.empty()) {
        std::uniform_int_distribution<> dist(0, parent1.size() - 1);
        current_node = parent1[dist(g)];
    } else {
        // Fallback if parent1 is empty (edge case)
        current_node = 0; 
    }

    offspring.push_back(current_node);
    visited.insert(current_node);

    // 4. Construct the path
    while (static_cast<int>(offspring.size()) < target_size) {
        int best_next_node = -1;
        double min_dist = std::numeric_limits<double>::max();
        bool found_in_parents = false;

        // Check neighbors from Parent 1
        for (int neighbor : adj_p1[current_node]) {
            if (visited.find(neighbor) == visited.end()) {
                double d = problem.get_distance(current_node, neighbor);
                if (d < min_dist) {
                    min_dist = d;
                    best_next_node = neighbor;
                    found_in_parents = true;
                }
            }
        }

        // Check neighbors from Parent 2
        for (int neighbor : adj_p2[current_node]) {
            if (visited.find(neighbor) == visited.end()) {
                double d = problem.get_distance(current_node, neighbor);
                if (d < min_dist) {
                    min_dist = d;
                    best_next_node = neighbor;
                    found_in_parents = true;
                }
            }
        }

        // If no valid neighbor in parents (or all visited), search globally in the available pool
        // We pick the nearest unvisited node from the union of parents
        if (!found_in_parents) {
            min_dist = std::numeric_limits<double>::max();
            for (int candidate : available_pool) {
                if (visited.find(candidate) == visited.end()) {
                    double d = problem.get_distance(current_node, candidate);
                    if (d < min_dist) {
                        min_dist = d;
                        best_next_node = candidate;
                    }
                }
            }
        }

        // If we still didn't find a node (e.g. pool exhausted before target size), break
        // (This shouldn't happen given the logic unless inputs are too small)
        if (best_next_node == -1) break;

        offspring.push_back(best_next_node);
        visited.insert(best_next_node);
        current_node = best_next_node;
    }

    return offspring;
}
