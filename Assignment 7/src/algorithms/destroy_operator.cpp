#include "destroy_operator.h"
#include <algorithm>
#include <set>

std::vector<int> destroy_solution(const std::vector<int>& solution, const TSPProblem& problem, std::mt19937& rng) {
    std::vector<int> current_solution = solution;
    int n = current_solution.size();
    if (n <= 3) return current_solution; // Too small to destroy meaningfully

    // Calculate edge costs to guide removal (higher cost -> higher prob of being in a removed segment)
    // For simplicity and speed, we can just pick random start points, or pick worst edges.
    
    struct Edge {
        int u, v;
        int idx_u; // index of u in solution
        double cost;
    };
    
    std::vector<Edge> edges;
    for (int i = 0; i < n; ++i) {
        int u = current_solution[i];
        int v = current_solution[(i + 1) % n];
        edges.push_back({u, v, i, (double)problem.get_distance(u, v)});
    }
    
    // Calculate target number of nodes to remove (20-40%)
    int min_remove = (int)(n * 0.20);
    int max_remove = (int)(n * 0.40);
    if (min_remove < 1) min_remove = 1;
    if (max_remove >= n) max_remove = n - 1;
    
    int target_removed = std::uniform_int_distribution<>(min_remove, max_remove)(rng);
    
    // Sort edges by cost descending
    std::sort(edges.begin(), edges.end(), [](const Edge& a, const Edge& b) {
        return a.cost > b.cost;
    });
    
    // Select top edges to break, with some randomness
    std::vector<int> indices_to_remove;
    std::set<int> removed_indices_set;
    
    int attempts = 0;
    while ((int)removed_indices_set.size() < target_removed && attempts < n * 10) {
        attempts++;
        
        // Pick one of the top edges with some randomness (top 30% of edges)
        int range = std::max(1, (int)edges.size() / 3);
        int edge_idx = std::uniform_int_distribution<>(0, range - 1)(rng);
        int start_idx = edges[edge_idx].idx_u;
        
        // Remove a segment of length 2-8 starting from this edge
        int segment_len = std::uniform_int_distribution<>(2, 8)(rng);
        
        for (int j = 0; j < segment_len && (int)removed_indices_set.size() < target_removed; ++j) {
            int idx = (start_idx + j) % n;
            if (removed_indices_set.find(idx) == removed_indices_set.end()) {
                removed_indices_set.insert(idx);
                indices_to_remove.push_back(idx);
            }
        }
    }
    
    // Construct new solution
    std::vector<int> new_solution;
    for (int i = 0; i < n; ++i) {
        if (removed_indices_set.find(i) == removed_indices_set.end()) {
            new_solution.push_back(current_solution[i]);
        }
    }
    
    return new_solution;
}
