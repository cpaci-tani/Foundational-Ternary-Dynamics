// Engineering equivalence gate for the Jacobian-free common-action solver.

#include "ftd/eft/connected_moore_block_action.h"
#include "ftd/eft/production_hop_kinematics.h"

#include <algorithm>
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

}  // namespace

int main() {
  const auto initialized = ftd::eft::initialize_connected_moore_block(
      9,1,0,0,0.125,1e-13,4096);
  if (!initialized.valid) return 1;
  auto initial = initialized.state;
  const ftd::Vec3 momentum =
      ftd::eft::production_flat_momentum({0.01,0.0,0.0});
  for (auto& point : initial.constituents) point.momentum = momentum;

  ftd::eft::ConnectedMooreBlockOptions dense_options;
  ftd::eft::ConnectedMooreBlockOptions matrix_free_options;
  matrix_free_options.use_matrix_free_newton_krylov = true;
  const auto dense = ftd::eft::solve_connected_moore_block_forward(
      initial,dense_options);
  const auto matrix_free = ftd::eft::solve_connected_moore_block_forward(
      initial,matrix_free_options);
  if (!dense.valid || !matrix_free.valid
      || !dense.common_action_gates_pass
      || !matrix_free.common_action_gates_pass
      || dense.solve.jacobian_refreshes < 1
      || matrix_free.solve.krylov_matvecs < 1
      || common_residual(dense) > 1e-10
      || common_residual(matrix_free) > 1e-10) return 1;

  const double forward_difference =
      ftd::eft::connected_moore_block_state_max_difference(
          dense.later,matrix_free.later);
  if (forward_difference > 1e-9) return 1;
  const auto reverse = ftd::eft::solve_connected_moore_block_reverse(
      matrix_free.later,matrix_free_options);
  if (!reverse.valid || !reverse.common_action_gates_pass
      || reverse.solve.krylov_matvecs < 1) return 1;
  const double recovery = ftd::eft::connected_moore_block_state_max_difference(
      initial,reverse.earlier);
  std::cout << "dense/matrix-free equivalence passes; forward_difference="
            << forward_difference << " recovery=" << recovery << '\n';
  return recovery <= 1e-9 ? 0 : 1;
}
