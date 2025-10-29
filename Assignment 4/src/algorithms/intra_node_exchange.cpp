#include "intra_node_exchange.h"

#include <vector>
#include <cmath>
#include <algorithm>

double intra_node_exchange(
    const std::vector<std::vector<int>>& distance_matrix,
    const std::vector<int>& solution,
    int node_1_position,
    int node_2_position
) {
    if (node_1_position == node_2_position) {
        return 0.0;
    }

    const int solution_size = solution.size();
    
    // Determine if the nodes are adjacent in the cycle
    bool is_adjacent = false;
    if (abs(node_1_position - node_2_position) == 1) {
        is_adjacent = true;
    }
    if ( (node_1_position == 0 && node_2_position == solution_size - 1) || 
         (node_2_position == 0 && node_1_position == solution_size - 1) ) {
        is_adjacent = true;
    }


    if (is_adjacent) {
        // For adjacent nodes X and Y in a cycle ... -> W -> X -> Y -> Z -> ...
        // swapping them results in ... -> W -> Y -> X -> Z -> ...
        // The change in cost is (dist(W,Y) + dist(X,Z)) - (dist(W,X) + dist(Y,Z))
        
        int pos_X = node_1_position;
        int pos_Y = node_2_position;

        // Ensure that Y is the node that follows X in the cycle
        if (pos_X != (pos_Y - 1 + solution_size) % solution_size) {
            std::swap(pos_X, pos_Y);
        }

        int pos_W = (pos_X - 1 + solution_size) % solution_size;
        int pos_Z = (pos_Y + 1) % solution_size;

        int id_W = solution[pos_W];
        int id_X = solution[pos_X];
        int id_Y = solution[pos_Y];
        int id_Z = solution[pos_Z];

        double current_cost = distance_matrix[id_W][id_X] + distance_matrix[id_Y][id_Z];
        double new_cost = distance_matrix[id_W][id_Y] + distance_matrix[id_X][id_Z];
        return new_cost - current_cost;

    } else { 
        // Case when nodes are not neighbours
        int before_node_1 = solution[((node_1_position - 1) + solution_size) % solution_size];
        int after_node_1 = solution[((node_1_position + 1) ) % solution_size];
        int before_node_2 = solution[((node_2_position - 1) + solution_size) % solution_size];
        int after_node_2 = solution[((node_2_position + 1) ) % solution_size];

        int node_1 = solution[node_1_position];
        int node_2 = solution[node_2_position];

        double current_cost = distance_matrix[before_node_1][node_1]
                            + distance_matrix[node_1][after_node_1]
                            + distance_matrix[before_node_2][node_2]
                            + distance_matrix[node_2][after_node_2];

        double new_cost = distance_matrix[before_node_1][node_2]
                                    + distance_matrix[node_2][after_node_1]
                                    + distance_matrix[before_node_2][node_1]
                                    + distance_matrix[node_1][after_node_2];
        return new_cost - current_cost;
    }
}
