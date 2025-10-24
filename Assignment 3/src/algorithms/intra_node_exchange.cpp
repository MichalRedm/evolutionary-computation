#include "intra_node_exchange.h"

#include <vector>
#include <math.h>

double intra_node_exchange(
    const std::vector<std::vector<int>>& distance_matrix,
    const std::vector<int>& solution,
    int node_1_position,
    int node_2_position
) {

    double delta;
    double current_cost;
    double cost_after_exchange;

    double difference_in_positions = abs(node_1_position - node_2_position);
    const int solution_size = solution.size();

    // Case when nodes are neighbours
    if ( difference_in_positions == 1 ){
        int earlier_node_position = std::min(node_1_position, node_2_position);
        int later_node_position = std::max(node_1_position, node_2_position);

        int before_earlier_node = solution[((earlier_node_position - 1) + solution_size) % solution_size];
        int after_later_node = solution[((later_node_position + 1) ) % solution_size];

        int earlier_node = solution[earlier_node_position];
        int later_node = solution[later_node_position];

        current_cost = distance_matrix[before_earlier_node][earlier_node] + distance_matrix[later_node][after_later_node];

        cost_after_exchange = distance_matrix[before_earlier_node][later_node] + distance_matrix[earlier_node][after_later_node];

        

    }
    // Case when nodes are neighbours at the end and beginning of solution vector
    else if(difference_in_positions == solution_size - 1){
        // Since in this case we know the ids of the nodes (first and last node),
        // we can skip the step of calculating the positions of nodes before and after them
        current_cost = distance_matrix[solution[0]][solution[1]] + distance_matrix[solution[solution_size - 2]][solution[solution_size - 1]];

        cost_after_exchange = distance_matrix[solution[solution_size - 1]][solution[1]] + distance_matrix[solution[solution_size - 2]][solution[0]];
    }
    // Case when nodes are not neighbours
    else{
        int before_node_1 = solution[((node_1_position - 1) + solution_size) % solution_size];
        int after_node_1 = solution[((node_1_position + 1) ) % solution_size];
        int before_node_2 = solution[((node_2_position - 1) + solution_size) % solution_size];
        int after_node_2 = solution[((node_2_position + 1) ) % solution_size];

        int node_1 = solution[node_1_position];
        int node_2 = solution[node_2_position];

        current_cost = distance_matrix[before_node_1][node_1]
                            + distance_matrix[node_1][after_node_1]
                            + distance_matrix[before_node_2][node_2]
                            + distance_matrix[node_2][after_node_2];

        cost_after_exchange = distance_matrix[before_node_1][node_2]
                                    + distance_matrix[node_2][after_node_1]
                                    + distance_matrix[before_node_2][node_1]
                                    + distance_matrix[node_1][after_node_2];  
    }

    delta = cost_after_exchange - current_cost;

    return delta;
}
