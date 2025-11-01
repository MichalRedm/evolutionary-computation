#include "local_search.h"

#include <vector>
#include <algorithm>
#include <random>
#include <stdexcept>
#include <limits>
#include "random_solution.h"
#include "nearest_neighbour_weighted_sum.h"
#include "../core/stagetimer.h"
#include "inter_node_exchange.h"
#include "intra_edge_exchange.h"
#include <iostream>
#include <set>

/**
 * @brief Pre-computes candidate neighbors for each node based on edge cost + node cost.
 * 
 * For each node, finds the K nearest neighbors where "nearest" means the sum of:
 * - The distance to that neighbor
 * - The cost of the neighbor node
 * 
 * @param data The vector of point data.
 * @param distance_matrix The pre-calculated distance matrix.
 * @param K The number of nearest neighbors to store (default 10).
 * @return A vector where candidate_neighbors[i] contains the K nearest neighbor IDs for node i.
 */
std::vector<std::vector<int>> precompute_candidate_neighbors(
    const std::vector<PointData>& data,
    const std::vector<std::vector<int>>& distance_matrix,
    int K = 10
) {
    int n = data.size();
    std::vector<std::vector<int>> candidate_neighbors(n);
    
    for (int i = 0; i < n; ++i) {
        // Create pairs of (cost, node_id) where cost = distance + node_cost
        std::vector<std::pair<int, int>> neighbors;
        neighbors.reserve(n - 1);
        
        for (int j = 0; j < n; ++j) {
            if (i != j) {
                int cost = distance_matrix[i][j] + data[j].cost;
                neighbors.push_back({cost, j});
            }
        }
        
        // Sort by cost (ascending)
        std::sort(neighbors.begin(), neighbors.end());
        
        // Take the K nearest
        int actual_k = std::min(K, (int)neighbors.size());
        candidate_neighbors[i].reserve(actual_k);
        for (int k = 0; k < actual_k; ++k) {
            candidate_neighbors[i].push_back(neighbors[k].second);
        }
    }
    
    return candidate_neighbors;
}

/**
 * @brief Applies a given move to the solution.
 */
void apply_change(
    NeighbourhoodType intra_or_inter,
    std::vector<int>& solution,
    const std::vector<int>& change,
    std::vector<int>& not_in_solution
){
    if (intra_or_inter == NeighbourhoodType::INTRA){
        apply_intra_edge_exchange(solution, change[0], change[1]);
    } else {
        not_in_solution[change[2]] = solution[change[0]];
        solution[change[0]] = change[1];
    }
}

/**
 * @brief Performs a local search with candidate moves to improve efficiency.
 * 
 * This implementation only evaluates moves that introduce at least one candidate edge,
 * dramatically reducing the number of moves evaluated while maintaining solution quality.
 */
std::vector<int> local_search(
    const std::vector<PointData>& data,
    std::vector<std::vector<int>>& distance_matrix,
    StartingSolutionType S,
    StageTimer& timer,
    int greedy_start_node_id
) {
    std::vector<int> solution;

    timer.start_stage("initial solution");
    if (S == StartingSolutionType::RANDOM) {
        solution = generate_random_solution(data);
    } else if (S == StartingSolutionType::GREEDY) {
        solution = nearest_neighbour_weighted_sum(data, distance_matrix, greedy_start_node_id);
    }
    timer.end_stage();

    // Pre-compute candidate neighbors (done once)
    timer.start_stage("precompute candidates");
    std::vector<std::vector<int>> candidate_neighbors = precompute_candidate_neighbors(data, distance_matrix, 10);
    timer.end_stage();

    timer.start_stage("local traversing");
    
    std::vector<int> not_in_solution = {};
    auto rng = std::default_random_engine {};
    std::random_device rd;
    rng.seed(rd());

    int solution_size = solution.size();
    double best_delta;
    std::vector<int> best_change;
    NeighbourhoodType best_intra_or_inter = NeighbourhoodType::INTER;
    const double epsilon = 1e-9;

    // Build not_in_solution list and a position lookup map
    std::set<int> solution_set(solution.begin(), solution.end());
    std::vector<int> node_to_position(data.size(), -1); // Maps node ID to position in solution (-1 if not in solution)
    
    for (int i = 0; i < int(data.size()); ++i){
        if (solution_set.find(i) == solution_set.end()){ 
            not_in_solution.push_back(i);
        }
    }
    
    // Build position lookup
    for (int pos = 0; pos < solution_size; ++pos) {
        node_to_position[solution[pos]] = pos;
    }

    while (true) {
        best_delta = std::numeric_limits<double>::max();
        best_change.clear();
        
        // ============================================================
        // INTER-ROUTE CANDIDATE MOVES
        // ============================================================
        // For each node in the solution, try swapping with its candidate neighbors that are NOT in solution
        for (int pos = 0; pos < solution_size; ++pos) {
            int node_in_sol = solution[pos];
            
            // Iterate over candidate neighbors of this node
            for (int candidate_neighbor : candidate_neighbors[node_in_sol]) {
                // Check if this candidate is not in solution
                if (solution_set.find(candidate_neighbor) == solution_set.end()) {
                    // Find position in not_in_solution
                    auto it = std::find(not_in_solution.begin(), not_in_solution.end(), candidate_neighbor);
                    if (it != not_in_solution.end()) {
                        int pos_in_not_in_sol = std::distance(not_in_solution.begin(), it);
                        
                        // Evaluate this inter move
                        double delta = inter_node_exchange(data, distance_matrix, solution, pos, candidate_neighbor);
                        
                        if (delta < best_delta) {
                            best_delta = delta;
                            best_change = {pos, candidate_neighbor, pos_in_not_in_sol};
                            best_intra_or_inter = NeighbourhoodType::INTER;
                        }
                    }
                }
            }
        }
        
        // Also consider the reverse: nodes NOT in solution whose candidate neighbors ARE in solution
        for (int i = 0; i < (int)not_in_solution.size(); ++i) {
            int node_not_in_sol = not_in_solution[i];
            
            for (int candidate_neighbor : candidate_neighbors[node_not_in_sol]) {
                // Check if this candidate IS in solution
                if (solution_set.find(candidate_neighbor) != solution_set.end()) {
                    // Find position in solution
                    auto it = std::find(solution.begin(), solution.end(), candidate_neighbor);
                    if (it != solution.end()) {
                        int pos = std::distance(solution.begin(), it);
                        
                        // Evaluate this inter move
                        double delta = inter_node_exchange(data, distance_matrix, solution, pos, node_not_in_sol);
                        
                        if (delta < best_delta) {
                            best_delta = delta;
                            best_change = {pos, node_not_in_sol, i};
                            best_intra_or_inter = NeighbourhoodType::INTER;
                        }
                    }
                }
            }
        }
        
        // ============================================================
        // INTRA-ROUTE CANDIDATE MOVES (EDGE EXCHANGE)
        // ============================================================
        // Following the lecture approach:
        // For each vertex n1 from 0 to N-1
        //     For each vertex n2 from the list of the closest vertices to n1
        //         Evaluate all (two) moves involving the addition of edge n1-n2 
        //         and the removal of one of the edges adjacent to n1
        
        for (int pos1 = 0; pos1 < solution_size; ++pos1) {
            int node1 = solution[pos1];
            int pos1_next = (pos1 + 1) % solution_size;
            int pos1_prev = (pos1 - 1 + solution_size) % solution_size;
            
            // For each candidate neighbor of node1
            for (int node2 : candidate_neighbors[node1]) {
                // Use position lookup instead of linear search
                int pos2 = node_to_position[node2];
                
                if (pos2 == -1) continue; // node2 not in solution
                
                // Skip if adjacent (2-opt doesn't make sense for adjacent positions)
                if (pos2 == pos1_next || pos2 == pos1_prev) continue;
                
                // In a TSP tour, node1 is at pos1 with edges to:
                // - node at pos1_prev (predecessor)
                // - node at pos1_next (successor)
                //
                // A 2-opt move that adds edge (node1, node2) can be done in TWO ways:
                //
                // Move 1: Remove edge (pos1, pos1_next) - standard 2-opt at (pos1, pos2)
                // Move 2: Remove edge (pos1_prev, pos1) - 2-opt at (pos1_prev, pos2)
                
                // Move 1: Remove edge (pos1, pos1_next)
                if (pos1 < pos2) {
                    double delta = intra_edge_exchange(distance_matrix, solution, pos1, pos2);
                    if (delta < best_delta) {
                        best_delta = delta;
                        best_change = {pos1, pos2};
                        best_intra_or_inter = NeighbourhoodType::INTRA;
                    }
                }
                
                // Move 2: Remove edge (pos1_prev, pos1)
                if (pos1_prev != pos2) {
                    if (pos1_prev < pos2) {
                        double delta = intra_edge_exchange(distance_matrix, solution, pos1_prev, pos2);
                        if (delta < best_delta) {
                            best_delta = delta;
                            best_change = {pos1_prev, pos2};
                            best_intra_or_inter = NeighbourhoodType::INTRA;
                        }
                    } else if (pos2 < pos1_prev) {
                        double delta = intra_edge_exchange(distance_matrix, solution, pos2, pos1_prev);
                        if (delta < best_delta) {
                            best_delta = delta;
                            best_change = {pos2, pos1_prev};
                            best_intra_or_inter = NeighbourhoodType::INTRA;
                        }
                    }
                }
            }
        }

        // Steepest Descent: if no improving move, we're at a local optimum
        if (best_delta >= -epsilon){
            break;
        } 

        // Apply the best move found
        apply_change(best_intra_or_inter, solution, best_change, not_in_solution);
        
        // Update data structures for next iteration
        if (best_intra_or_inter == NeighbourhoodType::INTER) {
            // Update solution_set
            int removed_node = not_in_solution[best_change[2]];
            int added_node = best_change[1];
            solution_set.erase(removed_node);
            solution_set.insert(added_node);
            
            // Update node_to_position
            node_to_position[removed_node] = -1;
            node_to_position[added_node] = best_change[0];
        } else {
            // For INTRA moves, positions changed due to segment reversal
            // Rebuild the position map (relatively cheap)
            for (int pos = 0; pos < solution_size; ++pos) {
                node_to_position[solution[pos]] = pos;
            }
        }

    } // End main optimization loop

    timer.end_stage();

    return solution;
}
