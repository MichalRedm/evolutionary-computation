#ifndef LOCAL_SEARCH_H
#define LOCAL_SEARCH_H

#include <vector>
#include <algorithm>
#include "../core/point_data.h"
#include "../core/stagetimer.h"

/**
 * @brief Defines the method for generating the initial solution for the local search.
 */
enum class StartingSolutionType {
    RANDOM, ///< Starts from a randomly generated solution.
    GREEDY  ///< Starts from a solution built by a greedy construction heuristic.
};

/**
 * @brief Defines the type of neighborhood being explored.
 */
enum class NeighbourhoodType {
    INTER, ///< Moves involving a node in the solution and a node outside of it.
    INTRA  ///< Moves involving only nodes already in the solution.
};

std::vector<int> local_search(
    const std::vector<PointData>& data,
    std::vector<std::vector<int>>& distance_matrix,
    StartingSolutionType S,
    StageTimer& timer,
    int greedy_start_node_id
);

#endif // LOCAL_SEARCH_H