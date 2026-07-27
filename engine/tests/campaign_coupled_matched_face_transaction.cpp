/**
 * @file campaign_coupled_matched_face_transaction.cpp
 * @brief FTD-0479 observer-only coupled matched-face matter/field gate.
 */

#include "ftd/eft/coupled_matched_face_transaction.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

constexpr double kGate = 1e-12;

struct Fixture {
  ftd::eft::CoupledMatchedFaceState state;
  std::vector<double> stationary_density;
  bool valid = false;

  explicit Fixture(int L)
      : state(L),
        stationary_density(static_cast<std::size_t>(L * L * L), 0.0) {}
};

Fixture make_fixture(int L,
                     int charge,
                     const ftd::Vec3& remainder,
                     const ftd::Vec3& velocity,
                     bool transverse) {
  Fixture fixture(L);
  fixture.state.matter.anchor = {L / 2, L / 2, L / 2};
  fixture.state.matter.remainder = remainder;
  fixture.state.matter.momentum =
      ftd::eft::production_flat_momentum(velocity);
  const auto shape = ftd::eft::make_subcell_polarity_shape(
      fixture.state.matter.anchor, remainder, charge);
  if (!shape.valid) return fixture;

  const ftd::Coord sink{1, 1, 1};
  const int sink_index = fixture.state.electric.index(
      sink.x, sink.y, sink.z);
  fixture.stationary_density[static_cast<std::size_t>(sink_index)] =
      -static_cast<double>(charge);
  bool seeded = true;
  for (std::size_t item = 0; item < shape.weight_count; ++item) {
    const auto& weight = shape.weights[item];
    const int source_index = fixture.state.electric.index(
        weight.site.x, weight.site.y, weight.site.z);
    seeded = seeded && ftd::eft::seed_dipole_path(
        fixture.state.electric, source_index, sink_index, weight.weight);
  }
  if (transverse) {
    const auto electric_potential =
        ftd::eft::make_transverse_challenge(L, 0.013);
    seeded = seeded && ftd::eft::apply_transverse_curl(
        fixture.state.electric, electric_potential) > 0.0;
    fixture.state.magnetic_half =
        ftd::eft::make_transverse_challenge(L, 0.017);
  }

  std::vector<double> density = fixture.stationary_density;
  for (std::size_t item = 0; item < shape.weight_count; ++item) {
    const auto& weight = shape.weights[item];
    const int index = fixture.state.electric.index(
        weight.site.x, weight.site.y, weight.site.z);
    density[static_cast<std::size_t>(index)] += weight.weight;
  }
  fixture.valid = seeded
      && ftd::eft::max_fractional_gauss_residual(
          fixture.state.electric, density) <= kGate;
  return fixture;
}

double maximum_identity_residual(
    const ftd::eft::CoupledMatchedFaceTransaction& transaction) {
  return std::max({
      transaction.continuity_residual,
      transaction.gauss_before_residual,
      transaction.gauss_after_residual,
      transaction.force_residual,
      std::abs(transaction.discrete_gradient_residual),
      std::abs(transaction.work_residual),
      std::abs(transaction.field_work_residual),
      std::abs(transaction.total_energy_residual),
      std::abs(transaction.magnetic_work),
      transaction.covariance_residual,
      transaction.causal_speed_excess,
      transaction.inverse.explicit_residual,
      transaction.inverse.inferred_solve_residual,
      transaction.inverse.inferred_state_residual});
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  std::cout << "FTD-0479 coupled matched-face matter-field transaction v1\n";

  ftd::eft::CoupledMatchedFaceOptions options;
  options.gate_tolerance = kGate;
  options.solve_tolerance = 2e-14;
  options.max_iterations = 48;

  bool protocol_valid = true;
  bool identity_pass = true;
  bool inverse_pass = true;
  bool magnetic_nontrivial = false;
  bool electric_components_work_fixed = true;
  bool underderivation_exposed = true;
  int rows = 0;
  int converged_rows = 0;
  double worst_identity = 0.0;
  double worst_continuity = 0.0;
  double worst_gauss = 0.0;
  double worst_work = 0.0;
  double worst_energy = 0.0;
  double worst_covariance = 0.0;
  double worst_causal = 0.0;
  double worst_explicit_inverse = 0.0;
  double worst_inferred_inverse = 0.0;
  double largest_magnetic_impulse = 0.0;

  const std::array<ftd::Vec3, 3> velocities{{
      {0.11, -0.035, 0.021},
      {-0.028, 0.13, 0.041},
      {0.037, -0.019, 0.14}}};
  const std::array<ftd::Vec3, 2> remainders{{
      {0.17, -0.23, 0.11},
      {-0.31, 0.19, -0.07}}};

  for (int L : {11, 12}) {
    for (int charge : {-1, +1}) {
      for (int axis = 0; axis < 3; ++axis) {
        for (bool transverse : {false, true}) {
          const Fixture fixture = make_fixture(
              L, charge, remainders[static_cast<std::size_t>(L % 2)],
              velocities[static_cast<std::size_t>(axis)], transverse);
          protocol_valid = protocol_valid && fixture.valid;
          const auto transaction =
              ftd::eft::solve_coupled_matched_face_transaction(
                  fixture.state, charge, fixture.stationary_density, options);
          ++rows;
          if (transaction.solve.converged) ++converged_rows;
          const double identity = maximum_identity_residual(transaction);
          worst_identity = std::max(worst_identity, identity);
          worst_continuity = std::max(
              worst_continuity, transaction.continuity_residual);
          worst_gauss = std::max({worst_gauss,
              transaction.gauss_before_residual,
              transaction.gauss_after_residual});
          worst_work = std::max(worst_work,
              std::abs(transaction.work_residual));
          worst_energy = std::max(worst_energy,
              std::abs(transaction.total_energy_residual));
          worst_covariance = std::max(
              worst_covariance, transaction.covariance_residual);
          worst_causal = std::max(
              worst_causal, transaction.causal_speed_excess);
          worst_explicit_inverse = std::max(
              worst_explicit_inverse,
              transaction.inverse.explicit_residual);
          worst_inferred_inverse = std::max(
              worst_inferred_inverse,
              transaction.inverse.inferred_state_residual);
          largest_magnetic_impulse = std::max(
              largest_magnetic_impulse,
              transaction.magnetic_impulse.mag());
          magnetic_nontrivial = magnetic_nontrivial
              || (transverse && transaction.magnetic_impulse.mag() > 1e-10);
          electric_components_work_fixed = electric_components_work_fixed
              && !transaction.electric_transverse_rule_underderived;
          underderivation_exposed = underderivation_exposed
              && transaction.magnetic_rule_underderived;
          protocol_valid = protocol_valid && transaction.valid;
          identity_pass = identity_pass && transaction.gates_pass
              && identity <= kGate;
          inverse_pass = inverse_pass
              && transaction.inverse.explicit_available
              && transaction.inverse.inferred_attempted
              && transaction.inverse.inferred_converged
              && transaction.inverse.explicit_residual <= kGate
              && transaction.inverse.inferred_solve_residual <= kGate
              && transaction.inverse.inferred_state_residual <= kGate;
          std::cout << "row,L," << L
                    << ",charge," << charge
                    << ",axis," << axis
                    << ",arm," << (transverse ? "transverse" : "electric")
                    << ",iterations," << transaction.solve.iterations
                    << ",solve," << transaction.solve.residual
                    << ",continuity," << transaction.continuity_residual
                    << ",gauss_before," << transaction.gauss_before_residual
                    << ",gauss_after," << transaction.gauss_after_residual
                    << ",work," << transaction.work_residual
                    << ",field_work," << transaction.field_work_residual
                    << ",energy," << transaction.total_energy_residual
                    << ",magnetic_work," << transaction.magnetic_work
                    << ",covariance," << transaction.covariance_residual
                    << ",speed," << transaction.discrete_gradient_velocity.mag()
                    << ",explicit_inverse,"
                    << transaction.inverse.explicit_residual
                    << ",inferred_inverse,"
                    << transaction.inverse.inferred_state_residual
                    << ",underderived,"
                    << (transaction.magnetic_rule_underderived
                        ? "true" : "false")
                    << ",valid," << (transaction.valid ? "true" : "false")
                    << '\n';
        }
      }
    }
  }

  // A deliberately impossible precision/iteration budget must be reported as
  // nonconverged rather than silently accepted as a physical transaction.
  const Fixture control_fixture = make_fixture(
      11, +1, remainders[0], velocities[0], true);
  auto nonconvergent_options = options;
  nonconvergent_options.solve_tolerance = 1e-30;
  nonconvergent_options.max_iterations = 1;
  nonconvergent_options.infer_inverse = false;
  const auto nonconvergent =
      ftd::eft::solve_coupled_matched_face_transaction(
          control_fixture.state, +1, control_fixture.stationary_density,
          nonconvergent_options);
  const bool nonconvergence_exposed = nonconvergent.solve.attempted
      && !nonconvergent.solve.converged
      && !nonconvergent.valid && !nonconvergent.gates_pass;
  std::cout << "nonconvergence,attempted,"
            << (nonconvergent.solve.attempted ? "true" : "false")
            << ",converged,"
            << (nonconvergent.solve.converged ? "true" : "false")
            << ",iterations," << nonconvergent.solve.iterations
            << ",residual," << nonconvergent.solve.residual
            << ",valid," << (nonconvergent.valid ? "true" : "false")
            << '\n';

  const bool pass = protocol_valid && identity_pass && inverse_pass
      && rows == 24 && converged_rows == rows
      && magnetic_nontrivial && electric_components_work_fixed
      && underderivation_exposed
      && nonconvergence_exposed;
  const std::string verdict = pass
      ? "IDENTITIES_CLOSE_MAGNETIC_GATHER_REMAINS_UNDERDERIVED"
      : "COUPLED_MATCHED_FACE_TRANSACTION_GATE_FAILS";
  std::cout << "summary,rows," << rows
            << ",converged_rows," << converged_rows
            << ",worst_identity," << worst_identity
            << ",worst_continuity," << worst_continuity
            << ",worst_gauss," << worst_gauss
            << ",worst_work," << worst_work
            << ",worst_energy," << worst_energy
            << ",worst_covariance," << worst_covariance
            << ",worst_causal," << worst_causal
            << ",worst_explicit_inverse," << worst_explicit_inverse
            << ",worst_inferred_inverse," << worst_inferred_inverse
            << ",largest_magnetic_impulse," << largest_magnetic_impulse
            << ",electric_components_work_fixed,"
            << (electric_components_work_fixed ? "true" : "false")
            << ",nonconvergence_exposed,"
            << (nonconvergence_exposed ? "true" : "false")
            << ",valid," << (protocol_valid ? "true" : "false")
            << '\n';
  std::cout << "verdict," << verdict << '\n';
  return pass ? 0 : 1;
}
