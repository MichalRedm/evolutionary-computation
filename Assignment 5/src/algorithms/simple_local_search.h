#ifndef SIMPLE_LOCAL_SEARCH_H
#define SIMPLE_LOCAL_SEARCH_H

#include <vector>
#include <algorithm>
#include "../core/point_data.h"
#include "../core/stagetimer.h"
#include "../core/TSPProblem.h"
#include "local_search.h"

std::vector<int> simple_local_search(
    TSPProblem& problem_instance,
    std::vector<int> starting_solution,
    SearchType T,
    StageTimer& timer
);

#endif // SIMPLE_LOCAL_SEARCH_H
