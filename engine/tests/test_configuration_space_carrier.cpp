/** FTD-0584: fixed-source configuration-space carrier necessity gate. */

#include "ftd/eft/configuration_space_carrier.h"

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
  const auto r = ftd::eft::analyze_configuration_space_carrier();
  check("fixed-source/fixed-harmonic real fibres are affine contractible",
        r.fixed_source_fibres_affine_contractible
        && r.volumes == 4 && r.fibre_fixtures == 192
        && r.homotopy_samples == 960
        && r.maximum_gauss_residual <= 1e-12
        && r.maximum_harmonic_coordinate_residual <= 1e-12
        && r.maximum_affine_residual <= 1e-12
        && r.maximum_energy_polynomial_residual <= 1e-12
        && r.maximum_divergence_free_deformation <= 1e-12);
  check("uncontained compact-support and finite-energy real spaces contract",
        r.uncontained_finite_energy_space_contractible
        && r.uncontained_support_samples == 5
        && r.maximum_support_excess == 0);
  check("ternary snapshot components are not production conservation laws",
        r.snapshot_is_disjoint_union_of_contractible_fibres
        && !r.ternary_snapshot_disconnectedness_is_conservation
        && r.transition_rows == 9
        && r.registered_feature_rank == 4
        && r.registered_feature_nullity == 0
        && !r.registered_additive_transition_invariant_exists
        && !r.universal_transition_graph_invariant_excluded);
  check("frozen zero vacuum has no wall/string/point/texture homotopy",
        r.frozen_vacuum_is_single_point
        && r.vacuum_pi0_nontrivial == 0
        && r.vacuum_pi1_rank == 0
        && r.vacuum_pi2_rank == 0
        && r.vacuum_pi3_rank == 0
        && !r.normalized_direction_protected_while_zero_allowed);
  check("two-derivative static core shrinks; active nonlinear mode remains open",
        !r.two_derivative_static_core_size_stable
        && r.four_derivative_term_can_balance_scaling
        && !r.same_variable_active_localized_mode_excluded);
  check("compact phase needs admissibility and does not imply electric charge",
        !r.compact_u1_automatically_supplies_electric_charge
        && r.compact_flux_integer_requires_admissibility);
  check("observer changes no production state", !r.production_changed);
  check("registered FTD-0584 verdict closes", r.valid);

  std::cout.precision(17);
  std::cout << "volumes=" << r.volumes << '\n'
    << "fibre_fixtures=" << r.fibre_fixtures << '\n'
    << "homotopy_samples=" << r.homotopy_samples << '\n'
    << "uncontained_support_samples=" << r.uncontained_support_samples << '\n'
    << "transition_rows=" << r.transition_rows << '\n'
    << "registered_feature_rank=" << r.registered_feature_rank << '\n'
    << "registered_feature_nullity=" << r.registered_feature_nullity << '\n'
    << "maximum_gauss_residual=" << r.maximum_gauss_residual << '\n'
    << "maximum_harmonic_coordinate_residual="
    << r.maximum_harmonic_coordinate_residual << '\n'
    << "maximum_affine_residual=" << r.maximum_affine_residual << '\n'
    << "maximum_energy_polynomial_residual="
    << r.maximum_energy_polynomial_residual << '\n'
    << "maximum_divergence_free_deformation="
    << r.maximum_divergence_free_deformation << '\n'
    << "maximum_support_excess=" << r.maximum_support_excess << '\n'
    << "configuration_space_carrier failures=" << failures << '\n'
    << "verdict=CURRENT_FIXED_SOURCE_FIBRES_CONTRACTIBLE_"
       "CURRENT_VACUUM_HAS_NO_DEFECT_HOMOTOPY_"
       "STATIC_TWO_DERIVATIVE_CORE_UNSTABLE_"
       "MINIMUM_ENLARGEMENT_CLASSIFIED_NOT_DERIVED\n";
  return failures == 0 ? 0 : 1;
}
