#pragma once

#include <chrono>
#include <string>
#include <unordered_map>
#include <map>
#include <stdexcept>

/**
 * @brief Utility class for measuring and accumulating runtimes of different stages
 * within an algorithm across multiple executions.
 *
 * This class supports two mutually exclusive modes:
 * 1. Staged mode: Measure multiple named stages within a single execution.
 * 2. Total-only mode: Measure only the runtime of the entire execution.
 */
class StageTimer {
public:
    /**
     * @brief Constructs a new StageTimer.
     */
    StageTimer();

    /**
     * @brief Signals the start of a timed stage.
     *
     * @param stage_name The name of the stage. If provided for the first call,
     * all subsequent calls must provide a name (Staged mode).
     * If not provided for the first call, no subsequent calls
     * must provide a name (Total-only mode).
     * @throws std::logic_error if a stage is already running or if the naming
     * convention violates the established mode.
     */
    void start_stage(const std::string& stage_name = "");

    /**
     * @brief Signals the end of the last started stage.
     *
     * The duration of the completed stage is accumulated for later averaging.
     * @throws std::logic_error if no stage is currently running.
     */
    void end_stage();

    /**
     * @brief Calculates and returns the average runtimes for all measured stages.
     *
     * The average is calculated based on accumulated total time and the number
     * of times each stage was measured.
     *
     * @return A map where keys are stage names and values are their average
     * runtimes in milliseconds.
     * - In Staged mode, a special key "total" is included, representing
     * the sum of all averaged stage runtimes.
     * - In Total-only mode, only the key "total" is returned.
     * @throws std::logic_error if a stage is currently running.
     */
    std::map<std::string, double> get_avg_runtimes() const;

private:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;
    using DurationMs = std::chrono::duration<double, std::milli>;

    /**
     * @brief Defines the measurement mode: UNSET, STAGED (named stages), or TOTAL_ONLY (unnamed).
     */
    enum class Mode { UNSET, STAGED, TOTAL_ONLY };

    Mode mode_;
    bool is_running_;
    std::string current_stage_name_;
    TimePoint start_time_;

    // Accumulation storage: total time (ms) and measurement count for each stage
    std::unordered_map<std::string, double> total_stage_time_ms_;
    std::unordered_map<std::string, int> measurement_counts_;
};
