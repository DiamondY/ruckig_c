use std::fmt;
use std::marker::PhantomData;
use std::ptr::NonNull;
use std::slice;

#[repr(C)]
struct RuckigRaw {
    _private: [u8; 0],
}

#[repr(C)]
struct InputRaw {
    _private: [u8; 0],
}

#[repr(C)]
struct OutputRaw {
    _private: [u8; 0],
}

#[repr(C)]
struct TrajectoryRaw {
    _private: [u8; 0],
}

#[repr(C)]
struct TrackingRaw {
    _private: [u8; 0],
}

#[repr(C)]
struct TargetStateRaw {
    _private: [u8; 0],
}

#[repr(C)]
struct TargetStateSequenceRaw {
    _private: [u8; 0],
}

#[repr(C)]
struct TrackingOutputSequenceRaw {
    _private: [u8; 0],
}

#[repr(C)]
#[derive(Clone, Copy)]
struct CBound {
    min_position: f64,
    max_position: f64,
    time_min: f64,
    time_max: f64,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
#[repr(i32)]
pub enum RuckigResult {
    Working = 0,
    Finished = 1,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
#[repr(i32)]
pub enum ControlInterface {
    Position = 0,
    Velocity = 1,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
#[repr(i32)]
pub enum Synchronization {
    Time = 0,
    TimeIfNecessary = 1,
    Phase = 2,
    None = 3,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
#[repr(i32)]
pub enum DurationDiscretization {
    Continuous = 0,
    Discrete = 1,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
#[repr(i32)]
pub enum TrackingMode {
    Fast = 0,
    Optimized = 1,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
#[repr(i32)]
pub enum TrackingCalculationStatus {
    None = 0,
    Fast = 1,
    Optimized = 2,
    FastFallback = 3,
    Error = 4,
}

#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Bound {
    pub min: f64,
    pub max: f64,
    pub t_min: f64,
    pub t_max: f64,
}

#[derive(Clone, Debug, PartialEq)]
pub struct State {
    pub position: Vec<f64>,
    pub velocity: Vec<f64>,
    pub acceleration: Vec<f64>,
    pub jerk: Vec<f64>,
    pub section: usize,
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub struct Error {
    pub code: i32,
    pub operation: &'static str,
}

impl fmt::Display for Error {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "{} failed with ruckig result {}",
            self.operation, self.code
        )
    }
}

impl std::error::Error for Error {}

pub type Result<T> = std::result::Result<T, Error>;

unsafe extern "C" {
    fn ruckig_create(otg: *mut *mut RuckigRaw, dofs: usize, delta_time: f64) -> i32;
    fn ruckig_create_with_waypoints(
        otg: *mut *mut RuckigRaw,
        dofs: usize,
        delta_time: f64,
        max_number_of_waypoints: usize,
    ) -> i32;
    fn ruckig_destroy(otg: *mut RuckigRaw);
    fn ruckig_get_max_number_of_waypoints(otg: *const RuckigRaw) -> usize;
    fn ruckig_calculate(
        otg: *mut RuckigRaw,
        input: *const InputRaw,
        trajectory: *mut TrajectoryRaw,
    ) -> i32;
    fn ruckig_update(otg: *mut RuckigRaw, input: *const InputRaw, output: *mut OutputRaw) -> i32;
    fn ruckig_filter_intermediate_positions(
        otg: *const RuckigRaw,
        input: *const InputRaw,
        threshold_distance: *const f64,
        threshold_count: usize,
        filtered_positions: *mut f64,
        capacity: usize,
        written_waypoints: *mut usize,
    ) -> i32;

    fn ruckig_input_create(input: *mut *mut InputRaw, dofs: usize) -> i32;
    fn ruckig_input_create_with_waypoints(
        input: *mut *mut InputRaw,
        dofs: usize,
        max_number_of_waypoints: usize,
    ) -> i32;
    fn ruckig_input_destroy(input: *mut InputRaw);
    fn ruckig_input_current_position_data(input: *mut InputRaw) -> *mut f64;
    fn ruckig_input_current_velocity_data(input: *mut InputRaw) -> *mut f64;
    fn ruckig_input_current_acceleration_data(input: *mut InputRaw) -> *mut f64;
    fn ruckig_input_target_position_data(input: *mut InputRaw) -> *mut f64;
    fn ruckig_input_target_velocity_data(input: *mut InputRaw) -> *mut f64;
    fn ruckig_input_target_acceleration_data(input: *mut InputRaw) -> *mut f64;
    fn ruckig_input_max_velocity_data(input: *mut InputRaw) -> *mut f64;
    fn ruckig_input_max_acceleration_data(input: *mut InputRaw) -> *mut f64;
    fn ruckig_input_max_jerk_data(input: *mut InputRaw) -> *mut f64;
    fn ruckig_input_max_position_data(input: *mut InputRaw) -> *mut f64;
    fn ruckig_input_min_position_data(input: *mut InputRaw) -> *mut f64;
    fn ruckig_input_set_control_interface(input: *mut InputRaw, control_interface: i32) -> i32;
    fn ruckig_input_set_synchronization(input: *mut InputRaw, synchronization: i32) -> i32;
    fn ruckig_input_set_duration_discretization(input: *mut InputRaw, discretization: i32) -> i32;
    fn ruckig_input_set_min_velocity(input: *mut InputRaw, values: *const f64, count: usize)
        -> i32;
    fn ruckig_input_clear_min_velocity(input: *mut InputRaw);
    fn ruckig_input_set_min_acceleration(
        input: *mut InputRaw,
        values: *const f64,
        count: usize,
    ) -> i32;
    fn ruckig_input_clear_min_acceleration(input: *mut InputRaw);
    fn ruckig_input_set_minimum_duration(input: *mut InputRaw, duration: f64) -> i32;
    fn ruckig_input_clear_minimum_duration(input: *mut InputRaw);
    fn ruckig_input_set_intermediate_positions(
        input: *mut InputRaw,
        flat_positions: *const f64,
        waypoint_count: usize,
        dofs: usize,
    ) -> i32;
    fn ruckig_input_clear_intermediate_positions(input: *mut InputRaw);
    fn ruckig_input_get_intermediate_position_count(input: *const InputRaw) -> usize;
    fn ruckig_input_get_intermediate_positions(
        input: *const InputRaw,
        flat_positions: *mut f64,
        capacity: usize,
    ) -> i32;
    fn ruckig_input_set_per_section_max_velocity(
        input: *mut InputRaw,
        values: *const f64,
        section_count: usize,
        dofs: usize,
    ) -> i32;
    fn ruckig_input_clear_per_section_max_velocity(input: *mut InputRaw);
    fn ruckig_input_set_per_section_min_velocity(
        input: *mut InputRaw,
        values: *const f64,
        section_count: usize,
        dofs: usize,
    ) -> i32;
    fn ruckig_input_clear_per_section_min_velocity(input: *mut InputRaw);
    fn ruckig_input_set_per_section_max_acceleration(
        input: *mut InputRaw,
        values: *const f64,
        section_count: usize,
        dofs: usize,
    ) -> i32;
    fn ruckig_input_clear_per_section_max_acceleration(input: *mut InputRaw);
    fn ruckig_input_set_per_section_min_acceleration(
        input: *mut InputRaw,
        values: *const f64,
        section_count: usize,
        dofs: usize,
    ) -> i32;
    fn ruckig_input_clear_per_section_min_acceleration(input: *mut InputRaw);
    fn ruckig_input_set_per_section_max_jerk(
        input: *mut InputRaw,
        values: *const f64,
        section_count: usize,
        dofs: usize,
    ) -> i32;
    fn ruckig_input_clear_per_section_max_jerk(input: *mut InputRaw);
    fn ruckig_input_set_per_section_max_position(
        input: *mut InputRaw,
        values: *const f64,
        section_count: usize,
        dofs: usize,
    ) -> i32;
    fn ruckig_input_clear_per_section_max_position(input: *mut InputRaw);
    fn ruckig_input_set_per_section_min_position(
        input: *mut InputRaw,
        values: *const f64,
        section_count: usize,
        dofs: usize,
    ) -> i32;
    fn ruckig_input_clear_per_section_min_position(input: *mut InputRaw);
    fn ruckig_input_set_per_section_minimum_duration(
        input: *mut InputRaw,
        values: *const f64,
        section_count: usize,
    ) -> i32;
    fn ruckig_input_clear_per_section_minimum_duration(input: *mut InputRaw);
    fn ruckig_input_set_interrupt_calculation_duration(input: *mut InputRaw, duration: f64) -> i32;
    fn ruckig_input_clear_interrupt_calculation_duration(input: *mut InputRaw);

    fn ruckig_output_create(output: *mut *mut OutputRaw, dofs: usize) -> i32;
    fn ruckig_output_create_with_waypoints(
        output: *mut *mut OutputRaw,
        dofs: usize,
        max_number_of_waypoints: usize,
    ) -> i32;
    fn ruckig_output_destroy(output: *mut OutputRaw);
    fn ruckig_output_pass_to_input(output: *const OutputRaw, input: *mut InputRaw);
    fn ruckig_output_new_position_data(output: *const OutputRaw) -> *const f64;
    fn ruckig_output_new_velocity_data(output: *const OutputRaw) -> *const f64;
    fn ruckig_output_new_acceleration_data(output: *const OutputRaw) -> *const f64;
    fn ruckig_output_new_jerk_data(output: *const OutputRaw) -> *const f64;
    fn ruckig_output_get_time(output: *const OutputRaw) -> f64;
    fn ruckig_output_get_new_section(output: *const OutputRaw) -> usize;
    fn ruckig_output_did_section_change(output: *const OutputRaw) -> bool;
    fn ruckig_output_new_calculation(output: *const OutputRaw) -> bool;
    fn ruckig_output_was_calculation_interrupted(output: *const OutputRaw) -> bool;
    fn ruckig_output_get_calculation_duration(output: *const OutputRaw) -> f64;

    fn ruckig_trajectory_create(trajectory: *mut *mut TrajectoryRaw, dofs: usize) -> i32;
    fn ruckig_trajectory_create_with_waypoints(
        trajectory: *mut *mut TrajectoryRaw,
        dofs: usize,
        max_number_of_waypoints: usize,
    ) -> i32;
    fn ruckig_trajectory_destroy(trajectory: *mut TrajectoryRaw);
    fn ruckig_trajectory_get_duration(trajectory: *const TrajectoryRaw) -> f64;
    fn ruckig_trajectory_get_section_count(trajectory: *const TrajectoryRaw) -> usize;
    fn ruckig_trajectory_get_intermediate_duration_count(trajectory: *const TrajectoryRaw)
        -> usize;
    fn ruckig_trajectory_get_intermediate_durations(
        trajectory: *const TrajectoryRaw,
        durations: *mut f64,
        duration_count: usize,
    ) -> i32;
    fn ruckig_trajectory_at_time(
        trajectory: *const TrajectoryRaw,
        time: f64,
        position: *mut f64,
        velocity: *mut f64,
        acceleration: *mut f64,
        jerk: *mut f64,
        section: *mut usize,
    ) -> i32;
    fn ruckig_trajectory_get_position_extrema(
        trajectory: *const TrajectoryRaw,
        extrema: *mut CBound,
        extrema_count: usize,
    ) -> i32;
    fn ruckig_trajectory_get_first_time_at_position(
        trajectory: *const TrajectoryRaw,
        dof: usize,
        position: f64,
        time_after: f64,
        time: *mut f64,
        found: *mut bool,
    ) -> i32;

    fn ruckig_tracking_create(tracking: *mut *mut TrackingRaw, dofs: usize, delta_time: f64)
        -> i32;
    fn ruckig_tracking_destroy(tracking: *mut TrackingRaw);
    fn ruckig_tracking_get_delta_time(tracking: *const TrackingRaw) -> f64;
    fn ruckig_tracking_set_mode(tracking: *mut TrackingRaw, mode: i32) -> i32;
    fn ruckig_tracking_get_mode(tracking: *const TrackingRaw) -> i32;
    fn ruckig_tracking_set_reactiveness(tracking: *mut TrackingRaw, reactiveness: f64) -> i32;
    fn ruckig_tracking_get_reactiveness(tracking: *const TrackingRaw) -> f64;
    fn ruckig_tracking_set_look_ahead_cycles(
        tracking: *mut TrackingRaw,
        look_ahead_cycles: usize,
    ) -> i32;
    fn ruckig_tracking_get_look_ahead_cycles(tracking: *const TrackingRaw) -> usize;
    fn ruckig_tracking_set_max_optimized_candidates(
        tracking: *mut TrackingRaw,
        max_candidates: usize,
    ) -> i32;
    fn ruckig_tracking_get_max_optimized_candidates(tracking: *const TrackingRaw) -> usize;
    fn ruckig_tracking_get_last_calculation_status(tracking: *const TrackingRaw) -> i32;
    fn ruckig_tracking_get_last_candidate_count(tracking: *const TrackingRaw) -> usize;
    fn ruckig_tracking_update(
        tracking: *mut TrackingRaw,
        target_state: *const TargetStateRaw,
        input: *const InputRaw,
        output: *mut OutputRaw,
    ) -> i32;
    fn ruckig_tracking_update_with_lookahead(
        tracking: *mut TrackingRaw,
        target_sequence: *const TargetStateSequenceRaw,
        input: *const InputRaw,
        output: *mut OutputRaw,
    ) -> i32;
    fn ruckig_tracking_calculate_sequence(
        tracking: *mut TrackingRaw,
        target_sequence: *const TargetStateSequenceRaw,
        input: *const InputRaw,
        output_sequence: *mut TrackingOutputSequenceRaw,
    ) -> i32;

    fn ruckig_target_state_create(target_state: *mut *mut TargetStateRaw, dofs: usize) -> i32;
    fn ruckig_target_state_destroy(target_state: *mut TargetStateRaw);
    fn ruckig_target_state_position_data(target_state: *mut TargetStateRaw) -> *mut f64;
    fn ruckig_target_state_velocity_data(target_state: *mut TargetStateRaw) -> *mut f64;
    fn ruckig_target_state_acceleration_data(target_state: *mut TargetStateRaw) -> *mut f64;

    fn ruckig_target_state_sequence_create(
        sequence: *mut *mut TargetStateSequenceRaw,
        dofs: usize,
        capacity: usize,
    ) -> i32;
    fn ruckig_target_state_sequence_destroy(sequence: *mut TargetStateSequenceRaw);
    fn ruckig_target_state_sequence_get_capacity(sequence: *const TargetStateSequenceRaw) -> usize;
    fn ruckig_target_state_sequence_get_count(sequence: *const TargetStateSequenceRaw) -> usize;
    fn ruckig_target_state_sequence_set_count(
        sequence: *mut TargetStateSequenceRaw,
        count: usize,
    ) -> i32;
    fn ruckig_target_state_sequence_position_data(
        sequence: *mut TargetStateSequenceRaw,
    ) -> *mut f64;
    fn ruckig_target_state_sequence_velocity_data(
        sequence: *mut TargetStateSequenceRaw,
    ) -> *mut f64;
    fn ruckig_target_state_sequence_acceleration_data(
        sequence: *mut TargetStateSequenceRaw,
    ) -> *mut f64;

    fn ruckig_tracking_output_sequence_create(
        sequence: *mut *mut TrackingOutputSequenceRaw,
        dofs: usize,
        capacity: usize,
    ) -> i32;
    fn ruckig_tracking_output_sequence_destroy(sequence: *mut TrackingOutputSequenceRaw);
    fn ruckig_tracking_output_sequence_get_capacity(
        sequence: *const TrackingOutputSequenceRaw,
    ) -> usize;
    fn ruckig_tracking_output_sequence_get_count(
        sequence: *const TrackingOutputSequenceRaw,
    ) -> usize;
    fn ruckig_tracking_output_sequence_new_position_const_data(
        sequence: *const TrackingOutputSequenceRaw,
    ) -> *const f64;
    fn ruckig_tracking_output_sequence_new_velocity_const_data(
        sequence: *const TrackingOutputSequenceRaw,
    ) -> *const f64;
    fn ruckig_tracking_output_sequence_new_acceleration_const_data(
        sequence: *const TrackingOutputSequenceRaw,
    ) -> *const f64;
    fn ruckig_tracking_output_sequence_new_jerk_const_data(
        sequence: *const TrackingOutputSequenceRaw,
    ) -> *const f64;
    fn ruckig_tracking_output_sequence_time_const_data(
        sequence: *const TrackingOutputSequenceRaw,
    ) -> *const f64;
    fn ruckig_tracking_output_sequence_section_const_data(
        sequence: *const TrackingOutputSequenceRaw,
    ) -> *const usize;
    fn ruckig_tracking_output_sequence_result_const_data(
        sequence: *const TrackingOutputSequenceRaw,
    ) -> *const i32;
}

fn check_code(code: i32, operation: &'static str) -> Result<RuckigResult> {
    match code {
        0 => Ok(RuckigResult::Working),
        1 => Ok(RuckigResult::Finished),
        _ => Err(Error { code, operation }),
    }
}

fn require_len(values: &[f64], expected: usize, operation: &'static str) -> Result<()> {
    if values.len() != expected {
        return Err(Error {
            code: -100,
            operation,
        });
    }
    Ok(())
}

unsafe fn write_data(ptr: *mut f64, values: &[f64], operation: &'static str) -> Result<()> {
    if ptr.is_null() {
        return Err(Error {
            code: -100,
            operation,
        });
    }
    ptr.copy_from_nonoverlapping(values.as_ptr(), values.len());
    Ok(())
}

pub struct Ruckig {
    raw: NonNull<RuckigRaw>,
    dofs: usize,
}

impl Ruckig {
    pub fn new(dofs: usize, delta_time: f64) -> Result<Self> {
        let mut raw = std::ptr::null_mut();
        check_code(
            unsafe { ruckig_create(&mut raw, dofs, delta_time) },
            "ruckig_create",
        )?;
        Ok(Self {
            raw: NonNull::new(raw).ok_or(Error {
                code: -1,
                operation: "ruckig_create",
            })?,
            dofs,
        })
    }

    pub fn with_waypoints(
        dofs: usize,
        delta_time: f64,
        max_number_of_waypoints: usize,
    ) -> Result<Self> {
        let mut raw = std::ptr::null_mut();
        check_code(
            unsafe {
                ruckig_create_with_waypoints(&mut raw, dofs, delta_time, max_number_of_waypoints)
            },
            "ruckig_create_with_waypoints",
        )?;
        Ok(Self {
            raw: NonNull::new(raw).ok_or(Error {
                code: -1,
                operation: "ruckig_create_with_waypoints",
            })?,
            dofs,
        })
    }

    pub fn max_number_of_waypoints(&self) -> usize {
        unsafe { ruckig_get_max_number_of_waypoints(self.raw.as_ptr()) }
    }

    pub fn dofs(&self) -> usize {
        self.dofs
    }

    pub fn calculate(
        &mut self,
        input: &InputParameter,
        trajectory: &mut Trajectory,
    ) -> Result<RuckigResult> {
        check_code(
            unsafe {
                ruckig_calculate(
                    self.raw.as_ptr(),
                    input.raw.as_ptr(),
                    trajectory.raw.as_ptr(),
                )
            },
            "ruckig_calculate",
        )
    }

    pub fn update(
        &mut self,
        input: &InputParameter,
        output: &mut OutputParameter,
    ) -> Result<RuckigResult> {
        check_code(
            unsafe { ruckig_update(self.raw.as_ptr(), input.raw.as_ptr(), output.raw.as_ptr()) },
            "ruckig_update",
        )
    }

    pub fn filter_intermediate_positions(
        &self,
        input: &InputParameter,
        threshold_distance: &[f64],
    ) -> Result<Vec<f64>> {
        require_len(
            threshold_distance,
            self.dofs,
            "ruckig_filter_intermediate_positions",
        )?;
        let mut filtered = vec![0.0; input.waypoint_count * self.dofs];
        let mut written = 0usize;
        check_code(
            unsafe {
                ruckig_filter_intermediate_positions(
                    self.raw.as_ptr(),
                    input.raw.as_ptr(),
                    threshold_distance.as_ptr(),
                    threshold_distance.len(),
                    filtered.as_mut_ptr(),
                    filtered.len(),
                    &mut written,
                )
            },
            "ruckig_filter_intermediate_positions",
        )?;
        filtered.truncate(written * self.dofs);
        Ok(filtered)
    }
}

impl Drop for Ruckig {
    fn drop(&mut self) {
        unsafe { ruckig_destroy(self.raw.as_ptr()) };
    }
}

pub struct InputParameter {
    raw: NonNull<InputRaw>,
    dofs: usize,
    waypoint_count: usize,
}

impl InputParameter {
    pub fn new(dofs: usize) -> Result<Self> {
        let mut raw = std::ptr::null_mut();
        check_code(
            unsafe { ruckig_input_create(&mut raw, dofs) },
            "ruckig_input_create",
        )?;
        Ok(Self {
            raw: NonNull::new(raw).ok_or(Error {
                code: -1,
                operation: "ruckig_input_create",
            })?,
            dofs,
            waypoint_count: 0,
        })
    }

    pub fn with_waypoints(dofs: usize, max_number_of_waypoints: usize) -> Result<Self> {
        let mut raw = std::ptr::null_mut();
        check_code(
            unsafe { ruckig_input_create_with_waypoints(&mut raw, dofs, max_number_of_waypoints) },
            "ruckig_input_create_with_waypoints",
        )?;
        Ok(Self {
            raw: NonNull::new(raw).ok_or(Error {
                code: -1,
                operation: "ruckig_input_create_with_waypoints",
            })?,
            dofs,
            waypoint_count: 0,
        })
    }

    fn set_vector(
        &mut self,
        values: &[f64],
        accessor: unsafe extern "C" fn(*mut InputRaw) -> *mut f64,
        operation: &'static str,
    ) -> Result<()> {
        require_len(values, self.dofs, operation)?;
        unsafe { write_data(accessor(self.raw.as_ptr()), values, operation) }
    }

    pub fn set_current_position(&mut self, values: &[f64]) -> Result<()> {
        self.set_vector(
            values,
            ruckig_input_current_position_data,
            "set_current_position",
        )
    }

    pub fn set_current_velocity(&mut self, values: &[f64]) -> Result<()> {
        self.set_vector(
            values,
            ruckig_input_current_velocity_data,
            "set_current_velocity",
        )
    }

    pub fn set_current_acceleration(&mut self, values: &[f64]) -> Result<()> {
        self.set_vector(
            values,
            ruckig_input_current_acceleration_data,
            "set_current_acceleration",
        )
    }

    pub fn set_target_position(&mut self, values: &[f64]) -> Result<()> {
        self.set_vector(
            values,
            ruckig_input_target_position_data,
            "set_target_position",
        )
    }

    pub fn set_target_velocity(&mut self, values: &[f64]) -> Result<()> {
        self.set_vector(
            values,
            ruckig_input_target_velocity_data,
            "set_target_velocity",
        )
    }

    pub fn set_target_acceleration(&mut self, values: &[f64]) -> Result<()> {
        self.set_vector(
            values,
            ruckig_input_target_acceleration_data,
            "set_target_acceleration",
        )
    }

    pub fn set_max_velocity(&mut self, values: &[f64]) -> Result<()> {
        self.set_vector(values, ruckig_input_max_velocity_data, "set_max_velocity")
    }

    pub fn set_max_acceleration(&mut self, values: &[f64]) -> Result<()> {
        self.set_vector(
            values,
            ruckig_input_max_acceleration_data,
            "set_max_acceleration",
        )
    }

    pub fn set_max_jerk(&mut self, values: &[f64]) -> Result<()> {
        self.set_vector(values, ruckig_input_max_jerk_data, "set_max_jerk")
    }

    pub fn set_max_position(&mut self, values: &[f64]) -> Result<()> {
        self.set_vector(values, ruckig_input_max_position_data, "set_max_position")
    }

    pub fn set_min_position(&mut self, values: &[f64]) -> Result<()> {
        self.set_vector(values, ruckig_input_min_position_data, "set_min_position")
    }

    pub fn set_control_interface(&mut self, value: ControlInterface) -> Result<()> {
        check_code(
            unsafe { ruckig_input_set_control_interface(self.raw.as_ptr(), value as i32) },
            "ruckig_input_set_control_interface",
        )?;
        Ok(())
    }

    pub fn set_synchronization(&mut self, value: Synchronization) -> Result<()> {
        check_code(
            unsafe { ruckig_input_set_synchronization(self.raw.as_ptr(), value as i32) },
            "ruckig_input_set_synchronization",
        )?;
        Ok(())
    }

    pub fn set_duration_discretization(&mut self, value: DurationDiscretization) -> Result<()> {
        check_code(
            unsafe { ruckig_input_set_duration_discretization(self.raw.as_ptr(), value as i32) },
            "ruckig_input_set_duration_discretization",
        )?;
        Ok(())
    }

    pub fn set_min_velocity(&mut self, values: &[f64]) -> Result<()> {
        require_len(values, self.dofs, "ruckig_input_set_min_velocity")?;
        check_code(
            unsafe {
                ruckig_input_set_min_velocity(self.raw.as_ptr(), values.as_ptr(), values.len())
            },
            "ruckig_input_set_min_velocity",
        )?;
        Ok(())
    }

    pub fn clear_min_velocity(&mut self) {
        unsafe { ruckig_input_clear_min_velocity(self.raw.as_ptr()) };
    }

    pub fn set_min_acceleration(&mut self, values: &[f64]) -> Result<()> {
        require_len(values, self.dofs, "ruckig_input_set_min_acceleration")?;
        check_code(
            unsafe {
                ruckig_input_set_min_acceleration(self.raw.as_ptr(), values.as_ptr(), values.len())
            },
            "ruckig_input_set_min_acceleration",
        )?;
        Ok(())
    }

    pub fn clear_min_acceleration(&mut self) {
        unsafe { ruckig_input_clear_min_acceleration(self.raw.as_ptr()) };
    }

    pub fn set_minimum_duration(&mut self, duration: f64) -> Result<()> {
        check_code(
            unsafe { ruckig_input_set_minimum_duration(self.raw.as_ptr(), duration) },
            "ruckig_input_set_minimum_duration",
        )?;
        Ok(())
    }

    pub fn clear_minimum_duration(&mut self) {
        unsafe { ruckig_input_clear_minimum_duration(self.raw.as_ptr()) };
    }

    pub fn set_intermediate_positions_flat(
        &mut self,
        values: &[f64],
        waypoint_count: usize,
    ) -> Result<()> {
        require_len(
            values,
            waypoint_count * self.dofs,
            "ruckig_input_set_intermediate_positions",
        )?;
        check_code(
            unsafe {
                ruckig_input_set_intermediate_positions(
                    self.raw.as_ptr(),
                    values.as_ptr(),
                    waypoint_count,
                    self.dofs,
                )
            },
            "ruckig_input_set_intermediate_positions",
        )?;
        self.waypoint_count = waypoint_count;
        Ok(())
    }

    pub fn clear_intermediate_positions(&mut self) {
        unsafe { ruckig_input_clear_intermediate_positions(self.raw.as_ptr()) };
        self.waypoint_count = 0;
    }

    pub fn intermediate_position_count(&self) -> usize {
        unsafe { ruckig_input_get_intermediate_position_count(self.raw.as_ptr()) }
    }

    pub fn intermediate_positions_flat(&self) -> Result<Vec<f64>> {
        let count = self.intermediate_position_count();
        let mut values = vec![0.0; count * self.dofs];
        check_code(
            unsafe {
                ruckig_input_get_intermediate_positions(
                    self.raw.as_ptr(),
                    values.as_mut_ptr(),
                    values.len(),
                )
            },
            "ruckig_input_get_intermediate_positions",
        )?;
        Ok(values)
    }

    fn set_per_section_vector(
        &mut self,
        values: &[f64],
        operation: &'static str,
        setter: unsafe extern "C" fn(*mut InputRaw, *const f64, usize, usize) -> i32,
    ) -> Result<()> {
        let section_count = self.waypoint_count + 1;
        require_len(values, section_count * self.dofs, operation)?;
        check_code(
            unsafe { setter(self.raw.as_ptr(), values.as_ptr(), section_count, self.dofs) },
            operation,
        )?;
        Ok(())
    }

    pub fn set_per_section_max_velocity(&mut self, values: &[f64]) -> Result<()> {
        self.set_per_section_vector(
            values,
            "ruckig_input_set_per_section_max_velocity",
            ruckig_input_set_per_section_max_velocity,
        )
    }

    pub fn clear_per_section_max_velocity(&mut self) {
        unsafe { ruckig_input_clear_per_section_max_velocity(self.raw.as_ptr()) };
    }

    pub fn set_per_section_min_velocity(&mut self, values: &[f64]) -> Result<()> {
        self.set_per_section_vector(
            values,
            "ruckig_input_set_per_section_min_velocity",
            ruckig_input_set_per_section_min_velocity,
        )
    }

    pub fn clear_per_section_min_velocity(&mut self) {
        unsafe { ruckig_input_clear_per_section_min_velocity(self.raw.as_ptr()) };
    }

    pub fn set_per_section_max_acceleration(&mut self, values: &[f64]) -> Result<()> {
        self.set_per_section_vector(
            values,
            "ruckig_input_set_per_section_max_acceleration",
            ruckig_input_set_per_section_max_acceleration,
        )
    }

    pub fn clear_per_section_max_acceleration(&mut self) {
        unsafe { ruckig_input_clear_per_section_max_acceleration(self.raw.as_ptr()) };
    }

    pub fn set_per_section_min_acceleration(&mut self, values: &[f64]) -> Result<()> {
        self.set_per_section_vector(
            values,
            "ruckig_input_set_per_section_min_acceleration",
            ruckig_input_set_per_section_min_acceleration,
        )
    }

    pub fn clear_per_section_min_acceleration(&mut self) {
        unsafe { ruckig_input_clear_per_section_min_acceleration(self.raw.as_ptr()) };
    }

    pub fn set_per_section_max_jerk(&mut self, values: &[f64]) -> Result<()> {
        self.set_per_section_vector(
            values,
            "ruckig_input_set_per_section_max_jerk",
            ruckig_input_set_per_section_max_jerk,
        )
    }

    pub fn clear_per_section_max_jerk(&mut self) {
        unsafe { ruckig_input_clear_per_section_max_jerk(self.raw.as_ptr()) };
    }

    pub fn set_per_section_max_position(&mut self, values: &[f64]) -> Result<()> {
        self.set_per_section_vector(
            values,
            "ruckig_input_set_per_section_max_position",
            ruckig_input_set_per_section_max_position,
        )
    }

    pub fn clear_per_section_max_position(&mut self) {
        unsafe { ruckig_input_clear_per_section_max_position(self.raw.as_ptr()) };
    }

    pub fn set_per_section_min_position(&mut self, values: &[f64]) -> Result<()> {
        self.set_per_section_vector(
            values,
            "ruckig_input_set_per_section_min_position",
            ruckig_input_set_per_section_min_position,
        )
    }

    pub fn clear_per_section_min_position(&mut self) {
        unsafe { ruckig_input_clear_per_section_min_position(self.raw.as_ptr()) };
    }

    pub fn set_per_section_minimum_duration(&mut self, values: &[f64]) -> Result<()> {
        let section_count = self.waypoint_count + 1;
        require_len(
            values,
            section_count,
            "ruckig_input_set_per_section_minimum_duration",
        )?;
        check_code(
            unsafe {
                ruckig_input_set_per_section_minimum_duration(
                    self.raw.as_ptr(),
                    values.as_ptr(),
                    values.len(),
                )
            },
            "ruckig_input_set_per_section_minimum_duration",
        )?;
        Ok(())
    }

    pub fn clear_per_section_minimum_duration(&mut self) {
        unsafe { ruckig_input_clear_per_section_minimum_duration(self.raw.as_ptr()) };
    }

    pub fn set_interrupt_calculation_duration(&mut self, duration: f64) -> Result<()> {
        check_code(
            unsafe { ruckig_input_set_interrupt_calculation_duration(self.raw.as_ptr(), duration) },
            "ruckig_input_set_interrupt_calculation_duration",
        )?;
        Ok(())
    }

    pub fn clear_interrupt_calculation_duration(&mut self) {
        unsafe { ruckig_input_clear_interrupt_calculation_duration(self.raw.as_ptr()) };
    }
}

impl Drop for InputParameter {
    fn drop(&mut self) {
        unsafe { ruckig_input_destroy(self.raw.as_ptr()) };
    }
}

pub struct OutputParameter {
    raw: NonNull<OutputRaw>,
    dofs: usize,
}

impl OutputParameter {
    pub fn new(dofs: usize) -> Result<Self> {
        let mut raw = std::ptr::null_mut();
        check_code(
            unsafe { ruckig_output_create(&mut raw, dofs) },
            "ruckig_output_create",
        )?;
        Ok(Self {
            raw: NonNull::new(raw).ok_or(Error {
                code: -1,
                operation: "ruckig_output_create",
            })?,
            dofs,
        })
    }

    pub fn with_waypoints(dofs: usize, max_number_of_waypoints: usize) -> Result<Self> {
        let mut raw = std::ptr::null_mut();
        check_code(
            unsafe { ruckig_output_create_with_waypoints(&mut raw, dofs, max_number_of_waypoints) },
            "ruckig_output_create_with_waypoints",
        )?;
        Ok(Self {
            raw: NonNull::new(raw).ok_or(Error {
                code: -1,
                operation: "ruckig_output_create_with_waypoints",
            })?,
            dofs,
        })
    }

    pub fn pass_to_input(&self, input: &mut InputParameter) {
        unsafe { ruckig_output_pass_to_input(self.raw.as_ptr(), input.raw.as_ptr()) };
    }

    pub fn new_position(&self) -> Vec<f64> {
        unsafe {
            slice::from_raw_parts(
                ruckig_output_new_position_data(self.raw.as_ptr()),
                self.dofs,
            )
        }
        .to_vec()
    }

    pub fn new_velocity(&self) -> Vec<f64> {
        unsafe {
            slice::from_raw_parts(
                ruckig_output_new_velocity_data(self.raw.as_ptr()),
                self.dofs,
            )
        }
        .to_vec()
    }

    pub fn new_acceleration(&self) -> Vec<f64> {
        unsafe {
            slice::from_raw_parts(
                ruckig_output_new_acceleration_data(self.raw.as_ptr()),
                self.dofs,
            )
        }
        .to_vec()
    }

    pub fn new_jerk(&self) -> Vec<f64> {
        unsafe { slice::from_raw_parts(ruckig_output_new_jerk_data(self.raw.as_ptr()), self.dofs) }
            .to_vec()
    }

    pub fn time(&self) -> f64 {
        unsafe { ruckig_output_get_time(self.raw.as_ptr()) }
    }

    pub fn new_section(&self) -> usize {
        unsafe { ruckig_output_get_new_section(self.raw.as_ptr()) }
    }

    pub fn did_section_change(&self) -> bool {
        unsafe { ruckig_output_did_section_change(self.raw.as_ptr()) }
    }

    pub fn new_calculation(&self) -> bool {
        unsafe { ruckig_output_new_calculation(self.raw.as_ptr()) }
    }

    pub fn was_calculation_interrupted(&self) -> bool {
        unsafe { ruckig_output_was_calculation_interrupted(self.raw.as_ptr()) }
    }

    pub fn calculation_duration(&self) -> f64 {
        unsafe { ruckig_output_get_calculation_duration(self.raw.as_ptr()) }
    }
}

impl Drop for OutputParameter {
    fn drop(&mut self) {
        unsafe { ruckig_output_destroy(self.raw.as_ptr()) };
    }
}

pub struct Trajectory {
    raw: NonNull<TrajectoryRaw>,
    dofs: usize,
    _not_send_sync: PhantomData<*mut ()>,
}

impl Trajectory {
    pub fn new(dofs: usize) -> Result<Self> {
        let mut raw = std::ptr::null_mut();
        check_code(
            unsafe { ruckig_trajectory_create(&mut raw, dofs) },
            "ruckig_trajectory_create",
        )?;
        Ok(Self {
            raw: NonNull::new(raw).ok_or(Error {
                code: -1,
                operation: "ruckig_trajectory_create",
            })?,
            dofs,
            _not_send_sync: PhantomData,
        })
    }

    pub fn with_waypoints(dofs: usize, max_number_of_waypoints: usize) -> Result<Self> {
        let mut raw = std::ptr::null_mut();
        check_code(
            unsafe {
                ruckig_trajectory_create_with_waypoints(&mut raw, dofs, max_number_of_waypoints)
            },
            "ruckig_trajectory_create_with_waypoints",
        )?;
        Ok(Self {
            raw: NonNull::new(raw).ok_or(Error {
                code: -1,
                operation: "ruckig_trajectory_create_with_waypoints",
            })?,
            dofs,
            _not_send_sync: PhantomData,
        })
    }

    pub fn duration(&self) -> f64 {
        unsafe { ruckig_trajectory_get_duration(self.raw.as_ptr()) }
    }

    pub fn section_count(&self) -> usize {
        unsafe { ruckig_trajectory_get_section_count(self.raw.as_ptr()) }
    }

    pub fn intermediate_durations(&self) -> Result<Vec<f64>> {
        let count = unsafe { ruckig_trajectory_get_intermediate_duration_count(self.raw.as_ptr()) };
        let mut values = vec![0.0; count];
        check_code(
            unsafe {
                ruckig_trajectory_get_intermediate_durations(
                    self.raw.as_ptr(),
                    values.as_mut_ptr(),
                    values.len(),
                )
            },
            "ruckig_trajectory_get_intermediate_durations",
        )?;
        Ok(values)
    }

    pub fn at_time(&self, time: f64) -> Result<State> {
        let mut position = vec![0.0; self.dofs];
        let mut velocity = vec![0.0; self.dofs];
        let mut acceleration = vec![0.0; self.dofs];
        let mut jerk = vec![0.0; self.dofs];
        let mut section = 0usize;
        check_code(
            unsafe {
                ruckig_trajectory_at_time(
                    self.raw.as_ptr(),
                    time,
                    position.as_mut_ptr(),
                    velocity.as_mut_ptr(),
                    acceleration.as_mut_ptr(),
                    jerk.as_mut_ptr(),
                    &mut section,
                )
            },
            "ruckig_trajectory_at_time",
        )?;
        Ok(State {
            position,
            velocity,
            acceleration,
            jerk,
            section,
        })
    }

    pub fn position_extrema(&self) -> Result<Vec<Bound>> {
        let mut extrema = vec![
            CBound {
                min_position: 0.0,
                max_position: 0.0,
                time_min: 0.0,
                time_max: 0.0,
            };
            self.dofs
        ];
        check_code(
            unsafe {
                ruckig_trajectory_get_position_extrema(
                    self.raw.as_ptr(),
                    extrema.as_mut_ptr(),
                    extrema.len(),
                )
            },
            "ruckig_trajectory_get_position_extrema",
        )?;
        Ok(extrema
            .into_iter()
            .map(|value| Bound {
                min: value.min_position,
                max: value.max_position,
                t_min: value.time_min,
                t_max: value.time_max,
            })
            .collect())
    }

    pub fn first_time_at_position(
        &self,
        dof: usize,
        position: f64,
        time_after: f64,
    ) -> Result<Option<f64>> {
        let mut time = 0.0;
        let mut found = false;
        check_code(
            unsafe {
                ruckig_trajectory_get_first_time_at_position(
                    self.raw.as_ptr(),
                    dof,
                    position,
                    time_after,
                    &mut time,
                    &mut found,
                )
            },
            "ruckig_trajectory_get_first_time_at_position",
        )?;
        Ok(if found { Some(time) } else { None })
    }
}

impl Drop for Trajectory {
    fn drop(&mut self) {
        unsafe { ruckig_trajectory_destroy(self.raw.as_ptr()) };
    }
}

pub struct TargetState {
    raw: NonNull<TargetStateRaw>,
    dofs: usize,
}

impl TargetState {
    pub fn new(dofs: usize) -> Result<Self> {
        let mut raw = std::ptr::null_mut();
        check_code(
            unsafe { ruckig_target_state_create(&mut raw, dofs) },
            "ruckig_target_state_create",
        )?;
        Ok(Self {
            raw: NonNull::new(raw).ok_or(Error {
                code: -1,
                operation: "ruckig_target_state_create",
            })?,
            dofs,
        })
    }

    fn set_vector(
        &mut self,
        values: &[f64],
        accessor: unsafe extern "C" fn(*mut TargetStateRaw) -> *mut f64,
        operation: &'static str,
    ) -> Result<()> {
        require_len(values, self.dofs, operation)?;
        unsafe { write_data(accessor(self.raw.as_ptr()), values, operation) }
    }

    pub fn set_position(&mut self, values: &[f64]) -> Result<()> {
        self.set_vector(
            values,
            ruckig_target_state_position_data,
            "set_target_position",
        )
    }

    pub fn set_velocity(&mut self, values: &[f64]) -> Result<()> {
        self.set_vector(
            values,
            ruckig_target_state_velocity_data,
            "set_target_velocity",
        )
    }

    pub fn set_acceleration(&mut self, values: &[f64]) -> Result<()> {
        self.set_vector(
            values,
            ruckig_target_state_acceleration_data,
            "set_target_acceleration",
        )
    }
}

impl Drop for TargetState {
    fn drop(&mut self) {
        unsafe { ruckig_target_state_destroy(self.raw.as_ptr()) };
    }
}

pub struct TargetStateSequence {
    raw: NonNull<TargetStateSequenceRaw>,
    dofs: usize,
}

impl TargetStateSequence {
    pub fn new(dofs: usize, capacity: usize) -> Result<Self> {
        let mut raw = std::ptr::null_mut();
        check_code(
            unsafe { ruckig_target_state_sequence_create(&mut raw, dofs, capacity) },
            "ruckig_target_state_sequence_create",
        )?;
        Ok(Self {
            raw: NonNull::new(raw).ok_or(Error {
                code: -1,
                operation: "ruckig_target_state_sequence_create",
            })?,
            dofs,
        })
    }

    pub fn capacity(&self) -> usize {
        unsafe { ruckig_target_state_sequence_get_capacity(self.raw.as_ptr()) }
    }

    pub fn count(&self) -> usize {
        unsafe { ruckig_target_state_sequence_get_count(self.raw.as_ptr()) }
    }

    pub fn set_count(&mut self, count: usize) -> Result<()> {
        check_code(
            unsafe { ruckig_target_state_sequence_set_count(self.raw.as_ptr(), count) },
            "ruckig_target_state_sequence_set_count",
        )?;
        Ok(())
    }

    pub fn set_state(
        &mut self,
        index: usize,
        position: &[f64],
        velocity: &[f64],
        acceleration: &[f64],
    ) -> Result<()> {
        require_len(position, self.dofs, "target_sequence_set_position")?;
        require_len(velocity, self.dofs, "target_sequence_set_velocity")?;
        require_len(acceleration, self.dofs, "target_sequence_set_acceleration")?;
        if index >= self.capacity() {
            return Err(Error {
                code: -100,
                operation: "target_sequence_set_state",
            });
        }
        let offset = index * self.dofs;
        unsafe {
            let position_ptr = ruckig_target_state_sequence_position_data(self.raw.as_ptr());
            let velocity_ptr = ruckig_target_state_sequence_velocity_data(self.raw.as_ptr());
            let acceleration_ptr =
                ruckig_target_state_sequence_acceleration_data(self.raw.as_ptr());
            write_data(
                position_ptr.add(offset),
                position,
                "target_sequence_set_position",
            )?;
            write_data(
                velocity_ptr.add(offset),
                velocity,
                "target_sequence_set_velocity",
            )?;
            write_data(
                acceleration_ptr.add(offset),
                acceleration,
                "target_sequence_set_acceleration",
            )?;
        }
        Ok(())
    }
}

impl Drop for TargetStateSequence {
    fn drop(&mut self) {
        unsafe { ruckig_target_state_sequence_destroy(self.raw.as_ptr()) };
    }
}

pub struct TrackingOutputSequence {
    raw: NonNull<TrackingOutputSequenceRaw>,
    dofs: usize,
}

impl TrackingOutputSequence {
    pub fn new(dofs: usize, capacity: usize) -> Result<Self> {
        let mut raw = std::ptr::null_mut();
        check_code(
            unsafe { ruckig_tracking_output_sequence_create(&mut raw, dofs, capacity) },
            "ruckig_tracking_output_sequence_create",
        )?;
        Ok(Self {
            raw: NonNull::new(raw).ok_or(Error {
                code: -1,
                operation: "ruckig_tracking_output_sequence_create",
            })?,
            dofs,
        })
    }

    pub fn capacity(&self) -> usize {
        unsafe { ruckig_tracking_output_sequence_get_capacity(self.raw.as_ptr()) }
    }

    pub fn count(&self) -> usize {
        unsafe { ruckig_tracking_output_sequence_get_count(self.raw.as_ptr()) }
    }

    fn flat_f64(
        &self,
        accessor: unsafe extern "C" fn(*const TrackingOutputSequenceRaw) -> *const f64,
    ) -> Vec<f64> {
        unsafe { slice::from_raw_parts(accessor(self.raw.as_ptr()), self.count() * self.dofs) }
            .to_vec()
    }

    pub fn new_positions_flat(&self) -> Vec<f64> {
        self.flat_f64(ruckig_tracking_output_sequence_new_position_const_data)
    }

    pub fn new_velocities_flat(&self) -> Vec<f64> {
        self.flat_f64(ruckig_tracking_output_sequence_new_velocity_const_data)
    }

    pub fn new_accelerations_flat(&self) -> Vec<f64> {
        self.flat_f64(ruckig_tracking_output_sequence_new_acceleration_const_data)
    }

    pub fn new_jerks_flat(&self) -> Vec<f64> {
        self.flat_f64(ruckig_tracking_output_sequence_new_jerk_const_data)
    }

    pub fn times(&self) -> Vec<f64> {
        unsafe {
            slice::from_raw_parts(
                ruckig_tracking_output_sequence_time_const_data(self.raw.as_ptr()),
                self.count(),
            )
        }
        .to_vec()
    }

    pub fn sections(&self) -> Vec<usize> {
        unsafe {
            slice::from_raw_parts(
                ruckig_tracking_output_sequence_section_const_data(self.raw.as_ptr()),
                self.count(),
            )
        }
        .to_vec()
    }

    pub fn result_codes(&self) -> Vec<i32> {
        unsafe {
            slice::from_raw_parts(
                ruckig_tracking_output_sequence_result_const_data(self.raw.as_ptr()),
                self.count(),
            )
        }
        .to_vec()
    }
}

impl Drop for TrackingOutputSequence {
    fn drop(&mut self) {
        unsafe { ruckig_tracking_output_sequence_destroy(self.raw.as_ptr()) };
    }
}

pub struct Tracking {
    raw: NonNull<TrackingRaw>,
    dofs: usize,
}

impl Tracking {
    pub fn new(dofs: usize, delta_time: f64) -> Result<Self> {
        let mut raw = std::ptr::null_mut();
        check_code(
            unsafe { ruckig_tracking_create(&mut raw, dofs, delta_time) },
            "ruckig_tracking_create",
        )?;
        Ok(Self {
            raw: NonNull::new(raw).ok_or(Error {
                code: -1,
                operation: "ruckig_tracking_create",
            })?,
            dofs,
        })
    }

    pub fn dofs(&self) -> usize {
        self.dofs
    }

    pub fn delta_time(&self) -> f64 {
        unsafe { ruckig_tracking_get_delta_time(self.raw.as_ptr()) }
    }

    pub fn mode(&self) -> TrackingMode {
        match unsafe { ruckig_tracking_get_mode(self.raw.as_ptr()) } {
            1 => TrackingMode::Optimized,
            _ => TrackingMode::Fast,
        }
    }

    pub fn set_mode(&mut self, mode: TrackingMode) -> Result<()> {
        check_code(
            unsafe { ruckig_tracking_set_mode(self.raw.as_ptr(), mode as i32) },
            "ruckig_tracking_set_mode",
        )?;
        Ok(())
    }

    pub fn reactiveness(&self) -> f64 {
        unsafe { ruckig_tracking_get_reactiveness(self.raw.as_ptr()) }
    }

    pub fn set_reactiveness(&mut self, reactiveness: f64) -> Result<()> {
        check_code(
            unsafe { ruckig_tracking_set_reactiveness(self.raw.as_ptr(), reactiveness) },
            "ruckig_tracking_set_reactiveness",
        )?;
        Ok(())
    }

    pub fn look_ahead_cycles(&self) -> usize {
        unsafe { ruckig_tracking_get_look_ahead_cycles(self.raw.as_ptr()) }
    }

    pub fn set_look_ahead_cycles(&mut self, cycles: usize) -> Result<()> {
        check_code(
            unsafe { ruckig_tracking_set_look_ahead_cycles(self.raw.as_ptr(), cycles) },
            "ruckig_tracking_set_look_ahead_cycles",
        )?;
        Ok(())
    }

    pub fn max_optimized_candidates(&self) -> usize {
        unsafe { ruckig_tracking_get_max_optimized_candidates(self.raw.as_ptr()) }
    }

    pub fn set_max_optimized_candidates(&mut self, max_candidates: usize) -> Result<()> {
        check_code(
            unsafe {
                ruckig_tracking_set_max_optimized_candidates(self.raw.as_ptr(), max_candidates)
            },
            "ruckig_tracking_set_max_optimized_candidates",
        )?;
        Ok(())
    }

    pub fn last_calculation_status(&self) -> TrackingCalculationStatus {
        match unsafe { ruckig_tracking_get_last_calculation_status(self.raw.as_ptr()) } {
            1 => TrackingCalculationStatus::Fast,
            2 => TrackingCalculationStatus::Optimized,
            3 => TrackingCalculationStatus::FastFallback,
            4 => TrackingCalculationStatus::Error,
            _ => TrackingCalculationStatus::None,
        }
    }

    pub fn last_candidate_count(&self) -> usize {
        unsafe { ruckig_tracking_get_last_candidate_count(self.raw.as_ptr()) }
    }

    pub fn update(
        &mut self,
        target_state: &TargetState,
        input: &InputParameter,
        output: &mut OutputParameter,
    ) -> Result<RuckigResult> {
        check_code(
            unsafe {
                ruckig_tracking_update(
                    self.raw.as_ptr(),
                    target_state.raw.as_ptr(),
                    input.raw.as_ptr(),
                    output.raw.as_ptr(),
                )
            },
            "ruckig_tracking_update",
        )
    }

    pub fn update_with_lookahead(
        &mut self,
        target_sequence: &TargetStateSequence,
        input: &InputParameter,
        output: &mut OutputParameter,
    ) -> Result<RuckigResult> {
        check_code(
            unsafe {
                ruckig_tracking_update_with_lookahead(
                    self.raw.as_ptr(),
                    target_sequence.raw.as_ptr(),
                    input.raw.as_ptr(),
                    output.raw.as_ptr(),
                )
            },
            "ruckig_tracking_update_with_lookahead",
        )
    }

    pub fn calculate_sequence(
        &mut self,
        target_sequence: &TargetStateSequence,
        input: &InputParameter,
        output_sequence: &mut TrackingOutputSequence,
    ) -> Result<RuckigResult> {
        check_code(
            unsafe {
                ruckig_tracking_calculate_sequence(
                    self.raw.as_ptr(),
                    target_sequence.raw.as_ptr(),
                    input.raw.as_ptr(),
                    output_sequence.raw.as_ptr(),
                )
            },
            "ruckig_tracking_calculate_sequence",
        )
    }
}

impl Drop for Tracking {
    fn drop(&mut self) {
        unsafe { ruckig_tracking_destroy(self.raw.as_ptr()) };
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn configure_position(input: &mut InputParameter) -> Result<()> {
        input.set_target_position(&[1.0])?;
        input.set_max_velocity(&[1.0])?;
        input.set_max_acceleration(&[1.0])?;
        input.set_max_jerk(&[1.0])?;
        Ok(())
    }

    fn configure_waypoint(input: &mut InputParameter) -> Result<()> {
        input.set_target_position(&[2.0])?;
        input.set_max_velocity(&[1.2])?;
        input.set_max_acceleration(&[2.0])?;
        input.set_max_jerk(&[4.0])?;
        input.set_intermediate_positions_flat(&[1.0], 1)?;
        Ok(())
    }

    fn configure_tracking_input(input: &mut InputParameter) -> Result<()> {
        input.set_current_position(&[0.0])?;
        input.set_current_velocity(&[0.0])?;
        input.set_current_acceleration(&[0.0])?;
        input.set_target_position(&[0.0])?;
        input.set_target_velocity(&[0.0])?;
        input.set_target_acceleration(&[0.0])?;
        input.set_max_velocity(&[1.0])?;
        input.set_max_acceleration(&[2.0])?;
        input.set_max_jerk(&[5.0])?;
        Ok(())
    }

    fn configure_tracking_input_nd(input: &mut InputParameter, dofs: usize) -> Result<()> {
        let zeros = vec![0.0; dofs];
        let max_velocity: Vec<f64> = (0..dofs).map(|dof| 1.0 + 0.1 * dof as f64).collect();
        let max_acceleration: Vec<f64> = (0..dofs).map(|dof| 2.0 + 0.1 * dof as f64).collect();
        let max_jerk: Vec<f64> = (0..dofs).map(|dof| 5.0 + 0.25 * dof as f64).collect();
        input.set_current_position(&zeros)?;
        input.set_current_velocity(&zeros)?;
        input.set_current_acceleration(&zeros)?;
        input.set_target_position(&zeros)?;
        input.set_target_velocity(&zeros)?;
        input.set_target_acceleration(&zeros)?;
        input.set_max_velocity(&max_velocity)?;
        input.set_max_acceleration(&max_acceleration)?;
        input.set_max_jerk(&max_jerk)?;
        Ok(())
    }

    #[test]
    fn offline_position_calculate() -> Result<()> {
        let mut otg = Ruckig::new(1, 0.1)?;
        let mut input = InputParameter::new(1)?;
        let mut trajectory = Trajectory::new(1)?;
        configure_position(&mut input)?;
        assert_eq!(
            otg.calculate(&input, &mut trajectory)?,
            RuckigResult::Working
        );
        let final_state = trajectory.at_time(trajectory.duration())?;
        assert!((final_state.position[0] - 1.0).abs() < 1e-8);
        assert!(final_state.velocity[0].abs() < 1e-8);
        Ok(())
    }

    #[test]
    fn online_update_loop() -> Result<()> {
        let mut otg = Ruckig::new(1, 0.1)?;
        let mut input = InputParameter::new(1)?;
        let mut output = OutputParameter::new(1)?;
        configure_position(&mut input)?;
        let mut result = RuckigResult::Working;
        for _ in 0..200 {
            result = otg.update(&input, &mut output)?;
            if result == RuckigResult::Finished {
                break;
            }
            output.pass_to_input(&mut input);
        }
        assert_eq!(result, RuckigResult::Finished);
        assert!((output.new_position()[0] - 1.0).abs() < 1e-8);
        Ok(())
    }

    #[test]
    fn waypoint_offline_calculate() -> Result<()> {
        let mut otg = Ruckig::with_waypoints(1, 0.05, 1)?;
        let mut input = InputParameter::with_waypoints(1, 1)?;
        let mut trajectory = Trajectory::with_waypoints(1, 1)?;
        configure_waypoint(&mut input)?;
        assert_eq!(otg.max_number_of_waypoints(), 1);
        assert_eq!(input.intermediate_position_count(), 1);
        assert_eq!(input.intermediate_positions_flat()?, vec![1.0]);
        assert_eq!(
            otg.calculate(&input, &mut trajectory)?,
            RuckigResult::Working
        );
        assert_eq!(trajectory.section_count(), 2);
        let durations = trajectory.intermediate_durations()?;
        assert_eq!(durations.len(), 1);
        let waypoint = trajectory.at_time(durations[0])?;
        assert!((waypoint.position[0] - 1.0).abs() < 1e-7);
        assert_eq!(waypoint.section, 1);
        let first_time = trajectory.first_time_at_position(0, 1.0, 0.0)?;
        assert!(first_time.is_some());
        assert!((first_time.unwrap() - durations[0]).abs() < 1e-7);
        assert_eq!(
            otg.filter_intermediate_positions(&input, &[0.25])?,
            Vec::<f64>::new()
        );
        Ok(())
    }

    #[test]
    fn per_section_minimum_duration() -> Result<()> {
        let mut otg = Ruckig::with_waypoints(1, 0.05, 1)?;
        let mut input = InputParameter::with_waypoints(1, 1)?;
        let mut trajectory = Trajectory::with_waypoints(1, 1)?;
        configure_waypoint(&mut input)?;
        input.set_per_section_minimum_duration(&[2.0, 1.0])?;
        assert_eq!(
            otg.calculate(&input, &mut trajectory)?,
            RuckigResult::Working
        );
        let durations = trajectory.intermediate_durations()?;
        assert!(durations[0] >= 2.0);
        assert!(trajectory.duration() >= 3.0);
        Ok(())
    }

    #[test]
    fn per_section_limits_and_extrema() -> Result<()> {
        let mut otg = Ruckig::with_waypoints(1, 0.01, 1)?;
        let mut input = InputParameter::with_waypoints(1, 1)?;
        let mut trajectory = Trajectory::with_waypoints(1, 1)?;
        input.set_target_position(&[2.0])?;
        input.set_max_velocity(&[1.5])?;
        input.set_max_acceleration(&[2.0])?;
        input.set_max_jerk(&[5.0])?;
        input.set_intermediate_positions_flat(&[1.0], 1)?;
        input.set_per_section_max_velocity(&[0.8, 1.4])?;
        input.set_per_section_min_velocity(&[-0.8, -1.4])?;
        input.set_per_section_max_acceleration(&[1.2, 2.0])?;
        input.set_per_section_min_acceleration(&[-1.2, -2.0])?;
        input.set_per_section_max_jerk(&[3.0, 5.0])?;
        input.set_per_section_max_position(&[1.1, 2.1])?;
        input.set_per_section_min_position(&[-0.1, 0.9])?;
        assert_eq!(
            otg.calculate(&input, &mut trajectory)?,
            RuckigResult::Working
        );
        let extrema = trajectory.position_extrema()?;
        assert!(extrema[0].min >= -1e-9);
        assert!(extrema[0].max <= 2.0 + 1e-9);
        Ok(())
    }

    #[test]
    fn waypoint_clear_and_interrupt_surface() -> Result<()> {
        let mut otg = Ruckig::with_waypoints(1, 0.05, 1)?;
        let mut input = InputParameter::with_waypoints(1, 1)?;
        let mut output = OutputParameter::with_waypoints(1, 1)?;
        configure_waypoint(&mut input)?;
        input.set_min_velocity(&[-1.0])?;
        input.clear_min_velocity();
        input.set_min_acceleration(&[-2.0])?;
        input.clear_min_acceleration();
        input.set_minimum_duration(1.0)?;
        input.clear_minimum_duration();
        input.set_per_section_max_velocity(&[1.2, 1.0])?;
        input.clear_per_section_max_velocity();
        input.set_per_section_min_velocity(&[-1.2, -1.0])?;
        input.clear_per_section_min_velocity();
        input.set_per_section_max_acceleration(&[2.0, 2.0])?;
        input.clear_per_section_max_acceleration();
        input.set_per_section_min_acceleration(&[-2.0, -2.0])?;
        input.clear_per_section_min_acceleration();
        input.set_per_section_max_jerk(&[4.0, 4.0])?;
        input.clear_per_section_max_jerk();
        input.set_per_section_max_position(&[1.5, 2.5])?;
        input.clear_per_section_max_position();
        input.set_per_section_min_position(&[-0.5, 0.5])?;
        input.clear_per_section_min_position();
        input.set_per_section_minimum_duration(&[0.0, 0.0])?;
        input.clear_per_section_minimum_duration();
        input.set_interrupt_calculation_duration(0.001)?;
        input.clear_interrupt_calculation_duration();

        assert_eq!(otg.update(&input, &mut output)?, RuckigResult::Working);
        assert!(output.new_calculation());
        assert_eq!(output.new_acceleration().len(), 1);
        assert_eq!(output.new_jerk().len(), 1);
        assert!(!output.was_calculation_interrupted());
        assert!(output.calculation_duration() >= 0.0);
        Ok(())
    }

    #[test]
    fn tracking_online_fast_loop() -> Result<()> {
        let mut tracking = Tracking::new(1, 0.01)?;
        let mut target = TargetState::new(1)?;
        let mut input = InputParameter::new(1)?;
        let mut output = OutputParameter::new(1)?;
        configure_tracking_input(&mut input)?;
        assert_eq!(tracking.mode(), TrackingMode::Fast);
        assert_eq!(tracking.look_ahead_cycles(), 1);
        assert_eq!(tracking.max_optimized_candidates(), 16);
        assert_eq!(
            tracking.last_calculation_status(),
            TrackingCalculationStatus::None
        );
        assert_eq!(tracking.last_candidate_count(), 0);
        assert!((tracking.reactiveness() - 1.0).abs() < f64::EPSILON);

        for step in 0..200 {
            let t = step as f64 * tracking.delta_time();
            target.set_position(&[0.5 * t])?;
            target.set_velocity(&[0.5])?;
            target.set_acceleration(&[0.0])?;
            assert_eq!(
                tracking.update(&target, &input, &mut output)?,
                RuckigResult::Working
            );
            output.pass_to_input(&mut input);
        }
        assert!(output.new_position()[0] > 0.0);
        assert!(output.new_position()[0] < 1.0);
        Ok(())
    }

    #[test]
    fn tracking_online_fast_multidof_loop() -> Result<()> {
        let mut tracking = Tracking::new(2, 0.01)?;
        let mut target = TargetState::new(2)?;
        let mut input = InputParameter::new(2)?;
        let mut output = OutputParameter::new(2)?;
        configure_tracking_input_nd(&mut input, 2)?;
        tracking.set_reactiveness(0.5)?;
        tracking.set_look_ahead_cycles(2)?;

        for step in 0..120 {
            let t = step as f64 * tracking.delta_time();
            target.set_position(&[0.35 * t, -0.2 * t])?;
            target.set_velocity(&[0.35, -0.2])?;
            target.set_acceleration(&[0.0, 0.0])?;
            assert_eq!(
                tracking.update(&target, &input, &mut output)?,
                RuckigResult::Working
            );
            assert_eq!(output.new_position().len(), 2);
            assert!(output.new_position().iter().all(|value| value.is_finite()));
            assert!(output.new_velocity().iter().all(|value| value.is_finite()));
            output.pass_to_input(&mut input);
        }
        Ok(())
    }

    #[test]
    fn tracking_offline_sequence() -> Result<()> {
        let count = 200;
        let mut tracking = Tracking::new(1, 0.01)?;
        let mut targets = TargetStateSequence::new(1, count)?;
        let mut outputs = TrackingOutputSequence::new(1, count)?;
        let mut input = InputParameter::new(1)?;
        configure_tracking_input(&mut input)?;
        targets.set_count(count)?;
        for step in 0..count {
            let t = step as f64 * tracking.delta_time();
            targets.set_state(step, &[0.5 * t], &[0.5], &[0.0])?;
        }
        assert_eq!(
            tracking.calculate_sequence(&targets, &input, &mut outputs)?,
            RuckigResult::Working
        );
        assert_eq!(outputs.count(), count);
        assert_eq!(outputs.times().len(), count);
        assert_eq!(outputs.new_velocities_flat().len(), count);
        assert_eq!(outputs.new_accelerations_flat().len(), count);
        assert_eq!(outputs.new_jerks_flat().len(), count);
        assert_eq!(outputs.sections().len(), count);
        assert!(outputs
            .result_codes()
            .iter()
            .all(|code| *code == RuckigResult::Working as i32
                || *code == RuckigResult::Finished as i32));
        let positions = outputs.new_positions_flat();
        assert!(positions[count - 1] > positions[0]);
        Ok(())
    }

    #[test]
    fn tracking_offline_sequence_multidof() -> Result<()> {
        let count = 80;
        let mut tracking = Tracking::new(2, 0.01)?;
        let mut targets = TargetStateSequence::new(2, count)?;
        let mut outputs = TrackingOutputSequence::new(2, count)?;
        let mut input = InputParameter::new(2)?;
        configure_tracking_input_nd(&mut input, 2)?;
        targets.set_count(count)?;
        for step in 0..count {
            let t = step as f64 * tracking.delta_time();
            targets.set_state(step, &[0.25 * t, -0.1 * t], &[0.25, -0.1], &[0.0, 0.0])?;
        }
        assert_eq!(
            tracking.calculate_sequence(&targets, &input, &mut outputs)?,
            RuckigResult::Working
        );
        assert_eq!(outputs.count(), count);
        assert_eq!(outputs.new_positions_flat().len(), count * 2);
        assert!(outputs
            .new_accelerations_flat()
            .iter()
            .all(|value| value.is_finite()));
        assert!(outputs
            .new_jerks_flat()
            .iter()
            .all(|value| value.is_finite()));
        assert!(outputs.times().iter().all(|time| *time > 0.0));
        Ok(())
    }

    #[test]
    fn tracking_invalid_parameters_map_errors() -> Result<()> {
        let mut tracking = Tracking::new(1, 0.01)?;
        assert_eq!(tracking.set_reactiveness(-0.01).unwrap_err().code, -100);
        assert_eq!(tracking.set_reactiveness(1.01).unwrap_err().code, -100);
        assert_eq!(tracking.set_reactiveness(f64::NAN).unwrap_err().code, -100);
        assert_eq!(tracking.set_look_ahead_cycles(0).unwrap_err().code, -100);
        assert_eq!(
            tracking.set_max_optimized_candidates(0).unwrap_err().code,
            -100
        );
        assert_eq!(
            tracking.set_max_optimized_candidates(129).unwrap_err().code,
            -100
        );
        Ok(())
    }

    #[test]
    fn tracking_optimized_smoke() -> Result<()> {
        let mut tracking = Tracking::new(1, 0.01)?;
        let mut target = TargetState::new(1)?;
        let mut targets = TargetStateSequence::new(1, 4)?;
        let mut outputs = TrackingOutputSequence::new(1, 4)?;
        let mut input = InputParameter::new(1)?;
        let mut output = OutputParameter::new(1)?;
        configure_tracking_input(&mut input)?;
        target.set_position(&[0.0])?;
        target.set_velocity(&[0.5])?;
        target.set_acceleration(&[0.0])?;
        targets.set_count(4)?;
        for step in 0..4 {
            let t = step as f64 * tracking.delta_time();
            targets.set_state(step, &[0.5 * t], &[0.5], &[0.0])?;
        }
        tracking.set_mode(TrackingMode::Optimized)?;
        tracking.set_look_ahead_cycles(4)?;
        tracking.set_max_optimized_candidates(16)?;
        let result = tracking.update(&target, &input, &mut output)?;
        assert!(result == RuckigResult::Working || result == RuckigResult::Finished);
        assert!(
            tracking.last_calculation_status() == TrackingCalculationStatus::Optimized
                || tracking.last_calculation_status() == TrackingCalculationStatus::FastFallback
        );
        assert!(tracking.last_candidate_count() >= 1);
        assert!(tracking.last_candidate_count() <= tracking.max_optimized_candidates());

        let result = tracking.update_with_lookahead(&targets, &input, &mut output)?;
        assert!(result == RuckigResult::Working || result == RuckigResult::Finished);
        assert!(
            tracking.last_calculation_status() == TrackingCalculationStatus::Optimized
                || tracking.last_calculation_status() == TrackingCalculationStatus::FastFallback
        );
        assert!(tracking.last_candidate_count() >= 1);

        assert_eq!(
            tracking.calculate_sequence(&targets, &input, &mut outputs)?,
            RuckigResult::Working
        );
        assert_eq!(outputs.count(), 4);
        assert!(
            tracking.last_calculation_status() == TrackingCalculationStatus::Optimized
                || tracking.last_calculation_status() == TrackingCalculationStatus::FastFallback
        );
        assert!(tracking.last_candidate_count() >= 4);
        Ok(())
    }
}
