#ifndef RANDOM_SOLUTION_H
#define RANDOM_SOLUTION_H

#include <vector>
#include "../core/point_data.h"

/**
 * @brief Generates a random solution for the modified Traveling Salesperson Problem.
 *
 * This function creates a random path that visits exactly half of the nodes given in the dataset.
 * It does this by shuffling the node indices and selecting the first half.
 *
 * @param data A vector of PointData objects, where each object represents a node.
 * @return A vector of integers representing the indices of the nodes in the generated random path.
 */
std::vector<int> generate_random_solution(const std::vector<PointData>& data);

#endif // RANDOM_SOLUTION_H
