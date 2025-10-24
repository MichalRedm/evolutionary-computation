#include "local_search.h"

#include <vector>
#include <algorithm>
#include <random>
#include "random_solution.h"
#include "nearest_neighbour_weighted_sum.h"
#include "../core/stagetimer.h"
#include "intra_node_exchange.h"
#include "inter_node_exchange.h"
#include <iostream>

// get mutation for inter change expressed as (node to be replaced pos, replacing node id, replacing node position)
std::vector<int> get_inter_node_exchange(
    std::vector<int>& not_in_solution,
    std::vector<int>& solution_pos,
    int inter_iterator,
    int solution_size
){
    int position_of_node_to_be_replaced = solution_pos[inter_iterator / solution_size];
    int position_of_replacing_node = inter_iterator % solution_size;
    int id_of_replacing_node = not_in_solution[position_of_replacing_node];

    std::vector<int> inter_change = {position_of_node_to_be_replaced, id_of_replacing_node, position_of_replacing_node};

    return inter_change;
}


// get mutation for intra expressed as (pos of switched node 1, pos of switched node 2)
std::vector<int> get_intra_node_exchange(const std::vector<int>& solution_pos, int i, int solution_size) {
    if (solution_size == 0) throw std::invalid_argument("Vector must not be empty");

    int totalPairs = solution_size * (solution_size + 1) / 2;
    if (i < 0 || i >= totalPairs)
        throw std::out_of_range("Index i is out of range");

    // Find row
    int row = 0;
    while (i >= solution_size - row) {
        i -= (solution_size - row);
        row++;
    }

    int col = row + i;  // i now represents the offset in that row

    return {solution_pos[row], solution_pos[col]};
}

// Apply given change to the solution and update the nodes that are not in solution
void apply_change(NeighbourhoodType intra_or_inter, std::vector<int>& solution, const std::vector<int>& change, std::vector<int>& not_in_solution){
    if (intra_or_inter == NeighbourhoodType::INTRA){
        // Switch the 2 nodes given in change
        int tmp = solution[change[0]];
        solution[change[0]] = solution[change[1]];
        solution[change[1]] = tmp;
    }
    else{
        // Switch the node in solution with the node that is not used according to the change
        // change[0] = position replaced node in solution, change[1] = id of replacing node, change[2] = position of replacing node in not_in_solution
        not_in_solution[change[2]] = solution[change[0]];
        solution[change[0]] = change[1];
    }
}

std::vector<int> local_search(
    const std::vector<PointData>& data,
    std::vector<std::vector<int>>& distance_matrix,
    SearchType T,
    IntraMoveType N,
    StartingSolutionType S,
    StageTimer& timer,
    int greedy_start_node_id = 0
) {
    std::vector<int> solution;

    timer.start_stage("initial_solution");
    if (S == StartingSolutionType::RANDOM) {
        solution = generate_random_solution(data);
    } else if (S == StartingSolutionType::GREEDY) {
        solution = nearest_neighbour_weighted_sum(data, distance_matrix, greedy_start_node_id);
    }
    timer.end_stage();

    timer.start_stage("local traversing");

    // TODO: Implement the local search logic based IntraMoveType N
    if (N == IntraMoveType::EDGES_EXCHANGE){
        timer.end_stage(); // Normally timer is supposed to be only at the end, but since we are returning earlier than we should. It is here
        return solution;
    }
    
    std::vector<int> not_in_solution = {};
    auto rng = std::default_random_engine {};
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



    for (int i = 0; i < int(data.size()); ++i){
        // If node i is not in the solution, it gets added to not_in_solution
        if(!(std::find(solution.begin(), solution.end(), i) != solution.end())){ 
            not_in_solution.push_back(i);
        }
    }

    // make an array of positions in solution (not ids) (useful for generating random mutations later)
    for (int i = 0; i < solution_size; ++i){
        solution_pos.push_back(i);
    }

    const int inter_limit = solution_size*not_in_solution.size();
    const int intra_limit = solution_size*(solution_size+1)/2;

    while (true) {
        std::shuffle(std::begin(solution_pos), std::end(solution_pos), rng);
        std::shuffle(std::begin(not_in_solution), std::end(not_in_solution), rng);
        inter_iterator = 0;
        intra_iterator = 0;
        best_delta = std::numeric_limits<double>::max();
        
        while(inter_iterator < inter_limit || intra_iterator < intra_limit){
            // decide which mutation we are doing

            // First check if either of them is over the limit (there are are no more possible neighbours left)
            if (inter_iterator >= inter_limit ){
                intra_or_inter = NeighbourhoodType::INTRA;
            }
            else if (intra_iterator >= intra_limit){
                intra_or_inter = NeighbourhoodType::INTER;
            }
            else{ // If both are below limit, we choose randomly
                int res = rand() % 2;
                if (res == 0 ){
                    intra_or_inter = NeighbourhoodType::INTRA;
                }
                else {
                    intra_or_inter = NeighbourhoodType::INTER;
                }
            }



            

            if (intra_or_inter == NeighbourhoodType::INTRA){
                //intra

                // Decide what change to test
                change = get_intra_node_exchange(solution_pos, intra_iterator, solution_size);

                //Get delta for this change
                delta = intra_node_exchange(distance_matrix, solution, change[0], change[1]);

                intra_iterator++;
            }
            else{
                //inter

                // Decide what change to test
                change = get_inter_node_exchange(not_in_solution, solution_pos, inter_iterator, solution_size);

                //Get delta for this change
                delta = inter_node_exchange(data, distance_matrix, solution, change[0], change[1]);

                inter_iterator++;
            }

            if (delta < best_delta){
                best_delta = delta;
                best_change = change;
                best_intra_or_inter = intra_or_inter;
            }

            if (T == SearchType::GREEDY && delta < 0){
                apply_change(intra_or_inter, solution, change, not_in_solution);
                break;
            }

        }

        if (best_delta >= 0){break;} // We have reached a local optimum

        if (T == SearchType::STEEPEST){
            apply_change(best_intra_or_inter, solution, best_change, not_in_solution);
        }

    }

    timer.end_stage();

    return solution;
}
