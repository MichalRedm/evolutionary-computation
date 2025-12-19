#include "inter_node_exchange.h"

#include <vector>
#include <math.h>
#include "../core/point_data.h"

double inter_node_exchange(
    TSPProblem& problem_instance,
    std::vector<int>& solution,
    int node_1_position,
    int node_2_id
) {

    double delta;
    double current_cost;
    double cost_after_exchange;

    const int solution_size = solution.size();

    int before_node_1 = solution[((node_1_position - 1) + solution_size) % solution_size];
    int after_node_1 = solution[((node_1_position + 1) ) % solution_size];

    int node_1 = solution[node_1_position];

    current_cost = problem_instance.get_distance(before_node_1, node_1) + 
                   problem_instance.get_distance(node_1, after_node_1) + 
                   problem_instance.get_point(node_1).cost;
    cost_after_exchange = problem_instance.get_distance(before_node_1, node_2_id) + 
                          problem_instance.get_distance(node_2_id, after_node_1) + 
                          problem_instance.get_point(node_2_id).cost;

    delta = cost_after_exchange - current_cost;
    return delta;
};