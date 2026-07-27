#include "ftd/eft/genesis_reservoir_dilation.h"

#include "ftd/eft/finite_memory_reversible_lift.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <vector>

namespace ftd::eft {

namespace {

constexpr double gate = 1e-12;

bool finite_vec(const Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z);
}

double max_abs(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

Vec3 normalized(Vec3 value) {
  const double magnitude = value.mag();
  if (!(magnitude > 0.0)) return {};
  return value * (1.0 / magnitude);
}

double state_difference(const GenesisEventState& lhs,
                        const GenesisEventState& rhs) {
  return std::max({
      static_cast<double>(std::abs(lhs.state - rhs.state)),
      max_abs(lhs.flux - rhs.flux),
      max_abs(lhs.wave - rhs.wave),
      static_cast<double>(std::abs(lhs.particle_id - rhs.particle_id)),
      static_cast<double>(std::abs(lhs.spin - rhs.spin)),
      static_cast<double>(std::abs(lhs.color - rhs.color))});
}

}  // namespace

double genesis_field_withdrawal(
    double genesis_threshold,
    double excess,
    double wave_magnitude_squared,
    double kinetic_drain) {
  return genesis_threshold * excess
      + 0.5 * genesis_threshold * genesis_threshold
      + (kinetic_drain - 0.5 * kinetic_drain * kinetic_drain)
          * wave_magnitude_squared;
}

GenesisMapResult accepted_single_genesis(
    const GenesisEventState& before,
    std::int8_t polarity,
    std::int8_t manifested_spin,
    double genesis_threshold,
    double manifestation_scale,
    double kinetic_drain) {
  GenesisMapResult result;
  const double magnitude = before.flux.mag();
  if (before.state != 0 || (polarity != -1 && polarity != 1)
      || (manifested_spin != -1 && manifested_spin != 1)
      || !(genesis_threshold > 0.0) || !(manifestation_scale > 0.0)
      || !(magnitude > genesis_threshold)
      || kinetic_drain < 0.0 || kinetic_drain > 1.0
      || !finite_vec(before.flux) || !finite_vec(before.wave)) {
    return result;
  }

  result.state = before;
  result.excess = magnitude - genesis_threshold;
  result.acceptance_probability =
      1.0 - std::exp(-result.excess / manifestation_scale);
  result.state.state = polarity;
  result.state.flux *= 1.0 - genesis_threshold / magnitude;
  result.state.wave *= 1.0 - kinetic_drain;
  result.state.particle_id = -2;
  result.state.spin = manifested_spin;
  const double ax = std::abs(result.state.flux.x);
  const double ay = std::abs(result.state.flux.y);
  const double az = std::abs(result.state.flux.z);
  result.state.color = static_cast<std::int8_t>(
      ax >= ay && ax >= az ? 1 : (ay >= ax && ay >= az ? 2 : 3));
  result.field_withdrawal = genesis_field_withdrawal(
      genesis_threshold, result.excess, before.wave.mag2(), kinetic_drain);
  result.valid = result.acceptance_probability > 0.0
      && result.acceptance_probability < 1.0;
  return result;
}

GenesisMapResult invert_accepted_single_genesis(
    const GenesisEventState& after,
    double genesis_threshold,
    double manifestation_scale,
    double kinetic_drain) {
  GenesisMapResult result;
  const double residual_magnitude = after.flux.mag();
  if ((after.state != -1 && after.state != 1)
      || !(genesis_threshold > 0.0) || !(manifestation_scale > 0.0)
      || !(residual_magnitude > 0.0)
      || kinetic_drain < 0.0 || !(kinetic_drain < 1.0)
      || !finite_vec(after.flux) || !finite_vec(after.wave)) {
    return result;
  }

  result.state = after;
  result.state.state = 0;
  result.state.flux *= 1.0 + genesis_threshold / residual_magnitude;
  result.state.wave *= 1.0 / (1.0 - kinetic_drain);
  result.state.particle_id = -1;
  result.state.spin = 0;
  result.state.color = 0;
  result.excess = residual_magnitude;
  result.acceptance_probability =
      1.0 - std::exp(-result.excess / manifestation_scale);
  result.field_withdrawal = genesis_field_withdrawal(
      genesis_threshold, result.excess, result.state.wave.mag2(),
      kinetic_drain);
  result.valid = true;
  return result;
}

GenesisEventState projected_evaporation(const GenesisEventState& before) {
  GenesisEventState after = before;
  after.state = 0;
  after.particle_id = -1;
  after.spin = 0;
  after.color = 0;
  return after;
}

double recover_bernoulli_phase(
    int branch, double future_phase, double probability) {
  if ((branch != 0 && branch != 1) || !(probability > 0.0)
      || !(probability < 1.0) || !(future_phase >= 0.0)
      || !(future_phase < 1.0)) {
    return std::numeric_limits<double>::quiet_NaN();
  }
  return branch == 1
      ? probability * future_phase
      : probability + (1.0 - probability) * future_phase;
}

BernoulliDilationResult dilate_bernoulli_phase(
    double phase, double probability) {
  BernoulliDilationResult result;
  result.probability = probability;
  result.phase_before = phase;
  if (!(phase >= 0.0) || !(phase < 1.0)
      || !(probability > 0.0) || !(probability < 1.0)) {
    return result;
  }
  if (phase < probability) {
    result.branch = 1;
    result.phase_after = phase / probability;
  } else {
    result.branch = 0;
    result.phase_after = (phase - probability) / (1.0 - probability);
  }
  result.recovered_phase = recover_bernoulli_phase(
      result.branch, result.phase_after, probability);
  result.valid = result.phase_after >= 0.0 && result.phase_after < 1.0
      && std::isfinite(result.recovered_phase);
  return result;
}

GenesisReservoirDilationResult analyze_genesis_reservoir_dilation() {
  GenesisReservoirDilationResult result;
  constexpr double kg = 1.0;
  constexpr double km = 1.0;
  const double inv_sqrt3 = 1.0 / std::sqrt(3.0);
  const std::array<Vec3, 10> directions{{
      {1.0, 0.0, 0.0}, {-1.0, 0.0, 0.0},
      {0.0, 1.0, 0.0}, {0.0, -1.0, 0.0},
      {0.0, 0.0, 1.0}, {0.0, 0.0, -1.0},
      {inv_sqrt3, inv_sqrt3, inv_sqrt3},
      {-inv_sqrt3, inv_sqrt3, inv_sqrt3},
      {inv_sqrt3, -inv_sqrt3, inv_sqrt3},
      {inv_sqrt3, inv_sqrt3, -inv_sqrt3}}};
  const std::array<double, 3> excesses{{0.125, 0.5, 1.25}};
  const std::array<Vec3, 3> waves{{
      {0.0, 0.0, 0.0}, {0.3, -0.4, 0.2}, {-0.25, 0.1, 0.5}}};
  const std::array<double, 3> drains{{0.0, 0.5, 0.9}};
  const std::array<std::int8_t, 2> polarities{{-1, 1}};

  bool inverse_ok = true;
  bool evaporation_not_inverse = true;
  bool detailed_balance_fails = true;
  double min_composition_distance = std::numeric_limits<double>::infinity();
  double minimum_withdrawal = std::numeric_limits<double>::infinity();
  double maximum_withdrawal = -std::numeric_limits<double>::infinity();

  for (const Vec3& raw_direction : directions) {
    const Vec3 direction = normalized(raw_direction);
    for (double excess : excesses) {
      for (const Vec3& wave : waves) {
        for (double drain : drains) {
          for (std::int8_t polarity : polarities) {
            GenesisEventState before;
            before.flux = direction * (kg + excess);
            before.wave = wave;
            const auto forward = accepted_single_genesis(
                before, polarity, polarity, kg, km, drain);
            const auto inverse = invert_accepted_single_genesis(
                forward.state, kg, km, drain);
            ++result.accepted_single_arms;
            inverse_ok = inverse_ok && forward.valid && inverse.valid;
            if (forward.valid && inverse.valid) {
              result.maximum_genesis_inverse_residual = std::max(
                  result.maximum_genesis_inverse_residual,
                  state_difference(before, inverse.state));
            }

            const double measured_withdrawal = 0.5 * (
                before.flux.mag2() + before.wave.mag2()
                - forward.state.flux.mag2() - forward.state.wave.mag2());
            result.maximum_withdrawal_residual = std::max(
                result.maximum_withdrawal_residual,
                std::abs(measured_withdrawal - forward.field_withdrawal));
            minimum_withdrawal = std::min(
                minimum_withdrawal, forward.field_withdrawal);
            maximum_withdrawal = std::max(
                maximum_withdrawal, forward.field_withdrawal);

            const auto evaporated = projected_evaporation(forward.state);
            const double flux_distance =
                (before.flux - evaporated.flux).mag();
            const double wave_distance =
                (before.wave - evaporated.wave).mag();
            min_composition_distance = std::min(
                min_composition_distance, flux_distance);
            result.maximum_evaporation_flux_distance_residual = std::max(
                result.maximum_evaporation_flux_distance_residual,
                std::abs(flux_distance - kg));
            result.maximum_evaporation_wave_distance_residual = std::max(
                result.maximum_evaporation_wave_distance_residual,
                std::abs(wave_distance - drain * wave.mag()));
            evaporation_not_inverse = evaporation_not_inverse
                && state_difference(before, evaporated) > 0.0;
            detailed_balance_fails = detailed_balance_fails
                && forward.acceptance_probability > 0.0;
          }
        }
      }
    }
  }

  for (std::size_t i = 1; i < excesses.size(); ++i) {
    const double d0 = genesis_field_withdrawal(kg, excesses[i - 1], 0.0, 0.0);
    const double d1 = genesis_field_withdrawal(kg, excesses[i], 0.0, 0.0);
    const double slope = (d1 - d0) / (excesses[i] - excesses[i - 1]);
    result.maximum_withdrawal_slope_residual = std::max(
        result.maximum_withdrawal_slope_residual, std::abs(slope - kg));
  }
  result.withdrawal_span = maximum_withdrawal - minimum_withdrawal;
  result.minimum_evaporation_composition_flux_distance =
      min_composition_distance;

  GenesisEventState collision_a;
  collision_a.flux = {kg + 0.5, 0.0, 0.0};
  collision_a.wave = {0.25, -0.5, 0.75};
  GenesisEventState collision_b = collision_a;
  collision_b.wave = {-0.4, 0.1, 0.2};
  const auto unit_a = accepted_single_genesis(
      collision_a, 1, 1, kg, km, 1.0);
  const auto unit_b = accepted_single_genesis(
      collision_b, 1, 1, kg, km, 1.0);
  result.unit_drain_has_wave_collision = unit_a.valid && unit_b.valid
      && max_abs(unit_a.state.flux - unit_b.state.flux) <= gate
      && max_abs(unit_a.state.wave) == 0.0
      && max_abs(unit_b.state.wave) == 0.0
      && max_abs(collision_a.wave - collision_b.wave) > 0.0;

  const std::array<double, 4> probabilities{{
      1.0 - std::exp(-excesses[0] / km),
      1.0 - std::exp(-excesses[1] / km),
      1.0 - std::exp(-excesses[2] / km), 0.5}};
  bool bernoulli_ok = true;
  for (double probability : probabilities) {
    const std::array<double, 4> phases{{
        0.25 * probability,
        0.75 * probability,
        probability + 0.25 * (1.0 - probability),
        probability + 0.75 * (1.0 - probability)}};
    for (std::size_t i = 0; i < phases.size(); ++i) {
      const auto dilation = dilate_bernoulli_phase(phases[i], probability);
      ++result.bernoulli_arms;
      const int expected_branch = i < 2 ? 1 : 0;
      bernoulli_ok = bernoulli_ok && dilation.valid
          && dilation.branch == expected_branch;
      result.maximum_bernoulli_inverse_residual = std::max(
          result.maximum_bernoulli_inverse_residual,
          std::abs(dilation.recovered_phase - phases[i]));
    }
  }

  constexpr int depth = 20;
  double phase = 0.3141592653589793;
  std::array<int, depth> branches{};
  std::array<double, depth> sequence_probabilities{};
  bool sequence_ok = true;
  for (int i = 0; i < depth; ++i) {
    const double probability = probabilities[static_cast<std::size_t>(i) %
        probabilities.size()];
    sequence_probabilities[static_cast<std::size_t>(i)] = probability;
    const auto dilation = dilate_bernoulli_phase(phase, probability);
    sequence_ok = sequence_ok && dilation.valid;
    branches[static_cast<std::size_t>(i)] = dilation.branch;
    phase = dilation.phase_after;
  }
  for (int i = depth; i-- > 0;) {
    phase = recover_bernoulli_phase(
        branches[static_cast<std::size_t>(i)], phase,
        sequence_probabilities[static_cast<std::size_t>(i)]);
  }
  result.history_depth = depth;
  result.erased_preimages_at_depth = std::uint64_t{1} << depth;
  result.minimum_history_bits_at_depth = minimum_history_bits(2, depth);
  result.maximum_history_inverse_residual =
      std::abs(phase - 0.3141592653589793);
  bool history_counts_ok = true;
  for (int n = 1; n <= depth; ++n) {
    history_counts_ok = history_counts_ok
        && (std::uint64_t{1} << n) == static_cast<std::uint64_t>(std::pow(2.0, n))
        && minimum_history_bits(2, static_cast<std::uint64_t>(n))
            == static_cast<std::uint64_t>(n);
  }

  result.accepted_genesis_conditionally_invertible = inverse_ok
      && result.accepted_single_arms == 540
      && result.maximum_genesis_inverse_residual <= gate;
  result.one_step_bernoulli_dilation_exact = bernoulli_ok
      && result.bernoulli_arms == 16
      && result.maximum_bernoulli_inverse_residual <= 1e-15;
  result.erased_trials_require_unbounded_history = sequence_ok
      && history_counts_ok
      && result.maximum_history_inverse_residual <= 1e-15
      && result.minimum_history_bits_at_depth == depth;
  result.evaporation_is_not_genesis_inverse = evaporation_not_inverse
      && result.maximum_evaporation_flux_distance_residual <= gate
      && result.maximum_evaporation_wave_distance_residual <= gate
      && std::abs(min_composition_distance - kg) <= gate;
  result.production_pair_violates_detailed_balance = detailed_balance_fails
      && result.evaporation_is_not_genesis_inverse;
  result.continuous_energy_payload_required =
      result.maximum_withdrawal_residual <= gate
      && result.maximum_withdrawal_slope_residual <= gate
      && result.withdrawal_span > 0.0;
  result.dual_and_single_energy_exchange_differ = minimum_withdrawal > 0.0;
  result.finite_local_reversible_production_dilation =
      result.accepted_genesis_conditionally_invertible
      && result.one_step_bernoulli_dilation_exact
      && !result.unit_drain_has_wave_collision
      && !result.erased_trials_require_unbounded_history
      && !result.evaporation_is_not_genesis_inverse
      && !result.continuous_energy_payload_required;
  result.one_event_dilation_open_system_only =
      result.accepted_genesis_conditionally_invertible
      && result.unit_drain_has_wave_collision
      && result.one_step_bernoulli_dilation_exact
      && result.erased_trials_require_unbounded_history
      && result.evaporation_is_not_genesis_inverse
      && result.production_pair_violates_detailed_balance
      && result.continuous_energy_payload_required
      && result.dual_and_single_energy_exchange_differ
      && !result.finite_local_reversible_production_dilation;
  result.valid = result.one_event_dilation_open_system_only;
  return result;
}

}  // namespace ftd::eft
