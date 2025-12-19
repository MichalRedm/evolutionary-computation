#include "elite_population.h"
#include "../core/evaluation.h"
#include <algorithm>
#include <cmath>
#include <iterator>

ElitePopulation::ElitePopulation(int target_size, 
                std::function<std::vector<int>()> solution_generator, 
                const TSPProblem& problem_instance)
    : problem(problem_instance), max_population_size(target_size) {
    
    int attempts = 0;
    // Safety guard: stop trying if we exceed 5x the target size in attempts
    // This prevents infinite loops if the solution space is small or the generator is poor.
    const int MAX_ATTEMPTS = target_size * 5; 

    while (static_cast<int>(population.size()) < max_population_size && attempts < MAX_ATTEMPTS) {
        std::vector<int> sol = solution_generator();
        double eval = evaluate_solution(sol, problem);
        
        // Reuse internal logic to ensure initial population is sorted and unique
        try_add_solution_internal(sol, eval);
        
        attempts++;
    }
}

bool ElitePopulation::try_add_solution(const std::vector<int>& solution) {
    double eval = evaluate_solution(solution, problem);
    return try_add_solution_internal(solution, eval);
}

std::pair<std::vector<int>, std::vector<int>> ElitePopulation::get_parents() {
    size_t N = population.size();
    
    if (N < 2) {
            if (N == 1) return {population[0].solution, population[0].solution};
            return {{}, {}};
    }

    // 1. Pick first parent index from [0, N-1]
    std::uniform_int_distribution<size_t> dis1(0, N - 1);
    size_t idx1 = dis1(gen);

    // 2. Pick a "raw" second index from [0, N-2] 
    // (representing the N-1 remaining choices)
    std::uniform_int_distribution<size_t> dis2(0, N - 2);
    size_t idx2 = dis2(gen);

    // 3. Shift idx2 to skip over idx1
    // This maps [0, N-2] to [0, N-1] \ {idx1}
    if (idx2 >= idx1) {
        idx2++;
    }

    return {population[idx1].solution, population[idx2].solution};
}

std::pair<std::vector<int>, std::vector<int>> ElitePopulation::get_parents_tournament() {
    size_t N = population.size();

    if (N < 2) {
        if (N == 1) return {population[0].solution, population[0].solution};
        return {{}, {}};
    }

    // Helper lambda: run a 2-way tournament and return the winner index
    auto tournament = [&]() -> size_t {
        // 1. Pick first competitor index from [0, N-1]
        std::uniform_int_distribution<size_t> dis1(0, N - 1);
        size_t idx1 = dis1(gen);

        // 2. Pick a "raw" second index from [0, N-2]
        // (representing the N-1 remaining choices)
        std::uniform_int_distribution<size_t> dis2(0, N - 2);
        size_t idx2 = dis2(gen);

        // 3. Shift idx2 to skip over idx1
        // This maps [0, N-2] to [0, N-1] \ {idx1}
        if (idx2 >= idx1) {
            idx2++;
        }

        // 4. Select the better individual based on evaluation
        // (assuming higher evaluation is better)
        if (population[idx1].evaluation >= population[idx2].evaluation) {
            return idx1;
        } else {
            return idx2;
        }
    };

    // Run two independent tournaments to select two parents
    size_t parent1 = tournament();
    size_t parent2 = tournament();

    return {population[parent1].solution, population[parent2].solution};
}


std::pair<std::vector<int>, double> ElitePopulation::get_best_solution() {
    if (population.empty()) return {{}, -1.0};
    return {population[0].solution, population[0].evaluation};
}

size_t ElitePopulation::size() const { return population.size(); }

bool ElitePopulation::try_add_solution_internal(const std::vector<int>& solution, double eval) {
    const double EPSILON = 1e-6; // Tolerance for floating point comparison

    // 1. Fast Fail: 
    // If population is full and new solution is worse than (or equal to) the worst current solution, reject.
    if (static_cast<int>(population.size()) >= max_population_size && eval >= population.back().evaluation - EPSILON) {
        return false;
    }

    SolutionWithEval new_entry = {solution, eval};

    // 2. Binary Search: Find the first element that has evaluation >= new_eval
    auto it = std::lower_bound(population.begin(), population.end(), new_entry);

    // 3. Uniqueness Check:
    // Check the element at the iterator (slightly worse or equal)
    if (it != population.end() && std::abs(it->evaluation - eval) < EPSILON) {
        return false; 
    }
    // Check the element before the iterator (slightly better)
    if (it != population.begin() && std::abs(std::prev(it)->evaluation - eval) < EPSILON) {
        return false;
    }

    // 4. Insert: 
    // Insert at the found position. This shifts elements to the right.
    population.insert(it, new_entry);

    // 5. Resize:
    // If we exceeded the max size, remove the last element (the worst one).
    if (static_cast<int>(population.size()) > max_population_size) {
        population.pop_back();
    }

    return true;
}
