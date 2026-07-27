/** FTD-0497: exact axial face hop and raw threshold-map reciprocity gate. */

#include "ftd/eft/axial_face_hop_reciprocity.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <string>

namespace {

constexpr int L = 17;
constexpr double c_speed = 0.57735026918962576451;
constexpr double rest_energy = 0.511;
constexpr double coupling = 0.73;
constexpr double gate = 1e-12;
constexpr double inverse_gate = 1e-10;

int failures = 0;
double contraction_bound = 0.0;
double largest_hop_displacement = 0.0;
double worst_fixed_point_residual = 0.0;
double worst_energy_residual = 0.0;
double worst_gauss_residual = 0.0;
double worst_continuity_residual = 0.0;
double worst_physical_inverse_residual = 0.0;
double worst_shape_inverse_residual = 0.0;
double worst_field_inverse_residual = 0.0;
double worst_momentum_inverse_residual = 0.0;
double worst_preimage_residual = 0.0;
double smallest_raw_remainder_inverse_defect = INFINITY;
int smallest_raw_anchor_inverse_mismatch = 1000;
int maximum_iterations = 0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

double component(const ftd::Vec3& value, int axis) {
  return axis == 0 ? value.x : axis == 1 ? value.y : value.z;
}

ftd::Vec3 axis_vector(int axis, double value) {
  if (axis == 0) return {value, 0.0, 0.0};
  if (axis == 1) return {0.0, value, 0.0};
  return {0.0, 0.0, value};
}

ftd::eft::MatchedFaceFlux uniform_field(int axis, double amplitude) {
  ftd::eft::MatchedFaceFlux field(L);
  auto& values = axis == 0 ? field.x : axis == 1 ? field.y : field.z;
  std::fill(values.begin(), values.end(), amplitude);
  return field;
}

ftd::eft::AxialFaceHopStep solve(
    int axis,
    ftd::Coord site,
    double remainder,
    double momentum,
    int charge,
    double field_amplitude,
    const double* guess = nullptr) {
  ftd::eft::AxialFaceHopInput input;
  input.electric_before = uniform_field(axis, field_amplitude);
  input.site = site;
  input.remainder = axis_vector(axis, remainder);
  input.momentum_before = axis_vector(axis, momentum);
  input.dressing_before = 0.271;
  input.axis = axis;
  input.charge = charge;
  input.coupling = coupling;
  input.dt = 1.0;
  input.rest_energy = rest_energy;
  input.causal_speed = c_speed;
  if (guess != nullptr) {
    input.initial_momentum_guess = *guess;
    input.use_initial_guess = true;
  }
  return ftd::eft::solve_axial_face_hop_step(input);
}

void accumulate(const ftd::eft::AxialFaceHopStep& result) {
  largest_hop_displacement = std::max(
      largest_hop_displacement, std::abs(component(result.displacement,
                                                    result.axis)));
  worst_fixed_point_residual = std::max(
      worst_fixed_point_residual, result.fixed_point_residual);
  worst_energy_residual = std::max(
      worst_energy_residual, result.total_energy_residual);
  worst_gauss_residual = std::max(
      worst_gauss_residual, result.relative_gauss_residual);
  worst_continuity_residual = std::max(
      worst_continuity_residual, result.continuity_residual);
  worst_physical_inverse_residual = std::max(
      worst_physical_inverse_residual, result.physical_inverse_residual);
  worst_shape_inverse_residual = std::max(
      worst_shape_inverse_residual, result.shape_inverse_residual);
  worst_field_inverse_residual = std::max(
      worst_field_inverse_residual, result.field_inverse_residual);
  worst_momentum_inverse_residual = std::max(
      worst_momentum_inverse_residual, result.momentum_inverse_residual);
  worst_preimage_residual = std::max({
      worst_preimage_residual,
      result.preimage_shape_residual,
      result.preimage_output_residual});
  if (result.hopped) {
    smallest_raw_remainder_inverse_defect = std::min(
        smallest_raw_remainder_inverse_defect,
        result.raw_remainder_inverse_residual);
    smallest_raw_anchor_inverse_mismatch = std::min(
        smallest_raw_anchor_inverse_mismatch,
        result.raw_anchor_inverse_mismatch);
  }
  maximum_iterations = std::max(maximum_iterations, result.iterations);
}

}  // namespace

int main() {
  contraction_bound = ftd::eft::axial_face_hop_contraction_bound(
      coupling, 1.0, rest_energy, c_speed);
  check("uniform axial map has a certified contraction",
        contraction_bound < 1.0);

  const ftd::Coord center{8, 8, 8};
  const auto rest = solve(0, center, 0.37, 0.0, +1, 0.0);
  accumulate(rest);
  check("interior zero-field state remains exactly static",
        rest.transaction_valid && !rest.hopped
        && std::abs(component(rest.displacement, 0)) <= gate
        && rest.strict_discrete_inverse);

  std::array<ftd::eft::AxialFaceHopStep, 3> positive;
  bool axes_ok = true;
  for (int axis = 0; axis < 3; ++axis) {
    positive[static_cast<std::size_t>(axis)] = solve(
        axis, center, +0.85, 0.0, +1, 1.35);
    accumulate(positive[static_cast<std::size_t>(axis)]);
    const auto& result = positive[static_cast<std::size_t>(axis)];
    axes_ok = axes_ok && result.transaction_valid && result.hopped
        && result.hop_direction == +1
        && component(result.displacement, axis) > 0.15;
  }
  check("uniform field produces a genuine hop on all three axes", axes_ok);

  const auto& driven = positive[0];
  check("the driven hop closes current, Gauss, work, energy, and causality",
        driven.continuity_residual <= gate
        && driven.relative_gauss_residual <= gate
        && driven.matter_work_residual <= gate
        && driven.total_energy_residual <= gate
        && driven.causal_excess <= gate);

  const std::array<double, 3> guesses{{-0.4, 0.0, 1.4}};
  bool multistart_ok = true;
  for (double guess : guesses) {
    const auto trial = solve(
        0, center, +0.85, 0.0, +1, 1.35, &guess);
    accumulate(trial);
    multistart_ok = multistart_ok && trial.transaction_valid
        && std::abs(component(trial.momentum_after, 0)
                    - component(driven.momentum_after, 0)) <= gate
        && std::abs(component(trial.displacement, 0)
                    - component(driven.displacement, 0)) <= gate;
  }
  check("three fixed-point starts converge to the same hopping root",
        multistart_ok);

  const auto negative = solve(
      0, center, -0.85, 0.0, -1, 1.35);
  accumulate(negative);
  check("opposite polarity produces the mirrored negative hop",
        negative.transaction_valid && negative.hopped
        && negative.hop_direction == -1
        && std::abs(component(negative.displacement, 0)
                    + component(driven.displacement, 0)) <= gate
        && std::abs(component(negative.momentum_after, 0)
                    + component(driven.momentum_after, 0)) <= gate);

  const auto translated = solve(
      0, {3, 11, 5}, +0.85, 0.0, +1, 1.35);
  accumulate(translated);
  check("integer translation preserves the hopping transaction",
        translated.transaction_valid && translated.hopped
        && std::abs(component(translated.displacement, 0)
                    - component(driven.displacement, 0)) <= gate
        && std::abs(component(translated.momentum_after, 0)
                    - component(driven.momentum_after, 0)) <= gate);

  check("physical position and polarity shape reverse exactly",
        driven.physical_inverse_residual <= inverse_gate
        && driven.shape_inverse_residual <= inverse_gate);
  check("face current, field, and momentum reverse exactly",
        driven.field_inverse_residual <= inverse_gate
        && driven.momentum_inverse_residual <= inverse_gate);

  check("two distinct raw preimages produce the same hopping output",
        driven.preimage_collision
        && driven.preimage_shape_residual <= gate
        && driven.preimage_output_residual <= gate);

  check("raw site/remainder inversion fails by the locked threshold defect",
        !driven.strict_discrete_inverse
        && driven.raw_anchor_inverse_mismatch == 1
        && std::abs(driven.raw_remainder_inverse_residual - 1.0) <= gate);

  ftd::eft::AxialFaceHopInput invalid;
  invalid.electric_before = uniform_field(0, 1.35);
  invalid.site = center;
  invalid.remainder = {0.85, 0.01, 0.0};
  invalid.axis = 0;
  invalid.charge = +1;
  invalid.coupling = coupling;
  invalid.rest_energy = rest_energy;
  invalid.causal_speed = c_speed;
  check("non-axial input fails closed",
        !ftd::eft::solve_axial_face_hop_step(invalid).transaction_valid);

  check("all algebraic gates pass while the frozen raw inverse gate fails",
        worst_fixed_point_residual <= gate
        && worst_energy_residual <= gate
        && worst_gauss_residual <= gate
        && worst_continuity_residual <= gate
        && worst_physical_inverse_residual <= inverse_gate
        && worst_shape_inverse_residual <= inverse_gate
        && worst_field_inverse_residual <= inverse_gate
        && worst_momentum_inverse_residual <= inverse_gate
        && worst_preimage_residual <= gate
        && smallest_raw_anchor_inverse_mismatch == 1
        && std::abs(smallest_raw_remainder_inverse_defect - 1.0) <= gate);

  std::cout.precision(17);
  std::cout << "contraction_bound=" << contraction_bound << '\n'
            << "largest_hop_displacement="
            << largest_hop_displacement << '\n'
            << "maximum_iterations=" << maximum_iterations << '\n'
            << "worst_fixed_point_residual="
            << worst_fixed_point_residual << '\n'
            << "worst_energy_residual=" << worst_energy_residual << '\n'
            << "worst_gauss_residual=" << worst_gauss_residual << '\n'
            << "worst_continuity_residual="
            << worst_continuity_residual << '\n'
            << "worst_physical_inverse_residual="
            << worst_physical_inverse_residual << '\n'
            << "worst_shape_inverse_residual="
            << worst_shape_inverse_residual << '\n'
            << "worst_field_inverse_residual="
            << worst_field_inverse_residual << '\n'
            << "worst_momentum_inverse_residual="
            << worst_momentum_inverse_residual << '\n'
            << "worst_preimage_residual="
            << worst_preimage_residual << '\n'
            << "smallest_raw_anchor_inverse_mismatch="
            << smallest_raw_anchor_inverse_mismatch << '\n'
            << "smallest_raw_remainder_inverse_defect="
            << smallest_raw_remainder_inverse_defect << '\n'
            << "axial_face_hop_reciprocity failures=" << failures << '\n'
            << "verdict=AXIAL_HOP_PHYSICAL_QUOTIENT_ONLY\n";
  return failures == 0 ? 0 : 1;
}
