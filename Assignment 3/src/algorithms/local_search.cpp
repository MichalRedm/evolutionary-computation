#include "local_search.h"

#include <vector>
#include <random_solution.h>
#include <nearest_neighbour_weighted_sum.h>

std::vector<int> local_search(
    const std::vector<PointData>& data,
    std::vector<std::vector<int>>& distance_matrix,
    SearchType T,
    IntraMoveType N,
    StartingSolutionType S,
    int greedy_start_node_id = 0
) {
    std::vector<int> solution;

    if (S == StartingSolutionType::RANDOM) {
        solution = generate_random_solution(data);
    } else if (S == StartingSolutionType::GREEDY) {
        solution = nearest_neighbour_weighted_sum(data, distance_matrix, greedy_start_node_id);
    }

    // TODO: Implement the local search logic based on SearchType T and IntraMoveType N

    return solution;
}
