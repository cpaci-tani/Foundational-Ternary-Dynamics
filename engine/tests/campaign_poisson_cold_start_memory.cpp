/**
 * @file campaign_poisson_cold_start_memory.cpp
 * @brief FTD-0441 matched cold/pre-relaxed Poisson trajectory replay.
 */

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

constexpr int kL = 33;
constexpr int kTicks = 200;
constexpr int kSeparation = 8;
constexpr int kSorIterations = 6;
constexpr int kWarmupTicks = 16;
constexpr std::uint64_t kSeed = 4410;
constexpr double kParticleMomentumGate = 1e-10;
constexpr double kCommonMotionGate = 1e-8;
constexpr double kSuppressionGate = 0.01;

enum class Axis { X = 0, Y = 1, Z = 2 };

const char* axis_name(Axis axis) {
    switch (axis) {
        case Axis::X: return "x";
        case Axis::Y: return "y";
        case Axis::Z: return "z";
    }
    return "unknown";
}

bool finite_vec(const ftd::Vec3& value) {
    return std::isfinite(value.x) && std::isfinite(value.y)
        && std::isfinite(value.z);
}

double wrapped_step(int current, int previous) {
    int delta = current - previous;
    if (delta > kL / 2) delta -= kL;
    if (delta < -kL / 2) delta += kL;
    return static_cast<double>(delta);
}

void configure(ftd::RenderBridge& bridge, bool movement) {
    bridge.force_cpu();
    bridge.seed_rng(kSeed);
    for (const auto& spec : ftd::TOGGLE_SPECS)
        bridge.toggles.*(spec.field) = false;
    bridge.toggles.flux_boundary = ftd::FluxBoundaryMode::Periodic;
    bridge.toggles.bcc_stencil = ftd::BccStencilMode::FULL;
    bridge.set_dt(1.0);
    bridge.set_sor_iterations(kSorIterations);
    bridge.toggles.wave_propagation = true;
    bridge.toggles.coupling = true;
    bridge.toggles.forces = true;
    bridge.toggles.movement = movement;
    bridge.toggles.poisson_coulomb = true;
    bridge.toggles.strict_validation = true;
}

bool toggles_valid(const ftd::RenderBridge& bridge) {
    const auto& t = bridge.toggles;
    return t.wave_propagation && t.coupling && t.forces && t.movement
        && t.poisson_coulomb && t.strict_validation
        && !t.emergent_forces && !t.damping && !t.genesis
        && !t.evaporation && !t.gauss_projection
        && !t.matched_gauss_dynamics && !t.gravity
        && !t.lorentz_force && !t.dual_substrate
        && !t.pair_production && !t.weak_transmutation
        && !t.color_forces && !t.symmetric_movement_order
        && bridge.sor_iterations() == kSorIterations;
}

struct Track {
    bool present = false;
    int x = 0;
    int y = 0;
    int z = 0;
    const ftd::Voxel* voxel = nullptr;
};

Track find_particle(const ftd::RenderBridge& bridge, int particle_id) {
    const auto& voxels = bridge.voxels();
    for (int index = 0; index < static_cast<int>(voxels.size()); ++index) {
        const auto& voxel = voxels[static_cast<std::size_t>(index)];
        if (voxel.state != 0 && voxel.particle_id == particle_id) {
            const auto c = bridge.lattice().coord(index);
            return {true, c.x, c.y, c.z, &voxel};
        }
    }
    return {};
}

std::array<ftd::Vec3, 2> pair_positions(Axis axis) {
    const double center = static_cast<double>(kL / 2);
    const double half = 0.5 * static_cast<double>(kSeparation);
    ftd::Vec3 low{center, center, center};
    ftd::Vec3 high{center, center, center};
    if (axis == Axis::X) { low.x -= half; high.x += half; }
    if (axis == Axis::Y) { low.y -= half; high.y += half; }
    if (axis == Axis::Z) { low.z -= half; high.z += half; }
    return {{low, high}};
}

int inject(ftd::RenderBridge& bridge, const ftd::Vec3& position,
           std::int8_t state) {
    const int x = static_cast<int>(position.x);
    const int y = static_cast<int>(position.y);
    const int z = static_cast<int>(position.z);
    bridge.inject_particle(x, y, z, state, {}, 0, 0);
    return bridge.voxels()[static_cast<std::size_t>(
        bridge.lattice().index(x, y, z))].particle_id;
}

struct Trace {
    Axis axis = Axis::X;
    bool pre_relaxed = false;
    ftd::Vec3 common_displacement{};
    ftd::Vec3 final_particle_momentum{};
    double max_particle_momentum = 0.0;
    double minimum_separation = std::numeric_limits<double>::infinity();
    int plus_hops = 0;
    int minus_hops = 0;
    bool plus_present = false;
    bool minus_present = false;
    bool finite = true;
    bool config_valid = false;
    bool cpu_backend = false;
};

Trace run_arm(Axis axis, bool pre_relaxed) {
    ftd::RenderBridge bridge(kL);
    configure(bridge, !pre_relaxed);
    const auto positions = pair_positions(axis);
    const int plus_id = inject(bridge, positions[0], +1);
    const int minus_id = inject(bridge, positions[1], -1);

    if (pre_relaxed) {
        for (int tick = 0; tick < kWarmupTicks; ++tick) bridge.tick();
        const Track plus = find_particle(bridge, plus_id);
        const Track minus = find_particle(bridge, minus_id);
        if (plus.present)
            bridge.voxels()[static_cast<std::size_t>(
                bridge.lattice().index(plus.x, plus.y, plus.z))].velocity = {};
        if (minus.present)
            bridge.voxels()[static_cast<std::size_t>(
                bridge.lattice().index(minus.x, minus.y, minus.z))].velocity = {};
        bridge.toggles.movement = true;
    }

    Trace trace;
    trace.axis = axis;
    trace.pre_relaxed = pre_relaxed;
    trace.cpu_backend = bridge.backend_kind() == ftd::Backend::Kind::Cpu;

    std::array<int, 3> previous_plus{{
        static_cast<int>(positions[0].x), static_cast<int>(positions[0].y),
        static_cast<int>(positions[0].z)}};
    std::array<int, 3> previous_minus{{
        static_cast<int>(positions[1].x), static_cast<int>(positions[1].y),
        static_cast<int>(positions[1].z)}};
    ftd::Vec3 plus_integer{};
    ftd::Vec3 minus_integer{};

    for (int tick = 0; tick < kTicks; ++tick) {
        bridge.tick();
        const Track plus = find_particle(bridge, plus_id);
        const Track minus = find_particle(bridge, minus_id);
        if (!plus.present || !minus.present) break;

        const double plus_dx = wrapped_step(plus.x, previous_plus[0]);
        const double plus_dy = wrapped_step(plus.y, previous_plus[1]);
        const double plus_dz = wrapped_step(plus.z, previous_plus[2]);
        const double minus_dx = wrapped_step(minus.x, previous_minus[0]);
        const double minus_dy = wrapped_step(minus.y, previous_minus[1]);
        const double minus_dz = wrapped_step(minus.z, previous_minus[2]);
        plus_integer.x += plus_dx;
        plus_integer.y += plus_dy;
        plus_integer.z += plus_dz;
        minus_integer.x += minus_dx;
        minus_integer.y += minus_dy;
        minus_integer.z += minus_dz;
        if (plus_dx != 0.0 || plus_dy != 0.0 || plus_dz != 0.0)
            ++trace.plus_hops;
        if (minus_dx != 0.0 || minus_dy != 0.0 || minus_dz != 0.0)
            ++trace.minus_hops;
        previous_plus = {{plus.x, plus.y, plus.z}};
        previous_minus = {{minus.x, minus.y, minus.z}};

        const auto plus_displacement = plus_integer + plus.voxel->remainder;
        const auto minus_displacement = minus_integer + minus.voxel->remainder;
        const auto relative = (positions[1] + minus_displacement)
            - (positions[0] + plus_displacement);
        trace.minimum_separation = std::min(
            trace.minimum_separation, relative.mag());
        const auto momentum = bridge.energy_audit().particle_momentum;
        trace.max_particle_momentum = std::max(
            trace.max_particle_momentum, momentum.mag());
        trace.finite = trace.finite && finite_vec(momentum);
    }

    const Track plus = find_particle(bridge, plus_id);
    const Track minus = find_particle(bridge, minus_id);
    trace.plus_present = plus.present;
    trace.minus_present = minus.present;
    if (plus.present && minus.present) {
        const auto plus_displacement = plus_integer + plus.voxel->remainder;
        const auto minus_displacement = minus_integer + minus.voxel->remainder;
        trace.common_displacement =
            (plus_displacement + minus_displacement) * 0.5;
    }
    trace.final_particle_momentum = bridge.energy_audit().particle_momentum;
    trace.config_valid = toggles_valid(bridge);
    trace.finite = trace.finite && finite_vec(trace.common_displacement)
        && finite_vec(trace.final_particle_momentum)
        && std::isfinite(trace.max_particle_momentum)
        && std::isfinite(trace.minimum_separation);
    return trace;
}

void print_trace(const Trace& trace) {
    std::cout << "arm,axis," << axis_name(trace.axis)
              << ",pre_relaxed," << (trace.pre_relaxed ? "true" : "false")
              << ",common_x," << trace.common_displacement.x
              << ",common_y," << trace.common_displacement.y
              << ",common_z," << trace.common_displacement.z
              << ",final_particle_x," << trace.final_particle_momentum.x
              << ",final_particle_y," << trace.final_particle_momentum.y
              << ",final_particle_z," << trace.final_particle_momentum.z
              << ",max_particle_momentum," << trace.max_particle_momentum
              << ",minimum_separation," << trace.minimum_separation
              << ",plus_hops," << trace.plus_hops
              << ",minus_hops," << trace.minus_hops
              << ",plus_present," << (trace.plus_present ? "true" : "false")
              << ",minus_present," << (trace.minus_present ? "true" : "false")
              << ",finite," << (trace.finite ? "true" : "false") << '\n';
}

}  // namespace

int main() {
    std::cout << std::setprecision(17);
    std::cout << "FTD-0441 Poisson cold-start memory v1\n";
    std::cout << "protocol,L," << kL << ",ticks," << kTicks
              << ",separation," << kSeparation
              << ",sor_iterations," << kSorIterations
              << ",warmup_ticks," << kWarmupTicks
              << ",seed," << kSeed
              << ",particle_momentum_gate," << kParticleMomentumGate
              << ",common_motion_gate," << kCommonMotionGate
              << ",suppression_gate," << kSuppressionGate << '\n';

    const std::array<Axis, 3> axes{{Axis::X, Axis::Y, Axis::Z}};
    std::vector<Trace> traces;
    traces.reserve(6);
    for (Axis axis : axes) {
        traces.push_back(run_arm(axis, false));
        traces.push_back(run_arm(axis, true));
    }
    for (const auto& trace : traces) print_trace(trace);

    bool valid = true;
    bool cold_resolved = true;
    bool warm_balanced = true;
    bool no_hops = true;
    double max_cold_momentum = 0.0;
    double max_warm_momentum = 0.0;
    double max_cold_common = 0.0;
    double max_warm_common = 0.0;
    for (const auto& trace : traces) {
        valid = valid && trace.finite && trace.config_valid
            && trace.cpu_backend && trace.plus_present && trace.minus_present;
        no_hops = no_hops && trace.plus_hops == 0 && trace.minus_hops == 0;
        if (trace.pre_relaxed) {
            max_warm_momentum = std::max(
                max_warm_momentum, trace.max_particle_momentum);
            max_warm_common = std::max(
                max_warm_common, trace.common_displacement.mag());
            warm_balanced = warm_balanced
                && trace.max_particle_momentum <= kParticleMomentumGate
                && trace.common_displacement.mag() <= kCommonMotionGate;
        } else {
            max_cold_momentum = std::max(
                max_cold_momentum, trace.max_particle_momentum);
            max_cold_common = std::max(
                max_cold_common, trace.common_displacement.mag());
            cold_resolved = cold_resolved
                && (trace.max_particle_momentum > kParticleMomentumGate
                    || trace.common_displacement.mag() > kCommonMotionGate);
        }
    }
    const double momentum_suppression = max_warm_momentum
        / std::max(1e-300, max_cold_momentum);
    const double motion_suppression = max_warm_common
        / std::max(1e-300, max_cold_common);
    const bool suppressed = momentum_suppression <= kSuppressionGate
        && motion_suppression <= kSuppressionGate;

    const char* verdict = "PERSISTENT_MOVEMENT_PHASE_DEFECT";
    if (!valid) verdict = "INVALID_PROTOCOL";
    else if (!cold_resolved) verdict = "NO_COLD_DEFECT_REPRODUCED";
    else if (warm_balanced && suppressed && no_hops)
        verdict = "COLD_START_TRANSIENT_EXPLAINS_POISSON_LEAK";

    std::cout << "summary,max_cold_momentum," << max_cold_momentum
              << ",max_warm_momentum," << max_warm_momentum
              << ",momentum_suppression," << momentum_suppression
              << ",max_cold_common," << max_cold_common
              << ",max_warm_common," << max_warm_common
              << ",motion_suppression," << motion_suppression
              << ",cold_resolved," << (cold_resolved ? "true" : "false")
              << ",warm_balanced," << (warm_balanced ? "true" : "false")
              << ",suppressed," << (suppressed ? "true" : "false")
              << ",no_hops," << (no_hops ? "true" : "false")
              << ",valid," << (valid ? "true" : "false") << '\n';
    std::cout << "verdict," << verdict << '\n';
    return std::string(verdict) == "INVALID_PROTOCOL" ? 1 : 0;
}
