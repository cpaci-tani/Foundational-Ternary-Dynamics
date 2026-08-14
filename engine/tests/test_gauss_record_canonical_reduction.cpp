/** FTD-0877/0880 matched Gauss-record canonical-reduction verifier. */

#include "ftd/eft/gauss_record_canonical_reduction.h"

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

double max_difference(const ftd::eft::MatchedFaceFlux& first,
                      const ftd::eft::MatchedFaceFlux& second) {
  double result = 0.0;
  for (std::size_t index = 0; index < first.x.size(); ++index) {
    result = std::max({result,
        std::abs(first.x[index] - second.x[index]),
        std::abs(first.y[index] - second.y[index]),
        std::abs(first.z[index] - second.z[index])});
  }
  return result;
}

void add(ftd::eft::MatchedFaceFlux& target,
         const ftd::eft::MatchedFaceFlux& value,
         double scale = 1.0) {
  for (std::size_t index = 0; index < target.x.size(); ++index) {
    target.x[index] += scale * value.x[index];
    target.y[index] += scale * value.y[index];
    target.z[index] += scale * value.z[index];
  }
}

ftd::eft::MatchedFaceFlux generic_face(int L, int offset) {
  ftd::eft::MatchedFaceFlux field(L);
  for (std::size_t index = 0; index < field.x.size(); ++index) {
    field.x[index] = static_cast<double>(
        (3 * static_cast<int>(index) + offset) % 11 - 5) / 7.0;
    field.y[index] = static_cast<double>(
        (5 * static_cast<int>(index) + offset) % 13 - 6) / 9.0;
    field.z[index] = static_cast<double>(
        (7 * static_cast<int>(index) + offset) % 17 - 8) / 11.0;
  }
  return field;
}

}  // namespace

int main() {
  using namespace ftd::eft;
  constexpr int L = 5;
  constexpr double gate = 1e-8;

  auto flux = generic_face(L, 2);
  auto momentum = generic_face(L, 7);
  const auto decomposition = decompose_matched_gauss_canonical(
      flux, momentum, 1e-12);
  check("generic matched decomposition valid", decomposition.valid());
  check("charge bracket is identity on compatible space",
        decomposition.charge_bracket_identity_on_mean_zero_space);
  check("canonical split verified", decomposition.canonical_split_verified);
  check("flux reconstructs", decomposition.maximum_flux_reconstruction_residual <= gate);
  check("momentum reconstructs", decomposition.maximum_momentum_reconstruction_residual <= gate);
  check("transverse flux is divergence-free",
        decomposition.maximum_transverse_flux_divergence <= gate);
  check("transverse momentum is divergence-free",
        decomposition.maximum_transverse_momentum_divergence <= gate);
  check("longitudinal/transverse cross pairing vanishes",
        decomposition.longitudinal_transverse_pairing_residual <= gate);
  check("uniformly local conjugate is not claimed",
        !decomposition.uniformly_local_charge_conjugate_supplied);
  check("production projector is not used",
        !decomposition.production_gauss_projector_used);

  auto flux_two = generic_face(L, 4);
  auto momentum_two = generic_face(L, 9);
  const auto decomposition_two = decompose_matched_gauss_canonical(
      flux_two, momentum_two, 1e-12);
  const double full_pairing = gauss_canonical_symplectic_pairing(
      flux, momentum, flux_two, momentum_two);
  const double reduced_pairing = reduced_gauss_symplectic_pairing(
      decomposition, decomposition_two);
  check("second matched decomposition valid", decomposition_two.valid());
  check("full and reduced symplectic pairings agree",
        close(full_pairing, reduced_pairing, 1e-8));

  MatchedFaceFlux indexing(L);
  std::vector<int> state(indexing.x.size(), 0);
  state[static_cast<std::size_t>(indexing.index(1, 2, 3))] = +1;
  state[static_cast<std::size_t>(indexing.index(4, 1, 0))] = -1;
  const auto record = make_static_ternary_gauss_record(
      L, state, 1.0, 1e-12);
  check("neutral static ternary record valid", record.valid());
  check("neutral record needs no background", record.neutral_without_background
        && !record.background_subtracted && close(record.mean_state, 0.0));
  check("static record satisfies matched Gauss law",
        record.maximum_gauss_residual <= gate);
  check("static record has zero charge momentum",
        record.static_charge_momentum_zero && close(l1_norm(record.momentum), 0.0));
  check("static record is minimum-energy longitudinal",
        record.minimum_energy_longitudinal);
  check("dynamic native preparation remains open",
        !record.dynamic_native_preparation_supplied);
  check("G-star synchronization remains open",
        !record.native_gstar_synchronization_supplied);

  std::vector<int> reversed = state;
  for (int& value : reversed) value = -value;
  const auto reverse_record = make_static_ternary_gauss_record(
      L, reversed, 1.0, 1e-12);
  MatchedFaceFlux polarity_sum = record.flux;
  add(polarity_sum, reverse_record.flux);
  check("polarity-reversed record valid", reverse_record.valid());
  check("static section is polarity-covariant", l1_norm(polarity_sum) <= gate);

  std::vector<int> lone_state(indexing.x.size(), 0);
  lone_state[static_cast<std::size_t>(indexing.index(2, 2, 2))] = +1;
  const auto background_record = make_static_ternary_gauss_record(
      L, lone_state, 1.0, 1e-12);
  check("nonneutral periodic record carries mean background",
        background_record.valid() && !background_record.neutral_without_background
        && background_record.background_subtracted
        && close(background_record.mean_state, 1.0 / static_cast<double>(L * L * L)));

  std::vector<double> target(record.compatible_charge.begin(),
                             record.compatible_charge.end());
  const auto prepared = prepare_matched_gauss_record(
      flux, target, 1e-12);
  check("affine matched preparation valid", prepared.valid());
  check("prepared field reaches target charge", prepared.maximum_target_residual <= gate);
  check("affine preparation is idempotent", prepared.affine_projection_idempotent);
  check("discrepancy is longitudinal",
        prepared.maximum_discrepancy_curl_adjoint <= gate);
  check("discrepancy ledger recovers input",
        prepared.reversible_with_discrepancy_ledger
        && prepared.maximum_recovery_residual <= gate
        && max_difference(prepared.recovered, flux) <= gate);
  check("unledgered preparation is not promoted reversible",
        !prepared.reversible_without_discrepancy_ledger);
  check("environment dynamics is not supplied",
        !prepared.environment_dynamics_supplied);

  auto collided_input = flux;
  add(collided_input, prepared.discarded_longitudinal_discrepancy, 0.375);
  const auto collided = prepare_matched_gauss_record(
      collided_input, target, 1e-12);
  check("longitudinally distinct input is distinct",
        max_difference(collided_input, flux) > 1e-6);
  check("longitudinally distinct inputs collide at prepared record",
        collided.valid()
        && max_difference(collided.prepared, prepared.prepared) <= gate);

  auto transverse = matched_curl(make_transverse_challenge(L, 1e-3));
  auto record_evolved = record.flux;
  add(record_evolved, transverse, 0.25);
  check("matched curl update is exactly charge-preserving",
        max_gauss_residual(record_evolved, state) <= gate);

  const auto production = production_gauss_symbol_boundary();
  check("half-pi production symbols are the exact mismatch",
        close(production.central_composition_half_pi, -1.0)
        && close(production.sor_18_point_half_pi, -2.0));
  check("Nyquist production symbols are the exact mismatch",
        close(production.central_composition_nyquist, 0.0)
        && close(production.sor_18_point_nyquist, -4.0));
  check("production operators are not promoted matched",
        !production.operators_match && !production.exact_projector);
  check("production boundary records finite relaxation and source skip",
        production.finite_iteration_relaxation
        && production.default_manifested_site_skip);

  auto bad_momentum = momentum;
  bad_momentum.L = L + 1;
  check("shape mismatch fails closed",
        decompose_matched_gauss_canonical(flux, bad_momentum).status
            == GaussRecordReductionStatus::ShapeMismatch);
  auto bad_state = state;
  bad_state[0] = 2;
  check("nonternary record fails closed",
        make_static_ternary_gauss_record(L, bad_state).status
            == GaussRecordReductionStatus::InvalidTernaryState);
  auto nonfinite = flux;
  nonfinite.x[0] = std::numeric_limits<double>::infinity();
  check("nonfinite preparation fails closed",
        prepare_matched_gauss_record(nonfinite, target).status
            == GaussRecordReductionStatus::NonFiniteInput);
  auto incompatible = target;
  incompatible[0] += 1.0;
  check("nonzero-sum target fails closed",
        prepare_matched_gauss_record(flux, incompatible).status
            == GaussRecordReductionStatus::IncompatibleCharge);

  std::cout << "FTD-0877/0880 Gauss-record canonical reduction EFT: "
            << (checks - failures) << "/" << checks << " PASS\n";
  std::cout << "MATCHED_CHARGE_BRACKET=IDENTITY_ON_MEAN_ZERO_SPACE\n"
            << "STATIC_TERNARY_RECORD_SECTION=EXACT\n"
            << "UNIFORMLY_LOCAL_CHARGE_CONJUGATE=NO\n"
            << "PRODUCTION_GAUSS_EXACT_PROJECTOR=NO\n"
            << "REVERSIBLE_PREPARATION_REQUIRES=DISCREPANCY_LEDGER\n";
  return failures == 0 ? 0 : 1;
}
