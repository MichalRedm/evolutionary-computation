#include "large_neighborhood_search.h"
#include "local_search.h"
#include "destroy_operator.h"
#include "repair_operator.h"
#include "../core/evaluation.h"
#include <random>
#include <chrono>

std::vector<int> large_neighborhood_search(
    TSPProblem& problem_instance,
    std::vector<int> starting_solution,
    int iteration_limit,
    bool use_local_search
) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::vector<int> current_solution = starting_solution;

    // Removed applying LS to starting solution
    StageTimer dummy_timer;
    
    std::vector<int> best_solution = current_solution;
    double best_score = evaluate_solution(best_solution, problem_instance);
    double current_score = best_score;
    
    std::mt19937 rng(std::random_device{}());
    
    for (int i = 0; i < iteration_limit; i++) {
        
        // Destroy
        std::vector<int> partial_solution = destroy_solution(current_solution, problem_instance, rng);
        
        // Repair
        std::vector<int> repaired_solution = repair_solution(partial_solution, problem_instance);
        
        // Optional Local Search
        if (use_local_search) {
            repaired_solution = local_search(problem_instance, repaired_solution, SearchType::STEEPEST, dummy_timer);
        }
        
        double repaired_score = evaluate_solution(repaired_solution, problem_instance);
        
        // Acceptance criteria: Accept if better than current (Hill Climbing)
        if (repaired_score < current_score) {
            current_solution = repaired_solution;
            current_score = repaired_score;
            
            if (current_score < best_score) {
                best_solution = current_solution;
                best_score = current_score;
            }
        }
    }
    
    return best_solution;
}
