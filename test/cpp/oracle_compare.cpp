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
};

int failures = 0;

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
    std::cerr << name << ": " << message << '\n';
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
        if (c_found && !near(c_time, *oracle_time, kFirstTimeTolerance)) {
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

void print_case_repro(const CaseData& test_case) {
    std::cerr.precision(17);
    std::cerr << "repro name=" << test_case.name
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
    print_control_vector("per_dof_control_interface", test_case.per_dof_control_interface);
    print_sync_vector("per_dof_synchronization", test_case.per_dof_synchronization);
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
            print_case_repro(test_case);
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
            print_case_repro(test_case);
            break;
        }
    }
    if (failures == failures_before) {
        std::cout << "Random per-DoF oracle comparisons passed: " << count << " seed " << seed << '\n';
    }
}

} // namespace

int main(int argc, char** argv) {
    const double inf = std::numeric_limits<double>::infinity();
    std::vector<CaseData> cases;
    size_t random_count = 0;
    size_t random_per_dof_count = 0;
    std::uint64_t random_seed = 1;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--random" && i + 1 < argc) {
            random_count = static_cast<size_t>(std::stoull(argv[++i]));
        } else if (arg == "--random-per-dof" && i + 1 < argc) {
            random_per_dof_count = static_cast<size_t>(std::stoull(argv[++i]));
        } else if (arg == "--seed" && i + 1 < argc) {
            random_seed = static_cast<std::uint64_t>(std::stoull(argv[++i]));
        } else {
            std::cerr << "usage: " << argv[0] << " [--random N] [--random-per-dof N] [--seed S]\n";
            return 2;
        }
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
