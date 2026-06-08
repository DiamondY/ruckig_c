use ruckig_c_alpha::{
    InputParameter, OutputParameter, RuckigResult, TargetStateSequence, Tracking, TrackingMode,
    TrackingOptimizedStrategy,
};

fn main() -> ruckig_c_alpha::Result<()> {
    let lookahead = 4;
    let mut tracking = Tracking::new(1, 0.01)?;
    let mut targets = TargetStateSequence::new(1, lookahead)?;
    let mut input = InputParameter::new(1)?;
    let mut output = OutputParameter::new(1)?;

    input.set_max_velocity(&[1.0])?;
    input.set_max_acceleration(&[2.0])?;
    input.set_max_jerk(&[5.0])?;
    tracking.set_mode(TrackingMode::Optimized)?;
    tracking.set_optimized_strategy(TrackingOptimizedStrategy::Aggressive)?;
    tracking.set_look_ahead_cycles(lookahead)?;
    targets.set_count(lookahead)?;

    for step in 0..200 {
        for sample in 0..lookahead {
            let t = (step + sample) as f64 * tracking.delta_time();
            targets.set_state(sample, &[0.5 * t], &[0.5], &[0.0])?;
        }
        let result = tracking.update_with_lookahead(&targets, &input, &mut output)?;
        if result != RuckigResult::Working && result != RuckigResult::Finished {
            return Err(ruckig_c_alpha::Error {
                code: result as i32,
                operation: "tracking optimized example",
            });
        }
        output.pass_to_input(&mut input);
    }

    let diagnostics = tracking.last_diagnostics()?;
    println!(
        "optimized tracking final position: {:.6}, status: {:?}, candidates: {}, improvement: {:.6}",
        output.new_position()[0],
        diagnostics.calculation_status,
        diagnostics.candidate_count,
        diagnostics.improvement_ratio
    );
    Ok(())
}
