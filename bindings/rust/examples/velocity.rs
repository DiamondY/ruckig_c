use ruckig_c_alpha::{ControlInterface, InputParameter, Result, Ruckig, Trajectory};

fn main() -> Result<()> {
    let mut otg = Ruckig::new(1, 0.01)?;
    let mut input = InputParameter::new(1)?;
    let mut trajectory = Trajectory::new(1)?;
    input.set_control_interface(ControlInterface::Velocity)?;
    input.set_target_velocity(&[0.8])?;
    input.set_max_acceleration(&[1.2])?;
    input.set_max_jerk(&[2.0])?;
    otg.calculate(&input, &mut trajectory)?;
    Ok(())
}
