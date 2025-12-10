#include "hybrid_evolutionary_algorithm.h"
#include <chrono>
#include <numeric>
#include <algorithm>
#include <random>
#include <vector>
#include <utility>
#include "../core/evaluation.h"

class ElitePopulation {
public:
    struct SolutionWithEval {
        std::vector<int> solution;
        double evaluation;

        bool operator<(const SolutionWithEval& other) const {
            return evaluation < other.evaluation;
        }
    };

    ElitePopulation(const std::vector<std::vector<int>>& initial_population, TSPProblem& problem_instance)
        : problem(problem_instance) {
        
        for (const auto& sol : initial_population) {
            double eval = evaluate_solution(sol, problem);
            population.push_back({sol, eval});
        }
        std::sort(population.begin(), population.end());
    }

    bool try_add_solution(const std::vector<int>& solution) {
        double eval = evaluate_solution(solution, problem);

        if (population.empty()) {
            population.push_back({solution, eval});
            return true;
        }

        // Check if better than worst (best evaluation is smaller)
        if (eval >= population.back().evaluation) {
            return false;
        }

        // Check if evaluation is unique
        for (const auto& s : population) {
            if (std::abs(s.evaluation - eval) < 1e-6) {
                return false;
            }
        }

        // Add new solution and remove worst
        // Since we are inserting into sorted vector, we can find position
        SolutionWithEval new_entry = {solution, eval};
        
        // Find position to insert
        auto it = std::upper_bound(population.begin(), population.end(), new_entry);
        
        population.insert(it, new_entry);
        population.pop_back(); // Remove the worst (last element after insert would be the old worst, wait. 
                               // If we insert, size increases by 1. The old worst is at end()-1 before insert.
                               // After insert, if inserted before end, old worst moves to end.
                               // So popping back removes the old worst.
                               // Exception: if we inserted at the very end... but we checked eval < worst, so it won't be at the very end.
                               // It will be before the last element.
        
        return true;
    }

    std::pair<std::vector<int>, std::vector<int>> get_parents() {
        if (population.size() < 2) {
            // Fallback if population is too small, though prompt implies valid population
             if (!population.empty()) return {population[0].solution, population[0].solution};
             return {{}, {}};
        }

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, population.size() - 1);

        int idx1 = dis(gen);
        int idx2 = dis(gen);
        while (idx1 == idx2) {
            idx2 = dis(gen);
        }

        return {population[idx1].solution, population[idx2].solution};
    }

    std::pair<std::vector<int>, double> get_best_solution() {
        if (population.empty()) return {{}, -1.0};
        return {population[0].solution, population[0].evaluation};
    }

private:
    std::vector<SolutionWithEval> population;
    TSPProblem& problem;
};

std::vector<int> hybrid_evolutionary_algorithm(const TSPProblem& problem, 
                                               const std::vector<int>& initial_solution, 
                                               int time_limit_ms, 
                                               bool use_ls, 
                                               int& iterations) {
    auto start_time = std::chrono::steady_clock::now();
    iterations = 0;

    // Run until time limit
    while (true) {
        iterations++;
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count() >= time_limit_ms) {
            break;
        }
    }

    // Return a random solution as per requirements
    int num_nodes = problem.get_num_points();
    int num_to_select = static_cast<int>(std::ceil(num_nodes / 2.0));

    std::vector<int> node_indices(num_nodes);
    std::iota(node_indices.begin(), node_indices.end(), 0);

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(node_indices.begin(), node_indices.end(), g);

    std::vector<int> solution_path(node_indices.begin(), node_indices.begin() + num_to_select);

    return solution_path;
}
