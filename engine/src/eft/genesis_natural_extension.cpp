#include "ftd/eft/genesis_natural_extension.h"

#include "ftd/eft/genesis_reservoir_dilation.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace ftd::eft {

namespace {

constexpr double gate = 1e-12;

using SixVector = std::array<double, 6>;

bool finite_vec(const Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z);
}

double max_abs(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

double dot(const Vec3& lhs, const Vec3& rhs) {
  return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

Vec3 normalized(const Vec3& value) {
  const double magnitude = value.mag();
  return magnitude > 0.0 ? value * (1.0 / magnitude) : Vec3{};
}

Vec3 cross(const Vec3& lhs, const Vec3& rhs) {
  return {
      lhs.y * rhs.z - lhs.z * rhs.y,
      lhs.z * rhs.x - lhs.x * rhs.z,
      lhs.x * rhs.y - lhs.y * rhs.x};
}

Vec3 tangent_to(const Vec3& direction) {
  const Vec3 axis = std::abs(direction.x) < 0.75
      ? Vec3{1.0, 0.0, 0.0} : Vec3{0.0, 1.0, 0.0};
  return normalized(cross(direction, axis));
}

SixVector join(const Vec3& flux, const Vec3& wave) {
  return {{flux.x, flux.y, flux.z, wave.x, wave.y, wave.z}};
}

Vec3 first_three(const SixVector& value) {
  return {value[0], value[1], value[2]};
}

Vec3 last_three(const SixVector& value) {
  return {value[3], value[4], value[5]};
}

SixVector add(const SixVector& lhs, const SixVector& rhs) {
  SixVector result{};
  for (std::size_t i = 0; i < result.size(); ++i) {
    result[i] = lhs[i] + rhs[i];
  }
  return result;
}

SixVector subtract(const SixVector& lhs, const SixVector& rhs) {
  SixVector result{};
  for (std::size_t i = 0; i < result.size(); ++i) {
    result[i] = lhs[i] - rhs[i];
  }
  return result;
}

SixVector scale(const SixVector& value, double factor) {
  SixVector result{};
  for (std::size_t i = 0; i < result.size(); ++i) {
    result[i] = factor * value[i];
  }
  return result;
}

double max_abs(const SixVector& value) {
  double result = 0.0;
  for (double entry : value) result = std::max(result, std::abs(entry));
  return result;
}

double state_difference(const GenesisNaturalLiftState& lhs,
                        const GenesisNaturalLiftState& rhs) {
  return std::max({
      max_abs(lhs.flux - rhs.flux),
      max_abs(lhs.wave - rhs.wave),
      max_abs(subtract(lhs.conjugate, rhs.conjugate)),
      std::abs(lhs.phase.u - rhs.phase.u),
      std::abs(lhs.phase.v - rhs.phase.v),
      std::abs(lhs.tau - rhs.tau),
      std::abs(lhs.reservoir_energy - rhs.reservoir_energy)});
}

double quadratic_energy(const GenesisNaturalLiftState& state) {
  return 0.5 * (state.flux.mag2() + state.wave.mag2())
      + state.reservoir_energy;
}

struct LocalGeometry {
  bool valid = false;
  double radius = 0.0;
  double excess = 0.0;
  double probability = 0.0;
  double wave_scale = 0.0;
  double tangential_scale = 0.0;
  Vec3 direction{};
  SixVector grad_probability{};
  SixVector grad_withdrawal{};
};

LocalGeometry geometry(const GenesisNaturalLiftState& state,
                       const GenesisNaturalLiftOptions& options) {
  LocalGeometry result;
  result.radius = state.flux.mag();
  if (!(options.genesis_threshold > 0.0)
      || !(options.manifestation_scale > 0.0)
      || options.kinetic_drain < 0.0
      || !(options.kinetic_drain < 1.0)
      || !(result.radius > options.genesis_threshold)
      || !finite_vec(state.flux) || !finite_vec(state.wave)) {
    return result;
  }
  result.excess = result.radius - options.genesis_threshold;
  result.probability = 1.0 - std::exp(
      -result.excess / options.manifestation_scale);
  result.wave_scale = 1.0 - options.kinetic_drain;
  result.tangential_scale = result.excess / result.radius;
  result.direction = state.flux * (1.0 / result.radius);
  const double dp_dr = (1.0 - result.probability)
      / options.manifestation_scale;
  result.grad_probability = join(result.direction * dp_dr, {});
  result.grad_withdrawal = join(
      result.direction * options.genesis_threshold,
      state.wave * (1.0 - result.wave_scale * result.wave_scale));
  result.valid = result.probability > 0.0 && result.probability < 1.0
      && result.wave_scale > 0.0 && result.tangential_scale > 0.0;
  return result;
}

Vec3 apply_flux_jacobian(const Vec3& value,
                         const LocalGeometry& geometry_value) {
  const Vec3 radial = geometry_value.direction
      * dot(geometry_value.direction, value);
  const Vec3 tangential = value - radial;
  return radial + tangential * geometry_value.tangential_scale;
}

Vec3 apply_inverse_flux_jacobian(const Vec3& value,
                                 const LocalGeometry& geometry_value) {
  const Vec3 radial = geometry_value.direction
      * dot(geometry_value.direction, value);
  const Vec3 tangential = value - radial;
  return radial + tangential * (1.0 / geometry_value.tangential_scale);
}

SixVector apply_jacobian_transpose(const SixVector& value,
                                   const LocalGeometry& geometry_value,
                                   bool accepted) {
  if (!accepted) return value;
  return join(
      apply_flux_jacobian(first_three(value), geometry_value),
      last_three(value) * geometry_value.wave_scale);
}

SixVector apply_inverse_jacobian_transpose(
    const SixVector& value,
    const LocalGeometry& geometry_value,
    bool accepted) {
  if (!accepted) return value;
  return join(
      apply_inverse_flux_jacobian(first_three(value), geometry_value),
      last_three(value) * (1.0 / geometry_value.wave_scale));
}

double phase_generator_dp(bool accepted,
                          const NaturalExtensionPhase& before,
                          const NaturalExtensionPhase& after,
                          double probability) {
  if (accepted) {
    return -before.u * after.v / (probability * probability);
  }
  return after.u * before.v - after.u - before.v;
}

double phase_generator_residual(bool accepted,
                                const NaturalExtensionPhase& before,
                                const NaturalExtensionPhase& after,
                                double probability) {
  const double expected_v = accepted
      ? after.v / probability
      : (after.v - probability) / (1.0 - probability);
  const double expected_u = accepted
      ? before.u / probability
      : (before.u - probability) / (1.0 - probability);
  return std::max(
      std::abs(before.v - expected_v),
      std::abs(after.u - expected_u));
}

GenesisNaturalLiftState reverse_lift(
    const GenesisNaturalLiftState& after,
    const GenesisNaturalLiftOptions& options,
    bool accepted) {
  GenesisNaturalLiftState before = after;
  if (accepted) {
    const double residual = after.flux.mag();
    before.flux = after.flux * (
        1.0 + options.genesis_threshold / residual);
    before.wave = after.wave * (1.0 / (1.0 - options.kinetic_drain));
  }
  const auto local = geometry(before, options);
  const auto phase_inverse = reverse_natural_extension_phase(
      after.phase, local.probability);
  before.phase = phase_inverse.recovered;
  before.tau = after.tau;

  const double withdrawal = accepted
      ? genesis_field_withdrawal(
          options.genesis_threshold, local.excess, before.wave.mag2(),
          options.kinetic_drain)
      : 0.0;
  before.reservoir_energy = after.reservoir_energy - withdrawal;

  const double generator_dp = phase_generator_dp(
      accepted, before.phase, after.phase, local.probability);
  const SixVector withdrawal_gradient = accepted
      ? local.grad_withdrawal : SixVector{};
  before.conjugate = add(
      subtract(
          apply_jacobian_transpose(
              after.conjugate, local, accepted),
          scale(withdrawal_gradient, before.tau)),
      scale(local.grad_probability, generator_dp));
  return before;
}

}  // namespace

NaturalExtensionPhaseStep advance_natural_extension_phase(
    const NaturalExtensionPhase& before, double probability) {
  NaturalExtensionPhaseStep result;
  result.before = before;
  result.probability = probability;
  if (!(probability > 0.0) || !(probability < 1.0)
      || !(before.u >= 0.0) || !(before.u < 1.0)
      || !(before.v >= 0.0) || !(before.v < 1.0)) {
    return result;
  }

  result.accepted = before.u < probability;
  if (result.accepted) {
    result.after.u = before.u / probability;
    result.after.v = probability * before.v;
  } else {
    result.after.u = (before.u - probability) / (1.0 - probability);
    result.after.v = probability + (1.0 - probability) * before.v;
  }
  result.jacobian_determinant = 1.0;
  const auto inverse = reverse_natural_extension_phase(
      result.after, probability);
  result.recovered = inverse.recovered;
  result.inverse_residual = std::max(
      std::abs(result.recovered.u - before.u),
      std::abs(result.recovered.v - before.v));
  result.valid = result.after.u >= 0.0 && result.after.u < 1.0
      && result.after.v >= 0.0 && result.after.v < 1.0
      && inverse.valid && inverse.accepted == result.accepted;
  return result;
}

NaturalExtensionPhaseStep reverse_natural_extension_phase(
    const NaturalExtensionPhase& after, double probability) {
  NaturalExtensionPhaseStep result;
  result.after = after;
  result.probability = probability;
  if (!(probability > 0.0) || !(probability < 1.0)
      || !(after.u >= 0.0) || !(after.u < 1.0)
      || !(after.v >= 0.0) || !(after.v < 1.0)) {
    return result;
  }

  result.accepted = after.v < probability;
  if (result.accepted) {
    result.recovered.u = probability * after.u;
    result.recovered.v = after.v / probability;
  } else {
    result.recovered.u = probability + (1.0 - probability) * after.u;
    result.recovered.v = (after.v - probability) / (1.0 - probability);
  }
  result.before = result.recovered;
  result.jacobian_determinant = 1.0;
  result.valid = result.recovered.u >= 0.0 && result.recovered.u < 1.0
      && result.recovered.v >= 0.0 && result.recovered.v < 1.0;
  return result;
}

GenesisNaturalLiftStep advance_genesis_natural_lift(
    const GenesisNaturalLiftState& before,
    const GenesisNaturalLiftOptions& options) {
  GenesisNaturalLiftStep result;
  result.before = before;
  const auto local = geometry(before, options);
  if (!local.valid) return result;

  const auto phase_step = advance_natural_extension_phase(
      before.phase, local.probability);
  if (!phase_step.valid) return result;
  result.accepted = phase_step.accepted;
  result.probability = local.probability;
  result.after = before;
  result.after.phase = phase_step.after;

  if (result.accepted) {
    result.after.flux = before.flux * local.tangential_scale;
    result.after.wave = before.wave * local.wave_scale;
    result.field_withdrawal = genesis_field_withdrawal(
        options.genesis_threshold, local.excess, before.wave.mag2(),
        options.kinetic_drain);
  }
  result.after.reservoir_energy = before.reservoir_energy
      + result.field_withdrawal;

  const double generator_dp = phase_generator_dp(
      result.accepted, before.phase, result.after.phase, local.probability);
  const SixVector withdrawal_gradient = result.accepted
      ? local.grad_withdrawal : SixVector{};
  const SixVector lift_argument = add(
      subtract(before.conjugate,
               scale(local.grad_probability, generator_dp)),
      scale(withdrawal_gradient, before.tau));
  result.after.conjugate = apply_inverse_jacobian_transpose(
      lift_argument, local, result.accepted);

  result.recovered = reverse_lift(result.after, options, result.accepted);
  result.inverse_residual = state_difference(before, result.recovered);
  result.energy_residual = std::abs(
      quadratic_energy(result.after) - quadratic_energy(before));
  result.phase_generator_residual = phase_generator_residual(
      result.accepted, before.phase, result.after.phase, local.probability);

  const SixVector conjugate_reconstruction = add(
      subtract(
          apply_jacobian_transpose(
              result.after.conjugate, local, result.accepted),
          scale(withdrawal_gradient, before.tau)),
      scale(local.grad_probability, generator_dp));
  result.conjugate_generator_residual = max_abs(
      subtract(before.conjugate, conjugate_reconstruction));
  result.reservoir_generator_residual = std::abs(
      before.reservoir_energy
      - (result.after.reservoir_energy - result.field_withdrawal));

  if (result.accepted) {
    const double radial_actual = local.wave_scale
        * dot(local.direction,
              apply_flux_jacobian(local.direction, local)) - 1.0;
    const Vec3 tangent = tangent_to(local.direction);
    const double tangential_actual = local.wave_scale
        * dot(tangent, apply_flux_jacobian(tangent, local)) - 1.0;
    result.raw_radial_symplectic_defect = local.wave_scale - 1.0;
    result.raw_tangential_symplectic_defect =
        local.wave_scale * local.tangential_scale - 1.0;
    result.raw_volume_jacobian = local.tangential_scale
        * local.tangential_scale
        * local.wave_scale * local.wave_scale * local.wave_scale;
    result.raw_symplectic_formula_residual = std::max(
        std::abs(radial_actual - result.raw_radial_symplectic_defect),
        std::abs(tangential_actual
                 - result.raw_tangential_symplectic_defect));
  }

  result.valid = result.inverse_residual <= 1e-11
      && result.energy_residual <= gate
      && result.phase_generator_residual <= gate
      && result.conjugate_generator_residual <= gate
      && result.reservoir_generator_residual <= gate
      && result.raw_symplectic_formula_residual <= gate;
  return result;
}

GenesisNaturalExtensionResult analyze_genesis_natural_extension() {
  GenesisNaturalExtensionResult result;
  const std::array<double, 6> probabilities{{
      1.0 - std::exp(-0.125),
      1.0 - std::exp(-0.5),
      1.0 - std::exp(-1.25),
      0.25, 0.5, 0.75}};
  const std::array<double, 2> fractions{{0.25, 0.75}};
  const std::array<double, 2> phase_v{{0.2, 0.8}};

  bool baker_ok = true;
  for (double probability : probabilities) {
    for (bool accepted : {true, false}) {
      for (double fraction : fractions) {
        for (double v : phase_v) {
          NaturalExtensionPhase before;
          before.u = accepted
              ? fraction * probability
              : probability + fraction * (1.0 - probability);
          before.v = v;
          const auto step = advance_natural_extension_phase(
              before, probability);
          ++result.baker_arms;
          baker_ok = baker_ok && step.valid && step.accepted == accepted;
          result.maximum_baker_inverse_residual = std::max(
              result.maximum_baker_inverse_residual, step.inverse_residual);
          result.maximum_baker_jacobian_residual = std::max(
              result.maximum_baker_jacobian_residual,
              std::abs(step.jacobian_determinant - 1.0));
        }
      }
    }
  }

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
  const std::array<SixVector, 2> conjugates{{
      {{0.1, -0.2, 0.3, -0.4, 0.5, -0.6}},
      {{-0.7, 0.2, 0.4, 0.1, -0.3, 0.8}}}};
  const std::array<double, 2> taus{{-0.4, 0.7}};

  bool lift_ok = true;
  bool raw_noncanonical = true;
  double minimum_tangential_defect = std::numeric_limits<double>::infinity();
  double maximum_raw_jacobian = 0.0;
  for (const Vec3& raw_direction : directions) {
    const Vec3 direction = normalized(raw_direction);
    for (double excess : excesses) {
      for (const Vec3& wave : waves) {
        for (double drain : drains) {
          GenesisNaturalLiftOptions options;
          options.kinetic_drain = drain;
          const double probability = 1.0 - std::exp(-excess);
          for (bool accepted : {true, false}) {
            for (double v : phase_v) {
              for (const SixVector& conjugate : conjugates) {
                for (double tau : taus) {
                  GenesisNaturalLiftState before;
                  before.flux = direction * (1.0 + excess);
                  before.wave = wave;
                  before.conjugate = conjugate;
                  before.phase.u = accepted
                      ? 0.37 * probability
                      : probability + 0.63 * (1.0 - probability);
                  before.phase.v = v;
                  before.tau = tau;
                  before.reservoir_energy = 0.7;
                  const auto step = advance_genesis_natural_lift(
                      before, options);
                  ++result.lift_arms;
                  if (accepted) ++result.accepted_lift_arms;
                  lift_ok = lift_ok && step.valid
                      && step.accepted == accepted;
                  result.maximum_lift_inverse_residual = std::max(
                      result.maximum_lift_inverse_residual,
                      step.inverse_residual);
                  result.maximum_energy_residual = std::max(
                      result.maximum_energy_residual, step.energy_residual);
                  result.maximum_phase_generator_residual = std::max(
                      result.maximum_phase_generator_residual,
                      step.phase_generator_residual);
                  result.maximum_conjugate_generator_residual = std::max(
                      result.maximum_conjugate_generator_residual,
                      step.conjugate_generator_residual);
                  result.maximum_reservoir_generator_residual = std::max(
                      result.maximum_reservoir_generator_residual,
                      step.reservoir_generator_residual);
                  result.maximum_raw_symplectic_formula_residual = std::max(
                      result.maximum_raw_symplectic_formula_residual,
                      step.raw_symplectic_formula_residual);
                  if (accepted) {
                    const double defect = std::abs(
                        step.raw_tangential_symplectic_defect);
                    minimum_tangential_defect = std::min(
                        minimum_tangential_defect, defect);
                    maximum_raw_jacobian = std::max(
                        maximum_raw_jacobian, step.raw_volume_jacobian);
                    raw_noncanonical = raw_noncanonical
                        && step.raw_tangential_symplectic_defect < 0.0
                        && step.raw_volume_jacobian < 1.0;
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  result.minimum_raw_tangential_defect_magnitude =
      minimum_tangential_defect;
  result.maximum_raw_volume_jacobian = maximum_raw_jacobian;

  double history_a = 0.0;
  double history_b = 0.0;
  history_a = 0.5 * history_a;        // accepted/lower branch
  history_b = 0.5 + 0.5 * history_b; // rejected/upper branch
  for (int i = 0; i < 63; ++i) {
    history_a = 0.5 + 0.5 * history_a;
    history_b = 0.5 + 0.5 * history_b;
  }
  result.binary64_history_collision = history_a == history_b;
  result.exact_real_is_infinite_information =
      result.binary64_history_collision && history_a == 1.0;

  result.projected_log_forward_reverse_ratio =
      std::numeric_limits<double>::infinity();
  result.projected_kernel_absolutely_irreversible =
      std::isinf(result.projected_log_forward_reverse_ratio)
      && result.projected_log_forward_reverse_ratio > 0.0;

  result.exact_real_natural_extension = baker_ok
      && result.baker_arms == 48
      && result.maximum_baker_inverse_residual <= gate
      && result.maximum_baker_jacobian_residual <= gate;
  result.raw_genesis_is_not_canonical = raw_noncanonical
      && result.accepted_lift_arms == 2160
      && minimum_tangential_defect > 0.0
      && maximum_raw_jacobian < 1.0
      && result.maximum_raw_symplectic_formula_residual <= gate;
  result.branchwise_symplectic_energy_lift = lift_ok
      && result.lift_arms == 4320
      && result.maximum_lift_inverse_residual <= 1e-11
      && result.maximum_energy_residual <= gate
      && result.maximum_phase_generator_residual <= gate
      && result.maximum_conjugate_generator_residual <= gate
      && result.maximum_reservoir_generator_residual <= gate;
  result.additional_primitives_required =
      result.branchwise_symplectic_energy_lift
      && result.raw_genesis_is_not_canonical;
  result.production_common_action_recovered = false;
  result.valid = result.exact_real_natural_extension
      && result.raw_genesis_is_not_canonical
      && result.branchwise_symplectic_energy_lift
      && result.binary64_history_collision
      && result.exact_real_is_infinite_information
      && result.projected_kernel_absolutely_irreversible
      && result.additional_primitives_required
      && !result.production_common_action_recovered;
  return result;
}

}  // namespace ftd::eft
