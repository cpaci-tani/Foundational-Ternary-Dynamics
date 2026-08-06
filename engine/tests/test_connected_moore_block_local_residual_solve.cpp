// FTD-0692: exact-equivalence gate for local nonlinear residual storage.

#include "ftd/eft/connected_moore_block_action.h"
#include "ftd/eft/production_hop_kinematics.h"

#include <algorithm>
#include <cmath>
#include <iostream>

namespace {

double common_residual(const ftd::eft::ConnectedMooreBlockStepResult& step) {
  return std::max({step.root_residual,step.continuity_residual,
      step.gauss_before_residual,step.gauss_after_residual,
      step.force_residual,step.kinematic_residual,
      step.kinetic_discrete_gradient_residual,
      step.electric_adjoint_residual,step.magnetic_work_residual,
      step.binding_work_residual,step.binding_impulse_sum_residual,
      step.matter_work_residual,step.field_work_residual,
      step.total_energy_residual,step.causal_speed_excess});
}

bool accepted(const ftd::eft::ConnectedMooreBlockStepResult& step) {
  return step.valid && step.common_action_gates_pass
      && common_residual(step) <= 1e-10;
}

}  // namespace

int main() {
  const auto initialized = ftd::eft::initialize_connected_moore_block(
      17,2,0,0,0.125,1e-13,4096);
  if (!initialized.valid || initialized.state.constituents.size() != 16)
    return 1;
  auto initial = initialized.state;
  const ftd::Vec3 launch_velocity{0.02,-0.01,0.015};
  const auto launch_momentum =
      ftd::eft::production_flat_momentum(launch_velocity);
  for (auto& constituent : initial.constituents)
    constituent.momentum = launch_momentum;

  ftd::eft::ConnectedMooreBlockOptions full_options;
  full_options.allow_shared_anchor_chart = true;
  full_options.use_sparse_local_current = true;
  auto local_options = full_options;
  local_options.use_local_residual_evaluation = true;

  const auto full_forward = ftd::eft::solve_connected_moore_block_forward(
      initial, full_options);
  const auto local_forward = ftd::eft::solve_connected_moore_block_forward(
      initial, local_options);
  if (!accepted(full_forward) || !accepted(local_forward)) return 1;

  const double forward_difference =
      ftd::eft::connected_moore_block_state_max_difference(
          full_forward.later, local_forward.later);
  const auto full_reverse = ftd::eft::solve_connected_moore_block_reverse(
      full_forward.later, full_options);
  const auto local_reverse = ftd::eft::solve_connected_moore_block_reverse(
      local_forward.later, local_options);
  if (!accepted(full_reverse) || !accepted(local_reverse)) return 1;

  const double reverse_difference =
      ftd::eft::connected_moore_block_state_max_difference(
          full_reverse.earlier, local_reverse.earlier);
  const double full_recovery =
      ftd::eft::connected_moore_block_state_max_difference(
          initial, full_reverse.earlier);
  const double local_recovery =
      ftd::eft::connected_moore_block_state_max_difference(
          initial, local_reverse.earlier);
  const double materialized_difference = std::max(
      local_forward.solve.materialized_residual_difference,
      local_reverse.solve.materialized_residual_difference);
  const bool same_solver_path =
      full_forward.solve.residual_evaluations
          == local_forward.solve.residual_evaluations
      && full_reverse.solve.residual_evaluations
          == local_reverse.solve.residual_evaluations
      && full_forward.solve.iterations == local_forward.solve.iterations
      && full_reverse.solve.iterations == local_reverse.solve.iterations;
  const bool constructive = same_solver_path
      && local_forward.solve.full_candidate_materializations == 1
      && local_reverse.solve.full_candidate_materializations == 1
      && materialized_difference <= 1e-14
      && forward_difference <= 1e-10
      && reverse_difference <= 1e-10
      && full_recovery <= 1e-9 && local_recovery <= 1e-9;

  // A deliberately stopped local solve must fail closed.  Its last residual
  // candidate is sparse scratch storage with no materialized lattice fields;
  // finalization must never interpret it as a physical transaction.
  auto stopped_options = local_options;
  stopped_options.max_iterations = 1;
  stopped_options.solve_tolerance = 1e-30;
  const auto stopped = ftd::eft::solve_connected_moore_block_forward(
      initial, stopped_options);
  const bool stopped_fail_closed = stopped.solve.attempted
      && !stopped.solve.converged && !stopped.valid
      && !stopped.common_action_gates_pass
      && stopped.earlier.electric.L == 0 && stopped.later.electric.L == 0;

  ftd::eft::ConnectedMooreBlockSolveCache poisoned_cache;
  poisoned_cache.valid = true;
  poisoned_cache.dimension = 3 * initial.constituents.size();
  poisoned_cache.jacobian.assign(
      poisoned_cache.dimension * poisoned_cache.dimension, 0.0);
  const auto fallback_forward =
      ftd::eft::solve_connected_moore_block_forward(
          initial, local_options, &poisoned_cache);
  const double fallback_difference =
      ftd::eft::connected_moore_block_state_max_difference(
          local_forward.later, fallback_forward.later);
  const bool cache_fail_closed_then_fallback = accepted(fallback_forward)
      && fallback_forward.solve.cache_fallbacks == 1
      && fallback_forward.solve.discarded_cache_residual_evaluations > 0
      && fallback_difference <= 1e-10;

  // FTD-0735 observer: measuring the accepted residual Jacobian must not
  // change the selected state, and its two-scale spectrum must be finite.
  auto regularity_options = local_options;
  regularity_options.measure_final_root_regularity = true;
  const auto regularity_forward =
      ftd::eft::solve_connected_moore_block_forward(
          initial, regularity_options);
  const double regularity_state_difference =
      ftd::eft::connected_moore_block_state_max_difference(
          local_forward.later, regularity_forward.later);
  const bool regularity_observer_only = accepted(regularity_forward)
      && regularity_forward.solve.final_root_regularity_measured
      && regularity_forward.solve.regularity_residual_evaluations
          == 12 * static_cast<int>(initial.constituents.size())
      && regularity_forward.solve.final_minimum_singular_value > 0.0
      && regularity_forward.solve.final_maximum_singular_value
          >= regularity_forward.solve.final_minimum_singular_value
      && std::isfinite(regularity_forward.solve.final_condition_number)
      && std::isfinite(
          regularity_forward.solve.regularity_scale_relative_difference)
      && regularity_state_difference <= 1e-12;

  std::cout << "forward_difference=" << forward_difference
            << " reverse_difference=" << reverse_difference
            << " materialized_residual_difference="
            << materialized_difference
            << " full_recovery=" << full_recovery
            << " local_recovery=" << local_recovery
            << " full_forward_evaluations="
            << full_forward.solve.residual_evaluations
            << " local_forward_evaluations="
            << local_forward.solve.residual_evaluations
            << " stopped_fail_closed=" << stopped_fail_closed
            << " cache_fallback=" << cache_fail_closed_then_fallback
            << " fallback_difference=" << fallback_difference
            << " regularity_observer=" << regularity_observer_only
            << " regularity_sigma_min="
            << regularity_forward.solve.final_minimum_singular_value
            << " regularity_condition="
            << regularity_forward.solve.final_condition_number
            << " regularity_scale_difference="
            << regularity_forward.solve.regularity_scale_relative_difference
            << " regularity_state_difference="
            << regularity_state_difference << '\n';
  return constructive && stopped_fail_closed
      && cache_fail_closed_then_fallback && regularity_observer_only ? 0 : 1;
}
