#include "hybrid_evolutionary_algorithm.h"
#include <chrono>
#include <random>
#include <unordered_set>
#include <cstdlib>

#include "elite_population.h"
#include "random_solution.h"
#include "local_search.h"
#include "crossovers/recombination_operator.h"
#include "crossovers/preservation_crossover.h"
#include "intra_edge_exchange.h"
#include "../core/stagetimer.h"
#include "large_neighborhood_search.h"

// Helper function to get nodes not in solution
std::vector<int> getNotInSolution(int size, const std::vector<int>& solution) {
    std::unordered_set<int> solSet(solution.begin(), solution.end());
    std::vector<int> not_in_solution;
    not_in_solution.reserve(size);
    for (int i = 0; i < size; ++i) {
        if (solSet.find(i) == solSet.end()) {
            not_in_solution.push_back(i);
        }
    }
    return not_in_solution;
}

// Mutation operator: performs perturbations
void mutate_solution(std::vector<int>& solution, int total_nodes, int mutation_count = 10) {
    int solution_size = solution.size();
    
    // Safety check: ensure mutation count doesn't exceed a reasonable threshold relative to solution size
    // to prevent the mutation from completely randomizing the solution.
    if (mutation_count > solution_size / 2 && solution_size > 2) {
        mutation_count = solution_size / 2;
    }

    for (int i = 0; i < mutation_count; ++i) {
        int randomNum = rand() % 100;
        if (randomNum < 40) {
            // Intra edge exchange
            int node1 = rand() % solution_size;
            int node2 = rand() % solution_size;
            apply_intra_edge_exchange(solution, node1, node2);
        }
        else if (randomNum < 80) {
            // Inter node exchange
            std::vector<int> not_in_solution = getNotInSolution(total_nodes, solution);
            if (!not_in_solution.empty()) {
                int node_in_solution_pos = rand() % solution_size;
                int node_not_in_solution_pos = rand() % not_in_solution.size();
                solution[node_in_solution_pos] = not_in_solution[node_not_in_solution_pos];
            }
        }
        else {
            // Intra node exchange (swap two nodes in solution)
            int node1 = rand() % solution_size;
            int node2 = rand() % solution_size;
            int tmp = solution[node1];
            solution[node1] = solution[node2];
            solution[node2] = tmp;
        }
    }
}

std::vector<int> hybrid_evolutionary_algorithm(const TSPProblem& problem, 
                                               const std::vector<int>& initial_solution, 
                                               int time_limit_ms, 
                                               int population_size,
                                               int& iterations,
                                               double mutation_probability,
                                               double lns_probability,
                                               double tournament_selection_probability,
                                               const std::vector<std::pair<CrossoverFunc, double>>& crossovers,
                                               bool use_adaptive_crossover,
                                               double adaptive_learning_rate,
                                               double adaptive_min_weight,
                                               int mutation_strength,
                                               bool use_adaptive_mutation,
                                               int stagnation_step,
                                               int k_candidates,
                                               int max_stagnation_iterations) {
    auto start_time = std::chrono::steady_clock::now();
    iterations = 0;

    // Use default crossovers if list is empty
    std::vector<std::pair<CrossoverFunc, double>> active_crossovers = crossovers;
    if (active_crossovers.empty()) {
        active_crossovers.push_back({recombination_operator, 0.5});
        active_crossovers.push_back({preservation_crossover, 0.5});
    }

    // Initialize weights for crossover selection
    std::vector<double> weights;
    for (const auto& p : active_crossovers) {
        weights.push_back(p.second);
    }
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> chance_out_of_100(0, 99);

    int total_nodes = problem.get_num_points();

    // Create a lambda that generates random solutions with local search applied
    auto solution_generator = [&]() {
        std::vector<int> random_sol = generate_random_solution(problem.get_points());
        
        // Apply local search to initial random solutions
        StageTimer dummy_timer;
        std::vector<int> improved_sol = local_search(
            const_cast<TSPProblem&>(problem), 
            random_sol, 
            SearchType::GREEDY, 
            dummy_timer,
            k_candidates
        );
        
        return improved_sol;
    };

    // Initialize elite population with improved random solutions
    ElitePopulation population(population_size, solution_generator, problem);

    // Track iterations without improvement for termination
    // const int MAX_ITERATIONS_NO_IMPROVEMENT = 3000; // Removed, now using parameter
    
    // Define how often (in iterations) we increase mutation strength during stagnation
    // const int STAGNATION_MUTATION_STEP = 20;  // Now passed as parameter 

    int iterations_without_improvement = 0;
    double best_known_evaluation = population.get_best_solution().second;

    // Run until time limit or stagnation
    while (true) {
        iterations++;
        
        // Check time limit
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count() >= time_limit_ms) {
            break;
        }

        // Check stagnation
        if (max_stagnation_iterations != -1 && iterations_without_improvement >= max_stagnation_iterations) {
            break;
        }

        // --- Adaptive Mutation Logic ---
        int current_mutation_strength = mutation_strength;
        
        if (use_adaptive_mutation) {
            // Calculate dynamic mutation strength based on stagnation.
            // We start with the base 'mutation_strength' and add 1 extra move 
            // for every 'stagnation_step' iterations without improvement.
            current_mutation_strength = mutation_strength + (iterations_without_improvement / stagnation_step);
            
            // Optional: Ensure it doesn't grow indefinitely (handled partially inside mutate_solution via size check,
            // but good to cap here too to avoid huge loops).
            if (current_mutation_strength > total_nodes) {
                current_mutation_strength = total_nodes;
            }
        }
        // -------------------------------

        std::vector<int> offspring;
        int op_index = -1; // Track which crossover operator was used (-1 if LNS or none)

        // Check LNS probability
        if (chance_out_of_100(gen) < (1.0 - lns_probability) * 100) {
            
            // Rebuild distribution with current weights (if adaptive is on, weights change)
            std::discrete_distribution<> crossover_dist(weights.begin(), weights.end());

            // Select two parents uniformly from the population
            std::pair<std::vector<int>, std::vector<int>> parents = population.get_parents();
            std::vector<int> parent1 = parents.first;
            std::vector<int> parent2 = parents.second;
            if (chance_out_of_100(gen) < tournament_selection_probability * 100){
                // Select two parents using the tournament selection
                 parents = population.get_parents_tournament();
                 parent1 = parents.first;
                 parent2 = parents.second;
            }

            // If population is too small, break
            if (parent1.empty() || parent2.empty()) {
                break;
            }

            // Randomly choose recombination operator based on weights
            op_index = crossover_dist(gen);
            offspring = active_crossovers[op_index].first(parent1, parent2, problem);

            // Apply mutation based on probability
            if (chance_out_of_100(gen) < mutation_probability * 100) {
                // Pass the DETERMINED strength (dynamic or fixed)
                mutate_solution(offspring, total_nodes, current_mutation_strength);
            }

            // Randomly choose local search type
            // Right now it has 100% chance of steepest as greedy did not perform well (at least at our time limit)
            int randomNum = rand() % 100;
            SearchType search_type;

            if (randomNum < 100){
                search_type = SearchType::STEEPEST;
            }
            else {
                search_type = SearchType::GREEDY;
            }

            StageTimer dummy_timer;
            offspring = local_search(
                const_cast<TSPProblem&>(problem), 
                offspring, 
                search_type, 
                dummy_timer,
                k_candidates
            );
            
        }
        else {
            // Perform large neighborhood search
            std::pair<std::vector<int>, std::vector<int>> parents = population.get_parents();
            offspring = large_neighborhood_search(const_cast<TSPProblem&>(problem), parents.first, 2, true);
        }

        // Try to add offspring to elite population and capture success status
        bool added_to_population = population.try_add_solution(offspring);

        // Adaptive Probability Update Logic
        if (use_adaptive_crossover && op_index != -1) {
            // Current weight represents the probability of selection
            double current_prob = weights[op_index];

            if (added_to_population) {
                // Reward: Increase probability
                // Logic: "slightly stronger for less probable operators"
                // If prob is low (e.g., 0.1), boost factor is high (1.9).
                // If prob is high (e.g., 0.9), boost factor is low (1.1).
                double boost_factor = 1.0 + (1.0 - current_prob);
                weights[op_index] *= (1.0 + adaptive_learning_rate * boost_factor);
            } else {
                // Penalize: Decrease probability
                // Logic: "slightly stronger for more probable operators"
                // If prob is high (e.g., 0.9), penalty factor is high (1.9).
                // If prob is low (e.g., 0.1), penalty factor is low (1.1).
                double penalty_factor = 1.0 + current_prob;
                weights[op_index] *= (1.0 - adaptive_learning_rate * penalty_factor);
            }

            // Ensure we don't drop below minimum weight
            if (weights[op_index] < adaptive_min_weight) {
                weights[op_index] = adaptive_min_weight;
            }

            // Normalize weights to prevent unbounded growth/shrinkage
            double total_weight = 0.0;
            for (double w : weights) total_weight += w;
            
            if (total_weight > 0.0) {
                for (double& w : weights) w /= total_weight;
            }
        }

        // Check if we improved the best solution
        double current_best = population.get_best_solution().second;
        if (current_best < best_known_evaluation - 1e-9) {
            best_known_evaluation = current_best;
            iterations_without_improvement = 0; // This resets the mutation strength back to base

            // std::cout<<"iteration: "<<iterations<<" best_score: "<<current_best<<"\n";
        } else {
            iterations_without_improvement++;
        }
    }

    return population.get_best_solution().first;
}
