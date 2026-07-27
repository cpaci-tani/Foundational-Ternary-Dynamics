/**
 * @file campaign_neutral_pair_wave_response.cpp
 * @brief FTD-0436 neutral-pair common-mode versus polarization campaign.
 *
 * Observer-only use of the existing selected production flux-gradient force.
 * No production dynamics are added or modified here.
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
constexpr double kAmplitude = 0.05;
constexpr std::uint64_t kSeed = 4360;
constexpr double kRepeatGate = 1e-12;
constexpr double kResponseGate = 1e-8;
constexpr double kDominanceGate = 0.80;
constexpr double kEnergyGate = 1e-6;

enum class PairAxis { Y, Z, None };

const char* axis_name(PairAxis axis) {
    switch (axis) {
        case PairAxis::Y: return "y";
        case PairAxis::Z: return "z";
        case PairAxis::None: return "none";
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

struct PairTrace {
    PairAxis axis = PairAxis::None;
    bool wave = false;
    bool pair_requested = false;
    bool plus_present = false;
    bool minus_present = false;
    bool finite = true;
    bool forbidden_toggles_off = false;
    bool cpu_backend = false;
    ftd::Vec3 plus_displacement{};
    ftd::Vec3 minus_displacement{};
    ftd::Vec3 plus_velocity{};
    ftd::Vec3 minus_velocity{};
    double minimum_separation = std::numeric_limits<double>::infinity();
    double initial_dynamic_energy = 0.0;
    double final_dynamic_energy = 0.0;
    std::vector<ftd::Vec3> plus_force;
    std::vector<ftd::Vec3> minus_force;
};

struct PairResponse {
    PairAxis axis = PairAxis::None;
    bool finite = true;
    bool pair_survived = true;
    double transverse_response_rms = 0.0;
    double transverse_common_rms = 0.0;
    double transverse_polarizing_rms = 0.0;
    double transverse_common_fraction = 0.0;
    double global_common_rms = 0.0;
    double global_polarizing_rms = 0.0;
    double global_common_fraction = 0.0;
    ftd::Vec3 common_displacement{};
    ftd::Vec3 polarizing_displacement{};
    double normalized_energy_closure = 0.0;
};

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
        && !t.weak_transmutation && !t.color_forces;
}

std::array<double, 2> travelling_component(double phase, double omega) {
    const double sine = std::sin(phase);
    const double cosine = std::cos(phase);
    return {{kAmplitude * sine,
             kAmplitude * ((1.0 - std::cos(omega)) * sine
                           - std::sin(omega) * cosine)}};
}

void inject_wave(ftd::RenderBridge& bridge) {
    const double k = 2.0 * ftd::PI * static_cast<double>(kMode)
        / static_cast<double>(kL);
    const double omega = 2.0 * std::asin(
        ftd::C_SPEED * std::abs(std::sin(0.5 * k)));
    auto& voxels = bridge.voxels();
    for (int x = 0; x < kL; ++x) {
        const auto component = travelling_component(
            k * static_cast<double>(x), omega);
        for (int y = 0; y < kL; ++y) {
            for (int z = 0; z < kL; ++z) {
                auto& voxel = voxels[static_cast<std::size_t>(
                    bridge.lattice().index(x, y, z))];
                voxel.flux.y += component[0];
                voxel.wave_vel.y += component[1];
            }
        }
    }
}

std::array<ftd::Vec3, 2> initial_positions(PairAxis axis) {
    const double center = static_cast<double>(kL / 2);
    const double half = 0.5 * static_cast<double>(kSeparation);
    if (axis == PairAxis::Y) {
        return {{{center, center - half, center},
                 {center, center + half, center}}};
    }
    return {{{center, center, center - half},
             {center, center, center + half}}};
}

PairTrace run_arm(PairAxis axis, bool pair, bool wave) {
    ftd::RenderBridge bridge(kL);
    configure(bridge);

    int plus_id = -1;
    int minus_id = -1;
    const auto starts = initial_positions(axis);
    if (pair) {
        const auto& plus = starts[0];
        const auto& minus = starts[1];
        bridge.inject_particle(static_cast<int>(plus.x),
                               static_cast<int>(plus.y),
                               static_cast<int>(plus.z), +1, {}, 0, 0);
        plus_id = bridge.voxels()[static_cast<std::size_t>(
            bridge.lattice().index(static_cast<int>(plus.x),
                                   static_cast<int>(plus.y),
                                   static_cast<int>(plus.z)))].particle_id;
        bridge.inject_particle(static_cast<int>(minus.x),
                               static_cast<int>(minus.y),
                               static_cast<int>(minus.z), -1, {}, 0, 0);
        minus_id = bridge.voxels()[static_cast<std::size_t>(
            bridge.lattice().index(static_cast<int>(minus.x),
                                   static_cast<int>(minus.y),
                                   static_cast<int>(minus.z)))].particle_id;
    }
    if (wave) inject_wave(bridge);

    PairTrace trace;
    trace.axis = axis;
    trace.wave = wave;
    trace.pair_requested = pair;
    trace.forbidden_toggles_off = forbidden_toggles_are_off(bridge);
    trace.cpu_backend = bridge.backend_kind() == ftd::Backend::Kind::Cpu;
    trace.initial_dynamic_energy = bridge.energy_audit().dynamic_energy;
    trace.plus_force.reserve(kTicks);
    trace.minus_force.reserve(kTicks);

    std::array<int, 3> previous_plus{{
        static_cast<int>(starts[0].x), static_cast<int>(starts[0].y),
        static_cast<int>(starts[0].z)}};
    std::array<int, 3> previous_minus{{
        static_cast<int>(starts[1].x), static_cast<int>(starts[1].y),
        static_cast<int>(starts[1].z)}};
    ftd::Vec3 plus_integer{};
    ftd::Vec3 minus_integer{};

    for (int tick = 0; tick < kTicks; ++tick) {
        Track plus_before;
        Track minus_before;
        if (pair) {
            plus_before = find_particle(bridge, plus_id);
            minus_before = find_particle(bridge, minus_id);
            if (!plus_before.present || !minus_before.present) break;
        }

        bridge.tick();

        if (pair) {
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
            const auto relative = (starts[1] + minus_displacement)
                - (starts[0] + plus_displacement);
            trace.minimum_separation = std::min(
                trace.minimum_separation, relative.mag());
        }
    }

    trace.final_dynamic_energy = bridge.energy_audit().dynamic_energy;
    if (pair) {
        const Track plus = find_particle(bridge, plus_id);
        const Track minus = find_particle(bridge, minus_id);
        trace.plus_present = plus.present;
        trace.minus_present = minus.present;
        if (plus.present) {
            trace.plus_displacement = plus_integer + plus.voxel->remainder;
            trace.plus_velocity = plus.voxel->velocity;
        }
        if (minus.present) {
            trace.minus_displacement = minus_integer + minus.voxel->remainder;
            trace.minus_velocity = minus.voxel->velocity;
        }
    }

    trace.finite = std::isfinite(trace.initial_dynamic_energy)
        && std::isfinite(trace.final_dynamic_energy)
        && finite_vec(trace.plus_displacement)
        && finite_vec(trace.minus_displacement)
        && finite_vec(trace.plus_velocity) && finite_vec(trace.minus_velocity);
    if (pair && trace.plus_present && trace.minus_present) {
        trace.finite = trace.finite && std::isfinite(trace.minimum_separation)
            && trace.plus_force.size() == kTicks
            && trace.minus_force.size() == kTicks;
    }
    for (const auto& force : trace.plus_force)
        trace.finite = trace.finite && finite_vec(force);
    for (const auto& force : trace.minus_force)
        trace.finite = trace.finite && finite_vec(force);
    return trace;
}

PairResponse make_response(const PairTrace& combined,
                           const PairTrace& pair_only,
                           const PairTrace& wave_only) {
    PairResponse response;
    response.axis = combined.axis;
    response.pair_survived = combined.plus_present && combined.minus_present
        && pair_only.plus_present && pair_only.minus_present;
    response.finite = combined.finite && pair_only.finite && wave_only.finite
        && combined.forbidden_toggles_off
        && pair_only.forbidden_toggles_off
        && wave_only.forbidden_toggles_off
        && combined.cpu_backend && pair_only.cpu_backend
        && wave_only.cpu_backend;

    if (!response.pair_survived
        || combined.plus_force.size() != pair_only.plus_force.size()
        || combined.minus_force.size() != pair_only.minus_force.size()) {
        response.finite = response.finite && response.pair_survived;
        return response;
    }

    double common_y2 = 0.0;
    double polarizing_y2 = 0.0;
    double common2 = 0.0;
    double polarizing2 = 0.0;
    double response_y2 = 0.0;
    const std::size_t count = combined.plus_force.size();
    for (std::size_t tick = 0; tick < count; ++tick) {
        const auto plus = combined.plus_force[tick]
            - pair_only.plus_force[tick];
        const auto minus = combined.minus_force[tick]
            - pair_only.minus_force[tick];
        const auto common = (plus + minus) * 0.5;
        const auto polarizing = (plus - minus) * 0.5;
        common_y2 += common.y * common.y;
        polarizing_y2 += polarizing.y * polarizing.y;
        common2 += common.mag2();
        polarizing2 += polarizing.mag2();
        response_y2 += 0.5 * (plus.y * plus.y + minus.y * minus.y);
    }
    response.transverse_common_rms = std::sqrt(common_y2 / count);
    response.transverse_polarizing_rms = std::sqrt(polarizing_y2 / count);
    response.transverse_response_rms = std::sqrt(response_y2 / count);
    response.global_common_rms = std::sqrt(common2 / count);
    response.global_polarizing_rms = std::sqrt(polarizing2 / count);
    response.transverse_common_fraction = response.transverse_common_rms
        / std::max(1e-30, response.transverse_common_rms
                          + response.transverse_polarizing_rms);
    response.global_common_fraction = response.global_common_rms
        / std::max(1e-30, response.global_common_rms
                          + response.global_polarizing_rms);

    const auto plus_displacement = combined.plus_displacement
        - pair_only.plus_displacement;
    const auto minus_displacement = combined.minus_displacement
        - pair_only.minus_displacement;
    response.common_displacement = (plus_displacement + minus_displacement)
        * 0.5;
    response.polarizing_displacement =
        (plus_displacement - minus_displacement) * 0.5;

    const double combined_change = combined.final_dynamic_energy
        - combined.initial_dynamic_energy;
    const double pair_change = pair_only.final_dynamic_energy
        - pair_only.initial_dynamic_energy;
    const double wave_change = wave_only.final_dynamic_energy
        - wave_only.initial_dynamic_energy;
    const double scale = std::max(
        1e-30, std::abs(pair_only.initial_dynamic_energy)
             + std::abs(wave_only.initial_dynamic_energy));
    response.normalized_energy_closure = std::abs(
        combined_change - pair_change - wave_change) / scale;
    response.finite = response.finite
        && std::isfinite(response.transverse_response_rms)
        && std::isfinite(response.transverse_common_fraction)
        && std::isfinite(response.global_common_fraction)
        && std::isfinite(response.normalized_energy_closure)
        && finite_vec(response.common_displacement)
        && finite_vec(response.polarizing_displacement);
    return response;
}

double trace_difference(const PairTrace& left, const PairTrace& right) {
    double maximum = 0.0;
    const auto absorb = [&](double value) {
        maximum = std::max(maximum, std::abs(value));
    };
    const std::array<ftd::Vec3, 4> vectors{{
        left.plus_displacement - right.plus_displacement,
        left.minus_displacement - right.minus_displacement,
        left.plus_velocity - right.plus_velocity,
        left.minus_velocity - right.minus_velocity}};
    for (const auto& vector : vectors) {
        absorb(vector.x); absorb(vector.y); absorb(vector.z);
    }
    absorb(left.minimum_separation - right.minimum_separation);
    absorb(left.initial_dynamic_energy - right.initial_dynamic_energy);
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

void print_trace(const char* kind, const PairTrace& trace) {
    std::cout << "trace,kind," << kind
              << ",axis," << axis_name(trace.axis)
              << ",wave," << (trace.wave ? "true" : "false")
              << ",plus_present," << (trace.plus_present ? "true" : "false")
              << ",minus_present," << (trace.minus_present ? "true" : "false")
              << ",plus_dx," << trace.plus_displacement.x
              << ",plus_dy," << trace.plus_displacement.y
              << ",plus_dz," << trace.plus_displacement.z
              << ",minus_dx," << trace.minus_displacement.x
              << ",minus_dy," << trace.minus_displacement.y
              << ",minus_dz," << trace.minus_displacement.z
              << ",minimum_separation," << trace.minimum_separation
              << ",initial_dynamic_energy," << trace.initial_dynamic_energy
              << ",final_dynamic_energy," << trace.final_dynamic_energy
              << ",finite," << (trace.finite ? "true" : "false") << '\n';
}

void print_response(const PairResponse& response) {
    std::cout << "response,axis," << axis_name(response.axis)
              << ",transverse_response_rms,"
              << response.transverse_response_rms
              << ",transverse_common_rms," << response.transverse_common_rms
              << ",transverse_polarizing_rms,"
              << response.transverse_polarizing_rms
              << ",transverse_common_fraction,"
              << response.transverse_common_fraction
              << ",global_common_rms," << response.global_common_rms
              << ",global_polarizing_rms," << response.global_polarizing_rms
              << ",global_common_fraction," << response.global_common_fraction
              << ",common_dx," << response.common_displacement.x
              << ",common_dy," << response.common_displacement.y
              << ",common_dz," << response.common_displacement.z
              << ",polarizing_dx," << response.polarizing_displacement.x
              << ",polarizing_dy," << response.polarizing_displacement.y
              << ",polarizing_dz," << response.polarizing_displacement.z
              << ",energy_closure," << response.normalized_energy_closure
              << ",pair_survived,"
              << (response.pair_survived ? "true" : "false")
              << ",finite," << (response.finite ? "true" : "false") << '\n';
}

}  // namespace

int main() {
    std::cout << std::setprecision(17);
    std::cout << "FTD-0436 neutral-pair wave response v1\n";
    std::cout << "protocol,L," << kL << ",ticks," << kTicks
              << ",mode_n," << kMode << ",amplitude," << kAmplitude
              << ",separation," << kSeparation << ",seed," << kSeed
              << ",repeat_gate," << kRepeatGate
              << ",response_gate," << kResponseGate << '\n';
    std::cout << "scope,selected_flux_gradient_force_neutral_pair_discriminator\n";

    const PairTrace wave_only = run_arm(PairAxis::None, false, true);
    const std::array<PairAxis, 2> axes{{PairAxis::Y, PairAxis::Z}};
    std::array<PairTrace, 2> pair_only;
    std::array<PairTrace, 2> combined;
    std::array<PairTrace, 2> repeat;
    std::array<PairResponse, 2> responses;
    std::array<double, 2> repeat_residuals{};

    for (std::size_t index = 0; index < axes.size(); ++index) {
        pair_only[index] = run_arm(axes[index], true, false);
        combined[index] = run_arm(axes[index], true, true);
        repeat[index] = run_arm(axes[index], true, true);
        responses[index] = make_response(
            combined[index], pair_only[index], wave_only);
        repeat_residuals[index] = trace_difference(
            combined[index], repeat[index]);
    }

    print_trace("wave_only", wave_only);
    for (std::size_t index = 0; index < axes.size(); ++index) {
        print_trace("pair_only", pair_only[index]);
        print_trace("pair_plus_wave", combined[index]);
        print_response(responses[index]);
    }

    bool finite = wave_only.finite && wave_only.forbidden_toggles_off
        && wave_only.cpu_backend;
    bool pair_lost = false;
    bool detected = true;
    bool deterministic = true;
    bool common = true;
    bool electric = true;
    bool global_common = true;
    bool com_translation = true;
    bool energy_closed = true;
    for (std::size_t index = 0; index < axes.size(); ++index) {
        finite = finite && pair_only[index].finite && combined[index].finite
            && repeat[index].finite && responses[index].finite
            && pair_only[index].forbidden_toggles_off
            && combined[index].forbidden_toggles_off
            && repeat[index].forbidden_toggles_off
            && pair_only[index].cpu_backend && combined[index].cpu_backend
            && repeat[index].cpu_backend;
        pair_lost = pair_lost || !pair_only[index].plus_present
            || !pair_only[index].minus_present
            || !combined[index].plus_present
            || !combined[index].minus_present
            || !repeat[index].plus_present
            || !repeat[index].minus_present;
        detected = detected
            && responses[index].transverse_response_rms > kResponseGate;
        deterministic = deterministic
            && repeat_residuals[index] <= kRepeatGate;
        common = common
            && responses[index].transverse_common_fraction >= kDominanceGate;
        electric = electric
            && responses[index].transverse_common_fraction
                <= (1.0 - kDominanceGate);
        global_common = global_common
            && responses[index].global_common_fraction >= kDominanceGate;
        com_translation = com_translation
            && responses[index].common_displacement.mag()
                > responses[index].polarizing_displacement.mag();
        energy_closed = energy_closed
            && responses[index].normalized_energy_closure <= kEnergyGate;
    }

    const char* verdict = "MIXED_ORIENTATION_DEPENDENT_PAIR_RESPONSE";
    if (!finite || !deterministic) verdict = "INVALID_PROTOCOL";
    else if (pair_lost) verdict = "PAIR_ANNIHILATION_OR_LOSS";
    else if (!detected) verdict = "NO_RESOLVED_PAIR_RESPONSE";
    else if (common) verdict = "COMMON_MODE_NEUTRAL_TRANSLATION";
    else if (electric) verdict = "ELECTRIC_LIKE_PAIR_POLARIZATION";

    std::cout << "repeat,y," << repeat_residuals[0]
              << ",z," << repeat_residuals[1]
              << ",deterministic," << (deterministic ? "true" : "false")
              << '\n';
    std::cout << "secondary,global_common,"
              << (global_common ? "GLOBAL_COMMON_DOMINANT"
                                : "GLOBAL_NOT_COMMON_DOMINANT")
              << ",trajectory,"
              << (com_translation ? "COM_TRANSLATION_DOMINANT"
                                  : "RELATIVE_DISPLACEMENT_DOMINANT")
              << ",energy,"
              << (energy_closed ? "ACCOUNTED_ENERGY_CLOSED"
                                : "ACCOUNTED_ENERGY_OPEN") << '\n';
    std::cout << "gates,finite," << (finite ? "true" : "false")
              << ",pair_lost," << (pair_lost ? "true" : "false")
              << ",detected," << (detected ? "true" : "false")
              << ",deterministic," << (deterministic ? "true" : "false")
              << '\n';
    std::cout << "verdict," << verdict << '\n';

    return std::string(verdict) == "INVALID_PROTOCOL" ? 1 : 0;
}

