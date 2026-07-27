/**
 * @file campaign_force_branch_reciprocity.cpp
 * @brief FTD-0439 identical-pair reciprocity matrix for existing force modes.
 *
 * Observer-only comparison.  This campaign changes toggles between registered
 * arms but does not modify any production force, source, stencil, or tick.
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
constexpr std::uint64_t kSeed = 4390;
constexpr double kParticleMomentumGate = 1e-10;
constexpr double kCommonMotionGate = 1e-8;
constexpr double kTotalMomentumGate = 1e-10;
constexpr double kRecoilResolution = 0.10;

enum class Axis { X = 0, Y = 1, Z = 2 };
enum class Mode { MagnitudeGradient, DivergenceGradient, Poisson };

const char* axis_name(Axis axis) {
    switch (axis) {
        case Axis::X: return "x";
        case Axis::Y: return "y";
        case Axis::Z: return "z";
    }
    return "unknown";
}

const char* mode_name(Mode mode) {
    switch (mode) {
        case Mode::MagnitudeGradient: return "magnitude_gradient";
        case Mode::DivergenceGradient: return "divergence_gradient";
        case Mode::Poisson: return "poisson";
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

ftd::Vec3 central_field_momentum(const ftd::RenderBridge& bridge) {
    const auto& lattice = bridge.lattice();
    const auto& voxels = bridge.voxels();
    ftd::Vec3 momentum{};
    for (int x = 0; x < kL; ++x) {
        for (int y = 0; y < kL; ++y) {
            for (int z = 0; z < kL; ++z) {
                const auto& center = voxels[static_cast<std::size_t>(
                    lattice.index(x, y, z))];
                const auto d_x = (voxels[static_cast<std::size_t>(
                    lattice.index(x + 1, y, z))].flux
                    - voxels[static_cast<std::size_t>(
                    lattice.index(x - 1, y, z))].flux) * 0.5;
                const auto d_y = (voxels[static_cast<std::size_t>(
                    lattice.index(x, y + 1, z))].flux
                    - voxels[static_cast<std::size_t>(
                    lattice.index(x, y - 1, z))].flux) * 0.5;
                const auto d_z = (voxels[static_cast<std::size_t>(
                    lattice.index(x, y, z + 1))].flux
                    - voxels[static_cast<std::size_t>(
                    lattice.index(x, y, z - 1))].flux) * 0.5;
                momentum.x -= center.wave_vel.dot(d_x);
                momentum.y -= center.wave_vel.dot(d_y);
                momentum.z -= center.wave_vel.dot(d_z);
            }
        }
    }
    return momentum;
}

void configure(ftd::RenderBridge& bridge, Mode mode) {
    bridge.force_cpu();
    bridge.seed_rng(kSeed);
    for (const auto& spec : ftd::TOGGLE_SPECS)
        bridge.toggles.*(spec.field) = false;
    bridge.toggles.flux_boundary = ftd::FluxBoundaryMode::Periodic;
    bridge.toggles.bcc_stencil = ftd::BccStencilMode::FULL;
    bridge.set_dt(1.0);
    bridge.toggles.wave_propagation = true;
    bridge.toggles.coupling = true;
    bridge.toggles.forces = true;
    bridge.toggles.movement = true;
    bridge.toggles.strict_validation = true;
    bridge.toggles.emergent_forces = mode == Mode::MagnitudeGradient;
    bridge.toggles.poisson_coulomb = mode == Mode::Poisson;
}

bool toggles_valid(const ftd::RenderBridge& bridge, Mode mode) {
    const auto& t = bridge.toggles;
    return t.wave_propagation && t.coupling && t.forces && t.movement
        && t.strict_validation
        && t.emergent_forces == (mode == Mode::MagnitudeGradient)
        && t.poisson_coulomb == (mode == Mode::Poisson)
        && !t.damping && !t.genesis && !t.evaporation
        && !t.gauss_projection && !t.matched_gauss_dynamics
        && !t.gravity && !t.lorentz_force && !t.dual_substrate
        && !t.pair_production && !t.weak_transmutation
        && !t.color_forces && !t.symmetric_movement_order;
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

std::array<ftd::Vec3, 2> low_high_positions(Axis axis) {
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
    Mode mode = Mode::MagnitudeGradient;
    Axis axis = Axis::X;
    int orientation = 1;
    ftd::Vec3 common_displacement{};
    ftd::Vec3 final_particle{};
    ftd::Vec3 final_field{};
    ftd::Vec3 final_total{};
    double max_particle_momentum = 0.0;
    double max_field_momentum = 0.0;
    double max_total_residual = 0.0;
    double minimum_separation = std::numeric_limits<double>::infinity();
    bool plus_present = false;
    bool minus_present = false;
    bool finite = true;
    bool config_valid = false;
    bool cpu_backend = false;
};

Trace run_arm(Mode mode, Axis axis, int orientation) {
    ftd::RenderBridge bridge(kL);
    configure(bridge, mode);
    const auto positions = low_high_positions(axis);
    const auto plus_position = orientation > 0 ? positions[0] : positions[1];
    const auto minus_position = orientation > 0 ? positions[1] : positions[0];
    const int plus_id = inject(bridge, plus_position, +1);
    const int minus_id = inject(bridge, minus_position, -1);

    Trace trace;
    trace.mode = mode;
    trace.axis = axis;
    trace.orientation = orientation;
    trace.config_valid = toggles_valid(bridge, mode);
    trace.cpu_backend = bridge.backend_kind() == ftd::Backend::Kind::Cpu;
    const auto initial_particle = bridge.energy_audit().particle_momentum;
    const auto initial_field = central_field_momentum(bridge);
    const auto initial_total = initial_particle + initial_field;

    std::array<int, 3> previous_plus{{
        static_cast<int>(plus_position.x), static_cast<int>(plus_position.y),
        static_cast<int>(plus_position.z)}};
    std::array<int, 3> previous_minus{{
        static_cast<int>(minus_position.x), static_cast<int>(minus_position.y),
        static_cast<int>(minus_position.z)}};
    ftd::Vec3 plus_integer{};
    ftd::Vec3 minus_integer{};

    for (int tick = 0; tick < kTicks; ++tick) {
        bridge.tick();
        const Track plus = find_particle(bridge, plus_id);
        const Track minus = find_particle(bridge, minus_id);
        if (!plus.present || !minus.present) break;

        plus_integer.x += wrapped_step(plus.x, previous_plus[0]);
        plus_integer.y += wrapped_step(plus.y, previous_plus[1]);
        plus_integer.z += wrapped_step(plus.z, previous_plus[2]);
        minus_integer.x += wrapped_step(minus.x, previous_minus[0]);
        minus_integer.y += wrapped_step(minus.y, previous_minus[1]);
        minus_integer.z += wrapped_step(minus.z, previous_minus[2]);
        previous_plus = {{plus.x, plus.y, plus.z}};
        previous_minus = {{minus.x, minus.y, minus.z}};

        const auto plus_displacement = plus_integer + plus.voxel->remainder;
        const auto minus_displacement = minus_integer + minus.voxel->remainder;
        const auto relative = (minus_position + minus_displacement)
            - (plus_position + plus_displacement);
        trace.minimum_separation = std::min(
            trace.minimum_separation, relative.mag());

        const auto particle = bridge.energy_audit().particle_momentum;
        const auto field = central_field_momentum(bridge);
        const auto total = particle + field;
        trace.max_particle_momentum = std::max(
            trace.max_particle_momentum, particle.mag());
        trace.max_field_momentum = std::max(
            trace.max_field_momentum, field.mag());
        trace.max_total_residual = std::max(
            trace.max_total_residual, (total - initial_total).mag());
        trace.finite = trace.finite && finite_vec(particle)
            && finite_vec(field) && finite_vec(total);
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
    trace.final_particle = bridge.energy_audit().particle_momentum;
    trace.final_field = central_field_momentum(bridge);
    trace.final_total = trace.final_particle + trace.final_field;
    trace.finite = trace.finite && finite_vec(trace.common_displacement)
        && finite_vec(trace.final_particle) && finite_vec(trace.final_field)
        && finite_vec(trace.final_total)
        && std::isfinite(trace.max_particle_momentum)
        && std::isfinite(trace.max_field_momentum)
        && std::isfinite(trace.max_total_residual);
    if (trace.plus_present && trace.minus_present)
        trace.finite = trace.finite && std::isfinite(trace.minimum_separation);
    return trace;
}

struct ModeSummary {
    Mode mode = Mode::MagnitudeGradient;
    double max_common_motion = 0.0;
    double max_particle_momentum = 0.0;
    double max_field_momentum = 0.0;
    double max_total_residual = 0.0;
    bool particle_balanced = true;
    bool total_balanced = true;
    bool opposing_recoil_all = true;
    bool resolved_recoil_all = true;
    bool valid = true;
};

ModeSummary summarize(Mode mode, const std::vector<Trace>& traces) {
    ModeSummary summary;
    summary.mode = mode;
    for (const auto& trace : traces) {
        if (trace.mode != mode) continue;
        summary.valid = summary.valid && trace.finite && trace.config_valid
            && trace.cpu_backend && trace.plus_present && trace.minus_present;
        summary.max_common_motion = std::max(
            summary.max_common_motion, trace.common_displacement.mag());
        summary.max_particle_momentum = std::max(
            summary.max_particle_momentum, trace.max_particle_momentum);
        summary.max_field_momentum = std::max(
            summary.max_field_momentum, trace.max_field_momentum);
        summary.max_total_residual = std::max(
            summary.max_total_residual, trace.max_total_residual);
        const double particle = trace.final_particle.mag();
        const double field = trace.final_field.mag();
        summary.opposing_recoil_all = summary.opposing_recoil_all
            && trace.final_particle.dot(trace.final_field) < 0.0;
        summary.resolved_recoil_all = summary.resolved_recoil_all
            && field >= kRecoilResolution * particle;
    }
    summary.particle_balanced = summary.max_common_motion <= kCommonMotionGate
        && summary.max_particle_momentum <= kParticleMomentumGate;
    summary.total_balanced = summary.max_total_residual <= kTotalMomentumGate;
    return summary;
}

void print_trace(const Trace& trace) {
    std::cout << "arm,mode," << mode_name(trace.mode)
              << ",axis," << axis_name(trace.axis)
              << ",orientation," << trace.orientation
              << ",common_x," << trace.common_displacement.x
              << ",common_y," << trace.common_displacement.y
              << ",common_z," << trace.common_displacement.z
              << ",final_particle_x," << trace.final_particle.x
              << ",final_particle_y," << trace.final_particle.y
              << ",final_particle_z," << trace.final_particle.z
              << ",final_field_x," << trace.final_field.x
              << ",final_field_y," << trace.final_field.y
              << ",final_field_z," << trace.final_field.z
              << ",final_total_x," << trace.final_total.x
              << ",final_total_y," << trace.final_total.y
              << ",final_total_z," << trace.final_total.z
              << ",max_particle_momentum," << trace.max_particle_momentum
              << ",max_field_momentum," << trace.max_field_momentum
              << ",max_total_residual," << trace.max_total_residual
              << ",minimum_separation," << trace.minimum_separation
              << ",plus_present," << (trace.plus_present ? "true" : "false")
              << ",minus_present," << (trace.minus_present ? "true" : "false")
              << ",finite," << (trace.finite ? "true" : "false") << '\n';
}

void print_summary(const ModeSummary& summary) {
    std::cout << "mode_summary,mode," << mode_name(summary.mode)
              << ",max_common_motion," << summary.max_common_motion
              << ",max_particle_momentum," << summary.max_particle_momentum
              << ",max_field_momentum," << summary.max_field_momentum
              << ",max_total_residual," << summary.max_total_residual
              << ",particle_balanced,"
              << (summary.particle_balanced ? "true" : "false")
              << ",total_balanced,"
              << (summary.total_balanced ? "true" : "false")
              << ",opposing_recoil_all,"
              << (summary.opposing_recoil_all ? "true" : "false")
              << ",resolved_recoil_all,"
              << (summary.resolved_recoil_all ? "true" : "false")
              << ",valid," << (summary.valid ? "true" : "false") << '\n';
}

}  // namespace

int main() {
    std::cout << std::setprecision(17);
    std::cout << "FTD-0439 existing-force-branch reciprocity v1\n";
    std::cout << "protocol,L," << kL << ",ticks," << kTicks
              << ",separation," << kSeparation << ",seed," << kSeed
              << ",particle_momentum_gate," << kParticleMomentumGate
              << ",common_motion_gate," << kCommonMotionGate
              << ",total_momentum_gate," << kTotalMomentumGate << '\n';

    const std::array<Mode, 3> modes{{Mode::MagnitudeGradient,
                                    Mode::DivergenceGradient,
                                    Mode::Poisson}};
    const std::array<Axis, 3> axes{{Axis::X, Axis::Y, Axis::Z}};
    std::vector<Trace> traces;
    traces.reserve(18);
    for (Mode mode : modes) {
        for (Axis axis : axes) {
            traces.push_back(run_arm(mode, axis, +1));
            traces.push_back(run_arm(mode, axis, -1));
        }
    }
    for (const auto& trace : traces) print_trace(trace);

    std::array<ModeSummary, 3> summaries{{
        summarize(Mode::MagnitudeGradient, traces),
        summarize(Mode::DivergenceGradient, traces),
        summarize(Mode::Poisson, traces)}};
    for (const auto& summary : summaries) print_summary(summary);

    const bool valid = summaries[0].valid && summaries[1].valid
        && summaries[2].valid;
    const bool mag_bad = !summaries[0].particle_balanced
        || !summaries[0].total_balanced;
    const bool div_bad = !summaries[1].particle_balanced
        || !summaries[1].total_balanced;
    const bool poisson_bad = !summaries[2].particle_balanced
        || !summaries[2].total_balanced;

    const char* verdict = "MIXED_BRANCH_OUTCOME";
    if (!valid) verdict = "INVALID_PROTOCOL";
    else if (mag_bad && !div_bad && !poisson_bad)
        verdict = "MAGNITUDE_GRADIENT_SPECIFIC_DEFECT";
    else if (mag_bad && div_bad && !poisson_bad)
        verdict = "FLUX_FORCE_FAMILY_DEFECT";
    else if (poisson_bad) verdict = "MOVEMENT_OR_PHASE_ORDER_DEFECT";
    else if (!mag_bad && !div_bad && !poisson_bad)
        verdict = "NO_DEFECT_REPRODUCED";

    std::cout << "classification,magnitude_bad," << (mag_bad ? "true" : "false")
              << ",divergence_bad," << (div_bad ? "true" : "false")
              << ",poisson_bad," << (poisson_bad ? "true" : "false")
              << ",valid," << (valid ? "true" : "false") << '\n';
    std::cout << "verdict," << verdict << '\n';
    return std::string(verdict) == "INVALID_PROTOCOL" ? 1 : 0;
}
