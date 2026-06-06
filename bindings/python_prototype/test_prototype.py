from __future__ import annotations

import os
import unittest

from ruckig_cffi import (
    Bound,
    ControlInterface,
    DurationDiscretization,
    Input,
    Output,
    Result,
    Ruckig,
    RuckigInvalidInputError,
    RuckigLifecycleError,
    RuckigUnsupportedError,
    Synchronization,
    TargetState,
    TargetStateSequence,
    Trajectory,
    Tracking,
    TrackingMode,
    TrackingOutputSequence,
    configure_library,
)


def configure_input(input_: Input) -> None:
    input_.set_current_position([0.0])
    input_.set_current_velocity([0.0])
    input_.set_current_acceleration([0.0])
    input_.set_target_position([1.0])
    input_.set_target_velocity([0.0])
    input_.set_target_acceleration([0.0])
    input_.set_max_velocity([1.0])
    input_.set_max_acceleration([1.0])
    input_.set_max_jerk([1.0])


def configure_waypoint_input(input_: Input) -> None:
    input_.set_current_position([0.0])
    input_.set_current_velocity([0.0])
    input_.set_current_acceleration([0.0])
    input_.set_target_position([2.0])
    input_.set_target_velocity([0.0])
    input_.set_target_acceleration([0.0])
    input_.set_max_velocity([1.2])
    input_.set_max_acceleration([2.0])
    input_.set_max_jerk([4.0])
    input_.set_max_position([3.0])
    input_.set_min_position([-1.0])
    input_.set_intermediate_positions([1.0])
    input_.set_per_section_max_velocity([1.2, 1.0])
    input_.set_per_section_min_velocity([-1.2, -1.0])
    input_.set_per_section_max_acceleration([2.0, 2.0])
    input_.set_per_section_min_acceleration([-2.0, -2.0])
    input_.set_per_section_max_jerk([4.0, 4.0])
    input_.set_per_section_max_position([1.5, 2.5])
    input_.set_per_section_min_position([-0.5, 0.5])
    input_.set_per_section_minimum_duration([0.0, 0.0])


def configure_tracking_input(input_: Input) -> None:
    input_.set_current_position([0.0])
    input_.set_current_velocity([0.0])
    input_.set_current_acceleration([0.0])
    input_.set_target_position([0.0])
    input_.set_target_velocity([0.0])
    input_.set_target_acceleration([0.0])
    input_.set_max_velocity([1.0])
    input_.set_max_acceleration([2.0])
    input_.set_max_jerk([5.0])


@unittest.skipUnless(
    os.environ.get("RUCKIG_C_SHARED_LIBRARY"),
    "set RUCKIG_C_SHARED_LIBRARY to a built ruckig_c shared library",
)
class PrototypeTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        configure_library(os.environ["RUCKIG_C_SHARED_LIBRARY"])

    def test_offline_calculate(self) -> None:
        with Ruckig(1, 0.1) as otg, Input(1) as input_, Trajectory(1) as trajectory:
            configure_input(input_)
            result = otg.calculate(input_, trajectory)
            self.assertEqual(result, Result.WORKING)
            self.assertGreater(trajectory.duration, 0.0)

            final_state = trajectory.at_time(trajectory.duration)
            self.assertAlmostEqual(final_state["position"][0], 1.0, places=8)
            self.assertAlmostEqual(final_state["velocity"][0], 0.0, places=8)

    def test_online_update_loop(self) -> None:
        with Ruckig(1, 0.1) as otg, Input(1) as input_, Output(1) as output:
            configure_input(input_)
            result = Result.WORKING
            for _ in range(200):
                result = otg.update(input_, output)
                if result == Result.FINISHED:
                    break
                output.pass_to_input(input_)

            self.assertEqual(result, Result.FINISHED)
            self.assertAlmostEqual(output.new_position()[0], 1.0, places=8)
            self.assertAlmostEqual(output.new_velocity()[0], 0.0, places=8)

    def test_waypoint_offline_calculate(self) -> None:
        with (
            Ruckig(1, 0.05, max_number_of_waypoints=2) as otg,
            Input(1, max_number_of_waypoints=2) as input_,
            Trajectory(1, max_number_of_waypoints=2) as trajectory,
        ):
            configure_waypoint_input(input_)
            self.assertEqual(otg.max_number_of_waypoints, 2)
            self.assertEqual(input_.intermediate_position_count, 1)
            self.assertEqual(input_.intermediate_positions(), [[1.0]])
            self.assertEqual(input_.per_section_max_velocity(), [[1.2], [1.0]])
            self.assertEqual(input_.per_section_minimum_duration(), [0.0, 0.0])

            result = otg.calculate(input_, trajectory)
            self.assertEqual(result, Result.WORKING)
            self.assertEqual(trajectory.section_count, 2)
            intermediate_durations = trajectory.intermediate_durations()
            self.assertEqual(len(intermediate_durations), 1)
            self.assertGreater(intermediate_durations[0], 0.0)
            self.assertLess(intermediate_durations[0], trajectory.duration)

            waypoint_state = trajectory.at_time(intermediate_durations[0])
            self.assertAlmostEqual(waypoint_state["position"][0], 1.0, places=7)
            self.assertEqual(waypoint_state["section"], 1)

            extrema = trajectory.position_extrema()
            self.assertIsInstance(extrema[0], Bound)
            self.assertLessEqual(extrema[0].max, 2.0 + 1e-7)
            self.assertGreaterEqual(extrema[0].min, -1e-7)
            self.assertAlmostEqual(
                trajectory.first_time_at_position(0, 1.0),
                intermediate_durations[0],
                places=7,
            )

    def test_waypoint_multi_dof_per_section_position_bounds(self) -> None:
        with (
            Ruckig(2, 0.02, max_number_of_waypoints=2) as otg,
            Input(2, max_number_of_waypoints=2) as input_,
            Trajectory(2, max_number_of_waypoints=2) as trajectory,
        ):
            input_.set_target_position([1.5, -0.6])
            input_.set_max_velocity([1.5, 1.4])
            input_.set_max_acceleration([2.0, 1.8])
            input_.set_max_jerk([4.0, 3.5])
            input_.set_intermediate_positions([[0.5, -0.2], [1.0, -0.4]])
            input_.set_per_section_minimum_duration([0.20, 0.30, 0.20])
            input_.set_per_section_max_position([
                [0.6, 0.0],
                [1.1, -0.1],
                [1.6, -0.3],
            ])
            input_.set_per_section_min_position([
                [-0.1, -0.3],
                [0.4, -0.5],
                [0.9, -0.8],
            ])

            self.assertEqual(input_.intermediate_positions(), [[0.5, -0.2], [1.0, -0.4]])
            self.assertEqual(otg.calculate(input_, trajectory), Result.WORKING)
            self.assertEqual(trajectory.section_count, 3)
            durations = trajectory.intermediate_durations()
            self.assertEqual(len(durations), 2)
            self.assertLess(durations[0], durations[1])
            second_waypoint = trajectory.at_time(durations[1])
            self.assertAlmostEqual(second_waypoint["position"][0], 1.0, places=7)
            self.assertAlmostEqual(second_waypoint["position"][1], -0.4, places=7)

            extrema = trajectory.position_extrema()
            self.assertLessEqual(extrema[0].max, 1.5 + 1e-7)
            self.assertGreaterEqual(extrema[1].min, -0.6 - 1e-7)

    def test_waypoint_four_dof_mixed_per_section_constraints(self) -> None:
        with (
            Ruckig(4, 0.01, max_number_of_waypoints=2) as otg,
            Input(4, max_number_of_waypoints=2) as input_,
            Trajectory(4, max_number_of_waypoints=2) as trajectory,
        ):
            input_.set_target_position([1.15, -0.62, 0.70, -0.48])
            input_.set_max_velocity([1.1, 1.1, 1.1, 1.1])
            input_.set_max_acceleration([1.8, 1.8, 1.8, 1.8])
            input_.set_max_jerk([4.0, 4.0, 4.0, 4.0])
            input_.set_intermediate_positions([
                [0.30, -0.15, 0.22, -0.10],
                [0.82, -0.42, 0.46, -0.30],
            ])
            input_.set_per_section_min_velocity([
                [-0.65, -0.75, -0.70, -0.60],
                [-0.80, -0.85, -0.78, -0.70],
                [-0.90, -0.95, -0.85, -0.80],
            ])
            input_.set_per_section_max_velocity([
                [0.70, 0.75, 0.70, 0.65],
                [0.85, 0.90, 0.82, 0.75],
                [1.00, 1.05, 0.92, 0.88],
            ])
            input_.set_per_section_max_jerk([
                [3.0, 3.0, 2.8, 2.6],
                [3.4, 3.4, 3.2, 3.0],
                [3.8, 3.8, 3.5, 3.3],
            ])
            input_.set_per_section_min_position([
                [-0.05, -0.20, -0.05, -0.15],
                [0.25, -0.50, 0.15, -0.35],
                [0.75, -0.75, 0.38, -0.55],
            ])
            input_.set_per_section_max_position([
                [0.35, 0.05, 0.25, 0.05],
                [0.88, -0.10, 0.50, -0.05],
                [1.25, -0.38, 0.78, -0.22],
            ])
            input_.set_per_section_minimum_duration([0.40, 0.70, 0.50])

            self.assertEqual(otg.calculate(input_, trajectory), Result.WORKING)
            self.assertEqual(trajectory.section_count, 3)
            durations = trajectory.intermediate_durations()
            self.assertEqual(len(durations), 2)
            self.assertLess(durations[0], durations[1])

            first_waypoint = trajectory.at_time(durations[0])
            second_waypoint = trajectory.at_time(durations[1])
            self.assertEqual(first_waypoint["section"], 1)
            self.assertEqual(second_waypoint["section"], 2)
            self.assertAlmostEqual(first_waypoint["position"][0], 0.30, places=7)
            self.assertAlmostEqual(second_waypoint["position"][1], -0.42, places=7)

            starts = [0.0, durations[0], durations[1]]
            ends = [durations[0], durations[1], trajectory.duration]
            for section, (start, end) in enumerate(zip(starts, ends)):
                sample = trajectory.at_time(start + (end - start) * 0.5)
                self.assertEqual(sample["section"], section)

            extrema = trajectory.position_extrema()
            self.assertLessEqual(extrema[0].max, 1.25 + 1e-7)
            self.assertGreaterEqual(extrema[1].min, -0.75 - 1e-7)
            self.assertAlmostEqual(
                trajectory.first_time_at_position(0, 0.82, durations[0] + 1e-9),
                durations[1],
                places=7,
            )

    def test_waypoint_clear_and_interrupt_surface(self) -> None:
        with (
            Ruckig(1, 0.05, max_number_of_waypoints=1) as otg,
            Input(1, max_number_of_waypoints=1) as input_,
            Output(1, max_number_of_waypoints=1) as output,
        ):
            configure_waypoint_input(input_)
            input_.set_control_interface(ControlInterface.POSITION)
            input_.set_synchronization(Synchronization.TIME)
            input_.set_duration_discretization(DurationDiscretization.CONTINUOUS)
            input_.clear_min_velocity()
            input_.clear_min_acceleration()
            input_.clear_minimum_duration()
            input_.clear_per_section_max_velocity()
            input_.clear_per_section_min_velocity()
            input_.clear_per_section_max_acceleration()
            input_.clear_per_section_min_acceleration()
            input_.clear_per_section_max_jerk()
            input_.clear_per_section_max_position()
            input_.clear_per_section_min_position()
            input_.clear_per_section_minimum_duration()
            input_.set_interrupt_calculation_duration(0.001)
            input_.clear_interrupt_calculation_duration()

            self.assertEqual(otg.update(input_, output), Result.WORKING)
            self.assertTrue(output.new_calculation)
            self.assertFalse(output.was_calculation_interrupted)
            self.assertEqual(len(output.new_acceleration()), 1)

    def test_waypoint_online_update_loop(self) -> None:
        with (
            Ruckig(1, 0.05, max_number_of_waypoints=2) as otg,
            Input(1, max_number_of_waypoints=2) as input_,
            Output(1, max_number_of_waypoints=2) as output,
        ):
            configure_waypoint_input(input_)
            result = Result.WORKING
            for _ in range(300):
                result = otg.update(input_, output)
                if result == Result.FINISHED:
                    break
                output.pass_to_input(input_)

            self.assertEqual(result, Result.FINISHED)
            self.assertAlmostEqual(output.new_position()[0], 2.0, places=7)
            self.assertGreaterEqual(output.new_section, 1)

    def test_filter_intermediate_positions(self) -> None:
        with (
            Ruckig(1, 0.05, max_number_of_waypoints=3) as otg,
            Input(1, max_number_of_waypoints=3) as input_,
        ):
            configure_input(input_)
            input_.set_target_position([4.0])
            input_.set_intermediate_positions([1.0, 2.0, 3.0])
            self.assertEqual(otg.filter_intermediate_positions(input_, [0.25]), [])

    def test_tracking_online_fast_loop(self) -> None:
        with Tracking(1, 0.01) as tracking, TargetState(1) as target, Input(1) as input_, Output(1) as output:
            configure_tracking_input(input_)
            self.assertEqual(tracking.mode, TrackingMode.FAST)
            self.assertEqual(tracking.look_ahead_cycles, 1)
            self.assertAlmostEqual(tracking.reactiveness, 1.0)
            tracking.set_reactiveness(1.0)
            tracking.set_look_ahead_cycles(1)

            for step in range(200):
                t = step * tracking.delta_time
                target.set_position([0.5 * t])
                target.set_velocity([0.5])
                target.set_acceleration([0.0])
                self.assertEqual(tracking.update(target, input_, output), Result.WORKING)
                output.pass_to_input(input_)

            self.assertGreater(output.new_position()[0], 0.0)
            self.assertLess(output.new_position()[0], 1.0)

    def test_tracking_offline_sequence(self) -> None:
        count = 200
        with (
            Tracking(1, 0.01) as tracking,
            TargetStateSequence(1, count) as targets,
            TrackingOutputSequence(1, count) as outputs,
            Input(1) as input_,
        ):
            configure_tracking_input(input_)
            targets.set_count(count)
            for step in range(count):
                t = step * tracking.delta_time
                targets.set_state(step, [0.5 * t], [0.5], [0.0])

            self.assertEqual(tracking.calculate_sequence(targets, input_, outputs), Result.WORKING)
            self.assertEqual(outputs.count, count)
            self.assertEqual(len(outputs.new_positions()), count)
            self.assertEqual(len(outputs.times()), count)
            self.assertTrue(all(result in (Result.WORKING, Result.FINISHED) for result in outputs.results()))
            self.assertGreater(outputs.new_positions()[-1][0], outputs.new_positions()[0][0])

    def test_tracking_optimized_is_unsupported_in_alpha(self) -> None:
        with Tracking(1, 0.01) as tracking, TargetState(1) as target, Input(1) as input_, Output(1) as output:
            configure_tracking_input(input_)
            target.set_position([0.0])
            target.set_velocity([0.5])
            target.set_acceleration([0.0])
            tracking.set_mode(TrackingMode.OPTIMIZED)
            with self.assertRaises(RuckigUnsupportedError):
                tracking.update(target, input_, output)

    def test_tuple_copy_in_and_list_copy_out(self) -> None:
        with Ruckig(1, 0.1) as otg, Input(1) as input_, Trajectory(1) as trajectory:
            input_.set_current_position((0.0,))
            input_.set_current_velocity((0.0,))
            input_.set_current_acceleration((0.0,))
            input_.set_target_position((1.0,))
            input_.set_target_velocity((0.0,))
            input_.set_target_acceleration((0.0,))
            input_.set_max_velocity((1.0,))
            input_.set_max_acceleration((1.0,))
            input_.set_max_jerk((1.0,))

            self.assertEqual(otg.calculate(input_, trajectory), Result.WORKING)
            sample = trajectory.at_time(0.0)
            self.assertIsInstance(sample["position"], list)
            self.assertEqual(len(sample["position"]), 1)

    def test_vector_length_mismatch_fails_before_c_call(self) -> None:
        with Input(1) as input_:
            with self.assertRaisesRegex(ValueError, "expected 1 values, got 2"):
                input_.set_current_position([0.0, 1.0])

    def test_lifecycle_double_close_and_after_close(self) -> None:
        input_ = Input(1)
        input_.close()
        input_.close()
        with self.assertRaises(RuckigLifecycleError):
            input_.set_current_position([0.0])

        output = Output(1)
        output.close()
        output.close()
        with self.assertRaises(RuckigLifecycleError):
            output.new_position()

        trajectory = Trajectory(1)
        trajectory.close()
        trajectory.close()
        with self.assertRaises(RuckigLifecycleError):
            _ = trajectory.duration

        tracking = Tracking(1, 0.01)
        tracking.close()
        tracking.close()
        with self.assertRaises(RuckigLifecycleError):
            _ = tracking.delta_time

        target = TargetState(1)
        target.close()
        target.close()
        with self.assertRaises(RuckigLifecycleError):
            target.set_position([0.0])

        targets = TargetStateSequence(1, 2)
        targets.close()
        targets.close()
        with self.assertRaises(RuckigLifecycleError):
            targets.set_count(1)

        outputs = TrackingOutputSequence(1, 2)
        outputs.close()
        outputs.close()
        with self.assertRaises(RuckigLifecycleError):
            _ = outputs.count

    def test_constructor_cleanup_context_manager(self) -> None:
        with Ruckig(1, 0.1) as otg:
            self.assertEqual(otg.dofs, 1)
        with self.assertRaises(RuckigLifecycleError):
            otg.reset()

    def test_invalid_constructor_maps_typed_exception(self) -> None:
        with self.assertRaises(RuckigInvalidInputError) as cm:
            Ruckig(0, 0.1)
        self.assertEqual(cm.exception.result, int(Result.ERROR_INVALID_INPUT))
        self.assertEqual(cm.exception.operation, "ruckig_create")

    def test_invalid_calculate_maps_typed_exception(self) -> None:
        with Ruckig(1, 0.1) as otg, Input(2) as input_, Trajectory(2) as trajectory:
            with self.assertRaises(RuckigInvalidInputError) as cm:
                otg.calculate(input_, trajectory)
        self.assertEqual(cm.exception.result, int(Result.ERROR_INVALID_INPUT))
        self.assertEqual(cm.exception.operation, "ruckig_calculate")


if __name__ == "__main__":
    unittest.main()
