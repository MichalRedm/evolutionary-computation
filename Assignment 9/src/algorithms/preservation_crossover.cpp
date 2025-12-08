#include "preservation_crossover.h"
#include "repair_operator.h"
#include <set>

std::vector<int> preservation_crossover(const std::vector<int>& parent1, const std::vector<int>& parent2, const TSPProblem& problem) {
    // 1. Identify nodes in parent 2
    std::set<int> p2_nodes(parent2.begin(), parent2.end());

    // 2. Filter parent1: keep only nodes present in parent2
    std::vector<int> partial_solution;
    for (int node : parent1) {
        if (p2_nodes.find(node) != p2_nodes.end()) {
            partial_solution.push_back(node);
        }
    }

    // 3. Repair the solution
    // We reuse the heuristic method from Assignment 7 (repair_operator)
    return repair_solution(partial_solution, problem);
}
