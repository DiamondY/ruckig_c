use ruckig_c_alpha::{InputParameter, OutputParameter, Result, Ruckig, RuckigResult};

fn main() -> Result<()> {
    let mut otg = Ruckig::new(1, 0.1)?;
    let mut input = InputParameter::new(1)?;
    let mut output = OutputParameter::new(1)?;
    input.set_target_position(&[1.0])?;
    input.set_max_velocity(&[1.0])?;

    loop {
        let result = otg.update(&input, &mut output)?;
        if result == RuckigResult::Finished {
            break;
        }
        output.pass_to_input(&mut input);
    }
    Ok(())
}
