#include "consensus_based_greedy_insertion.h"
#include <algorithm>
#include <set>
#include <map>
#include <vector>
#include <limits>
#include <iterator>

namespace {
    // Helper to represent an undirected edge
    struct Edge {
        int u, v;
        bool operator<(const Edge& other) const {
            if (u != other.u) return u < other.u;
            return v < other.v;
        }
        bool operator==(const Edge& other) const {
            return u == other.u && v == other.v;
        }
    };

    Edge make_edge(int u, int v) {
        if (u < v) return {u, v};
        return {v, u};
    }
}

// Helper to calculate cost contribution of a node in a specific position
// contribution = NodeCost + (dist(prev, node) + dist(node, next) - dist(prev, next))
double calculate_insertion_cost(int node, int prev, int next, const TSPProblem& problem) {
    // ASSUMPTION: TSPProblem has a method to get node cost. 
    // Adjust 'problem.get_node_cost(node)' to match your specific API (e.g., problem.costs[node]).
    double node_cost = problem.get_point(node).cost; 
    
    // Distances (Euclidean rounded to int, as per problem description)
    double d_prev_node = problem.get_distance(prev, node);
    double d_node_next = problem.get_distance(node, next);
    double d_prev_next = problem.get_distance(prev, next); // The edge we are removing

    return node_cost + (d_prev_node + d_node_next - d_prev_next);
}

std::vector<int> consensus_based_greedy_insertion(const std::vector<int>& parent1, const std::vector<int>& parent2, const TSPProblem& problem) {
    
    // --- 1. Identify Common Edges (The "Consensus") ---
    std::set<Edge> p2_edges;
    if (!parent2.empty()) {
        for (size_t i = 0; i < parent2.size(); ++i) {
            p2_edges.insert(make_edge(parent2[i], parent2[(i + 1) % parent2.size()]));
        }
    }

    std::vector<Edge> common_edges;
    std::set<int> common_nodes_set;
    
    if (!parent1.empty()) {
        for (size_t i = 0; i < parent1.size(); ++i) {
            Edge e = make_edge(parent1[i], parent1[(i + 1) % parent1.size()]);
            if (p2_edges.count(e)) {
                common_edges.push_back(e);
                common_nodes_set.insert(e.u);
                common_nodes_set.insert(e.v);
            }
        }
    }

    // --- 2. Build Subpaths from Common Edges ---
    // This logic reconstructs connected components (chains) from the edge list
    std::map<int, std::vector<int>> adj;
    for (const auto& e : common_edges) {
        adj[e.u].push_back(e.v);
        adj[e.v].push_back(e.u);
    }

    std::vector<std::vector<int>> subpaths;
    std::set<int> visited;

    for (int node : common_nodes_set) {
        if (visited.count(node)) continue;

        // Start a new path traversal
        std::vector<int> path;
        int curr = node;
        
        // Find a degree-1 node to start traversing if possible (start of a chain)
        // If it's a cycle, any node is fine.
        std::vector<int> component_nodes;
        std::vector<int> q = {node};
        std::set<int> comp_visited = {node};
        int head = 0;
        int start_node = node;
        
        // BFS to find component structure
        while(head < (int)q.size()){
            int u = q[head++];
            if (adj[u].size() == 1) start_node = u; // Found an endpoint
            for (int v : adj[u]) {
                if (!comp_visited.count(v)) {
                    comp_visited.insert(v);
                    q.push_back(v);
                }
            }
        }

        // Trace the path starting from 'start_node'
        curr = start_node;
        path.push_back(curr);
        visited.insert(curr);
        
        while (true) {
            bool found = false;
            for (int neighbor : adj[curr]) {
                if (!visited.count(neighbor)) {
                    visited.insert(neighbor);
                    path.push_back(neighbor);
                    curr = neighbor;
                    found = true;
                    break; 
                }
            }
            if (!found) break;
        }
        subpaths.push_back(path);
    }

    // Add common nodes that were isolated (no common edges) as single-node paths
    // Find intersection of nodes
    std::set<int> p1_nodes(parent1.begin(), parent1.end());
    std::set<int> p2_nodes(parent2.begin(), parent2.end());
    std::vector<int> all_common_nodes;
    std::set_intersection(p1_nodes.begin(), p1_nodes.end(),
                          p2_nodes.begin(), p2_nodes.end(),
                          std::back_inserter(all_common_nodes));

    for (int node : all_common_nodes) {
        if (!common_nodes_set.count(node)) {
            subpaths.push_back({node});
        }
    }

    // If no common structure at all, pick a random start node from Parent 1
    if (subpaths.empty() && !parent1.empty()) {
        subpaths.push_back({parent1[0]});
    }

    // --- 3. Link Subpaths into a Single Cycle (Nearest Neighbor Linkage) ---
    // Start with the first subpath and greedily attach the closest remaining subpath
    std::vector<int> offspring = subpaths[0];
    subpaths.erase(subpaths.begin());

    while (!subpaths.empty()) {
        int tail = offspring.back();
        int best_idx = -1;
        double min_dist = std::numeric_limits<double>::max();
        bool reverse_best = false;

        // Find the subpath whose start or end is closest to our current tail
        for (size_t i = 0; i < subpaths.size(); ++i) {
            int head_node = subpaths[i].front();
            int tail_node = subpaths[i].back();

            double d_head = problem.get_distance(tail, head_node);
            double d_tail = problem.get_distance(tail, tail_node);

            if (d_head < min_dist) {
                min_dist = d_head;
                best_idx = i;
                reverse_best = false; // Attach as is: tail -> head...tail
            }
            if (d_tail < min_dist) {
                min_dist = d_tail;
                best_idx = i;
                reverse_best = true; // Attach reversed: tail -> tail...head
            }
        }

        if (best_idx != -1) {
            std::vector<int>& best_path = subpaths[best_idx];
            if (reverse_best) {
                std::reverse(best_path.begin(), best_path.end());
            }
            offspring.insert(offspring.end(), best_path.begin(), best_path.end());
            subpaths.erase(subpaths.begin() + best_idx);
        } else {
            // Should not happen, but safe fallback
            break; 
        }
    }

    // --- 4. Enforce Exact Size Constraint (50% Selection) ---
    int total_nodes = problem.get_num_points();
    int target_size = (total_nodes + 1) / 2; // Round up

    // A. Greedy Insertion if we are too small
    if ((int)offspring.size() < target_size) {
        // Candidate pool: Union of parents (minus already selected)
        // If union is not enough, fall back to all nodes.
        std::set<int> current_selection(offspring.begin(), offspring.end());
        std::vector<int> candidates;
        
        // Prioritize nodes from parents
        std::set<int> parent_union = p1_nodes;
        parent_union.insert(p2_nodes.begin(), p2_nodes.end());

        for (int node : parent_union) {
            if (!current_selection.count(node)) candidates.push_back(node);
        }
        
        // If still not enough candidates (rare), add the rest of the world
        if ((int)(candidates.size() + offspring.size()) < target_size) {
             for (int i = 0; i < total_nodes; ++i) {
                 if (!current_selection.count(i) && parent_union.find(i) == parent_union.end()) {
                     candidates.push_back(i);
                 }
             }
        }

        while ((int)offspring.size() < target_size && !candidates.empty()) {
            int best_cand_idx = -1;
            int best_pos = -1;
            double best_cost_increase = std::numeric_limits<double>::max();

            // Check every candidate against every position
            // Optimization: For very large instances, we might randomize the candidate subset,
            // but for typical TSP sizes this O(N*K) is acceptable.
            for (size_t c = 0; c < candidates.size(); ++c) {
                int cand = candidates[c];
                double node_c = problem.get_point(cand).cost; 

                for (size_t i = 0; i < offspring.size(); ++i) {
                    int prev = offspring[i];
                    int next = offspring[(i + 1) % offspring.size()];
                    
                    // Cost change = NodeCost + (dist_increase)
                    double increase = node_c + (problem.get_distance(prev, cand) + 
                                                problem.get_distance(cand, next) - 
                                                problem.get_distance(prev, next));
                    
                    if (increase < best_cost_increase) {
                        best_cost_increase = increase;
                        best_cand_idx = c;
                        best_pos = i + 1; // Insert after i
                    }
                }
            }

            if (best_cand_idx != -1) {
                offspring.insert(offspring.begin() + best_pos, candidates[best_cand_idx]);
                candidates.erase(candidates.begin() + best_cand_idx);
            } else {
                break;
            }
        }
    }
    // B. Pruning if we are too big (Parents shared > 50% nodes)
    else if ((int)offspring.size() > target_size) {
        while ((int)offspring.size() > target_size) {
            int worst_idx = -1;
            double max_cost_reduction = -std::numeric_limits<double>::max();

            // Find node whose removal maximizes savings (reduces objective most)
            // Saving = NodeCost + (dist(prev, node) + dist(node, next) - dist(prev, next))
            // We want to maximize this Saving.
            for (size_t i = 0; i < offspring.size(); ++i) {
                int node = offspring[i];
                int prev = offspring[(i - 1 + offspring.size()) % offspring.size()];
                int next = offspring[(i + 1) % offspring.size()];

                double saving = problem.get_point(node).cost + 
                                (problem.get_distance(prev, node) + 
                                 problem.get_distance(node, next) - 
                                 problem.get_distance(prev, next));
                
                if (saving > max_cost_reduction) {
                    max_cost_reduction = saving;
                    worst_idx = i;
                }
            }
            
            if (worst_idx != -1) {
                offspring.erase(offspring.begin() + worst_idx);
            }
        }
    }

    return offspring;
}
