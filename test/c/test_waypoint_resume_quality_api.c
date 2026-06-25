#include "test_api_internal.h"

#define WAYPOINT_RESUME_QUALITY_CASES 128u

typedef struct waypoint_resume_quality_baseline {
    ruckig_result_t result;
    double duration;
} waypoint_resume_quality_baseline_t;

static const waypoint_resume_quality_baseline_t waypoint_resume_quality_baseline[WAYPOINT_RESUME_QUALITY_CASES] = {
    {RUCKIG_WORKING, 1.8761222240029074},
    {RUCKIG_WORKING, 2.0103531913159798},
    {RUCKIG_WORKING, 2.1367785811810092},
    {RUCKIG_WORKING, 2.3231046365914789},
    {RUCKIG_WORKING, 1.9563060529630401},
    {RUCKIG_WORKING, 2.1639081016881558},
    {RUCKIG_WORKING, 2.334308960646033},
    {RUCKIG_WORKING, 2.463331019114928},
    {RUCKIG_WORKING, 2.0843835602493597},
    {RUCKIG_WORKING, 2.9613331216506076},
    {RUCKIG_WORKING, 3.0447526376853462},
    {RUCKIG_WORKING, 2.9220220742529053},
    {RUCKIG_WORKING, 1.8927697784843993},
    {RUCKIG_WORKING, 2.0322608374341988},
    {RUCKIG_WORKING, 2.1567261904761903},
    {RUCKIG_WORKING, 2.2663324979114456},
    {RUCKIG_WORKING, 1.9684155547745192},
    {RUCKIG_WORKING, 2.1692990031800696},
    {RUCKIG_WORKING, 2.5305866703295874},
    {RUCKIG_WORKING, 2.4550580647027891},
    {RUCKIG_WORKING, 2.0569052768441773},
    {RUCKIG_WORKING, 2.7292669721409863},
    {RUCKIG_WORKING, 3.0123994118392505},
    {RUCKIG_WORKING, 2.8644266241023915},
    {RUCKIG_WORKING, 1.9245663979634555},
    {RUCKIG_WORKING, 2.0413240040177811},
    {RUCKIG_WORKING, 2.1612818916534553},
    {RUCKIG_WORKING, 2.4002506265664163},
    {RUCKIG_WORKING, 1.977557379612221},
    {RUCKIG_WORKING, 2.2563187996268708},
    {RUCKIG_WORKING, 2.4584710349512853},
    {RUCKIG_WORKING, 2.4222272872301414},
    {RUCKIG_WORKING, 2.1047787855405429},
    {RUCKIG_WORKING, 2.4657343587793634},
    {RUCKIG_WORKING, 3.1531469118030202},
    {RUCKIG_WORKING, 2.6061641302819289},
    {RUCKIG_WORKING, 1.9239972254420541},
    {RUCKIG_WORKING, 2.0558384224789195},
    {RUCKIG_WORKING, 2.2248443577610386},
    {RUCKIG_WORKING, 2.3223942208462334},
    {RUCKIG_WORKING, 2.0283780734336379},
    {RUCKIG_WORKING, 2.2574411257868867},
    {RUCKIG_WORKING, 2.4644611096425031},
    {RUCKIG_WORKING, 2.4457278825371329},
    {RUCKIG_WORKING, 2.0848511936006613},
    {RUCKIG_WORKING, 2.6054809401039547},
    {RUCKIG_WORKING, 3.0800045689903248},
    {RUCKIG_WORKING, 2.8832218947413302},
    {RUCKIG_WORKING, 1.9160505589883272},
    {RUCKIG_WORKING, 2.0760672687280288},
    {RUCKIG_WORKING, 2.1888431138892592},
    {RUCKIG_WORKING, 2.2999908098908737},
    {RUCKIG_WORKING, 2.0038814140165444},
    {RUCKIG_WORKING, 2.3860493642534069},
    {RUCKIG_WORKING, 2.557441802528555},
    {RUCKIG_WORKING, 2.23899929243351},
    {RUCKIG_WORKING, 2.0428579478127338},
    {RUCKIG_WORKING, 2.3215913079831605},
    {RUCKIG_WORKING, 3.4106228864090751},
    {RUCKIG_WORKING, 2.8149811574204939},
    {RUCKIG_WORKING, 1.9318541856703626},
    {RUCKIG_WORKING, 2.0522195091848081},
    {RUCKIG_WORKING, 2.0631838839490344},
    {RUCKIG_WORKING, 2.3900689223057645},
    {RUCKIG_WORKING, 2.0494105941215643},
    {RUCKIG_WORKING, 2.5094798441135246},
    {RUCKIG_WORKING, 2.2752288475778109},
    {RUCKIG_WORKING, 2.5289410155537513},
    {RUCKIG_WORKING, 2.0357220222917145},
    {RUCKIG_WORKING, 2.0774397928966808},
    {RUCKIG_WORKING, 3.0343783763481187},
    {RUCKIG_WORKING, 2.7125519658727493},
    {RUCKIG_WORKING, 1.9442812786899282},
    {RUCKIG_WORKING, 2.069495314403671},
    {RUCKIG_WORKING, 2.2236904761904763},
    {RUCKIG_WORKING, 2.3257881283373774},
    {RUCKIG_WORKING, 2.0353605455021526},
    {RUCKIG_WORKING, 2.4109810151649858},
    {RUCKIG_WORKING, 2.4176495882389277},
    {RUCKIG_WORKING, 2.4860267003990359},
    {RUCKIG_WORKING, 2.0510236404242619},
    {RUCKIG_WORKING, 2.6080382297388756},
    {RUCKIG_WORKING, 3.091791134072011},
    {RUCKIG_WORKING, 2.903038931573815},
    {RUCKIG_WORKING, 1.9702790260533687},
    {RUCKIG_WORKING, 2.083353074399418},
    {RUCKIG_WORKING, 2.1970749992119081},
    {RUCKIG_WORKING, 2.4716791979949875},
    {RUCKIG_WORKING, 1.978703111129096},
    {RUCKIG_WORKING, 2.0772403453156212},
    {RUCKIG_WORKING, 2.3591172672337084},
    {RUCKIG_WORKING, 2.3636957636010121},
    {RUCKIG_WORKING, 2.0532460636728844},
    {RUCKIG_WORKING, 2.5913216388654354},
    {RUCKIG_WORKING, 3.3428153457599308},
    {RUCKIG_WORKING, 2.54395314189289},
    {RUCKIG_WORKING, 1.9754432000429079},
    {RUCKIG_WORKING, 2.0936269995027805},
    {RUCKIG_WORKING, 2.2964285714285708},
    {RUCKIG_WORKING, 2.2466345402602599},
    {RUCKIG_WORKING, 1.9142690602950223},
    {RUCKIG_WORKING, 2.177528236611459},
    {RUCKIG_WORKING, 2.3408605072052615},
    {RUCKIG_WORKING, 2.5460348885692179},
    {RUCKIG_WORKING, 2.0512234950866777},
    {RUCKIG_WORKING, 2.7281909321239359},
    {RUCKIG_WORKING, 3.1320961226633219},
    {RUCKIG_WORKING, 2.8955176361553034},
    {RUCKIG_WORKING, 1.9944569522369353},
    {RUCKIG_WORKING, 2.1238095238095238},
    {RUCKIG_WORKING, 2.1104950014176227},
    {RUCKIG_WORKING, 2.2762819997371411},
    {RUCKIG_WORKING, 1.9421733108693535},
    {RUCKIG_WORKING, 2.4233387413110896},
    {RUCKIG_WORKING, 2.2582695351593518},
    {RUCKIG_WORKING, 2.4168334927430806},
    {RUCKIG_WORKING, 2.0789756193223998},
    {RUCKIG_WORKING, 2.5650590147225167},
    {RUCKIG_WORKING, 3.335222010769173},
    {RUCKIG_WORKING, 2.8060429882502538},
    {RUCKIG_WORKING, 1.9847952137456903},
    {RUCKIG_WORKING, 1.9957062475352765},
    {RUCKIG_WORKING, 2.1236601897753999},
    {RUCKIG_WORKING, 2.3097117794486217},
    {RUCKIG_WORKING, 1.998232074574996},
    {RUCKIG_WORKING, 2.2494421209405626},
    {RUCKIG_WORKING, 2.3453201943745858},
    {RUCKIG_WORKING, 2.5133170309898056}
};

static void configure_waypoint_resume_quality_case(
    ruckig_input_t* input,
    size_t case_id,
    double* waypoints_out
) {
    const size_t dofs = input->dofs;
    const size_t waypoint_count = input->max_number_of_waypoints;
    const size_t section_count = waypoint_count + 1;
    double waypoints[12] = {0.0};
    double per_section_min_velocity[16] = {0.0};
    double per_section_max_velocity[16] = {0.0};
    double per_section_min_acceleration[16] = {0.0};
    double per_section_max_acceleration[16] = {0.0};
    double per_section_max_jerk[16] = {0.0};
    double per_section_min_position[16] = {0.0};
    double per_section_max_position[16] = {0.0};
    double per_section_minimum_duration[4] = {0.0};
    size_t waypoint;
    size_t section;
    size_t dof;

    CHECK_EQ_INT(ruckig_input_set_synchronization(
        input,
        (ruckig_synchronization_t)(case_id % 4u)), RUCKIG_WORKING);

    for (dof = 0; dof < dofs; ++dof) {
        const bool disabled = dofs > 1 && ((case_id + dof * 7u) % 19u == 0u);
        const double sign = ((case_id + dof) % 2u) == 0u ? 1.0 : -1.0;
        const double distance = disabled ? 0.0 : sign * (0.85 + 0.18 * (double)dof + 0.015 * (double)(case_id % 11u));
        ruckig_input_current_position_data(input)[dof] = 0.0;
        ruckig_input_current_velocity_data(input)[dof] = 0.0;
        ruckig_input_current_acceleration_data(input)[dof] = 0.0;
        ruckig_input_target_position_data(input)[dof] = distance;
        ruckig_input_target_velocity_data(input)[dof] = 0.0;
        ruckig_input_target_acceleration_data(input)[dof] = 0.0;
        ruckig_input_max_velocity_data(input)[dof] = 1.05 + 0.07 * (double)((case_id + dof) % 5u);
        ruckig_input_max_acceleration_data(input)[dof] = 1.80 + 0.10 * (double)((case_id + 2u * dof) % 4u);
        ruckig_input_max_jerk_data(input)[dof] = 4.20 + 0.20 * (double)((case_id + 3u * dof) % 4u);
        ruckig_input_max_position_data(input)[dof] = distance >= 0.0 ? distance + 0.75 : 0.75;
        ruckig_input_min_position_data(input)[dof] = distance >= 0.0 ? -0.75 : distance - 0.75;
        CHECK_EQ_INT(ruckig_input_set_dof_enabled(input, dof, !disabled), RUCKIG_WORKING);
    }

    for (waypoint = 0; waypoint < waypoint_count; ++waypoint) {
        const double fraction = (double)(waypoint + 1u) / (double)(waypoint_count + 1u);
        for (dof = 0; dof < dofs; ++dof) {
            const bool enabled = ruckig_input_enabled_const_data(input)[dof];
            const double target = ruckig_input_target_position_const_data(input)[dof];
            const double curvature = enabled
                ? 0.025 * (double)(((case_id + waypoint * 3u + dof) % 3u) + 1u)
                    * (target >= 0.0 ? 1.0 : -1.0)
                : 0.0;
            double value = target * fraction + curvature * sin((double)(waypoint + 1u));
            if (fabs(value) > fabs(target) && target != 0.0) {
                value = target * fraction;
            }
            waypoints[waypoint * dofs + dof] = enabled ? value : 0.0;
        }
    }
    CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoints, waypoint_count, dofs), RUCKIG_WORKING);
    if (waypoints_out) {
        memcpy(waypoints_out, waypoints, sizeof(double) * waypoint_count * dofs);
    }

    for (section = 0; section < section_count; ++section) {
        const double start_fraction = (double)section / (double)section_count;
        const double end_fraction = (double)(section + 1u) / (double)section_count;
        per_section_minimum_duration[section] = 0.04 + 0.01 * (double)((case_id + section) % 3u);
        for (dof = 0; dof < dofs; ++dof) {
            const size_t index = section * dofs + dof;
            const double target = ruckig_input_target_position_const_data(input)[dof];
            const double lo = target >= 0.0 ? target * start_fraction - 0.55 : target * end_fraction - 0.55;
            const double hi = target >= 0.0 ? target * end_fraction + 0.55 : target * start_fraction + 0.55;
            const double max_velocity = ruckig_input_max_velocity_const_data(input)[dof];
            const double max_acceleration = ruckig_input_max_acceleration_const_data(input)[dof];
            per_section_min_velocity[index] = -max_velocity;
            per_section_max_velocity[index] = max_velocity;
            per_section_min_acceleration[index] = -max_acceleration;
            per_section_max_acceleration[index] = max_acceleration;
            per_section_max_jerk[index] = ruckig_input_max_jerk_const_data(input)[dof];
            per_section_min_position[index] = lo;
            per_section_max_position[index] = hi;
        }
    }

    if ((case_id % 2u) == 0u) {
        CHECK_EQ_INT(ruckig_input_set_per_section_max_velocity(input, per_section_max_velocity, section_count, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_min_velocity(input, per_section_min_velocity, section_count, dofs), RUCKIG_WORKING);
    }
    if ((case_id % 3u) == 0u) {
        CHECK_EQ_INT(ruckig_input_set_per_section_max_acceleration(input, per_section_max_acceleration, section_count, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_min_acceleration(input, per_section_min_acceleration, section_count, dofs), RUCKIG_WORKING);
    }
    if ((case_id % 5u) == 0u) {
        CHECK_EQ_INT(ruckig_input_set_per_section_max_jerk(input, per_section_max_jerk, section_count, dofs), RUCKIG_WORKING);
    }
    if ((case_id % 7u) == 0u) {
        CHECK_EQ_INT(ruckig_input_set_per_section_min_position(input, per_section_min_position, section_count, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_max_position(input, per_section_max_position, section_count, dofs), RUCKIG_WORKING);
    }
    if ((case_id % 11u) == 0u) {
        CHECK_EQ_INT(ruckig_input_set_per_section_minimum_duration(input, per_section_minimum_duration, section_count), RUCKIG_WORKING);
    }
}

static ruckig_result_t waypoint_resume_quality_calculate_case(
    size_t case_id,
    ruckig_trajectory_t** trajectory_out
) {
    const size_t dofs = 1u + (case_id % 4u);
    const size_t waypoint_count = 1u + ((case_id / 4u) % 3u);
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    ruckig_result_t result;

    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, dofs, 0.01, waypoint_count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, dofs, waypoint_count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, dofs, waypoint_count), RUCKIG_WORKING);
    configure_waypoint_resume_quality_case(input, case_id, NULL);

    result = ruckig_calculate(otg, input, trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
    if (trajectory_out) {
        *trajectory_out = trajectory;
    } else {
        ruckig_trajectory_destroy(trajectory);
    }
    return result;
}

void run_waypoint_resume_quality_baseline_dump(void) {
    size_t case_id;
    printf("static const waypoint_resume_quality_baseline_t waypoint_resume_quality_baseline[WAYPOINT_RESUME_QUALITY_CASES] = {\n");
    for (case_id = 0; case_id < WAYPOINT_RESUME_QUALITY_CASES; ++case_id) {
        ruckig_trajectory_t* trajectory = NULL;
        const ruckig_result_t result = waypoint_resume_quality_calculate_case(case_id, &trajectory);
        const double duration = result == RUCKIG_WORKING ? ruckig_trajectory_get_duration(trajectory) : -1.0;
        printf("    {%s, %.17g}%s\n",
            result == RUCKIG_WORKING ? "RUCKIG_WORKING" : "RUCKIG_ERROR",
            duration,
            case_id + 1u == WAYPOINT_RESUME_QUALITY_CASES ? "" : ",");
        ruckig_trajectory_destroy(trajectory);
    }
    printf("};\n");
}

void run_waypoint_resume_quality_audit_tests(void) {
    double max_regression = 0.0;
    double sum_ratio = 0.0;
    size_t successful_cases = 0;
    size_t publish_count = 0;
    size_t interrupted_without_publish_count = 0;
    size_t completion_count = 0;
    size_t fresh_reference_count = 0;
    size_t case_id;

    for (case_id = 0; case_id < WAYPOINT_RESUME_QUALITY_CASES; ++case_id) {
        ruckig_trajectory_t* trajectory = NULL;
        const ruckig_result_t result = waypoint_resume_quality_calculate_case(case_id, &trajectory);
        const waypoint_resume_quality_baseline_t baseline = waypoint_resume_quality_baseline[case_id];
        CHECK_EQ_INT(result, baseline.result);
        if (result == RUCKIG_WORKING) {
            const double duration = ruckig_trajectory_get_duration(trajectory);
            const double regression = duration - baseline.duration;
            CHECK_TRUE(duration > 0.0);
            CHECK_TRUE(duration <= baseline.duration + 1.0e-9);
            if (regression > max_regression) {
                max_regression = regression;
            }
            sum_ratio += duration / baseline.duration;
            ++successful_cases;
        }
        ruckig_trajectory_destroy(trajectory);
    }

    for (case_id = 0; case_id < WAYPOINT_RESUME_QUALITY_CASES; case_id += 4u) {
        const size_t dofs = 1u + (case_id % 4u);
        const size_t waypoint_count = 1u + ((case_id / 4u) % 3u);
        ruckig_t* otg = NULL;
        ruckig_t* fresh_otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_input_t* fresh_input = NULL;
        ruckig_output_t* output = NULL;
        ruckig_trajectory_t* fresh_trajectory = NULL;
        size_t cycle;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, dofs, 0.01, waypoint_count), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_create_with_waypoints(&fresh_otg, dofs, 0.01, waypoint_count), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, dofs, waypoint_count), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&fresh_input, dofs, waypoint_count), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, dofs, waypoint_count), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&fresh_trajectory, dofs, waypoint_count), RUCKIG_WORKING);
        configure_waypoint_resume_quality_case(input, case_id, NULL);
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
        CHECK_TRUE(otg->waypoint_engine.active);

        for (cycle = 0; cycle < 40; ++cycle) {
            const double previous_time = ruckig_output_get_time(output);
            const double incumbent_remaining_duration =
                ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)) - previous_time;
            const bool was_active = otg->waypoint_engine.active;
            ruckig_output_pass_to_input(output, input);
            if (cycle == 5u) {
                CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1.0), RUCKIG_WORKING);
            } else if (cycle == 6u) {
                CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
            } else if (cycle == 20u) {
                CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1000000000.0), RUCKIG_WORKING);
            }
            if (was_active) {
                CHECK_EQ_INT(ruckig_input_copy_state(input, fresh_input), RUCKIG_WORKING);
                ruckig_input_clear_interrupt_calculation_duration(fresh_input);
                if (ruckig_calculate(fresh_otg, fresh_input, fresh_trajectory) == RUCKIG_WORKING) {
                    CHECK_TRUE(ruckig_trajectory_get_duration(fresh_trajectory) > 0.0);
                    ++fresh_reference_count;
                }
            }
            CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
            CHECK_TRUE(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)) > 0.0);
            if (ruckig_output_new_calculation(output)) {
                CHECK_NEAR(ruckig_output_get_time(output), 0.01, 1e-12);
                CHECK_TRUE(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output))
                    < incumbent_remaining_duration - 1.0e-12);
                ++publish_count;
            } else {
                CHECK_TRUE(ruckig_output_get_time(output) > previous_time);
                if (ruckig_output_was_calculation_interrupted(output)) {
                    ++interrupted_without_publish_count;
                }
            }
            if (was_active && !otg->waypoint_engine.active) {
                ++completion_count;
                break;
            }
        }

        ruckig_trajectory_destroy(fresh_trajectory);
        ruckig_output_destroy(output);
        ruckig_input_destroy(fresh_input);
        ruckig_input_destroy(input);
        ruckig_destroy(fresh_otg);
        ruckig_destroy(otg);
    }

    CHECK_TRUE(successful_cases > 0);
    CHECK_TRUE(publish_count > 0);
    CHECK_TRUE(interrupted_without_publish_count > 0);
    CHECK_TRUE(completion_count > 0);
    CHECK_TRUE(fresh_reference_count > 0);
    printf("waypoint resume quality audit: cases %zu successful %zu avg_ratio %.12g max_regression %.12g publish %zu interrupted_without_publish %zu completion %zu fresh_reference %zu\n",
        (size_t)WAYPOINT_RESUME_QUALITY_CASES,
        successful_cases,
        sum_ratio / (double)successful_cases,
        max_regression,
        publish_count,
        interrupted_without_publish_count,
        completion_count,
        fresh_reference_count);
}

void run_waypoint_resume_quality_tests(void) {
    run_waypoint_resume_quality_audit_tests();
}
