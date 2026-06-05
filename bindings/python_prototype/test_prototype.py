from __future__ import annotations

import os
import unittest

from ruckig_cffi import (
    Input,
    Output,
    Result,
    Ruckig,
    RuckigLifecycleError,
    Trajectory,
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

    def test_lifecycle_double_close_and_after_close(self) -> None:
        input_ = Input(1)
        input_.close()
        input_.close()
        with self.assertRaises(RuckigLifecycleError):
            input_.set_current_position([0.0])

    def test_constructor_cleanup_context_manager(self) -> None:
        with Ruckig(1, 0.1) as otg:
            self.assertEqual(otg.dofs, 1)
        with self.assertRaises(RuckigLifecycleError):
            otg.reset()


if __name__ == "__main__":
    unittest.main()
