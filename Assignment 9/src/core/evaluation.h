#ifndef EVALUATION_H
#define EVALUATION_H

#include <vector>
#include "point_data.h"
#include "TSPProblem.h"

std::vector<std::vector<int>> calculate_distance_matrix(const std::vector<PointData>& data);
double evaluate_solution(const std::vector<int>& solution, TSPProblem& problem_instance);

#endif // EVALUATION_H
