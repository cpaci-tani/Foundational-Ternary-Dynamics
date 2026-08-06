// Engineering equivalence gate for the observer-only repeated-root cache.

#include "ftd/eft/connected_moore_block_action.h"

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

bool equivalent(const ftd::eft::ConnectedMooreBlockStepResult& cached,
                const ftd::eft::ConnectedMooreBlockStepResult& direct,
                bool compare_later) {
  if (!cached.valid || !direct.valid || !cached.common_action_gates_pass
      || !direct.common_action_gates_pass
      || common_residual(cached) > 1e-10
      || common_residual(direct) > 1e-10) return false;
  const auto& lhs = compare_later ? cached.later : cached.earlier;
  const auto& rhs = compare_later ? direct.later : direct.earlier;
  return ftd::eft::connected_moore_block_state_max_difference(lhs,rhs)
      <= 1e-9;
}

}  // namespace

int main() {
  const auto initial = ftd::eft::initialize_connected_moore_block(
      17,2,0,0,0.25,1e-13,4096);
  if (!initial.valid) return 1;
  ftd::eft::ConnectedMooreBlockOptions options;
  ftd::eft::ConnectedMooreBlockSolveCache forward_cache,reverse_cache;

  const auto cached1 = ftd::eft::solve_connected_moore_block_forward(
      initial.state,options,&forward_cache);
  const auto direct1 = ftd::eft::solve_connected_moore_block_forward(
      initial.state,options);
  if (!equivalent(cached1,direct1,true)
      || cached1.solve.jacobian_refreshes < 1) return 1;

  const auto cached2 = ftd::eft::solve_connected_moore_block_forward(
      cached1.later,options,&forward_cache);
  const auto direct2 = ftd::eft::solve_connected_moore_block_forward(
      cached1.later,options);
  if (!equivalent(cached2,direct2,true)
      || cached2.solve.jacobian_reuses < 1) return 1;

  const auto cached_reverse1 = ftd::eft::solve_connected_moore_block_reverse(
      cached2.later,options,&reverse_cache);
  const auto direct_reverse1 = ftd::eft::solve_connected_moore_block_reverse(
      cached2.later,options);
  if (!equivalent(cached_reverse1,direct_reverse1,false)
      || cached_reverse1.solve.jacobian_refreshes < 1) return 1;

  const auto cached_reverse2 = ftd::eft::solve_connected_moore_block_reverse(
      cached_reverse1.earlier,options,&reverse_cache);
  const auto direct_reverse2 = ftd::eft::solve_connected_moore_block_reverse(
      cached_reverse1.earlier,options);
  if (!equivalent(cached_reverse2,direct_reverse2,false)
      || cached_reverse2.solve.jacobian_reuses < 1) return 1;

  const double recovery = ftd::eft::connected_moore_block_state_max_difference(
      initial.state,cached_reverse2.earlier);
  std::cout << "cached/direct repeated-root equivalence passes; recovery="
            << recovery << '\n';
  return recovery <= 1e-9 ? 0 : 1;
}
