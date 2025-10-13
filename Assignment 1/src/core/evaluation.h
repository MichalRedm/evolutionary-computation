#ifndef EVALUATION_H
#define EVALUATION_H

#include <vector>
#include "point_data.h"

std::vector<std::vector<int>> calculate_distance_matrix(const std::vector<PointData>& data);
double evaluate_solution(const std::vector<int>& solution, const std::vector<PointData>& data, const std::vector<std::vector<int>>& distance_matrix);

#endif // EVALUATION_H
