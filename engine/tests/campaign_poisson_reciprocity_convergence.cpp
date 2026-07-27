/**
 * @file campaign_poisson_reciprocity_convergence.cpp
 * @brief FTD-0440 cold/pre-relaxed Poisson reciprocity convergence audit.
 */

#include "ftd/render_bridge.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

constexpr int kL = 33;
constexpr int kSeparation = 8;
constexpr int kPreRelaxSweeps = 96;
constexpr int kDefaultScientificProbe = 6;
constexpr int kPreRelaxTicks =
    kPreRelaxSweeps / kDefaultScientificProbe;
constexpr std::uint64_t kSeed = 4400;
constexpr double kColdResolutionGate = 1e-12;
constexpr double kConvergedForceGate = 1e-12;
constexpr double kSuppressionRatioGate = 0.01;
constexpr double kMonotonicSlack = 1.01;
constexpr std::array<int, 8> kSorIterations{{1, 2, 4, 6, 12, 24, 48, 96}};

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

void configure(ftd::RenderBridge& bridge, int sor_iterations) {
    bridge.force_cpu();
    bridge.seed_rng(kSeed);
    for (const auto& spec : ftd::TOGGLE_SPECS)
        bridge.toggles.*(spec.field) = false;
    bridge.toggles.flux_boundary = ftd::FluxBoundaryMode::Periodic;
    bridge.toggles.bcc_stencil = ftd::BccStencilMode::FULL;
    bridge.set_dt(1.0);
    bridge.set_sor_iterations(sor_iterations);
    bridge.toggles.forces = true;
    bridge.toggles.poisson_coulomb = true;
    bridge.toggles.strict_validation = true;
}

bool toggles_valid(const ftd::RenderBridge& bridge) {
    const auto& t = bridge.toggles;
    return t.forces && t.poisson_coulomb && t.strict_validation
        && !t.wave_propagation && !t.coupling && !t.movement
        && !t.emergent_forces && !t.damping && !t.genesis
        && !t.evaporation && !t.gauss_projection
        && !t.matched_gauss_dynamics && !t.gravity
        && !t.lorentz_force && !t.dual_substrate
        && !t.pair_production && !t.weak_transmutation
        && !t.color_forces && !t.symmetric_movement_order;
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
    return bridge.lattice().index(x, y, z);
}

struct Trace {
    Axis axis = Axis::X;
    int sor_iterations = 0;
    bool pre_relaxed = false;
    ftd::Vec3 plus_force{};
    ftd::Vec3 minus_force{};
    ftd::Vec3 net_force{};
    ftd::Vec3 particle_momentum{};
    double net_force_magnitude = 0.0;
    bool finite = true;
    bool config_valid = false;
    bool cpu_backend = false;
};

Trace run_arm(Axis axis, int sor_iterations, bool pre_relaxed) {
    ftd::RenderBridge bridge(kL);
    configure(bridge, sor_iterations);
    const auto positions = pair_positions(axis);
    const int plus_index = inject(bridge, positions[0], +1);
    const int minus_index = inject(bridge, positions[1], -1);
    if (pre_relaxed) {
        bridge.set_sor_iterations(kDefaultScientificProbe);
        for (int tick = 0; tick < kPreRelaxTicks; ++tick) bridge.tick();
        bridge.voxels()[static_cast<std::size_t>(plus_index)].velocity = {};
        bridge.voxels()[static_cast<std::size_t>(minus_index)].velocity = {};
        bridge.set_sor_iterations(sor_iterations);
    }
    bridge.tick();

    Trace trace;
    trace.axis = axis;
    trace.sor_iterations = sor_iterations;
    trace.pre_relaxed = pre_relaxed;
    trace.plus_force = bridge.force_diag_at(plus_index).f_coulomb;
    trace.minus_force = bridge.force_diag_at(minus_index).f_coulomb;
    trace.net_force = trace.plus_force + trace.minus_force;
    trace.particle_momentum = bridge.energy_audit().particle_momentum;
    trace.net_force_magnitude = trace.net_force.mag();
    trace.config_valid = toggles_valid(bridge)
        && bridge.sor_iterations() == sor_iterations;
    trace.cpu_backend = bridge.backend_kind() == ftd::Backend::Kind::Cpu;
    trace.finite = finite_vec(trace.plus_force)
        && finite_vec(trace.minus_force) && finite_vec(trace.net_force)
        && finite_vec(trace.particle_momentum)
        && std::isfinite(trace.net_force_magnitude);
    return trace;
}

const Trace& find_trace(const std::vector<Trace>& traces, Axis axis,
                        int sor_iterations, bool pre_relaxed) {
    for (const auto& trace : traces) {
        if (trace.axis == axis && trace.sor_iterations == sor_iterations
            && trace.pre_relaxed == pre_relaxed)
            return trace;
    }
    std::abort();
}

void print_trace(const Trace& trace) {
    std::cout << "arm,axis," << axis_name(trace.axis)
              << ",sor_iterations," << trace.sor_iterations
              << ",pre_relaxed," << (trace.pre_relaxed ? "true" : "false")
              << ",plus_force_x," << trace.plus_force.x
              << ",plus_force_y," << trace.plus_force.y
              << ",plus_force_z," << trace.plus_force.z
              << ",minus_force_x," << trace.minus_force.x
              << ",minus_force_y," << trace.minus_force.y
              << ",minus_force_z," << trace.minus_force.z
              << ",net_force_x," << trace.net_force.x
              << ",net_force_y," << trace.net_force.y
              << ",net_force_z," << trace.net_force.z
              << ",net_force_magnitude," << trace.net_force_magnitude
              << ",particle_momentum_x," << trace.particle_momentum.x
              << ",particle_momentum_y," << trace.particle_momentum.y
              << ",particle_momentum_z," << trace.particle_momentum.z
              << ",finite," << (trace.finite ? "true" : "false") << '\n';
}

}  // namespace

int main() {
    std::cout << std::setprecision(17);
    std::cout << "FTD-0440 Poisson reciprocity convergence v1\n";
    std::cout << "protocol,L," << kL << ",separation," << kSeparation
              << ",pre_relax_sweeps," << kPreRelaxSweeps
              << ",pre_relax_ticks," << kPreRelaxTicks
              << ",seed," << kSeed
              << ",cold_resolution_gate," << kColdResolutionGate
              << ",converged_force_gate," << kConvergedForceGate
              << ",suppression_ratio_gate," << kSuppressionRatioGate
              << ",monotonic_slack," << kMonotonicSlack << '\n';

    std::vector<Trace> traces;
    traces.reserve(18);
    for (int iterations : kSorIterations) {
        traces.push_back(run_arm(Axis::X, iterations, false));
        traces.push_back(run_arm(Axis::X, iterations, true));
    }
    traces.push_back(run_arm(Axis::Y, kDefaultScientificProbe, true));
    traces.push_back(run_arm(Axis::Z, kDefaultScientificProbe, true));
    for (const auto& trace : traces) print_trace(trace);

    bool valid = true;
    for (const auto& trace : traces)
        valid = valid && trace.finite && trace.config_valid
            && trace.cpu_backend;

    bool cold_monotone = true;
    double previous = std::numeric_limits<double>::infinity();
    double max_pre_relaxed = 0.0;
    for (int iterations : kSorIterations) {
        const auto& cold = find_trace(traces, Axis::X, iterations, false);
        const auto& relaxed = find_trace(traces, Axis::X, iterations, true);
        cold_monotone = cold_monotone
            && cold.net_force_magnitude <= previous * kMonotonicSlack;
        previous = cold.net_force_magnitude;
        max_pre_relaxed = std::max(
            max_pre_relaxed, relaxed.net_force_magnitude);
    }
    max_pre_relaxed = std::max(max_pre_relaxed,
        find_trace(traces, Axis::Y, kDefaultScientificProbe, true)
            .net_force_magnitude);
    max_pre_relaxed = std::max(max_pre_relaxed,
        find_trace(traces, Axis::Z, kDefaultScientificProbe, true)
            .net_force_magnitude);

    const double cold6 = find_trace(
        traces, Axis::X, 6, false).net_force_magnitude;
    const double cold96 = find_trace(
        traces, Axis::X, 96, false).net_force_magnitude;
    const double suppression_ratio = cold96 / std::max(1e-300, cold6);
    const bool cold_resolved = cold6 > kColdResolutionGate;
    const bool strongly_suppressed = suppression_ratio <= kSuppressionRatioGate;
    const bool relaxed_balanced = max_pre_relaxed <= kConvergedForceGate;

    const char* verdict = "CONVERGED_POISSON_RECIPROCITY_FLOOR";
    if (!valid) verdict = "INVALID_PROTOCOL";
    else if (cold_resolved && cold_monotone && strongly_suppressed
             && relaxed_balanced)
        verdict = "SOR_TRANSIENT_EXPLAINS_POISSON_LEAK";
    else if (!strongly_suppressed)
        verdict = "NO_SOR_DEPENDENCE";

    std::cout << "summary,cold6," << cold6
              << ",cold96," << cold96
              << ",suppression_ratio," << suppression_ratio
              << ",max_pre_relaxed," << max_pre_relaxed
              << ",cold_monotone," << (cold_monotone ? "true" : "false")
              << ",cold_resolved," << (cold_resolved ? "true" : "false")
              << ",strongly_suppressed,"
              << (strongly_suppressed ? "true" : "false")
              << ",relaxed_balanced," << (relaxed_balanced ? "true" : "false")
              << ",valid," << (valid ? "true" : "false") << '\n';
    std::cout << "verdict," << verdict << '\n';
    return std::string(verdict) == "INVALID_PROTOCOL" ? 1 : 0;
}
