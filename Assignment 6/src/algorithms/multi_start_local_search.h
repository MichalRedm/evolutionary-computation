#ifndef MULTI_START_LOCAL_SEACH_H
#define MULTI_START_LOCAL_SEACH_H

#include <vector>
#include <algorithm>
#include "../core/point_data.h"
#include "../core/stagetimer.h"
#include "../core/TSPProblem.h"
#include "local_search.h"

/**
 * @brief Performs a multiple start local search
 *
 * This functions runs local search started from a randomly generated solution
 * multiple times and returns the best found solution.
 *
 * @param data The data describind our TSPProblem instance
 * @param problem_instance The TSPProblem instance containing points and distance matrix.
 * @param T The search type (STEEPEST or GREEDY).
 * @param num_runs The amount of times we will run local search.
 * @param timer A StageTimer object to record performance metrics.
 * @return The best found solution.
 */
std::vector<int> multi_start_local_search(
    std::vector<PointData> data,
    TSPProblem& problem_instance,
    SearchType T,
    int num_runs,
    StageTimer& timer
);

#endif // MULTI_START_LOCAL_SEACH_H
