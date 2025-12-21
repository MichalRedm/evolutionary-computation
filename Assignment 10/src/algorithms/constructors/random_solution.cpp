#include "random_solution.h"
#include <vector>
#include <numeric>
#include <algorithm>
#include <random>
#include <cmath>

std::vector<int> generate_random_solution(const std::vector<PointData>& data) {
    int num_nodes = data.size();
    int num_to_select = static_cast<int>(std::ceil(num_nodes / 2.0));

    std::vector<int> node_indices(num_nodes);
    std::iota(node_indices.begin(), node_indices.end(), 0);

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(node_indices.begin(), node_indices.end(), g);

    std::vector<int> solution_path(node_indices.begin(), node_indices.begin() + num_to_select);

    return solution_path;
}
