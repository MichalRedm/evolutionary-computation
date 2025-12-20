#include "cost_weighted_edge_recombination.h"
#include <vector>
#include <set>
#include <map>
#include <limits>

// Helper to represent an undirected edge for checking commonality
struct Edge {
    int u, v;
    bool operator<(const Edge& other) const {
        if (u != other.u) return u < other.u;
        return v < other.v;
    }
};

Edge make_edge(int u, int v) {
    if (u < v) return {u, v};
    return {v, u};
}

// Helper to calculate the "cost" of moving to a node
// We want to minimize: Node Cost + Edge Length
double get_transition_cost(int from, int to, const TSPProblem& problem) {
    // ASSUMPTION: TSPProblem has get_node_cost(int) and get_distance(int, int)
    return problem.get_point(to).cost + problem.get_distance(from, to);
}

std::vector<int> cost_weighted_edge_recombination(const std::vector<int>& parent1, const std::vector<int>& parent2, const TSPProblem& problem) {
    
    // 1. Setup Data Structures
    int total_nodes = problem.get_num_points();
    // Target size: 50% of nodes, rounded up 
    int target_size = (total_nodes + 1) / 2; 

    // Adjacency Map: Node -> Set of Neighbors (from both parents)
    std::map<int, std::set<int>> edge_map;
    
    // Set of "Common Edges" (present in both parents)
    std::set<Edge> p1_edges_set;
    std::set<Edge> common_edges;

    // --- Build Edge Map & Identify Common Edges ---
    auto process_parent = [&](const std::vector<int>& p, bool is_p1) {
        if (p.empty()) return;
        for (std::size_t i = 0; i < p.size(); ++i) {
            int u = p[i];
            int v = p[(i + 1) % p.size()];
            
            // Add to Edge Map (Undirected)
            edge_map[u].insert(v);
            edge_map[v].insert(u);

            Edge e = make_edge(u, v);
            if (is_p1) {
                p1_edges_set.insert(e);
            } else {
                // If checking P2, and it exists in P1 set, it's common
                if (p1_edges_set.count(e)) {
                    common_edges.insert(e);
                }
            }
        }
    };

    process_parent(parent1, true);
    process_parent(parent2, false);

    // Identify the pool of available nodes (Union of P1 and P2)
    // We strictly prefer nodes that were good enough to be in at least one parent.
    std::set<int> candidate_pool;
    candidate_pool.insert(parent1.begin(), parent1.end());
    candidate_pool.insert(parent2.begin(), parent2.end());

    // 2. Initialization
    std::vector<int> offspring;
    std::set<int> visited;
    
    // Start with the first node of Parent 1 (preserves some order bias)
    int current_node = (!parent1.empty()) ? parent1[0] : 
                       ((!candidate_pool.empty()) ? *candidate_pool.begin() : 0);
    
    offspring.push_back(current_node);
    visited.insert(current_node);

    // 3. Construction Loop
    while ((int)offspring.size() < target_size) {
        // Remove current node from candidate pool to speed up future searches
        if (candidate_pool.count(current_node)) {
            candidate_pool.erase(current_node);
        }

        int next_node = -1;
        bool found = false;

        // -- Priority 1: Common Edges (Consensus) --
        // Check if any neighbor in the map shares a "common edge" with current
        // and is unvisited.
        if (edge_map.count(current_node)) {
            for (int neighbor : edge_map[current_node]) {
                if (visited.count(neighbor)) continue;

                if (common_edges.count(make_edge(current_node, neighbor))) {
                    next_node = neighbor;
                    found = true;
                    break; 
                }
            }
        }

        // -- Priority 2: Cost-Weighted Nearest Neighbor in Edge Map --
        // If no common edge, pick the "best" neighbor from the Edge Map.
        // Best = Min(NodeCost + Distance)
        if (!found && edge_map.count(current_node)) {
            double best_score = std::numeric_limits<double>::max();
            
            for (int neighbor : edge_map[current_node]) {
                if (visited.count(neighbor)) continue;

                // Heuristic Score: 
                // We minimize the objective contribution directly.
                double score = get_transition_cost(current_node, neighbor, problem);

                // Tie-breaking: If scores are equal, prefer the one with fewer 
                // remaining neighbors (standard ERX heuristic to prevent isolation)
                // But here, cost is dominant.
                
                if (score < best_score) {
                    best_score = score;
                    next_node = neighbor;
                    found = true;
                }
            }
        }

        // -- Priority 3: Global Rescue (Jump) --
        // If current node has no unvisited neighbors in the map (dead end),
        // we must jump to a node in the 'candidate_pool' (P1 U P2).
        if (!found) {
            double best_score = std::numeric_limits<double>::max();
            int best_candidate = -1;
            
            // Scan the candidate pool (nodes from parents that aren't visited)
            // If the pool is empty (rare, if parents were small), fallback to all nodes.
            if (candidate_pool.empty()) {
                 // Fallback: Scan all problem nodes
                 for(int i=0; i<total_nodes; ++i) {
                     if(!visited.count(i)) {
                         double score = get_transition_cost(current_node, i, problem);
                         if(score < best_score) {
                             best_score = score;
                             best_candidate = i;
                         }
                     }
                 }
            } else {
                // Normal Rescue: Scan P1 U P2
                for (int candidate : candidate_pool) {
                    // candidate is guaranteed unvisited by set management logic
                    double score = get_transition_cost(current_node, candidate, problem);
                    if (score < best_score) {
                        best_score = score;
                        best_candidate = candidate;
                    }
                }
            }

            if (best_candidate != -1) {
                next_node = best_candidate;
                found = true;
            }
        }

        // Apply Selection
        if (found) {
            offspring.push_back(next_node);
            visited.insert(next_node);
            current_node = next_node;
        } else {
            // Should only happen if we ran out of nodes completely
            break; 
        }
    }

    return offspring;
}
