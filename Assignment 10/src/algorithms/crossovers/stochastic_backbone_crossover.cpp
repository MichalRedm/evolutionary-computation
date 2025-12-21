#include "stochastic_backbone_crossover.h"
#include <algorithm>
#include <set>
#include <map>
#include <random>
#include <cmath>

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

std::vector<int> stochastic_backbone_crossover(const std::vector<int>& parent1, const std::vector<int>& parent2, const TSPProblem& problem) {
    // 1. Identify common nodes
    std::set<int> p1_nodes(parent1.begin(), parent1.end());
    std::set<int> p2_nodes(parent2.begin(), parent2.end());
    
    std::vector<int> common_nodes;
    std::set_intersection(p1_nodes.begin(), p1_nodes.end(),
                          p2_nodes.begin(), p2_nodes.end(),
                          std::back_inserter(common_nodes));
    
    std::set<int> common_nodes_set(common_nodes.begin(), common_nodes.end());

    // 2. Identify common edges
    std::set<Edge> p2_edges;
    if (!parent2.empty()) {
        for (size_t i = 0; i < parent2.size(); ++i) {
            p2_edges.insert(make_edge(parent2[i], parent2[(i + 1) % parent2.size()]));
        }
    }

    std::vector<Edge> common_edges;
    if (!parent1.empty()) {
        for (size_t i = 0; i < parent1.size(); ++i) {
            Edge e = make_edge(parent1[i], parent1[(i + 1) % parent1.size()]);
            // Check if edge is in parent2 AND both nodes are in common_nodes (implicitly true if edge is in both)
            if (p2_edges.count(e)) {
                common_edges.push_back(e);
            }
        }
    }

    // 3. Build subpaths from common edges
    // Adjacency list for common edges restricted to common nodes
    std::map<int, std::vector<int>> adj;
    for (const auto& e : common_edges) {
        adj[e.u].push_back(e.v);
        adj[e.v].push_back(e.u);
    }

    // Identify connected components (subpaths)
    std::vector<std::vector<int>> subpaths;
    std::set<int> visited_in_subpath;
    
    // Explicitly handle nodes involved in common edges
    for (auto const& entry : adj) {
        int node = entry.first;
        // auto neighbors = entry.second; (not strictly needed to copy yet)
        if (visited_in_subpath.count(node)) continue;

        // Start traversing a component
        // Note: A component could be a cycle or a path.
        // In path, start/end nodes have degree 1 (in common graph). In cycle, all degree 2.
        
        // Find a start node for traversal (degree 1 preferred to trace path)
        int start_node = node;
        // Optimization: try to find a degree-1 node in this component to start from
        // BFS/DFS to collect component and find degree 1
        std::vector<int> component;
        std::vector<int> q;
        q.push_back(node);
        std::set<int> comp_visited;
        comp_visited.insert(node);
        
        int head = 0;
        bool has_degree_1 = false;
        int degree_1_node = -1;

        while (head < static_cast<int>(q.size())){
            int curr = q[head++];
            component.push_back(curr);
            if (adj[curr].size() == 1) {
                has_degree_1 = true;
                degree_1_node = curr;
            }
            for(int nbr : adj[curr]){
                if(comp_visited.find(nbr) == comp_visited.end()){
                    comp_visited.insert(nbr);
                    q.push_back(nbr);
                }
            }
        }

        // Trace the path
        std::vector<int> path;
        int curr = has_degree_1 ? degree_1_node : start_node; // If cycle, just pick any
        
        // Simple traversal
        std::vector<int> trace_stack;
        trace_stack.push_back(curr);
        std::set<int> trace_visited;
        trace_visited.insert(curr);

        // We need to trace linearly.
        // It's easier to just greedily follow neighbors
        path.push_back(curr);
        
        // If it is a cycle (no degree 1), we treat it as a path by stopping when we come back or size matched
        // If it is a path, we go until end.
        
        while (path.size() < component.size()) {
            int u = path.back();
            bool found_next = false;
            for (int nbr : adj[u]) {
                if (trace_visited.find(nbr) == trace_visited.end()) {
                    trace_visited.insert(nbr);
                    path.push_back(nbr);
                    found_next = true;
                    break;
                }
            }
            if (!found_next) break; // Should not happen if connected
        }
        
        subpaths.push_back(path);
        for (int n : path) visited_in_subpath.insert(n);
    }

    // 4. Add common nodes that form single-node subpaths (not in any edge)
    for (int node : common_nodes) {
        if (visited_in_subpath.find(node) == visited_in_subpath.end()) {
            subpaths.push_back({node});
            visited_in_subpath.insert(node);
        }
    }

    // 5. Fill with random nodes to reach 50%
    int total_nodes = problem.get_num_points();
    int target_size = static_cast<int>(ceil(total_nodes / 2.0));
    
    // Nodes currently selected
    std::set<int> selected_nodes = common_nodes_set;
    
    // Available nodes
    std::vector<int> available_nodes;
    for (int i = 0; i < total_nodes; ++i) {
        if (selected_nodes.find(i) == selected_nodes.end()) {
            available_nodes.push_back(i);
        }
    }

    // Shuffle available nodes
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(available_nodes.begin(), available_nodes.end(), g);

    // Add until target size
    int needed = target_size - (int)selected_nodes.size();
    if (needed > 0) {
        for (int i = 0; i < needed && i < static_cast<int>(available_nodes.size()); ++i) {
            subpaths.push_back({available_nodes[i]});
        }
    }

    // 6. Connect randomly
    std::shuffle(subpaths.begin(), subpaths.end(), g);
    
    std::vector<int> offspring;
    for (auto& path : subpaths) {
        // Randomly reverse path (including random choice of connected end)
        if (path.size() > 1) {
             if (std::uniform_int_distribution<>(0, 1)(g) == 1) {
                 std::reverse(path.begin(), path.end());
             }
        }
        offspring.insert(offspring.end(), path.begin(), path.end());
    }

    return offspring;
}
