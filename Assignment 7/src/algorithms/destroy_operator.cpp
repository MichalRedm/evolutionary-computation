#include "destroy_operator.h"
#include <algorithm>
#include <set>

std::vector<int> destroy_solution(const std::vector<int>& solution, const TSPProblem& problem, std::mt19937& rng) {
    std::vector<int> current_solution = solution;
    int n = current_solution.size();
    if (n <= 3) return current_solution; // Too small to destroy meaningfully

    // We want to remove segments. Let's remove 2-3 segments.
    int num_segments = std::uniform_int_distribution<>(2, 3)(rng);
    
    // Calculate edge costs to guide removal (higher cost -> higher prob of being in a removed segment)
    // For simplicity and speed, we can just pick random start points, or pick worst edges.
    // Let's pick 'num_segments' worst edges and remove a small chunk around them.
    
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
    
    // Sort edges by cost descending
    std::sort(edges.begin(), edges.end(), [](const Edge& a, const Edge& b) {
        return a.cost > b.cost;
    });
    
    // Select top edges to break, with some randomness
    std::vector<int> indices_to_remove;
    std::set<int> removed_indices_set;
    
    for (int k = 0; k < num_segments && k < (int)edges.size(); ++k) {
        // Pick one of the top edges with some randomness
        int edge_idx = std::uniform_int_distribution<>(0, std::min((int)edges.size() - 1, k + 5))(rng);
        int start_idx = edges[edge_idx].idx_u;
        
        // Remove a segment of length 2-5 starting from this edge
        int segment_len = std::uniform_int_distribution<>(2, 5)(rng);
        
        for (int j = 0; j < segment_len; ++j) {
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
