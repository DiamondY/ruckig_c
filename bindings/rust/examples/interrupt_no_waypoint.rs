use ruckig_c_alpha::{InputParameter, OutputParameter, Ruckig, RuckigResult};

fn main() -> ruckig_c_alpha::Result<()> {
    let mut otg = Ruckig::new(1, 0.05)?;
    let mut input = InputParameter::new(1)?;
    let mut output = OutputParameter::new(1)?;

    input.set_target_position(&[1.0])?;
    input.set_max_velocity(&[1.0])?;
    input.set_max_acceleration(&[2.0])?;
    input.set_max_jerk(&[5.0])?;
    input.set_interrupt_calculation_duration(1_000_000_000.0)?;

    let result = otg.update(&input, &mut output)?;
    if result != RuckigResult::Working && result != RuckigResult::Finished {
        return Err(ruckig_c_alpha::Error {
            code: result as i32,
            operation: "no-waypoint interrupt initial update",
        });
    }

    output.pass_to_input(&mut input);
    input.set_target_position(&[1.8])?;
    input.set_interrupt_calculation_duration(0.0)?;
    let result = otg.update(&input, &mut output)?;
    if result != RuckigResult::Working && result != RuckigResult::Finished {
        return Err(ruckig_c_alpha::Error {
            code: result as i32,
            operation: "no-waypoint interrupt boundary update",
        });
    }

    println!(
        "no-waypoint interrupt: interrupted={} new_calculation={} position={:.6}",
        output.was_calculation_interrupted(),
        output.new_calculation(),
        output.new_position()[0]
    );
    Ok(())
}
