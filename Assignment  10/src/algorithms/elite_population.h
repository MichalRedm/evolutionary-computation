#ifndef ELITE_POPULATION_H
#define ELITE_POPULATION_H

#include <vector>
#include <utility>
#include <functional>
#include <random>
#include "../core/TSPProblem.h"

/**
 * @brief Manages a fixed-size population of elite solutions for the Traveling Salesperson Problem (TSP).
 * This class maintains a sorted vector of solutions (best to worst). It ensures that:
 * 1. The population never exceeds a maximum size.
 * 2. All solutions in the population have unique evaluation scores (within a tolerance).
 * 3. Only solutions better than the current worst solution are accepted (once the population is full).
 */
class ElitePopulation {
public:
    /**
     * @brief Structure representing a single candidate solution and its fitness.
     */
    struct SolutionWithEval {
        std::vector<int> solution; ///< The sequence of cities (path).
        double evaluation;         ///< The total cost/distance of the path (lower is better).

        /**
         * @brief operator< for sorting.
         * Defines strict weak ordering based on evaluation score. 
         * Used to keep the population sorted in ascending order (minimization).
         */
        bool operator<(const SolutionWithEval& other) const {
            return evaluation < other.evaluation;
        }
    };

    /**
     * @brief Constructs and initializes the population.
     * Fills the population using the provided generator function until the target size is reached.
     * Includes a safety mechanism to prevent infinite loops if the generator produces 
     * too many duplicate solutions.
     * @param target_size The maximum number of solutions to maintain.
     * @param solution_generator A function/lambda that returns a single valid std::vector<int> solution.
     * @param problem_instance Reference to the problem object used for evaluation.
     */
    ElitePopulation(int target_size, 
                    std::function<std::vector<int>()> solution_generator, 
                    const TSPProblem& problem_instance);

    /**
     * @brief Attempts to add a new solution to the population.
     * If the population is full, the new solution is only added if it is strictly better
     * than the worst solution currently in the population. The worst solution is then removed.
     * @param solution The TSP path vector to attempt to add.
     * @return true if the solution was added; false if it was rejected (duplicate or too poor).
     */
    bool try_add_solution(const std::vector<int>& solution);

    /**
     * @brief Selects two parents from the population for crossover.
     * Uses uniform random selection to pick two distinct indices.
     * @return A pair containing two parent solution vectors. Returns empty vectors if population is insufficient.
     */
    std::pair<std::vector<int>, std::vector<int>> get_parents();

    /**
     * @brief Retrieves the best solution found so far (the 0-th element).
     * @return A pair containing the best path vector and its evaluation score.
     */
    std::pair<std::vector<int>, double> get_best_solution();

    /**
     * @brief Returns the current number of solutions in the population.
     */
    size_t size() const;

private:
    std::vector<SolutionWithEval> population; ///< The sorted container of elite solutions.
    const TSPProblem& problem;                ///< Reference to the problem context.
    int max_population_size;                  ///< The fixed capacity of the population.
    std::mt19937 gen{std::random_device{}()}; ///< Mersenne Twister RNG.

    /**
     * @brief Internal helper to handle sorted insertion and uniqueness constraints.
     * Uses binary search (std::lower_bound) to find the insertion point in O(log N).
     * Checks neighbors for evaluation equality to enforce uniqueness.
     * Inserts in O(N) (due to vector shift) and maintains the fixed size.
     * @param solution The path vector.
     * @param eval The pre-calculated evaluation score.
     * @return true if added, false otherwise.
     */
    bool try_add_solution_internal(const std::vector<int>& solution, double eval);
};

#endif // ELITE_POPULATION_H
