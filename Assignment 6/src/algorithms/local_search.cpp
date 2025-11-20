#include "local_search.h"

#include <vector>
#include <algorithm>
#include <random>
#include <stdexcept>
#include <limits>
#include <set>
#include <map>
#include "random_solution.h"
#include "../core/stagetimer.h"
#include "../core/evaluation.h"
#include "inter_node_exchange.h"
#include "intra_edge_exchange.h"
#include "neighborhood_utils.h"

// Enum to differentiate move types
enum class MoveType {
    INTRA,
    INTER
};

// Forward declaration
struct MoveKey;

// Structure to represent a move
struct Move {
    double delta;
    MoveType type;
    
    // For INTRA moves (2-opt edge exchange)
    int node_i;
    int node_i_plus_1;
    int node_j;
    int node_j_plus_1;
    
    // For INTER moves (node exchange)
    int node_in_solution;
    int node_outside;
    
    // Constructor for intra moves
    Move(double d, int ni, int nip1, int nj, int njp1)
        : delta(d), type(MoveType::INTRA),
          node_i(ni), node_i_plus_1(nip1), 
          node_j(nj), node_j_plus_1(njp1),
          node_in_solution(-1), node_outside(-1) {}
    
    // Constructor for inter moves
    Move(double d, int nis, int no)
        : delta(d), type(MoveType::INTER),
          node_i(-1), node_i_plus_1(-1),
          node_j(-1), node_j_plus_1(-1),
          node_in_solution(nis), node_outside(no) {}

    // Default constructor
    Move() : delta(0.0), type(MoveType::INTRA),
             node_i(-1), node_i_plus_1(-1),
             node_j(-1), node_j_plus_1(-1),
             node_in_solution(-1), node_outside(-1) {}
    
    // For sorted container - sort by delta, then by key for stability
    bool operator<(const Move& other) const {
        if (std::abs(delta - other.delta) > 1e-9) {
            return delta < other.delta;
        }
        // For equal deltas, compare by move details for stable ordering
        if (type != other.type) return type < other.type;
        if (type == MoveType::INTRA) {
            if (node_i != other.node_i) return node_i < other.node_i;
            if (node_i_plus_1 != other.node_i_plus_1) return node_i_plus_1 < other.node_i_plus_1;
            if (node_j != other.node_j) return node_j < other.node_j;
            return node_j_plus_1 < other.node_j_plus_1;
        } else {
            if (node_in_solution != other.node_in_solution) return node_in_solution < other.node_in_solution;
            return node_outside < other.node_outside;
        }
    }
};

// Unique, order-independent identifier for a move
struct MoveKey {
    MoveType type;
    int a, b, c, d;
    
    MoveKey(const Move& move) {
        type = move.type;
        if (type == MoveType::INTRA) {
            int edge1_a = std::min(move.node_i, move.node_i_plus_1);
            int edge1_b = std::max(move.node_i, move.node_i_plus_1);
            int edge2_a = std::min(move.node_j, move.node_j_plus_1);
            int edge2_b = std::max(move.node_j, move.node_j_plus_1);
            
            if (edge1_a < edge2_a || (edge1_a == edge2_a && edge1_b < edge2_b)) {
                a = edge1_a; b = edge1_b; c = edge2_a; d = edge2_b;
            } else {
                a = edge2_a; b = edge2_b; c = edge1_a; d = edge1_b;
            }
        } else {
            a = std::min(move.node_in_solution, move.node_outside);
            b = std::max(move.node_in_solution, move.node_outside);
            c = -1; d = -1;
        }
    }

    MoveKey(MoveType t, int n1, int n2, int n3 = -1, int n4 = -1) {
        type = t;
        if (type == MoveType::INTRA) {
            int edge1_a = std::min(n1, n2);
            int edge1_b = std::max(n1, n2);
            int edge2_a = std::min(n3, n4);
            int edge2_b = std::max(n3, n4);
            if (edge1_a < edge2_a || (edge1_a == edge2_a && edge1_b < edge2_b)) {
                a = edge1_a; b = edge1_b; c = edge2_a; d = edge2_b;
            } else {
                a = edge2_a; b = edge2_b; c = edge1_a; d = edge1_b;
            }
        } else {
            a = std::min(n1, n2);
            b = std::max(n1, n2);
            c = -1; d = -1;
        }
    }
    
    bool operator<(const MoveKey& other) const {
        if (type != other.type) return type < other.type;
        if (a != other.a) return a < other.a;
        if (b != other.b) return b < other.b;
        if (c != other.c) return c < other.c;
        return d < other.d;
    }
};

// Dual data structure for efficient move management
class MoveManager {
private:
    std::map<MoveKey, Move> move_map;           // For O(log n) lookup/update by key
    std::multiset<Move> sorted_moves;            // For O(log n) access to best move
    const double epsilon = 1e-9;

public:
    // Insert or update a move
    void update(const Move& move) {
        MoveKey key(move);
        
        // Check if move already exists
        auto it = move_map.find(key);
        if (it != move_map.end()) {
            // Remove old version from sorted set
            sorted_moves.erase(sorted_moves.find(it->second));
        }
        
        // Insert new version
        move_map[key] = move;
        sorted_moves.insert(move);
    }
    
    // Remove a move by key
    void remove(const MoveKey& key) {
        auto it = move_map.find(key);
        if (it != move_map.end()) {
            sorted_moves.erase(sorted_moves.find(it->second));
            move_map.erase(it);
        }
    }
    
    // Get iterator to best (smallest delta) move
    std::multiset<Move>::iterator begin_sorted() {
        return sorted_moves.begin();
    }
    
    std::multiset<Move>::iterator end_sorted() {
        return sorted_moves.end();
    }
    
    // Check if any improving moves exist
    bool has_improving_move() const {
        return !sorted_moves.empty() && sorted_moves.begin()->delta < -epsilon;
    }
    
    size_t size() const {
        return move_map.size();
    }
    
    void clear() {
        move_map.clear();
        sorted_moves.clear();
    }
};

const double epsilon = 1e-9;

// Check if intra move edges exist with same orientation
int check_edge_orientation(const std::vector<int>& solution, const Move& move) {
    const int solution_size = solution.size();
    if (solution_size < 3) return 0;

    int current_pos_i = -1;
    for (int i = 0; i < solution_size; ++i) {
        if (solution[i] == move.node_i) {
            current_pos_i = i;
            break;
        }
    }
    if (current_pos_i == -1) return 0;
    
    int next_i_node = solution[(current_pos_i + 1) % solution_size];
    int prev_i_node = solution[(current_pos_i - 1 + solution_size) % solution_size];
    
    int current_pos_j = -1;
    for (int i = 0; i < solution_size; ++i) {
        if (solution[i] == move.node_j) {
            current_pos_j = i;
            break;
        }
    }
    if (current_pos_j == -1) return 0;

    int next_j_node = solution[(current_pos_j + 1) % solution_size];
    int prev_j_node = solution[(current_pos_j - 1 + solution_size) % solution_size];
    
    bool edge1_exists = (next_i_node == move.node_i_plus_1);
    bool edge1_reversed = (prev_i_node == move.node_i_plus_1);
    bool edge2_exists = (next_j_node == move.node_j_plus_1);
    bool edge2_reversed = (prev_j_node == move.node_j_plus_1);
    
    if (!edge1_exists && !edge1_reversed) return 0;
    if (!edge2_exists && !edge2_reversed) return 0;
    
    if (edge1_exists && edge2_exists) return 1;
    if (edge1_reversed && edge2_reversed) return 1;
    
    return -1;
}

// Check if inter move is applicable
bool check_inter_move_applicable(const std::vector<int>& solution, 
                                  const std::vector<int>& not_in_solution,
                                  const Move& move) {
    bool in_sol = false;
    for (size_t i = 0; i < solution.size(); ++i) {
        if (solution[i] == move.node_in_solution) {
            in_sol = true;
            break;
        }
    }
    if (!in_sol) return false;
    
    bool outside = false;
    for (size_t i = 0; i < not_in_solution.size(); ++i) {
        if (not_in_solution[i] == move.node_outside) {
            outside = true;
            break;
        }
    }
    if (!outside) return false;
    
    return true;
}

// Re-evaluate and update intra move
void reevaluate_and_update_intra(
    TSPProblem& problem_instance,
    const std::vector<int>& solution,
    int pos1, int pos2,
    MoveManager& LM
) {
    const int solution_size = solution.size();
    if (solution_size < 3) return;
    if (pos1 == pos2) return;
    
    double delta = intra_edge_exchange(problem_instance, solution, pos1, pos2);
    
    int pos1_plus_1 = (pos1 + 1) % solution_size;
    int pos2_plus_1 = (pos2 + 1) % solution_size;
    
    Move move(delta, solution[pos1], solution[pos1_plus_1],
              solution[pos2], solution[pos2_plus_1]);
    
    LM.update(move);
}

// Re-evaluate and update inter move
void reevaluate_and_update_inter(
    TSPProblem& problem_instance,
    const std::vector<int>& solution,
    const std::vector<int>& not_in_solution,
    int sol_idx, int out_node_id,
    MoveManager& LM
) {
    double delta = inter_node_exchange(problem_instance, 
                                      const_cast<std::vector<int>&>(solution),
                                      sol_idx, out_node_id);
    
    Move move(delta, solution[sol_idx], out_node_id);
    LM.update(move);
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
    
    // STEEPEST implementation with MoveManager
    std::vector<int> solution = starting_solution;
    timer.start_stage("local traversing");
    
    std::vector<int> not_in_solution;
    const int solution_size = solution.size();
    
    // Build not_in_solution list
    for (int i = 0; i < problem_instance.get_num_points(); ++i) {
        bool in_solution = false;
        for (int j = 0; j < solution_size; ++j) {
            if (solution[j] == i) {
                in_solution = true;
                break;
            }
        }
        if (!in_solution) {
            not_in_solution.push_back(i);
        }
    }
    
    // MoveManager: combines map for updates and multiset for sorted access
    MoveManager LM;
    
    // Initial evaluation: evaluate all moves
    if (solution_size >= 2) {
        for (int i = 0; i < solution_size; ++i) {
            for (int j = i + 1; j < solution_size; ++j) {
                reevaluate_and_update_intra(problem_instance, solution, i, j, LM);
            }
        }
    }
    
    for (int sol_idx = 0; sol_idx < solution_size; ++sol_idx) {
        for (size_t out_idx = 0; out_idx < not_in_solution.size(); ++out_idx) {
            reevaluate_and_update_inter(problem_instance, solution, not_in_solution,
                                       sol_idx, not_in_solution[out_idx], LM);
        }
    }
    
    // Main loop
    while (LM.has_improving_move()) {
        bool move_found = false;
        Move best_move;
        
        // Iterate through sorted moves (best first)
        for (auto it = LM.begin_sorted(); it != LM.end_sorted(); ++it) {
            const Move& move = *it;
            
            // Stop if we've passed all improving moves
            if (move.delta >= -epsilon) break;
            
            if (move.type == MoveType::INTRA) {
                int orientation = check_edge_orientation(solution, move);
                if (orientation == 0) {
                    // Edges don't exist - mark for removal
                    LM.remove(MoveKey(move));
                    continue;
                } else if (orientation == -1) {
                    // Different orientation - not applicable now
                    continue;
                }
                // orientation == 1: applicable
                best_move = move;
                move_found = true;
                break;
            } else {
                // INTER move
                if (!check_inter_move_applicable(solution, not_in_solution, move)) {
                    // Move is invalid - mark for removal
                    LM.remove(MoveKey(move));
                    continue;
                }
                best_move = move;
                move_found = true;
                break;
            }
        }
        
        if (!move_found) {
            break; // Local optimum reached
        }
        
        // Apply the best applicable move
        if (best_move.type == MoveType::INTRA) {
            // Apply intra move
            int current_pos1 = -1, current_pos2 = -1;
            for (int i = 0; i < solution_size; ++i) {
                if (solution[i] == best_move.node_i) current_pos1 = i;
                if (solution[i] == best_move.node_j) current_pos2 = i;
            }
            
            if (current_pos1 == -1 || current_pos2 == -1) continue;
            
            apply_intra_edge_exchange(solution, current_pos1, current_pos2);
            
            // Identify 4 affected nodes
            int n_i = best_move.node_i;
            int n_i_plus_1 = best_move.node_i_plus_1;
            int n_j = best_move.node_j;
            int n_j_plus_1 = best_move.node_j_plus_1;
            
            // Find their new positions
            int pos_i = -1, pos_i_plus_1 = -1, pos_j = -1, pos_j_plus_1 = -1;
            for(int i=0; i < solution_size; ++i) {
                if (solution[i] == n_i) pos_i = i;
                if (solution[i] == n_i_plus_1) pos_i_plus_1 = i;
                if (solution[i] == n_j) pos_j = i;
                if (solution[i] == n_j_plus_1) pos_j_plus_1 = i;
            }
            
            // Re-evaluate intra moves involving the 4 affected nodes
            for (int other_pos = 0; other_pos < solution_size; ++other_pos) {
                reevaluate_and_update_intra(problem_instance, solution, pos_i, other_pos, LM);
                reevaluate_and_update_intra(problem_instance, solution, pos_i_plus_1, other_pos, LM);
                reevaluate_and_update_intra(problem_instance, solution, pos_j, other_pos, LM);
                reevaluate_and_update_intra(problem_instance, solution, pos_j_plus_1, other_pos, LM);
            }
            
            // Re-evaluate inter moves for the 4 affected nodes
            for (size_t out_idx = 0; out_idx < not_in_solution.size(); ++out_idx) {
                reevaluate_and_update_inter(problem_instance, solution, not_in_solution, pos_i, not_in_solution[out_idx], LM);
                reevaluate_and_update_inter(problem_instance, solution, not_in_solution, pos_i_plus_1, not_in_solution[out_idx], LM);
                reevaluate_and_update_inter(problem_instance, solution, not_in_solution, pos_j, not_in_solution[out_idx], LM);
                reevaluate_and_update_inter(problem_instance, solution, not_in_solution, pos_j_plus_1, not_in_solution[out_idx], LM);
            }
            
        } else {
            // Apply inter move
            int sol_pos = -1;
            for (int i = 0; i < solution_size; ++i) {
                if (solution[i] == best_move.node_in_solution) {
                    sol_pos = i;
                    break;
                }
            }
            
            int out_pos = -1;
            for(size_t i=0; i < not_in_solution.size(); ++i) {
                if (not_in_solution[i] == best_move.node_outside) {
                    out_pos = i;
                    break;
                }
            }
            
            if (sol_pos == -1 || out_pos == -1) continue;
            
            // Get neighbors BEFORE swap
            int pos_before = (sol_pos - 1 + solution_size) % solution_size;
            int pos_after = (sol_pos + 1) % solution_size;
            
            int node_that_left = solution[sol_pos];
            int node_that_entered = not_in_solution[out_pos];
            
            // Apply the exchange
            solution[sol_pos] = node_that_entered;
            not_in_solution[out_pos] = node_that_left;
            
            // Re-evaluate inter moves
            for (size_t out_idx = 0; out_idx < not_in_solution.size(); ++out_idx) {
                reevaluate_and_update_inter(problem_instance, solution, not_in_solution, sol_pos, not_in_solution[out_idx], LM);
                reevaluate_and_update_inter(problem_instance, solution, not_in_solution, pos_before, not_in_solution[out_idx], LM);
                reevaluate_and_update_inter(problem_instance, solution, not_in_solution, pos_after, not_in_solution[out_idx], LM);
            }
            
            for (int in_idx = 0; in_idx < solution_size; ++in_idx) {
                reevaluate_and_update_inter(problem_instance, solution, not_in_solution, in_idx, node_that_left, LM);
            }
            
            // Re-evaluate intra moves involving the 2 new edges
            for (int other_pos = 0; other_pos < solution_size; ++other_pos) {
                reevaluate_and_update_intra(problem_instance, solution, pos_before, other_pos, LM);
                reevaluate_and_update_intra(problem_instance, solution, sol_pos, other_pos, LM);
            }
        }
    }
    
    timer.end_stage();
    return solution;
}
