#ifndef ASSIGNMENT1_H
#define ASSIGNMENT1_H

#include <vector>

struct PointData {
    int id;
    int x;
    int y;
    int cost;
};

std::vector<int> generate_random_solution(const std::vector<PointData>& data);

#endif // ASSIGNMENT1_H
