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

} // namespace

int main(int argc, char** argv) {
    size_t sample_count = 10000;
    std::uint64_t seed = 1;
    bool enforce_threshold = false;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--samples" && i + 1 < argc) {
            sample_count = static_cast<size_t>(std::strtoull(argv[++i], nullptr, 10));
        } else if (arg == "--seed" && i + 1 < argc) {
            seed = static_cast<std::uint64_t>(std::strtoull(argv[++i], nullptr, 10));
        } else if (arg == "--enforce-threshold") {
            enforce_threshold = true;
        } else {
            std::cerr << "usage: " << argv[0] << " [--samples N] [--seed S] [--enforce-threshold]\n";
            return 2;
        }
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
