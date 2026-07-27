/** FTD-0551: quadratic-coat reciprocal discrete-gradient transaction. */

#include "ftd/eft/quadratic_coat_discrete_gradient_transaction.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr int L = 17;
constexpr double gate = 1e-12;
int failures = 0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

struct Fixture {
  ftd::eft::CoupledMatchedFaceState state{L};
  std::vector<double> stationary_density =
      std::vector<double>(static_cast<std::size_t>(L)*L*L, 0.0);
  bool valid = false;
};

Fixture make_fixture(int charge, const ftd::Vec3& velocity,
                     double transverse_amplitude) {
  Fixture fixture;
  fixture.state.matter.anchor = {8, 8, 8};
  fixture.state.matter.remainder = {0.173, -0.219, 0.287};
  fixture.state.matter.momentum =
      ftd::eft::production_flat_momentum(velocity);
  const ftd::Vec3 position{
      fixture.state.matter.anchor.x+fixture.state.matter.remainder.x,
      fixture.state.matter.anchor.y+fixture.state.matter.remainder.y,
      fixture.state.matter.anchor.z+fixture.state.matter.remainder.z};
  const auto coat = ftd::eft::make_quadratic_polarity_coat(position, charge);
  if (!coat.valid) return fixture;
  const ftd::Coord sink{2, 3, 1};
  const int sink_index = fixture.state.electric.index(sink.x, sink.y, sink.z);
  fixture.stationary_density[static_cast<std::size_t>(sink_index)] = -charge;
  bool seeded = true;
  for (std::size_t item = 0; item < coat.weight_count; ++item) {
    const auto& weight = coat.weights[item];
    const int source_index = fixture.state.electric.index(
        weight.site.x, weight.site.y, weight.site.z);
    seeded = seeded && ftd::eft::seed_dipole_path(
        fixture.state.electric, source_index, sink_index, weight.weight);
  }
  if (transverse_amplitude != 0.0) {
    const auto challenge = ftd::eft::make_transverse_challenge(
        L, transverse_amplitude);
    seeded = seeded && ftd::eft::apply_transverse_curl(
        fixture.state.electric, challenge) > 0.0;
    fixture.state.magnetic_half = ftd::eft::make_transverse_challenge(
        L, 1.3*transverse_amplitude);
  }
  std::vector<double> density = fixture.stationary_density;
  for (std::size_t item = 0; item < coat.weight_count; ++item) {
    const auto& weight = coat.weights[item];
    const int i = fixture.state.electric.index(
        weight.site.x, weight.site.y, weight.site.z);
    density[static_cast<std::size_t>(i)] += weight.weight;
  }
  fixture.valid = seeded && ftd::eft::max_fractional_gauss_residual(
      fixture.state.electric, density) <= gate;
  return fixture;
}

Fixture translate_fixture(const Fixture& source, const ftd::Coord& shift) {
  Fixture target;
  target.state.matter = source.state.matter;
  target.state.matter.anchor = {
      (source.state.matter.anchor.x+shift.x+L)%L,
      (source.state.matter.anchor.y+shift.y+L)%L,
      (source.state.matter.anchor.z+shift.z+L)%L};
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const int from = source.state.electric.index(x, y, z);
        const int to = target.state.electric.index(
            x+shift.x, y+shift.y, z+shift.z);
        target.state.electric.x[to] = source.state.electric.x[from];
        target.state.electric.y[to] = source.state.electric.y[from];
        target.state.electric.z[to] = source.state.electric.z[from];
        target.state.magnetic_half.x[to] = source.state.magnetic_half.x[from];
        target.state.magnetic_half.y[to] = source.state.magnetic_half.y[from];
        target.state.magnetic_half.z[to] = source.state.magnetic_half.z[from];
        target.stationary_density[to] = source.stationary_density[from];
      }
  target.valid = source.valid;
  return target;
}

ftd::Vec3 cyclic(const ftd::Vec3& value) {
  return {value.y, value.z, value.x};
}

Fixture cyclic_fixture(const Fixture& source) {
  Fixture target;
  target.state.matter.anchor = {source.state.matter.anchor.y,
                                source.state.matter.anchor.z,
                                source.state.matter.anchor.x};
  target.state.matter.remainder = cyclic(source.state.matter.remainder);
  target.state.matter.momentum = cyclic(source.state.matter.momentum);
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const int from = source.state.electric.index(x, y, z);
        const int to = target.state.electric.index(y, z, x);
        target.state.electric.x[to] = source.state.electric.y[from];
        target.state.electric.y[to] = source.state.electric.z[from];
        target.state.electric.z[to] = source.state.electric.x[from];
        target.state.magnetic_half.x[to] = source.state.magnetic_half.y[from];
        target.state.magnetic_half.y[to] = source.state.magnetic_half.z[from];
        target.state.magnetic_half.z[to] = source.state.magnetic_half.x[from];
        target.stationary_density[to] = source.stationary_density[from];
      }
  target.valid = source.valid;
  return target;
}

double vec_residual(const ftd::Vec3& lhs, const ftd::Vec3& rhs) {
  return std::max({std::abs(lhs.x-rhs.x), std::abs(lhs.y-rhs.y),
                   std::abs(lhs.z-rhs.z)});
}

double maximum_identity(
    const ftd::eft::QuadraticCoatDGTransaction& result) {
  return std::max({result.solve_residual, result.continuity_residual,
      result.gauss_before_residual, result.gauss_after_residual,
      result.force_residual, result.discrete_gradient_residual,
      result.gather.electric_adjoint_residual,
      result.electric_work_residual, result.field_work_residual,
      result.total_energy_residual, result.magnetic_work_residual,
      result.kinematic_residual, result.causal_speed_excess,
      result.inverse_residual});
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  ftd::eft::QuadraticCoatDGOptions options;
  options.gate_tolerance = gate;
  options.solve_tolerance = 2e-14;
  options.max_iterations = 48;
  options.infer_inverse = false;

  const std::array<ftd::Vec3, 6> velocities{{
      {0.08, 0.0, 0.0}, {0.0, -0.11, 0.0}, {0.0, 0.0, 0.14},
      {0.08, 0.07, 0.0}, {-0.06, 0.09, 0.05}, {0.07, 0.08, -0.09}}};
  const std::array<double, 3> amplitudes{{0.0, 0.006, 0.012}};
  const ftd::Coord shift{2, -1, 3};
  int arms = 0;
  int converged = 0;
  int moved = 0;
  double worst_identity = 0.0;
  double worst_solve = 0.0;
  double worst_continuity = 0.0;
  double worst_gauss = 0.0;
  double worst_work = 0.0;
  double worst_energy = 0.0;
  double worst_inverse = 0.0;
  double translation_residual = 0.0;
  bool campaign_valid = true;
  for (int charge : {-1, +1}) {
    for (const auto& velocity : velocities) {
      for (double amplitude : amplitudes) {
        const Fixture base_fixture = make_fixture(
            charge, velocity, amplitude);
        const Fixture shifted_fixture = translate_fixture(base_fixture, shift);
        const auto base = ftd::eft::solve_quadratic_coat_dg_transaction(
            base_fixture.state, charge, base_fixture.stationary_density,
            options);
        const auto shifted = ftd::eft::solve_quadratic_coat_dg_transaction(
            shifted_fixture.state, charge, shifted_fixture.stationary_density,
            options);
        for (const auto* result : {&base, &shifted}) {
          ++arms;
          if (result->solve.converged) ++converged;
          if (result->displacement.mag() > 1e-4) ++moved;
          campaign_valid = campaign_valid && base_fixture.valid
              && shifted_fixture.valid && result->valid && result->gates_pass;
          worst_identity = std::max(worst_identity,
              maximum_identity(*result));
          worst_solve = std::max(worst_solve, result->solve_residual);
          worst_continuity = std::max(worst_continuity,
              result->continuity_residual);
          worst_gauss = std::max({worst_gauss,
              result->gauss_before_residual, result->gauss_after_residual});
          worst_work = std::max({worst_work,
              result->electric_work_residual, result->field_work_residual});
          worst_energy = std::max(worst_energy,
              result->total_energy_residual);
          worst_inverse = std::max(worst_inverse,
              result->inverse_residual);
        }
        translation_residual = std::max({translation_residual,
            vec_residual(base.displacement, shifted.displacement),
            vec_residual(base.after.matter.momentum,
                         shifted.after.matter.momentum),
            vec_residual(base.total_impulse, shifted.total_impulse),
            std::abs(base.current_work-shifted.current_work)});
      }
    }
  }
  check("all 72 neutral periodic transaction arms close",
        campaign_valid && arms == 72 && converged == arms
        && worst_identity <= gate);
  check("registered arms produce nontrivial continuous motion",
        moved == arms);
  check("integer-translated transactions agree",
        translation_residual <= gate);

  const Fixture rotation_base = make_fixture(+1, velocities[4], amplitudes[2]);
  const Fixture rotation_fixture = cyclic_fixture(rotation_base);
  const auto rotation_reference =
      ftd::eft::solve_quadratic_coat_dg_transaction(
          rotation_base.state, +1, rotation_base.stationary_density, options);
  const auto rotation_result =
      ftd::eft::solve_quadratic_coat_dg_transaction(
          rotation_fixture.state, +1, rotation_fixture.stationary_density,
          options);
  const double rotation_residual = std::max({
      vec_residual(cyclic(rotation_reference.displacement),
                   rotation_result.displacement),
      vec_residual(cyclic(rotation_reference.after.matter.momentum),
                   rotation_result.after.matter.momentum),
      vec_residual(cyclic(rotation_reference.total_impulse),
                   rotation_result.total_impulse),
      std::abs(rotation_reference.current_work-rotation_result.current_work)});
  check("cyclic cubic rotation transports the complete transaction",
        rotation_reference.gates_pass && rotation_result.gates_pass
        && rotation_residual <= gate);

  auto impossible = options;
  impossible.solve_tolerance = 1e-30;
  impossible.max_iterations = 1;
  const auto nonconvergent =
      ftd::eft::solve_quadratic_coat_dg_transaction(
          rotation_base.state, +1, rotation_base.stationary_density,
          impossible);
  const auto invalid_charge =
      ftd::eft::solve_quadratic_coat_dg_transaction(
          rotation_base.state, 0, rotation_base.stationary_density, options);
  check("nonconvergence and invalid charge fail closed",
        nonconvergent.solve.attempted && !nonconvergent.solve.converged
        && !nonconvergent.valid && !nonconvergent.gates_pass
        && !invalid_charge.valid && !invalid_charge.gates_pass);

  const bool constructive = failures == 0 && arms == 72
      && worst_identity <= gate && rotation_residual <= gate;
  const char* verdict = constructive
      ? "QUADRATIC_COAT_DG_TRANSACTION_CONSTRUCTIVE"
      : (converged == arms
          ? "QUADRATIC_COAT_DG_TRANSACTION_CLOSED_NEGATIVE"
          : "QUADRATIC_COAT_DG_TRANSACTION_UNRESOLVED");
  std::cout << "arms," << arms << '\n'
            << "converged," << converged << '\n'
            << "moved," << moved << '\n'
            << "worst_identity_residual," << worst_identity << '\n'
            << "worst_solve_residual," << worst_solve << '\n'
            << "worst_continuity_residual," << worst_continuity << '\n'
            << "worst_gauss_residual," << worst_gauss << '\n'
            << "worst_work_residual," << worst_work << '\n'
            << "worst_energy_residual," << worst_energy << '\n'
            << "worst_inverse_residual," << worst_inverse << '\n'
            << "translation_residual," << translation_residual << '\n'
            << "rotation_residual," << rotation_residual << '\n'
            << "verdict," << verdict << '\n';
  return failures == 0 ? 0 : 1;
}
