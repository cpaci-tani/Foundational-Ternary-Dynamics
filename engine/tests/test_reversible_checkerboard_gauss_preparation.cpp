/** FTD-0881/0882 reversible checkerboard Gauss-preparation EFT verifier. */

#include "ftd/eft/gauss_record_canonical_reduction.h"
#include "ftd/eft/reversible_checkerboard_gauss_preparation.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

int checks = 0;
int failures = 0;

void check(const std::string& label, bool condition) {
  ++checks;
  if (!condition) {
    ++failures;
    std::cerr << "FAIL  " << label << '\n';
  }
}

bool close(double first, double second, double tolerance = 1e-9) {
  return std::abs(first - second)
      <= tolerance * std::max({1.0, std::abs(first), std::abs(second)});
}

double maximum_face_magnitude(const ftd::eft::MatchedFaceFlux& field) {
  double result = 0.0;
  for (std::size_t index = 0; index < field.x.size(); ++index) {
    result = std::max({result,
        std::abs(field.x[index]),
        std::abs(field.y[index]),
        std::abs(field.z[index])});
  }
  return result;
}

}  // namespace

int main() {
  using namespace ftd::eft;
  constexpr int L = 4;
  MatchedFaceFlux indexing(L);
  std::vector<int> ternary(indexing.x.size(), 0);
  ternary[static_cast<std::size_t>(indexing.index(0, 0, 0))] = +1;
  ternary[static_cast<std::size_t>(indexing.index(2, 2, 2))] = -1;
  std::vector<double> charge(ternary.begin(), ternary.end());

  const auto exact = make_static_ternary_gauss_record(
      L, ternary, 1.0, 1e-13);
  check("static matched reference valid", exact.valid());

  ReversibleCheckerboardGaussPreparation preparation(L, charge, 1e-12);
  check("even neutral preparation initializes", preparation.valid());
  const double initial_centered = preparation.centered_energy(exact.flux);
  check("initial centered energy is static field energy",
      close(initial_centered, quadratic_energy(exact.flux), 1e-10));

  std::vector<double> even_residual_norms;
  double prior_centered = initial_centered;
  for (int sweep = 0; sweep < 8; ++sweep) {
    const auto even = preparation.step_fresh_layer();
    check("fresh even layer valid and exactly active-projected",
        even.valid()
        && even.parity == 0
        && even.fresh_environment
        && even.active_affine_projection_exact
        && even.active_cells == charge.size() / 2
        && close(even.maximum_active_residual_after, 0.0, 1e-10));
    check("even layer is isolated target-blind local algebra",
        even.disjoint_checkerboard_support
        && even.six_face_local
        && !even.pseudoinverse_used
        && !even.born_target_used
        && !even.production_coupling_used
        && !even.new_selected_type_added);
    check("even layer energy/work ledger closes",
        even.exact_inverse_formula
        && close(even.energy_ledger_residual, 0.0, 1e-10));

    const auto odd = preparation.step_fresh_layer();
    check("fresh odd layer valid and exactly active-projected",
        odd.valid()
        && odd.parity == 1
        && odd.fresh_environment
        && odd.active_affine_projection_exact
        && odd.active_cells == charge.size() / 2
        && close(odd.maximum_active_residual_after, 0.0, 1e-10));
    check("odd layer energy/work ledger closes",
        odd.exact_inverse_formula
        && close(odd.energy_ledger_residual, 0.0, 1e-10));

    const double centered = preparation.centered_energy(exact.flux);
    check("retained history preserves centered energy",
        close(centered, prior_centered, 1e-9)
        && close(centered, initial_centered, 1e-9));
    prior_centered = centered;
    check("physical field history source balance closes",
        close(preparation.physical_balance_residual(), 0.0, 1e-9));
    even_residual_norms.push_back(checkerboard_gauss_residual_l2_squared(
        preparation.flux(), charge, 0));
  }

  bool contraction_bound = true;
  for (std::size_t index = 1; index < even_residual_norms.size(); ++index) {
    contraction_bound &= 81.0 * even_residual_norms[index]
        <= 16.0 * even_residual_norms[index - 1] + 1e-12;
  }
  check("L4 full-sweep residual obeys exact four-ninths norm bound",
      contraction_bound);
  check("finite sequence approaches but does not claim exact completion",
      preparation.maximum_gauss_residual() < 1e-2
      && preparation.maximum_gauss_residual() > 0.0
      && preparation.minimum_energy_record_is_limit()
      && !preparation.generic_fixed_finite_sweep_completion());
  check("history and scope prices remain explicit",
      preparation.history().size() == 16
      && preparation.history_energy() > 0.0
      && preparation.source_work() > 0.0
      && preparation.finite_history_reversible()
      && preparation.limiting_field_history_energy_equal()
      && preparation.environment_freshness_required()
      && !preparation.autonomous_environment_recycling_supplied()
      && !preparation.positive_source_reservoir_microdynamics_supplied()
      && !preparation.moving_source_continuity_supplied()
      && !preparation.production_coupling_supplied()
      && !preparation.native_gstar_synchronization_supplied()
      && !preparation.born_weights_used()
      && !preparation.new_selected_type_added());

  ReversibleCheckerboardGaussPreparation reversed = preparation;
  while (reversed.half_layer_count() > 0) {
    check("retained layer reverses exactly", reversed.reverse_last_layer());
  }
  check("complete retained history returns to empty field",
      maximum_face_magnitude(reversed.flux()) <= 1e-9
      && close(reversed.history_energy(), 0.0, 1e-9)
      && close(reversed.source_work(), 0.0, 1e-9)
      && close(reversed.physical_balance_residual(), 0.0, 1e-9));

  std::vector<double> incoming(charge.size(), 0.0);
  incoming[static_cast<std::size_t>(indexing.index(0, 0, 0))] = 0.25;
  MatchedFaceFlux nonfresh_flux(L);
  const auto nonfresh = apply_reversible_checkerboard_gauss_layer(
      nonfresh_flux, charge, 0, incoming, 1e-12);
  check("nonfresh port remains reversible but is not a projection",
      nonfresh.valid()
      && !nonfresh.fresh_environment
      && !nonfresh.active_affine_projection_exact
      && close(divergence_at(nonfresh_flux, 0, 0, 0) - 1.0, 0.25));
  std::vector<double> recovered_incoming;
  check("public inverse recovers nonfresh input and empty field",
      reverse_reversible_checkerboard_gauss_layer(
          nonfresh_flux, charge, nonfresh, &recovered_incoming, 1e-12)
          == ReversibleCheckerboardGaussStatus::Valid
      && maximum_face_magnitude(nonfresh_flux) <= 1e-10
      && close(
          recovered_incoming[static_cast<std::size_t>(
              indexing.index(0, 0, 0))],
          0.25));

  ReversibleCheckerboardGaussPreparation polarity_reverse(
      L,
      [&charge]() {
        auto result = charge;
        for (double& value : result) value = -value;
        return result;
      }(),
      1e-12);
  for (int layer = 0; layer < 16; ++layer) {
    polarity_reverse.step_fresh_layer();
  }
  auto polarity_sum = preparation.flux();
  for (std::size_t index = 0; index < polarity_sum.x.size(); ++index) {
    polarity_sum.x[index] += polarity_reverse.flux().x[index];
    polarity_sum.y[index] += polarity_reverse.flux().y[index];
    polarity_sum.z[index] += polarity_reverse.flux().z[index];
  }
  check("complete preparation is polarity covariant",
      maximum_face_magnitude(polarity_sum) <= 1e-10
      && close(preparation.history_energy(), polarity_reverse.history_energy())
      && close(preparation.source_work(), polarity_reverse.source_work()));

  check("small probe fails closed",
      ReversibleCheckerboardGaussPreparation(2, std::vector<double>(8, 0.0)).status()
          == ReversibleCheckerboardGaussStatus::InvalidSize);
  check("odd periodic probe fails closed",
      ReversibleCheckerboardGaussPreparation(5, std::vector<double>(125, 0.0)).status()
          == ReversibleCheckerboardGaussStatus::OddPeriodicSize);
  auto incompatible = charge;
  incompatible[0] += 1.0;
  check("nonneutral periodic charge fails closed",
      ReversibleCheckerboardGaussPreparation(L, incompatible).status()
          == ReversibleCheckerboardGaussStatus::IncompatibleCharge);
  auto nonfinite = charge;
  nonfinite[0] = std::numeric_limits<double>::infinity();
  check("nonfinite input fails closed",
      ReversibleCheckerboardGaussPreparation(L, nonfinite).status()
          == ReversibleCheckerboardGaussStatus::NonFiniteInput);

  std::cout << "FTD-0881/0882 reversible checkerboard Gauss preparation EFT: "
            << (checks - failures) << '/' << checks << " PASS\n";
  std::cout << "minimum_energy_record=ASYMPTOTIC_EXACT_LIMIT\n"
            << "finite_history=EXACTLY_REVERSIBLE\n"
            << "energy_split=FIELD_HALF_HISTORY_HALF_IN_LIMIT\n"
            << "production_gstar_born=OPEN_UNTOUCHED\n";
  return failures == 0 ? 0 : 1;
}
