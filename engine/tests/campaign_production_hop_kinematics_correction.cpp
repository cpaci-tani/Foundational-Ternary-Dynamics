/**
 * @file campaign_production_hop_kinematics_correction.cpp
 * @brief FTD-0450 correction of FTD-0444's selected energy convention.
 */

#include "ftd/eft/discrete_hop_mechanics.h"
#include "ftd/eft/production_hop_kinematics.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

namespace {

constexpr double kWork = 1e-4;
constexpr double kIdentityGate = 1e-13;
constexpr double kRoundTripGate = 1e-12;
constexpr double kOldRelativeMismatchGate = 1.0;

struct Basis {
  ftd::Vec3 direction{};
  ftd::Vec3 transverse{};
};

Basis make_basis(const ftd::Vec3& displacement) {
  Basis basis;
  basis.direction = displacement * (1.0 / displacement.mag());
  const ftd::Vec3 reference = std::abs(basis.direction.x) < 0.8
      ? ftd::Vec3{1.0, 0.0, 0.0}
      : ftd::Vec3{0.0, 1.0, 0.0};
  basis.transverse = ftd::Vec3::cross(basis.direction, reference);
  basis.transverse *= 1.0 / basis.transverse.mag();
  return basis;
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  std::cout << "FTD-0450 production hop kinematics correction v1\n";
  std::cout << "protocol,directions,26,work," << kWork
            << ",identity_gate," << kIdentityGate
            << ",round_trip_gate," << kRoundTripGate
            << ",old_relative_mismatch_gate," << kOldRelativeMismatchGate
            << '\n';

  int direction_count = 0;
  bool updates_valid = true;
  double worst_production_identity_residual = 0.0;
  double worst_velocity_round_trip_residual = 0.0;
  double minimum_old_relative_mismatch =
      std::numeric_limits<double>::infinity();
  double maximum_old_relative_mismatch = 0.0;
  double worst_forward_work_residual = 0.0;
  double worst_reverse_work_residual = 0.0;
  double worst_momentum_round_trip_residual = 0.0;
  double worst_reconstructed_energy_residual = 0.0;
  double worst_recoil_balance_residual = 0.0;

  for (int dx = -1; dx <= 1; ++dx) {
    for (int dy = -1; dy <= 1; ++dy) {
      for (int dz = -1; dz <= 1; ++dz) {
        if (dx == 0 && dy == 0 && dz == 0) continue;
        ++direction_count;
        const ftd::Vec3 displacement{
            static_cast<double>(dx), static_cast<double>(dy),
            static_cast<double>(dz)};
        const Basis basis = make_basis(displacement);
        const ftd::Vec3 velocity =
            basis.direction * 0.15 + basis.transverse * 0.03;
        const ftd::Vec3 momentum =
            ftd::eft::production_flat_momentum(velocity);
        const double production_velocity_energy =
            ftd::flat_particle_energy(velocity.mag2());
        const double corrected_momentum_energy =
            ftd::eft::production_flat_energy_from_momentum(momentum);
        const double old_momentum_energy =
            ftd::eft::flat_particle_energy_from_momentum(
                momentum, ftd::M_INERTIAL, ftd::C_SPEED);
        worst_production_identity_residual = std::max(
            worst_production_identity_residual,
            std::abs(production_velocity_energy - corrected_momentum_energy));
        const ftd::Vec3 reconstructed_velocity =
            ftd::eft::production_flat_velocity_from_momentum(momentum);
        worst_velocity_round_trip_residual = std::max(
            worst_velocity_round_trip_residual,
            (reconstructed_velocity - velocity).mag());
        const double old_relative_mismatch = std::abs(
            old_momentum_energy - production_velocity_energy)
            / production_velocity_energy;
        minimum_old_relative_mismatch = std::min(
            minimum_old_relative_mismatch, old_relative_mismatch);
        maximum_old_relative_mismatch = std::max(
            maximum_old_relative_mismatch, old_relative_mismatch);

        const auto forward = ftd::eft::selected_production_hop_update(
            momentum, displacement, kWork);
        const auto reverse = ftd::eft::selected_production_hop_update(
            forward.momentum_after, displacement * -1.0, -kWork);
        updates_valid = updates_valid && forward.valid && reverse.valid;
        worst_forward_work_residual = std::max(
            worst_forward_work_residual, std::abs(forward.work_residual));
        worst_reverse_work_residual = std::max(
            worst_reverse_work_residual, std::abs(reverse.work_residual));
        worst_momentum_round_trip_residual = std::max(
            worst_momentum_round_trip_residual,
            (reverse.momentum_after - momentum).mag());
        worst_recoil_balance_residual = std::max(
            worst_recoil_balance_residual,
            (forward.momentum_after - momentum
             + forward.required_field_recoil).mag());
        const auto velocity_after =
            ftd::eft::production_flat_velocity_from_momentum(
                forward.momentum_after);
        worst_reconstructed_energy_residual = std::max(
            worst_reconstructed_energy_residual,
            std::abs(ftd::flat_particle_energy(velocity_after.mag2())
                     - forward.energy_after));
      }
    }
  }

  const bool finite = std::isfinite(worst_production_identity_residual)
      && std::isfinite(worst_velocity_round_trip_residual)
      && std::isfinite(minimum_old_relative_mismatch)
      && std::isfinite(maximum_old_relative_mismatch)
      && std::isfinite(worst_forward_work_residual)
      && std::isfinite(worst_reverse_work_residual)
      && std::isfinite(worst_momentum_round_trip_residual)
      && std::isfinite(worst_reconstructed_energy_residual)
      && std::isfinite(worst_recoil_balance_residual);
  const bool production_identity_pass = direction_count == 26
      && worst_production_identity_residual <= kIdentityGate
      && worst_velocity_round_trip_residual <= kIdentityGate
      && worst_reconstructed_energy_residual <= kIdentityGate;
  const bool old_convention_misidentified =
      minimum_old_relative_mismatch >= kOldRelativeMismatchGate;
  const bool corrected_map_reversible = updates_valid
      && worst_forward_work_residual <= kIdentityGate
      && worst_reverse_work_residual <= kIdentityGate
      && worst_momentum_round_trip_residual <= kRoundTripGate
      && worst_recoil_balance_residual <= kRoundTripGate;

  const char* verdict = "PROTOCOL_INVALID";
  if (finite && production_identity_pass && old_convention_misidentified
      && corrected_map_reversible)
    verdict = "PRODUCTION_KINEMATICS_CORRECTS_SELECTED_MAP";
  else if (finite && !old_convention_misidentified
           && corrected_map_reversible)
    verdict = "OLD_CONVENTION_MATCHES_PRODUCTION";

  std::cout << "dispersion_identity,worst_energy_residual,"
            << worst_production_identity_residual
            << ",worst_velocity_round_trip_residual,"
            << worst_velocity_round_trip_residual
            << ",worst_reconstructed_energy_residual,"
            << worst_reconstructed_energy_residual << '\n';
  std::cout << "old_convention,minimum_relative_mismatch,"
            << minimum_old_relative_mismatch
            << ",maximum_relative_mismatch," << maximum_old_relative_mismatch
            << ",misidentified,"
            << (old_convention_misidentified ? "true" : "false") << '\n';
  std::cout << "corrected_map,updates_valid,"
            << (updates_valid ? "true" : "false")
            << ",worst_forward_work_residual," << worst_forward_work_residual
            << ",worst_reverse_work_residual," << worst_reverse_work_residual
            << ",worst_momentum_round_trip_residual,"
            << worst_momentum_round_trip_residual
            << ",worst_recoil_balance_residual,"
            << worst_recoil_balance_residual << '\n';
  std::cout << "gates,finite," << (finite ? "true" : "false")
            << ",production_identity_pass,"
            << (production_identity_pass ? "true" : "false")
            << ",old_convention_misidentified,"
            << (old_convention_misidentified ? "true" : "false")
            << ",corrected_map_reversible,"
            << (corrected_map_reversible ? "true" : "false") << '\n';
  std::cout << "verdict," << verdict << '\n';
  return std::string(verdict) == "PROTOCOL_INVALID" ? 1 : 0;
}
