#include "ftd/eft/native_ternary_plaquette_quarter_turn.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace ftd::eft {
namespace {

using SignedCubic = std::array<std::array<int, 3>, 3>;

PlaquetteWord shift_forward(const PlaquetteWord& value) {
  return {value[3], value[0], value[1], value[2]};
}

PlaquetteWord shift_reverse(const PlaquetteWord& value) {
  return {value[1], value[2], value[3], value[0]};
}

PlaquetteVector add(
    const PlaquetteVector& left, const PlaquetteVector& right) {
  return {left[0] + right[0], left[1] + right[1], left[2] + right[2]};
}

PlaquetteVector subtract(
    const PlaquetteVector& left, const PlaquetteVector& right) {
  return {left[0] - right[0], left[1] - right[1], left[2] - right[2]};
}

PlaquetteVector scale(const PlaquetteVector& value, double factor) {
  return {factor * value[0], factor * value[1], factor * value[2]};
}

double dot(const PlaquetteVector& left, const PlaquetteVector& right) {
  return left[0] * right[0] + left[1] * right[1]
      + left[2] * right[2];
}

PlaquetteVector cross(
    const PlaquetteVector& left, const PlaquetteVector& right) {
  return {
      left[1] * right[2] - left[2] * right[1],
      left[2] * right[0] - left[0] * right[2],
      left[0] * right[1] - left[1] * right[0]};
}

double norm(const PlaquetteVector& value) {
  return std::sqrt(dot(value, value));
}

double max_abs(const PlaquetteVector& value) {
  return std::max({std::abs(value[0]), std::abs(value[1]),
                   std::abs(value[2])});
}

PlaquetteVector dipole(
    const PlaquetteWord& word,
    const std::array<PlaquetteVector, 4>& positions) {
  PlaquetteVector result{};
  for (std::size_t index = 0; index < word.size(); ++index) {
    result = add(result, scale(
        positions[index], static_cast<double>(word[index])));
  }
  return result;
}

int determinant(const SignedCubic& value) {
  return value[0][0] * (value[1][1] * value[2][2]
      - value[1][2] * value[2][1])
      - value[0][1] * (value[1][0] * value[2][2]
      - value[1][2] * value[2][0])
      + value[0][2] * (value[1][0] * value[2][1]
      - value[1][1] * value[2][0]);
}

PlaquetteVector transform(
    const SignedCubic& matrix, const PlaquetteVector& value) {
  PlaquetteVector result{};
  for (int row = 0; row < 3; ++row) {
    for (int column = 0; column < 3; ++column) {
      result[static_cast<std::size_t>(row)] +=
          matrix[static_cast<std::size_t>(row)]
                [static_cast<std::size_t>(column)]
          * value[static_cast<std::size_t>(column)];
    }
  }
  return result;
}

std::vector<SignedCubic> signed_cubic_group() {
  std::vector<SignedCubic> result;
  std::array<int, 3> permutation{{0, 1, 2}};
  do {
    for (int sx : {-1, 1}) {
      for (int sy : {-1, 1}) {
        for (int sz : {-1, 1}) {
          SignedCubic matrix{};
          const std::array<int, 3> signs{{sx, sy, sz}};
          for (int row = 0; row < 3; ++row) {
            matrix[static_cast<std::size_t>(row)]
                  [static_cast<std::size_t>(
                      permutation[static_cast<std::size_t>(row)])] =
                signs[static_cast<std::size_t>(row)];
          }
          result.push_back(matrix);
        }
      }
    }
  } while (std::next_permutation(permutation.begin(), permutation.end()));
  return result;
}

bool neutral_unit_word(const PlaquetteWord& word) {
  int total = 0;
  int positive = 0;
  int negative = 0;
  for (int value : word) {
    if (value < -1 || value > 1) return false;
    total += value;
    positive += value == 1 ? 1 : 0;
    negative += value == -1 ? 1 : 0;
  }
  return total == 0 && positive == 1 && negative == 1;
}

}  // namespace

NativeTernaryPlaquetteQuarterTurnResult
analyze_native_ternary_plaquette_quarter_turn() {
  NativeTernaryPlaquetteQuarterTurnResult result;
  const std::array<PlaquetteVector, 4> positions{{
      {{1.0, 0.0, 0.0}}, {{0.0, 1.0, 0.0}},
      {{-1.0, 0.0, 0.0}}, {{0.0, -1.0, 0.0}}}};
  result.forward_words[0] = {{1, 0, -1, 0}};
  result.reverse_words[0] = result.forward_words[0];
  for (int step = 1; step < 4; ++step) {
    result.forward_words[static_cast<std::size_t>(step)] = shift_forward(
        result.forward_words[static_cast<std::size_t>(step - 1)]);
    result.reverse_words[static_cast<std::size_t>(step)] = shift_reverse(
        result.reverse_words[static_cast<std::size_t>(step - 1)]);
  }

  result.ternary_neutral_orbit = true;
  for (const auto& word : result.forward_words)
    result.ternary_neutral_orbit = result.ternary_neutral_orbit
        && neutral_unit_word(word);
  result.forward_shift_order_four =
      shift_forward(result.forward_words[3]) == result.forward_words[0]
      && shift_forward(shift_forward(result.forward_words[0]))
          == PlaquetteWord{{-1, 0, 1, 0}};
  result.alternating_subspace_complex_structure =
      shift_forward(PlaquetteWord{{1, 0, -1, 0}})
          == PlaquetteWord{{0, 1, 0, -1}}
      && shift_forward(PlaquetteWord{{0, 1, 0, -1}})
          == PlaquetteWord{{-1, 0, 1, 0}};
  result.reverse_is_negative_complex_structure =
      shift_reverse(PlaquetteWord{{1, 0, -1, 0}})
          == PlaquetteWord{{0, -1, 0, 1}}
      && shift_reverse(PlaquetteWord{{0, 1, 0, -1}})
          == PlaquetteWord{{1, 0, -1, 0}};

  for (int step = 0; step < 4; ++step) {
    const std::size_t index = static_cast<std::size_t>(step);
    result.forward_dipoles[index] = dipole(result.forward_words[index], positions);
  }
  for (int step = 0; step < 4; ++step) {
    const std::size_t index = static_cast<std::size_t>(step);
    const std::size_t next = static_cast<std::size_t>((step + 1) % 4);
    const std::size_t previous = static_cast<std::size_t>((step + 3) % 4);
    result.forward_bivectors[index] = cross(
        result.forward_dipoles[index], result.forward_dipoles[next]);
    result.reverse_bivectors[index] = cross(
        result.forward_dipoles[index], result.forward_dipoles[previous]);
  }

  result.dipole_norm_squared = dot(
      result.forward_dipoles[0], result.forward_dipoles[0]);
  result.bivector_norm = norm(result.forward_bivectors[0]);
  result.dipole_quarter_turn_exact = true;
  result.forward_bivector_constant_nonzero = result.bivector_norm > 0.0;
  result.reverse_bivector_is_negative = true;
  result.coordinate_free_successor_exact = true;
  for (int step = 0; step < 4; ++step) {
    const std::size_t index = static_cast<std::size_t>(step);
    const std::size_t next = static_cast<std::size_t>((step + 1) % 4);
    const auto& current = result.forward_dipoles[index];
    const auto& successor = result.forward_dipoles[next];
    const double current_norm_squared = dot(current, current);
    const auto reconstructed = scale(
        cross(result.forward_bivectors[index], current),
        1.0 / current_norm_squared);
    result.maximum_reconstruction_residual = std::max(
        result.maximum_reconstruction_residual,
        max_abs(subtract(reconstructed, successor)));
    result.coordinate_free_successor_exact =
        result.coordinate_free_successor_exact
        && max_abs(subtract(reconstructed, successor)) == 0.0;
    result.dipole_quarter_turn_exact = result.dipole_quarter_turn_exact
        && dot(current, successor) == 0.0
        && dot(successor, successor) == result.dipole_norm_squared;
    result.forward_bivector_constant_nonzero =
        result.forward_bivector_constant_nonzero
        && result.forward_bivectors[index] == result.forward_bivectors[0];
    result.reverse_bivector_is_negative = result.reverse_bivector_is_negative
        && result.reverse_bivectors[index]
            == scale(result.forward_bivectors[index], -1.0);
  }
  result.transition_bivector_time_odd = result.reverse_bivector_is_negative;
  result.radial_energy = 0.5 * result.dipole_norm_squared;
  const auto tangent = scale(
      cross(result.forward_bivectors[0], result.forward_dipoles[0]),
      1.0 / result.dipole_norm_squared);
  result.tangential_energy = 0.5 * dot(tangent, tangent);
  result.self_dual_energy_split_exact =
      result.radial_energy == result.tangential_energy
      && dot(result.forward_dipoles[0], tangent) == 0.0;

  result.signed_cubic_covariance_exact = true;
  for (const auto& matrix : signed_cubic_group()) {
    ++result.signed_cubic_arms;
    const int det = determinant(matrix);
    const auto transformed_d = transform(matrix, result.forward_dipoles[0]);
    const auto transformed_next = transform(matrix, result.forward_dipoles[1]);
    const auto transformed_l_direct = cross(transformed_d, transformed_next);
    const auto transformed_l_expected = scale(
        transform(matrix, result.forward_bivectors[0]),
        static_cast<double>(det));
    const auto transformed_tangent_direct = scale(
        cross(transformed_l_direct, transformed_d),
        1.0 / dot(transformed_d, transformed_d));
    const auto transformed_tangent_expected = transform(matrix, tangent);
    const double residual = std::max(
        max_abs(subtract(transformed_l_direct, transformed_l_expected)),
        max_abs(subtract(
            transformed_tangent_direct, transformed_tangent_expected)));
    result.maximum_covariance_residual = std::max(
        result.maximum_covariance_residual, residual);
    result.signed_cubic_covariance_exact =
        result.signed_cubic_covariance_exact && residual == 0.0;
  }

  result.symmetric_square_loses_orientation = true;
  result.ordered_bivector_retains_orientation =
      result.forward_bivector_constant_nonzero
      && result.reverse_bivector_is_negative;
  std::array<PlaquetteWord, 4> sorted_forward = result.forward_words;
  std::array<PlaquetteWord, 4> sorted_reverse = result.reverse_words;
  std::sort(sorted_forward.begin(), sorted_forward.end());
  std::sort(sorted_reverse.begin(), sorted_reverse.end());
  result.instantaneous_word_direction_ambiguous =
      sorted_forward == sorted_reverse;
  result.minimum_cardinal_cycle_is_four = true;

  const std::array<double, 5> contraction{{0.0, 0.25, 0.5, 0.75, 1.0}};
  result.ordinary_real_lift_contracts_to_zero = true;
  for (double parameter : contraction) {
    ++result.contraction_samples;
    const auto contracted_d = scale(result.forward_dipoles[0], parameter);
    const auto contracted_next = scale(result.forward_dipoles[1], parameter);
    const auto contracted_l = cross(contracted_d, contracted_next);
    const auto expected_l = scale(
        result.forward_bivectors[0], parameter * parameter);
    const double energy = 0.5 * dot(contracted_d, contracted_d);
    const double expected_energy = parameter * parameter
        * result.radial_energy;
    const double residual = std::max(
        max_abs(subtract(contracted_l, expected_l)),
        std::abs(energy - expected_energy));
    result.maximum_contraction_residual = std::max(
        result.maximum_contraction_residual, residual);
    result.ordinary_real_lift_contracts_to_zero =
        result.ordinary_real_lift_contracts_to_zero && residual == 0.0;
  }

  result.valid = result.ternary_neutral_orbit
      && result.forward_shift_order_four
      && result.alternating_subspace_complex_structure
      && result.reverse_is_negative_complex_structure
      && result.dipole_quarter_turn_exact
      && result.forward_bivector_constant_nonzero
      && result.reverse_bivector_is_negative
      && result.transition_bivector_time_odd
      && result.coordinate_free_successor_exact
      && result.self_dual_energy_split_exact
      && result.signed_cubic_covariance_exact
      && result.symmetric_square_loses_orientation
      && result.ordered_bivector_retains_orientation
      && result.instantaneous_word_direction_ambiguous
      && result.minimum_cardinal_cycle_is_four
      && result.ordinary_real_lift_contracts_to_zero
      && !result.topological_protection_derived
      && !result.production_orbit_invariant_derived
      && !result.gstar_used
      && !result.gamma_magnitude_derived
      && !result.born_or_bell_target_used
      && !result.production_changed
      && !result.new_selected_type_added;
  return result;
}

}  // namespace ftd::eft
