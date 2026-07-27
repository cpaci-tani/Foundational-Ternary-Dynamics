/**
 * @file campaign_hop_mechanics_underdetermination.cpp
 * @brief FTD-0444 scalar-work sufficiency and selected reversible-map audit.
 */

#include "ftd/constants.h"
#include "ftd/eft/discrete_hop_mechanics.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

constexpr double kWork = 1e-4;
constexpr double kForceWorkGate = 1e-14;
constexpr double kEnergyGate = 1e-13;
constexpr double kRoundTripGate = 1e-12;
constexpr double kDegeneracySeparation = 0.1;
constexpr double kFieldConstraintGate = 1e-15;

struct Basis {
    ftd::Vec3 direction{};
    ftd::Vec3 transverse_1{};
    ftd::Vec3 transverse_2{};
    bool valid = false;
};

Basis make_basis(const ftd::Vec3& displacement) {
    Basis basis;
    if (!(displacement.mag2() > 0.0)) return basis;
    basis.direction = displacement * (1.0 / displacement.mag());
    const ftd::Vec3 reference = std::abs(basis.direction.x) < 0.8
        ? ftd::Vec3{1.0, 0.0, 0.0}
        : ftd::Vec3{0.0, 1.0, 0.0};
    basis.transverse_1 = ftd::Vec3::cross(
        basis.direction, reference);
    basis.transverse_1 *= 1.0 / basis.transverse_1.mag();
    basis.transverse_2 = ftd::Vec3::cross(
        basis.direction, basis.transverse_1);
    basis.transverse_2 *= 1.0 / basis.transverse_2.mag();
    basis.valid = true;
    return basis;
}

ftd::Vec3 sum_vectors(const std::vector<ftd::Vec3>& values) {
    ftd::Vec3 total{};
    for (const auto& value : values) total += value;
    return total;
}

double quadratic_norm(const std::vector<ftd::Vec3>& values) {
    double total = 0.0;
    for (const auto& value : values) total += value.mag2();
    return total;
}

}  // namespace

int main() {
    std::cout << std::setprecision(17);
    std::cout << "FTD-0444 hop-mechanics underdetermination v1\n";
    std::cout << "protocol,work," << kWork
              << ",force_work_gate," << kForceWorkGate
              << ",energy_gate," << kEnergyGate
              << ",round_trip_gate," << kRoundTripGate
              << ",degeneracy_separation," << kDegeneracySeparation
              << ",field_constraint_gate," << kFieldConstraintGate << '\n';

    double worst_force_work_residual = 0.0;
    double minimum_force_separation = std::numeric_limits<double>::infinity();
    double worst_forward_energy_residual = 0.0;
    double worst_reverse_energy_residual = 0.0;
    double worst_round_trip_residual = 0.0;
    double worst_recoil_balance_residual = 0.0;
    bool bases_valid = true;
    bool updates_valid = true;
    int direction_count = 0;

    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dz = -1; dz <= 1; ++dz) {
                if (dx == 0 && dy == 0 && dz == 0) continue;
                const ftd::Vec3 displacement{
                    static_cast<double>(dx), static_cast<double>(dy),
                    static_cast<double>(dz)};
                const Basis basis = make_basis(displacement);
                bases_valid = bases_valid && basis.valid;
                const auto longitudinal = displacement
                    * (kWork / displacement.mag2());
                const std::array<ftd::Vec3, 5> forces{{
                    longitudinal,
                    longitudinal + basis.transverse_1 * 0.2,
                    longitudinal - basis.transverse_1 * 0.2,
                    longitudinal + basis.transverse_2 * 0.3,
                    longitudinal - basis.transverse_2 * 0.3}};
                for (const auto& force : forces) {
                    worst_force_work_residual = std::max(
                        worst_force_work_residual,
                        std::abs(force.dot(displacement) - kWork));
                    minimum_force_separation = std::min(
                        minimum_force_separation,
                        (force - longitudinal).mag() == 0.0
                            ? std::numeric_limits<double>::infinity()
                            : (force - longitudinal).mag());
                }

                const ftd::Vec3 momentum_before =
                    basis.direction * 0.02 + basis.transverse_1 * 0.003;
                const auto forward =
                    ftd::eft::selected_longitudinal_hop_update(
                        momentum_before, displacement, kWork,
                        ftd::M_INERTIAL, ftd::C_SPEED);
                const auto reverse =
                    ftd::eft::selected_longitudinal_hop_update(
                        forward.momentum_after, displacement * -1.0, -kWork,
                        ftd::M_INERTIAL, ftd::C_SPEED);
                updates_valid = updates_valid && forward.valid && reverse.valid;
                worst_forward_energy_residual = std::max(
                    worst_forward_energy_residual,
                    std::abs(forward.work_residual));
                worst_reverse_energy_residual = std::max(
                    worst_reverse_energy_residual,
                    std::abs(reverse.work_residual));
                worst_round_trip_residual = std::max(
                    worst_round_trip_residual,
                    (reverse.momentum_after - momentum_before).mag());
                worst_recoil_balance_residual = std::max(
                    worst_recoil_balance_residual,
                    (forward.momentum_after - momentum_before
                        + forward.required_field_recoil).mag());
                ++direction_count;
            }
        }
    }

    const double rest_energy = ftd::M_INERTIAL;
    const double target_energy = rest_energy + 0.01;
    const double shell_momentum = ftd::C_SPEED * std::sqrt(
        target_energy * target_energy - rest_energy * rest_energy);
    const std::array<ftd::Vec3, 6> shell_states{{
        { shell_momentum, 0.0, 0.0}, {-shell_momentum, 0.0, 0.0},
        {0.0,  shell_momentum, 0.0}, {0.0, -shell_momentum, 0.0},
        {0.0, 0.0,  shell_momentum}, {0.0, 0.0, -shell_momentum}}};
    double worst_shell_energy_residual = 0.0;
    double minimum_shell_separation = std::numeric_limits<double>::infinity();
    for (std::size_t i = 0; i < shell_states.size(); ++i) {
        const double energy = ftd::eft::flat_particle_energy_from_momentum(
            shell_states[i], rest_energy, ftd::C_SPEED);
        worst_shell_energy_residual = std::max(
            worst_shell_energy_residual, std::abs(energy - target_energy));
        for (std::size_t j = i + 1; j < shell_states.size(); ++j)
            minimum_shell_separation = std::min(
                minimum_shell_separation,
                (shell_states[i] - shell_states[j]).mag());
    }

    const ftd::Vec3 required_recoil{0.011, -0.017, 0.023};
    std::vector<ftd::Vec3> deposit_a(4);
    std::vector<ftd::Vec3> deposit_b(4);
    deposit_a[0] = required_recoil;
    deposit_b[3] = required_recoil;
    const double field_momentum_residual =
        (sum_vectors(deposit_a) - sum_vectors(deposit_b)).mag();
    const double field_quadratic_residual = std::abs(
        quadratic_norm(deposit_a) - quadratic_norm(deposit_b));
    double field_configuration_separation = 0.0;
    for (std::size_t i = 0; i < deposit_a.size(); ++i)
        field_configuration_separation +=
            (deposit_a[i] - deposit_b[i]).mag2();
    field_configuration_separation = std::sqrt(
        field_configuration_separation);

    const bool force_degenerate = bases_valid && direction_count == 26
        && worst_force_work_residual <= kForceWorkGate
        && minimum_force_separation >= kDegeneracySeparation;
    const bool momentum_direction_degenerate =
        worst_shell_energy_residual <= kEnergyGate
        && minimum_shell_separation > 0.0;
    const bool field_recoil_degenerate =
        field_momentum_residual <= kFieldConstraintGate
        && field_quadratic_residual <= kFieldConstraintGate
        && field_configuration_separation > 0.0;
    const bool selected_map_reversible = updates_valid
        && worst_forward_energy_residual <= kEnergyGate
        && worst_reverse_energy_residual <= kEnergyGate
        && worst_round_trip_residual <= kRoundTripGate
        && worst_recoil_balance_residual <= kRoundTripGate;
    const bool finite = std::isfinite(worst_force_work_residual)
        && std::isfinite(minimum_force_separation)
        && std::isfinite(worst_shell_energy_residual)
        && std::isfinite(minimum_shell_separation)
        && std::isfinite(field_momentum_residual)
        && std::isfinite(field_quadratic_residual)
        && std::isfinite(field_configuration_separation)
        && std::isfinite(worst_forward_energy_residual)
        && std::isfinite(worst_reverse_energy_residual)
        && std::isfinite(worst_round_trip_residual)
        && std::isfinite(worst_recoil_balance_residual);

    const char* verdict = "PROTOCOL_INVALID";
    if (finite && force_degenerate && momentum_direction_degenerate
        && field_recoil_degenerate && selected_map_reversible)
        verdict = "REVERSIBLE_SELECTED_MAP_BUT_LOCAL_DYNAMICS_UNDERDETERMINED";
    else if (finite && !selected_map_reversible)
        verdict = "SELECTED_MAP_NOT_REVERSIBLE";
    else if (finite && !force_degenerate && !momentum_direction_degenerate
             && !field_recoil_degenerate)
        verdict = "UNIQUE_MECHANICS_FORCED";

    std::cout << "force_family,directions," << direction_count
              << ",worst_work_residual," << worst_force_work_residual
              << ",minimum_transverse_separation," << minimum_force_separation
              << ",degenerate," << (force_degenerate ? "true" : "false")
              << '\n';
    std::cout << "momentum_shell,states," << shell_states.size()
              << ",worst_energy_residual," << worst_shell_energy_residual
              << ",minimum_state_separation," << minimum_shell_separation
              << ",degenerate,"
              << (momentum_direction_degenerate ? "true" : "false") << '\n';
    std::cout << "field_recoil,momentum_residual," << field_momentum_residual
              << ",quadratic_residual," << field_quadratic_residual
              << ",configuration_separation," << field_configuration_separation
              << ",degenerate," << (field_recoil_degenerate ? "true" : "false")
              << '\n';
    std::cout << "selected_map,worst_forward_energy_residual,"
              << worst_forward_energy_residual
              << ",worst_reverse_energy_residual," << worst_reverse_energy_residual
              << ",worst_round_trip_residual," << worst_round_trip_residual
              << ",worst_recoil_balance_residual," << worst_recoil_balance_residual
              << ",reversible," << (selected_map_reversible ? "true" : "false")
              << '\n';
    std::cout << "gates,finite," << (finite ? "true" : "false")
              << ",force_degenerate," << (force_degenerate ? "true" : "false")
              << ",momentum_direction_degenerate,"
              << (momentum_direction_degenerate ? "true" : "false")
              << ",field_recoil_degenerate,"
              << (field_recoil_degenerate ? "true" : "false")
              << ",selected_map_reversible,"
              << (selected_map_reversible ? "true" : "false") << '\n';
    std::cout << "verdict," << verdict << '\n';
    return std::string(verdict) == "PROTOCOL_INVALID" ? 1 : 0;
}
