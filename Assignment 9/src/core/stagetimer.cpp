#include "stagetimer.h"
#include <iostream>

/**
 * @brief Constructs a new StageTimer.
 *
 * Initializes the timer state variables.
 */
StageTimer::StageTimer() : mode_(Mode::UNSET), is_running_(false) {}

/**
 * @brief Signals the start of a timed stage.
 *
 * Enforces the mutual exclusivity rule between staged and total-only modes.
 *
 * @param stage_name The name of the stage.
 * @throws std::logic_error if a stage is already running or if the naming
 * convention violates the established mode.
 */
void StageTimer::start_stage(const std::string& stage_name) {
    if (is_running_) {
        throw std::logic_error("Cannot start stage '" + stage_name + "'. Previous stage '" + current_stage_name_ + "' is still running. Call end_stage() first.");
    }

    bool name_used = !stage_name.empty();

    // 1. Establish Mode
    if (mode_ == Mode::UNSET) {
        mode_ = name_used ? Mode::STAGED : Mode::TOTAL_ONLY;
    }
    // 2. Enforce Mode Consistency
    else if (mode_ == Mode::STAGED && !name_used) {
        throw std::logic_error("Mode violation: StageTimer is in STAGED mode. start_stage() call requires a stage_name.");
    } else if (mode_ == Mode::TOTAL_ONLY && name_used) {
        throw std::logic_error("Mode violation: StageTimer is in TOTAL_ONLY mode. start_stage() call must not provide a stage_name.");
    }

    // 3. Set up measurement
    current_stage_name_ = name_used ? stage_name : "total";
    start_time_ = Clock::now();
    is_running_ = true;
}

/**
 * @brief Signals the end of the last started stage.
 *
 * Calculates the duration and accumulates it.
 *
 * @throws std::logic_error if no stage is currently running.
 */
void StageTimer::end_stage() {
    if (!is_running_) {
        throw std::logic_error("Cannot end stage. No stage has been started. Call start_stage() first.");
    }

    TimePoint end_time = Clock::now();
    
    // Calculate duration in milliseconds (double precision)
    DurationMs duration = end_time - start_time_;
    double duration_ms = duration.count();

    // Accumulate time and count
    total_stage_time_ms_[current_stage_name_] += duration_ms;
    measurement_counts_[current_stage_name_]++;

    // Reset state
    is_running_ = false;
    current_stage_name_.clear();
}

/**
 * @brief Calculates and returns the average runtimes for all measured stages.
 *
 * @return A map of stage names to their average runtimes in milliseconds.
 * @throws std::logic_error if a stage is currently running.
 */
std::map<std::string, double> StageTimer::get_avg_runtimes() const {
    if (is_running_) {
        throw std::logic_error("Cannot get average runtimes. Stage '" + current_stage_name_ + "' is still running. Call end_stage() first.");
    }

    std::map<std::string, double> avg_runtimes;
    double total_avg_ms = 0.0;
    
    // Check if any measurement was actually taken
    if (total_stage_time_ms_.empty()) {
        return avg_runtimes; // Return empty map if no measurements were taken
    }

    if (mode_ == Mode::TOTAL_ONLY) {
        // Only the key "total" exists in this mode
        const std::string total_key = "total";
        if (total_stage_time_ms_.count(total_key) && measurement_counts_.count(total_key)) {
            double total_time = total_stage_time_ms_.at(total_key);
            int count = measurement_counts_.at(total_key);
            avg_runtimes[total_key] = total_time / count;
        }
        return avg_runtimes;
    }
    
    // STAGED Mode
    for (const auto& pair : total_stage_time_ms_) {
        const std::string& name = pair.first;
        double total_time = pair.second;
        int count = measurement_counts_.at(name);

        double avg_ms = total_time / count;
        avg_runtimes[name] = avg_ms;
        total_avg_ms += avg_ms;
    }

    // Add the sum of all averaged stage runtimes as "total"
    if (mode_ == Mode::STAGED) {
        avg_runtimes["total"] = total_avg_ms;
    }

    return avg_runtimes;
}
