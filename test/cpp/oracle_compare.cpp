#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdint>

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <ruckig_c/ruckig.h>
#include <ruckig/ruckig.hpp>

namespace {

constexpr double kPositionTolerance = 1e-8;
constexpr double kVelocityTolerance = 1e-8;
constexpr double kAccelerationTolerance = 1e-10;
constexpr double kDurationTolerance = 1e-12;
constexpr double kExtremaTimeTolerance = 1e-7;
constexpr double kFirstTimeTolerance = 1e-4;

struct FirstTimeQuery {
    size_t dof {0};
    double position {0.0};
    double time_after {0.0};
};

struct CaseData {
    std::string name;
    size_t dofs {1};
    double delta_time {0.1};
    ruckig_control_interface_t control_interface {RUCKIG_CONTROL_POSITION};
    ruckig_synchronization_t synchronization {RUCKIG_SYNCHRONIZATION_TIME};
    ruckig_duration_discretization_t duration_discretization {RUCKIG_DURATION_CONTINUOUS};
    bool has_minimum_duration {false};
    double minimum_duration {0.0};
    std::vector<double> current_position;
    std::vector<double> current_velocity;
    std::vector<double> current_acceleration;
    std::vector<double> target_position;
    std::vector<double> target_velocity;
    std::vector<double> target_acceleration;
    std::vector<double> max_velocity;
    std::vector<double> max_acceleration;
    std::vector<double> max_jerk;
    std::vector<bool> enabled {};
    std::vector<double> min_velocity {};
    std::vector<double> min_acceleration {};
    std::vector<ruckig_control_interface_t> per_dof_control_interface {};
    std::vector<ruckig_synchronization_t> per_dof_synchronization {};
    std::vector<FirstTimeQuery> first_time_queries {};
    std::vector<double> extra_sample_times {};
    bool compare_first_time_queries {true};
    bool compare_update_loop {true};
    double first_time_tolerance {kFirstTimeTolerance};
};

int failures = 0;
std::string last_failure_message;

struct RandomGenerator {
    std::uint64_t state {1};

    explicit RandomGenerator(std::uint64_t seed): state(seed ? seed : 1) {}

    std::uint64_t next_u64() {
        state = state * 6364136223846793005ULL + 1442695040888963407ULL;
        return state;
    }

    double unit() {
        return static_cast<double>(next_u64() >> 11) * (1.0 / 9007199254740992.0);
    }

    double range(double min_value, double max_value) {
        return min_value + (max_value - min_value) * unit();
    }

    bool coin() {
        return (next_u64() & 1ULL) != 0ULL;
    }

    int pick(int max_exclusive) {
        return static_cast<int>(next_u64() % static_cast<std::uint64_t>(max_exclusive));
    }
};

void fail(const std::string& name, const std::string& message) {
    const std::string full_message = name + ": " + message;
    if (last_failure_message.empty()) {
        last_failure_message = full_message;
    }
    std::cerr << full_message << '\n';
    ++failures;
}

bool near(double lhs, double rhs, double tolerance) {
    const double scale = std::max({1.0, std::fabs(lhs), std::fabs(rhs)});
    return std::fabs(lhs - rhs) <= tolerance + 1.0e-13 * scale;
}

std::string values(double c_value, double oracle_value) {
    std::ostringstream stream;
    stream.precision(17);
    stream << " C=" << c_value << " oracle=" << oracle_value;
    return stream.str();
}

void copy_vector(double* dst, const std::vector<double>& src) {
    for (size_t i = 0; i < src.size(); ++i) {
        dst[i] = src[i];
    }
}

ruckig::Result to_oracle_result(ruckig_result_t result) {
    switch (result) {
        case RUCKIG_WORKING:
            return ruckig::Result::Working;
        case RUCKIG_FINISHED:
            return ruckig::Result::Finished;
        case RUCKIG_ERROR_INVALID_INPUT:
            return ruckig::Result::ErrorInvalidInput;
        case RUCKIG_ERROR_TRAJECTORY_DURATION:
            return ruckig::Result::ErrorTrajectoryDuration;
        case RUCKIG_ERROR_POSITIONAL_LIMITS:
            return ruckig::Result::ErrorPositionalLimits;
        case RUCKIG_ERROR_ZERO_LIMITS:
            return ruckig::Result::ErrorZeroLimits;
        case RUCKIG_ERROR_EXECUTION_TIME_CALCULATION:
            return ruckig::Result::ErrorExecutionTimeCalculation;
        case RUCKIG_ERROR_SYNCHRONIZATION_CALCULATION:
            return ruckig::Result::ErrorSynchronizationCalculation;
        default:
            return ruckig::Result::Error;
    }
}

void fill_oracle_input(const CaseData& test_case, ruckig::InputParameter<ruckig::DynamicDOFs>& input) {
    input.control_interface = test_case.control_interface == RUCKIG_CONTROL_VELOCITY
        ? ruckig::ControlInterface::Velocity
        : ruckig::ControlInterface::Position;

    switch (test_case.synchronization) {
        case RUCKIG_SYNCHRONIZATION_TIME:
            input.synchronization = ruckig::Synchronization::Time;
            break;
        case RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY:
            input.synchronization = ruckig::Synchronization::TimeIfNecessary;
            break;
        case RUCKIG_SYNCHRONIZATION_PHASE:
            input.synchronization = ruckig::Synchronization::Phase;
            break;
        case RUCKIG_SYNCHRONIZATION_NONE:
            input.synchronization = ruckig::Synchronization::None;
            break;
    }

    input.duration_discretization = test_case.duration_discretization == RUCKIG_DURATION_DISCRETE
        ? ruckig::DurationDiscretization::Discrete
        : ruckig::DurationDiscretization::Continuous;

    input.current_position = test_case.current_position;
    input.current_velocity = test_case.current_velocity;
    input.current_acceleration = test_case.current_acceleration;
    input.target_position = test_case.target_position;
    input.target_velocity = test_case.target_velocity;
    input.target_acceleration = test_case.target_acceleration;
    input.max_velocity = test_case.max_velocity;
    input.max_acceleration = test_case.max_acceleration;
    input.max_jerk = test_case.max_jerk;
    if (!test_case.enabled.empty()) {
        input.enabled = test_case.enabled;
    }
    if (!test_case.min_velocity.empty()) {
        input.min_velocity = test_case.min_velocity;
    }
    if (!test_case.min_acceleration.empty()) {
        input.min_acceleration = test_case.min_acceleration;
    }
    if (test_case.has_minimum_duration) {
        input.minimum_duration = test_case.minimum_duration;
    }
    if (!test_case.per_dof_control_interface.empty()) {
        std::vector<ruckig::ControlInterface> per_dof(test_case.per_dof_control_interface.size());
        for (size_t i = 0; i < per_dof.size(); ++i) {
            per_dof[i] = test_case.per_dof_control_interface[i] == RUCKIG_CONTROL_VELOCITY
                ? ruckig::ControlInterface::Velocity
                : ruckig::ControlInterface::Position;
        }
        input.per_dof_control_interface = per_dof;
    }
    if (!test_case.per_dof_synchronization.empty()) {
        std::vector<ruckig::Synchronization> per_dof(test_case.per_dof_synchronization.size());
        for (size_t i = 0; i < per_dof.size(); ++i) {
            switch (test_case.per_dof_synchronization[i]) {
                case RUCKIG_SYNCHRONIZATION_TIME:
                    per_dof[i] = ruckig::Synchronization::Time;
                    break;
                case RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY:
                    per_dof[i] = ruckig::Synchronization::TimeIfNecessary;
                    break;
                case RUCKIG_SYNCHRONIZATION_PHASE:
                    per_dof[i] = ruckig::Synchronization::Phase;
                    break;
                case RUCKIG_SYNCHRONIZATION_NONE:
                    per_dof[i] = ruckig::Synchronization::None;
                    break;
            }
        }
        input.per_dof_synchronization = per_dof;
    }
}

void fill_c_input(const CaseData& test_case, ruckig_input_t* input) {
    ruckig_input_set_control_interface(input, test_case.control_interface);
    ruckig_input_set_synchronization(input, test_case.synchronization);
    ruckig_input_set_duration_discretization(input, test_case.duration_discretization);
    if (test_case.has_minimum_duration) {
        ruckig_input_set_minimum_duration(input, test_case.minimum_duration);
    }
    if (!test_case.per_dof_control_interface.empty()) {
        ruckig_input_set_per_dof_control_interface(
            input,
            test_case.per_dof_control_interface.data(),
            test_case.per_dof_control_interface.size()
        );
    }
    if (!test_case.per_dof_synchronization.empty()) {
        ruckig_input_set_per_dof_synchronization(
            input,
            test_case.per_dof_synchronization.data(),
            test_case.per_dof_synchronization.size()
        );
    }

    copy_vector(ruckig_input_current_position_data(input), test_case.current_position);
    copy_vector(ruckig_input_current_velocity_data(input), test_case.current_velocity);
    copy_vector(ruckig_input_current_acceleration_data(input), test_case.current_acceleration);
    copy_vector(ruckig_input_target_position_data(input), test_case.target_position);
    copy_vector(ruckig_input_target_velocity_data(input), test_case.target_velocity);
    copy_vector(ruckig_input_target_acceleration_data(input), test_case.target_acceleration);
    copy_vector(ruckig_input_max_velocity_data(input), test_case.max_velocity);
    copy_vector(ruckig_input_max_acceleration_data(input), test_case.max_acceleration);
    copy_vector(ruckig_input_max_jerk_data(input), test_case.max_jerk);
    if (!test_case.enabled.empty()) {
        for (size_t i = 0; i < test_case.enabled.size(); ++i) {
            ruckig_input_set_dof_enabled(input, i, test_case.enabled[i]);
        }
    }
    if (!test_case.min_velocity.empty()) {
        ruckig_input_set_min_velocity(input, test_case.min_velocity.data(), test_case.min_velocity.size());
    }
    if (!test_case.min_acceleration.empty()) {
        ruckig_input_set_min_acceleration(input, test_case.min_acceleration.data(), test_case.min_acceleration.size());
    }
}

void compare_samples(
    const CaseData& test_case,
    const ruckig::Trajectory<ruckig::DynamicDOFs>& oracle_trajectory,
    const ruckig_trajectory_t* c_trajectory,
    double duration
) {
    std::vector<double> sample_times {0.0, duration / 2.0, duration};
    if (duration > 2e-9) {
        sample_times.push_back(1e-9);
        sample_times.push_back(duration - 1e-9);
    }
    for (double sample_time: test_case.extra_sample_times) {
        if (sample_time >= 0.0 && sample_time <= duration) {
            sample_times.push_back(sample_time);
        }
    }

    for (double sample_time: sample_times) {
        std::vector<double> oracle_position(test_case.dofs);
        std::vector<double> oracle_velocity(test_case.dofs);
        std::vector<double> oracle_acceleration(test_case.dofs);
        std::vector<double> c_position(test_case.dofs);
        std::vector<double> c_velocity(test_case.dofs);
        std::vector<double> c_acceleration(test_case.dofs);
        std::vector<double> c_jerk(test_case.dofs);

        oracle_trajectory.at_time(sample_time, oracle_position, oracle_velocity, oracle_acceleration);
        const auto c_result = ruckig_trajectory_at_time(
            c_trajectory,
            sample_time,
            c_position.data(),
            c_velocity.data(),
            c_acceleration.data(),
            c_jerk.data(),
            nullptr
        );
        if (c_result != RUCKIG_WORKING) {
            fail(test_case.name, "C trajectory_at_time returned error at t=" + std::to_string(sample_time));
            continue;
        }

        for (size_t dof = 0; dof < test_case.dofs; ++dof) {
            if (!near(c_position[dof], oracle_position[dof], kPositionTolerance)) {
                fail(test_case.name, "position mismatch at t=" + std::to_string(sample_time) + " dof=" + std::to_string(dof) + values(c_position[dof], oracle_position[dof]));
            }
            if (!near(c_velocity[dof], oracle_velocity[dof], kVelocityTolerance)) {
                fail(test_case.name, "velocity mismatch at t=" + std::to_string(sample_time) + " dof=" + std::to_string(dof) + values(c_velocity[dof], oracle_velocity[dof]));
            }
            if (!near(c_acceleration[dof], oracle_acceleration[dof], kAccelerationTolerance)) {
                fail(test_case.name, "acceleration mismatch at t=" + std::to_string(sample_time) + " dof=" + std::to_string(dof) + values(c_acceleration[dof], oracle_acceleration[dof]));
            }
        }
    }
}

void print_oracle_profile_details(
    const CaseData& test_case,
    const ruckig::Trajectory<ruckig::DynamicDOFs>& oracle_trajectory
) {
    const auto profiles = oracle_trajectory.get_profiles();
    if (!profiles.empty()) {
        for (size_t dof = 0; dof < profiles[0].size(); ++dof) {
            const auto& p = profiles[0][dof];
            std::cerr << test_case.name << ": oracle profile dof=" << dof << ' ' << p.to_string() << '\n';
            std::cerr << "t:";
            for (double value: p.t) {
                std::cerr << ' ' << value;
            }
            std::cerr << "\nj:";
            for (double value: p.j) {
                std::cerr << ' ' << value;
            }
            std::cerr << "\na:";
            for (double value: p.a) {
                std::cerr << ' ' << value;
            }
            std::cerr << "\nv:";
            for (double value: p.v) {
                std::cerr << ' ' << value;
            }
            std::cerr << "\np:";
            for (double value: p.p) {
                std::cerr << ' ' << value;
            }
            std::cerr << '\n';
        }
    }
}

void compare_trajectory_queries(
    const CaseData& test_case,
    const ruckig::Trajectory<ruckig::DynamicDOFs>& oracle_trajectory,
    const ruckig_trajectory_t* c_trajectory,
    double duration,
    bool compare_first_time_queries
) {
    std::vector<ruckig::Bound> oracle_extrema(test_case.dofs);
    std::vector<ruckig_position_extrema_t> c_extrema(test_case.dofs);
    oracle_trajectory.get_position_extrema(oracle_extrema);
    const auto c_extrema_result = ruckig_trajectory_get_position_extrema(c_trajectory, c_extrema.data(), c_extrema.size());
    if (c_extrema_result != RUCKIG_WORKING) {
        fail(test_case.name, "C position extrema accessor returned error");
    } else {
        for (size_t dof = 0; dof < test_case.dofs; ++dof) {
            if (!near(c_extrema[dof].min_position, oracle_extrema[dof].min, kPositionTolerance)) {
                fail(test_case.name, "position extrema min mismatch dof=" + std::to_string(dof) + values(c_extrema[dof].min_position, oracle_extrema[dof].min));
            }
            if (!near(c_extrema[dof].max_position, oracle_extrema[dof].max, kPositionTolerance)) {
                fail(test_case.name, "position extrema max mismatch dof=" + std::to_string(dof) + values(c_extrema[dof].max_position, oracle_extrema[dof].max));
            }
            if (!near(c_extrema[dof].time_min, oracle_extrema[dof].t_min, kExtremaTimeTolerance)) {
                fail(test_case.name, "position extrema t_min mismatch dof=" + std::to_string(dof) + values(c_extrema[dof].time_min, oracle_extrema[dof].t_min));
            }
            if (!near(c_extrema[dof].time_max, oracle_extrema[dof].t_max, kExtremaTimeTolerance)) {
                fail(test_case.name, "position extrema t_max mismatch dof=" + std::to_string(dof) + values(c_extrema[dof].time_max, oracle_extrema[dof].t_max));
            }
        }
    }

    if (!compare_first_time_queries) {
        return;
    }

    auto compare_first_time_query = [&](size_t dof, double query_position, double time_after) {
        const auto oracle_time = oracle_trajectory.get_first_time_at_position(dof, query_position, time_after);
        double c_time = 0.0;
        bool c_found = false;
        const auto c_time_result = ruckig_trajectory_get_first_time_at_position(c_trajectory, dof, query_position, time_after, &c_time, &c_found);
        if (c_time_result != RUCKIG_WORKING) {
            fail(test_case.name, "C first-time-at-position returned error dof=" + std::to_string(dof));
            return;
        }
        if (c_found != oracle_time.has_value()) {
            bool equivalent_boundary = false;
            if (oracle_time.has_value()) {
                std::vector<double> c_position(test_case.dofs);
                const auto c_sample_result = ruckig_trajectory_at_time(
                    c_trajectory,
                    *oracle_time,
                    c_position.data(),
                    nullptr,
                    nullptr,
                    nullptr,
                    nullptr
                );
                equivalent_boundary = c_sample_result == RUCKIG_WORKING
                    && near(c_position[dof], query_position, kPositionTolerance);
            } else if (c_found) {
                std::vector<double> oracle_position_at_c_time(test_case.dofs);
                std::vector<double> oracle_velocity_at_c_time(test_case.dofs);
                std::vector<double> oracle_acceleration_at_c_time(test_case.dofs);
                oracle_trajectory.at_time(c_time, oracle_position_at_c_time, oracle_velocity_at_c_time, oracle_acceleration_at_c_time);
                equivalent_boundary = near(oracle_position_at_c_time[dof], query_position, kPositionTolerance);
            }
            if (equivalent_boundary) {
                return;
            }
            std::ostringstream message;
            message.precision(17);
            message << "first-time-at-position found mismatch dof=" << dof
                << " C=" << static_cast<int>(c_found)
                << " oracle=" << static_cast<int>(oracle_time.has_value())
                << " query=" << query_position
                << " time_after=" << time_after;
            fail(test_case.name, message.str());
            return;
        }
        if (c_found && !near(c_time, *oracle_time, test_case.first_time_tolerance)) {
            fail(test_case.name, "first-time-at-position mismatch dof=" + std::to_string(dof) + values(c_time, *oracle_time));
        }
    };

    for (size_t dof = 0; dof < test_case.dofs; ++dof) {
        std::vector<double> oracle_position(test_case.dofs);
        std::vector<double> oracle_velocity(test_case.dofs);
        std::vector<double> oracle_acceleration(test_case.dofs);
        std::vector<double> query_positions;
        oracle_trajectory.at_time(0.0, oracle_position, oracle_velocity, oracle_acceleration);
        query_positions.push_back(oracle_position[dof]);
        oracle_trajectory.at_time(duration / 2.0, oracle_position, oracle_velocity, oracle_acceleration);
        query_positions.push_back(oracle_position[dof]);
        oracle_trajectory.at_time(duration, oracle_position, oracle_velocity, oracle_acceleration);
        query_positions.push_back(oracle_position[dof]);

        for (double query_position: query_positions) {
            compare_first_time_query(dof, query_position, 0.0);
        }
    }

    for (const FirstTimeQuery& query: test_case.first_time_queries) {
        compare_first_time_query(query.dof, query.position, query.time_after);
    }
}

void compare_update_loop(const CaseData& test_case) {
    ruckig::Ruckig<ruckig::DynamicDOFs> oracle_otg(test_case.dofs, test_case.delta_time);
    ruckig::InputParameter<ruckig::DynamicDOFs> oracle_input(test_case.dofs);
    ruckig::OutputParameter<ruckig::DynamicDOFs> oracle_output(test_case.dofs);
    fill_oracle_input(test_case, oracle_input);

    ruckig_t* c_otg = nullptr;
    ruckig_input_t* c_input = nullptr;
    ruckig_output_t* c_output = nullptr;
    if (ruckig_create(&c_otg, test_case.dofs, test_case.delta_time) != RUCKIG_WORKING
        || ruckig_input_create(&c_input, test_case.dofs) != RUCKIG_WORKING
        || ruckig_output_create(&c_output, test_case.dofs) != RUCKIG_WORKING) {
        fail(test_case.name, "failed to create C update handles");
        ruckig_output_destroy(c_output);
        ruckig_input_destroy(c_input);
        ruckig_destroy(c_otg);
        return;
    }
    fill_c_input(test_case, c_input);

    for (size_t step = 0; step < 1000; ++step) {
        const auto oracle_result = oracle_otg.update(oracle_input, oracle_output);
        const auto c_result = ruckig_update(c_otg, c_input, c_output);
        if (to_oracle_result(c_result) != oracle_result) {
            fail(test_case.name, "update result mismatch at step=" + std::to_string(step));
            break;
        }
        if (!near(ruckig_output_get_time(c_output), oracle_output.time, kDurationTolerance)) {
            fail(test_case.name, "update time mismatch at step=" + std::to_string(step) + values(ruckig_output_get_time(c_output), oracle_output.time));
            break;
        }
        if (ruckig_output_get_new_section(c_output) != oracle_output.new_section) {
            fail(test_case.name, "update section mismatch at step=" + std::to_string(step));
            break;
        }
        if (ruckig_output_did_section_change(c_output) != oracle_output.did_section_change) {
            fail(test_case.name, "update section-change mismatch at step=" + std::to_string(step));
            break;
        }
        if (ruckig_output_new_calculation(c_output) != oracle_output.new_calculation) {
            fail(test_case.name, "update new-calculation mismatch at step=" + std::to_string(step));
            break;
        }
        for (size_t dof = 0; dof < test_case.dofs; ++dof) {
            if (!near(ruckig_output_new_position_data(c_output)[dof], oracle_output.new_position[dof], kPositionTolerance)) {
                fail(test_case.name, "update position mismatch at step=" + std::to_string(step) + " dof=" + std::to_string(dof) + values(ruckig_output_new_position_data(c_output)[dof], oracle_output.new_position[dof]));
                break;
            }
            if (!near(ruckig_output_new_velocity_data(c_output)[dof], oracle_output.new_velocity[dof], kVelocityTolerance)) {
                fail(test_case.name, "update velocity mismatch at step=" + std::to_string(step) + " dof=" + std::to_string(dof) + values(ruckig_output_new_velocity_data(c_output)[dof], oracle_output.new_velocity[dof]));
                break;
            }
            if (!near(ruckig_output_new_acceleration_data(c_output)[dof], oracle_output.new_acceleration[dof], kAccelerationTolerance)) {
                fail(test_case.name, "update acceleration mismatch at step=" + std::to_string(step) + " dof=" + std::to_string(dof) + values(ruckig_output_new_acceleration_data(c_output)[dof], oracle_output.new_acceleration[dof]));
                break;
            }
        }
        if (failures != 0 || oracle_result == ruckig::Result::Finished || c_result == RUCKIG_FINISHED) {
            break;
        }
        oracle_output.pass_to_input(oracle_input);
        ruckig_output_pass_to_input(c_output, c_input);
    }

    ruckig_output_destroy(c_output);
    ruckig_input_destroy(c_input);
    ruckig_destroy(c_otg);
}

void run_case(const CaseData& test_case, bool compare_first_time_queries = true) {
    ruckig::Ruckig<ruckig::DynamicDOFs> oracle_otg(test_case.dofs, test_case.delta_time);
    ruckig::InputParameter<ruckig::DynamicDOFs> oracle_input(test_case.dofs);
    ruckig::Trajectory<ruckig::DynamicDOFs> oracle_trajectory(test_case.dofs);
    fill_oracle_input(test_case, oracle_input);

    ruckig_t* c_otg = nullptr;
    ruckig_input_t* c_input = nullptr;
    ruckig_trajectory_t* c_trajectory = nullptr;
    if (ruckig_create(&c_otg, test_case.dofs, test_case.delta_time) != RUCKIG_WORKING
        || ruckig_input_create(&c_input, test_case.dofs) != RUCKIG_WORKING
        || ruckig_trajectory_create(&c_trajectory, test_case.dofs) != RUCKIG_WORKING) {
        fail(test_case.name, "failed to create C handles");
        ruckig_trajectory_destroy(c_trajectory);
        ruckig_input_destroy(c_input);
        ruckig_destroy(c_otg);
        return;
    }
    fill_c_input(test_case, c_input);

    const auto oracle_result = oracle_otg.calculate(oracle_input, oracle_trajectory);
    const auto c_result = ruckig_calculate(c_otg, c_input, c_trajectory);
    if (to_oracle_result(c_result) != oracle_result) {
        fail(test_case.name, "result mismatch: C=" + std::to_string(static_cast<int>(c_result))
            + " oracle=" + std::to_string(static_cast<int>(oracle_result)));
        if (oracle_result == ruckig::Result::Working) {
            print_oracle_profile_details(test_case, oracle_trajectory);
        }
    }

    if (oracle_result == ruckig::Result::Working && c_result == RUCKIG_WORKING) {
        const int failures_before_samples = failures;
        const double oracle_duration = oracle_trajectory.get_duration();
        const double c_duration = ruckig_trajectory_get_duration(c_trajectory);
        if (!near(c_duration, oracle_duration, kDurationTolerance)) {
            fail(test_case.name, "duration mismatch: C=" + std::to_string(c_duration) + " oracle=" + std::to_string(oracle_duration));
        }
        {
            const auto oracle_independent_min_durations = oracle_trajectory.get_independent_min_durations();
            std::vector<double> c_independent_min_durations(test_case.dofs);
            const auto c_independent_result = ruckig_trajectory_get_independent_min_durations(
                c_trajectory,
                c_independent_min_durations.data(),
                c_independent_min_durations.size()
            );
            if (c_independent_result != RUCKIG_WORKING) {
                fail(test_case.name, "C independent min duration accessor returned error");
            } else {
                for (size_t dof = 0; dof < test_case.dofs; ++dof) {
                    if (!near(c_independent_min_durations[dof], oracle_independent_min_durations[dof], kDurationTolerance)) {
                        fail(test_case.name, "independent min duration mismatch dof=" + std::to_string(dof) + values(c_independent_min_durations[dof], oracle_independent_min_durations[dof]));
                    }
                }
            }
        }
        compare_samples(test_case, oracle_trajectory, c_trajectory, oracle_duration);
        compare_trajectory_queries(test_case, oracle_trajectory, c_trajectory, oracle_duration, compare_first_time_queries);
        if (failures == failures_before_samples && test_case.compare_update_loop) {
            compare_update_loop(test_case);
        }
        if (failures != failures_before_samples) {
            print_oracle_profile_details(test_case, oracle_trajectory);
        }
    }

    ruckig_trajectory_destroy(c_trajectory);
    ruckig_input_destroy(c_input);
    ruckig_destroy(c_otg);
}

struct WaypointSectionOracleCase {
    std::string name;
    size_t dofs {1};
    double delta_time {0.01};
    std::vector<double> current_position;
    std::vector<double> current_velocity;
    std::vector<double> current_acceleration;
    std::vector<double> target_position;
    std::vector<double> target_velocity;
    std::vector<double> target_acceleration;
    std::vector<double> max_velocity;
    std::vector<double> max_acceleration;
    std::vector<double> max_jerk;
    std::vector<double> intermediate_positions;
    std::vector<double> per_section_max_velocity {};
    std::vector<double> per_section_min_velocity {};
    std::vector<double> per_section_max_acceleration {};
    std::vector<double> per_section_min_acceleration {};
    std::vector<double> per_section_max_jerk {};
    std::vector<double> per_section_minimum_duration {};
};

void fill_waypoint_c_input(const WaypointSectionOracleCase& test_case, ruckig_input_t* input) {
    copy_vector(ruckig_input_current_position_data(input), test_case.current_position);
    copy_vector(ruckig_input_current_velocity_data(input), test_case.current_velocity);
    copy_vector(ruckig_input_current_acceleration_data(input), test_case.current_acceleration);
    copy_vector(ruckig_input_target_position_data(input), test_case.target_position);
    copy_vector(ruckig_input_target_velocity_data(input), test_case.target_velocity);
    copy_vector(ruckig_input_target_acceleration_data(input), test_case.target_acceleration);
    copy_vector(ruckig_input_max_velocity_data(input), test_case.max_velocity);
    copy_vector(ruckig_input_max_acceleration_data(input), test_case.max_acceleration);
    copy_vector(ruckig_input_max_jerk_data(input), test_case.max_jerk);

    const size_t waypoint_count = test_case.intermediate_positions.size() / test_case.dofs;
    ruckig_input_set_intermediate_positions(input, test_case.intermediate_positions.data(), waypoint_count, test_case.dofs);
    if (!test_case.per_section_max_velocity.empty()) {
        ruckig_input_set_per_section_max_velocity(input, test_case.per_section_max_velocity.data(), waypoint_count + 1, test_case.dofs);
    }
    if (!test_case.per_section_min_velocity.empty()) {
        ruckig_input_set_per_section_min_velocity(input, test_case.per_section_min_velocity.data(), waypoint_count + 1, test_case.dofs);
    }
    if (!test_case.per_section_max_acceleration.empty()) {
        ruckig_input_set_per_section_max_acceleration(input, test_case.per_section_max_acceleration.data(), waypoint_count + 1, test_case.dofs);
    }
    if (!test_case.per_section_min_acceleration.empty()) {
        ruckig_input_set_per_section_min_acceleration(input, test_case.per_section_min_acceleration.data(), waypoint_count + 1, test_case.dofs);
    }
    if (!test_case.per_section_max_jerk.empty()) {
        ruckig_input_set_per_section_max_jerk(input, test_case.per_section_max_jerk.data(), waypoint_count + 1, test_case.dofs);
    }
    if (!test_case.per_section_minimum_duration.empty()) {
        ruckig_input_set_per_section_minimum_duration(input, test_case.per_section_minimum_duration.data(), waypoint_count + 1);
    }
}

double section_value_or_global(
    const std::vector<double>& per_section_values,
    const std::vector<double>& global_values,
    size_t section,
    size_t dof,
    size_t dofs
) {
    if (!per_section_values.empty()) {
        return per_section_values[section * dofs + dof];
    }
    return global_values[dof];
}

void compare_waypoint_section_to_oracle(
    const WaypointSectionOracleCase& test_case,
    const ruckig_trajectory_t* c_trajectory,
    const std::vector<double>& cumulative_times,
    size_t section,
    const std::vector<double>& start_position,
    const std::vector<double>& start_velocity,
    const std::vector<double>& start_acceleration,
    const std::vector<double>& end_position,
    const std::vector<double>& end_velocity,
    const std::vector<double>& end_acceleration
) {
    const double offset = section == 0 ? 0.0 : cumulative_times[section - 1];
    const double section_duration = cumulative_times[section] - offset;
    ruckig::Ruckig<ruckig::DynamicDOFs> oracle_otg(test_case.dofs, test_case.delta_time);
    ruckig::InputParameter<ruckig::DynamicDOFs> oracle_input(test_case.dofs);
    ruckig::Trajectory<ruckig::DynamicDOFs> oracle_trajectory(test_case.dofs);

    oracle_input.current_position = start_position;
    oracle_input.current_velocity = start_velocity;
    oracle_input.current_acceleration = start_acceleration;
    oracle_input.target_position = end_position;
    oracle_input.target_velocity = end_velocity;
    oracle_input.target_acceleration = end_acceleration;
    oracle_input.max_velocity.assign(test_case.dofs, 0.0);
    oracle_input.min_velocity = std::vector<double>(test_case.dofs, 0.0);
    oracle_input.max_acceleration.assign(test_case.dofs, 0.0);
    oracle_input.min_acceleration = std::vector<double>(test_case.dofs, 0.0);
    oracle_input.max_jerk.assign(test_case.dofs, 0.0);
    for (size_t dof = 0; dof < test_case.dofs; ++dof) {
        oracle_input.max_velocity[dof] = section_value_or_global(test_case.per_section_max_velocity, test_case.max_velocity, section, dof, test_case.dofs);
        oracle_input.min_velocity.value()[dof] = test_case.per_section_min_velocity.empty()
            ? -oracle_input.max_velocity[dof]
            : test_case.per_section_min_velocity[section * test_case.dofs + dof];
        oracle_input.max_acceleration[dof] = section_value_or_global(test_case.per_section_max_acceleration, test_case.max_acceleration, section, dof, test_case.dofs);
        oracle_input.min_acceleration.value()[dof] = test_case.per_section_min_acceleration.empty()
            ? -oracle_input.max_acceleration[dof]
            : test_case.per_section_min_acceleration[section * test_case.dofs + dof];
        oracle_input.max_jerk[dof] = section_value_or_global(test_case.per_section_max_jerk, test_case.max_jerk, section, dof, test_case.dofs);
    }
    if (!test_case.per_section_minimum_duration.empty()) {
        oracle_input.minimum_duration = test_case.per_section_minimum_duration[section];
    }

    const auto oracle_result = oracle_otg.calculate(oracle_input, oracle_trajectory);
    if (oracle_result != ruckig::Result::Working) {
        fail(test_case.name, "section oracle solve failed section=" + std::to_string(section));
        return;
    }
    if (!near(section_duration, oracle_trajectory.get_duration(), kDurationTolerance)) {
        fail(test_case.name, "section duration mismatch section=" + std::to_string(section) + values(section_duration, oracle_trajectory.get_duration()));
    }

    const std::vector<double> sample_times {0.0, section_duration / 2.0, section_duration};
    for (double local_time: sample_times) {
        std::vector<double> oracle_position(test_case.dofs);
        std::vector<double> oracle_velocity(test_case.dofs);
        std::vector<double> oracle_acceleration(test_case.dofs);
        std::vector<double> c_position(test_case.dofs);
        std::vector<double> c_velocity(test_case.dofs);
        std::vector<double> c_acceleration(test_case.dofs);
        oracle_trajectory.at_time(local_time, oracle_position, oracle_velocity, oracle_acceleration);
        const auto c_result = ruckig_trajectory_at_time(
            c_trajectory,
            offset + local_time,
            c_position.data(),
            c_velocity.data(),
            c_acceleration.data(),
            nullptr,
            nullptr
        );
        if (c_result != RUCKIG_WORKING) {
            fail(test_case.name, "section C trajectory sample failed section=" + std::to_string(section));
            continue;
        }
        for (size_t dof = 0; dof < test_case.dofs; ++dof) {
            if (!near(c_position[dof], oracle_position[dof], kPositionTolerance)) {
                fail(test_case.name, "section position mismatch section=" + std::to_string(section) + " dof=" + std::to_string(dof) + values(c_position[dof], oracle_position[dof]));
            }
            if (!near(c_velocity[dof], oracle_velocity[dof], kVelocityTolerance)) {
                fail(test_case.name, "section velocity mismatch section=" + std::to_string(section) + " dof=" + std::to_string(dof) + values(c_velocity[dof], oracle_velocity[dof]));
            }
            if (!near(c_acceleration[dof], oracle_acceleration[dof], kAccelerationTolerance)) {
                fail(test_case.name, "section acceleration mismatch section=" + std::to_string(section) + " dof=" + std::to_string(dof) + values(c_acceleration[dof], oracle_acceleration[dof]));
            }
        }
    }
}

void run_waypoint_section_oracle_case(const WaypointSectionOracleCase& test_case) {
    const size_t waypoint_count = test_case.intermediate_positions.size() / test_case.dofs;
    const size_t section_count = waypoint_count + 1;
    ruckig_t* c_otg = nullptr;
    ruckig_input_t* c_input = nullptr;
    ruckig_trajectory_t* c_trajectory = nullptr;
    if (ruckig_create_with_waypoints(&c_otg, test_case.dofs, test_case.delta_time, waypoint_count) != RUCKIG_WORKING
        || ruckig_input_create_with_waypoints(&c_input, test_case.dofs, waypoint_count) != RUCKIG_WORKING
        || ruckig_trajectory_create_with_waypoints(&c_trajectory, test_case.dofs, waypoint_count) != RUCKIG_WORKING) {
        fail(test_case.name, "failed to create waypoint section oracle C handles");
        ruckig_trajectory_destroy(c_trajectory);
        ruckig_input_destroy(c_input);
        ruckig_destroy(c_otg);
        return;
    }

    fill_waypoint_c_input(test_case, c_input);
    const auto c_result = ruckig_calculate(c_otg, c_input, c_trajectory);
    if (c_result != RUCKIG_WORKING) {
        fail(test_case.name, "waypoint calculate failed before section oracle comparison");
        ruckig_trajectory_destroy(c_trajectory);
        ruckig_input_destroy(c_input);
        ruckig_destroy(c_otg);
        return;
    }

    std::vector<double> intermediate_times(waypoint_count);
    if (waypoint_count > 0) {
        const auto durations_result = ruckig_trajectory_get_intermediate_durations(c_trajectory, intermediate_times.data(), intermediate_times.size());
        if (durations_result != RUCKIG_WORKING) {
            fail(test_case.name, "failed to get intermediate durations");
        }
    }
    std::vector<double> cumulative_times(section_count);
    for (size_t i = 0; i < waypoint_count; ++i) {
        cumulative_times[i] = intermediate_times[i];
    }
    cumulative_times[section_count - 1] = ruckig_trajectory_get_duration(c_trajectory);

    std::vector<std::vector<double>> positions(section_count + 1, std::vector<double>(test_case.dofs));
    std::vector<std::vector<double>> velocities(section_count + 1, std::vector<double>(test_case.dofs));
    std::vector<std::vector<double>> accelerations(section_count + 1, std::vector<double>(test_case.dofs));
    for (size_t boundary = 0; boundary < section_count + 1; ++boundary) {
        const double time = boundary == 0 ? 0.0 : cumulative_times[boundary - 1];
        const auto sample_result = ruckig_trajectory_at_time(
            c_trajectory,
            time,
            positions[boundary].data(),
            velocities[boundary].data(),
            accelerations[boundary].data(),
            nullptr,
            nullptr
        );
        if (sample_result != RUCKIG_WORKING) {
            fail(test_case.name, "failed to sample waypoint section boundary=" + std::to_string(boundary));
        }
    }

    for (size_t section = 0; section < section_count; ++section) {
        compare_waypoint_section_to_oracle(
            test_case,
            c_trajectory,
            cumulative_times,
            section,
            positions[section],
            velocities[section],
            accelerations[section],
            positions[section + 1],
            velocities[section + 1],
            accelerations[section + 1]
        );
    }

    ruckig_trajectory_destroy(c_trajectory);
    ruckig_input_destroy(c_input);
    ruckig_destroy(c_otg);
}

void run_waypoint_section_oracle_cases() {
    const int failures_before = failures;
    std::vector<WaypointSectionOracleCase> cases;
    cases.push_back(WaypointSectionOracleCase{
        "waypoint-section-oracle-1d-per-section-min-duration",
        1,
        0.01,
        {0.0},
        {0.0},
        {0.0},
        {2.0},
        {0.0},
        {0.0},
        {1.5},
        {2.0},
        {4.0},
        {1.0},
        {1.2, 1.0},
        {-1.2, -1.0},
        {2.0, 1.8},
        {-2.0, -1.8},
        {4.0, 4.0},
        {0.5, 0.25}
    });
    cases.push_back(WaypointSectionOracleCase{
        "waypoint-section-oracle-2d-two-waypoints",
        2,
        0.02,
        {0.0, 0.0},
        {0.0, 0.0},
        {0.0, 0.0},
        {1.5, -0.8},
        {0.0, 0.0},
        {0.0, 0.0},
        {1.2, 1.1},
        {2.0, 1.8},
        {4.0, 3.5},
        {0.5, -0.2, 1.0, -0.5}
    });
    cases.push_back(WaypointSectionOracleCase{
        "waypoint-section-oracle-3d-per-section-limits",
        3,
        0.01,
        {0.0, 0.0, 0.0},
        {0.05, -0.02, 0.03},
        {0.0, 0.0, 0.0},
        {1.2, -0.7, 0.9},
        {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0},
        {1.5, 1.4, 1.3},
        {2.2, 2.0, 1.8},
        {4.5, 4.0, 3.8},
        {0.4, -0.2, 0.25, 0.85, -0.45, 0.60},
        {
            1.2, 1.1, 1.0,
            1.4, 1.3, 1.2,
            1.5, 1.4, 1.3
        },
        {
            -1.2, -1.1, -1.0,
            -1.4, -1.3, -1.2,
            -1.5, -1.4, -1.3
        },
        {
            2.0, 1.8, 1.7,
            2.1, 1.9, 1.8,
            2.2, 2.0, 1.9
        },
        {
            -2.0, -1.8, -1.7,
            -2.1, -1.9, -1.8,
            -2.2, -2.0, -1.9
        },
        {
            4.0, 3.8, 3.6,
            4.2, 4.0, 3.8,
            4.5, 4.2, 4.0
        },
        {0.30, 0.40, 0.35}
    });
    cases.push_back(WaypointSectionOracleCase{
        "waypoint-section-oracle-4d-three-waypoints",
        4,
        0.02,
        {0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0},
        {1.6, -0.8, 1.0, -0.6},
        {0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0},
        {1.4, 1.3, 1.2, 1.1},
        {2.0, 1.9, 1.8, 1.7},
        {4.0, 3.8, 3.6, 3.4},
        {
            0.35, -0.15, 0.20, -0.10,
            0.80, -0.35, 0.55, -0.30,
            1.20, -0.60, 0.80, -0.45
        }
    });

    for (const auto& test_case: cases) {
        run_waypoint_section_oracle_case(test_case);
    }
    if (failures == failures_before) {
        std::cout << "Waypoint section oracle comparisons passed: " << cases.size() << '\n';
    }
}

void print_case_repro(const CaseData& test_case, std::uint64_t seed, size_t sample_index, const char* kind) {
    std::cerr.precision(17);
    std::cerr << "repro kind=" << kind
        << " seed=" << seed
        << " sample=" << sample_index
        << " name=" << test_case.name
        << " dofs=" << test_case.dofs
        << " dt=" << test_case.delta_time
        << " control=" << static_cast<int>(test_case.control_interface)
        << " sync=" << static_cast<int>(test_case.synchronization)
        << " discrete=" << static_cast<int>(test_case.duration_discretization)
        << '\n';
    auto print_control_vector = [](const char* label, const std::vector<ruckig_control_interface_t>& values) {
        if (values.empty()) {
            return;
        }
        std::cerr << label << ':';
        for (auto value: values) {
            std::cerr << ' ' << static_cast<int>(value);
        }
        std::cerr << '\n';
    };
    auto print_sync_vector = [](const char* label, const std::vector<ruckig_synchronization_t>& values) {
        if (values.empty()) {
            return;
        }
        std::cerr << label << ':';
        for (auto value: values) {
            std::cerr << ' ' << static_cast<int>(value);
        }
        std::cerr << '\n';
    };
    auto print_bool_vector = [](const char* label, const std::vector<bool>& values) {
        if (values.empty()) {
            return;
        }
        std::cerr << label << ':';
        for (bool value: values) {
            std::cerr << ' ' << (value ? 1 : 0);
        }
        std::cerr << '\n';
    };
    auto print_vector = [](const char* label, const std::vector<double>& values) {
        std::cerr << label << ':';
        for (double value: values) {
            std::cerr << ' ' << value;
        }
        std::cerr << '\n';
    };
    print_vector("current_position", test_case.current_position);
    print_vector("current_velocity", test_case.current_velocity);
    print_vector("current_acceleration", test_case.current_acceleration);
    print_vector("target_position", test_case.target_position);
    print_vector("target_velocity", test_case.target_velocity);
    print_vector("target_acceleration", test_case.target_acceleration);
    print_vector("max_velocity", test_case.max_velocity);
    print_vector("max_acceleration", test_case.max_acceleration);
    print_vector("max_jerk", test_case.max_jerk);
    print_bool_vector("enabled", test_case.enabled);
    print_control_vector("per_dof_control_interface", test_case.per_dof_control_interface);
    print_sync_vector("per_dof_synchronization", test_case.per_dof_synchronization);
}

const char* control_initializer(ruckig_control_interface_t value) {
    return value == RUCKIG_CONTROL_VELOCITY ? "RUCKIG_CONTROL_VELOCITY" : "RUCKIG_CONTROL_POSITION";
}

const char* synchronization_initializer(ruckig_synchronization_t value) {
    switch (value) {
        case RUCKIG_SYNCHRONIZATION_TIME:
            return "RUCKIG_SYNCHRONIZATION_TIME";
        case RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY:
            return "RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY";
        case RUCKIG_SYNCHRONIZATION_PHASE:
            return "RUCKIG_SYNCHRONIZATION_PHASE";
        case RUCKIG_SYNCHRONIZATION_NONE:
            return "RUCKIG_SYNCHRONIZATION_NONE";
    }
    return "RUCKIG_SYNCHRONIZATION_TIME";
}

const char* discretization_initializer(ruckig_duration_discretization_t value) {
    return value == RUCKIG_DURATION_DISCRETE ? "RUCKIG_DURATION_DISCRETE" : "RUCKIG_DURATION_CONTINUOUS";
}

void print_double_initializer(std::ostream& stream, double value) {
    if (std::isinf(value)) {
        stream << (value < 0.0 ? "-inf" : "inf");
        return;
    }
    if (std::isnan(value)) {
        stream << "std::numeric_limits<double>::quiet_NaN()";
        return;
    }
    stream.precision(17);
    stream << value;
}

void print_double_vector_initializer(const std::vector<double>& values) {
    std::cout << "{";
    for (size_t i = 0; i < values.size(); ++i) {
        if (i != 0) {
            std::cout << ", ";
        }
        print_double_initializer(std::cout, values[i]);
    }
    std::cout << "}";
}

void print_bool_vector_initializer(const std::vector<bool>& values) {
    std::cout << "{";
    for (size_t i = 0; i < values.size(); ++i) {
        if (i != 0) {
            std::cout << ", ";
        }
        std::cout << (values[i] ? "true" : "false");
    }
    std::cout << "}";
}

void print_control_vector_initializer(const std::vector<ruckig_control_interface_t>& values) {
    std::cout << "{";
    for (size_t i = 0; i < values.size(); ++i) {
        if (i != 0) {
            std::cout << ", ";
        }
        std::cout << control_initializer(values[i]);
    }
    std::cout << "}";
}

void print_sync_vector_initializer(const std::vector<ruckig_synchronization_t>& values) {
    std::cout << "{";
    for (size_t i = 0; i < values.size(); ++i) {
        if (i != 0) {
            std::cout << ", ";
        }
        std::cout << synchronization_initializer(values[i]);
    }
    std::cout << "}";
}

void print_first_time_vector_initializer(const std::vector<FirstTimeQuery>& values) {
    std::cout << "{";
    for (size_t i = 0; i < values.size(); ++i) {
        if (i != 0) {
            std::cout << ", ";
        }
        std::cout << "{";
        std::cout << values[i].dof << ", ";
        print_double_initializer(std::cout, values[i].position);
        std::cout << ", ";
        print_double_initializer(std::cout, values[i].time_after);
        std::cout << "}";
    }
    std::cout << "}";
}

void print_case_fixture_initializer(const CaseData& test_case, std::uint64_t seed, size_t sample_index, const char* kind) {
    std::cout << "oracle random replay fixture kind=" << kind << " seed=" << seed << " sample=" << sample_index << '\n';
    std::cout << "cases.push_back(CaseData{\n";
    std::cout << "    \"" << test_case.name << "\",\n";
    std::cout << "    " << test_case.dofs << ",\n";
    std::cout << "    ";
    print_double_initializer(std::cout, test_case.delta_time);
    std::cout << ",\n";
    std::cout << "    " << control_initializer(test_case.control_interface) << ",\n";
    std::cout << "    " << synchronization_initializer(test_case.synchronization) << ",\n";
    std::cout << "    " << discretization_initializer(test_case.duration_discretization) << ",\n";
    std::cout << "    " << (test_case.has_minimum_duration ? "true" : "false") << ",\n";
    std::cout << "    ";
    print_double_initializer(std::cout, test_case.minimum_duration);
    std::cout << ",\n";
    const auto print_field = [](const char* indent, const auto& printer) {
        std::cout << indent;
        printer();
        std::cout << ",\n";
    };
    print_field("    ", [&]() { print_double_vector_initializer(test_case.current_position); });
    print_field("    ", [&]() { print_double_vector_initializer(test_case.current_velocity); });
    print_field("    ", [&]() { print_double_vector_initializer(test_case.current_acceleration); });
    print_field("    ", [&]() { print_double_vector_initializer(test_case.target_position); });
    print_field("    ", [&]() { print_double_vector_initializer(test_case.target_velocity); });
    print_field("    ", [&]() { print_double_vector_initializer(test_case.target_acceleration); });
    print_field("    ", [&]() { print_double_vector_initializer(test_case.max_velocity); });
    print_field("    ", [&]() { print_double_vector_initializer(test_case.max_acceleration); });
    print_field("    ", [&]() { print_double_vector_initializer(test_case.max_jerk); });
    print_field("    ", [&]() { print_bool_vector_initializer(test_case.enabled); });
    print_field("    ", [&]() { print_double_vector_initializer(test_case.min_velocity); });
    print_field("    ", [&]() { print_double_vector_initializer(test_case.min_acceleration); });
    print_field("    ", [&]() { print_control_vector_initializer(test_case.per_dof_control_interface); });
    print_field("    ", [&]() { print_sync_vector_initializer(test_case.per_dof_synchronization); });
    print_field("    ", [&]() { print_first_time_vector_initializer(test_case.first_time_queries); });
    print_field("    ", [&]() { print_double_vector_initializer(test_case.extra_sample_times); });
    std::cout << "    " << (test_case.compare_first_time_queries ? "true" : "false") << ",\n";
    std::cout << "    " << (test_case.compare_update_loop ? "true" : "false") << ",\n";
    std::cout << "    ";
    print_double_initializer(std::cout, test_case.first_time_tolerance);
    std::cout << "\n";
    std::cout << "});\n";
}

CaseData make_random_case(RandomGenerator& rng, size_t index) {
    const double inf = std::numeric_limits<double>::infinity();
    CaseData test_case;
    test_case.name = "random-" + std::to_string(index);
    test_case.dofs = static_cast<size_t>(1 + rng.pick(3));
    test_case.delta_time = rng.coin() ? 0.01 : 0.1;
    switch (rng.pick(4)) {
        case 0:
            test_case.synchronization = RUCKIG_SYNCHRONIZATION_TIME;
            break;
        case 1:
            test_case.synchronization = RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY;
            break;
        case 2:
            test_case.synchronization = RUCKIG_SYNCHRONIZATION_NONE;
            break;
        default:
            test_case.synchronization = RUCKIG_SYNCHRONIZATION_PHASE;
            break;
    }
    test_case.duration_discretization = rng.coin() ? RUCKIG_DURATION_CONTINUOUS : RUCKIG_DURATION_DISCRETE;
    test_case.current_position.assign(test_case.dofs, 0.0);
    test_case.current_velocity.assign(test_case.dofs, 0.0);
    test_case.current_acceleration.assign(test_case.dofs, 0.0);
    test_case.target_position.assign(test_case.dofs, 0.0);
    test_case.target_velocity.assign(test_case.dofs, 0.0);
    test_case.target_acceleration.assign(test_case.dofs, 0.0);
    test_case.max_velocity.assign(test_case.dofs, 0.0);
    test_case.max_acceleration.assign(test_case.dofs, inf);
    test_case.max_jerk.assign(test_case.dofs, inf);

    switch (rng.pick(6)) {
        case 0: {
            test_case.control_interface = RUCKIG_CONTROL_POSITION;
            for (size_t dof = 0; dof < test_case.dofs; ++dof) {
                test_case.target_position[dof] = rng.range(-4.0, 4.0);
                if (std::abs(test_case.target_position[dof]) < 0.1) {
                    test_case.target_position[dof] += test_case.target_position[dof] < 0.0 ? -0.5 : 0.5;
                }
                test_case.max_velocity[dof] = rng.range(0.5, 3.0);
            }
            break;
        }
        case 1: {
            test_case.control_interface = RUCKIG_CONTROL_POSITION;
            for (size_t dof = 0; dof < test_case.dofs; ++dof) {
                test_case.target_position[dof] = rng.range(-2.0, 2.0);
                if (std::abs(test_case.target_position[dof]) < 0.1) {
                    test_case.target_position[dof] += test_case.target_position[dof] < 0.0 ? -0.5 : 0.5;
                }
                test_case.max_velocity[dof] = rng.range(0.8, 3.0);
                test_case.max_acceleration[dof] = rng.range(0.8, 3.0);
            }
            break;
        }
        case 2: {
            test_case.control_interface = RUCKIG_CONTROL_VELOCITY;
            for (size_t dof = 0; dof < test_case.dofs; ++dof) {
                test_case.target_velocity[dof] = rng.range(-1.5, 1.5);
                if (std::abs(test_case.target_velocity[dof]) < 0.1) {
                    test_case.target_velocity[dof] += test_case.target_velocity[dof] < 0.0 ? -0.5 : 0.5;
                }
                test_case.max_acceleration[dof] = rng.range(0.8, 3.0);
            }
            break;
        }
        case 3: {
            test_case.control_interface = RUCKIG_CONTROL_VELOCITY;
            for (size_t dof = 0; dof < test_case.dofs; ++dof) {
                test_case.target_velocity[dof] = rng.range(-1.2, 1.2);
                if (std::abs(test_case.target_velocity[dof]) < 0.1) {
                    test_case.target_velocity[dof] += test_case.target_velocity[dof] < 0.0 ? -0.5 : 0.5;
                }
                test_case.max_acceleration[dof] = rng.range(0.8, 3.0);
                test_case.max_jerk[dof] = rng.range(0.8, 3.0);
            }
            break;
        }
        case 4: {
            test_case.control_interface = RUCKIG_CONTROL_POSITION;
            for (size_t dof = 0; dof < test_case.dofs; ++dof) {
                test_case.current_position[dof] = rng.range(-0.5, 0.5);
                test_case.target_position[dof] = test_case.current_position[dof] + rng.range(-2.0, 2.0);
                if (std::abs(test_case.target_position[dof] - test_case.current_position[dof]) < 0.2) {
                    test_case.target_position[dof] += test_case.target_position[dof] >= test_case.current_position[dof] ? 0.5 : -0.5;
                }
                test_case.max_velocity[dof] = rng.range(1.2, 3.0);
                test_case.max_acceleration[dof] = rng.range(1.0, 2.5);
                test_case.max_jerk[dof] = rng.range(0.8, 2.0);
                test_case.current_velocity[dof] = rng.range(-0.4, 0.4);
                test_case.target_velocity[dof] = rng.range(-0.4, 0.4);
                test_case.current_acceleration[dof] = rng.range(-0.3, 0.3);
                test_case.target_acceleration[dof] = rng.range(-0.3, 0.3);
            }
            break;
        }
        default: {
            test_case.control_interface = RUCKIG_CONTROL_POSITION;
            test_case.dofs = 1;
            const double direction = rng.coin() ? 1.0 : -1.0;
            test_case.current_position.assign(1, 0.0);
            test_case.current_velocity.assign(1, direction * (rng.coin() ? 0.0 : rng.range(0.1, 0.6)));
            test_case.current_acceleration.assign(1, direction * (rng.coin() ? 0.0 : rng.range(0.1, 0.3)));
            test_case.target_position.assign(1, direction * (rng.coin() ? 2.0 : 5.0));
            test_case.target_velocity.assign(1, std::abs(test_case.target_position[0]) > 3.0 || rng.coin() ? 0.0 : direction * rng.range(0.2, 0.7));
            test_case.target_acceleration.assign(1, direction * (rng.coin() ? 0.0 : rng.range(0.1, 0.3)));
            test_case.max_velocity.assign(1, std::abs(test_case.target_position[0]) > 3.0 ? 1.0 : 2.0);
            test_case.max_acceleration.assign(1, std::abs(test_case.target_position[0]) > 3.0 ? 1.0 : 1.5);
            test_case.max_jerk.assign(1, 1.0);
            break;
        }
    }

    return test_case;
}

CaseData make_random_per_dof_case(RandomGenerator& rng, size_t index) {
    const double inf = std::numeric_limits<double>::infinity();
    CaseData test_case;
    test_case.name = "random-per-dof-" + std::to_string(index);
    test_case.dofs = 2;
    test_case.delta_time = rng.coin() ? 0.01 : 0.05;
    test_case.control_interface = rng.coin() ? RUCKIG_CONTROL_POSITION : RUCKIG_CONTROL_VELOCITY;
    test_case.synchronization = rng.coin() ? RUCKIG_SYNCHRONIZATION_TIME : RUCKIG_SYNCHRONIZATION_NONE;
    test_case.duration_discretization = rng.coin() ? RUCKIG_DURATION_CONTINUOUS : RUCKIG_DURATION_DISCRETE;
    test_case.current_position.assign(2, 0.0);
    test_case.current_velocity.assign(2, 0.0);
    test_case.current_acceleration.assign(2, 0.0);
    test_case.target_position.assign(2, 0.0);
    test_case.target_velocity.assign(2, 0.0);
    test_case.target_acceleration.assign(2, 0.0);
    test_case.max_velocity.assign(2, 0.0);
    test_case.max_acceleration.assign(2, 0.0);
    test_case.max_jerk.assign(2, 0.0);

    switch (rng.pick(5)) {
        case 0:
            test_case.per_dof_control_interface = {RUCKIG_CONTROL_POSITION, RUCKIG_CONTROL_VELOCITY};
            test_case.per_dof_synchronization = {RUCKIG_SYNCHRONIZATION_TIME, RUCKIG_SYNCHRONIZATION_NONE};
            test_case.target_position = {rng.range(0.5, 2.0), 0.0};
            test_case.target_velocity = {0.0, rng.range(0.3, 1.0)};
            test_case.max_velocity = {rng.range(1.0, 2.5), 0.0};
            test_case.max_acceleration = {rng.range(1.0, 2.5), rng.range(1.0, 2.5)};
            test_case.max_jerk = {rng.range(1.0, 2.5), rng.range(1.0, 2.5)};
            break;
        case 1:
            test_case.per_dof_control_interface = {RUCKIG_CONTROL_VELOCITY, RUCKIG_CONTROL_POSITION};
            test_case.per_dof_synchronization = {RUCKIG_SYNCHRONIZATION_NONE, RUCKIG_SYNCHRONIZATION_TIME};
            test_case.target_position = {0.0, -rng.range(0.5, 2.0)};
            test_case.target_velocity = {rng.range(-1.0, -0.3), 0.0};
            test_case.max_velocity = {0.0, rng.range(1.0, 2.5)};
            test_case.max_acceleration = {rng.range(1.0, 2.5), rng.range(1.0, 2.5)};
            test_case.max_jerk = {rng.range(1.0, 2.5), rng.range(1.0, 2.5)};
            break;
        case 2:
            test_case.per_dof_control_interface = {RUCKIG_CONTROL_POSITION, RUCKIG_CONTROL_POSITION};
            test_case.per_dof_synchronization = {RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY, RUCKIG_SYNCHRONIZATION_TIME};
            test_case.target_position = {rng.range(0.5, 1.5), rng.range(1.0, 2.5)};
            test_case.target_velocity = {0.0, rng.range(0.1, 0.4)};
            test_case.max_velocity = {rng.range(1.0, 2.0), rng.range(1.0, 2.0)};
            test_case.max_acceleration = {rng.range(1.0, 2.0), rng.range(1.0, 2.0)};
            test_case.max_jerk = {rng.range(1.0, 2.0), rng.range(1.0, 2.0)};
            break;
        case 3:
            test_case.per_dof_control_interface = {RUCKIG_CONTROL_POSITION, RUCKIG_CONTROL_POSITION};
            test_case.per_dof_synchronization = {RUCKIG_SYNCHRONIZATION_NONE, RUCKIG_SYNCHRONIZATION_TIME};
            test_case.enabled = {true, false};
            test_case.current_position = {0.0, 0.0};
            test_case.current_velocity = {0.0, 0.0};
            test_case.current_acceleration = {0.0, 0.0};
            test_case.target_position = {rng.range(0.5, 2.0), rng.range(5.0, 10.0)};
            test_case.max_velocity = {rng.range(1.0, 2.0), rng.range(1.0, 2.0)};
            test_case.max_acceleration = {rng.range(1.0, 2.0), rng.range(1.0, 2.0)};
            test_case.max_jerk = {rng.range(1.0, 2.0), rng.range(1.0, 2.0)};
            break;
        default:
            test_case.per_dof_control_interface = {RUCKIG_CONTROL_POSITION, RUCKIG_CONTROL_POSITION};
            test_case.per_dof_synchronization = {RUCKIG_SYNCHRONIZATION_TIME, RUCKIG_SYNCHRONIZATION_NONE};
            test_case.target_position = {rng.range(0.5, 1.5), rng.range(1.5, 3.0)};
            test_case.max_velocity = {rng.range(1.0, 2.0), rng.range(1.0, 2.0)};
            test_case.max_acceleration = {inf, rng.range(1.0, 2.0)};
            test_case.max_jerk = {inf, inf};
            break;
    }

    return test_case;
}

void run_random_cases(size_t count, std::uint64_t seed) {
    RandomGenerator rng(seed);
    const int failures_before = failures;
    for (size_t i = 0; i < count; ++i) {
        const int case_failures_before = failures;
        CaseData test_case = make_random_case(rng, i);
        run_case(test_case);
        if (failures != case_failures_before) {
            print_case_repro(test_case, seed, i, "random");
            break;
        }
    }
    if (failures == failures_before) {
        std::cout << "Random oracle comparisons passed: " << count << " seed " << seed << '\n';
    }
}

void run_random_per_dof_cases(size_t count, std::uint64_t seed) {
    RandomGenerator rng(seed);
    const int failures_before = failures;
    for (size_t i = 0; i < count; ++i) {
        const int case_failures_before = failures;
        CaseData test_case = make_random_per_dof_case(rng, i);
        run_case(test_case, false);
        if (failures != case_failures_before) {
            print_case_repro(test_case, seed, i, "random-per-dof");
            break;
        }
    }
    if (failures == failures_before) {
        std::cout << "Random per-DoF oracle comparisons passed: " << count << " seed " << seed << '\n';
    }
}

void replay_random_case(size_t sample_index, std::uint64_t seed) {
    RandomGenerator rng(seed);
    CaseData test_case;
    for (size_t i = 0; i <= sample_index; ++i) {
        test_case = make_random_case(rng, i);
    }
    print_case_fixture_initializer(test_case, seed, sample_index, "random");
    run_case(test_case);
}

void replay_random_per_dof_case(size_t sample_index, std::uint64_t seed) {
    RandomGenerator rng(seed);
    CaseData test_case;
    for (size_t i = 0; i <= sample_index; ++i) {
        test_case = make_random_per_dof_case(rng, i);
    }
    print_case_fixture_initializer(test_case, seed, sample_index, "random-per-dof");
    run_case(test_case, false);
}

CaseData random_case_at_sample(size_t sample_index, std::uint64_t seed, bool per_dof) {
    RandomGenerator rng(seed);
    CaseData test_case;
    for (size_t i = 0; i <= sample_index; ++i) {
        test_case = per_dof ? make_random_per_dof_case(rng, i) : make_random_case(rng, i);
    }
    return test_case;
}

std::string failure_class_from_message(const std::string& message) {
    const size_t detail_start = message.find(": ");
    std::string detail = detail_start == std::string::npos ? message : message.substr(detail_start + 2);
    const char* markers[] = {
        ": C=",
        " at step=",
        " time=",
        " dof=",
        " section=",
        " query=",
        " returned "
    };
    size_t end = std::string::npos;
    for (const char* marker: markers) {
        const size_t found = detail.find(marker);
        if (found != std::string::npos) {
            end = std::min(end, found);
        }
    }
    if (end != std::string::npos) {
        detail.resize(end);
    }
    return detail;
}

struct OracleFailureSignature {
    int count {0};
    std::string first_message;
    std::string failure_class;
};

OracleFailureSignature oracle_case_failure_signature(const CaseData& test_case, bool compare_first_time_queries) {
    const int failures_before = failures;
    const std::string last_failure_before = last_failure_message;
    failures = 0;
    last_failure_message.clear();
    run_case(test_case, compare_first_time_queries);
    OracleFailureSignature signature;
    signature.count = failures;
    signature.first_message = last_failure_message;
    signature.failure_class = failure_class_from_message(last_failure_message);
    failures = failures_before;
    last_failure_message = last_failure_before;
    return signature;
}

bool oracle_case_passes(const CaseData& test_case, bool compare_first_time_queries) {
    return oracle_case_failure_signature(test_case, compare_first_time_queries).count == 0;
}

template <typename T>
void truncate_vector(std::vector<T>& values, size_t count) {
    if (!values.empty() && values.size() > count) {
        values.resize(count);
    }
}

void truncate_case_dofs(CaseData& test_case, size_t dofs) {
    test_case.dofs = dofs;
    truncate_vector(test_case.current_position, dofs);
    truncate_vector(test_case.current_velocity, dofs);
    truncate_vector(test_case.current_acceleration, dofs);
    truncate_vector(test_case.target_position, dofs);
    truncate_vector(test_case.target_velocity, dofs);
    truncate_vector(test_case.target_acceleration, dofs);
    truncate_vector(test_case.max_velocity, dofs);
    truncate_vector(test_case.max_acceleration, dofs);
    truncate_vector(test_case.max_jerk, dofs);
    truncate_vector(test_case.enabled, dofs);
    truncate_vector(test_case.min_velocity, dofs);
    truncate_vector(test_case.min_acceleration, dofs);
    truncate_vector(test_case.per_dof_control_interface, dofs);
    truncate_vector(test_case.per_dof_synchronization, dofs);
    std::vector<FirstTimeQuery> first_time_queries;
    for (const FirstTimeQuery& query: test_case.first_time_queries) {
        if (query.dof < dofs) {
            first_time_queries.push_back(query);
        }
    }
    test_case.first_time_queries = first_time_queries;
}

double rounded_quarter(double value) {
    if (!std::isfinite(value)) {
        return value;
    }
    return std::round(value * 4.0) / 4.0;
}

void round_vector_to_quarters(std::vector<double>& values) {
    for (double& value: values) {
        value = rounded_quarter(value);
    }
}

void round_case_numbers(CaseData& test_case) {
    round_vector_to_quarters(test_case.current_position);
    round_vector_to_quarters(test_case.current_velocity);
    round_vector_to_quarters(test_case.current_acceleration);
    round_vector_to_quarters(test_case.target_position);
    round_vector_to_quarters(test_case.target_velocity);
    round_vector_to_quarters(test_case.target_acceleration);
    round_vector_to_quarters(test_case.max_velocity);
    round_vector_to_quarters(test_case.max_acceleration);
    round_vector_to_quarters(test_case.max_jerk);
    round_vector_to_quarters(test_case.min_velocity);
    round_vector_to_quarters(test_case.min_acceleration);
    round_vector_to_quarters(test_case.extra_sample_times);
    for (FirstTimeQuery& query: test_case.first_time_queries) {
        query.position = rounded_quarter(query.position);
        query.time_after = rounded_quarter(query.time_after);
    }
    if (test_case.has_minimum_duration) {
        test_case.minimum_duration = rounded_quarter(test_case.minimum_duration);
    }
}

bool try_oracle_shrink(
    CaseData& current,
    const char* label,
    bool compare_first_time_queries,
    size_t* accepted_count
) {
    if (oracle_case_passes(current, compare_first_time_queries)) {
        ++*accepted_count;
        std::cout << "oracle shrink accepted " << label << '\n';
        return true;
    }
    return false;
}

template <typename TryCandidate>
void shrink_case_by_strategy(CaseData& test_case, TryCandidate&& try_candidate) {
    if (test_case.dofs > 1) {
        for (size_t dofs = 1; dofs < test_case.dofs; ++dofs) {
            CaseData candidate = test_case;
            truncate_case_dofs(candidate, dofs);
            if (try_candidate(candidate, "dofs")) {
                break;
            }
        }
    }
    if (!test_case.enabled.empty()) {
        CaseData candidate = test_case;
        candidate.enabled.clear();
        try_candidate(candidate, "enabled-mask");
    }
    if (!test_case.per_dof_control_interface.empty()) {
        CaseData candidate = test_case;
        candidate.per_dof_control_interface.clear();
        try_candidate(candidate, "per-dof-control-interface");
    }
    if (!test_case.per_dof_synchronization.empty()) {
        CaseData candidate = test_case;
        candidate.per_dof_synchronization.clear();
        try_candidate(candidate, "per-dof-synchronization");
    }
    if (!test_case.min_velocity.empty()) {
        CaseData candidate = test_case;
        candidate.min_velocity.clear();
        try_candidate(candidate, "min-velocity");
    }
    if (!test_case.min_acceleration.empty()) {
        CaseData candidate = test_case;
        candidate.min_acceleration.clear();
        try_candidate(candidate, "min-acceleration");
    }
    if (test_case.synchronization != RUCKIG_SYNCHRONIZATION_TIME) {
        CaseData candidate = test_case;
        candidate.synchronization = RUCKIG_SYNCHRONIZATION_TIME;
        try_candidate(candidate, "synchronization-time");
    }
    if (test_case.duration_discretization != RUCKIG_DURATION_CONTINUOUS) {
        CaseData candidate = test_case;
        candidate.duration_discretization = RUCKIG_DURATION_CONTINUOUS;
        try_candidate(candidate, "duration-continuous");
    }
    if (test_case.has_minimum_duration) {
        CaseData candidate = test_case;
        candidate.has_minimum_duration = false;
        candidate.minimum_duration = 0.0;
        try_candidate(candidate, "minimum-duration");
    }
    {
        CaseData candidate = test_case;
        round_case_numbers(candidate);
        try_candidate(candidate, "numeric-quarter-rounding");
    }
}

bool try_oracle_shrink_candidate(
    CaseData& current,
    const CaseData& candidate,
    const char* label,
    bool compare_first_time_queries,
    size_t* accepted_count
) {
    CaseData trial = candidate;
    if (try_oracle_shrink(trial, label, compare_first_time_queries, accepted_count)) {
        current = trial;
        return true;
    }
    return false;
}

void shrink_case(CaseData& test_case, bool compare_first_time_queries, size_t* accepted_count) {
    shrink_case_by_strategy(test_case, [&](const CaseData& candidate, const char* label) {
        return try_oracle_shrink_candidate(
            test_case,
            candidate,
            label,
            compare_first_time_queries,
            accepted_count
        );
    });
}

void print_shrink_summary(
    const CaseData& original,
    const CaseData& reduced,
    std::uint64_t seed,
    size_t sample_index,
    const char* kind,
    bool per_dof,
    size_t accepted_count
) {
    std::cout << "oracle shrink original kind=" << kind
        << " seed=" << seed
        << " sample=" << sample_index
        << " name=" << original.name
        << " dofs=" << original.dofs
        << " control=" << control_initializer(original.control_interface)
        << " sync=" << synchronization_initializer(original.synchronization)
        << " discrete=" << discretization_initializer(original.duration_discretization)
        << '\n';
    std::cout << "oracle shrink reduced kind=" << kind
        << " accepted=" << accepted_count
        << " dofs=" << reduced.dofs
        << " control=" << control_initializer(reduced.control_interface)
        << " sync=" << synchronization_initializer(reduced.synchronization)
        << " discrete=" << discretization_initializer(reduced.duration_discretization)
        << " enabled=" << reduced.enabled.size()
        << " per_dof_control=" << reduced.per_dof_control_interface.size()
        << " per_dof_sync=" << reduced.per_dof_synchronization.size()
        << '\n';
    std::cout << "oracle shrink replay command: ruckig_c_oracle_tests.exe "
        << (per_dof ? "--replay-random-per-dof " : "--replay-random ")
        << sample_index << " --seed " << seed << '\n';
    print_case_fixture_initializer(reduced, seed, sample_index, kind);
}

void shrink_random_case(size_t sample_index, std::uint64_t seed, bool per_dof) {
    const char* kind = per_dof ? "random-per-dof" : "random";
    const bool compare_first_time_queries = !per_dof;
    const CaseData original = random_case_at_sample(sample_index, seed, per_dof);
    CaseData reduced = original;
    size_t accepted_count = 0;

    if (!oracle_case_passes(original, compare_first_time_queries)) {
        ++failures;
        std::cerr << "oracle shrink original case failed before shrinking kind=" << kind
            << " seed=" << seed
            << " sample=" << sample_index
            << '\n';
        print_case_repro(original, seed, sample_index, kind);
        return;
    }

    shrink_case(reduced, compare_first_time_queries, &accepted_count);
    if (!oracle_case_passes(reduced, compare_first_time_queries)) {
        ++failures;
        std::cerr << "oracle shrink reduced case failed after accepted simplifications kind=" << kind
            << " seed=" << seed
            << " sample=" << sample_index
            << '\n';
        print_case_repro(reduced, seed, sample_index, kind);
        return;
    }

    print_shrink_summary(original, reduced, seed, sample_index, kind, per_dof, accepted_count);
}

bool try_oracle_failure_shrink_candidate(
    CaseData& current,
    const CaseData& candidate,
    const char* label,
    bool compare_first_time_queries,
    const std::string& required_failure_class,
    size_t* accepted_count
) {
    CaseData trial = candidate;
    const OracleFailureSignature signature = oracle_case_failure_signature(trial, compare_first_time_queries);
    if (signature.count > 0 && signature.failure_class == required_failure_class) {
        ++*accepted_count;
        std::cout << "oracle failure shrink accepted " << label
            << " class=\"" << signature.failure_class << "\"\n";
        current = trial;
        return true;
    }
    return false;
}

void print_failure_shrink_summary(
    const CaseData& original,
    const CaseData& reduced,
    const OracleFailureSignature& original_signature,
    const OracleFailureSignature& reduced_signature,
    std::uint64_t seed,
    size_t sample_index,
    const char* kind,
    bool per_dof,
    size_t accepted_count
) {
    std::cout << "oracle failure shrink original kind=" << kind
        << " seed=" << seed
        << " sample=" << sample_index
        << " name=" << original.name
        << " dofs=" << original.dofs
        << " failure_class=\"" << original_signature.failure_class << "\""
        << " failure_count=" << original_signature.count
        << '\n';
    std::cout << "oracle failure shrink original failure: "
        << original_signature.first_message << '\n';
    std::cout << "oracle failure shrink reduced kind=" << kind
        << " accepted=" << accepted_count
        << " dofs=" << reduced.dofs
        << " control=" << control_initializer(reduced.control_interface)
        << " sync=" << synchronization_initializer(reduced.synchronization)
        << " discrete=" << discretization_initializer(reduced.duration_discretization)
        << " failure_class=\"" << reduced_signature.failure_class << "\""
        << " failure_count=" << reduced_signature.count
        << '\n';
    std::cout << "oracle failure shrink reduced failure: "
        << reduced_signature.first_message << '\n';
    std::cout << "oracle failure shrink original replay command: ruckig_c_oracle_tests.exe "
        << (per_dof ? "--replay-random-per-dof " : "--replay-random ")
        << sample_index << " --seed " << seed << '\n';
    std::cout << "oracle failure shrink reduced replay: paste the initializer below into the fixed oracle corpus\n";
    print_case_fixture_initializer(reduced, seed, sample_index, kind);
}

void shrink_random_failure_case(size_t sample_index, std::uint64_t seed, bool per_dof) {
    const char* kind = per_dof ? "random-per-dof" : "random";
    const bool compare_first_time_queries = !per_dof;
    const CaseData original = random_case_at_sample(sample_index, seed, per_dof);
    CaseData reduced = original;
    size_t accepted_count = 0;

    const OracleFailureSignature original_signature =
        oracle_case_failure_signature(original, compare_first_time_queries);
    if (original_signature.count == 0) {
        ++failures;
        std::cerr << "oracle failure shrink original case did not fail kind=" << kind
            << " seed=" << seed
            << " sample=" << sample_index
            << "; use --shrink-random"
            << (per_dof ? "-per-dof" : "")
            << " for pass-preserving simplification\n";
        std::cerr << "oracle failure shrink replay command: ruckig_c_oracle_tests.exe "
            << (per_dof ? "--replay-random-per-dof " : "--replay-random ")
            << sample_index << " --seed " << seed << '\n';
        return;
    }

    shrink_case_by_strategy(reduced, [&](const CaseData& candidate, const char* label) {
        return try_oracle_failure_shrink_candidate(
            reduced,
            candidate,
            label,
            compare_first_time_queries,
            original_signature.failure_class,
            &accepted_count
        );
    });

    const OracleFailureSignature reduced_signature =
        oracle_case_failure_signature(reduced, compare_first_time_queries);
    if (reduced_signature.count == 0 || reduced_signature.failure_class != original_signature.failure_class) {
        ++failures;
        std::cerr << "oracle failure shrink reduced case no longer preserves failure class kind=" << kind
            << " seed=" << seed
            << " sample=" << sample_index
            << " original_class=\"" << original_signature.failure_class << "\""
            << " reduced_class=\"" << reduced_signature.failure_class << "\"\n";
        print_case_repro(reduced, seed, sample_index, kind);
        return;
    }

    print_failure_shrink_summary(
        original,
        reduced,
        original_signature,
        reduced_signature,
        seed,
        sample_index,
        kind,
        per_dof,
        accepted_count
    );
}

} // namespace

int main(int argc, char** argv) {
    const double inf = std::numeric_limits<double>::infinity();
    std::vector<CaseData> cases;
    size_t random_count = 0;
    size_t random_per_dof_count = 0;
    size_t replay_random_sample = 0;
    size_t replay_random_per_dof_sample = 0;
    size_t shrink_random_sample = 0;
    size_t shrink_random_per_dof_sample = 0;
    size_t shrink_random_failure_sample = 0;
    size_t shrink_random_per_dof_failure_sample = 0;
    std::uint64_t random_seed = 1;
    bool replay_random = false;
    bool replay_random_per_dof = false;
    bool shrink_random = false;
    bool shrink_random_per_dof = false;
    bool shrink_random_failure = false;
    bool shrink_random_per_dof_failure = false;
    bool seed_seen = false;
    bool waypoint_section_oracle_only = false;
    const auto print_usage = [&]() {
        std::cerr << "usage: " << argv[0]
            << " [--random N] [--random-per-dof N]"
            << " [--replay-random SAMPLE --seed S] [--replay-random-per-dof SAMPLE --seed S]"
            << " [--shrink-random SAMPLE --seed S] [--shrink-random-per-dof SAMPLE --seed S]"
            << " [--shrink-random-failure SAMPLE --seed S] [--shrink-random-per-dof-failure SAMPLE --seed S]"
            << " [--seed S] [--waypoint-section-oracle]\n";
    };

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--random" && i + 1 < argc) {
            random_count = static_cast<size_t>(std::stoull(argv[++i]));
        } else if (arg == "--random-per-dof" && i + 1 < argc) {
            random_per_dof_count = static_cast<size_t>(std::stoull(argv[++i]));
        } else if (arg == "--replay-random" && i + 1 < argc) {
            replay_random = true;
            replay_random_sample = static_cast<size_t>(std::stoull(argv[++i]));
        } else if (arg == "--replay-random-per-dof" && i + 1 < argc) {
            replay_random_per_dof = true;
            replay_random_per_dof_sample = static_cast<size_t>(std::stoull(argv[++i]));
        } else if (arg == "--shrink-random" && i + 1 < argc) {
            shrink_random = true;
            shrink_random_sample = static_cast<size_t>(std::stoull(argv[++i]));
        } else if (arg == "--shrink-random-per-dof" && i + 1 < argc) {
            shrink_random_per_dof = true;
            shrink_random_per_dof_sample = static_cast<size_t>(std::stoull(argv[++i]));
        } else if (arg == "--shrink-random-failure" && i + 1 < argc) {
            shrink_random_failure = true;
            shrink_random_failure_sample = static_cast<size_t>(std::stoull(argv[++i]));
        } else if (arg == "--shrink-random-per-dof-failure" && i + 1 < argc) {
            shrink_random_per_dof_failure = true;
            shrink_random_per_dof_failure_sample = static_cast<size_t>(std::stoull(argv[++i]));
        } else if (arg == "--seed" && i + 1 < argc) {
            random_seed = static_cast<std::uint64_t>(std::stoull(argv[++i]));
            seed_seen = true;
        } else if (arg == "--waypoint-section-oracle") {
            waypoint_section_oracle_only = true;
        } else {
            print_usage();
            return 2;
        }
    }

    const int single_sample_modes = (replay_random ? 1 : 0)
        + (replay_random_per_dof ? 1 : 0)
        + (shrink_random ? 1 : 0)
        + (shrink_random_per_dof ? 1 : 0)
        + (shrink_random_failure ? 1 : 0)
        + (shrink_random_per_dof_failure ? 1 : 0);
    if (single_sample_modes > 1
        || (single_sample_modes > 0 && (random_count > 0 || random_per_dof_count > 0 || waypoint_section_oracle_only))
        || (single_sample_modes > 0 && !seed_seen)) {
        print_usage();
        return 2;
    }

    if (waypoint_section_oracle_only) {
        run_waypoint_section_oracle_cases();
        if (failures != 0) {
            std::cerr << failures << " waypoint section oracle comparison failures\n";
            return 1;
        }
        return 0;
    }

    if (replay_random) {
        replay_random_case(replay_random_sample, random_seed);
        if (failures != 0) {
            std::cerr << failures << " oracle comparison failures\n";
            return 1;
        }
        return 0;
    }

    if (replay_random_per_dof) {
        replay_random_per_dof_case(replay_random_per_dof_sample, random_seed);
        if (failures != 0) {
            std::cerr << failures << " oracle comparison failures\n";
            return 1;
        }
        return 0;
    }

    if (shrink_random) {
        shrink_random_case(shrink_random_sample, random_seed, false);
        if (failures != 0) {
            std::cerr << failures << " oracle comparison failures\n";
            return 1;
        }
        return 0;
    }

    if (shrink_random_per_dof) {
        shrink_random_case(shrink_random_per_dof_sample, random_seed, true);
        if (failures != 0) {
            std::cerr << failures << " oracle comparison failures\n";
            return 1;
        }
        return 0;
    }

    if (shrink_random_failure) {
        shrink_random_failure_case(shrink_random_failure_sample, random_seed, false);
        if (failures != 0) {
            std::cerr << failures << " oracle comparison failures\n";
            return 1;
        }
        return 0;
    }

    if (shrink_random_per_dof_failure) {
        shrink_random_failure_case(shrink_random_per_dof_failure_sample, random_seed, true);
        if (failures != 0) {
            std::cerr << failures << " oracle comparison failures\n";
            return 1;
        }
        return 0;
    }

    cases.push_back(CaseData{
        "position-first-order-2d",
        2,
        0.1,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0, 0.0},
        {0.0, 0.0},
        {0.0, 0.0},
        {2.0, -3.0},
        {0.0, 0.0},
        {0.0, 0.0},
        {1.0, 1.5},
        {inf, inf},
        {inf, inf},
        {}
    });

    cases.push_back(CaseData{
        "position-first-order-sync-none-2d",
        2,
        0.1,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_NONE,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0, 0.0},
        {0.0, 0.0},
        {0.0, 0.0},
        {1.0, 3.0},
        {0.0, 0.0},
        {0.0, 0.0},
        {1.0, 1.0},
        {inf, inf},
        {inf, inf},
        {}
    });

    cases.push_back(CaseData{
        "position-first-order-directional-min-velocity",
        1,
        0.1,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0},
        {0.0},
        {0.0},
        {-1.0},
        {0.0},
        {0.0},
        {2.0},
        {inf},
        {inf},
        {},
        {-0.5},
        {}
    });

    cases.push_back(CaseData{
        "position-second-order-1d",
        1,
        0.1,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0},
        {0.0},
        {0.0},
        {1.0},
        {0.0},
        {0.0},
        {1.0},
        {1.0},
        {inf},
        {}
    });

    cases.push_back(CaseData{
        "position-second-order-directional-min-acceleration",
        1,
        0.1,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0},
        {0.0},
        {0.0},
        {-1.0},
        {0.0},
        {0.0},
        {2.0},
        {2.0},
        {inf},
        {},
        {},
        {-0.5}
    });

    cases.push_back(CaseData{
        "position-second-order-discrete-duration",
        1,
        0.3,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_DISCRETE,
        false,
        0.0,
        {0.0},
        {0.0},
        {0.0},
        {1.0},
        {0.0},
        {0.0},
        {1.0},
        {1.0},
        {inf},
        {}
    });

    cases.push_back(CaseData{
        "position-second-order-min-duration",
        1,
        0.1,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        true,
        3.0,
        {0.0},
        {0.0},
        {0.0},
        {1.0},
        {0.0},
        {0.0},
        {1.0},
        {1.0},
        {inf},
        {}
    });

    cases.push_back(CaseData{
        "position-second-order-nonzero-target-velocity",
        1,
        0.05,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0},
        {0.25},
        {0.0},
        {1.4},
        {0.35},
        {0.0},
        {1.8},
        {1.1},
        {inf},
        {}
    });

    cases.push_back(CaseData{
        "position-second-order-stretched-nonzero-velocity",
        1,
        0.05,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        true,
        2.25,
        {0.15},
        {-0.2},
        {0.0},
        {1.1},
        {0.15},
        {0.0},
        {1.6},
        {0.9},
        {inf},
        {}
    });

    cases.push_back(CaseData{
        "position-second-order-2d-step2-time-sync",
        2,
        0.02,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0, 0.6},
        {0.45, -0.25},
        {0.0, 0.0},
        {0.65, -1.4},
        {0.15, -0.1},
        {0.0, 0.0},
        {1.5, 1.1},
        {1.0, 0.85},
        {inf, inf},
        {}
    });

    cases.push_back(CaseData{
        "position-third-order-rest-to-rest",
        1,
        0.1,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0},
        {0.0},
        {0.0},
        {2.0},
        {0.0},
        {0.0},
        {2.0},
        {1.5},
        {1.0},
        {}
    });

    cases.push_back(CaseData{
        "position-third-order-min-duration",
        1,
        0.1,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        true,
        5.0,
        {0.0},
        {0.0},
        {0.0},
        {2.0},
        {0.0},
        {0.0},
        {2.0},
        {1.5},
        {1.0},
        {}
    });

    cases.push_back(CaseData{
        "position-third-order-velocity-limit",
        1,
        0.1,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0},
        {0.0},
        {0.0},
        {5.0},
        {0.0},
        {0.0},
        {1.0},
        {1.0},
        {1.0},
        {}
    });

    cases.push_back(CaseData{
        "position-third-order-nonzero-target-velocity",
        1,
        0.1,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0},
        {0.0},
        {0.0},
        {2.0},
        {0.5},
        {0.0},
        {2.0},
        {1.5},
        {1.0},
        {}
    });

    cases.push_back(CaseData{
        "position-third-order-nonzero-target-acceleration",
        1,
        0.1,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0},
        {0.0},
        {0.0},
        {2.0},
        {0.0},
        {0.2},
        {2.0},
        {1.5},
        {1.0},
        {}
    });

    cases.push_back(CaseData{
        "position-third-order-nonzero-current-velocity",
        1,
        0.1,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0},
        {0.4},
        {0.0},
        {2.0},
        {0.0},
        {0.0},
        {2.0},
        {1.5},
        {1.0},
        {}
    });

    cases.push_back(CaseData{
        "position-third-order-nonzero-current-acceleration",
        1,
        0.1,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0},
        {0.0},
        {0.2},
        {2.0},
        {0.0},
        {0.0},
        {2.0},
        {1.5},
        {1.0},
        {}
    });

    cases.push_back(CaseData{
        "position-third-order-negative-current-acceleration-discrete",
        1,
        0.1,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_DISCRETE,
        false,
        0.0,
        {0.0},
        {0.0},
        {-0.233558},
        {-5.0},
        {0.0},
        {0.0},
        {1.0},
        {1.0},
        {1.0},
        {}
    });

    cases.push_back(CaseData{
        "position-third-order-acc1-vel-uddu-min-duration",
        1,
        0.1,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        true,
        4.7698578863061272,
        {0.0},
        {-0.51482181927210058},
        {-0.21156294175919443},
        {-5.0193501163302532},
        {0.31110281593649936},
        {-0.28066925938475284},
        {1.9139090734704132},
        {1.4978969546406415},
        {1.5534917775119448},
        {}
    });

    cases.push_back(CaseData{
        "position-third-order-acc0-acc1-vel-discrete",
        1,
        0.1,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_DISCRETE,
        false,
        0.0,
        {0.0},
        {0.38606408627260258},
        {-0.65076885047890054},
        {5.5266600128521954},
        {0.55379029593536089},
        {0.1513874612887276},
        {2.388408971072411},
        {0.97941735132244567},
        {1.5803177427926829},
        {}
    });

    cases.push_back(CaseData{
        "position-third-order-vel-udud-min-duration",
        1,
        0.1,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_DISCRETE,
        true,
        4.5959190211700012,
        {0.0},
        {0.19726605344946935},
        {-0.27675836132976905},
        {-0.79290954409606651},
        {-0.77133369011466923},
        {0.15070354333207625},
        {2.2263231476154095},
        {1.9172728916099522},
        {1.54757224273984},
        {}
    });

    cases.push_back(CaseData{
        "position-third-order-acc0-uddu-min-duration",
        1,
        0.1,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        true,
        2.2939437799794864,
        {0.0},
        {0.52686535207542617},
        {0.56254583533791203},
        {-0.77130061481920709},
        {-0.057976180108412323},
        {0.65983741513186567},
        {1.0327516576230431},
        {1.6702266338455245},
        {2.3634052518409341},
        {}
    });

    cases.push_back(CaseData{
        "position-third-order-acc0-acc1-uddu-min-duration",
        1,
        0.1,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        true,
        1.9629868274361006,
        {0.0},
        {-0.73061118650525436},
        {-0.82404688457854991},
        {-0.92203748574924116},
        {-0.77802584378402806},
        {-0.54669861548256871},
        {1.4634951823613171},
        {0.75017236300774959},
        {2.3195268766856549},
        {}
    });

    cases.push_back(CaseData{
        "position-third-order-acc0-vel-udud-discrete",
        1,
        0.1,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_DISCRETE,
        true,
        2.358561998947903,
        {0.0},
        {0.82380905946814076},
        {-0.35340139522766745},
        {0.97956075077388816},
        {0.30074670302942696},
        {0.34825669304994455},
        {0.96626010659506234},
        {0.67940076878414413},
        {2.1494477670043262},
        {}
    });

    cases.push_back(CaseData{
        "position-third-order-acc1-uddu-min-duration",
        1,
        0.1,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        true,
        1.8889294560437007,
        {0.0},
        {0.39736177167706188},
        {-0.35960365815729778},
        {-0.12399467215832688},
        {-0.5384912247892043},
        {-0.81220290312284316},
        {1.3254449548187672},
        {1.3993425741745917},
        {1.1074862309359874},
        {}
    });

    cases.push_back(CaseData{
        "position-third-order-none-udud-discrete",
        1,
        0.1,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_DISCRETE,
        false,
        0.0,
        {0.0},
        {0.84925886611491086},
        {-0.83797206726030948},
        {0.21897122520176024},
        {-0.38746268099260772},
        {-0.28232567501730488},
        {0.71723326287609612},
        {2.2694483838535269},
        {1.2127463368181957},
        {}
    });

    cases.push_back(CaseData{
        "position-third-order-2d-time-sync",
        2,
        0.1,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0, 0.0},
        {0.0, 0.0},
        {0.0, 0.0},
        {2.0, 5.0},
        {0.0, 0.0},
        {0.0, 0.0},
        {2.0, 1.0},
        {1.5, 1.0},
        {1.0, 1.0},
        {}
    });

    cases.push_back(CaseData{
        "position-third-order-3d-online-high-frequency",
        3,
        0.001,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0, -0.25, 0.4},
        {0.05, -0.03, 0.02},
        {0.01, -0.02, 0.015},
        {0.8, -0.7, 1.1},
        {0.0, 0.05, -0.04},
        {0.0, 0.01, -0.015},
        {1.2, 1.1, 1.4},
        {1.5, 1.4, 1.6},
        {2.0, 1.8, 2.2},
        {}
    });

    cases.push_back(CaseData{
        "position-third-order-small-delta-time-discrete",
        1,
        0.0001,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_DISCRETE,
        false,
        0.0,
        {0.0},
        {0.02},
        {-0.01},
        {0.35},
        {0.0},
        {0.0},
        {0.8},
        {1.1},
        {1.7},
        {}
    });

    cases.push_back(CaseData{
        "position-third-order-mixed-disabled-active-3d",
        3,
        0.01,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0, 1.0, -0.5},
        {0.0, 0.4, -0.1},
        {0.0, 0.2, 0.05},
        {1.2, 100.0, -1.4},
        {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0},
        {1.4, 0.7, 1.2},
        {1.1, 0.8, 1.3},
        {1.5, 1.0, 1.7},
        {true, false, true}
    });

    cases.push_back(CaseData{
        "position-third-order-discrete-minimum-duration-3d",
        3,
        0.07,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_DISCRETE,
        true,
        1.23,
        {0.0, 0.1, -0.2},
        {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0},
        {0.9, -0.6, 1.1},
        {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0},
        {1.0, 1.2, 1.1},
        {1.3, 1.1, 1.4},
        {1.8, 1.6, 2.0},
        {}
    });

    cases.push_back(CaseData{
        "position-third-order-directional-min-edge",
        1,
        0.01,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0},
        {0.0},
        {0.0},
        {-0.35},
        {0.0},
        {0.0},
        {1.0},
        {1.0},
        {1.0},
        {},
        {-1.0e-6},
        {-1.0e-6}
    });

    cases.push_back(CaseData{
        "position-third-order-per-dof-synchronization-mixed",
        3,
        0.02,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0, -0.25, 0.4},
        {0.05, -0.18, 0.12},
        {0.02, 0.0, -0.04},
        {1.1, -0.9, 0.65},
        {0.0, 0.05, 0.0},
        {0.0, -0.02, 0.0},
        {1.4, 1.2, 1.1},
        {1.3, 1.0, 1.2},
        {1.7, 1.4, 1.5},
        {},
        {},
        {},
        {},
        {RUCKIG_SYNCHRONIZATION_TIME, RUCKIG_SYNCHRONIZATION_NONE, RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY}
    });

    cases.push_back(CaseData{
        "position-third-order-phase-fallback-nonproportional-state",
        2,
        0.01,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_PHASE,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0, 0.0},
        {0.12, -0.03},
        {0.0, 0.05},
        {1.0, 1.7},
        {0.0, 0.0},
        {0.0, 0.0},
        {1.6, 1.4},
        {1.2, 1.1},
        {1.8, 1.5},
        {}
    });

    cases.push_back(CaseData{
        "position-third-order-directional-min-limits-2d",
        2,
        0.015,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.25, -0.15},
        {-0.05, 0.08},
        {0.03, -0.04},
        {-0.9, 0.95},
        {0.0, -0.02},
        {0.0, 0.01},
        {1.4, 1.3},
        {1.2, 1.1},
        {1.6, 1.5},
        {},
        {-0.75, -0.9},
        {-0.65, -0.7}
    });

    cases.push_back(CaseData{
        "position-second-order-brake-current-velocity-high",
        1,
        0.02,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0},
        {1.4},
        {0.0},
        {2.0},
        {0.0},
        {0.0},
        {1.0},
        {1.0},
        {inf},
        {}
    });

    cases.push_back(CaseData{
        "position-third-order-brake-current-velocity-high",
        1,
        0.02,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0},
        {1.4},
        {0.0},
        {2.0},
        {0.0},
        {0.0},
        {1.0},
        {1.0},
        {2.0},
        {}
    });

    cases.push_back(CaseData{
        "position-third-order-brake-current-velocity-low",
        1,
        0.02,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0},
        {-1.4},
        {0.0},
        {-2.0},
        {0.0},
        {0.0},
        {1.0},
        {1.0},
        {2.0},
        {}
    });

    cases.push_back(CaseData{
        "position-third-order-brake-current-acceleration-high",
        1,
        0.02,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0},
        {0.0},
        {1.6},
        {1.2},
        {0.0},
        {0.0},
        {1.0},
        {1.0},
        {2.0},
        {}
    });

    cases.push_back(CaseData{
        "position-third-order-brake-current-acceleration-low",
        1,
        0.02,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0},
        {0.0},
        {-1.6},
        {-1.2},
        {0.0},
        {0.0},
        {1.0},
        {1.0},
        {2.0},
        {}
    });

    cases.push_back(CaseData{
        "position-second-order-disabled-dof",
        2,
        0.1,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0, 1.0},
        {0.0, 0.5},
        {0.0, 0.2},
        {1.0, 100.0},
        {0.0, 0.0},
        {0.0, 0.0},
        {1.0, 1.0},
        {1.0, 1.0},
        {inf, inf},
        {true, false}
    });

    cases.push_back(CaseData{
        "position-second-order-phase-sync",
        2,
        0.1,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_PHASE,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0, 0.0},
        {0.0, 0.0},
        {0.0, 0.0},
        {1.0, 2.0},
        {0.0, 0.0},
        {0.0, 0.0},
        {2.0, 2.0},
        {2.0, 2.0},
        {inf, inf},
        {},
        {},
        {}
    });

    cases.push_back(CaseData{
        "position-third-order-phase-sync",
        2,
        0.1,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_PHASE,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0, 0.0},
        {0.0, 0.0},
        {0.0, 0.0},
        {1.0, 2.0},
        {0.0, 0.0},
        {0.0, 0.0},
        {2.0, 2.0},
        {2.0, 2.0},
        {1.0, 1.0},
        {},
        {},
        {}
    });

    cases.push_back(CaseData{
        "position-second-order-time-if-necessary-zero-target",
        2,
        0.1,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0, 0.0},
        {0.0, 0.0},
        {0.0, 0.0},
        {1.0, 4.0},
        {0.0, 0.0},
        {0.0, 0.0},
        {1.0, 1.0},
        {1.0, 1.0},
        {inf, inf},
        {},
        {},
        {}
    });

    cases.push_back(CaseData{
        "position-second-order-time-if-necessary-nonzero-target",
        2,
        0.1,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0, 0.0},
        {0.0, 0.0},
        {0.0, 0.0},
        {1.0, 4.0},
        {0.2, 0.0},
        {0.0, 0.0},
        {1.5, 1.0},
        {1.0, 1.0},
        {inf, inf},
        {},
        {},
        {}
    });

    cases.push_back(CaseData{
        "velocity-second-order-1d",
        1,
        0.1,
        RUCKIG_CONTROL_VELOCITY,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0},
        {0.0},
        {0.0},
        {0.0},
        {1.0},
        {0.0},
        {0.0},
        {1.0},
        {inf},
        {}
    });

    cases.push_back(CaseData{
        "velocity-second-order-min-duration",
        1,
        0.1,
        RUCKIG_CONTROL_VELOCITY,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        true,
        2.0,
        {0.0},
        {0.0},
        {0.0},
        {0.0},
        {1.0},
        {0.0},
        {0.0},
        {1.0},
        {inf},
        {}
    });

    cases.push_back(CaseData{
        "velocity-third-order-1d",
        1,
        0.1,
        RUCKIG_CONTROL_VELOCITY,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0},
        {0.0},
        {0.0},
        {0.0},
        {1.0},
        {0.0},
        {0.0},
        {1.0},
        {1.0},
        {}
    });

    cases.push_back(CaseData{
        "velocity-third-order-min-duration",
        1,
        0.1,
        RUCKIG_CONTROL_VELOCITY,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        true,
        3.0,
        {0.0},
        {0.0},
        {0.0},
        {0.0},
        {1.0},
        {0.0},
        {0.0},
        {1.0},
        {1.0},
        {}
    });

    cases.push_back(CaseData{
        "velocity-third-order-near-limits",
        1,
        0.005,
        RUCKIG_CONTROL_VELOCITY,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0},
        {0.94},
        {0.48},
        {0.0},
        {-0.92},
        {-0.47},
        {0.0},
        {0.5},
        {1.4},
        {}
    });

    cases.push_back(CaseData{
        "velocity-third-order-nonzero-target-acceleration",
        1,
        0.02,
        RUCKIG_CONTROL_VELOCITY,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0},
        {0.1},
        {-0.2},
        {0.0},
        {0.9},
        {0.3},
        {0.0},
        {1.2},
        {1.5},
        {}
    });

    cases.push_back(CaseData{
        "velocity-third-order-negative-stretched",
        1,
        0.02,
        RUCKIG_CONTROL_VELOCITY,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        true,
        2.4,
        {0.0},
        {0.65},
        {0.2},
        {0.0},
        {-0.7},
        {-0.25},
        {0.0},
        {1.0},
        {1.3},
        {}
    });

    cases.push_back(CaseData{
        "velocity-third-order-2d-step2-time-sync",
        2,
        0.01,
        RUCKIG_CONTROL_VELOCITY,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0, 0.0},
        {0.15, -0.35},
        {0.25, -0.15},
        {0.0, 0.0},
        {0.95, -0.85},
        {0.1, -0.25},
        {0.0, 0.0},
        {1.2, 0.95},
        {1.4, 1.1},
        {}
    });

    cases.push_back(CaseData{
        "velocity-third-order-brake-current-acceleration-high",
        1,
        0.01,
        RUCKIG_CONTROL_VELOCITY,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0},
        {0.1},
        {1.6},
        {0.0},
        {-0.4},
        {0.0},
        {0.0},
        {1.0},
        {2.0},
        {}
    });

    cases.push_back(CaseData{
        "velocity-third-order-brake-current-acceleration-low",
        1,
        0.01,
        RUCKIG_CONTROL_VELOCITY,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0},
        {-0.1},
        {-1.6},
        {0.0},
        {0.4},
        {0.0},
        {0.0},
        {1.0},
        {2.0},
        {}
    });

    cases.push_back(CaseData{
        "velocity-second-order-phase-sync",
        2,
        0.1,
        RUCKIG_CONTROL_VELOCITY,
        RUCKIG_SYNCHRONIZATION_PHASE,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0, 0.0},
        {0.0, 0.0},
        {0.0, 0.0},
        {0.0, 0.0},
        {-1.0, 0.5},
        {0.0, 0.0},
        {0.0, 0.0},
        {2.0, 1.0},
        {inf, inf},
        {}
    });

    cases.push_back(CaseData{
        "velocity-third-order-phase-sync",
        2,
        0.1,
        RUCKIG_CONTROL_VELOCITY,
        RUCKIG_SYNCHRONIZATION_PHASE,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0, 0.0},
        {0.0, 0.0},
        {0.0, 0.0},
        {0.0, 0.0},
        {-1.0, -0.5},
        {0.0, 0.0},
        {0.0, 0.0},
        {2.0, 1.0},
        {2.0, 1.0},
        {}
    });

    cases.push_back(CaseData{
        "position-third-order-step2-candidate-order-regression",
        2,
        0.01,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {-0.29287540744928786, -0.062351},
        {-0.181154, 0.337315},
        {0.14649687554940716, -0.235179},
        {-0.845995, -1.36219},
        {0.0716699, 0.293374},
        {0.190053, -0.181296},
        {1.28354, 2.14598},
        {2.15556, 1.58114},
        {1.43504, 1.71967},
        {}
    });

    cases.push_back(CaseData{
        "position-third-order-step2-none-uddu-regression",
        2,
        0.01,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.46632436190268045, -0.34342321958359234},
        {0.3516066027914182, 0.069072887473560052},
        {0.1386551411436126, -0.10857816871208797},
        {0.81867793450273041, -1.1460536595860544},
        {0.14816159532383832, 0.29430845475355283},
        {-0.21721020694176763, 0.26704593530952453},
        {1.7321644874039985, 2.3568494113032132},
        {2.3514167673131867, 2.0796567902588272},
        {0.80114047045068326, 1.3530861200814788},
        {}
    });

    cases.push_back(CaseData{
        "position-third-order-none-sync-block-regression",
        2,
        0.01,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_NONE,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {-0.026792148902659019, 0.23769809077448811},
        {-0.015248010395320144, 0.37344344161204102},
        {-0.26379595370324366, -0.01352957312576708},
        {-0.59693200901234356, 0.4588680577566302},
        {-0.34035290886195979, 0.36505633415275884},
        {-0.20491704595682192, 0.13181643110935215},
        {2.7698022126875737, 2.5626762616364323},
        {1.2457942579019192, 1.0851441673204831},
        {1.3710032459860537, 1.1638368312831098},
        {}
    });

    cases.push_back(CaseData{
        "per-dof-global-position-one-velocity-override",
        2,
        0.01,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0, 0.0},
        {0.0, 0.0},
        {0.0, 0.0},
        {1.0, 0.0},
        {0.0, 0.8},
        {0.0, 0.0},
        {1.5, 0.0},
        {1.2, 1.1},
        {2.0, 1.7},
        {},
        {},
        {},
        {RUCKIG_CONTROL_POSITION, RUCKIG_CONTROL_VELOCITY}
    });

    cases.push_back(CaseData{
        "per-dof-global-velocity-one-position-override",
        2,
        0.01,
        RUCKIG_CONTROL_VELOCITY,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0, 0.0},
        {0.0, 0.0},
        {0.0, 0.0},
        {0.0, -1.0},
        {0.7, 0.0},
        {0.0, 0.0},
        {0.0, 1.3},
        {1.0, 1.1},
        {1.6, 1.8},
        {},
        {},
        {},
        {RUCKIG_CONTROL_VELOCITY, RUCKIG_CONTROL_POSITION}
    });

    cases.push_back(CaseData{
        "per-dof-global-time-one-none-sync",
        2,
        0.01,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0, 0.0},
        {0.0, 0.0},
        {0.0, 0.0},
        {1.0, 3.0},
        {0.0, 0.0},
        {0.0, 0.0},
        {1.0, 1.0},
        {1.0, 1.0},
        {2.0, 2.0},
        {},
        {},
        {},
        {},
        {RUCKIG_SYNCHRONIZATION_TIME, RUCKIG_SYNCHRONIZATION_NONE}
    });

    cases.push_back(CaseData{
        "per-dof-global-none-one-time-sync",
        2,
        0.01,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_NONE,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0, 0.0},
        {0.0, 0.0},
        {0.0, 0.0},
        {1.0, 3.0},
        {0.0, 0.0},
        {0.0, 0.0},
        {1.0, 1.0},
        {1.0, 1.0},
        {2.0, 2.0},
        {},
        {},
        {},
        {},
        {RUCKIG_SYNCHRONIZATION_NONE, RUCKIG_SYNCHRONIZATION_TIME}
    });

    cases.push_back(CaseData{
        "per-dof-time-if-necessary-and-time",
        2,
        0.01,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0, 0.0},
        {0.0, 0.0},
        {0.0, 0.0},
        {1.0, 3.0},
        {0.0, 0.2},
        {0.0, 0.0},
        {1.2, 1.2},
        {1.0, 1.0},
        {2.0, 2.0},
        {},
        {},
        {},
        {},
        {RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY, RUCKIG_SYNCHRONIZATION_TIME}
    });

    cases.push_back(CaseData{
        "per-dof-discrete-none-and-time",
        2,
        0.07,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_DISCRETE,
        false,
        0.0,
        {0.0, 0.0},
        {0.0, 0.0},
        {0.0, 0.0},
        {0.8, 2.3},
        {0.0, 0.0},
        {0.0, 0.0},
        {1.1, 1.0},
        {1.2, 1.0},
        {1.6, 1.4},
        {},
        {},
        {},
        {},
        {RUCKIG_SYNCHRONIZATION_NONE, RUCKIG_SYNCHRONIZATION_TIME}
    });

    cases.push_back(CaseData{
        "per-dof-disabled-dof-overrides",
        3,
        0.01,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0, 1.0, -0.2},
        {0.0, 0.4, 0.0},
        {0.0, 0.15, 0.0},
        {1.2, 10.0, -1.4},
        {0.0, 0.8, 0.0},
        {0.0, 0.0, 0.0},
        {1.4, 0.0, 1.2},
        {1.1, 1.0, 1.3},
        {1.5, 1.0, 1.7},
        {true, false, true},
        {},
        {},
        {RUCKIG_CONTROL_POSITION, RUCKIG_CONTROL_VELOCITY, RUCKIG_CONTROL_POSITION},
        {RUCKIG_SYNCHRONIZATION_TIME, RUCKIG_SYNCHRONIZATION_NONE, RUCKIG_SYNCHRONIZATION_TIME}
    });

    cases.push_back(CaseData{
        "per-dof-mixed-order-mixed-control",
        3,
        0.01,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0},
        {1.0, 0.0, -1.5},
        {0.0, 0.7, 0.0},
        {0.0, 0.0, 0.0},
        {1.2, 0.0, 1.4},
        {inf, 1.1, 1.2},
        {inf, 1.7, inf},
        {},
        {},
        {},
        {RUCKIG_CONTROL_POSITION, RUCKIG_CONTROL_VELOCITY, RUCKIG_CONTROL_POSITION},
        {RUCKIG_SYNCHRONIZATION_TIME, RUCKIG_SYNCHRONIZATION_NONE, RUCKIG_SYNCHRONIZATION_TIME}
    });

    cases.push_back(CaseData{
        "per-dof-phase-and-time",
        2,
        0.01,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0, 0.0},
        {0.0, 0.0},
        {0.0, 0.0},
        {1.0, 2.0},
        {0.0, 0.0},
        {0.0, 0.0},
        {2.0, 2.0},
        {2.0, 2.0},
        {1.0, 1.0},
        {},
        {},
        {},
        {},
        {RUCKIG_SYNCHRONIZATION_PHASE, RUCKIG_SYNCHRONIZATION_TIME}
    });

    cases.push_back(CaseData{
        "position-third-order-large-magnitude-small-move",
        2,
        0.01,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {1.0e9, -1.0e9},
        {0.01, -0.02},
        {0.001, -0.0015},
        {1.0e9 + 0.125, -1.0e9 - 0.2},
        {0.0, 0.0},
        {0.0, 0.0},
        {0.4, 0.5},
        {0.3, 0.35},
        {0.8, 0.9},
        {}
    });

    cases.push_back(CaseData{
        "position-third-order-tiny-limits-near-zero",
        1,
        0.001,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0},
        {0.0},
        {0.0},
        {1.0e-6},
        {0.0},
        {0.0},
        {1.0e-6},
        {1.0e-7},
        {1.0e-8},
        {}
    });

    cases.push_back(CaseData{
        "position-third-order-large-min-duration-discrete",
        1,
        0.03,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_DISCRETE,
        true,
        1234.567,
        {0.0},
        {0.0},
        {0.0},
        {0.75},
        {0.0},
        {0.0},
        {1.0},
        {0.8},
        {1.2},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        false,
        false
    });

    cases.push_back(CaseData{
        "per-dof-mixed-first-second-third-with-sync",
        3,
        0.02,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0},
        {1.0, -0.8, 0.0},
        {0.0, 0.0, 0.7},
        {0.0, 0.0, 0.0},
        {1.0, 1.2, 0.0},
        {inf, 0.9, 1.0},
        {inf, inf, 1.5},
        {},
        {},
        {},
        {RUCKIG_CONTROL_POSITION, RUCKIG_CONTROL_POSITION, RUCKIG_CONTROL_VELOCITY},
        {RUCKIG_SYNCHRONIZATION_TIME, RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY, RUCKIG_SYNCHRONIZATION_TIME}
    });

    cases.push_back(CaseData{
        "first-time-at-position-boundaries",
        1,
        0.01,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0},
        {0.0},
        {0.0},
        {1.0},
        {0.0},
        {0.0},
        {1.0},
        {inf},
        {inf},
        {},
        {},
        {},
        {},
        {},
        {
            {0, 0.0, 0.0},
            {0, 0.5, 0.25},
            {0, 1.0, 0.999999}
        }
    });

    cases.push_back(CaseData{
        "position-third-order-disabled-dof-discrete-per-dof-overrides",
        3,
        0.05,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_DISCRETE,
        false,
        0.0,
        {0.0, 1.0, -0.4},
        {0.0, 0.3, -0.05},
        {0.0, 0.12, 0.02},
        {1.0, 5.0, -1.2},
        {0.0, 0.6, 0.0},
        {0.0, 0.0, 0.0},
        {1.2, 0.0, 1.0},
        {1.0, 1.0, 1.1},
        {1.6, 1.0, 1.7},
        {true, false, true},
        {},
        {},
        {RUCKIG_CONTROL_POSITION, RUCKIG_CONTROL_VELOCITY, RUCKIG_CONTROL_POSITION},
        {RUCKIG_SYNCHRONIZATION_TIME, RUCKIG_SYNCHRONIZATION_NONE, RUCKIG_SYNCHRONIZATION_TIME}
    });

    cases.push_back(CaseData{
        "position-third-order-5d-disabled-mixed-sync",
        5,
        0.01,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0, -0.2, 0.4, 1.0, -0.8},
        {0.0, 0.1, -0.15, 0.2, -0.1},
        {0.0, 0.02, -0.03, 0.04, 0.01},
        {1.2, -0.7, 0.9, 3.0, -1.4},
        {0.0, -0.05, 0.0, 0.4, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0},
        {1.3, 1.0, 1.1, 0.0, 1.4},
        {1.2, 0.8, 0.9, 1.0, 1.1},
        {1.8, 1.4, 1.5, 1.0, 1.6},
        {true, true, true, false, true},
        {},
        {},
        {},
        {RUCKIG_SYNCHRONIZATION_TIME, RUCKIG_SYNCHRONIZATION_NONE, RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY, RUCKIG_SYNCHRONIZATION_NONE, RUCKIG_SYNCHRONIZATION_TIME}
    });

    cases.push_back(CaseData{
        "position-third-order-4d-long-online-high-frequency",
        4,
        0.001,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0, -0.5, 0.25, 1.0},
        {0.02, -0.03, 0.04, -0.05},
        {0.005, -0.004, 0.003, -0.002},
        {0.8, -1.1, 0.9, 0.2},
        {0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0},
        {1.2, 1.1, 1.3, 1.0},
        {0.9, 0.8, 1.0, 0.7},
        {1.5, 1.4, 1.6, 1.3},
        {},
        {},
        {},
        {},
        {RUCKIG_SYNCHRONIZATION_TIME, RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY, RUCKIG_SYNCHRONIZATION_NONE, RUCKIG_SYNCHRONIZATION_TIME}
    });

    cases.push_back(CaseData{
        "per-dof-small-delta-mixed-sync-discrete",
        3,
        0.0005,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_DISCRETE,
        false,
        0.0,
        {0.0, 0.15, -0.25},
        {0.0, 0.02, -0.015},
        {0.0, 0.001, -0.002},
        {0.2, -0.35, 0.45},
        {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0},
        {0.6, 0.7, 0.65},
        {0.5, 0.55, 0.45},
        {0.9, 0.85, 0.8},
        {},
        {},
        {},
        {RUCKIG_CONTROL_POSITION, RUCKIG_CONTROL_POSITION, RUCKIG_CONTROL_POSITION},
        {RUCKIG_SYNCHRONIZATION_NONE, RUCKIG_SYNCHRONIZATION_TIME, RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY}
    });

    cases.push_back(CaseData{
        "first-time-segment-boundary-query-combined",
        1,
        0.01,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0},
        {0.0},
        {0.0},
        {2.0},
        {0.0},
        {0.0},
        {1.0},
        {1.0},
        {1.0},
        {},
        {},
        {},
        {},
        {},
        {
            {0, 0.0, 0.0},
            {0, 0.125, 0.0},
            {0, 2.0, 0.0},
            {0, 2.0, 0.999999}
        },
        {0.01, 0.5, 1.0}
    });

    cases.push_back(CaseData{
        "position-multi-disabled-mixed-order-per-dof",
        6,
        0.02,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_DISCRETE,
        false,
        0.0,
        {0.0, 0.2, -0.4, 1.0, -1.5, 0.75},
        {0.0, 0.02, -0.03, 0.1, -0.12, 0.04},
        {0.0, 0.005, -0.004, 0.02, -0.01, 0.003},
        {1.0, 0.0, -1.0, 4.0, -3.0, 0.0},
        {0.0, 0.0, 0.0, 0.3, -0.4, 0.6},
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {1.2, 0.0, 1.1, 0.0, 1.3, 0.0},
        {inf, 1.0, 0.9, 1.0, 1.1, 0.8},
        {inf, 1.0, 1.5, 1.0, 1.6, inf},
        {true, false, true, false, true, true},
        {},
        {},
        {RUCKIG_CONTROL_POSITION, RUCKIG_CONTROL_POSITION, RUCKIG_CONTROL_POSITION, RUCKIG_CONTROL_POSITION, RUCKIG_CONTROL_POSITION, RUCKIG_CONTROL_VELOCITY},
        {RUCKIG_SYNCHRONIZATION_TIME, RUCKIG_SYNCHRONIZATION_NONE, RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY, RUCKIG_SYNCHRONIZATION_NONE, RUCKIG_SYNCHRONIZATION_TIME, RUCKIG_SYNCHRONIZATION_TIME}
    });

    cases.push_back(CaseData{
        "position-third-order-8d-mixed-sync-disabled",
        8,
        0.01,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0, 0.1, -0.2, 0.3, -0.4, 0.5, -0.6, 0.7},
        {0.0, 0.03, -0.02, 0.04, -0.03, 0.02, -0.01, 0.0},
        {0.0, 0.004, -0.003, 0.002, -0.001, 0.003, -0.002, 0.001},
        {0.8, -0.5, 0.6, -0.9, 1.0, -0.2, 0.4, -1.1},
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {1.1, 1.0, 1.2, 1.1, 1.3, 1.0, 1.2, 1.4},
        {0.9, 0.8, 1.0, 0.9, 1.1, 0.8, 0.95, 1.05},
        {1.4, 1.3, 1.5, 1.4, 1.6, 1.3, 1.45, 1.55},
        {true, true, false, true, true, false, true, true},
        {},
        {},
        {},
        {RUCKIG_SYNCHRONIZATION_TIME, RUCKIG_SYNCHRONIZATION_NONE, RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY, RUCKIG_SYNCHRONIZATION_PHASE, RUCKIG_SYNCHRONIZATION_TIME, RUCKIG_SYNCHRONIZATION_NONE, RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY, RUCKIG_SYNCHRONIZATION_TIME}
    });

    cases.push_back(CaseData{
        "per-dof-8d-disabled-mixed-control-overrides",
        8,
        0.02,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0, 0.2, -0.3, 0.4, -0.5, 0.6, -0.7, 0.8},
        {0.0, 0.02, -0.02, 0.03, -0.03, 0.04, -0.04, 0.05},
        {0.0, 0.003, -0.002, 0.004, -0.003, 0.002, -0.001, 0.001},
        {1.0, 0.0, -1.1, 0.0, -0.9, 0.0, 0.6, 0.0},
        {0.0, 0.5, 0.0, -0.45, 0.0, 0.35, 0.0, -0.3},
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {1.2, 0.0, 1.1, 0.0, 1.0, 0.0, 1.3, 0.0},
        {inf, 0.9, 1.0, 0.8, 0.95, 0.85, 1.05, 0.9},
        {inf, 1.4, 1.5, 1.3, 1.45, 1.35, 1.55, 1.4},
        {true, false, true, true, true, false, true, true},
        {},
        {},
        {RUCKIG_CONTROL_POSITION, RUCKIG_CONTROL_VELOCITY, RUCKIG_CONTROL_POSITION, RUCKIG_CONTROL_VELOCITY, RUCKIG_CONTROL_POSITION, RUCKIG_CONTROL_VELOCITY, RUCKIG_CONTROL_POSITION, RUCKIG_CONTROL_VELOCITY},
        {RUCKIG_SYNCHRONIZATION_TIME, RUCKIG_SYNCHRONIZATION_NONE, RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY, RUCKIG_SYNCHRONIZATION_NONE, RUCKIG_SYNCHRONIZATION_TIME, RUCKIG_SYNCHRONIZATION_NONE, RUCKIG_SYNCHRONIZATION_TIME, RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY}
    });

    cases.push_back(CaseData{
        "per-dof-discrete-min-duration-sync-edge",
        4,
        0.06,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_DISCRETE,
        true,
        1.37,
        {0.0, 0.1, -0.15, 0.2},
        {0.0, 0.01, -0.015, 0.02},
        {0.0, 0.001, -0.002, 0.0015},
        {0.9, -0.7, 0.45, -0.35},
        {0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0},
        {1.0, 1.1, 0.9, 1.2},
        {0.8, 0.9, 0.75, 0.95},
        {1.3, 1.4, 1.2, 1.5},
        {},
        {},
        {},
        {},
        {RUCKIG_SYNCHRONIZATION_NONE, RUCKIG_SYNCHRONIZATION_TIME, RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY, RUCKIG_SYNCHRONIZATION_TIME}
    });

    cases.push_back(CaseData{
        "position-large-magnitude-tiny-nonzero-limits",
        2,
        0.1,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_NONE,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {1.0e6, -1.0e6},
        {0.0, 0.0},
        {0.0, 0.0},
        {1.0e6 + 1.0e-6, -1.0e6 - 2.0e-6},
        {0.0, 0.0},
        {0.0, 0.0},
        {1.0e-6, 1.2e-6},
        {1.0e-7, 1.1e-7},
        {1.0e-8, 1.2e-8},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        false,
        false
    });

    cases.push_back(CaseData{
        "position-third-order-4d-very-long-online-accumulated",
        4,
        0.0005,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0, -0.1, 0.2, -0.3},
        {0.005, -0.004, 0.003, -0.002},
        {0.0005, -0.0004, 0.0003, -0.0002},
        {0.12, -0.18, 0.16, -0.14},
        {0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0},
        {0.4, 0.45, 0.42, 0.44},
        {0.35, 0.36, 0.34, 0.37},
        {0.8, 0.85, 0.82, 0.86},
        {},
        {},
        {},
        {},
        {RUCKIG_SYNCHRONIZATION_TIME, RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY, RUCKIG_SYNCHRONIZATION_NONE, RUCKIG_SYNCHRONIZATION_TIME}
    });

    cases.push_back(CaseData{
        "per-dof-8d-discrete-min-duration-time-if-necessary",
        8,
        0.04,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_DISCRETE,
        true,
        2.36,
        {0.0, 0.15, -0.25, 0.35, -0.45, 0.55, -0.65, 0.75},
        {0.0, 0.01, -0.012, 0.014, -0.016, 0.018, -0.02, 0.022},
        {0.0, 0.001, -0.0012, 0.0014, -0.0016, 0.0018, -0.002, 0.0022},
        {0.7, -0.55, 0.6, -0.75, 0.8, -0.4, 0.5, -0.9},
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {1.0, 1.1, 1.05, 1.15, 1.2, 0.95, 1.25, 1.3},
        {0.8, 0.85, 0.82, 0.88, 0.9, 0.78, 0.92, 0.95},
        {1.3, 1.35, 1.32, 1.38, 1.4, 1.28, 1.42, 1.45},
        {},
        {},
        {},
        {},
        {RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY, RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY, RUCKIG_SYNCHRONIZATION_NONE, RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY, RUCKIG_SYNCHRONIZATION_TIME, RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY, RUCKIG_SYNCHRONIZATION_NONE, RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY}
    });

    cases.push_back(CaseData{
        "position-disabled-dof-long-online-constant-acceleration",
        5,
        0.0005,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0, 1.0, -0.2, -1.0, 0.4},
        {0.01, 0.2, -0.015, -0.25, 0.02},
        {0.001, 0.05, -0.0015, -0.04, 0.002},
        {0.16, 3.0, -0.22, -2.0, 0.35},
        {0.0, 0.5, 0.0, -0.4, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0},
        {0.35, 0.0, 0.32, 0.0, 0.34},
        {0.3, 0.8, 0.28, 0.9, 0.29},
        {0.7, 1.0, 0.68, 1.1, 0.69},
        {true, false, true, false, true},
        {},
        {},
        {},
        {RUCKIG_SYNCHRONIZATION_TIME, RUCKIG_SYNCHRONIZATION_NONE, RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY, RUCKIG_SYNCHRONIZATION_NONE, RUCKIG_SYNCHRONIZATION_TIME}
    });

    cases.push_back(CaseData{
        "position-large-duration-first-time-boundary",
        1,
        0.1,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        true,
        20.0,
        {0.0},
        {0.0},
        {0.0},
        {1.0},
        {0.0},
        {0.0},
        {1.0},
        {1.0},
        {1.0},
        {},
        {},
        {},
        {},
        {},
        {
            {0, 0.0, 0.0},
            {0, 1.0, 0.0},
            {0, 1.0, 19.0}
        },
        {0.1, 10.0, 19.9}
    });

    cases.push_back(CaseData{
        "position-very-large-duration-exact-target-first-time",
        1,
        0.1,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        true,
        50.0,
        {0.0},
        {0.0},
        {0.0},
        {1.0},
        {0.0},
        {0.0},
        {10.0},
        {10.0},
        {10.0},
        {},
        {},
        {},
        {},
        {},
        {
            {0, 1.0, 0.0},
            {0, 1.0, 49.0}
        },
        {0.1, 25.0, 49.0, 50.0},
        true,
        false
    });

    // Long near-flat final segment: the frozen oracle and C implementation agree
    // within a documented first-time boundary tolerance exception.
    cases.push_back(CaseData{
        "position-very-large-duration-exact-target-first-time-100s",
        1,
        0.1,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        true,
        100.0,
        {0.0},
        {0.0},
        {0.0},
        {1.0},
        {0.0},
        {0.0},
        {10.0},
        {10.0},
        {10.0},
        {},
        {},
        {},
        {},
        {},
        {
            {0, 1.0, 0.0},
            {0, 1.0, 99.0}
        },
        {0.1, 50.0, 99.0, 100.0},
        true,
        false,
        2.0e-4
    });

    cases.push_back(CaseData{
        "per-dof-mixed-order-none-phase-edge",
        4,
        0.02,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0, -0.2, 0.0, 0.4},
        {0.0, 0.03, 0.0, -0.04},
        {0.0, 0.0, 0.0, 0.003},
        {0.6, -0.6, 0.0, -0.3},
        {0.0, 0.0, 0.45, 0.0},
        {0.0, 0.0, 0.0, 0.0},
        {0.8, 0.9, 0.0, 1.0},
        {inf, 0.7, 0.8, 0.75},
        {inf, inf, 1.2, 1.3},
        {},
        {},
        {},
        {RUCKIG_CONTROL_POSITION, RUCKIG_CONTROL_POSITION, RUCKIG_CONTROL_VELOCITY, RUCKIG_CONTROL_POSITION},
        {RUCKIG_SYNCHRONIZATION_NONE, RUCKIG_SYNCHRONIZATION_PHASE, RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY, RUCKIG_SYNCHRONIZATION_NONE}
    });

    cases.push_back(CaseData{
        "first-time-repeated-position-boundary-fixed",
        1,
        0.01,
        RUCKIG_CONTROL_POSITION,
        RUCKIG_SYNCHRONIZATION_TIME,
        RUCKIG_DURATION_CONTINUOUS,
        false,
        0.0,
        {0.0},
        {0.4},
        {0.0},
        {0.0},
        {-0.3},
        {0.0},
        {1.0},
        {1.0},
        {1.0},
        {},
        {},
        {},
        {},
        {},
        {
            {0, 0.0, 0.0},
            {0, 0.0, 0.001},
            {0, 0.0, 0.1}
        },
        {0.001, 0.01, 0.1}
    });

    for (const auto& test_case: cases) {
        run_case(test_case, test_case.compare_first_time_queries);
    }
    run_waypoint_section_oracle_cases();

    if (failures != 0) {
        std::cerr << failures << " oracle comparison failures\n";
        return 1;
    }

    std::cout << "Oracle comparisons passed: " << cases.size() << '\n';
    if (random_count > 0) {
        run_random_cases(random_count, random_seed);
        if (failures != 0) {
            std::cerr << failures << " oracle comparison failures\n";
            return 1;
        }
    }
    if (random_per_dof_count > 0) {
        run_random_per_dof_cases(random_per_dof_count, random_seed);
        if (failures != 0) {
            std::cerr << failures << " oracle comparison failures\n";
            return 1;
        }
    }
    return 0;
}
