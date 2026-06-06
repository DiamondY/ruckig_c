#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <numeric>
#include <string>
#include <vector>

#include <ruckig_c/ruckig.h>
#include <ruckig/ruckig.hpp>

namespace {

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

struct CaseData {
    size_t dofs {1};
    double delta_time {0.1};
    ruckig_control_interface_t control_interface {RUCKIG_CONTROL_POSITION};
    ruckig_synchronization_t synchronization {RUCKIG_SYNCHRONIZATION_TIME};
    ruckig_duration_discretization_t duration_discretization {RUCKIG_DURATION_CONTINUOUS};
    std::vector<double> current_position;
    std::vector<double> current_velocity;
    std::vector<double> current_acceleration;
    std::vector<double> target_position;
    std::vector<double> target_velocity;
    std::vector<double> target_acceleration;
    std::vector<double> max_velocity;
    std::vector<double> max_acceleration;
    std::vector<double> max_jerk;
};

struct WaypointCaseData {
    size_t dofs {1};
    size_t waypoint_count {1};
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
    std::vector<double> max_position;
    std::vector<double> min_position;
    std::vector<bool> enabled;
    std::vector<double> intermediate_positions;
    std::vector<double> per_section_max_velocity;
    std::vector<double> per_section_min_velocity;
    std::vector<double> per_section_max_acceleration;
    std::vector<double> per_section_min_acceleration;
    std::vector<double> per_section_max_jerk;
    std::vector<double> per_section_max_position;
    std::vector<double> per_section_min_position;
    std::vector<double> per_section_minimum_duration;
};

struct Stats {
    double average_ns {0.0};
    double p99_ns {0.0};
    double worst_ns {0.0};
};

CaseData make_random_case(RandomGenerator& rng) {
    const double inf = std::numeric_limits<double>::infinity();
    CaseData test_case;
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
        case 0:
            test_case.control_interface = RUCKIG_CONTROL_POSITION;
            for (size_t dof = 0; dof < test_case.dofs; ++dof) {
                test_case.target_position[dof] = rng.range(-4.0, 4.0);
                if (std::abs(test_case.target_position[dof]) < 0.1) {
                    test_case.target_position[dof] += test_case.target_position[dof] < 0.0 ? -0.5 : 0.5;
                }
                test_case.max_velocity[dof] = rng.range(0.5, 3.0);
            }
            break;
        case 1:
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
        case 2:
            test_case.control_interface = RUCKIG_CONTROL_VELOCITY;
            for (size_t dof = 0; dof < test_case.dofs; ++dof) {
                test_case.target_velocity[dof] = rng.range(-1.5, 1.5);
                if (std::abs(test_case.target_velocity[dof]) < 0.1) {
                    test_case.target_velocity[dof] += test_case.target_velocity[dof] < 0.0 ? -0.5 : 0.5;
                }
                test_case.max_acceleration[dof] = rng.range(0.8, 3.0);
            }
            break;
        case 3:
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
        case 4:
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
        default: {
            const double direction = rng.coin() ? 1.0 : -1.0;
            test_case.control_interface = RUCKIG_CONTROL_POSITION;
            test_case.dofs = 1;
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

WaypointCaseData make_waypoint_case(size_t index) {
    const double inf = std::numeric_limits<double>::infinity();
    WaypointCaseData test_case;

    switch (index % 10) {
        case 0:
            test_case.dofs = 2;
            test_case.waypoint_count = 1;
            test_case.delta_time = 0.01;
            test_case.current_position = {0.0, 0.0};
            test_case.current_velocity = {0.0, 0.0};
            test_case.current_acceleration = {0.0, 0.0};
            test_case.target_position = {2.0, -1.0};
            test_case.target_velocity = {0.0, 0.0};
            test_case.target_acceleration = {0.0, 0.0};
            test_case.max_velocity = {1.0, 1.0};
            test_case.max_acceleration = {2.0, 2.0};
            test_case.max_jerk = {4.0, 4.0};
            test_case.max_position = {3.0, 1.0};
            test_case.min_position = {-1.0, -2.0};
            test_case.intermediate_positions = {1.0, -0.5};
            break;
        case 1:
            test_case.dofs = 1;
            test_case.waypoint_count = 1;
            test_case.delta_time = 0.01;
            test_case.current_position = {0.0};
            test_case.current_velocity = {0.0};
            test_case.current_acceleration = {0.0};
            test_case.target_position = {2.0};
            test_case.target_velocity = {0.0};
            test_case.target_acceleration = {0.0};
            test_case.max_velocity = {1.5};
            test_case.max_acceleration = {2.0};
            test_case.max_jerk = {5.0};
            test_case.max_position = {inf};
            test_case.min_position = {-inf};
            test_case.intermediate_positions = {1.0};
            test_case.per_section_max_velocity = {0.8, 1.4};
            test_case.per_section_min_velocity = {-0.8, -1.4};
            test_case.per_section_max_acceleration = {1.2, 2.0};
            test_case.per_section_min_acceleration = {-1.2, -2.0};
            test_case.per_section_max_jerk = {3.0, 5.0};
            test_case.per_section_max_position = {1.1, 2.1};
            test_case.per_section_min_position = {-0.1, 0.9};
            break;
        case 2:
            test_case.dofs = 3;
            test_case.waypoint_count = 2;
            test_case.delta_time = 0.02;
            test_case.current_position = {0.0, 0.0, 0.0};
            test_case.current_velocity = {0.0, 0.0, 0.0};
            test_case.current_acceleration = {0.0, 0.0, 0.0};
            test_case.target_position = {1.5, -0.6, 0.75};
            test_case.target_velocity = {0.0, 0.0, 0.0};
            test_case.target_acceleration = {0.0, 0.0, 0.0};
            test_case.max_velocity = {1.5, 1.5, 1.5};
            test_case.max_acceleration = {2.0, 2.0, 2.0};
            test_case.max_jerk = {4.0, 4.0, 4.0};
            test_case.max_position = {inf, inf, inf};
            test_case.min_position = {-inf, -inf, -inf};
            test_case.intermediate_positions = {0.5, -0.2, 0.25, 1.0, -0.4, 0.50};
            break;
        case 3:
            test_case.dofs = 2;
            test_case.waypoint_count = 1;
            test_case.delta_time = 0.05;
            test_case.current_position = {0.0, 5.0};
            test_case.current_velocity = {0.0, 0.0};
            test_case.current_acceleration = {0.0, 0.0};
            test_case.target_position = {2.0, 5.0};
            test_case.target_velocity = {0.0, 0.0};
            test_case.target_acceleration = {0.0, 0.0};
            test_case.max_velocity = {1.2, 1.0};
            test_case.max_acceleration = {2.0, 1.0};
            test_case.max_jerk = {4.0, 2.0};
            test_case.max_position = {inf, inf};
            test_case.min_position = {-inf, -inf};
            test_case.enabled = {true, false};
            test_case.intermediate_positions = {1.0, 5.0};
            break;
        default:
            test_case.dofs = 1;
            test_case.waypoint_count = 1;
            test_case.delta_time = 0.01;
            test_case.current_position = {0.0};
            test_case.current_velocity = {0.0};
            test_case.current_acceleration = {0.0};
            test_case.target_position = {2.0};
            test_case.target_velocity = {0.0};
            test_case.target_acceleration = {0.0};
            test_case.max_velocity = {1.5};
            test_case.max_acceleration = {2.0};
            test_case.max_jerk = {4.0};
            test_case.max_position = {3.0};
            test_case.min_position = {-1.0};
            test_case.intermediate_positions = {1.0};
            test_case.per_section_minimum_duration = {2.0, 1.0};
            break;
        case 5:
            test_case.dofs = 4;
            test_case.waypoint_count = 2;
            test_case.delta_time = 0.01;
            test_case.current_position = {0.0, 0.0, 0.0, 0.0};
            test_case.current_velocity = {0.0, 0.0, 0.0, 0.0};
            test_case.current_acceleration = {0.0, 0.0, 0.0, 0.0};
            test_case.target_position = {1.15, -0.62, 0.70, -0.48};
            test_case.target_velocity = {0.0, 0.0, 0.0, 0.0};
            test_case.target_acceleration = {0.0, 0.0, 0.0, 0.0};
            test_case.max_velocity = {1.1, 1.1, 1.1, 1.1};
            test_case.max_acceleration = {1.8, 1.8, 1.8, 1.8};
            test_case.max_jerk = {4.0, 4.0, 4.0, 4.0};
            test_case.max_position = {inf, inf, inf, inf};
            test_case.min_position = {-inf, -inf, -inf, -inf};
            test_case.intermediate_positions = {
                0.30, -0.15, 0.22, -0.10,
                0.82, -0.42, 0.46, -0.30
            };
            test_case.per_section_max_velocity = {
                0.70, 0.75, 0.70, 0.65,
                0.85, 0.90, 0.82, 0.75,
                1.00, 1.05, 0.92, 0.88
            };
            test_case.per_section_min_velocity = {
                -0.65, -0.75, -0.70, -0.60,
                -0.80, -0.85, -0.78, -0.70,
                -0.90, -0.95, -0.85, -0.80
            };
            test_case.per_section_max_acceleration = {
                1.2, 1.2, 1.1, 1.0,
                1.4, 1.4, 1.3, 1.2,
                1.6, 1.6, 1.5, 1.4
            };
            test_case.per_section_min_acceleration = {
                -1.2, -1.2, -1.1, -1.0,
                -1.4, -1.4, -1.3, -1.2,
                -1.6, -1.6, -1.5, -1.4
            };
            test_case.per_section_max_jerk = {
                3.0, 3.0, 2.8, 2.6,
                3.4, 3.4, 3.2, 3.0,
                3.8, 3.8, 3.5, 3.3
            };
            test_case.per_section_max_position = {
                0.35, 0.05, 0.25, 0.05,
                0.88, -0.10, 0.50, -0.05,
                1.25, -0.38, 0.78, -0.22
            };
            test_case.per_section_min_position = {
                -0.05, -0.20, -0.05, -0.15,
                0.25, -0.50, 0.15, -0.35,
                0.75, -0.75, 0.38, -0.55
            };
            test_case.per_section_minimum_duration = {0.40, 0.70, 0.50};
            break;
        case 6:
            test_case.dofs = 8;
            test_case.waypoint_count = 3;
            test_case.delta_time = 0.01;
            test_case.current_position = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 7.0, -2.0};
            test_case.current_velocity = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
            test_case.current_acceleration = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
            test_case.target_position = {1.00, -0.65, 0.75, -0.55, 0.45, -0.33, 7.0, -2.0};
            test_case.target_velocity = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
            test_case.target_acceleration = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
            test_case.max_velocity = {1.8, 1.8, 1.8, 1.8, 1.8, 1.8, 1.8, 1.8};
            test_case.max_acceleration = {2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5};
            test_case.max_jerk = {5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0};
            test_case.max_position = {inf, inf, inf, inf, inf, inf, inf, inf};
            test_case.min_position = {-inf, -inf, -inf, -inf, -inf, -inf, -inf, -inf};
            test_case.enabled = {true, true, true, true, true, true, false, false};
            test_case.intermediate_positions = {
                0.10, -0.05, 0.12, -0.08, 0.06, -0.04, 7.0, -2.0,
                0.35, -0.20, 0.30, -0.25, 0.18, -0.12, 7.0, -2.0,
                0.70, -0.45, 0.55, -0.40, 0.32, -0.24, 7.0, -2.0
            };
            break;
        case 7:
            test_case.dofs = 1;
            test_case.waypoint_count = 2;
            test_case.delta_time = 0.01;
            test_case.current_position = {0.0};
            test_case.current_velocity = {0.20};
            test_case.current_acceleration = {0.0};
            test_case.target_position = {1.40};
            test_case.target_velocity = {-0.10};
            test_case.target_acceleration = {0.0};
            test_case.max_velocity = {1.2};
            test_case.max_acceleration = {2.4};
            test_case.max_jerk = {5.0};
            test_case.max_position = {2.0};
            test_case.min_position = {-0.5};
            test_case.intermediate_positions = {0.45, 0.95};
            break;
        case 8:
            test_case.dofs = 2;
            test_case.waypoint_count = 2;
            test_case.delta_time = 0.02;
            test_case.current_position = {0.0, 0.0};
            test_case.current_velocity = {0.0, 0.0};
            test_case.current_acceleration = {0.0, 0.0};
            test_case.target_position = {1.6, -0.8};
            test_case.target_velocity = {0.0, 0.0};
            test_case.target_acceleration = {0.0, 0.0};
            test_case.max_velocity = {1.4, 1.2};
            test_case.max_acceleration = {2.0, 1.8};
            test_case.max_jerk = {4.0, 3.5};
            test_case.max_position = {1.7, 0.1};
            test_case.min_position = {-0.1, -0.9};
            test_case.intermediate_positions = {0.45, -0.20, 1.05, -0.55};
            test_case.per_section_minimum_duration = {0.25, 0.45, 0.30};
            break;
        case 9:
            test_case.dofs = 3;
            test_case.waypoint_count = 3;
            test_case.delta_time = 0.05;
            test_case.current_position = {0.0, 0.0, 0.0};
            test_case.current_velocity = {0.10, -0.05, 0.0};
            test_case.current_acceleration = {0.0, 0.0, 0.0};
            test_case.target_position = {1.20, -0.90, 0.60};
            test_case.target_velocity = {-0.05, 0.05, 0.0};
            test_case.target_acceleration = {0.0, 0.0, 0.0};
            test_case.max_velocity = {1.1, 1.1, 1.0};
            test_case.max_acceleration = {1.6, 1.6, 1.5};
            test_case.max_jerk = {3.5, 3.5, 3.0};
            test_case.max_position = {1.4, 0.1, 0.8};
            test_case.min_position = {-0.1, -1.1, -0.1};
            test_case.intermediate_positions = {
                0.25, -0.20, 0.15,
                0.55, -0.45, 0.30,
                0.90, -0.70, 0.45
            };
            break;
    }

    if (test_case.enabled.empty()) {
        test_case.enabled.assign(test_case.dofs, true);
    }

    return test_case;
}

void fill_c_input(const CaseData& test_case, ruckig_input_t* input) {
    ruckig_input_set_control_interface(input, test_case.control_interface);
    ruckig_input_set_synchronization(input, test_case.synchronization);
    ruckig_input_set_duration_discretization(input, test_case.duration_discretization);
    for (size_t i = 0; i < test_case.dofs; ++i) {
        ruckig_input_current_position_data(input)[i] = test_case.current_position[i];
        ruckig_input_current_velocity_data(input)[i] = test_case.current_velocity[i];
        ruckig_input_current_acceleration_data(input)[i] = test_case.current_acceleration[i];
        ruckig_input_target_position_data(input)[i] = test_case.target_position[i];
        ruckig_input_target_velocity_data(input)[i] = test_case.target_velocity[i];
        ruckig_input_target_acceleration_data(input)[i] = test_case.target_acceleration[i];
        ruckig_input_max_velocity_data(input)[i] = test_case.max_velocity[i];
        ruckig_input_max_acceleration_data(input)[i] = test_case.max_acceleration[i];
        ruckig_input_max_jerk_data(input)[i] = test_case.max_jerk[i];
    }
}

ruckig_result_t fill_waypoint_input(const WaypointCaseData& test_case, ruckig_input_t* input) {
    ruckig_input_clear_intermediate_positions(input);
    ruckig_input_clear_min_velocity(input);
    ruckig_input_clear_min_acceleration(input);
    ruckig_input_clear_minimum_duration(input);
    ruckig_input_clear_per_dof_control_interface(input);
    ruckig_input_clear_per_dof_synchronization(input);
    ruckig_input_clear_per_section_max_velocity(input);
    ruckig_input_clear_per_section_min_velocity(input);
    ruckig_input_clear_per_section_max_acceleration(input);
    ruckig_input_clear_per_section_min_acceleration(input);
    ruckig_input_clear_per_section_max_jerk(input);
    ruckig_input_clear_per_section_max_position(input);
    ruckig_input_clear_per_section_min_position(input);
    ruckig_input_clear_per_section_minimum_duration(input);
    ruckig_input_clear_interrupt_calculation_duration(input);

    if (ruckig_input_set_control_interface(input, RUCKIG_CONTROL_POSITION) != RUCKIG_WORKING
        || ruckig_input_set_synchronization(input, RUCKIG_SYNCHRONIZATION_TIME) != RUCKIG_WORKING
        || ruckig_input_set_duration_discretization(input, RUCKIG_DURATION_CONTINUOUS) != RUCKIG_WORKING) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }

    for (size_t i = 0; i < test_case.dofs; ++i) {
        ruckig_input_current_position_data(input)[i] = test_case.current_position[i];
        ruckig_input_current_velocity_data(input)[i] = test_case.current_velocity[i];
        ruckig_input_current_acceleration_data(input)[i] = test_case.current_acceleration[i];
        ruckig_input_target_position_data(input)[i] = test_case.target_position[i];
        ruckig_input_target_velocity_data(input)[i] = test_case.target_velocity[i];
        ruckig_input_target_acceleration_data(input)[i] = test_case.target_acceleration[i];
        ruckig_input_max_velocity_data(input)[i] = test_case.max_velocity[i];
        ruckig_input_max_acceleration_data(input)[i] = test_case.max_acceleration[i];
        ruckig_input_max_jerk_data(input)[i] = test_case.max_jerk[i];
        ruckig_input_max_position_data(input)[i] = test_case.max_position[i];
        ruckig_input_min_position_data(input)[i] = test_case.min_position[i];
        if (ruckig_input_set_dof_enabled(input, i, test_case.enabled[i]) != RUCKIG_WORKING) {
            return RUCKIG_ERROR_INVALID_INPUT;
        }
    }

    if (ruckig_input_set_intermediate_positions(
            input,
            test_case.intermediate_positions.data(),
            test_case.waypoint_count,
            test_case.dofs) != RUCKIG_WORKING) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }

    const size_t section_count = test_case.waypoint_count + 1;
    if (!test_case.per_section_max_velocity.empty()
        && ruckig_input_set_per_section_max_velocity(input, test_case.per_section_max_velocity.data(), section_count, test_case.dofs) != RUCKIG_WORKING) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    if (!test_case.per_section_min_velocity.empty()
        && ruckig_input_set_per_section_min_velocity(input, test_case.per_section_min_velocity.data(), section_count, test_case.dofs) != RUCKIG_WORKING) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    if (!test_case.per_section_max_acceleration.empty()
        && ruckig_input_set_per_section_max_acceleration(input, test_case.per_section_max_acceleration.data(), section_count, test_case.dofs) != RUCKIG_WORKING) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    if (!test_case.per_section_min_acceleration.empty()
        && ruckig_input_set_per_section_min_acceleration(input, test_case.per_section_min_acceleration.data(), section_count, test_case.dofs) != RUCKIG_WORKING) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    if (!test_case.per_section_max_jerk.empty()
        && ruckig_input_set_per_section_max_jerk(input, test_case.per_section_max_jerk.data(), section_count, test_case.dofs) != RUCKIG_WORKING) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    if (!test_case.per_section_max_position.empty()
        && ruckig_input_set_per_section_max_position(input, test_case.per_section_max_position.data(), section_count, test_case.dofs) != RUCKIG_WORKING) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    if (!test_case.per_section_min_position.empty()
        && ruckig_input_set_per_section_min_position(input, test_case.per_section_min_position.data(), section_count, test_case.dofs) != RUCKIG_WORKING) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    if (!test_case.per_section_minimum_duration.empty()
        && ruckig_input_set_per_section_minimum_duration(input, test_case.per_section_minimum_duration.data(), section_count) != RUCKIG_WORKING) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }

    return RUCKIG_WORKING;
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
}

Stats summarize(std::vector<double>& samples) {
    Stats stats;
    if (samples.empty()) {
        return stats;
    }
    std::sort(samples.begin(), samples.end());
    const double sum = std::accumulate(samples.begin(), samples.end(), 0.0);
    const size_t p99_index = samples.size() > 1 ? static_cast<size_t>(std::floor(0.99 * static_cast<double>(samples.size() - 1))) : 0;
    stats.average_ns = sum / static_cast<double>(samples.size());
    stats.p99_ns = samples[p99_index];
    stats.worst_ns = samples.back();
    return stats;
}

const char* compiler_string() {
#if defined(__clang__)
    return "clang " __clang_version__;
#elif defined(__GNUC__)
    return "gcc " __VERSION__;
#elif defined(_MSC_VER)
    return "msvc";
#else
    return "unknown";
#endif
}

const char* os_string() {
#if defined(_WIN32)
    return "Windows";
#elif defined(__APPLE__)
    return "macOS";
#elif defined(__linux__)
    return "Linux";
#else
    return "unknown";
#endif
}

int run_waypoint_benchmark(size_t sample_count, std::uint64_t seed) {
    constexpr size_t max_dofs = 8;
    constexpr size_t max_number_of_waypoints = 3;
    constexpr size_t waypoint_case_count = 10;
    std::vector<WaypointCaseData> cases;
    cases.reserve(sample_count);
    for (size_t i = 0; i < sample_count; ++i) {
        cases.push_back(make_waypoint_case(static_cast<size_t>(seed) + i));
    }

    ruckig_t* c_otg[3][max_dofs] = {};
    ruckig_input_t* c_input[max_dofs] = {};
    ruckig_trajectory_t* c_trajectory[max_dofs] = {};
    for (size_t dof = 1; dof <= max_dofs; ++dof) {
        if (ruckig_input_create_with_waypoints(&c_input[dof - 1], dof, max_number_of_waypoints) != RUCKIG_WORKING
            || ruckig_trajectory_create_with_waypoints(&c_trajectory[dof - 1], dof, max_number_of_waypoints) != RUCKIG_WORKING
            || ruckig_create_with_waypoints(&c_otg[0][dof - 1], dof, 0.01, max_number_of_waypoints) != RUCKIG_WORKING
            || ruckig_create_with_waypoints(&c_otg[1][dof - 1], dof, 0.02, max_number_of_waypoints) != RUCKIG_WORKING
            || ruckig_create_with_waypoints(&c_otg[2][dof - 1], dof, 0.05, max_number_of_waypoints) != RUCKIG_WORKING) {
            std::cerr << "failed to create C waypoint benchmark handles\n";
            return 1;
        }
    }

    std::vector<double> c_samples;
    c_samples.reserve(sample_count);

    for (const WaypointCaseData& test_case: cases) {
        const size_t dof_index = test_case.dofs - 1;
        const size_t dt_index = test_case.delta_time < 0.015 ? 0 : (test_case.delta_time < 0.035 ? 1 : 2);
        if (fill_waypoint_input(test_case, c_input[dof_index]) != RUCKIG_WORKING) {
            std::cerr << "failed to fill waypoint benchmark input\n";
            return 1;
        }
        auto start = std::chrono::steady_clock::now();
        const ruckig_result_t result = ruckig_calculate(c_otg[dt_index][dof_index], c_input[dof_index], c_trajectory[dof_index]);
        auto end = std::chrono::steady_clock::now();
        if (result != RUCKIG_WORKING) {
            std::cerr << "C waypoint benchmark calculate failed with result " << static_cast<int>(result) << '\n';
            return 1;
        }
        c_samples.push_back(static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()));
    }

    Stats c_stats = summarize(c_samples);

    std::cout << "Ruckig C waypoint alpha benchmark\n";
    std::cout << "samples: " << sample_count << '\n';
    std::cout << "seed: " << seed << '\n';
    std::cout << "compiler: " << compiler_string() << '\n';
    std::cout << "os: " << os_string() << '\n';
    std::cout << "waypoint_case_count: " << waypoint_case_count << '\n';
    std::cout << "waypoint_max_dofs: " << max_dofs << '\n';
    std::cout << "waypoint_max_intermediate_positions: " << max_number_of_waypoints << '\n';
    std::cout << "waypoint_c_average_ns: " << c_stats.average_ns << '\n';
    std::cout << "waypoint_c_p99_ns: " << c_stats.p99_ns << '\n';
    std::cout << "waypoint_c_worst_ns: " << c_stats.worst_ns << '\n';
    std::cout << "waypoint_oracle_ratio: unavailable\n";
    std::cout << "waypoint_benchmark_policy: alpha C-only local optimizer corpus\n";

    for (size_t dof = 1; dof <= max_dofs; ++dof) {
        ruckig_destroy(c_otg[0][dof - 1]);
        ruckig_destroy(c_otg[1][dof - 1]);
        ruckig_destroy(c_otg[2][dof - 1]);
        ruckig_input_destroy(c_input[dof - 1]);
        ruckig_trajectory_destroy(c_trajectory[dof - 1]);
    }

    return 0;
}

} // namespace

int main(int argc, char** argv) {
    size_t sample_count = 10000;
    std::uint64_t seed = 1;
    bool enforce_threshold = false;
    bool waypoint_benchmark = false;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--samples" && i + 1 < argc) {
            sample_count = static_cast<size_t>(std::strtoull(argv[++i], nullptr, 10));
        } else if (arg == "--seed" && i + 1 < argc) {
            seed = static_cast<std::uint64_t>(std::strtoull(argv[++i], nullptr, 10));
        } else if (arg == "--enforce-threshold") {
            enforce_threshold = true;
        } else if (arg == "--waypoints") {
            waypoint_benchmark = true;
        } else {
            std::cerr << "usage: " << argv[0] << " [--samples N] [--seed S] [--enforce-threshold] [--waypoints]\n";
            return 2;
        }
    }

    if (waypoint_benchmark) {
        return run_waypoint_benchmark(sample_count, seed);
    }

    RandomGenerator rng(seed);
    std::vector<CaseData> cases;
    cases.reserve(sample_count);
    for (size_t i = 0; i < sample_count; ++i) {
        cases.push_back(make_random_case(rng));
    }

    ruckig_t* c_otg[2][3] = {};
    ruckig_input_t* c_input[3] = {};
    ruckig_trajectory_t* c_trajectory[3] = {};
    for (size_t dof = 1; dof <= 3; ++dof) {
        if (ruckig_input_create(&c_input[dof - 1], dof) != RUCKIG_WORKING
            || ruckig_trajectory_create(&c_trajectory[dof - 1], dof) != RUCKIG_WORKING
            || ruckig_create(&c_otg[0][dof - 1], dof, 0.01) != RUCKIG_WORKING
            || ruckig_create(&c_otg[1][dof - 1], dof, 0.1) != RUCKIG_WORKING) {
            std::cerr << "failed to create C benchmark handles\n";
            return 1;
        }
    }

    ruckig::Ruckig<ruckig::DynamicDOFs> oracle_otg_001[3] = {
        ruckig::Ruckig<ruckig::DynamicDOFs>(1, 0.01),
        ruckig::Ruckig<ruckig::DynamicDOFs>(2, 0.01),
        ruckig::Ruckig<ruckig::DynamicDOFs>(3, 0.01),
    };
    ruckig::Ruckig<ruckig::DynamicDOFs> oracle_otg_01[3] = {
        ruckig::Ruckig<ruckig::DynamicDOFs>(1, 0.1),
        ruckig::Ruckig<ruckig::DynamicDOFs>(2, 0.1),
        ruckig::Ruckig<ruckig::DynamicDOFs>(3, 0.1),
    };
    std::vector<ruckig::InputParameter<ruckig::DynamicDOFs>> oracle_inputs;
    std::vector<ruckig::Trajectory<ruckig::DynamicDOFs>> oracle_trajectories;
    for (size_t dof = 1; dof <= 3; ++dof) {
        oracle_inputs.emplace_back(dof);
        oracle_trajectories.emplace_back(dof);
    }

    std::vector<double> c_samples;
    std::vector<double> oracle_samples;
    c_samples.reserve(sample_count);
    oracle_samples.reserve(sample_count);

    for (const CaseData& test_case: cases) {
        const size_t dof_index = test_case.dofs - 1;
        const size_t dt_index = test_case.delta_time < 0.05 ? 0 : 1;
        fill_c_input(test_case, c_input[dof_index]);
        auto start = std::chrono::steady_clock::now();
        const ruckig_result_t result = ruckig_calculate(c_otg[dt_index][dof_index], c_input[dof_index], c_trajectory[dof_index]);
        auto end = std::chrono::steady_clock::now();
        if (result != RUCKIG_WORKING) {
            std::cerr << "C benchmark calculate failed with result " << static_cast<int>(result) << '\n';
            return 1;
        }
        c_samples.push_back(static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()));
    }

    for (const CaseData& test_case: cases) {
        const size_t dof_index = test_case.dofs - 1;
        fill_oracle_input(test_case, oracle_inputs[dof_index]);
        auto& otg = test_case.delta_time < 0.05 ? oracle_otg_001[dof_index] : oracle_otg_01[dof_index];
        auto start = std::chrono::steady_clock::now();
        const auto result = otg.calculate(oracle_inputs[dof_index], oracle_trajectories[dof_index]);
        auto end = std::chrono::steady_clock::now();
        if (result != ruckig::Result::Working) {
            std::cerr << "oracle benchmark calculate failed with result " << static_cast<int>(result) << '\n';
            return 1;
        }
        oracle_samples.push_back(static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()));
    }

    Stats c_stats = summarize(c_samples);
    Stats oracle_stats = summarize(oracle_samples);
    const double average_ratio = oracle_stats.average_ns > 0.0 ? c_stats.average_ns / oracle_stats.average_ns : std::numeric_limits<double>::infinity();

    std::cout << "Ruckig C performance benchmark\n";
    std::cout << "samples: " << sample_count << '\n';
    std::cout << "seed: " << seed << '\n';
    std::cout << "compiler: " << compiler_string() << '\n';
    std::cout << "os: " << os_string() << '\n';
    std::cout << "c_average_ns: " << c_stats.average_ns << '\n';
    std::cout << "c_p99_ns: " << c_stats.p99_ns << '\n';
    std::cout << "c_worst_ns: " << c_stats.worst_ns << '\n';
    std::cout << "oracle_average_ns: " << oracle_stats.average_ns << '\n';
    std::cout << "oracle_p99_ns: " << oracle_stats.p99_ns << '\n';
    std::cout << "oracle_worst_ns: " << oracle_stats.worst_ns << '\n';
    std::cout << "average_ratio_c_over_oracle: " << average_ratio << '\n';
    std::cout << "release_threshold_average_ratio: 1.5\n";

    for (size_t dof = 1; dof <= 3; ++dof) {
        ruckig_destroy(c_otg[0][dof - 1]);
        ruckig_destroy(c_otg[1][dof - 1]);
        ruckig_input_destroy(c_input[dof - 1]);
        ruckig_trajectory_destroy(c_trajectory[dof - 1]);
    }

    if (enforce_threshold && average_ratio > 1.5) {
        return 1;
    }
    return 0;
}
