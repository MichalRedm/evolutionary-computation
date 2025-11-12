#include "evaluation.h"

#include <cmath>
#include <numeric>
#include <algorithm>
#include "TSPProblem.h"

// Function to calculate the Euclidean distance between two points and round it to the nearest integer
int calculate_distance(const PointData& p1, const PointData& p2) {
    double dist = std::sqrt(std::pow(p1.x - p2.x, 2) + std::pow(p1.y - p2.y, 2));
    return static_cast<int>(std::round(dist));
}

// Function to pre-calculate the distance matrix for all pairs of points
std::vector<std::vector<int>> calculate_distance_matrix(const std::vector<PointData>& data) {
    size_t n = data.size();
    std::vector<std::vector<int>> matrix(n, std::vector<int>(n, 0));
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            int dist = calculate_distance(data[i], data[j]);
            matrix[i][j] = dist;
            matrix[j][i] = dist;
        }
    }
    return matrix;
}

// Function to evaluate a solution
double evaluate_solution(const std::vector<int>& solution, TSPProblem& problem_instance) {
    double total_cost = 0.0;
    double total_distance = 0.0;

    // Calculate total cost of selected nodes
    for (int node_id : solution) {
        total_cost += problem_instance.get_point(node_id).cost;
    }

    // Calculate total distance of the Hamiltonian cycle
    if (solution.size() > 1) {
        for (size_t i = 0; i < solution.size() - 1; ++i) {
            total_distance += problem_instance.get_distance(solution[i], solution[i + 1]);
        }
        // Add distance from the last node back to the first to close the cycle
        total_distance += problem_instance.get_distance(solution.back(), solution.front());
    }

    return total_cost + total_distance;
}
