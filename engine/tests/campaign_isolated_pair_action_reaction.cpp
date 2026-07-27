/**
 * @file campaign_isolated_pair_action_reaction.cpp
 * @brief FTD-0437 polarity/injection-order mirror for isolated pair mechanics.
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
constexpr int kTicks = 200;
constexpr int kSeparation = 8;
constexpr std::uint64_t kSeed = 4370;
constexpr double kRepeatGate = 1e-12;
constexpr double kMotionGate = 1e-8;
constexpr double kSymmetryGate = 0.10;

enum class Axis { X = 0, Y = 1, Z = 2 };

const char* axis_name(Axis axis) {
    switch (axis) {
        case Axis::X: return "x";
        case Axis::Y: return "y";
        case Axis::Z: return "z";
    }
    return "unknown";
}

struct Track {
    bool present = false;
    int index = -1;
    int x = 0;
    int y = 0;
    int z = 0;
    const ftd::Voxel* voxel = nullptr;
};

struct Trace {
    Axis axis = Axis::X;
    int orientation = 1;
    bool positive_first = true;
    bool plus_present = false;
    bool minus_present = false;
    bool finite = true;
    bool forbidden_toggles_off = false;
    bool cpu_backend = false;
    ftd::Vec3 plus_displacement{};
    ftd::Vec3 minus_displacement{};
    ftd::Vec3 common_displacement{};
    ftd::Vec3 polarizing_displacement{};
    ftd::Vec3 net_impulse{};
    double net_force_rms = 0.0;
    double minimum_separation = std::numeric_limits<double>::infinity();
    double initial_dynamic_energy = 0.0;
    double final_dynamic_energy = 0.0;
    std::vector<ftd::Vec3> plus_force;
    std::vector<ftd::Vec3> minus_force;
};

double component(const ftd::Vec3& value, Axis axis) {
    if (axis == Axis::X) return value.x;
    if (axis == Axis::Y) return value.y;
    return value.z;
}

double wrapped_step(int current, int previous) {
    int delta = current - previous;
    if (delta > kL / 2) delta -= kL;
    if (delta < -kL / 2) delta += kL;
    return static_cast<double>(delta);
}

bool finite_vec(const ftd::Vec3& value) {
    return std::isfinite(value.x) && std::isfinite(value.y)
        && std::isfinite(value.z);
}

Track find_particle(const ftd::RenderBridge& bridge, int particle_id) {
    const auto& voxels = bridge.voxels();
    for (int index = 0; index < static_cast<int>(voxels.size()); ++index) {
        const auto& voxel = voxels[static_cast<std::size_t>(index)];
        if (voxel.state != 0 && voxel.particle_id == particle_id) {
            const auto coordinate = bridge.lattice().coord(index);
            return {true, index, coordinate.x, coordinate.y, coordinate.z,
                    &voxel};
        }
    }
    return {};
}

void configure(ftd::RenderBridge& bridge) {
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
    bridge.toggles.emergent_forces = true;
    bridge.toggles.strict_validation = true;
}

bool forbidden_toggles_are_off(const ftd::RenderBridge& bridge) {
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

Trace run_arm(Axis axis, int orientation, bool positive_first) {
    ftd::RenderBridge bridge(kL);
    configure(bridge);
    const auto positions = low_high_positions(axis);
    const ftd::Vec3 plus_position = orientation > 0
        ? positions[0] : positions[1];
    const ftd::Vec3 minus_position = orientation > 0
        ? positions[1] : positions[0];

    int plus_id = -1;
    int minus_id = -1;
    if (positive_first) {
        plus_id = inject(bridge, plus_position, +1);
        minus_id = inject(bridge, minus_position, -1);
    } else {
        minus_id = inject(bridge, minus_position, -1);
        plus_id = inject(bridge, plus_position, +1);
    }

    Trace trace;
    trace.axis = axis;
    trace.orientation = orientation;
    trace.positive_first = positive_first;
    trace.forbidden_toggles_off = forbidden_toggles_are_off(bridge);
    trace.cpu_backend = bridge.backend_kind() == ftd::Backend::Kind::Cpu;
    trace.initial_dynamic_energy = bridge.energy_audit().dynamic_energy;
    trace.plus_force.reserve(kTicks);
    trace.minus_force.reserve(kTicks);

    std::array<int, 3> previous_plus{{
        static_cast<int>(plus_position.x), static_cast<int>(plus_position.y),
        static_cast<int>(plus_position.z)}};
    std::array<int, 3> previous_minus{{
        static_cast<int>(minus_position.x), static_cast<int>(minus_position.y),
        static_cast<int>(minus_position.z)}};
    ftd::Vec3 plus_integer{};
    ftd::Vec3 minus_integer{};

    for (int tick = 0; tick < kTicks; ++tick) {
        const Track plus_before = find_particle(bridge, plus_id);
        const Track minus_before = find_particle(bridge, minus_id);
        if (!plus_before.present || !minus_before.present) break;
        bridge.tick();
        trace.plus_force.push_back(
            bridge.force_diag_at(plus_before.index).f_coulomb);
        trace.minus_force.push_back(
            bridge.force_diag_at(minus_before.index).f_coulomb);

        const Track plus_after = find_particle(bridge, plus_id);
        const Track minus_after = find_particle(bridge, minus_id);
        if (!plus_after.present || !minus_after.present) break;
        plus_integer.x += wrapped_step(plus_after.x, previous_plus[0]);
        plus_integer.y += wrapped_step(plus_after.y, previous_plus[1]);
        plus_integer.z += wrapped_step(plus_after.z, previous_plus[2]);
        minus_integer.x += wrapped_step(minus_after.x, previous_minus[0]);
        minus_integer.y += wrapped_step(minus_after.y, previous_minus[1]);
        minus_integer.z += wrapped_step(minus_after.z, previous_minus[2]);
        previous_plus = {{plus_after.x, plus_after.y, plus_after.z}};
        previous_minus = {{minus_after.x, minus_after.y, minus_after.z}};

        const auto plus_displacement = plus_integer
            + plus_after.voxel->remainder;
        const auto minus_displacement = minus_integer
            + minus_after.voxel->remainder;
        const auto relative = (minus_position + minus_displacement)
            - (plus_position + plus_displacement);
        trace.minimum_separation = std::min(
            trace.minimum_separation, relative.mag());
    }

    trace.final_dynamic_energy = bridge.energy_audit().dynamic_energy;
    const Track plus = find_particle(bridge, plus_id);
    const Track minus = find_particle(bridge, minus_id);
    trace.plus_present = plus.present;
    trace.minus_present = minus.present;
    if (plus.present)
        trace.plus_displacement = plus_integer + plus.voxel->remainder;
    if (minus.present)
        trace.minus_displacement = minus_integer + minus.voxel->remainder;
    trace.common_displacement =
        (trace.plus_displacement + trace.minus_displacement) * 0.5;
    trace.polarizing_displacement =
        (trace.plus_displacement - trace.minus_displacement) * 0.5;

    double net_force2 = 0.0;
    const std::size_t count = std::min(
        trace.plus_force.size(), trace.minus_force.size());
    for (std::size_t tick = 0; tick < count; ++tick) {
        const auto net = trace.plus_force[tick] + trace.minus_force[tick];
        trace.net_impulse = trace.net_impulse + net;
        net_force2 += net.mag2();
    }
    if (count > 0)
        trace.net_force_rms = std::sqrt(net_force2 / count);

    trace.finite = finite_vec(trace.plus_displacement)
        && finite_vec(trace.minus_displacement)
        && finite_vec(trace.common_displacement)
        && finite_vec(trace.polarizing_displacement)
        && finite_vec(trace.net_impulse)
        && std::isfinite(trace.net_force_rms)
        && std::isfinite(trace.initial_dynamic_energy)
        && std::isfinite(trace.final_dynamic_energy);
    if (trace.plus_present && trace.minus_present) {
        trace.finite = trace.finite && std::isfinite(trace.minimum_separation)
            && trace.plus_force.size() == kTicks
            && trace.minus_force.size() == kTicks;
    }
    return trace;
}

double trace_difference(const Trace& left, const Trace& right) {
    double maximum = 0.0;
    const auto absorb = [&](double value) {
        maximum = std::max(maximum, std::abs(value));
    };
    const std::array<ftd::Vec3, 5> vectors{{
        left.plus_displacement - right.plus_displacement,
        left.minus_displacement - right.minus_displacement,
        left.common_displacement - right.common_displacement,
        left.polarizing_displacement - right.polarizing_displacement,
        left.net_impulse - right.net_impulse}};
    for (const auto& value : vectors) {
        absorb(value.x); absorb(value.y); absorb(value.z);
    }
    absorb(left.net_force_rms - right.net_force_rms);
    absorb(left.minimum_separation - right.minimum_separation);
    absorb(left.final_dynamic_energy - right.final_dynamic_energy);
    if (left.plus_force.size() != right.plus_force.size()
        || left.minus_force.size() != right.minus_force.size())
        return std::numeric_limits<double>::infinity();
    for (std::size_t tick = 0; tick < left.plus_force.size(); ++tick) {
        const auto plus = left.plus_force[tick] - right.plus_force[tick];
        const auto minus = left.minus_force[tick] - right.minus_force[tick];
        absorb(plus.x); absorb(plus.y); absorb(plus.z);
        absorb(minus.x); absorb(minus.y); absorb(minus.z);
    }
    return maximum;
}

const Trace& find_trace(const std::vector<Trace>& traces, Axis axis,
                        int orientation, bool positive_first) {
    for (const auto& trace : traces) {
        if (trace.axis == axis && trace.orientation == orientation
            && trace.positive_first == positive_first)
            return trace;
    }
    std::abort();
}

double symmetry_residual(double plus, double minus, bool odd) {
    const double numerator = odd ? std::abs(plus + minus)
                                 : std::abs(plus - minus);
    return numerator / std::max(1e-30, std::abs(plus) + std::abs(minus));
}

void print_trace(const Trace& trace) {
    std::cout << "trace,axis," << axis_name(trace.axis)
              << ",orientation," << trace.orientation
              << ",injection_order,"
              << (trace.positive_first ? "positive_first" : "negative_first")
              << ",plus_present," << (trace.plus_present ? "true" : "false")
              << ",minus_present," << (trace.minus_present ? "true" : "false")
              << ",common_x," << trace.common_displacement.x
              << ",common_y," << trace.common_displacement.y
              << ",common_z," << trace.common_displacement.z
              << ",polarizing_x," << trace.polarizing_displacement.x
              << ",polarizing_y," << trace.polarizing_displacement.y
              << ",polarizing_z," << trace.polarizing_displacement.z
              << ",net_impulse_x," << trace.net_impulse.x
              << ",net_impulse_y," << trace.net_impulse.y
              << ",net_impulse_z," << trace.net_impulse.z
              << ",net_force_rms," << trace.net_force_rms
              << ",minimum_separation," << trace.minimum_separation
              << ",dynamic_energy_change,"
              << trace.final_dynamic_energy - trace.initial_dynamic_energy
              << ",finite," << (trace.finite ? "true" : "false") << '\n';
}

}  // namespace

int main() {
    std::cout << std::setprecision(17);
    std::cout << "FTD-0437 isolated-pair action-reaction mirror v1\n";
    std::cout << "protocol,L," << kL << ",ticks," << kTicks
              << ",separation," << kSeparation << ",seed," << kSeed
              << ",repeat_gate," << kRepeatGate
              << ",motion_gate," << kMotionGate << '\n';
    std::cout << "scope,selected_flux_gradient_force_isolated_pair_action_reaction\n";

    const std::array<Axis, 3> axes{{Axis::X, Axis::Y, Axis::Z}};
    std::vector<Trace> traces;
    traces.reserve(12);
    for (Axis axis : axes) {
        for (int orientation : {+1, -1}) {
            traces.push_back(run_arm(axis, orientation, true));
            traces.push_back(run_arm(axis, orientation, false));
        }
    }
    const Trace repeat = run_arm(Axis::Y, +1, true);
    const double repeat_residual = trace_difference(
        find_trace(traces, Axis::Y, +1, true), repeat);

    for (const auto& trace : traces) print_trace(trace);

    bool finite = true;
    bool pair_lost = false;
    bool balanced = true;
    bool motion_all = true;
    double injection_order_residual = 0.0;
    double minimum_common_magnitude = std::numeric_limits<double>::infinity();
    double maximum_common_magnitude = 0.0;
    std::array<double, 6> odd_residuals{};
    std::array<double, 6> even_residuals{};
    int residual_index = 0;

    for (const auto& trace : traces) {
        finite = finite && trace.finite && trace.forbidden_toggles_off
            && trace.cpu_backend;
        pair_lost = pair_lost || !trace.plus_present || !trace.minus_present;
        const double magnitude = trace.common_displacement.mag();
        minimum_common_magnitude = std::min(minimum_common_magnitude, magnitude);
        maximum_common_magnitude = std::max(maximum_common_magnitude, magnitude);
        balanced = balanced && magnitude <= kMotionGate
            && trace.net_force_rms <= kMotionGate;
        motion_all = motion_all && magnitude > kMotionGate
            && trace.net_force_rms > kMotionGate;
    }

    bool odd_all = true;
    bool even_all = true;
    for (Axis axis : axes) {
        for (bool positive_first : {true, false}) {
            const auto& positive = find_trace(
                traces, axis, +1, positive_first);
            const auto& negative = find_trace(
                traces, axis, -1, positive_first);
            const double c_plus = component(
                positive.common_displacement, axis);
            const double c_minus = component(
                negative.common_displacement, axis);
            const double odd = symmetry_residual(c_plus, c_minus, true);
            const double even = symmetry_residual(c_plus, c_minus, false);
            odd_residuals[static_cast<std::size_t>(residual_index)] = odd;
            even_residuals[static_cast<std::size_t>(residual_index)] = even;
            ++residual_index;
            odd_all = odd_all && odd <= kSymmetryGate;
            even_all = even_all && even <= kSymmetryGate;
        }
        for (int orientation : {+1, -1}) {
            injection_order_residual = std::max(
                injection_order_residual,
                trace_difference(find_trace(traces, axis, orientation, true),
                                 find_trace(traces, axis, orientation, false)));
        }
    }

    const bool deterministic = repeat_residual <= kRepeatGate;
    const bool injection_invariant = injection_order_residual <= kRepeatGate;
    const char* verdict = "MIXED_SELF_ACCELERATION";
    if (!finite || !deterministic) verdict = "INVALID_PROTOCOL";
    else if (pair_lost) verdict = "PAIR_ANNIHILATION_OR_LOSS";
    else if (balanced) verdict = "ACTION_REACTION_BALANCED";
    else if (!injection_invariant) verdict = "INJECTION_ORDER_DEPENDENT_RESPONSE";
    else if (motion_all && odd_all) verdict = "DIPOLE_ORIENTED_SELF_PROPULSION";
    else if (motion_all && even_all) verdict = "SCAN_ORIENTATION_SELF_ACCELERATION";

    std::cout << "symmetry";
    for (std::size_t index = 0; index < odd_residuals.size(); ++index)
        std::cout << ",odd_" << index << ',' << odd_residuals[index]
                  << ",even_" << index << ',' << even_residuals[index];
    std::cout << '\n';
    std::cout << "injection_order,max_residual," << injection_order_residual
              << ",invariant," << (injection_invariant ? "true" : "false")
              << '\n';
    std::cout << "motion,min_common," << minimum_common_magnitude
              << ",max_common," << maximum_common_magnitude
              << ",motion_all," << (motion_all ? "true" : "false")
              << ",balanced," << (balanced ? "true" : "false") << '\n';
    std::cout << "repeat,residual," << repeat_residual
              << ",deterministic," << (deterministic ? "true" : "false")
              << '\n';
    std::cout << "gates,finite," << (finite ? "true" : "false")
              << ",pair_lost," << (pair_lost ? "true" : "false")
              << ",injection_invariant,"
              << (injection_invariant ? "true" : "false")
              << ",deterministic," << (deterministic ? "true" : "false")
              << '\n';
    std::cout << "verdict," << verdict << '\n';

    return std::string(verdict) == "INVALID_PROTOCOL" ? 1 : 0;
}
