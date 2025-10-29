#include "intra_edge_exchange.h"
#include <vector>
#include <algorithm> // For std::swap

double intra_edge_exchange(
    const std::vector<std::vector<int>>& distance_matrix,
    const std::vector<int>& solution,
    int pos1,
    int pos2
) {
    const int solution_size = solution.size();
    if (solution_size < 3) return 0.0; // Not possible with < 3 nodes

    // Get the positions of the four nodes involved
    int pos1_plus_1 = (pos1 + 1) % solution_size;
    int pos2_plus_1 = (pos2 + 1) % solution_size;

    // Check for adjacent edges, which is an invalid 2-opt move.
    // (pos1, pos1+1) and (pos1+1, pos1+2) -> pos1_plus_1 == pos2
    // (pos2, pos2+1) and (pos2+1, pos2+2) -> pos2_plus_1 == pos1
    if (pos1 == pos2 || pos1_plus_1 == pos2 || pos2_plus_1 == pos1) {
        return 0.0; // No change, invalid move
    }

    // Get the IDs of the nodes
    int node_i = solution[pos1];
    int node_i_plus_1 = solution[pos1_plus_1];
    int node_j = solution[pos2];
    int node_j_plus_1 = solution[pos2_plus_1];

    // Cost of edges to remove
    double current_cost = distance_matrix[node_i][node_i_plus_1] + distance_matrix[node_j][node_j_plus_1];

    // Cost of edges to add (reversing segment from pos1+1 to pos2)
    // New edges are (i, j) and (i+1, j+1)
    double new_cost = distance_matrix[node_i][node_j] + distance_matrix[node_i_plus_1][node_j_plus_1];

    return new_cost - current_cost;
}

void apply_intra_edge_exchange(std::vector<int>& solution, int pos1, int pos2) {
    const int solution_size = solution.size();
    if (solution_size < 3) return;

    int start = (pos1 + 1) % solution_size;
    int end = pos2;

    if (start == end) return; // Nothing to reverse

    // Calculate the number of elements in the segment to be reversed
    int num_to_swap;
    if (start > end) {
        // Wrap-around case: e.g., size=10, start=9, end=2. Segment is [9, 0, 1, 2].
        num_to_swap = (solution_size - start) + (end + 1);
    } else {
        // Standard case: e.g., size=10, start=3, end=6. Segment is [3, 4, 5, 6].
        num_to_swap = (end - start + 1);
    }

    // Perform the reversal
    int i = start;
    int j = end;
    for (int k = 0; k < num_to_swap / 2; ++k) {
        std::swap(solution[i], solution[j]);
        i = (i + 1) % solution_size;
        j = (j - 1 + solution_size) % solution_size; // Move j backwards, wrapping around
    }
}
