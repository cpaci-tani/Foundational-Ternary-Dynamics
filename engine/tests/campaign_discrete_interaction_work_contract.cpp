/**
 * @file campaign_discrete_interaction_work_contract.cpp
 * @brief FTD-0443 exact hop-work and production-force contract audit.
 */

#include "ftd/eft/discrete_interaction_work.h"
#include "ftd/lagrangian.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

constexpr int kL = 17;
constexpr int kX = 4;
constexpr int kY = 5;
constexpr int kZ = 6;
constexpr std::uint64_t kSeed = 4430;
constexpr double kAlgebraGate = 1e-14;
constexpr double kRemainderGate = 1e-15;
constexpr double kProductionMatchGate = 1e-12;
constexpr double kOppositeCosineGate = -0.999999999999;

bool finite_vec(const ftd::Vec3& value) {
    return std::isfinite(value.x) && std::isfinite(value.y)
        && std::isfinite(value.z);
}

void clear_toggles(ftd::RenderBridge& bridge) {
    bridge.force_cpu();
    bridge.seed_rng(kSeed);
    for (const auto& spec : ftd::TOGGLE_SPECS)
        bridge.toggles.*(spec.field) = false;
    bridge.toggles.flux_boundary = ftd::FluxBoundaryMode::Periodic;
    bridge.toggles.bcc_stencil = ftd::BccStencilMode::FULL;
    bridge.set_dt(1.0);
    bridge.toggles.forces = true;
    bridge.toggles.strict_validation = true;
}

void install_deterministic_flux(ftd::RenderBridge& bridge) {
    const double k = 2.0 * ftd::PI / static_cast<double>(kL);
    for (int x = 0; x < kL; ++x) {
        for (int y = 0; y < kL; ++y) {
            for (int z = 0; z < kL; ++z) {
                auto& voxel = bridge.voxels()[static_cast<std::size_t>(
                    bridge.lattice().index(x, y, z))];
                voxel.flux = {
                    0.31 * std::sin(k * static_cast<double>(x))
                        + 0.07 * std::cos(k * static_cast<double>(y + z)),
                    0.23 * std::sin(k * static_cast<double>(y) + 0.37)
                        + 0.05 * std::cos(k * static_cast<double>(z + x)),
                    0.19 * std::sin(k * static_cast<double>(z) - 0.29)
                        + 0.03 * std::cos(k * static_cast<double>(x + y))};
                voxel.wave_vel = {};
            }
        }
    }
}

std::vector<double> divergence_snapshot(const ftd::RenderBridge& bridge) {
    std::vector<double> divergence(
        static_cast<std::size_t>(bridge.lattice().total_sites()), 0.0);
    for (int i = 0; i < static_cast<int>(divergence.size()); ++i)
        divergence[static_cast<std::size_t>(i)] = bridge.divergence_flux(i);
    return divergence;
}

std::vector<std::int8_t> neutral_state_with_mobile_charge(
    const ftd::RenderBridge& bridge, int mobile_index, std::int8_t charge) {
    std::vector<std::int8_t> state(
        static_cast<std::size_t>(bridge.lattice().total_sites()), 0);
    state[static_cast<std::size_t>(mobile_index)] = charge;
    const int counter = bridge.lattice().index(kX + 6, kY + 5, kZ + 4);
    state[static_cast<std::size_t>(counter)] =
        static_cast<std::int8_t>(-charge);
    return state;
}

double cosine(const ftd::Vec3& left, const ftd::Vec3& right) {
    const double denom = left.mag() * right.mag();
    return denom > 0.0 ? left.dot(right) / denom : 0.0;
}

double relative_vec_error(const ftd::Vec3& actual,
                          const ftd::Vec3& expected) {
    return (actual - expected).mag() / std::max(1e-300, expected.mag());
}

}  // namespace

int main() {
    std::cout << std::setprecision(17);
    std::cout << "FTD-0443 discrete interaction-work contract v1\n";
    std::cout << "protocol,L," << kL << ",site_x," << kX
              << ",site_y," << kY << ",site_z," << kZ
              << ",seed," << kSeed
              << ",algebra_gate," << kAlgebraGate
              << ",remainder_gate," << kRemainderGate
              << ",production_match_gate," << kProductionMatchGate << '\n';

    ftd::RenderBridge algebra_bridge(kL);
    algebra_bridge.force_cpu();
    install_deterministic_flux(algebra_bridge);
    const auto divergence = divergence_snapshot(algebra_bridge);
    const int origin = algebra_bridge.lattice().index(kX, kY, kZ);

    double worst_hop_residual = 0.0;
    double worst_reverse_residual = 0.0;
    bool hop_valid = true;
    int hop_count = 0;
    for (std::int8_t charge : {std::int8_t{+1}, std::int8_t{-1}}) {
        const auto state = neutral_state_with_mobile_charge(
            algebra_bridge, origin, charge);
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dz = -1; dz <= 1; ++dz) {
                    if (dx == 0 && dy == 0 && dz == 0) continue;
                    const int target = algebra_bridge.lattice().index(
                        kX + dx, kY + dy, kZ + dz);
                    const auto hop = ftd::eft::evaluate_discrete_hop_work(
                        state, divergence, static_cast<std::size_t>(origin),
                        static_cast<std::size_t>(target), charge);
                    hop_valid = hop_valid && hop.valid;
                    worst_hop_residual = std::max(
                        worst_hop_residual, std::abs(hop.residual));
                    const double reverse = ftd::eft::discrete_hop_work(
                        charge, divergence[static_cast<std::size_t>(target)],
                        divergence[static_cast<std::size_t>(origin)]);
                    worst_reverse_residual = std::max(
                        worst_reverse_residual,
                        std::abs(hop.endpoint_work + reverse));
                    ++hop_count;
                }
            }
        }
    }

    const auto& lattice = algebra_bridge.lattice();
    const double loop_work =
        ftd::eft::discrete_hop_work(+1,
            divergence[static_cast<std::size_t>(origin)],
            divergence[static_cast<std::size_t>(lattice.index(kX + 1, kY, kZ))])
        + ftd::eft::discrete_hop_work(+1,
            divergence[static_cast<std::size_t>(lattice.index(kX + 1, kY, kZ))],
            divergence[static_cast<std::size_t>(lattice.index(kX + 1, kY + 1, kZ))])
        + ftd::eft::discrete_hop_work(+1,
            divergence[static_cast<std::size_t>(lattice.index(kX + 1, kY + 1, kZ))],
            divergence[static_cast<std::size_t>(lattice.index(kX, kY + 1, kZ))])
        + ftd::eft::discrete_hop_work(+1,
            divergence[static_cast<std::size_t>(lattice.index(kX, kY + 1, kZ))],
            divergence[static_cast<std::size_t>(origin)]);

    const auto central_candidate = ftd::eft::symmetric_interaction_force(
        +1,
        divergence[static_cast<std::size_t>(lattice.index(kX + 1, kY, kZ))],
        divergence[static_cast<std::size_t>(lattice.index(kX - 1, kY, kZ))],
        divergence[static_cast<std::size_t>(lattice.index(kX, kY + 1, kZ))],
        divergence[static_cast<std::size_t>(lattice.index(kX, kY - 1, kZ))],
        divergence[static_cast<std::size_t>(lattice.index(kX, kY, kZ + 1))],
        divergence[static_cast<std::size_t>(lattice.index(kX, kY, kZ - 1))]);
    const auto central_from_bridge = algebra_bridge.gradient_divergence(origin)
        * ftd::G_C;
    const double central_residual =
        (central_candidate - central_from_bridge).mag();

    algebra_bridge.inject_particle(kX, kY, kZ, +1, {}, 0, 0);
    const double coupling_before =
        ftd::compute_lagrangian_diagnostics(algebra_bridge).coupling_sum;
    algebra_bridge.voxel_at(kX, kY, kZ).remainder = {0.31, -0.27, 0.19};
    const double coupling_after =
        ftd::compute_lagrangian_diagnostics(algebra_bridge).coupling_sum;
    const double remainder_action_change = coupling_after - coupling_before;

    ftd::RenderBridge production(kL);
    clear_toggles(production);
    production.inject_particle(kX, kY, kZ, +1, {}, 0, 0);
    install_deterministic_flux(production);
    const int production_index = production.lattice().index(kX, kY, kZ);
    const auto production_gradient =
        production.gradient_divergence(production_index);
    const auto action_force = production_gradient * ftd::G_C;
    const auto copied_production_formula =
        production_gradient * (-ftd::ALPHA);
    production.tick();
    const auto production_force =
        production.force_diag_at(production_index).f_coulomb;

    const double production_formula_residual = relative_vec_error(
        production_force, copied_production_formula);
    const double action_force_relative_error = relative_vec_error(
        production_force, action_force);
    const double production_action_cosine = cosine(
        production_force, action_force);
    const double magnitude_ratio = production_force.mag()
        / std::max(1e-300, action_force.mag());
    const double expected_ratio = ftd::ALPHA / ftd::G_C;
    const double ratio_residual = std::abs(magnitude_ratio - expected_ratio);

    const bool finite = std::isfinite(worst_hop_residual)
        && std::isfinite(worst_reverse_residual) && std::isfinite(loop_work)
        && std::isfinite(central_residual)
        && std::isfinite(remainder_action_change)
        && finite_vec(production_force) && finite_vec(action_force)
        && std::isfinite(production_formula_residual)
        && std::isfinite(action_force_relative_error)
        && std::isfinite(production_action_cosine)
        && std::isfinite(magnitude_ratio) && std::isfinite(ratio_residual);
    const bool algebra_pass = hop_valid && hop_count == 52
        && worst_hop_residual <= kAlgebraGate
        && worst_reverse_residual <= kAlgebraGate
        && std::abs(loop_work) <= kAlgebraGate
        && central_residual <= kAlgebraGate;
    const bool remainder_invariant =
        std::abs(remainder_action_change) <= kRemainderGate;
    const bool production_matches_copy =
        production_formula_residual <= kProductionMatchGate;
    const bool production_matches_action =
        action_force_relative_error <= kProductionMatchGate;
    const bool locked_mismatch_signature =
        production_action_cosine <= kOppositeCosineGate
        && ratio_residual <= kProductionMatchGate;
    const bool config_valid = production.backend_kind()
            == ftd::Backend::Kind::Cpu
        && production.toggles.forces && production.toggles.strict_validation
        && !production.toggles.poisson_coulomb
        && !production.toggles.emergent_forces
        && !production.toggles.movement
        && !production.toggles.wave_propagation
        && !production.toggles.coupling;

    const char* verdict = "PROTOCOL_INVALID";
    if (finite && config_valid && algebra_pass && remainder_invariant
        && production_matches_copy) {
        if (production_matches_action)
            verdict = "EXACT_HOP_WORK_PRODUCTION_MATCH";
        else if (locked_mismatch_signature)
            verdict = "EXACT_HOP_WORK_PRODUCTION_MISMATCH";
        else
            verdict = "UNCLASSIFIED_PRODUCTION_MISMATCH";
    } else if (finite && config_valid && !algebra_pass) {
        verdict = "HOP_IDENTITY_FAILURE";
    }

    std::cout << "hop_algebra,count," << hop_count
              << ",valid," << (hop_valid ? "true" : "false")
              << ",worst_action_residual," << worst_hop_residual
              << ",worst_reverse_residual," << worst_reverse_residual
              << ",loop_work," << loop_work
              << ",central_residual," << central_residual << '\n';
    std::cout << "remainder,coupling_before," << coupling_before
              << ",coupling_after," << coupling_after
              << ",action_change," << remainder_action_change
              << ",invariant," << (remainder_invariant ? "true" : "false")
              << '\n';
    std::cout << "production,gradient_x," << production_gradient.x
              << ",gradient_y," << production_gradient.y
              << ",gradient_z," << production_gradient.z
              << ",force_x," << production_force.x
              << ",force_y," << production_force.y
              << ",force_z," << production_force.z
              << ",action_force_x," << action_force.x
              << ",action_force_y," << action_force.y
              << ",action_force_z," << action_force.z
              << ",copied_formula_residual," << production_formula_residual
              << ",action_relative_error," << action_force_relative_error
              << ",action_cosine," << production_action_cosine
              << ",magnitude_ratio," << magnitude_ratio
              << ",expected_ratio," << expected_ratio
              << ",ratio_residual," << ratio_residual << '\n';
    std::cout << "gates,finite," << (finite ? "true" : "false")
              << ",config_valid," << (config_valid ? "true" : "false")
              << ",algebra_pass," << (algebra_pass ? "true" : "false")
              << ",remainder_invariant,"
              << (remainder_invariant ? "true" : "false")
              << ",production_matches_copy,"
              << (production_matches_copy ? "true" : "false")
              << ",production_matches_action,"
              << (production_matches_action ? "true" : "false")
              << ",locked_mismatch_signature,"
              << (locked_mismatch_signature ? "true" : "false") << '\n';
    std::cout << "verdict," << verdict << '\n';
    return std::string(verdict) == "PROTOCOL_INVALID" ? 1 : 0;
}
