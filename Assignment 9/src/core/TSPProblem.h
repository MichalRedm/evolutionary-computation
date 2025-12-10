#ifndef TSPPROBLEM_H
#define TSPPROBLEM_H

#include <vector>
#include "point_data.h"

/**
 * @brief Represents the Traveling Salesperson Problem (TSP) data structure.
 * Stores the list of points and the pre-calculated distance matrix.
 */
class TSPProblem {
private:
    std::vector<PointData> points;
    std::vector<std::vector<int>> distance_matrix;

public:
    /**
     * @brief Constructor for the TSPProblem.
     * @param points A constant reference to the vector of points.
     */
    TSPProblem(const std::vector<PointData>& points);

    /**
     * @brief Retrieves a point by its ID (index).
     * @param id The index of the point.
     * @return The PointData object at the given index.
     */
    PointData get_point(int id) const;

    /**
     * @brief Retrieves the pre-calculated distance between two points.
     * @param id1 The index of the first point.
     * @param id2 The index of the second point.
     * @return The integer distance between the two points.
     */
    int get_distance(int id1, int id2) const;

    /**
     * @brief Retrieves the number of points in the problem.
     * @return The number of points.
     */
    int get_num_points() const;

    /**
     * @brief Retrieves the vector of all points.
     * @return A constant reference to the vector of points.
     */
    const std::vector<PointData>& get_points() const;
};

#endif // TSPPROBLEM_H
