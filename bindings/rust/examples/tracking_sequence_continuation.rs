use ruckig_c_alpha::{
    InputParameter, RuckigResult, TargetStateSequence, Tracking, TrackingMode,
    TrackingOptimizedStrategy, TrackingOutputSequence, TrackingSequenceContinuation,
};

fn main() -> ruckig_c_alpha::Result<()> {
    let count = 3;
    let mut tracking = Tracking::new(1, 0.01)?;
    let mut input = InputParameter::new(1)?;
    let mut targets = TargetStateSequence::new(1, count)?;
    let mut outputs = TrackingOutputSequence::new(1, count)?;
    let mut continuation = TrackingSequenceContinuation::new(1, count)?;

    input.set_current_position(&[0.0])?;
    input.set_current_velocity(&[0.0])?;
    input.set_current_acceleration(&[0.0])?;
    input.set_target_position(&[0.0])?;
    input.set_target_velocity(&[0.0])?;
    input.set_target_acceleration(&[0.0])?;
    input.set_max_velocity(&[1.0])?;
    input.set_max_acceleration(&[2.0])?;
    input.set_max_jerk(&[5.0])?;
    input.set_interrupt_calculation_duration(0.0)?;

    tracking.set_mode(TrackingMode::Optimized)?;
    tracking.set_optimized_strategy(TrackingOptimizedStrategy::Aggressive)?;
    tracking.set_look_ahead_cycles(count)?;
    tracking.set_max_optimized_candidates(8)?;

    targets.set_count(count)?;
    for step in 0..count {
        let t = step as f64 * tracking.delta_time();
        targets.set_state(
            step,
            &[0.2 * (0.45 * t).sin()],
            &[0.09 * (0.45 * t).cos()],
            &[-0.0405 * (0.45 * t).sin()],
        )?;
    }

    assert_eq!(
        tracking.calculate_sequence_interruptible(&targets, &input, &mut outputs, &mut continuation)?,
        RuckigResult::Working
    );
    for _ in 0..128 {
        if continuation.is_complete() {
            break;
        }
        assert_eq!(
            tracking.resume_sequence(&mut continuation, &mut outputs)?,
            RuckigResult::Working
        );
    }
    assert!(continuation.is_complete());
    assert_eq!(outputs.count(), count);
    println!(
        "completed {}/{} interruptible sequence steps",
        continuation.completed_count(),
        continuation.target_count()
    );
    Ok(())
}
