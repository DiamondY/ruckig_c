use ruckig_c_alpha::{
    InputParameter, OutputParameter, RuckigResult, TargetState, TargetStateSequence, Tracking,
    TrackingMode, TrackingOptimizedStrategy,
};

fn main() -> ruckig_c_alpha::Result<()> {
    let mut tracking = Tracking::new(1, 0.01)?;
    let mut target = TargetState::new(1)?;
    let mut targets = TargetStateSequence::new(1, 4)?;
    let mut input = InputParameter::new(1)?;
    let mut output = OutputParameter::new(1)?;

    input.set_max_velocity(&[1.0])?;
    input.set_max_acceleration(&[2.0])?;
    input.set_max_jerk(&[5.0])?;
    input.set_interrupt_calculation_duration(0.0)?;
    tracking.set_mode(TrackingMode::Optimized)?;
    tracking.set_optimized_strategy(TrackingOptimizedStrategy::Aggressive)?;
    tracking.set_look_ahead_cycles(4)?;
    target.set_position(&[0.0])?;
    target.set_velocity(&[0.5])?;
    target.set_acceleration(&[0.0])?;

    let result = tracking.update(&target, &input, &mut output)?;
    if result != RuckigResult::Working && result != RuckigResult::Finished {
        return Err(ruckig_c_alpha::Error {
            code: result as i32,
            operation: "tracking interrupt update",
        });
    }

    output.pass_to_input(&mut input);
    targets.set_count(4)?;
    for step in 0..4 {
        let t = (step + 1) as f64 * tracking.delta_time();
        targets.set_state(step, &[0.5 * t], &[0.5], &[0.0])?;
    }
    let result = tracking.update_with_lookahead(&targets, &input, &mut output)?;
    if result != RuckigResult::Working && result != RuckigResult::Finished {
        return Err(ruckig_c_alpha::Error {
            code: result as i32,
            operation: "tracking interrupt lookahead update",
        });
    }

    let diagnostics = tracking.last_diagnostics()?;
    println!(
        "tracking interrupt: interrupted={} candidates={} budget_exhausted={}",
        output.was_calculation_interrupted(),
        diagnostics.candidate_count,
        diagnostics.budget_exhausted_count
    );
    Ok(())
}
