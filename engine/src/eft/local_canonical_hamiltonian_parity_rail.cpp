#include "ftd/eft/local_canonical_hamiltonian_parity_rail.h"

#include <algorithm>
#include <cmath>

namespace ftd::eft {
namespace {

bool close(double first, double second, double tolerance) {
  return std::abs(first - second)
      <= tolerance * std::max({1.0, std::abs(first), std::abs(second)});
}

bool finite_site(const CanonicalParityRailSite& site) {
  return std::isfinite(site.q) && std::isfinite(site.p);
}

bool gate_zero(double phase, double tolerance) {
  const double two_pi = 2.0 * std::acos(-1.0);
  return std::abs(std::remainder(phase, two_pi)) <= tolerance;
}

double site_norm(const CanonicalParityRailSite& site) {
  return 0.5 * (site.q * site.q + site.p * site.p);
}

double total_norm(const std::vector<CanonicalParityRailSite>& sites) {
  double value = 0.0;
  for (const auto& site : sites) value += site_norm(site);
  return value;
}

double bond_generator(
    const CanonicalParityRailSite& left,
    const CanonicalParityRailSite& right) {
  return left.q * right.p - right.q * left.p;
}

bool matching_is_disjoint(
    const std::vector<std::pair<std::size_t, std::size_t>>& bonds,
    std::size_t length) {
  std::vector<bool> used(length, false);
  for (const auto& [left, right] : bonds) {
    if (right != left + 1 || right >= length || used[left] || used[right]) {
      return false;
    }
    used[left] = true;
    used[right] = true;
  }
  return true;
}

std::vector<CanonicalParityRailSite> apply_endpoint(
    const std::vector<CanonicalParityRailSite>& sites,
    const std::vector<std::pair<std::size_t, std::size_t>>& bonds,
    TernaryQuarterTurnOrientation orientation) {
  auto result = sites;
  for (const auto& [left, right] : bonds) {
    if (orientation == TernaryQuarterTurnOrientation::Forward) {
      result[left] = {-sites[right].q, -sites[right].p};
      result[right] = {sites[left].q, sites[left].p};
    } else {
      result[left] = {sites[right].q, sites[right].p};
      result[right] = {-sites[left].q, -sites[left].p};
    }
  }
  return result;
}

bool decode_actual_section(
    const std::vector<CanonicalParityRailSite>& sites,
    double amplitude,
    double tolerance,
    std::vector<std::int8_t>& labels) {
  labels.clear();
  labels.reserve(sites.size());
  for (const auto& site : sites) {
    if (!close(site.p, 0.0, tolerance)) return false;
    const double scaled = site.q / amplitude;
    const double rounded = std::round(scaled);
    if (!close(scaled, rounded, tolerance)
        || rounded < -1.0 || rounded > 1.0) {
      return false;
    }
    labels.push_back(static_cast<std::int8_t>(rounded));
  }
  return true;
}

bool finite_result(const LocalCanonicalHamiltonianParityRailResult& result) {
  const bool sites_finite = std::all_of(
      result.after.begin(), result.after.end(), finite_site);
  return sites_finite
      && std::isfinite(result.cycle_duration)
      && std::isfinite(result.spatial_pulse_angle)
      && std::isfinite(result.carrier_norm_before)
      && std::isfinite(result.carrier_norm_after)
      && std::isfinite(result.oriented_bond_generator)
      && std::isfinite(result.carrier_energy_before)
      && std::isfinite(result.carrier_energy_after)
      && std::isfinite(result.maximum_clock_action_excursion)
      && std::isfinite(result.maximum_reference_energy_exchange)
      && std::isfinite(result.endpoint_energy_residual);
}

}  // namespace

int scalar_boundary_global_symplectic_entry(
    std::size_t rail_length,
    std::size_t row,
    std::size_t column) {
  if (rail_length == 0 || (rail_length & 1U) != 0
      || row >= rail_length || column >= rail_length
      || row + column != rail_length - 1 || row == column) {
    return 0;
  }
  const std::size_t half = rail_length / 2;
  if (row < half) {
    return ((half + row + 1) & 1U) == 0 ? 1 : -1;
  }
  return -scalar_boundary_global_symplectic_entry(
      rail_length, column, row);
}

LocalCanonicalHamiltonianParityRailResult
evolve_local_canonical_hamiltonian_parity_rail_cycle(
    const LocalCanonicalHamiltonianParityRailInput& input) {
  LocalCanonicalHamiltonianParityRailResult result;
  result.global_tick = input.global_tick;
  result.orientation = input.orientation;
  result.before = input.sites;
  result.scalar_common_form_boundary_global = true;
  result.minimum_registered_local_carrier_dimension_per_site = 2;
  result.common_harmonic_clock_selected = true;
  result.continuous_subtick_ontology_claimed = false;
  result.native_doublet_formation_supplied = false;
  result.native_record_energy_scale_derived = false;
  result.production_coupling_supplied = false;
  result.native_gstar_synchronization_supplied = false;

  if (input.sites.size() < 2) {
    result.status = LocalCanonicalHamiltonianParityRailStatus::InvalidRailLength;
    return result;
  }
  if (!std::all_of(input.sites.begin(), input.sites.end(), finite_site)) {
    result.status = LocalCanonicalHamiltonianParityRailStatus::InvalidSiteState;
    return result;
  }
  if (!std::isfinite(input.ternary_amplitude)
      || !(input.ternary_amplitude > 0.0)) {
    result.status = LocalCanonicalHamiltonianParityRailStatus::InvalidAmplitude;
    return result;
  }
  if (!std::isfinite(input.clock_frequency)
      || !(input.clock_frequency > 0.0)) {
    result.status =
        LocalCanonicalHamiltonianParityRailStatus::InvalidClockFrequency;
    return result;
  }
  if (!std::isfinite(input.reference_action)
      || !(input.reference_action > 0.0)) {
    result.status =
        LocalCanonicalHamiltonianParityRailStatus::InvalidReferenceAction;
    return result;
  }
  if (!std::isfinite(input.tolerance) || input.tolerance < 0.0) {
    result.status = LocalCanonicalHamiltonianParityRailStatus::InvalidTolerance;
    return result;
  }
  if (!std::isfinite(input.reference_phase)
      || !gate_zero(input.reference_phase, input.tolerance)) {
    result.status =
        LocalCanonicalHamiltonianParityRailStatus::InvalidReferencePhase;
    return result;
  }

  result.active_bonds = alternating_oriented_ternary_parity_matching(
      input.sites.size(), input.global_tick);
  result.disjoint_matching = matching_is_disjoint(
      result.active_bonds, input.sites.size());
  result.carrier_norm_before = total_norm(input.sites);
  for (const auto& [left, right] : result.active_bonds) {
    result.oriented_bond_generator += bond_generator(
        input.sites[left], input.sites[right]);
  }
  result.maximum_clock_action_excursion =
      0.5 * std::abs(result.oriented_bond_generator);
  if (!(input.reference_action
        > result.maximum_clock_action_excursion + input.tolerance)) {
    result.status =
        LocalCanonicalHamiltonianParityRailStatus::InsufficientReferenceReserve;
    return result;
  }

  const double pi = std::acos(-1.0);
  const double orientation_sign =
      input.orientation == TernaryQuarterTurnOrientation::Forward ? 1.0 : -1.0;
  result.cycle_duration = 2.0 * pi / input.clock_frequency;
  result.spatial_pulse_angle = orientation_sign * 0.5 * pi;
  result.after = apply_endpoint(
      input.sites, result.active_bonds, input.orientation);
  const auto inverse_orientation =
      input.orientation == TernaryQuarterTurnOrientation::Forward
      ? TernaryQuarterTurnOrientation::Reverse
      : TernaryQuarterTurnOrientation::Forward;
  result.recovered = apply_endpoint(
      result.after, result.active_bonds, inverse_orientation);
  result.exact_inverse_verified = true;
  for (std::size_t index = 0; index < input.sites.size(); ++index) {
    result.exact_inverse_verified &=
        close(result.recovered[index].q, input.sites[index].q, input.tolerance)
        && close(result.recovered[index].p, input.sites[index].p, input.tolerance);
  }

  result.carrier_norm_after = total_norm(result.after);
  result.carrier_norm_preserved = close(
      result.carrier_norm_before,
      result.carrier_norm_after,
      input.tolerance);
  result.imposed_record_energy_scale = 0.5 * input.clock_frequency
      * input.ternary_amplitude * input.ternary_amplitude;
  result.carrier_energy_before =
      input.clock_frequency * result.carrier_norm_before;
  result.carrier_energy_after =
      input.clock_frequency * result.carrier_norm_after;
  result.carrier_plus_interaction_lower_bound =
      0.5 * input.clock_frequency * result.carrier_norm_before;
  result.total_carrier_energy_conserved = close(
      result.carrier_energy_before,
      result.carrier_energy_after,
      input.tolerance);
  result.positive_carrier_interaction_bound =
      result.carrier_plus_interaction_lower_bound >= 0.0;

  result.local_bond_energy_conserved = true;
  result.local_antisymmetric_current_supplied = true;
  for (const auto& [left, right] : result.active_bonds) {
    CanonicalParityRailBondLedger ledger;
    ledger.left = left;
    ledger.right = right;
    ledger.oriented_generator = bond_generator(
        input.sites[left], input.sites[right]);
    ledger.left_energy_before = input.clock_frequency
        * site_norm(input.sites[left]);
    ledger.right_energy_before = input.clock_frequency
        * site_norm(input.sites[right]);
    ledger.left_energy_after = input.clock_frequency
        * site_norm(result.after[left]);
    ledger.right_energy_after = input.clock_frequency
        * site_norm(result.after[right]);
    ledger.integrated_current_left_to_right =
        ledger.right_energy_after - ledger.right_energy_before;
    ledger.bond_energy_conserved = close(
        ledger.left_energy_before + ledger.right_energy_before,
        ledger.left_energy_after + ledger.right_energy_after,
        input.tolerance);
    result.local_bond_energy_conserved &= ledger.bond_energy_conserved;
    result.local_antisymmetric_current_supplied &= close(
        ledger.left_energy_after - ledger.left_energy_before,
        -ledger.integrated_current_left_to_right,
        input.tolerance);
    result.bond_ledgers.push_back(ledger);
  }

  result.reference_action_before = input.reference_action;
  const double signed_antiphase_action =
      0.5 * orientation_sign * result.oriented_bond_generator;
  result.minimum_reference_action = std::min(
      input.reference_action,
      input.reference_action - signed_antiphase_action);
  result.maximum_reference_action = std::max(
      input.reference_action,
      input.reference_action - signed_antiphase_action);
  result.reference_action_after = input.reference_action;
  result.maximum_reference_energy_exchange = input.clock_frequency
      * result.maximum_clock_action_excursion;
  result.maximum_interaction_energy_magnitude =
      result.maximum_reference_energy_exchange;
  result.gate_zero_switch_work = 0.0;
  result.antiphase_switch_work_magnitude =
      result.maximum_interaction_energy_magnitude;
  result.endpoint_total_energy_before = input.clock_frequency
      * input.reference_action + result.carrier_energy_before;
  result.endpoint_total_energy_after = input.clock_frequency
      * result.reference_action_after + result.carrier_energy_after;
  result.endpoint_energy_residual =
      result.endpoint_total_energy_after - result.endpoint_total_energy_before;

  result.actual_ternary_section = decode_actual_section(
      input.sites,
      input.ternary_amplitude,
      input.tolerance,
      result.actual_labels_before);
  result.actual_section_returns_to_section = result.actual_ternary_section
      && decode_actual_section(
          result.after,
          input.ternary_amplitude,
          input.tolerance,
          result.actual_labels_after);
  if (result.actual_section_returns_to_section) {
    const auto discrete = input.orientation
        == TernaryQuarterTurnOrientation::Forward
        ? step_alternating_oriented_ternary_parity_rail(
              result.actual_labels_before, input.global_tick)
        : reverse_alternating_oriented_ternary_parity_rail(
              result.actual_labels_before, input.global_tick);
    result.actual_section_matches_discrete_rail =
        discrete.valid() && discrete.after == result.actual_labels_after;
  }
  result.actual_section_clock_backreaction_zero =
      result.actual_ternary_section
      && close(result.oriented_bond_generator, 0.0, input.tolerance)
      && close(result.maximum_clock_action_excursion, 0.0, input.tolerance);
  result.nontrivial_transport_with_zero_generator_value =
      result.actual_section_clock_backreaction_zero
      && !result.active_bonds.empty()
      && !std::equal(
          result.before.begin(), result.before.end(), result.after.begin(),
          [&](const CanonicalParityRailSite& first,
              const CanonicalParityRailSite& second) {
            return close(first.q, second.q, input.tolerance)
                && close(first.p, second.p, input.tolerance);
          });

  if (!finite_result(result)) {
    result.status = LocalCanonicalHamiltonianParityRailStatus::NonFiniteOutput;
    return result;
  }
  result.status = LocalCanonicalHamiltonianParityRailStatus::Valid;
  return result;
}

}  // namespace ftd::eft
