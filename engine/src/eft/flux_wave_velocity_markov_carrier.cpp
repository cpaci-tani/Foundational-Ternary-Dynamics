#include "ftd/eft/flux_wave_velocity_markov_carrier.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace ftd::eft {
namespace {

bool finite_vec(const Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z);
}

bool finite_site(const FluxWaveVelocityCarrierSite& site) {
  return finite_vec(site.flux) && finite_vec(site.wave_velocity);
}

double maximum_component(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y), std::abs(value.z)});
}

bool close_vec(const Vec3& first, const Vec3& second, double tolerance) {
  const Vec3 difference = first - second;
  const double scale = std::max({1.0, maximum_component(first),
                                 maximum_component(second)});
  return maximum_component(difference) <= tolerance * scale;
}

std::vector<Vec3> apply_stiffness(
    const std::vector<double>& stiffness,
    const std::vector<FluxWaveVelocityCarrierSite>& sites) {
  const std::size_t count = sites.size();
  std::vector<Vec3> result(count);
  for (std::size_t row = 0; row < count; ++row) {
    for (std::size_t column = 0; column < count; ++column) {
      result[row] += sites[column].flux * stiffness[row * count + column];
    }
  }
  return result;
}

bool symmetric(
    const std::vector<double>& stiffness,
    std::size_t count,
    double tolerance) {
  for (std::size_t row = 0; row < count; ++row) {
    for (std::size_t column = row + 1; column < count; ++column) {
      const double first = stiffness[row * count + column];
      const double second = stiffness[column * count + row];
      const double scale = std::max({1.0, std::abs(first), std::abs(second)});
      if (std::abs(first - second) > tolerance * scale) return false;
    }
  }
  return true;
}

}  // namespace

FluxWaveVelocityCarrierSite carrier_from_voxel(const Voxel& voxel) {
  return {voxel.flux, voxel.wave_vel};
}

FluxHistoryChartResult flux_history_to_markov_carrier(
    const Vec3& previous_flux,
    const Vec3& current_flux,
    double step,
    double tolerance) {
  FluxHistoryChartResult result;
  result.previous_flux = previous_flux;
  result.current_flux = current_flux;
  if (!std::isfinite(step) || step <= 0.0) {
    result.status = FluxWaveVelocityCarrierStatus::InvalidStep;
    return result;
  }
  if (!std::isfinite(tolerance) || tolerance < 0.0) {
    result.status = FluxWaveVelocityCarrierStatus::InvalidTolerance;
    return result;
  }
  if (!finite_vec(previous_flux) || !finite_vec(current_flux)) {
    result.status = FluxWaveVelocityCarrierStatus::NonFiniteInput;
    return result;
  }
  result.carrier = {current_flux, (current_flux - previous_flux) * (1.0 / step)};
  result.recovered_previous_flux =
      result.carrier.flux - result.carrier.wave_velocity * step;
  result.exact_roundtrip = close_vec(
      result.recovered_previous_flux, previous_flux, tolerance);
  if (!finite_site(result.carrier) || !finite_vec(result.recovered_previous_flux)) {
    result.status = FluxWaveVelocityCarrierStatus::NonFiniteOutput;
    return result;
  }
  result.status = FluxWaveVelocityCarrierStatus::Valid;
  return result;
}

FreeWaveKickDriftResult evolve_free_wave_kick_drift(
    const FreeWaveKickDriftInput& input) {
  FreeWaveKickDriftResult result;
  result.before = input.sites;
  const std::size_t count = input.sites.size();
  if (count == 0) {
    result.status = FluxWaveVelocityCarrierStatus::EmptyState;
    return result;
  }
  if (!std::isfinite(input.step) || input.step <= 0.0) {
    result.status = FluxWaveVelocityCarrierStatus::InvalidStep;
    return result;
  }
  if (!std::isfinite(input.tolerance) || input.tolerance < 0.0) {
    result.status = FluxWaveVelocityCarrierStatus::InvalidTolerance;
    return result;
  }
  if (input.stiffness.size() != count * count) {
    result.status = FluxWaveVelocityCarrierStatus::InvalidStiffnessShape;
    return result;
  }
  for (const auto& site : input.sites) {
    if (!finite_site(site)) {
      result.status = FluxWaveVelocityCarrierStatus::NonFiniteInput;
      return result;
    }
  }
  for (double value : input.stiffness) {
    if (!std::isfinite(value)) {
      result.status = FluxWaveVelocityCarrierStatus::NonFiniteInput;
      return result;
    }
  }
  result.stiffness_symmetric =
      symmetric(input.stiffness, count, input.tolerance);
  if (!result.stiffness_symmetric) {
    result.status = FluxWaveVelocityCarrierStatus::NonsymmetricStiffness;
    return result;
  }

  const auto force = apply_stiffness(input.stiffness, input.sites);
  result.after = input.sites;
  for (std::size_t index = 0; index < count; ++index) {
    result.after[index].wave_velocity -= force[index] * input.step;
    result.after[index].flux +=
        result.after[index].wave_velocity * input.step;
  }

  result.recovered = result.after;
  for (std::size_t index = 0; index < count; ++index) {
    result.recovered[index].flux -=
        result.recovered[index].wave_velocity * input.step;
  }
  const auto recovered_force = apply_stiffness(
      input.stiffness, result.recovered);
  for (std::size_t index = 0; index < count; ++index) {
    result.recovered[index].wave_velocity +=
        recovered_force[index] * input.step;
  }

  result.maximum_inverse_residual = 0.0;
  result.exact_inverse_verified = true;
  for (std::size_t index = 0; index < count; ++index) {
    const Vec3 dq = result.recovered[index].flux - input.sites[index].flux;
    const Vec3 dp = result.recovered[index].wave_velocity
        - input.sites[index].wave_velocity;
    result.maximum_inverse_residual = std::max(
        result.maximum_inverse_residual,
        std::max(maximum_component(dq), maximum_component(dp)));
    result.exact_inverse_verified &=
        close_vec(result.recovered[index].flux, input.sites[index].flux,
                  input.tolerance)
        && close_vec(result.recovered[index].wave_velocity,
                     input.sites[index].wave_velocity, input.tolerance);
    if (!finite_site(result.after[index]) || !finite_site(result.recovered[index])) {
      result.status = FluxWaveVelocityCarrierStatus::NonFiniteOutput;
      return result;
    }
  }
  result.free_wave_symplectic = result.exact_inverse_verified;
  result.status = FluxWaveVelocityCarrierStatus::Valid;
  return result;
}

double vector_canonical_bond_generator(
    const FluxWaveVelocityCarrierSite& left,
    const FluxWaveVelocityCarrierSite& right) {
  return left.flux.dot(right.wave_velocity)
      - right.flux.dot(left.wave_velocity);
}

double uniform_damping_symplectic_scale(double rho) {
  return rho * rho;
}

double uniform_damping_phase_determinant(double rho, std::size_t sites) {
  if (!std::isfinite(rho)) return std::numeric_limits<double>::quiet_NaN();
  return std::pow(rho, static_cast<double>(6 * sites));
}

}  // namespace ftd::eft
