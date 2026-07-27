/**
 * @file campaign_face_flux_observer_qualification.cpp
 * @brief FTD-0480 observer-only qualification of the FTD-0478/0479 records.
 *
 * This campaign deliberately cannot promote a production branch.  It checks
 * the algebraic observer over deterministic field families and exact lattice
 * symmetries, while retaining the explicit magnetic-action-origin defect.
 */

#include "ftd/eft/coupled_matched_face_transaction.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr double kGate = 1e-12;

struct Fixture {
  ftd::eft::CoupledMatchedFaceState state;
  std::vector<double> stationary;
  bool valid = false;
  explicit Fixture(int L)
      : state(L), stationary(static_cast<std::size_t>(L * L * L), 0.0) {}
};

int wrap(int value, int L) {
  const int remainder = value % L;
  return remainder < 0 ? remainder + L : remainder;
}

ftd::Coord rotate(const ftd::Coord& value) {
  return {value.y, value.z, value.x};
}

ftd::Vec3 rotate(const ftd::Vec3& value) {
  return {value.y, value.z, value.x};
}

ftd::Coord translate(const ftd::Coord& value, const ftd::Coord& delta,
                     int L) {
  return {wrap(value.x + delta.x, L), wrap(value.y + delta.y, L),
          wrap(value.z + delta.z, L)};
}

template <typename Field>
Field rotate_field(const Field& input) {
  Field output(input.L);
  for (int x = 0; x < input.L; ++x) {
    for (int y = 0; y < input.L; ++y) {
      for (int z = 0; z < input.L; ++z) {
        const ftd::Coord source{x, y, z};
        const ftd::Coord target = rotate(source);
        const int i = input.index(x, y, z);
        const int j = output.index(target.x, target.y, target.z);
        output.x[static_cast<std::size_t>(j)] =
            input.y[static_cast<std::size_t>(i)];
        output.y[static_cast<std::size_t>(j)] =
            input.z[static_cast<std::size_t>(i)];
        output.z[static_cast<std::size_t>(j)] =
            input.x[static_cast<std::size_t>(i)];
      }
    }
  }
  return output;
}

template <typename Field>
Field translate_field(const Field& input, const ftd::Coord& delta) {
  Field output(input.L);
  for (int x = 0; x < input.L; ++x) {
    for (int y = 0; y < input.L; ++y) {
      for (int z = 0; z < input.L; ++z) {
        const ftd::Coord target = translate({x, y, z}, delta, input.L);
        const int i = input.index(x, y, z);
        const int j = output.index(target.x, target.y, target.z);
        output.x[static_cast<std::size_t>(j)] =
            input.x[static_cast<std::size_t>(i)];
        output.y[static_cast<std::size_t>(j)] =
            input.y[static_cast<std::size_t>(i)];
        output.z[static_cast<std::size_t>(j)] =
            input.z[static_cast<std::size_t>(i)];
      }
    }
  }
  return output;
}

std::vector<double> rotate_density(const std::vector<double>& input, int L) {
  ftd::eft::MatchedFaceFlux indexing(L);
  std::vector<double> output(input.size(), 0.0);
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const ftd::Coord target = rotate(ftd::Coord{x, y, z});
        output[static_cast<std::size_t>(
            indexing.index(target.x, target.y, target.z))] =
            input[static_cast<std::size_t>(indexing.index(x, y, z))];
      }
    }
  }
  return output;
}

std::vector<double> translate_density(const std::vector<double>& input, int L,
                                      const ftd::Coord& delta) {
  ftd::eft::MatchedFaceFlux indexing(L);
  std::vector<double> output(input.size(), 0.0);
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const ftd::Coord target = translate({x, y, z}, delta, L);
        output[static_cast<std::size_t>(
            indexing.index(target.x, target.y, target.z))] =
            input[static_cast<std::size_t>(indexing.index(x, y, z))];
      }
    }
  }
  return output;
}

void add_background(ftd::eft::CoupledMatchedFaceState& state,
                    const std::string& family) {
  if (family == "static_dressing" || family == "movement_history") return;
  ftd::eft::MatchedEdgeField potential(state.electric.L);
  const double L = static_cast<double>(state.electric.L);
  for (int x = 0; x < state.electric.L; ++x) {
    for (int y = 0; y < state.electric.L; ++y) {
      for (int z = 0; z < state.electric.L; ++z) {
        const double X = (2.0 * x - L) / L;
        const double Y = (2.0 * y - L) / L;
        const double Z = (2.0 * z - L) / L;
        const int i = potential.index(x, y, z);
        if (family == "affine") {
          potential.x[static_cast<std::size_t>(i)] = 2e-4 * Y;
          potential.y[static_cast<std::size_t>(i)] = -3e-4 * Z;
          potential.z[static_cast<std::size_t>(i)] = 1e-4 * X;
        } else if (family == "quadratic") {
          potential.x[static_cast<std::size_t>(i)] = 2e-4 * Y * Z;
          potential.y[static_cast<std::size_t>(i)] = -2e-4 * Z * X;
          potential.z[static_cast<std::size_t>(i)] = 1e-4 * X * Y;
        } else if (family == "cubic") {
          potential.x[static_cast<std::size_t>(i)] = 8e-5 * X * Y * Z;
          potential.y[static_cast<std::size_t>(i)] = 8e-5 * X * X * Z;
          potential.z[static_cast<std::size_t>(i)] = -8e-5 * Y * Y * X;
        } else if (family == "wave_packet") {
          const double radius2 = X * X + Y * Y + Z * Z;
          const double envelope = std::exp(-5.0 * radius2);
          potential.x[static_cast<std::size_t>(i)] =
              4e-4 * envelope * std::sin(2.0 * 3.14159265358979323846 * Z);
          potential.y[static_cast<std::size_t>(i)] =
              4e-4 * envelope * std::cos(2.0 * 3.14159265358979323846 * X);
        }
      }
    }
  }
  ftd::eft::apply_transverse_curl(state.electric, potential);
  if (family == "wave_packet") state.magnetic_half = potential;
}

Fixture make_fixture(int L, int charge, const ftd::Vec3& velocity,
                     const std::string& family) {
  Fixture fixture(L);
  fixture.state.matter.anchor = {L / 2, L / 2, L / 2};
  fixture.state.matter.remainder = {0.17, -0.23, 0.11};
  fixture.state.matter.momentum = ftd::eft::production_flat_momentum(velocity);
  const auto shape = ftd::eft::make_subcell_polarity_shape(
      fixture.state.matter.anchor, fixture.state.matter.remainder, charge);
  if (!shape.valid) return fixture;
  const ftd::Coord sink{1, 1, 1};
  const int sink_index = fixture.state.electric.index(sink.x, sink.y, sink.z);
  fixture.stationary[static_cast<std::size_t>(sink_index)] = -charge;
  bool seeded = true;
  for (std::size_t item = 0; item < shape.weight_count; ++item) {
    const auto& weight = shape.weights[item];
    const int source_index = fixture.state.electric.index(
        weight.site.x, weight.site.y, weight.site.z);
    seeded = seeded && ftd::eft::seed_dipole_path(
        fixture.state.electric, source_index, sink_index, weight.weight);
  }
  add_background(fixture.state, family);
  std::vector<double> density = fixture.stationary;
  for (std::size_t item = 0; item < shape.weight_count; ++item) {
    const auto& weight = shape.weights[item];
    const int index = fixture.state.electric.index(
        weight.site.x, weight.site.y, weight.site.z);
    density[static_cast<std::size_t>(index)] += weight.weight;
  }
  fixture.valid = seeded && ftd::eft::max_fractional_gauss_residual(
      fixture.state.electric, density) <= kGate;
  return fixture;
}

double transaction_residual(const ftd::eft::CommonActionStepResult& result) {
  return std::max({
      result.continuity_residual, result.gauss_before_residual,
      result.gauss_after_residual, result.force_residual,
      std::abs(result.discrete_gradient_residual),
      std::abs(result.work_residual), std::abs(result.field_work_residual),
      std::abs(result.total_energy_residual), std::abs(result.magnetic_work),
      result.covariance_residual, result.causal_speed_excess,
      result.inverse.explicit_residual,
      result.inverse.inferred_solve_residual,
      result.inverse.inferred_state_residual,
      result.segment.partition_residual, result.segment.first_moment_residual,
      result.segment.locality_residual});
}

double state_difference(const ftd::eft::CoupledMatchedFaceState& expected,
                        const ftd::eft::CoupledMatchedFaceState& actual) {
  const auto effective = [](const ftd::eft::MatchedMatterPoint& point) {
    return ftd::Vec3{point.anchor.x + point.remainder.x,
                     point.anchor.y + point.remainder.y,
                     point.anchor.z + point.remainder.z};
  };
  const ftd::Vec3 dx = effective(expected.matter) - effective(actual.matter);
  const ftd::Vec3 dp = expected.matter.momentum - actual.matter.momentum;
  return std::max({
      ftd::eft::matched_face_max_difference(expected.electric,
                                             actual.electric),
      ftd::eft::matched_edge_max_difference(expected.magnetic_half,
                                             actual.magnetic_half),
      std::abs(dx.x), std::abs(dx.y), std::abs(dx.z),
      std::abs(dp.x), std::abs(dp.y), std::abs(dp.z)});
}

ftd::eft::CoupledMatchedFaceState rotate_state(
    const ftd::eft::CoupledMatchedFaceState& input) {
  ftd::eft::CoupledMatchedFaceState output(input.electric.L);
  output.electric = rotate_field(input.electric);
  output.magnetic_half = rotate_field(input.magnetic_half);
  output.matter.anchor = rotate(input.matter.anchor);
  output.matter.remainder = rotate(input.matter.remainder);
  output.matter.momentum = rotate(input.matter.momentum);
  return output;
}

ftd::eft::CoupledMatchedFaceState translate_state(
    const ftd::eft::CoupledMatchedFaceState& input, const ftd::Coord& delta) {
  ftd::eft::CoupledMatchedFaceState output(input.electric.L);
  output.electric = translate_field(input.electric, delta);
  output.magnetic_half = translate_field(input.magnetic_half, delta);
  output.matter = input.matter;
  output.matter.anchor = translate(input.matter.anchor, delta, input.electric.L);
  return output;
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  std::cout << "FTD-0480 face-flux observer qualification v1\n";
  ftd::eft::CoupledMatchedFaceOptions options;
  options.gate_tolerance = kGate;
  options.solve_tolerance = 2e-14;

  const std::array<std::string, 6> families{{
      "affine", "quadratic", "cubic", "wave_packet",
      "static_dressing", "movement_history"}};
  const std::array<ftd::Vec3, 6> directions{{
      {0.12, 0.0, 0.0}, {-0.12, 0.0, 0.0},
      {0.0, 0.12, 0.0}, {0.0, -0.12, 0.0},
      {0.0, 0.0, 0.12}, {0.0, 0.0, -0.12}}};
  const std::array<ftd::Vec3, 4> diagonals{{
      {0.09, 0.07, 0.05}, {-0.08, 0.06, 0.04},
      {0.07, -0.09, 0.05}, {0.06, 0.04, -0.08}}};

  int rows = 0;
  int passed = 0;
  double worst_identity = 0.0;
  bool action_origin_gate = true;
  for (const auto& family : families) {
    for (int charge : {-1, +1}) {
      for (std::size_t direction_index = 0;
           direction_index < directions.size(); ++direction_index) {
        if (family == "static_dressing" && direction_index > 0) continue;
        const auto& velocity = directions[direction_index];
        const ftd::Vec3 selected_velocity = family == "static_dressing"
            ? ftd::Vec3{} : velocity;
        const Fixture fixture = make_fixture(
            11, charge, selected_velocity, family);
        const ftd::eft::CommonActionStepResult result =
            ftd::eft::solve_coupled_matched_face_transaction(
                fixture.state, charge, fixture.stationary, options);
        const double residual = transaction_residual(result);
        ++rows;
        if (fixture.valid && result.gates_pass && residual <= kGate) ++passed;
        if (!(fixture.valid && result.gates_pass && residual <= kGate)) {
          std::cout << "failed,family," << family
                    << ",charge," << charge
                    << ",vx," << selected_velocity.x
                    << ",vy," << selected_velocity.y
                    << ",vz," << selected_velocity.z
                    << ",fixture," << (fixture.valid ? "true" : "false")
                    << ",valid," << (result.valid ? "true" : "false")
                    << ",converged," << (result.solve.converged ? "true" : "false")
                    << ",iterations," << result.solve.iterations
                    << ",rejected," << result.solve.rejected_steps
                    << ",jacobian," << result.solve.minimum_abs_jacobian_determinant
                    << ",solve," << result.solve.residual
                    << ",identity," << residual
                    << ",inverse," << result.inverse.inferred_state_residual
                    << '\n';
        }
        worst_identity = std::max(worst_identity, residual);
        action_origin_gate = action_origin_gate
            && !result.electric_transverse_rule_underderived
            && !result.magnetic_rule_underderived;
      }
    }
  }
  for (int charge : {-1, +1}) {
    for (const auto& velocity : diagonals) {
      const Fixture fixture = make_fixture(
          11, charge, velocity, "movement_history");
      const auto result = ftd::eft::solve_coupled_matched_face_transaction(
          fixture.state, charge, fixture.stationary, options);
      const double residual = transaction_residual(result);
      ++rows;
      if (fixture.valid && result.gates_pass && residual <= kGate) ++passed;
      worst_identity = std::max(worst_identity, residual);
      action_origin_gate = action_origin_gate
          && !result.electric_transverse_rule_underderived
          && !result.magnetic_rule_underderived;
    }
  }

  const Fixture symmetry = make_fixture(
      11, +1, {0.09, 0.07, 0.05}, "wave_packet");
  const auto base = ftd::eft::solve_coupled_matched_face_transaction(
      symmetry.state, +1, symmetry.stationary, options);

  const auto rotated_before = rotate_state(symmetry.state);
  const auto rotated_density = rotate_density(symmetry.stationary, 11);
  const auto rotated = ftd::eft::solve_coupled_matched_face_transaction(
      rotated_before, +1, rotated_density, options);
  const double rotation_residual = state_difference(
      rotate_state(base.after), rotated.after);

  const ftd::Coord shift{2, -3, 1};
  const auto translated_before = translate_state(symmetry.state, shift);
  const auto translated_density = translate_density(
      symmetry.stationary, 11, shift);
  const auto translated = ftd::eft::solve_coupled_matched_face_transaction(
      translated_before, +1, translated_density, options);
  const double translation_residual = state_difference(
      translate_state(base.after, shift), translated.after);

  const bool algebraic_pass = passed == rows
      && rotation_residual <= kGate && translation_residual <= kGate;
  const bool toggle_gate = algebraic_pass && action_origin_gate;
  const std::string verdict = toggle_gate
      ? "OBSERVER_AND_COMMON_ACTION_ORIGIN_GATES_PASS"
      : (algebraic_pass
          ? "OBSERVER_IDENTITIES_PASS_MAGNETIC_ACTION_ORIGIN_FAILS"
          : "OBSERVER_QUALIFICATION_IDENTITIES_FAIL");
  const bool expected_negative_recorded = rows == 70 && passed > 0
      && passed < rows && rotation_residual <= kGate
      && translation_residual <= kGate && !action_origin_gate;
  std::cout << "summary,rows," << rows
            << ",passed," << passed
            << ",worst_identity," << worst_identity
            << ",rotation_residual," << rotation_residual
            << ",translation_residual," << translation_residual
            << ",action_origin_gate," << (action_origin_gate ? "true" : "false")
            << ",toggle_gate," << (toggle_gate ? "true" : "false") << '\n';
  std::cout << "verdict," << verdict << '\n';
  // This run-of-record is a qualification campaign, not a positive-only unit
  // test.  A reproducible preregistered negative is a successful campaign.
  return (algebraic_pass && !action_origin_gate) || expected_negative_recorded
      ? 0 : 1;
}
