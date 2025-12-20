#include "local_search.h"

#include <vector>
#include <algorithm>
#include <random>
#include <stdexcept>
#include <limits>
#include <cstring>
#include "../core/stagetimer.h"
#include "inter_node_exchange.h"
#include "intra_edge_exchange.h"
#include <iostream>

/**
 * @brief Helper to precompute candidate neighbors using TSPProblem interface.
 */
std::vector<std::vector<int>> precompute_candidates_internal(
    TSPProblem& problem,
    int K
) {
    int n = problem.get_num_points();
    std::vector<std::vector<int>> candidate_neighbors(n);
    
    for (int i = 0; i < n; ++i) {
        std::vector<std::pair<int, int>> neighbors;
        neighbors.reserve(n - 1);
        
        for (int j = 0; j < n; ++j) {
            if (i != j) {
                // Cost metric: distance + cost of the target node
                int cost = problem.get_distance(i, j) + problem.get_point(j).cost;
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
inline void apply_change(
    NeighbourhoodType intra_or_inter,
    std::vector<int>& solution,
    int pos1,
    int pos2_or_id,
    int pos_in_not_used,
    int* not_in_solution
){
    if (intra_or_inter == NeighbourhoodType::INTRA){
        apply_intra_edge_exchange(solution, pos1, pos2_or_id);
    }
    else{
        not_in_solution[pos_in_not_used] = solution[pos1];
        solution[pos1] = pos2_or_id;
    }
}

std::vector<int> local_search(
    TSPProblem& problem_instance,
    std::vector<int> starting_solution,
    SearchType T,
    StageTimer& timer,
    int k_candidates
) {
    const bool use_candidate_moves = (k_candidates > 0);
    std::vector<int> solution = starting_solution;

    timer.start_stage("local search");
    
    // --- MEMORY ALLOCATION & INITIALIZATION ---
    // Use fixed arrays instead of vectors for efficiency
    const int solution_size = solution.size();
    const int data_size = problem_instance.get_num_points();
    const int not_in_solution_size = data_size - solution_size;
    
    int* not_in_solution = new int[not_in_solution_size];
    int* solution_pos = new int[solution_size]; // Used for shuffling order
    
    // Lookup arrays for O(1) checks (Crucial for Candidate Moves efficiency)
    // - node_to_sol_pos[node_id] = position in solution (0..N-1) or -1 if not in solution
    // - node_to_not_in_pos[node_id] = index in not_in_solution array (0..M-1) or -1
    int* node_to_sol_pos = new int[data_size];
    int* node_to_not_in_pos = new int[data_size];

    // Initialize Lookups
    std::fill(node_to_sol_pos, node_to_sol_pos + data_size, -1);
    std::fill(node_to_not_in_pos, node_to_not_in_pos + data_size, -1);

    // Initialize Lookups based on starting solution
    for (int i = 0; i < solution_size; ++i) {
        node_to_sol_pos[solution[i]] = i;
        solution_pos[i] = i;
    }

    // Build not_in_solution array and lookups
    int not_in_idx = 0;
    for (int i = 0; i < data_size; ++i){
        if (node_to_sol_pos[i] == -1) {
            not_in_solution[not_in_idx] = i;
            node_to_not_in_pos[i] = not_in_idx;
            not_in_idx++;
        }
    }

    // Precompute candidates if requested
    std::vector<std::vector<int>> candidate_neighbors;
    if (use_candidate_moves) {
        timer.end_stage();
        timer.start_stage("precompute candidates");
        candidate_neighbors = precompute_candidates_internal(problem_instance, k_candidates);
        timer.end_stage();
        timer.start_stage("local search");
    }

    auto rng = std::default_random_engine {};
    std::random_device rd;
    rng.seed(rd());
    
    const double epsilon = 1e-9;
    
    // Limits for the efficient iterator-based approach
    const int inter_limit = solution_size * not_in_solution_size;
    const int intra_limit = (solution_size < 2) ? 0 : (solution_size * (solution_size - 1) / 2);

    // --- MAIN OPTIMIZATION LOOP ---
    while (true) {
        double best_delta = std::numeric_limits<double>::max();
        
        int best_pos1 = -1, best_pos2_or_id = -1, best_pos_in_not_used = -1;
        NeighbourhoodType best_intra_or_inter = NeighbourhoodType::INTRA;
        bool improving_move_found_greedy = false;

        // ============================================================
        // BRANCH: CANDIDATE MOVES LOGIC
        // ============================================================
        if (use_candidate_moves) {
            // Logic adapted from local_search_candidate.cpp but using O(1) array lookups
            // Iterates over nodes in solution -> their candidate neighbors
            
            for (int pos1 = 0; pos1 < solution_size; ++pos1) {
                int node1 = solution[pos1];
                int pos1_prev = (pos1 - 1 + solution_size) % solution_size;
                int pos1_next = (pos1 + 1) % solution_size;

                for (int node2 : candidate_neighbors[node1]) {
                    
                    // CHECK 1: Node2 is NOT in solution -> Try INTER exchange
                    if (node_to_sol_pos[node2] == -1) {
                        int pos_in_not_in_sol = node_to_not_in_pos[node2];
                        
                        // Move 1: Add n2, remove n1-1
                        // Inline inter_node_exchange delta calculation
                        {
                            int p_prev = pos1_prev;
                            int before = solution[(p_prev - 1 + solution_size) % solution_size];
                            int after = solution[(p_prev + 1) % solution_size]; // which is node1
                            int removed = solution[p_prev];

                            double delta = problem_instance.get_distance(before, node2) + problem_instance.get_distance(node2, after) + problem_instance.get_point(node2).cost
                                         - problem_instance.get_distance(before, removed) - problem_instance.get_distance(removed, after) - problem_instance.get_point(removed).cost;

                            if (delta < best_delta) {
                                best_delta = delta;
                                best_pos1 = p_prev;
                                best_pos2_or_id = node2;
                                best_pos_in_not_used = pos_in_not_in_sol;
                                best_intra_or_inter = NeighbourhoodType::INTER;
                                if (T == SearchType::GREEDY && delta < -epsilon) goto apply_move;
                            }
                        }

                        // Move 2: Add n2, remove n1+1
                        {
                            int p_next = pos1_next;
                            int before = solution[(p_next - 1 + solution_size) % solution_size]; // which is node1
                            int after = solution[(p_next + 1) % solution_size];
                            int removed = solution[p_next];

                            double delta = problem_instance.get_distance(before, node2) + problem_instance.get_distance(node2, after) + problem_instance.get_point(node2).cost
                                         - problem_instance.get_distance(before, removed) - problem_instance.get_distance(removed, after) - problem_instance.get_point(removed).cost;

                            if (delta < best_delta) {
                                best_delta = delta;
                                best_pos1 = p_next;
                                best_pos2_or_id = node2;
                                best_pos_in_not_used = pos_in_not_in_sol;
                                best_intra_or_inter = NeighbourhoodType::INTER;
                                if (T == SearchType::GREEDY && delta < -epsilon) goto apply_move;
                            }
                        }
                    }
                    // CHECK 2: Node2 IS in solution -> Try INTRA exchange
                    else {
                        int pos2 = node_to_sol_pos[node2];
                        
                        // Skip if adjacent (standard 2-opt restriction)
                        if (pos2 == pos1_next || pos2 == pos1_prev) continue;

                        // Try 2-opt: Remove (pos1, pos1_next) add (pos1, node2)
                        // requires reversal between pos1+1 and pos2
                        if (pos1 < pos2) {
                            // Inline Intra calculation for (pos1, pos2)
                            // This corresponds to breaking (pos1, pos1+1) and (pos2, pos2+1)
                            // And reconnecting (pos1, pos2) and (pos1+1, pos2+1)
                            // Note: local_search.cpp Intra calc might differ slightly in index meaning, 
                            // ensuring standard 2-opt delta:
                            int p1 = pos1;
                            int p2 = pos2;
                            int p1_next = (p1 + 1) % solution_size;
                            int p2_next = (p2 + 1) % solution_size;
                            
                            int n_p1 = solution[p1];
                            int n_p1_next = solution[p1_next];
                            int n_p2 = solution[p2];
                            int n_p2_next = solution[p2_next];

                            double delta = problem_instance.get_distance(n_p1, n_p2) + problem_instance.get_distance(n_p1_next, n_p2_next)
                                         - problem_instance.get_distance(n_p1, n_p1_next) - problem_instance.get_distance(n_p2, n_p2_next);

                            if (delta < best_delta) {
                                best_delta = delta;
                                best_pos1 = pos1;
                                best_pos2_or_id = pos2;
                                best_intra_or_inter = NeighbourhoodType::INTRA;
                                if (T == SearchType::GREEDY && delta < -epsilon) goto apply_move;
                            }
                        }
                        
                        // Also check the "prev" edge case from candidate logic if necessary
                        // (Candidates logic often checks both incoming and outgoing edges for completeness)
                        // Move 2: Remove edge (pos1_prev, pos1) - connect (pos1_prev, pos2)
                        // This effectively acts as the 2-opt starting from prev node.
                        if (pos1_prev != pos2) {
                            int pA, pB;
                            if (pos1_prev < pos2) { pA = pos1_prev; pB = pos2; }
                            else { pA = pos2; pB = pos1_prev; }
                            
                            // Check if valid interval (avoid wrapping issues if not handled by generic swap)
                            // Simple logic: just call the same delta func on these indices
                            int pA_next = (pA + 1) % solution_size;
                            int pB_next = (pB + 1) % solution_size;
                            int n_pA = solution[pA]; int n_pA_next = solution[pA_next];
                            int n_pB = solution[pB]; int n_pB_next = solution[pB_next];
                            
                            double delta = problem_instance.get_distance(n_pA, n_pB) + problem_instance.get_distance(n_pA_next, n_pB_next)
                                         - problem_instance.get_distance(n_pA, n_pA_next) - problem_instance.get_distance(n_pB, n_pB_next);

                             if (delta < best_delta) {
                                best_delta = delta;
                                best_pos1 = pA;
                                best_pos2_or_id = pB;
                                best_intra_or_inter = NeighbourhoodType::INTRA;
                                if (T == SearchType::GREEDY && delta < -epsilon) goto apply_move;
                            }
                        }
                    }
                }
            }
        }
        // ============================================================
        // BRANCH: EFFICIENT ITERATOR (FULL NEIGHBORHOOD)
        // ============================================================
        else {
            // Shuffle arrays for random sampling order
            std::shuffle(solution_pos, solution_pos + solution_size, rng);
            std::shuffle(not_in_solution, not_in_solution + not_in_solution_size, rng);
            
            // Note: When not using candidates, we don't need to update `node_to_not_in_pos`
            // because we iterate `not_in_solution` directly.
            
            int inter_iterator = 0;
            int intra_iterator = 0;

            while (inter_iterator < inter_limit || intra_iterator < intra_limit){
                const bool can_do_intra = intra_iterator < intra_limit;
                const bool can_do_inter = inter_iterator < inter_limit;
                
                if (!can_do_intra && !can_do_inter) break;
                
                NeighbourhoodType intra_or_inter;
                double delta;
                int pos1, pos2_or_id, pos_in_not_used = -1;

                if (can_do_intra && can_do_inter) {
                    intra_or_inter = (std::uniform_int_distribution<>(0, 1)(rng) == 0) 
                        ? NeighbourhoodType::INTRA : NeighbourhoodType::INTER;
                } else {
                    intra_or_inter = can_do_intra ? NeighbourhoodType::INTRA : NeighbourhoodType::INTER;
                }

                if (intra_or_inter == NeighbourhoodType::INTRA){
                    // Decode intra_iterator to (row, col)
                    int i = intra_iterator;
                    int row = 0;
                    while (i >= (solution_size - 1 - row)) {
                        i -= (solution_size - 1 - row);
                        row++;
                    }
                    int col = row + 1 + i;
                    pos1 = solution_pos[row];
                    pos2_or_id = solution_pos[col];
                    
                    const int pos1_plus_1 = (pos1 + 1) % solution_size;
                    const int pos2_plus_1 = (pos2_or_id + 1) % solution_size;
                    
                    if (pos1 == pos2_or_id || pos1_plus_1 == pos2_or_id || pos2_plus_1 == pos1) {
                        delta = 0.0;
                    } else {
                        const int node_i = solution[pos1];
                        const int node_i_plus_1 = solution[pos1_plus_1];
                        const int node_j = solution[pos2_or_id];
                        const int node_j_plus_1 = solution[pos2_plus_1];
                        
                        delta = problem_instance.get_distance(node_i, node_j) + problem_instance.get_distance(node_i_plus_1, node_j_plus_1)
                                - problem_instance.get_distance(node_i, node_i_plus_1) - problem_instance.get_distance(node_j, node_j_plus_1);
                    }
                    intra_iterator++;
                } else {
                    // Decode inter_iterator
                    pos1 = solution_pos[inter_iterator / not_in_solution_size];
                    pos_in_not_used = inter_iterator % not_in_solution_size;
                    pos2_or_id = not_in_solution[pos_in_not_used];
                    
                    const int before_node_1 = solution[(pos1 - 1 + solution_size) % solution_size];
                    const int after_node_1 = solution[(pos1 + 1) % solution_size];
                    const int node_1 = solution[pos1];
                    
                    delta = problem_instance.get_distance(before_node_1, pos2_or_id) + problem_instance.get_distance(pos2_or_id, after_node_1) + problem_instance.get_point(pos2_or_id).cost
                          - problem_instance.get_distance(before_node_1, node_1) - problem_instance.get_distance(node_1, after_node_1) - problem_instance.get_point(node_1).cost;
                    
                    inter_iterator++;
                }
                
                if (delta < best_delta){
                    best_delta = delta;
                    best_pos1 = pos1;
                    best_pos2_or_id = pos2_or_id;
                    best_pos_in_not_used = pos_in_not_used;
                    best_intra_or_inter = intra_or_inter;
                }

                if (T == SearchType::GREEDY && delta < -epsilon){
                    goto apply_move;
                }
            }
        } // End of BRANCH B

    apply_move:
        // Check convergence
        if (best_delta >= -epsilon){
            break;
        } 

        // Apply best move
        apply_change(best_intra_or_inter, solution, best_pos1, best_pos2_or_id, 
                    best_pos_in_not_used, not_in_solution);
        
        // --- UPDATE LOOKUP ARRAYS ---
        // Crucial for Candidate Moves efficiency to maintain O(1) lookups
        if (best_intra_or_inter == NeighbourhoodType::INTER) {
            int added_node = best_pos2_or_id;
            int removed_node = not_in_solution[best_pos_in_not_used]; // This is now the OLD node (swap happened in apply_change)
            // Wait: apply_change SWAPS them.
            // solution[pos1] became best_pos2_or_id (added_node)
            // not_in_solution[pos_in_not_used] became removed_node
            
            node_to_sol_pos[added_node] = best_pos1;
            node_to_sol_pos[removed_node] = -1;
            
            node_to_not_in_pos[removed_node] = best_pos_in_not_used;
            node_to_not_in_pos[added_node] = -1;
        } else {
            // Intra moves (2-opt) reverse a segment.
            // We must update positions for all nodes in the reversed segment.
            // Segment is roughly between pos1 and pos2 (inclusive of elements between)
            // Indices: (best_pos1 + 1) to best_pos2_or_id
            
            int start = (best_pos1 + 1) % solution_size;
            int end = best_pos2_or_id;
            // Handle wrap-around or simple traversal. 
            // Since apply_intra_edge_exchange handles the vector reverse, we just re-scan the changed segment.
            // For simplicity and robustness, we can just scan the whole solution or the affected range.
            // Re-scanning whole solution is O(N) but 2-opt moves are expensive anyway.
            // Optimization: Only scan strictly necessary indices.
            
            int curr = start;
            while (curr != (end + 1) % solution_size) {
                node_to_sol_pos[solution[curr]] = curr;
                curr = (curr + 1) % solution_size;
            }
        }
        
        if (T == SearchType::GREEDY) {
            continue;
        }
    }
    
    timer.end_stage();

    // Cleanup
    delete[] not_in_solution;
    delete[] solution_pos;
    delete[] node_to_sol_pos;
    delete[] node_to_not_in_pos;

    return solution;
}
