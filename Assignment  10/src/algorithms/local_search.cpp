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
    StageTimer& timer
) {
    std::vector<int> solution = starting_solution;

    timer.start_stage("local search");
    
    // Use fixed arrays instead of vectors
    const int solution_size = solution.size();
    const int data_size = problem_instance.get_num_points();
    const int not_in_solution_size = data_size - solution_size;
    
    int* not_in_solution = new int[not_in_solution_size];
    int* solution_pos = new int[solution_size];
    
    auto rng = std::default_random_engine {};
    std::random_device rd;
    rng.seed(rd());
    
    const double epsilon = 1e-9;

    // Build not_in_solution array
    int not_in_idx = 0;
    for (int i = 0; i < data_size; ++i){
        bool found = false;
        for (int j = 0; j < solution_size; ++j) {
            if (solution[j] == i) {
                found = true;
                break;
            }
        }
        if (!found) {
            not_in_solution[not_in_idx++] = i;
        }
    }

    // Initialize solution_pos array
    for (int i = 0; i < solution_size; ++i){
        solution_pos[i] = i;
    }

    const int inter_limit = solution_size * not_in_solution_size;
    int intra_limit;

    intra_limit = (solution_size < 2) ? 0 : (solution_size * (solution_size - 1) / 2);
    
    while (true) {
        // Shuffle arrays
        std::shuffle(solution_pos, solution_pos + solution_size, rng);
        std::shuffle(not_in_solution, not_in_solution + not_in_solution_size, rng);
        
        int inter_iterator = 0;
        int intra_iterator = 0;
        double best_delta = std::numeric_limits<double>::max();
        
        int best_pos1 = -1, best_pos2_or_id = -1, best_pos_in_not_used = -1;
        NeighbourhoodType best_intra_or_inter = NeighbourhoodType::INTRA;
        bool improving_move_found_greedy = false;
        
        // Browse neighborhood - HOT LOOP WITH NO TIMERS
        while (inter_iterator < inter_limit || intra_iterator < intra_limit){
            
            const bool can_do_intra = intra_iterator < intra_limit;
            const bool can_do_inter = inter_iterator < inter_limit;
            
            if (!can_do_intra && !can_do_inter) break;
            
            NeighbourhoodType intra_or_inter;
            double delta;
            int pos1, pos2_or_id, pos_in_not_used = -1;

            // Decide move type
            if (can_do_intra && can_do_inter) {
                intra_or_inter = (std::uniform_int_distribution<>(0, 1)(rng) == 0) 
                    ? NeighbourhoodType::INTRA : NeighbourhoodType::INTER;
            } else {
                intra_or_inter = can_do_intra ? NeighbourhoodType::INTRA : NeighbourhoodType::INTER;
            }

            // Generate move and calculate delta
            if (intra_or_inter == NeighbourhoodType::INTRA){
                // EDGES_EXCHANGE
                // Inline get_intra_edge_exchange
                int i = intra_iterator;
                int row = 0;
                while (i >= (solution_size - 1 - row)) {
                    i -= (solution_size - 1 - row);
                    row++;
                }
                int col = row + 1 + i;
                pos1 = solution_pos[row];
                pos2_or_id = solution_pos[col];
                
                // Inline intra_edge_exchange delta calculation
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
                // Inline get_inter_node_exchange
                pos1 = solution_pos[inter_iterator / not_in_solution_size];
                pos_in_not_used = inter_iterator % not_in_solution_size;
                pos2_or_id = not_in_solution[pos_in_not_used];
                
                // Inline inter_node_exchange delta calculation
                const int before_node_1 = solution[(pos1 - 1 + solution_size) % solution_size];
                const int after_node_1 = solution[(pos1 + 1) % solution_size];
                const int node_1 = solution[pos1];
                
                delta = problem_instance.get_distance(before_node_1, pos2_or_id) + problem_instance.get_distance(pos2_or_id, after_node_1) + problem_instance.get_point(pos2_or_id).cost
                      - problem_instance.get_distance(before_node_1, node_1) - problem_instance.get_distance(node_1, after_node_1) - problem_instance.get_point(node_1).cost;
                
                inter_iterator++;
            }
            
            // Track best move - inlined comparison
            if (delta < best_delta){
                best_delta = delta;
                best_pos1 = pos1;
                best_pos2_or_id = pos2_or_id;
                best_pos_in_not_used = pos_in_not_used;
                best_intra_or_inter = intra_or_inter;
            }

            // Greedy: apply first improving move
            if (T == SearchType::GREEDY && delta < -epsilon){
                apply_change(intra_or_inter, solution, pos1, pos2_or_id, pos_in_not_used, not_in_solution);
                improving_move_found_greedy = true;
                break;
            }
        }

        if (T == SearchType::GREEDY && improving_move_found_greedy) {
            continue;
        }

        // Check convergence
        if (best_delta >= -epsilon){
            break;
        } 

        // Apply best move for steepest
        if (T == SearchType::STEEPEST){
            apply_change(best_intra_or_inter, solution, best_pos1, best_pos2_or_id, 
                        best_pos_in_not_used, not_in_solution);
        }
    }
    
    timer.end_stage();

    // Cleanup
    delete[] not_in_solution;
    delete[] solution_pos;

    return solution;
}