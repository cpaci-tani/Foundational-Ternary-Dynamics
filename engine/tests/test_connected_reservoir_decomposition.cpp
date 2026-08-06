/** FTD-0673: exact complete perturbation reservoir decomposition. */

#include "ftd/eft/connected_reservoir_decomposition.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

constexpr int L = 7;
constexpr double gate = 1e-12;
int failures = 0;

void check(const std::string& label, bool condition) {
  if (condition) return;
  ++failures;
  std::cerr << "FAIL: " << label << '\n';
}

ftd::eft::ConnectedMooreBlockState make_control() {
  ftd::eft::ConnectedMooreBlockState state(L);
  state.width = 1;
  state.orientation_axis = 0;
  state.constituents.resize(2);
  state.charges = {+1, -1};
  state.constituents[0].anchor = {2, 3, 3};
  state.constituents[0].remainder = {0.1, -0.2, 0.05};
  state.constituents[0].momentum = {0.01, -0.02, 0.015};
  state.constituents[1].anchor = {4, 3, 3};
  state.constituents[1].remainder = {-0.1, 0.2, -0.05};
  state.constituents[1].momentum = {-0.01, 0.02, -0.015};
  ftd::eft::MooreBindingEdge edge;
  edge.first = 0;
  edge.second = 1;
  edge.reference_delta = {-2, 0, 0};
  edge.rest_length_squared = 3.24;
  state.edges.push_back(edge);
  constexpr double pi = 3.1415926535897932384626433832795;
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const auto index = static_cast<std::size_t>(
            state.electric.index(x, y, z));
        const double px = 2.0 * pi * x / L;
        const double py = 2.0 * pi * y / L;
        const double pz = 2.0 * pi * z / L;
        state.electric.x[index] = 0.03 * std::sin(px + py);
        state.electric.y[index] = -0.02 * std::cos(py + pz);
        state.electric.z[index] = 0.01 * std::sin(pz + px);
        state.magnetic_half.x[index] = 0.015 * std::cos(px - pz);
        state.magnetic_half.y[index] = 0.012 * std::sin(py - px);
        state.magnetic_half.z[index] = -0.009 * std::cos(pz - py);
      }
    }
  }
  return state;
}

ftd::eft::ConnectedMooreBlockState make_excited(
    const ftd::eft::ConnectedMooreBlockState& control) {
  auto state = control;
  state.constituents[0].remainder += ftd::Vec3{0.01, -0.015, 0.005};
  state.constituents[1].remainder += ftd::Vec3{-0.008, 0.012, -0.004};
  state.constituents[0].momentum += ftd::Vec3{0.004, -0.003, 0.002};
  state.constituents[1].momentum += ftd::Vec3{-0.002, 0.005, -0.001};
  for (std::size_t index = 0; index < state.electric.x.size(); ++index) {
    const double phase = static_cast<double>(index % 17);
    state.electric.x[index] += 1e-4 * std::sin(phase);
    state.electric.y[index] -= 8e-5 * std::cos(phase);
    state.electric.z[index] += 6e-5 * std::sin(0.5 * phase);
    state.magnetic_half.x[index] += 7e-5 * std::cos(0.4 * phase);
    state.magnetic_half.y[index] -= 5e-5 * std::sin(0.3 * phase);
    state.magnetic_half.z[index] += 4e-5 * std::cos(0.2 * phase);
  }
  return state;
}

std::vector<ftd::eft::ConnectedTangentMode> identity_modes() {
  constexpr std::size_t dimension = 6;
  const double mass = ftd::M_INERTIAL;
  std::vector<ftd::eft::ConnectedTangentMode> modes(dimension);
  for (std::size_t mode = 0; mode < dimension; ++mode) {
    modes[mode].omega = 0.4 + 0.1 * mode;
    modes[mode].vector.assign(dimension, 0.0);
    modes[mode].vector[mode] = 1.0 / std::sqrt(mass);
  }
  return modes;
}

}  // namespace

int main() {
  const auto control = make_control();
  const auto excited = make_excited(control);
  const auto modes = identity_modes();
  ftd::eft::ConnectedMooreBlockOptions options;
  options.binding_stiffness = 0.75;
  const auto result = ftd::eft::evaluate_connected_reservoir_decomposition(
      control, excited, modes, {2, 3}, 0.61, options);
  check("valid", result.valid);
  check("complete basis", result.mode_count == 6);
  check("target count", result.target_mode_count == 2);
  check("orthonormality", result.mode_orthonormality_residual <= gate);
  check("modal partition", std::abs(
      result.total_mode_energy - result.target_mode_energy
      - result.other_mode_energy) <= gate);
  check("matter partition", result.matter_decomposition_residual <= gate);
  check("field partition", result.field_decomposition_residual <= gate);
  check("complete partition", result.complete_decomposition_residual <= gate);
  check("nontrivial target", result.target_mode_energy > 0.0);
  check("nontrivial other", result.other_mode_energy > 0.0);
  check("nontrivial dynamic field", result.dynamic_field_energy > 0.0);

  auto tiny = control;
  tiny.constituents[0].momentum.x += 1e-7;
  tiny.constituents[1].remainder.y -= 1e-7;
  tiny.electric.x[3] += 1e-8;
  tiny.magnetic_half.z[5] -= 1e-8;
  const auto tiny_result =
      ftd::eft::evaluate_connected_reservoir_decomposition(
          control, tiny, modes, {2, 3}, 0.61, options);
  check("cancellation-safe tiny perturbation", tiny_result.valid);
  check("tiny field partition",
      tiny_result.field_decomposition_residual <= gate);
  check("tiny complete partition",
      tiny_result.complete_decomposition_residual <= gate);

  auto nonorthogonal = modes;
  nonorthogonal[1].vector = nonorthogonal[0].vector;
  check("nonorthogonal basis fails closed",
      !ftd::eft::evaluate_connected_reservoir_decomposition(
          control, excited, nonorthogonal, {2, 3}, 0.61, options).valid);
  auto incomplete = modes;
  incomplete.pop_back();
  check("incomplete basis fails closed",
      !ftd::eft::evaluate_connected_reservoir_decomposition(
          control, excited, incomplete, {2, 3}, 0.61, options).valid);
  check("duplicate target fails closed",
      !ftd::eft::evaluate_connected_reservoir_decomposition(
          control, excited, modes, {2, 2}, 0.61, options).valid);
  auto graph_mismatch = excited;
  graph_mismatch.charges[0] = -1;
  check("graph mismatch fails closed",
      !ftd::eft::evaluate_connected_reservoir_decomposition(
          control, graph_mismatch, modes, {2, 3}, 0.61, options).valid);
  auto nonfinite = excited;
  nonfinite.electric.x[0] = std::numeric_limits<double>::quiet_NaN();
  check("nonfinite state fails closed",
      !ftd::eft::evaluate_connected_reservoir_decomposition(
          control, nonfinite, modes, {2, 3}, 0.61, options).valid);

  std::cout.precision(17);
  std::cout << "orthonormality_residual="
            << result.mode_orthonormality_residual << '\n'
            << "field_decomposition_residual="
            << result.field_decomposition_residual << '\n'
            << "matter_decomposition_residual="
            << result.matter_decomposition_residual << '\n'
            << "complete_decomposition_residual="
            << result.complete_decomposition_residual << '\n'
            << "connected_reservoir_decomposition failures="
            << failures << '\n';
  return failures == 0 ? 0 : 1;
}
