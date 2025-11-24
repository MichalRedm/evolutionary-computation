#include "local_search.h"

#include <vector>
#include <algorithm>
#include <random>
#include <limits>    // For numeric_limits
#include "../core/stagetimer.h"
#include "inter_node_exchange.h"
#include "intra_edge_exchange.h"
#include "neighborhood_utils.h" // Include the new utility header

/**
 * @brief Performs a local search to improve an initial solution.
 *
 * This function implements both Steepest Descent and Greedy local search algorithms.
 * It explores two types of neighborhoods: inter-route (swapping a node in the solution
 * with one outside) and intra-route (swapping nodes or edges within the solution).
 *
 * @param problem_instance The TSPProblem instance containing points and distance matrix.
 * @param starting_solution The initial solution vector to be improved.
 * @param T The search type (STEEPEST or GREEDY).
 * @param timer A StageTimer object to record performance metrics.
 * @return The improved solution vector.
 */
std::vector<int> local_search(
    TSPProblem& problem_instance,
    std::vector<int> starting_solution,
    SearchType T,
    StageTimer& timer
) {
    std::vector<int> solution = starting_solution;

    timer.start_stage("local traversing");
    
    std::vector<int> not_in_solution = {};
    auto rng = std::default_random_engine {};
    std::random_device rd; // Use random_device for a non-deterministic seed
    rng.seed(rd());       // Seed the random number engine

    int inter_iterator;
    int intra_iterator;
    int solution_size = solution.size();
    std::vector<int> solution_pos = {};
    double delta;
    double best_delta;
    std::vector<int> change;
    std::vector<int> best_change;
    NeighbourhoodType intra_or_inter;
    NeighbourhoodType best_intra_or_inter;
    const double epsilon = 1e-9; // For floating point comparisons

    for (int i = 0; i < problem_instance.get_num_points(); ++i){
        // If node i is not in the solution, it gets added to not_in_solution
        if(!(std::find(solution.begin(), solution.end(), i) != solution.end())){ 
            not_in_solution.push_back(i);
        }
    }

    // Make an array of positions in solution (not ids) (useful for generating random mutations later)
    for (int i = 0; i < solution_size; ++i){
        solution_pos.push_back(i);
    }

    // Define neighborhood limits
    const int inter_limit = solution_size * not_in_solution.size();
    int intra_limit;
    if (solution_size < 2) {
        intra_limit = 0;
    } else {
        intra_limit = solution_size * (solution_size - 1) / 2;
    }

    while (true) {
        // Per assignment: "In greedy version the neighborhood should be browsed in random/randomized order."
        // We achieve this by shuffling the position/node lists and randomly picking
        // between inter/intra at each step.
        std::shuffle(std::begin(solution_pos), std::end(solution_pos), rng);
        std::shuffle(std::begin(not_in_solution), std::end(not_in_solution), rng);
        
        inter_iterator = 0;
        intra_iterator = 0;
        best_delta = std::numeric_limits<double>::max();
        best_change.clear();
        bool improving_move_found_greedy = false;
        
        while (inter_iterator < inter_limit || intra_iterator < intra_limit){
            // Decide which mutation we are doing

            bool can_do_intra = intra_iterator < intra_limit;
            bool can_do_inter = inter_iterator < inter_limit;

            // Randomly pick between move types if both are available
            // This satisfies the "browse moves of two kinds in a random order" requirement
            if (can_do_intra && can_do_inter) {
                // Use a simple random choice
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
                break; // Both neighborhoods exhausted
            }

            if (intra_or_inter == NeighbourhoodType::INTRA){
                // --- Intra-route move ---
                std::vector<int> indices = get_intra_edge_exchange(intra_iterator, solution_size);
                int pos1 = solution_pos[indices[0]];
                int pos2 = solution_pos[indices[1]];
                delta = intra_edge_exchange(problem_instance, solution, pos1, pos2);
                change = {pos1, pos2}; // Store actual positions for apply_change
                intra_iterator++;
            } else {
                // --- Inter-route move ---
                change = get_inter_node_exchange(not_in_solution, solution_pos, inter_iterator, solution_size);
                delta = inter_node_exchange(problem_instance, solution, change[0], change[1]);
                inter_iterator++;
            }

            // Update best move for Steepest search
            if (delta < best_delta){
                best_delta = delta;
                best_change = change;
                best_intra_or_inter = intra_or_inter;
            }

            // For Greedy search, apply the first improving move
            if (T == SearchType::GREEDY && delta < -epsilon){
                apply_change(intra_or_inter, solution, change, not_in_solution);
                improving_move_found_greedy = true;
                break; // Stop browsing neighborhood
            }
        } // End neighborhood browse loop

        // If Greedy search found a move, continue to the next iteration
        if (T == SearchType::GREEDY && improving_move_found_greedy) {
            continue;
        }

        // If no improving move was found (best_delta >= 0), we've reached a local optimum
        if (best_delta >= -epsilon){
            break; // Exit main while(true) loop
        } 

        // If Steepest search, apply the best move found
        if (T == SearchType::STEEPEST){
            apply_change(best_intra_or_inter, solution, best_change, not_in_solution);
        }

    } // End main while(true) loop

    timer.end_stage();

    return solution;
}
