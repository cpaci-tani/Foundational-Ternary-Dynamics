/** FTD-0496: unique centered knot-to-subcell fiber transaction. */

#include "ftd/eft/centered_fiber_knot_transaction.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <string>

namespace {

constexpr int L = 17;
constexpr ftd::Coord knot{8, 8, 8};
constexpr double c_speed = 0.57735026918962576451;
constexpr double rest_energy = 0.511;
constexpr double coupling = 0.73;
constexpr double gate = 1e-12;
int failures = 0;
double contraction_bound = 0.0;
double worst_trace_residual = 0.0;
double worst_fixed_point_residual = 0.0;
double worst_energy_residual = 0.0;
double worst_gauss_residual = 0.0;
double worst_continuity_residual = 0.0;
double worst_inverse_residual = 0.0;
double worst_causal_excess = 0.0;
double worst_symmetry_residual = 0.0;
double largest_driven_displacement = 0.0;
double largest_dressing_change = 0.0;
int maximum_iterations = 0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

double norm(const ftd::Vec3& value) {
  return std::sqrt(value.x * value.x + value.y * value.y
                   + value.z * value.z);
}

double max_difference(const ftd::Vec3& lhs, const ftd::Vec3& rhs) {
  return std::max({std::abs(lhs.x - rhs.x),
                   std::abs(lhs.y - rhs.y),
                   std::abs(lhs.z - rhs.z)});
}

ftd::eft::MatchedFaceFlux plane_trace_field(
    ftd::Coord site, const ftd::Vec3& centered,
    const ftd::Vec3& jump) {
  ftd::eft::MatchedFaceFlux field(L);
  std::fill(field.x.begin(), field.x.end(), centered.x);
  std::fill(field.y.begin(), field.y.end(), centered.y);
  std::fill(field.z.begin(), field.z.end(), centered.z);
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const int i = field.index(x, y, z);
        if (x == site.x) field.x[static_cast<std::size_t>(i)] += 0.5 * jump.x;
        if (x == site.x - 1) field.x[static_cast<std::size_t>(i)] -= 0.5 * jump.x;
        if (y == site.y) field.y[static_cast<std::size_t>(i)] += 0.5 * jump.y;
        if (y == site.y - 1) field.y[static_cast<std::size_t>(i)] -= 0.5 * jump.y;
        if (z == site.z) field.z[static_cast<std::size_t>(i)] += 0.5 * jump.z;
        if (z == site.z - 1) field.z[static_cast<std::size_t>(i)] -= 0.5 * jump.z;
      }
    }
  }
  return field;
}

ftd::eft::CenteredFiberKnotStep solve(
    const ftd::eft::MatchedFaceFlux& field,
    ftd::Coord site,
    const ftd::Vec3& momentum,
    int charge,
    const ftd::Vec3* guess = nullptr) {
  ftd::eft::CenteredFiberKnotInput input;
  input.electric_before = field;
  input.site = site;
  input.momentum_before = momentum;
  input.dressing_before = 0.37;
  input.charge = charge;
  input.coupling = coupling;
  input.dt = 1.0;
  input.rest_energy = rest_energy;
  input.causal_speed = c_speed;
  if (guess != nullptr) {
    input.initial_guess = *guess;
    input.use_initial_guess = true;
  }
  return ftd::eft::solve_centered_fiber_knot_step(input);
}

void accumulate(const ftd::eft::CenteredFiberKnotStep& result) {
  worst_trace_residual = std::max(
      worst_trace_residual, result.centered_current_trace_residual);
  worst_fixed_point_residual = std::max(
      worst_fixed_point_residual, result.fixed_point_residual);
  worst_energy_residual = std::max(
      worst_energy_residual, result.total_energy_residual);
  worst_gauss_residual = std::max(
      worst_gauss_residual, result.relative_gauss_residual);
  worst_continuity_residual = std::max(
      worst_continuity_residual, result.continuity_residual);
  worst_inverse_residual = std::max(
      worst_inverse_residual, result.inverse_residual);
  worst_causal_excess = std::max(
      worst_causal_excess, result.causal_excess);
  largest_driven_displacement = std::max(
      largest_driven_displacement, norm(result.displacement));
  largest_dressing_change = std::max(
      largest_dressing_change, std::abs(result.dressing_change));
  maximum_iterations = std::max(maximum_iterations, result.iterations);
}

ftd::Vec3 permute(const ftd::Vec3& value,
                  const std::array<int, 3>& p) {
  const double v[3] = {value.x, value.y, value.z};
  return {v[p[0]], v[p[1]], v[p[2]]};
}

}  // namespace

int main() {
  contraction_bound = ftd::eft::centered_knot_contraction_bound(
      coupling, 1.0, rest_energy, c_speed);
  check("frozen knot map is a certified contraction",
        contraction_bound < 1.0);

  bool trace_formula_ok = true;
  const std::array<ftd::Vec3, 3> magnitudes{{
      {0.11, 0.19, 0.23}, {0.31, 0.07, 0.27}, {0.41, 0.29, 0.13}}};
  for (const auto& magnitude : magnitudes) {
    for (int sx : {-1, +1}) {
      for (int sy : {-1, +1}) {
        for (int sz : {-1, +1}) {
          const ftd::Vec3 d{
              sx * magnitude.x, sy * magnitude.y, sz * magnitude.z};
          for (int charge : {-1, +1}) {
            const auto current = ftd::eft::make_face_current_segment(
                L, knot, {}, knot, d, charge);
            ftd::eft::MatchedFaceFlux face(L);
            face.x = current.current_x;
            face.y = current.current_y;
            face.z = current.current_z;
            const auto trace = ftd::eft::evaluate_centered_knot_trace(
                face, knot);
            const auto predicted =
                ftd::eft::predict_centered_knot_current_trace(d, charge);
            const double residual = max_difference(trace.centered, predicted);
            worst_trace_residual = std::max(worst_trace_residual, residual);
            trace_formula_ok = trace_formula_ok && current.valid && trace.valid
                && residual <= gate;
          }
        }
      }
    }
  }
  check("exact centered-current trace formula holds in every octant",
        trace_formula_ok);

  ftd::eft::MatchedFaceFlux zero(L);
  const auto rest = solve(zero, knot, {}, +1);
  accumulate(rest);
  check("zero field and zero momentum remain exactly at rest",
        rest.valid && norm(rest.displacement) <= gate
        && norm(rest.momentum_after) <= gate
        && std::abs(rest.dressing_change) <= gate);

  const ftd::Vec3 positive_jump{1.0 / 3.0, 1.0 / 3.0, 1.0 / 3.0};
  const auto self_rest = solve(
      plane_trace_field(knot, {}, positive_jump), knot, {}, +1);
  accumulate(self_rest);
  check("symmetric Gauss self source has a unique centered rest state",
        self_rest.valid && norm(self_rest.displacement) <= gate
        && norm(self_rest.momentum_after) <= gate);

  const ftd::Vec3 bias{0.08, -0.05, 0.03};
  const auto driven = solve(
      plane_trace_field(knot, bias, positive_jump), knot, {}, +1);
  accumulate(driven);
  check("external centered bias causes a unique signed displacement",
        driven.valid && norm(driven.displacement) > 1e-6
        && driven.displacement.x > 0.0
        && driven.displacement.y < 0.0
        && driven.displacement.z > 0.0);

  const ftd::Vec3 initial_momentum{0.12, -0.07, 0.04};
  const auto moving = solve(
      plane_trace_field(knot, {}, positive_jump),
      knot, initial_momentum, +1);
  accumulate(moving);
  check("existing momentum selects a unique nonzero branch",
        moving.valid && moving.displacement.x > 0.0
        && moving.displacement.y < 0.0
        && moving.displacement.z > 0.0);

  const std::array<ftd::Vec3, 3> guesses{{
      {}, {0.3, -0.2, 0.1}, {-0.25, 0.15, -0.05}}};
  bool multistart_ok = true;
  ftd::eft::CenteredFiberKnotStep multistart_reference;
  for (std::size_t i = 0; i < guesses.size(); ++i) {
    const auto trial = solve(
        plane_trace_field(knot, bias, positive_jump),
        knot, {}, +1, &guesses[i]);
    accumulate(trial);
    if (i == 0) multistart_reference = trial;
    multistart_ok = multistart_ok && trial.valid
        && max_difference(trial.momentum_after,
                          multistart_reference.momentum_after) <= gate
        && max_difference(trial.displacement,
                          multistart_reference.displacement) <= gate;
  }
  check("three fixed-point starts converge to the same root", multistart_ok);

  bool amplitude_ok = true;
  for (double amplitude : {0.5, 1.0, 2.0}) {
    const auto trial = solve(
        plane_trace_field(knot, bias * amplitude, positive_jump),
        knot, {}, +1);
    accumulate(trial);
    amplitude_ok = amplitude_ok && trial.valid
        && norm(trial.displacement) > 1e-6;
  }
  check("three external-field amplitudes stay inside the exact gates",
        amplitude_ok);

  const auto negative = solve(
      plane_trace_field(knot, bias, positive_jump * -1.0),
      knot, {}, -1);
  accumulate(negative);
  const double polarity_residual = std::max(
      max_difference(negative.displacement, driven.displacement * -1.0),
      max_difference(negative.momentum_after,
                     driven.momentum_after * -1.0));
  worst_symmetry_residual = std::max(
      worst_symmetry_residual, polarity_residual);
  check("opposite polarity mirrors the driven trajectory",
        negative.valid && polarity_residual <= gate);

  const ftd::Coord shifted{11, 5, 12};
  const auto translated = solve(
      plane_trace_field(shifted, bias, positive_jump), shifted, {}, +1);
  accumulate(translated);
  const double translation_residual = std::max(
      max_difference(translated.displacement, driven.displacement),
      max_difference(translated.momentum_after, driven.momentum_after));
  worst_symmetry_residual = std::max(
      worst_symmetry_residual, translation_residual);
  check("integer translation preserves the transaction",
        translated.valid && translation_residual <= gate);

  const std::array<std::array<int, 3>, 6> permutations{{
      {{0, 1, 2}}, {{0, 2, 1}}, {{1, 0, 2}},
      {{1, 2, 0}}, {{2, 0, 1}}, {{2, 1, 0}}}};
  bool cubic_ok = true;
  for (const auto& p : permutations) {
    for (int sx : {-1, +1}) {
      for (int sy : {-1, +1}) {
        for (int sz : {-1, +1}) {
          ftd::Vec3 transformed_bias = permute(bias, p);
          transformed_bias.x *= sx;
          transformed_bias.y *= sy;
          transformed_bias.z *= sz;
          ftd::Vec3 expected = permute(driven.displacement, p);
          expected.x *= sx;
          expected.y *= sy;
          expected.z *= sz;
          const auto trial = solve(
              plane_trace_field(knot, transformed_bias, positive_jump),
              knot, {}, +1);
          accumulate(trial);
          const double residual = max_difference(trial.displacement, expected);
          worst_symmetry_residual = std::max(
              worst_symmetry_residual, residual);
          cubic_ok = cubic_ok && trial.valid && residual <= gate;
        }
      }
    }
  }
  check("all 48 signed cubic maps preserve the knot step", cubic_ok);

  check("all exact transaction identities close",
        worst_fixed_point_residual <= gate
        && worst_energy_residual <= gate
        && worst_gauss_residual <= gate
        && worst_continuity_residual <= gate
        && worst_inverse_residual <= 1e-10
        && worst_causal_excess <= gate);
  check("invalid charge fails closed",
        !solve(zero, knot, {}, 0).valid);

  std::cout.precision(17);
  std::cout << "contraction_bound=" << contraction_bound << '\n'
            << "largest_driven_displacement="
            << largest_driven_displacement << '\n'
            << "largest_dressing_change=" << largest_dressing_change << '\n'
            << "maximum_iterations=" << maximum_iterations << '\n'
            << "worst_trace_residual=" << worst_trace_residual << '\n'
            << "worst_fixed_point_residual="
            << worst_fixed_point_residual << '\n'
            << "worst_energy_residual=" << worst_energy_residual << '\n'
            << "worst_gauss_residual=" << worst_gauss_residual << '\n'
            << "worst_continuity_residual="
            << worst_continuity_residual << '\n'
            << "worst_inverse_residual=" << worst_inverse_residual << '\n'
            << "worst_causal_excess=" << worst_causal_excess << '\n'
            << "worst_symmetry_residual="
            << worst_symmetry_residual << '\n'
            << "centered_fiber_knot_transaction failures="
            << failures << '\n'
            << "verdict=UNIQUE_CENTERED_FIBER_KNOT_STEP\n";
  return failures == 0 ? 0 : 1;
}
