/** FTD-0875 isolated local canonical Hamiltonian parity-rail verifier. */

#include "ftd/eft/local_canonical_hamiltonian_parity_rail.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
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

bool close(double first, double second, double tolerance = 1e-11) {
  return std::abs(first - second)
      <= tolerance * std::max({1.0, std::abs(first), std::abs(second)});
}

std::vector<std::vector<std::int8_t>> ternary_states(std::size_t length) {
  std::vector<std::vector<std::int8_t>> states(1);
  for (std::size_t index = 0; index < length; ++index) {
    std::vector<std::vector<std::int8_t>> next;
    next.reserve(states.size() * 3);
    for (const auto& state : states) {
      for (std::int8_t value = -1; value <= 1; ++value) {
        auto extended = state;
        extended.push_back(value);
        next.push_back(std::move(extended));
      }
    }
    states = std::move(next);
  }
  return states;
}

ftd::eft::LocalCanonicalHamiltonianParityRailInput actual_input(
    const std::vector<std::int8_t>& labels,
    std::uint64_t tick,
    ftd::eft::TernaryQuarterTurnOrientation orientation) {
  ftd::eft::LocalCanonicalHamiltonianParityRailInput input;
  input.global_tick = tick;
  input.orientation = orientation;
  input.ternary_amplitude = 2.0;
  input.clock_frequency = 3.0;
  input.reference_action = 10.0;
  for (const auto label : labels) {
    input.sites.push_back({2.0 * label, 0.0});
  }
  return input;
}

}  // namespace

int main() {
  using namespace ftd::eft;

  for (std::size_t length = 2; length <= 6; ++length) {
    for (std::uint64_t tick = 0; tick < 2; ++tick) {
      for (const auto& labels : ternary_states(length)) {
        for (const auto orientation : {
                 TernaryQuarterTurnOrientation::Forward,
                 TernaryQuarterTurnOrientation::Reverse}) {
          const auto result =
              evolve_local_canonical_hamiltonian_parity_rail_cycle(
                  actual_input(labels, tick, orientation));
          check("actual-section cycle is valid", result.valid());
          check("actual section closes and matches discrete rail",
              result.actual_ternary_section
              && result.actual_section_returns_to_section
              && result.actual_section_matches_discrete_rail);
          check("canonical endpoint is exactly invertible",
              result.exact_inverse_verified);
          check("carrier energy and bond ledgers close",
              result.carrier_norm_preserved
              && result.total_carrier_energy_conserved
              && result.local_bond_energy_conserved
              && result.local_antisymmetric_current_supplied);
          check("actual section is the zero-backreaction special orbit",
              result.actual_section_clock_backreaction_zero);
          check("scope flags remain explicit",
              result.scalar_common_form_boundary_global
              && result.minimum_registered_local_carrier_dimension_per_site == 2
              && result.common_harmonic_clock_selected
              && !result.continuous_subtick_ontology_claimed
              && !result.native_doublet_formation_supplied
              && !result.native_record_energy_scale_derived
              && !result.production_coupling_supplied
              && !result.native_gstar_synchronization_supplied);
        }
      }
    }
  }

  auto ready = actual_input(
      {1, 0, 0, 0},
      0,
      TernaryQuarterTurnOrientation::Forward);
  const auto ready_result =
      evolve_local_canonical_hamiltonian_parity_rail_cycle(ready);
  check("ready record transfers all imposed energy",
      ready_result.valid()
      && close(ready_result.bond_ledgers[0].left_energy_after, 0.0)
      && close(
          ready_result.bond_ledgers[0].right_energy_after,
          ready_result.imposed_record_energy_scale)
      && close(
          ready_result.bond_ledgers[0].integrated_current_left_to_right,
          ready_result.imposed_record_energy_scale));
  check("ready record transports nontrivially at zero generator value",
      ready_result.nontrivial_transport_with_zero_generator_value);

  LocalCanonicalHamiltonianParityRailInput generic;
  generic.sites = {
      {1.0, 0.2},
      {0.5, -0.3},
      {-0.25, 0.4},
      {0.75, -0.1},
  };
  generic.clock_frequency = 2.0;
  generic.reference_action = 10.0;
  const auto generic_forward =
      evolve_local_canonical_hamiltonian_parity_rail_cycle(generic);
  check("generic phase-space cycle is valid",
      generic_forward.valid()
      && !generic_forward.actual_ternary_section
      && generic_forward.maximum_clock_action_excursion > 0.0);
  check("generic clock and endpoint ledgers close",
      close(
          generic_forward.maximum_reference_energy_exchange,
          generic_forward.maximum_interaction_energy_magnitude)
      && close(generic_forward.endpoint_energy_residual, 0.0)
      && generic_forward.minimum_reference_action > 0.0);
  check("positive carrier-interaction lower bound is explicit",
      generic_forward.positive_carrier_interaction_bound
      && generic_forward.carrier_plus_interaction_lower_bound > 0.0);

  auto inverse = generic;
  inverse.sites = generic_forward.after;
  inverse.orientation = TernaryQuarterTurnOrientation::Reverse;
  const auto generic_reverse =
      evolve_local_canonical_hamiltonian_parity_rail_cycle(inverse);
  bool recovered = generic_reverse.valid();
  for (std::size_t index = 0; index < generic.sites.size(); ++index) {
    recovered &= close(generic_reverse.after[index].q, generic.sites[index].q)
        && close(generic_reverse.after[index].p, generic.sites[index].p);
  }
  check("public reverse cycle recovers a generic continuous state", recovered);

  for (std::size_t length : {2U, 4U, 6U, 8U}) {
    std::vector<std::vector<int>> form(length, std::vector<int>(length, 0));
    for (std::size_t row = 0; row < length; ++row) {
      for (std::size_t column = 0; column < length; ++column) {
        form[row][column] = scalar_boundary_global_symplectic_entry(
            length, row, column);
      }
    }
    bool square_minus_identity = true;
    for (std::size_t row = 0; row < length; ++row) {
      for (std::size_t column = 0; column < length; ++column) {
        int value = 0;
        for (std::size_t inner = 0; inner < length; ++inner) {
          value += form[row][inner] * form[inner][column];
        }
        square_minus_identity &= value == (row == column ? -1 : 0);
      }
    }
    check("scalar common form is nondegenerate", square_minus_identity);
    check("scalar common form explicitly pairs endpoints",
        scalar_boundary_global_symplectic_entry(length, 0, length - 1) != 0);
  }
  check("odd scalar rail has no registered common form",
      scalar_boundary_global_symplectic_entry(5, 0, 4) == 0);

  auto low_reserve = generic;
  low_reserve.reference_action = 1e-6;
  check("insufficient reference reserve fails closed",
      evolve_local_canonical_hamiltonian_parity_rail_cycle(low_reserve).status
          == LocalCanonicalHamiltonianParityRailStatus::InsufficientReferenceReserve);
  auto off_phase = ready;
  off_phase.reference_phase = 0.25;
  check("off-phase request fails closed",
      evolve_local_canonical_hamiltonian_parity_rail_cycle(off_phase).status
          == LocalCanonicalHamiltonianParityRailStatus::InvalidReferencePhase);
  auto bad_amplitude = ready;
  bad_amplitude.ternary_amplitude = 0.0;
  check("zero amplitude fails closed",
      evolve_local_canonical_hamiltonian_parity_rail_cycle(bad_amplitude).status
          == LocalCanonicalHamiltonianParityRailStatus::InvalidAmplitude);
  auto bad_site = ready;
  bad_site.sites[0].q = std::numeric_limits<double>::infinity();
  check("nonfinite site fails closed",
      evolve_local_canonical_hamiltonian_parity_rail_cycle(bad_site).status
          == LocalCanonicalHamiltonianParityRailStatus::InvalidSiteState);

  std::cout << "FTD-0875 local canonical Hamiltonian parity rail EFT: "
            << (checks - failures) << '/' << checks << " PASS\n";
  std::cout << "minimum_local_carrier_dimension_per_site=2\n";
  std::cout << "scalar_common_form=BOUNDARY_GLOBAL\n";
  std::cout << "record_energy_current=EXACT_LOCAL_ANTISYMMETRIC\n";
  std::cout << "production_gstar=OPEN\n";
  return failures == 0 ? 0 : 1;
}
