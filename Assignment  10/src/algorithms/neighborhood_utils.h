#ifndef NEIGHBORHOOD_UTILS_H
#define NEIGHBORHOOD_UTILS_H

#include <vector>
#include "../core/TSPProblem.h"

enum class NeighbourhoodType;

std::vector<int> get_inter_node_exchange(
    std::vector<int>& not_in_solution,
    std::vector<int>& solution_pos,
    int inter_iterator,
    int solution_size
);

std::vector<int> get_intra_edge_exchange(int i, int solution_size);

void apply_change(
    NeighbourhoodType intra_or_inter,
    std::vector<int>& solution,
    const std::vector<int>& change,
    std::vector<int>& not_in_solution
);

#endif // NEIGHBORHOOD_UTILS_H
