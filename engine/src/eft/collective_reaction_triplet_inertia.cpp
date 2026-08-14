#include "ftd/eft/collective_reaction_triplet_inertia.h"

#include <algorithm>
#include <cmath>

namespace ftd::eft {
namespace {

bool finite(double value) {
  return std::isfinite(value);
}

bool finite(const Vec3& value) {
  return finite(value.x) && finite(value.y) && finite(value.z);
}

double max_abs(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

Vec3 sum_vectors(const std::vector<Vec3>& values) {
  Vec3 result{};
  for (const auto& value : values) result += value;
  return result;
}

std::vector<double> helmert_matrix(std::size_t n) {
  std::vector<double> matrix(n * n, 0.0);
  const double mean_weight = 1.0 / std::sqrt(static_cast<double>(n));
  for (std::size_t column = 0; column < n; ++column) {
    matrix[column] = mean_weight;
  }
  for (std::size_t row = 1; row < n; ++row) {
    const double denominator = std::sqrt(
        static_cast<double>(row * (row + 1)));
    for (std::size_t column = 0; column < row; ++column) {
      matrix[row * n + column] = 1.0 / denominator;
    }
    matrix[row * n + row] = -static_cast<double>(row) / denominator;
  }
  return matrix;
}

std::vector<Vec3> transform(const std::vector<double>& matrix,
                            const std::vector<Vec3>& values) {
  const std::size_t n = values.size();
  std::vector<Vec3> result(n);
  for (std::size_t row = 0; row < n; ++row) {
    for (std::size_t column = 0; column < n; ++column) {
      result[row] += values[column] * matrix[row * n + column];
    }
  }
  return result;
}

std::vector<Vec3> inverse_transform(const std::vector<double>& matrix,
                                    const std::vector<Vec3>& values) {
  const std::size_t n = values.size();
  std::vector<Vec3> result(n);
  for (std::size_t column = 0; column < n; ++column) {
    for (std::size_t row = 0; row < n; ++row) {
      result[column] += values[row] * matrix[row * n + column];
    }
  }
  return result;
}

double reconstruction_residual(const std::vector<Vec3>& reconstructed,
                               const std::vector<Vec3>& original) {
  double worst = 0.0;
  for (std::size_t i = 0; i < original.size(); ++i) {
    worst = std::max(worst, max_abs(reconstructed[i] - original[i]));
  }
  return worst;
}

}  // namespace

CollectiveReactionTripletResult analyze_collective_reaction_triplet_inertia(
    const CollectiveReactionTripletInput& input) {
  CollectiveReactionTripletResult result;
  const std::size_t n = input.positions.size();
  if (n == 0) {
    result.status = CollectiveReactionTripletStatus::EmptyConstituentSet;
    return result;
  }
  if (input.momenta.size() != n
      || input.position_tangents.size() != n
      || input.constituent_rest_energies.size() != n
      || input.constituent_impulses.size() != n) {
    result.status = CollectiveReactionTripletStatus::SizeMismatch;
    return result;
  }
  if (!(input.limiting_speed > 0.0) || !finite(input.limiting_speed)) {
    result.status = CollectiveReactionTripletStatus::InvalidSpeed;
    return result;
  }
  if (!(input.tolerance > 0.0) || !finite(input.tolerance)) {
    result.status = CollectiveReactionTripletStatus::InvalidTolerance;
    return result;
  }
  if (!finite(input.static_binding_offset)) return result;

  for (std::size_t i = 0; i < n; ++i) {
    if (!finite(input.positions[i]) || !finite(input.momenta[i])
        || !finite(input.position_tangents[i])
        || !finite(input.constituent_impulses[i])
        || !finite(input.constituent_rest_energies[i])) {
      return result;
    }
    if (!(input.constituent_rest_energies[i] > 0.0)) {
      result.status = CollectiveReactionTripletStatus::InvalidRestEnergy;
      return result;
    }
    result.summed_rest_energy += input.constituent_rest_energies[i];
  }

  const auto U = helmert_matrix(n);
  result.modal_positions = transform(U, input.positions);
  result.modal_momenta = transform(U, input.momenta);
  result.modal_position_tangents = transform(U, input.position_tangents);
  result.reconstructed_positions = inverse_transform(U, result.modal_positions);
  result.reconstructed_momenta = inverse_transform(U, result.modal_momenta);

  const double sqrt_n = std::sqrt(static_cast<double>(n));
  result.center = result.modal_positions.front() * (1.0 / sqrt_n);
  result.total_momentum = result.modal_momenta.front() * sqrt_n;
  result.summed_constituent_impulse = sum_vectors(input.constituent_impulses);
  result.momentum_after_impulse = result.total_momentum
      + result.summed_constituent_impulse;

  for (std::size_t row = 0; row < n; ++row) {
    for (std::size_t other = 0; other < n; ++other) {
      double dot = 0.0;
      for (std::size_t column = 0; column < n; ++column) {
        dot += U[row * n + column] * U[other * n + column];
      }
      result.orthogonality_residual = std::max(
          result.orthogonality_residual,
          std::abs(dot - (row == other ? 1.0 : 0.0)));
    }
  }

  for (std::size_t i = 0; i < n; ++i) {
    result.constituent_one_form += input.momenta[i].dot(
        input.position_tangents[i]);
    result.modal_one_form += result.modal_momenta[i].dot(
        result.modal_position_tangents[i]);
  }
  const Vec3 center_tangent = result.modal_position_tangents.front()
      * (1.0 / sqrt_n);
  result.collective_internal_one_form = result.total_momentum.dot(
      center_tangent);
  for (std::size_t i = 1; i < n; ++i) {
    result.collective_internal_one_form += result.modal_momenta[i].dot(
        result.modal_position_tangents[i]);
  }
  result.one_form_residual = std::max(
      std::abs(result.constituent_one_form - result.modal_one_form),
      std::abs(result.constituent_one_form
               - result.collective_internal_one_form));
  result.position_reconstruction_residual = reconstruction_residual(
      result.reconstructed_positions, input.positions);
  result.momentum_reconstruction_residual = reconstruction_residual(
      result.reconstructed_momenta, input.momenta);

  const Vec3 direct_momentum_after = sum_vectors(input.momenta)
      + sum_vectors(input.constituent_impulses);
  result.impulse_sum_residual = max_abs(
      result.momentum_after_impulse - direct_momentum_after);
  result.internal_zero_sum_impulses_cancel =
      result.summed_constituent_impulse.mag() <= input.tolerance;
  result.external_impulses_sum_to_collective_kick =
      result.impulse_sum_residual <= input.tolerance;

  const double c2 = input.limiting_speed * input.limiting_speed;
  const double total_p2 = result.total_momentum.mag2();
  result.minimum_energy_momenta.reserve(n);
  const double collective_energy = std::sqrt(
      result.summed_rest_energy * result.summed_rest_energy
      + c2 * total_p2);
  const Vec3 common_velocity = result.total_momentum
      * (c2 / collective_energy);
  for (std::size_t i = 0; i < n; ++i) {
    const double rest = input.constituent_rest_energies[i];
    result.input_constituent_energy += std::sqrt(
        rest * rest + c2 * input.momenta[i].mag2());

    const Vec3 allocated = result.total_momentum
        * (rest / result.summed_rest_energy);
    result.minimum_energy_momenta.push_back(allocated);
    const double allocated_energy = std::sqrt(
        rest * rest + c2 * allocated.mag2());
    result.minimum_constituent_energy += allocated_energy;
    const Vec3 velocity = allocated * (c2 / allocated_energy);
    result.common_velocity_residual = std::max(
        result.common_velocity_residual,
        max_abs(velocity - common_velocity));
  }
  result.collective_dispersion_energy = collective_energy;
  result.composite_energy_residual = std::abs(
      result.minimum_constituent_energy - collective_energy);
  result.collective_inertial_mass = result.summed_rest_energy / c2;
  result.zero_momentum_energy_curvature = c2 / result.summed_rest_energy;
  result.rest_energy_with_static_offset = result.summed_rest_energy
      + input.static_binding_offset;
  result.static_offset_mass_mismatch = input.static_binding_offset / c2;

  const double scale = std::max({
      1.0, std::abs(result.constituent_one_form),
      result.summed_rest_energy, result.collective_dispersion_energy});
  result.exact_collective_symplectic_sector =
      result.orthogonality_residual <= input.tolerance
      && result.position_reconstruction_residual <= input.tolerance * scale
      && result.momentum_reconstruction_residual <= input.tolerance * scale
      && result.one_form_residual <= input.tolerance * scale;
  result.three_collective_canonical_pairs =
      result.exact_collective_symplectic_sector;
  result.constituent_dispersion_strictly_convex = true;
  result.exact_conditional_composite_dispersion =
      result.composite_energy_residual <= input.tolerance * scale
      && result.common_velocity_residual <= input.tolerance * scale;
  result.conditional_inertial_additivity =
      result.exact_conditional_composite_dispersion;

  if (result.exact_collective_symplectic_sector
      && result.external_impulses_sum_to_collective_kick
      && result.exact_conditional_composite_dispersion) {
    result.status = CollectiveReactionTripletStatus::Valid;
  }
  return result;
}

}  // namespace ftd::eft
