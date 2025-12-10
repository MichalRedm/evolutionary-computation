#include "hybrid_evolutionary_algorithm.h"
#include <chrono>

#include "elite_population.h"
#include "random_solution.h"

// TODO: Implement the hybrid evolutionary algorithm
// You may need to modify the parameters of this function - remember to update the header file!
std::vector<int> hybrid_evolutionary_algorithm(const TSPProblem& problem, 
                                               const std::vector<int>& initial_solution, 
                                               int time_limit_ms, 
                                               bool use_ls,
                                               int population_size,
                                               int& iterations) {
    auto start_time = std::chrono::steady_clock::now();
    iterations = 0;

    // Maybe we need to improve those random solutions with local search?
    ElitePopulation population(population_size, [&]() {
        return generate_random_solution(problem.get_points());
    }, problem);

    // Run until time limit
    while (true) {
        iterations++;
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count() >= time_limit_ms) {
            break;
        }

        // TODO: Implement the main loop of the hybrid evolutionary algorithm
    }

    return population.get_best_solution().first;
}
