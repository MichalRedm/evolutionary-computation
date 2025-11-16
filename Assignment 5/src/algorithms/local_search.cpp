#include "local_search.h"

#include <vector>
#include <algorithm>
#include <random>
#include <stdexcept>
#include <limits>
#include "random_solution.h"
#include "../core/stagetimer.h"
#include "../core/evaluation.h"
#include "inter_node_exchange.h"
#include "intra_edge_exchange.h"
#include "neighborhood_utils.h"

// Structure to represent an intra-route move (edge exchange)
struct IntraMove {
    double delta;
    int node_i;
    int node_i_plus_1;
    int node_j;
    int node_j_plus_1;
    
    IntraMove(double d, int ni, int nip1, int nj, int njp1)
        : delta(d), node_i(ni), node_i_plus_1(nip1), 
          node_j(nj), node_j_plus_1(njp1) {}
};

// Structure to represent an inter-route move (node exchange)
// We store neighbors to verify if context hasn't changed
struct InterMove {
    double delta;
    int node_in_solution;
    int node_before;  // Node before node_in_solution in the cycle
    int node_after;   // Node after node_in_solution in the cycle
    int node_outside;
    
    InterMove(double d, int nis, int before, int after, int no)
        : delta(d), node_in_solution(nis), node_before(before), 
          node_after(after), node_outside(no) {}
};

/**
 * @brief Check if inter move context is still valid
 * 
 * Checks if the node_in_solution still has the same neighbors (context)
 * as when the delta was calculated.
 * 
 * @return true if context unchanged (delta is valid), false otherwise
 */
bool check_inter_move_context(const std::vector<int>& solution, const InterMove& move) {
    const int solution_size = solution.size();
    
    // Find position of node_in_solution
    int current_pos = -1;
    for (int i = 0; i < solution_size; ++i) {
        if (solution[i] == move.node_in_solution) {
            current_pos = i;
            break;
        }
    }
    
    if (current_pos == -1) return false; // Node not in solution
    
    // Check if neighbors are the same
    int current_before = solution[(current_pos - 1 + solution_size) % solution_size];
    int current_after = solution[(current_pos + 1) % solution_size];
    
    return (current_before == move.node_before && current_after == move.node_after);
}
/**
 * @brief Check if edges exist in current solution with same orientation
 */
int check_edge_orientation(const std::vector<int>& solution, const IntraMove& move) {
    const int solution_size = solution.size();
    
    int current_pos_i = -1;
    for (int i = 0; i < solution_size; ++i) {
        if (solution[i] == move.node_i) {
            current_pos_i = i;
            break;
        }
    }
    
    if (current_pos_i == -1) return 0;
    
    int next_i = solution[(current_pos_i + 1) % solution_size];
    int prev_i = solution[(current_pos_i - 1 + solution_size) % solution_size];
    
    int current_pos_j = -1;
    for (int i = 0; i < solution_size; ++i) {
        if (solution[i] == move.node_j) {
            current_pos_j = i;
            break;
        }
    }
    
    if (current_pos_j == -1) return 0;
    
    int next_j = solution[(current_pos_j + 1) % solution_size];
    int prev_j = solution[(current_pos_j - 1 + solution_size) % solution_size];
    
    bool edge1_exists = (next_i == move.node_i_plus_1);
    bool edge1_reversed = (prev_i == move.node_i_plus_1);
    bool edge2_exists = (next_j == move.node_j_plus_1);
    bool edge2_reversed = (prev_j == move.node_j_plus_1);
    
    if (!edge1_exists && !edge1_reversed) return 0;
    if (!edge2_exists && !edge2_reversed) return 0;
    
    if (edge1_exists && edge2_exists) return 1;
    if (edge1_reversed && edge2_reversed) return 1;
    
    return -1;
}

/**
 * @brief Evaluate all improving moves
 */
std::pair<std::vector<IntraMove>, std::vector<InterMove>> evaluate_all_moves(
    TSPProblem& problem_instance,
    const std::vector<int>& solution,
    const std::vector<int>& not_in_solution
) {
    const int solution_size = solution.size();
    const double epsilon = 1e-9;
    
    std::vector<IntraMove> intra_moves;
    std::vector<InterMove> inter_moves;
    
    // Evaluate all intra-route moves
    if (solution_size >= 2) {
        for (int i = 0; i < solution_size; ++i) {
            for (int j = i + 1; j < solution_size; ++j) {
                double delta = intra_edge_exchange(problem_instance, solution, i, j);
                
                if (delta < -epsilon) {
                    int pos1_plus_1 = (i + 1) % solution_size;
                    int pos2_plus_1 = (j + 1) % solution_size;
                    
                    intra_moves.emplace_back(
                        delta,
                        solution[i], solution[pos1_plus_1],
                        solution[j], solution[pos2_plus_1]
                    );
                }
            }
        }
    }
    
    // Evaluate all inter-route moves
    for (size_t sol_idx = 0; sol_idx < solution.size(); ++sol_idx) {
        // Get neighbors of the node at sol_idx
        int node_in_sol = solution[sol_idx];
        int node_before = solution[(sol_idx - 1 + solution_size) % solution_size];
        int node_after = solution[(sol_idx + 1) % solution_size];
        
        for (size_t out_idx = 0; out_idx < not_in_solution.size(); ++out_idx) {
            double delta = inter_node_exchange(
                problem_instance, 
                const_cast<std::vector<int>&>(solution), 
                sol_idx, 
                not_in_solution[out_idx]
            );
            
            if (delta < -epsilon) {
                inter_moves.emplace_back(
                    delta,
                    node_in_sol,
                    node_before,
                    node_after,
                    not_in_solution[out_idx]
                );
            }
        }
    }
    
    return {intra_moves, inter_moves};
}

std::vector<int> local_search(
    TSPProblem& problem_instance,
    std::vector<int> starting_solution,
    SearchType T,
    StageTimer& timer
) {
    // GREEDY implementation - unchanged
    if (T == SearchType::GREEDY) {
        std::vector<int> solution = starting_solution;
        timer.start_stage("local traversing");
        
        std::vector<int> not_in_solution = {};
        auto rng = std::default_random_engine {};
        std::random_device rd;
        rng.seed(rd());

        int inter_iterator;
        int intra_iterator;
        int solution_size = solution.size();
        std::vector<int> solution_pos = {};
        double delta;
        std::vector<int> change;
        NeighbourhoodType intra_or_inter;
        const double epsilon = 1e-9;

        for (int i = 0; i < problem_instance.get_num_points(); ++i){
            if(!(std::find(solution.begin(), solution.end(), i) != solution.end())){ 
                not_in_solution.push_back(i);
            }
        }

        for (int i = 0; i < solution_size; ++i){
            solution_pos.push_back(i);
        }

        const int inter_limit = solution_size * not_in_solution.size();
        int intra_limit;
        if (solution_size < 2) {
            intra_limit = 0;
        } else {
            intra_limit = solution_size * (solution_size - 1) / 2;
        }

        while (true) {
            std::shuffle(std::begin(solution_pos), std::end(solution_pos), rng);
            std::shuffle(std::begin(not_in_solution), std::end(not_in_solution), rng);
            
            inter_iterator = 0;
            intra_iterator = 0;
            bool improving_move_found_greedy = false;
            
            while (inter_iterator < inter_limit || intra_iterator < intra_limit){
                bool can_do_intra = intra_iterator < intra_limit;
                bool can_do_inter = inter_iterator < inter_limit;

                if (can_do_intra && can_do_inter) {
                    if (std::uniform_int_distribution<>(0, 1)(rng) == 0) {
                        intra_or_inter = NeighbourhoodType::INTRA;
                    } else {
                        intra_or_inter = NeighbourhoodType::INTER;
                    }
                } else if (can_do_intra) {
                    intra_or_inter = NeighbourhoodType::INTRA;
                } else if (can_do_inter) {
                    intra_or_inter = NeighbourhoodType::INTER;
                } else {
                    break;
                }

                if (intra_or_inter == NeighbourhoodType::INTRA){
                    std::vector<int> indices = get_intra_edge_exchange(intra_iterator, solution_size);
                    int pos1 = solution_pos[indices[0]];
                    int pos2 = solution_pos[indices[1]];
                    delta = intra_edge_exchange(problem_instance, solution, pos1, pos2);
                    change = {pos1, pos2};
                    intra_iterator++;
                } else {
                    change = get_inter_node_exchange(not_in_solution, solution_pos, inter_iterator, solution_size);
                    delta = inter_node_exchange(problem_instance, solution, change[0], change[1]);
                    inter_iterator++;
                }

                if (delta < -epsilon){
                    apply_change(intra_or_inter, solution, change, not_in_solution);
                    improving_move_found_greedy = true;
                    break;
                }
            }

            if (improving_move_found_greedy) {
                continue;
            }

            break;
        }

        timer.end_stage();
        return solution;
    }
    
    // STEEPEST implementation with LM
    std::vector<int> solution = starting_solution;
    timer.start_stage("local traversing");
    
    std::vector<int> not_in_solution;
    const double epsilon = 1e-9;
    
    for (int i = 0; i < problem_instance.get_num_points(); ++i) {
        if (std::find(solution.begin(), solution.end(), i) == solution.end()) {
            not_in_solution.push_back(i);
        }
    }
    
    // Initial evaluation - populate LM with all improving moves
    auto [intra_move_list, inter_move_list] = evaluate_all_moves(
        problem_instance, solution, not_in_solution
    );
    
    // Main loop
    while (true) {
        double best_delta = 0.0;
        bool best_is_intra = false;
        int best_intra_idx = -1;
        int best_inter_idx = -1;
        
        // Browse LM to find best applicable move
        // For INTRA moves: delta stays valid if edges exist in same orientation
        for (size_t i = 0; i < intra_move_list.size(); ) {
            const IntraMove& move = intra_move_list[i];
            int orientation = check_edge_orientation(solution, move);
            
            if (orientation == 0) {
                // Edges don't exist - remove from LM
                intra_move_list.erase(intra_move_list.begin() + i);
                continue;
            } else if (orientation == -1) {
                // Different orientation - not applicable now, keep in LM
                ++i;
                continue;
            }
            
            // orientation == 1: applicable
            // For intra moves, delta is still valid (depends only on 4 nodes)
            if (move.delta < best_delta) {
                best_delta = move.delta;
                best_is_intra = true;
                best_intra_idx = i;
            }
            ++i;
        }
        
        // For INTER moves: check if context (neighbors) hasn't changed
        for (size_t i = 0; i < inter_move_list.size(); ) {
            const InterMove& move = inter_move_list[i];
            
            // Check if context is still valid (neighbors haven't changed)
            if (!check_inter_move_context(solution, move)) {
                // Context changed - remove from LM
                inter_move_list.erase(inter_move_list.begin() + i);
                continue;
            }
            
            // Check if outside node available
            auto out_it = std::find(not_in_solution.begin(), not_in_solution.end(), move.node_outside);
            if (out_it == not_in_solution.end()) {
                inter_move_list.erase(inter_move_list.begin() + i);
                continue;
            }
            
            // Context is valid - delta is still correct!
            if (move.delta < best_delta) {
                best_delta = move.delta;
                best_is_intra = false;
                best_inter_idx = i;
            }
            ++i;
        }
        
        // If found improving move in LM, apply it
        if (best_is_intra && best_intra_idx >= 0) {
            const IntraMove& move = intra_move_list[best_intra_idx];
            
            // Find current positions
            int current_pos1 = -1, current_pos2 = -1;
            for (size_t j = 0; j < solution.size(); ++j) {
                if (solution[j] == move.node_i) current_pos1 = j;
                if (solution[j] == move.node_j) current_pos2 = j;
            }
            
            apply_intra_edge_exchange(solution, current_pos1, current_pos2);
            intra_move_list.erase(intra_move_list.begin() + best_intra_idx);
            
            // Continue with updated solution
            continue;
            
        } else if (!best_is_intra && best_inter_idx >= 0) {
            const InterMove& move = inter_move_list[best_inter_idx];
            
            // Find position of node_in_solution
            int sol_pos = -1;
            for (size_t j = 0; j < solution.size(); ++j) {
                if (solution[j] == move.node_in_solution) {
                    sol_pos = j;
                    break;
                }
            }
            
            // Find position of node_outside in not_in_solution
            auto out_it = std::find(not_in_solution.begin(), not_in_solution.end(), move.node_outside);
            int out_pos = std::distance(not_in_solution.begin(), out_it);
            
            // Apply the exchange
            not_in_solution[out_pos] = solution[sol_pos];
            solution[sol_pos] = move.node_outside;
            
            inter_move_list.erase(inter_move_list.begin() + best_inter_idx);
            
            // Continue with updated solution
            continue;
        }
        
        // LM exhausted - re-evaluate all moves
        auto [new_intra, new_inter] = evaluate_all_moves(
            problem_instance, solution, not_in_solution
        );
        
        // If no improving moves, local optimum reached
        if (new_intra.empty() && new_inter.empty()) {
            break;
        }
        
        // Find and apply best move
        best_delta = 0.0;
        best_intra_idx = -1;
        best_inter_idx = -1;
        
        for (size_t i = 0; i < new_intra.size(); ++i) {
            if (new_intra[i].delta < best_delta) {
                best_delta = new_intra[i].delta;
                best_intra_idx = i;
            }
        }
        
        for (size_t i = 0; i < new_inter.size(); ++i) {
            if (new_inter[i].delta < best_delta) {
                best_delta = new_inter[i].delta;
                best_intra_idx = -1;
                best_inter_idx = i;
            }
        }
        
        if (best_intra_idx >= 0) {
            const IntraMove& move = new_intra[best_intra_idx];
            
            int current_pos1 = -1, current_pos2 = -1;
            for (size_t i = 0; i < solution.size(); ++i) {
                if (solution[i] == move.node_i) current_pos1 = i;
                if (solution[i] == move.node_j) current_pos2 = i;
            }
            apply_intra_edge_exchange(solution, current_pos1, current_pos2);
            
            new_intra.erase(new_intra.begin() + best_intra_idx);
            
        } else if (best_inter_idx >= 0) {
            const InterMove& move = new_inter[best_inter_idx];
            
            // Find position of node_in_solution
            int sol_pos = -1;
            for (size_t i = 0; i < solution.size(); ++i) {
                if (solution[i] == move.node_in_solution) {
                    sol_pos = i;
                    break;
                }
            }
            
            // Find position of node_outside
            auto out_it = std::find(not_in_solution.begin(), not_in_solution.end(), move.node_outside);
            
            // Apply the exchange
            not_in_solution[std::distance(not_in_solution.begin(), out_it)] = solution[sol_pos];
            solution[sol_pos] = move.node_outside;
            
            new_inter.erase(new_inter.begin() + best_inter_idx);
        }
        
        // Repopulate LM with remaining improving moves
        intra_move_list = std::move(new_intra);
        inter_move_list = std::move(new_inter);
    }
    
    timer.end_stage();
    return solution;
}
