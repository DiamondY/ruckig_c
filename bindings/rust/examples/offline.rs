use ruckig_c_alpha::{InputParameter, Result, Ruckig, Trajectory};

fn main() -> Result<()> {
    let mut otg = Ruckig::new(1, 0.01)?;
    let mut input = InputParameter::new(1)?;
    let mut trajectory = Trajectory::new(1)?;
    input.set_target_position(&[2.0])?;
    input.set_max_velocity(&[2.0])?;
    input.set_max_acceleration(&[1.5])?;
    input.set_max_jerk(&[1.0])?;
    otg.calculate(&input, &mut trajectory)?;
    println!("duration {}", trajectory.duration());
    Ok(())
}
