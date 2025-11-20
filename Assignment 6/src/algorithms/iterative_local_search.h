#ifndef ITERATIVE_LOCAL_SEARCH_H
#define ITERATIVE_LOCAL_SEARCH_H

#include <vector>
#include <algorithm>
#include <map>
#include "../core/point_data.h"
#include "../core/stagetimer.h"
#include "../core/TSPProblem.h"
#include "local_search.h"

std::map<std::string, std::vector<int>> iterative_local_search(
    std::vector<PointData> data,
    TSPProblem& problem_instance,
    SearchType T,
    double stopping_time,
    StageTimer& timer
);

#endif // ITERATIVE_LOCAL_SEARCH_H
