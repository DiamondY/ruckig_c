use ruckig_c_alpha::{InputParameter, OutputParameter, RuckigResult, TargetState, Tracking};

fn main() -> ruckig_c_alpha::Result<()> {
    let mut tracking = Tracking::new(1, 0.01)?;
    let mut target = TargetState::new(1)?;
    let mut input = InputParameter::new(1)?;
    let mut output = OutputParameter::new(1)?;

    input.set_max_velocity(&[1.0])?;
    input.set_max_acceleration(&[2.0])?;
    input.set_max_jerk(&[5.0])?;

    for step in 0..200 {
        let t = step as f64 * tracking.delta_time();
        target.set_position(&[0.5 * t])?;
        target.set_velocity(&[0.5])?;
        target.set_acceleration(&[0.0])?;
        let result = tracking.update(&target, &input, &mut output)?;
        if result != RuckigResult::Working && result != RuckigResult::Finished {
            return Err(ruckig_c_alpha::Error {
                code: result as i32,
                operation: "tracking example",
            });
        }
        output.pass_to_input(&mut input);
    }

    println!("tracking final position: {:.6}", output.new_position()[0]);
    Ok(())
}
