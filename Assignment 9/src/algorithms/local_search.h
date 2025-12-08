#ifndef LOCAL_SEARCH_H
#define LOCAL_SEARCH_H

#include "../core/TSPProblem.h"
#include "../core/point_data.h"
#include "../core/stagetimer.h"
#include <algorithm>
#include <vector>

/**
 * @brief Defines the type of local search algorithm to use.
 */
enum class SearchType {
    STEEPEST, ///< Explores the entire neighborhood and makes the best possible move.
    GREEDY    ///< Makes the first improving move it finds.
};

/**
 * @brief Defines the type of neighborhood being explored.
 */
enum class NeighbourhoodType {
    INTER, ///< Moves involving a node in the solution and a node outside of it.
    INTRA  ///< Moves involving only nodes already in the solution.
};

std::vector<int> local_search(TSPProblem &problem_instance,
                                     std::vector<int> starting_solution,
                                     SearchType T, StageTimer &timer);

#endif // LOCAL_SEARCH_H
