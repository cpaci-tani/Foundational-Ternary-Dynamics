/**
 * @file campaign_central_field_momentum_recoil.cpp
 * @brief FTD-0438 central-generator field-recoil audit.
 *
 * Observation only.  The field-momentum candidate uses the same periodic
 * central derivative as the production state-gradient source.  No engine
 * state, force, source, or update order is modified by this campaign.
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
constexpr int kMode = 4;
constexpr int kSeparation = 8;
constexpr double kWaveAmplitude = 0.05;
constexpr std::uint64_t kSeed = 4380;
constexpr double kWaveAbsoluteGate = 1e-10;
constexpr double kWaveRelativeGate = 1e-10;
constexpr double kPairAbsoluteGate = 1e-10;
constexpr double kPairRelativeGate = 1e-6;
constexpr double kRecoilResolution = 0.10;

enum class Axis { X = 0, Y = 1, Z = 2 };

const char* axis_name(Axis axis) {
    switch (axis) {
        case Axis::X: return "x";
        case Axis::Y: return "y";
        case Axis::Z: return "z";
    }
    return "unknown";
}

double component(const ftd::Vec3& value, Axis axis) {
    if (axis == Axis::X) return value.x;
    if (axis == Axis::Y) return value.y;
    return value.z;
}

void set_component(ftd::Vec3& value, Axis axis, double scalar) {
    if (axis == Axis::X) value.x = scalar;
    else if (axis == Axis::Y) value.y = scalar;
    else value.z = scalar;
}

bool finite_vec(const ftd::Vec3& value) {
    return std::isfinite(value.x) && std::isfinite(value.y)
        && std::isfinite(value.z);
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

void clear_toggles(ftd::RenderBridge& bridge) {
    bridge.force_cpu();
    bridge.seed_rng(kSeed);
    for (const auto& spec : ftd::TOGGLE_SPECS)
        bridge.toggles.*(spec.field) = false;
    bridge.toggles.flux_boundary = ftd::FluxBoundaryMode::Periodic;
    bridge.toggles.bcc_stencil = ftd::BccStencilMode::FULL;
    bridge.set_dt(1.0);
    bridge.toggles.strict_validation = true;
}

void configure_wave(ftd::RenderBridge& bridge) {
    clear_toggles(bridge);
    bridge.toggles.wave_propagation = true;
}

void configure_pair(ftd::RenderBridge& bridge) {
    clear_toggles(bridge);
    bridge.toggles.wave_propagation = true;
    bridge.toggles.coupling = true;
    bridge.toggles.forces = true;
    bridge.toggles.movement = true;
    bridge.toggles.emergent_forces = true;
}

bool pair_toggles_valid(const ftd::RenderBridge& bridge) {
    const auto& t = bridge.toggles;
    return t.wave_propagation && t.coupling && t.forces && t.movement
        && t.emergent_forces && t.strict_validation
        && !t.damping && !t.genesis && !t.evaporation
        && !t.gauss_projection && !t.matched_gauss_dynamics
        && !t.gravity && !t.poisson_coulomb && !t.lorentz_force
        && !t.dual_substrate && !t.pair_production
        && !t.weak_transmutation && !t.color_forces
        && !t.symmetric_movement_order;
}

std::array<double, 2> travelling_component(double amplitude, double phase,
                                           double omega) {
    const double sine = std::sin(phase);
    const double cosine = std::cos(phase);
    return {{amplitude * sine,
             amplitude * ((1.0 - std::cos(omega)) * sine
                          - std::sin(omega) * cosine)}};
}

void inject_travelling_wave(ftd::RenderBridge& bridge, Axis propagation) {
    const Axis polarization = propagation == Axis::X ? Axis::Y : Axis::X;
    const double k = 2.0 * ftd::PI * static_cast<double>(kMode)
        / static_cast<double>(kL);
    const double omega = 2.0 * std::asin(
        ftd::C_SPEED * std::abs(std::sin(0.5 * k)));
    auto& voxels = bridge.voxels();
    for (int x = 0; x < kL; ++x) {
        for (int y = 0; y < kL; ++y) {
            for (int z = 0; z < kL; ++z) {
                const int coordinate = propagation == Axis::X ? x
                    : (propagation == Axis::Y ? y : z);
                const auto wave = travelling_component(
                    kWaveAmplitude, k * static_cast<double>(coordinate),
                    omega);
                auto& voxel = voxels[static_cast<std::size_t>(
                    bridge.lattice().index(x, y, z))];
                set_component(voxel.flux, polarization, wave[0]);
                set_component(voxel.wave_vel, polarization, wave[1]);
            }
        }
    }
}

struct WaveTrace {
    Axis axis = Axis::X;
    ftd::Vec3 initial{};
    ftd::Vec3 final{};
    double max_absolute_drift = 0.0;
    double max_relative_drift = 0.0;
    bool finite = true;
    bool cpu_backend = false;
};

WaveTrace run_wave_control(Axis axis) {
    ftd::RenderBridge bridge(kL);
    configure_wave(bridge);
    inject_travelling_wave(bridge, axis);
    WaveTrace trace;
    trace.axis = axis;
    trace.cpu_backend = bridge.backend_kind() == ftd::Backend::Kind::Cpu;
    trace.initial = central_field_momentum(bridge);
    const double scale = std::max(1e-30, trace.initial.mag());
    for (int tick = 0; tick < kTicks; ++tick) {
        bridge.tick();
        const auto current = central_field_momentum(bridge);
        const double drift = (current - trace.initial).mag();
        trace.max_absolute_drift = std::max(
            trace.max_absolute_drift, drift);
        trace.max_relative_drift = std::max(
            trace.max_relative_drift, drift / scale);
        trace.finite = trace.finite && finite_vec(current);
    }
    trace.final = central_field_momentum(bridge);
    trace.finite = trace.finite && finite_vec(trace.initial)
        && finite_vec(trace.final)
        && std::isfinite(trace.max_absolute_drift)
        && std::isfinite(trace.max_relative_drift);
    return trace;
}

bool particle_survives(const ftd::RenderBridge& bridge, int particle_id) {
    for (const auto& voxel : bridge.voxels()) {
        if (voxel.state != 0 && voxel.particle_id == particle_id) return true;
    }
    return false;
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

int inject_particle(ftd::RenderBridge& bridge, const ftd::Vec3& position,
                    std::int8_t state) {
    const int x = static_cast<int>(position.x);
    const int y = static_cast<int>(position.y);
    const int z = static_cast<int>(position.z);
    bridge.inject_particle(x, y, z, state, {}, 0, 0);
    return bridge.voxels()[static_cast<std::size_t>(
        bridge.lattice().index(x, y, z))].particle_id;
}

struct PairTrace {
    Axis axis = Axis::X;
    int orientation = 1;
    ftd::Vec3 initial_particle{};
    ftd::Vec3 initial_field{};
    ftd::Vec3 initial_total{};
    ftd::Vec3 final_particle{};
    ftd::Vec3 final_field{};
    ftd::Vec3 final_total{};
    double max_total_residual = 0.0;
    double max_particle_magnitude = 0.0;
    double max_field_magnitude = 0.0;
    double closure_ratio = 0.0;
    bool plus_present = false;
    bool minus_present = false;
    bool finite = true;
    bool toggles_valid = false;
    bool cpu_backend = false;
};

PairTrace run_pair(Axis axis, int orientation) {
    ftd::RenderBridge bridge(kL);
    configure_pair(bridge);
    const auto positions = low_high_positions(axis);
    const auto plus_position = orientation > 0 ? positions[0] : positions[1];
    const auto minus_position = orientation > 0 ? positions[1] : positions[0];
    const int plus_id = inject_particle(bridge, plus_position, +1);
    const int minus_id = inject_particle(bridge, minus_position, -1);

    PairTrace trace;
    trace.axis = axis;
    trace.orientation = orientation;
    trace.toggles_valid = pair_toggles_valid(bridge);
    trace.cpu_backend = bridge.backend_kind() == ftd::Backend::Kind::Cpu;
    trace.initial_particle = bridge.energy_audit().particle_momentum;
    trace.initial_field = central_field_momentum(bridge);
    trace.initial_total = trace.initial_particle + trace.initial_field;

    for (int tick = 0; tick < kTicks; ++tick) {
        bridge.tick();
        const auto particle = bridge.energy_audit().particle_momentum;
        const auto field = central_field_momentum(bridge);
        const auto total = particle + field;
        trace.max_particle_magnitude = std::max(
            trace.max_particle_magnitude, particle.mag());
        trace.max_field_magnitude = std::max(
            trace.max_field_magnitude, field.mag());
        trace.max_total_residual = std::max(
            trace.max_total_residual, (total - trace.initial_total).mag());
        trace.finite = trace.finite && finite_vec(particle)
            && finite_vec(field) && finite_vec(total);
    }

    trace.final_particle = bridge.energy_audit().particle_momentum;
    trace.final_field = central_field_momentum(bridge);
    trace.final_total = trace.final_particle + trace.final_field;
    const double scale = std::max(
        1e-30, trace.max_particle_magnitude + trace.max_field_magnitude);
    trace.closure_ratio = trace.max_total_residual / scale;
    trace.plus_present = particle_survives(bridge, plus_id);
    trace.minus_present = particle_survives(bridge, minus_id);
    trace.finite = trace.finite && finite_vec(trace.initial_particle)
        && finite_vec(trace.initial_field) && finite_vec(trace.initial_total)
        && finite_vec(trace.final_particle) && finite_vec(trace.final_field)
        && finite_vec(trace.final_total)
        && std::isfinite(trace.max_total_residual)
        && std::isfinite(trace.closure_ratio);
    return trace;
}

void print_wave(const WaveTrace& trace) {
    std::cout << "wave_control,axis," << axis_name(trace.axis)
              << ",initial_x," << trace.initial.x
              << ",initial_y," << trace.initial.y
              << ",initial_z," << trace.initial.z
              << ",final_x," << trace.final.x
              << ",final_y," << trace.final.y
              << ",final_z," << trace.final.z
              << ",max_absolute_drift," << trace.max_absolute_drift
              << ",max_relative_drift," << trace.max_relative_drift
              << ",finite," << (trace.finite ? "true" : "false")
              << '\n';
}

void print_pair(const PairTrace& trace) {
    std::cout << "pair,axis," << axis_name(trace.axis)
              << ",orientation," << trace.orientation
              << ",initial_total_x," << trace.initial_total.x
              << ",initial_total_y," << trace.initial_total.y
              << ",initial_total_z," << trace.initial_total.z
              << ",final_particle_x," << trace.final_particle.x
              << ",final_particle_y," << trace.final_particle.y
              << ",final_particle_z," << trace.final_particle.z
              << ",final_field_x," << trace.final_field.x
              << ",final_field_y," << trace.final_field.y
              << ",final_field_z," << trace.final_field.z
              << ",final_total_x," << trace.final_total.x
              << ",final_total_y," << trace.final_total.y
              << ",final_total_z," << trace.final_total.z
              << ",max_particle_magnitude," << trace.max_particle_magnitude
              << ",max_field_magnitude," << trace.max_field_magnitude
              << ",max_total_residual," << trace.max_total_residual
              << ",closure_ratio," << trace.closure_ratio
              << ",plus_present," << (trace.plus_present ? "true" : "false")
              << ",minus_present," << (trace.minus_present ? "true" : "false")
              << ",finite," << (trace.finite ? "true" : "false")
              << '\n';
}

}  // namespace

int main() {
    std::cout << std::setprecision(17);
    std::cout << "FTD-0438 central-generator field-momentum recoil v1\n";
    std::cout << "protocol,L," << kL << ",ticks," << kTicks
              << ",mode," << kMode << ",wave_amplitude," << kWaveAmplitude
              << ",separation," << kSeparation << ",seed," << kSeed
              << ",wave_absolute_gate," << kWaveAbsoluteGate
              << ",wave_relative_gate," << kWaveRelativeGate
              << ",pair_absolute_gate," << kPairAbsoluteGate
              << ",pair_relative_gate," << kPairRelativeGate << '\n';
    std::cout << "operator,P_field_i=-sum_x_wave_vel_dot_central_i_flux\n";

    const std::array<Axis, 3> axes{{Axis::X, Axis::Y, Axis::Z}};
    std::vector<WaveTrace> waves;
    std::vector<PairTrace> pairs;
    for (Axis axis : axes) waves.push_back(run_wave_control(axis));
    for (Axis axis : axes) {
        pairs.push_back(run_pair(axis, +1));
        pairs.push_back(run_pair(axis, -1));
    }
    for (const auto& wave : waves) print_wave(wave);
    for (const auto& pair : pairs) print_pair(pair);

    bool valid = true;
    bool wave_conserved = true;
    bool pair_closed = true;
    bool recoil_opposes_all = true;
    bool recoil_resolved_all = true;
    double worst_wave_absolute = 0.0;
    double worst_wave_relative = 0.0;
    double worst_pair_absolute = 0.0;
    double worst_pair_relative = 0.0;
    for (const auto& wave : waves) {
        valid = valid && wave.finite && wave.cpu_backend;
        wave_conserved = wave_conserved
            && wave.max_absolute_drift <= kWaveAbsoluteGate
            && wave.max_relative_drift <= kWaveRelativeGate;
        worst_wave_absolute = std::max(
            worst_wave_absolute, wave.max_absolute_drift);
        worst_wave_relative = std::max(
            worst_wave_relative, wave.max_relative_drift);
    }
    for (const auto& pair : pairs) {
        valid = valid && pair.finite && pair.toggles_valid
            && pair.cpu_backend && pair.plus_present && pair.minus_present;
        pair_closed = pair_closed
            && pair.max_total_residual <= kPairAbsoluteGate
            && pair.closure_ratio <= kPairRelativeGate;
        worst_pair_absolute = std::max(
            worst_pair_absolute, pair.max_total_residual);
        worst_pair_relative = std::max(
            worst_pair_relative, pair.closure_ratio);
        const double particle_axial = component(pair.final_particle, pair.axis);
        const double field_axial = component(pair.final_field, pair.axis);
        recoil_opposes_all = recoil_opposes_all
            && particle_axial * field_axial < 0.0;
        recoil_resolved_all = recoil_resolved_all
            && std::abs(field_axial) >= kRecoilResolution
                * std::abs(particle_axial);
    }

    const char* verdict = "MIXED_FIELD_RECOIL";
    if (!valid) verdict = "INVALID_PROTOCOL";
    else if (!wave_conserved) verdict = "CANDIDATE_INVALID";
    else if (pair_closed) verdict = "TOTAL_MOMENTUM_CLOSED_CENTRAL_GENERATOR";
    else if (recoil_opposes_all && recoil_resolved_all)
        verdict = "FIELD_RECOIL_PARTIAL";
    else if (!recoil_opposes_all || !recoil_resolved_all)
        verdict = "NO_COMPENSATING_FIELD_RECOIL";

    std::cout << "summary,worst_wave_absolute," << worst_wave_absolute
              << ",worst_wave_relative," << worst_wave_relative
              << ",worst_pair_absolute," << worst_pair_absolute
              << ",worst_pair_relative," << worst_pair_relative
              << ",wave_conserved," << (wave_conserved ? "true" : "false")
              << ",pair_closed," << (pair_closed ? "true" : "false")
              << ",recoil_opposes_all,"
              << (recoil_opposes_all ? "true" : "false")
              << ",recoil_resolved_all,"
              << (recoil_resolved_all ? "true" : "false")
              << ",valid," << (valid ? "true" : "false") << '\n';
    std::cout << "verdict," << verdict << '\n';
    return std::string(verdict) == "INVALID_PROTOCOL" ? 1 : 0;
}
