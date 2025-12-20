#include "cost_priority_crossover.h"
#include <vector>
#include <set>
#include <algorithm>
#include <cmath>

namespace {
    // Helper struct to sort nodes by cost
    struct NodeCostPair {
        int node_idx;
        double cost;
        
        bool operator<(const NodeCostPair& other) const {
            if (cost != other.cost) return cost < other.cost;
            return node_idx < other.node_idx; // Tie-breaker
        }
        
        NodeCostPair(int n, double c) : node_idx(n), cost(c) {}
    };
}

std::vector<int> cost_priority_crossover(const std::vector<int>& parent1, const std::vector<int>& parent2, const TSPProblem& problem) {
    int total_nodes = problem.get_num_points();
    int target_size = static_cast<int>(ceil(total_nodes / 2.0));

    // 1. Identify all unique nodes in both parents
    std::set<int> union_nodes;
    union_nodes.insert(parent1.begin(), parent1.end());
    union_nodes.insert(parent2.begin(), parent2.end());

    // 2. Rank nodes by cost (Ascending)
    std::vector<NodeCostPair> ranked_nodes;
    ranked_nodes.reserve(union_nodes.size());
    
    for (int node : union_nodes) {
        ranked_nodes.push_back(NodeCostPair(node, (double)problem.get_point(node).cost));
    }
    
    std::sort(ranked_nodes.begin(), ranked_nodes.end());

    // 3. Select the top N cheapest nodes
    std::set<int> selected_nodes;
    for (size_t i = 0; i < ranked_nodes.size() && static_cast<int>(selected_nodes.size()) < target_size; ++i) {
        selected_nodes.insert(ranked_nodes[i].node_idx);
    }

    // 4. Construct the offspring sequence
    // Strategy: Preserve the relative order of Parent 1 for the selected nodes.
    // If Parent 1 doesn't contain a selected node (it came from P2), append it at the end.
    // (Local search heuristics usually run after crossover and will fix the appended tail)
    
    std::vector<int> offspring;
    offspring.reserve(target_size);

    // First pass: Add nodes present in Parent 1 in their original order
    for (int node : parent1) {
        if (selected_nodes.find(node) != selected_nodes.end()) {
            offspring.push_back(node);
            selected_nodes.erase(node); // Remove to mark as added
        }
    }

    // Second pass: Add remaining nodes (those unique to Parent 2)
    // We try to follow Parent 2's order for these remainder nodes
    for (int node : parent2) {
        if (selected_nodes.find(node) != selected_nodes.end()) {
            offspring.push_back(node);
            selected_nodes.erase(node);
        }
    }

    // Edge case: If somehow we still haven't met the target (unlikely if logic is correct),
    // add remaining from the set (though the loops above should cover the union).
    if (!selected_nodes.empty()) {
        for (int node : selected_nodes) {
            offspring.push_back(node);
        }
    }

    return offspring;
}
