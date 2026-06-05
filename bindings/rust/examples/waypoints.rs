use ruckig_c_alpha::{InputParameter, Result, Ruckig, Trajectory};

fn main() -> Result<()> {
    let mut otg = Ruckig::with_waypoints(1, 0.05, 1)?;
    let mut input = InputParameter::with_waypoints(1, 1)?;
    let mut trajectory = Trajectory::with_waypoints(1, 1)?;
    input.set_target_position(&[2.0])?;
    input.set_max_velocity(&[1.2])?;
    input.set_max_acceleration(&[2.0])?;
    input.set_max_jerk(&[4.0])?;
    input.set_intermediate_positions_flat(&[1.0], 1)?;
    otg.calculate(&input, &mut trajectory)?;
    println!("sections {}", trajectory.section_count());
    Ok(())
}
