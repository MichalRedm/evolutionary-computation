#include "TSPProblem.h"

#include <vector>
#include <cmath>
#include "point_data.h"

// Start of anonymous namespace
// Functions inside here are local to this file only and are not exported.
namespace { 

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

}

TSPProblem::TSPProblem(const std::vector<PointData>& points) {
    this->points = points;
    this->distance_matrix = calculate_distance_matrix(points);
}

PointData TSPProblem::get_point(int id) const {
    return points[id];
}

int TSPProblem::get_distance(int id1, int id2) const {
    return distance_matrix[id1][id2];
}

int TSPProblem::get_num_points() const {
    return static_cast<int>(points.size());
}

const std::vector<PointData>& TSPProblem::get_points() const {
    return points;
}
