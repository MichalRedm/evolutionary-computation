#include <vector>
#include <algorithm>
#include <random>
#include <stdexcept>
#include <limits>
#include <set>
#include <map>
#include <unordered_set>
#include <chrono>
#include "random_solution.h"
#include "../core/stagetimer.h"
#include "../core/evaluation.h"
#include "inter_node_exchange.h"
#include "intra_edge_exchange.h"
#include "neighborhood_utils.h"
#include "local_search.h"
#include "simple_local_search.h"

std::vector<int> getNotInSolution(int size, const std::vector<int>& solution) {
    // Put solution elements into a fast lookup set
    std::unordered_set<int> solSet(solution.begin(), solution.end());

    std::vector<int> not_in_solution;
    not_in_solution.reserve(size); // optional optimization

    for (int i = 0; i < size; ++i) {
        if (solSet.find(i) == solSet.end()) {
            not_in_solution.push_back(i);
        }
    }

    return not_in_solution;
}
/**
 * @brief Performs iterative local search
 * 
 * This method starts from a random solutions and finds the local optimum for this solution.
 * Then it perturbates the solution and uses local search again.
 * This cycle of local search and perturbation is performed until the function has been running longer than stopping_time.
 * 
 * @param data The data describind our TSPProblem instance.
 * @param problem_instance The TSPProblem instance containing points and distance matrix.
 * @param T The search type (STEEPEST or GREEDY).
 * @param stopping_time Time limit.
* @param timer A StageTimer object to record performance metrics.
 * @return The best found solution.
 */
std::map<std::string, std::vector<int>> iterative_local_search(
    std::vector<PointData> data,
    TSPProblem& problem_instance,
    SearchType T,
    double stopping_time,
    StageTimer& timer
){
    std::vector<int> best_solution;
    double best_score = std::numeric_limits<double>::max();

    StageTimer dummy_timer;
    int num_searches = 0;

    std::vector<int> solution = generate_random_solution(data);
    int solution_size = solution.size();
    timer.start_stage("iterative");

    

    // Perform local search as long as we don't exceed the time
    auto start = std::chrono::high_resolution_clock::now();
    while (true){
        num_searches++;

        // Reach a local optimum
        solution = simple_local_search(problem_instance, solution, T, dummy_timer);

        // Evaluate the solution
        double score = evaluate_solution(solution, problem_instance);
        if (best_score > score){
            best_score = score;
            best_solution = solution;
        }

        // Perturbate
        for (int i = 0; i < 15; ++i){
            // Choose a random move and apply it
            int randomNum = rand() % 100;

            if (randomNum < 40){
                // Intra edge
                int node1 = rand() % solution_size;
                int node2 = rand() % solution_size;

                apply_intra_edge_exchange(solution, node1, node2);
            }
            else if (randomNum < 80){
                // Inter node
                std::vector<int> not_in_solution = getNotInSolution(solution_size, solution);
                int node_in_solution_pos = rand() % solution_size;
                int node_not_in_solution_pos = rand() % not_in_solution.size();

                solution[node_in_solution_pos] = not_in_solution[node_not_in_solution_pos];

            }
            else{
                // Intra node
                int node1 = rand() % solution_size;
                int node2 = rand() % solution_size;

                int tmp = solution[node1];
                solution[node1] = solution[node2];
                solution[node2] = tmp;
            }
        }

        // Check if we exceeded the time limit
        auto now = std::chrono::high_resolution_clock::now();
        double elapsedMs = std::chrono::duration<double, std::milli>(now - start).count();

        if (elapsedMs > stopping_time) {
            break;
        }
    }

    timer.end_stage();

    return {{"solution", best_solution}, {"num_searches", {num_searches}}};

}