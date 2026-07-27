/** FTD-0574: native field discrete action and source-operator audit. */

#include "ftd/eft/native_field_discrete_action.h"

#include <iostream>
#include <string>

namespace {

int failures = 0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

}  // namespace

int main() {
  const auto result = ftd::eft::analyze_native_field_discrete_action();

  check("36 production-mode arms execute", result.mode_arms == 36);
  check("four periodic lattice action arms execute",
        result.lattice_action_arms == 4);
  check("discrete Euler-Lagrange equation reproduces the tick",
        result.local_discrete_action_reproduces_tick
        && result.maximum_discrete_el_residual <= 1e-12);
  check("wave_vel is the left and right discrete Legendre momentum",
        result.wave_velocity_is_legendre_momentum
        && result.maximum_legendre_momentum_residual <= 1e-12);
  check("standard J-W pairing is preserved and native",
        result.standard_pairing_is_native
        && result.maximum_symplectic_residual <= 1e-12);
  check("quadratic invariant space has rank-two constraints and nullity one",
        result.invariant_constraint_rank == 2
        && result.invariant_constraint_nullity == 1);
  check("normalized exact tick invariant closes",
        result.normalized_tick_invariant_is_unique
        && result.maximum_tick_invariant_residual <= 1e-12
        && result.maximum_invariant_matrix_residual <= 1e-12);
  check("exact shadow Hamiltonian exponentiates to the production tick",
        result.maximum_shadow_flow_residual <= 1e-12);
  check("exact continuous shadow generator has no fixed finite-range symbol",
        result.exact_continuous_shadow_generator_is_nonlocal);
  check("four periodic source-operator arms execute",
        result.source_operator_arms == 4);
  check("central divergence and gradient are negative adjoints",
        result.maximum_electric_adjoint_residual <= 1e-12);
  check("central curl is self-adjoint",
        result.maximum_curl_adjoint_residual <= 1e-12);
  check("correct prescribed-source action differentiates to phase_read",
        result.prescribed_source_action_reproduces_phase_read
        && result.maximum_correct_source_action_residual <= 1e-12);
  check("documented onsite interaction differentiates to minus G_C s v",
        result.maximum_documented_action_derivative_residual <= 1e-12);
  check("prescribed-source map remains affine symplectic",
        result.prescribed_source_map_is_affine_symplectic
        && result.maximum_affine_source_symplectic_residual <= 1e-12);
  check("96 proper-cubic covariance arms close",
        result.proper_cubic_covariance_arms == 96
        && result.maximum_proper_cubic_covariance_residual <= 1e-12);
  check("eight uniform moving-source counterexamples execute",
        result.uniform_counterexample_arms == 8);
  check("uniform coded source vanishes",
        result.maximum_uniform_coded_source <= 1e-12);
  check("documented velocity interaction fails the uniform source",
        result.minimum_uniform_documented_source_mismatch > 1e-6
        && !result.documented_velocity_interaction_generates_coded_source);
  check("no full dynamic matter-field action is promoted",
        !result.full_dynamic_matter_field_action_derived
        && !result.production_changed);
  check("registered FTD-0574 verdict closes", result.valid);

  std::cout.precision(17);
  std::cout << "mode_arms=" << result.mode_arms << '\n'
            << "lattice_action_arms=" << result.lattice_action_arms << '\n'
            << "source_operator_arms=" << result.source_operator_arms << '\n'
            << "uniform_counterexample_arms="
            << result.uniform_counterexample_arms << '\n'
            << "proper_cubic_covariance_arms="
            << result.proper_cubic_covariance_arms << '\n'
            << "maximum_symplectic_residual="
            << result.maximum_symplectic_residual << '\n'
            << "maximum_discrete_el_residual="
            << result.maximum_discrete_el_residual << '\n'
            << "maximum_legendre_momentum_residual="
            << result.maximum_legendre_momentum_residual << '\n'
            << "maximum_tick_invariant_residual="
            << result.maximum_tick_invariant_residual << '\n'
            << "maximum_shadow_flow_residual="
            << result.maximum_shadow_flow_residual << '\n'
            << "maximum_electric_adjoint_residual="
            << result.maximum_electric_adjoint_residual << '\n'
            << "maximum_curl_adjoint_residual="
            << result.maximum_curl_adjoint_residual << '\n'
            << "maximum_correct_source_action_residual="
            << result.maximum_correct_source_action_residual << '\n'
            << "maximum_documented_action_derivative_residual="
            << result.maximum_documented_action_derivative_residual << '\n'
            << "maximum_proper_cubic_covariance_residual="
            << result.maximum_proper_cubic_covariance_residual << '\n'
            << "maximum_uniform_coded_source="
            << result.maximum_uniform_coded_source << '\n'
            << "minimum_uniform_documented_source_mismatch="
            << result.minimum_uniform_documented_source_mismatch << '\n'
            << "native_field_discrete_action failures=" << failures << '\n'
            << "verdict=NATIVE_FIELD_DISCRETE_ACTION_DERIVED_MAGNETIC_SOURCE_ACTION_MISMATCH\n";
  return failures == 0 ? 0 : 1;
}
