/**
 * @file campaign_flux_self_interference_response.cpp
 * @brief FTD-0435 polarity and polarization audit of the selected
 *        flux/self-field force response.
 *
 * Observation only: this campaign configures and measures the existing
 * production RenderBridge tick.  It adds no force, source, or sidecar state.
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
constexpr std::uint64_t kSeed = 4350;
constexpr double kRepeatGate = 1e-12;
constexpr double kResponseGate = 1e-8;
constexpr double kSymmetryGate = 0.10;
constexpr double kDisplacementSymmetryGate = 0.20;
constexpr double kEnergyGate = 1e-6;
constexpr std::array<double, 3> kAmplitudes{{0.025, 0.05, 0.10}};

enum class Polarization { LinearY, LinearZ, CircularYZ, None };

const char* polarization_name(Polarization polarization) {
    switch (polarization) {
        case Polarization::LinearY: return "linear_y";
        case Polarization::LinearZ: return "linear_z";
        case Polarization::CircularYZ: return "circular_yz";
        case Polarization::None: return "none";
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
    int polarity = 0;
    Polarization polarization = Polarization::None;
    double amplitude = 0.0;
    bool particle_requested = false;
    bool particle_present = false;
    bool finite = true;
    bool forbidden_toggles_off = false;
    bool cpu_backend = false;
    ftd::Vec3 displacement{};
    ftd::Vec3 velocity{};
    double max_speed = 0.0;
    double initial_dynamic_energy = 0.0;
    double final_dynamic_energy = 0.0;
    std::vector<ftd::Vec3> force_history;
};

struct Response {
    int polarity = 0;
    Polarization polarization = Polarization::None;
    double amplitude = 0.0;
    ftd::Vec3 displacement{};
    ftd::Vec3 velocity{};
    ftd::Vec3 impulse{};
    double rms_force = 0.0;
    double peak_force = 0.0;
    double normalized_energy_closure = 0.0;
    bool finite = true;
    std::vector<ftd::Vec3> force_history;
};

double wrapped_step(int current, int previous) {
    int delta = current - previous;
    if (delta > kL / 2) delta -= kL;
    if (delta < -kL / 2) delta += kL;
    return static_cast<double>(delta);
}

Track find_particle(const ftd::RenderBridge& bridge, int particle_id,
                    int polarity) {
    const auto& voxels = bridge.voxels();
    for (int i = 0; i < static_cast<int>(voxels.size()); ++i) {
        const auto& voxel = voxels[static_cast<std::size_t>(i)];
        if (voxel.state != 0 && voxel.particle_id == particle_id) {
            const auto coordinate = bridge.lattice().coord(i);
            return {true, i, coordinate.x, coordinate.y, coordinate.z, &voxel};
        }
    }
    for (int i = 0; i < static_cast<int>(voxels.size()); ++i) {
        const auto& voxel = voxels[static_cast<std::size_t>(i)];
        if ((polarity > 0 && voxel.state > 0)
            || (polarity < 0 && voxel.state < 0)) {
            const auto coordinate = bridge.lattice().coord(i);
            return {true, i, coordinate.x, coordinate.y, coordinate.z, &voxel};
        }
    }
    return {};
}

void configure(ftd::RenderBridge& bridge) {
    bridge.force_cpu();
    bridge.seed_rng(kSeed);
    for (const auto& spec : ftd::TOGGLE_SPECS) {
        bridge.toggles.*(spec.field) = false;
    }
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

std::array<double, 2> travelling_component(double amplitude, double phase,
                                           double omega) {
    const double sine = std::sin(phase);
    const double cosine = std::cos(phase);
    return {{amplitude * sine,
             amplitude * ((1.0 - std::cos(omega)) * sine
                          - std::sin(omega) * cosine)}};
}

void inject_wave(ftd::RenderBridge& bridge, Polarization polarization,
                 double amplitude) {
    if (polarization == Polarization::None || amplitude == 0.0) return;
    const double k = 2.0 * ftd::PI * static_cast<double>(kMode)
        / static_cast<double>(kL);
    const double omega = 2.0 * std::asin(
        ftd::C_SPEED * std::abs(std::sin(0.5 * k)));
    const double component_amplitude =
        polarization == Polarization::CircularYZ
        ? amplitude / std::sqrt(2.0) : amplitude;

    auto& voxels = bridge.voxels();
    for (int x = 0; x < kL; ++x) {
        const double phase = k * static_cast<double>(x);
        const auto primary = travelling_component(
            component_amplitude, phase, omega);
        const auto quadrature = travelling_component(
            component_amplitude, phase + 0.5 * ftd::PI, omega);
        for (int y = 0; y < kL; ++y) {
            for (int z = 0; z < kL; ++z) {
                auto& voxel = voxels[static_cast<std::size_t>(
                    bridge.lattice().index(x, y, z))];
                if (polarization == Polarization::LinearY
                    || polarization == Polarization::CircularYZ) {
                    voxel.flux.y += primary[0];
                    voxel.wave_vel.y += primary[1];
                }
                if (polarization == Polarization::LinearZ) {
                    voxel.flux.z += primary[0];
                    voxel.wave_vel.z += primary[1];
                } else if (polarization == Polarization::CircularYZ) {
                    voxel.flux.z += quadrature[0];
                    voxel.wave_vel.z += quadrature[1];
                }
            }
        }
    }
}

bool finite_vec(const ftd::Vec3& value) {
    return std::isfinite(value.x) && std::isfinite(value.y)
        && std::isfinite(value.z);
}

Trace run_arm(int polarity, Polarization polarization, double amplitude) {
    ftd::RenderBridge bridge(kL);
    configure(bridge);

    const bool particle = polarity != 0;
    const int center = kL / 2;
    int particle_id = -1;
    if (particle) {
        bridge.inject_particle(center, center, center,
                               static_cast<std::int8_t>(polarity),
                               {}, 0, 0);
        particle_id = bridge.voxels()[static_cast<std::size_t>(
            bridge.lattice().index(center, center, center))].particle_id;
    }
    inject_wave(bridge, polarization, amplitude);

    Trace trace;
    trace.polarity = polarity;
    trace.polarization = polarization;
    trace.amplitude = amplitude;
    trace.particle_requested = particle;
    trace.forbidden_toggles_off = forbidden_toggles_are_off(bridge);
    trace.cpu_backend = bridge.backend_kind() == ftd::Backend::Kind::Cpu;
    trace.initial_dynamic_energy = bridge.energy_audit().dynamic_energy;
    trace.force_history.reserve(kTicks);

    int previous_x = center;
    int previous_y = center;
    int previous_z = center;
    ftd::Vec3 integer_displacement{};

    for (int tick = 0; tick < kTicks; ++tick) {
        Track before;
        if (particle) before = find_particle(bridge, particle_id, polarity);
        if (particle && !before.present) {
            trace.finite = false;
            break;
        }

        bridge.tick();

        if (particle) {
            const auto force = bridge.force_diag_at(before.index).f_coulomb;
            trace.force_history.push_back(force);
            const Track after = find_particle(bridge, particle_id, polarity);
            if (!after.present) {
                trace.finite = false;
                break;
            }
            integer_displacement.x += wrapped_step(after.x, previous_x);
            integer_displacement.y += wrapped_step(after.y, previous_y);
            integer_displacement.z += wrapped_step(after.z, previous_z);
            previous_x = after.x;
            previous_y = after.y;
            previous_z = after.z;
            trace.max_speed = std::max(trace.max_speed, after.voxel->speed());
        }
    }

    trace.final_dynamic_energy = bridge.energy_audit().dynamic_energy;
    if (particle) {
        const Track final = find_particle(bridge, particle_id, polarity);
        trace.particle_present = final.present;
        if (final.present) {
            trace.displacement = integer_displacement + final.voxel->remainder;
            trace.velocity = final.voxel->velocity;
        }
    }

    trace.finite = trace.finite
        && (!particle || (trace.particle_present
                          && trace.force_history.size() == kTicks))
        && finite_vec(trace.displacement) && finite_vec(trace.velocity)
        && std::isfinite(trace.max_speed)
        && std::isfinite(trace.initial_dynamic_energy)
        && std::isfinite(trace.final_dynamic_energy);
    for (const auto& force : trace.force_history)
        trace.finite = trace.finite && finite_vec(force);
    return trace;
}

Response make_response(const Trace& combined, const Trace& source,
                       const Trace& wave) {
    Response response;
    response.polarity = combined.polarity;
    response.polarization = combined.polarization;
    response.amplitude = combined.amplitude;
    response.displacement = combined.displacement - source.displacement;
    response.velocity = combined.velocity - source.velocity;
    response.finite = combined.finite && source.finite && wave.finite
        && combined.forbidden_toggles_off && source.forbidden_toggles_off
        && wave.forbidden_toggles_off && combined.cpu_backend
        && source.cpu_backend && wave.cpu_backend
        && combined.force_history.size() == source.force_history.size();

    double sum_force2 = 0.0;
    response.force_history.reserve(combined.force_history.size());
    for (std::size_t tick = 0; tick < combined.force_history.size(); ++tick) {
        const ftd::Vec3 extra = combined.force_history[tick]
            - source.force_history[tick];
        response.force_history.push_back(extra);
        response.impulse = response.impulse + extra;
        sum_force2 += extra.mag2();
        response.peak_force = std::max(response.peak_force, extra.mag());
    }
    if (!response.force_history.empty()) {
        response.rms_force = std::sqrt(
            sum_force2 / static_cast<double>(response.force_history.size()));
    }

    const double combined_change = combined.final_dynamic_energy
        - combined.initial_dynamic_energy;
    const double source_change = source.final_dynamic_energy
        - source.initial_dynamic_energy;
    const double wave_change = wave.final_dynamic_energy
        - wave.initial_dynamic_energy;
    const double closure = combined_change - source_change - wave_change;
    const double energy_scale = std::max(
        1e-30, std::abs(source.initial_dynamic_energy)
             + std::abs(wave.initial_dynamic_energy));
    response.normalized_energy_closure = std::abs(closure) / energy_scale;
    response.finite = response.finite && finite_vec(response.displacement)
        && finite_vec(response.velocity) && finite_vec(response.impulse)
        && std::isfinite(response.rms_force)
        && std::isfinite(response.peak_force)
        && std::isfinite(response.normalized_energy_closure);
    return response;
}

double trace_difference(const Trace& left, const Trace& right) {
    double maximum = 0.0;
    const auto absorb = [&](double value) {
        maximum = std::max(maximum, std::abs(value));
    };
    absorb(left.displacement.x - right.displacement.x);
    absorb(left.displacement.y - right.displacement.y);
    absorb(left.displacement.z - right.displacement.z);
    absorb(left.velocity.x - right.velocity.x);
    absorb(left.velocity.y - right.velocity.y);
    absorb(left.velocity.z - right.velocity.z);
    absorb(left.max_speed - right.max_speed);
    absorb(left.initial_dynamic_energy - right.initial_dynamic_energy);
    absorb(left.final_dynamic_energy - right.final_dynamic_energy);
    if (left.force_history.size() != right.force_history.size())
        return std::numeric_limits<double>::infinity();
    for (std::size_t tick = 0; tick < left.force_history.size(); ++tick) {
        absorb(left.force_history[tick].x - right.force_history[tick].x);
        absorb(left.force_history[tick].y - right.force_history[tick].y);
        absorb(left.force_history[tick].z - right.force_history[tick].z);
    }
    return maximum;
}

double history_residual(const std::vector<ftd::Vec3>& plus,
                        const std::vector<ftd::Vec3>& minus,
                        bool odd) {
    if (plus.size() != minus.size() || plus.empty())
        return std::numeric_limits<double>::infinity();
    double numerator = 0.0;
    double denominator = 0.0;
    for (std::size_t tick = 0; tick < plus.size(); ++tick) {
        const auto residual = odd ? plus[tick] + minus[tick]
                                  : plus[tick] - minus[tick];
        numerator += residual.mag2();
        denominator += plus[tick].mag2() + minus[tick].mag2();
    }
    return std::sqrt(numerator / std::max(1e-30, denominator));
}

double vector_residual(const ftd::Vec3& plus, const ftd::Vec3& minus,
                       bool odd) {
    const auto residual = odd ? plus + minus : plus - minus;
    const double denominator = std::sqrt(plus.mag2() + minus.mag2());
    return residual.mag() / std::max(1e-30, denominator);
}

ftd::Vec3 rotate_y_to_z(const ftd::Vec3& value) {
    return {value.x, -value.z, value.y};
}

double rotation_residual(const std::vector<ftd::Vec3>& linear_y,
                         const std::vector<ftd::Vec3>& linear_z) {
    if (linear_y.size() != linear_z.size() || linear_y.empty())
        return std::numeric_limits<double>::infinity();
    double numerator = 0.0;
    double denominator = 0.0;
    for (std::size_t tick = 0; tick < linear_y.size(); ++tick) {
        const auto rotated = rotate_y_to_z(linear_y[tick]);
        numerator += (linear_z[tick] - rotated).mag2();
        denominator += linear_z[tick].mag2() + rotated.mag2();
    }
    return std::sqrt(numerator / std::max(1e-30, denominator));
}

double amplitude_exponent(const std::array<const Response*, 3>& responses) {
    double mean_x = 0.0;
    double mean_y = 0.0;
    for (const auto* response : responses) {
        mean_x += std::log(response->amplitude);
        mean_y += std::log(std::max(1e-300, response->rms_force));
    }
    mean_x /= 3.0;
    mean_y /= 3.0;
    double covariance = 0.0;
    double variance = 0.0;
    for (const auto* response : responses) {
        const double dx = std::log(response->amplitude) - mean_x;
        const double dy = std::log(std::max(1e-300, response->rms_force))
            - mean_y;
        covariance += dx * dy;
        variance += dx * dx;
    }
    return covariance / variance;
}

const Response& find_response(const std::vector<Response>& responses,
                              int polarity, Polarization polarization,
                              double amplitude) {
    for (const auto& response : responses) {
        if (response.polarity == polarity
            && response.polarization == polarization
            && std::abs(response.amplitude - amplitude) < 1e-15)
            return response;
    }
    throw std::logic_error("FTD-0435 response matrix is incomplete");
}

void print_trace(const char* kind, const Trace& trace) {
    std::cout << "trace,kind," << kind
              << ",polarity," << trace.polarity
              << ",polarization," << polarization_name(trace.polarization)
              << ",amplitude," << trace.amplitude
              << ",disp_x," << trace.displacement.x
              << ",disp_y," << trace.displacement.y
              << ",disp_z," << trace.displacement.z
              << ",vel_x," << trace.velocity.x
              << ",vel_y," << trace.velocity.y
              << ",vel_z," << trace.velocity.z
              << ",max_speed," << trace.max_speed
              << ",initial_dynamic_energy," << trace.initial_dynamic_energy
              << ",final_dynamic_energy," << trace.final_dynamic_energy
              << ",finite," << (trace.finite ? "true" : "false")
              << '\n';
}

void print_response(const Response& response) {
    std::cout << "response,polarity," << response.polarity
              << ",polarization," << polarization_name(response.polarization)
              << ",amplitude," << response.amplitude
              << ",disp_x," << response.displacement.x
              << ",disp_y," << response.displacement.y
              << ",disp_z," << response.displacement.z
              << ",disp_mag," << response.displacement.mag()
              << ",impulse_x," << response.impulse.x
              << ",impulse_y," << response.impulse.y
              << ",impulse_z," << response.impulse.z
              << ",impulse_mag," << response.impulse.mag()
              << ",rms_force," << response.rms_force
              << ",peak_force," << response.peak_force
              << ",energy_closure," << response.normalized_energy_closure
              << ",finite," << (response.finite ? "true" : "false")
              << '\n';
}

}  // namespace

int main() {
    std::cout << std::setprecision(17);
    std::cout << "FTD-0435 flux/self-field interference response v1\n";
    std::cout << "protocol,L," << kL << ",ticks," << kTicks
              << ",mode_n," << kMode << ",seed," << kSeed
              << ",repeat_gate," << kRepeatGate
              << ",response_gate," << kResponseGate << '\n';
    std::cout << "scope,selected_production_flux_gradient_force_not_qed_not_photon_not_pilot_wave\n";

    std::array<Trace, 2> sources{{
        run_arm(+1, Polarization::None, 0.0),
        run_arm(-1, Polarization::None, 0.0)}};

    const std::array<Polarization, 3> polarizations{{
        Polarization::LinearY, Polarization::LinearZ,
        Polarization::CircularYZ}};
    std::vector<Trace> waves;
    std::vector<Response> responses;
    std::vector<Trace> combined_traces;
    waves.reserve(polarizations.size() * kAmplitudes.size());
    responses.reserve(2 * polarizations.size() * kAmplitudes.size());
    combined_traces.reserve(responses.capacity());

    for (const auto polarization : polarizations) {
        for (double amplitude : kAmplitudes) {
            waves.push_back(run_arm(0, polarization, amplitude));
            const Trace& wave = waves.back();
            for (int polarity : {+1, -1}) {
                combined_traces.push_back(
                    run_arm(polarity, polarization, amplitude));
                const Trace& source = polarity > 0 ? sources[0] : sources[1];
                responses.push_back(make_response(
                    combined_traces.back(), source, wave));
            }
        }
    }

    const Trace repeat_plus = run_arm(+1, Polarization::LinearY, 0.05);
    const Trace repeat_minus = run_arm(-1, Polarization::LinearY, 0.05);
    const auto& baseline_plus_trace = *std::find_if(
        combined_traces.begin(), combined_traces.end(), [](const Trace& trace) {
            return trace.polarity == +1
                && trace.polarization == Polarization::LinearY
                && std::abs(trace.amplitude - 0.05) < 1e-15;
        });
    const auto& baseline_minus_trace = *std::find_if(
        combined_traces.begin(), combined_traces.end(), [](const Trace& trace) {
            return trace.polarity == -1
                && trace.polarization == Polarization::LinearY
                && std::abs(trace.amplitude - 0.05) < 1e-15;
        });
    const double repeat_plus_residual = trace_difference(
        baseline_plus_trace, repeat_plus);
    const double repeat_minus_residual = trace_difference(
        baseline_minus_trace, repeat_minus);

    for (const auto& source : sources) print_trace("source_only", source);
    for (const auto& wave : waves) print_trace("wave_only", wave);
    for (const auto& response : responses) print_response(response);

    const auto& baseline_plus = find_response(
        responses, +1, Polarization::LinearY, 0.05);
    const auto& baseline_minus = find_response(
        responses, -1, Polarization::LinearY, 0.05);
    const double force_odd = history_residual(
        baseline_plus.force_history, baseline_minus.force_history, true);
    const double force_even = history_residual(
        baseline_plus.force_history, baseline_minus.force_history, false);
    const double displacement_odd = vector_residual(
        baseline_plus.displacement, baseline_minus.displacement, true);
    const double displacement_even = vector_residual(
        baseline_plus.displacement, baseline_minus.displacement, false);

    std::array<double, 2> exponents{};
    std::array<double, 2> rotation_residuals{};
    std::array<double, 2> circular_ratios{};
    std::array<double, 2> transverse_fractions{};
    for (int index = 0; index < 2; ++index) {
        const int polarity = index == 0 ? +1 : -1;
        const std::array<const Response*, 3> amplitude_responses{{
            &find_response(responses, polarity, Polarization::LinearY, 0.025),
            &find_response(responses, polarity, Polarization::LinearY, 0.05),
            &find_response(responses, polarity, Polarization::LinearY, 0.10)}};
        exponents[static_cast<std::size_t>(index)] =
            amplitude_exponent(amplitude_responses);
        const auto& linear_y = *amplitude_responses[1];
        const auto& linear_z = find_response(
            responses, polarity, Polarization::LinearZ, 0.05);
        const auto& circular = find_response(
            responses, polarity, Polarization::CircularYZ, 0.05);
        rotation_residuals[static_cast<std::size_t>(index)] =
            rotation_residual(linear_y.force_history, linear_z.force_history);
        circular_ratios[static_cast<std::size_t>(index)] = circular.rms_force
            / std::max(1e-30, linear_y.rms_force);
        transverse_fractions[static_cast<std::size_t>(index)] =
            std::abs(linear_y.displacement.y)
            / std::max(1e-30, linear_y.displacement.mag());
    }

    double maximum_energy_closure = 0.0;
    bool finite = true;
    for (const auto& source : sources) {
        finite = finite && source.finite && source.forbidden_toggles_off
            && source.cpu_backend && source.particle_present;
    }
    for (const auto& wave : waves) {
        finite = finite && wave.finite && wave.forbidden_toggles_off
            && wave.cpu_backend;
    }
    for (const auto& response : responses) {
        finite = finite && response.finite;
        maximum_energy_closure = std::max(
            maximum_energy_closure, response.normalized_energy_closure);
    }
    const bool deterministic = repeat_plus_residual <= kRepeatGate
        && repeat_minus_residual <= kRepeatGate;
    const bool detected = std::max(baseline_plus.rms_force,
                                   baseline_plus.displacement.mag())
            > kResponseGate
        && std::max(baseline_minus.rms_force,
                    baseline_minus.displacement.mag()) > kResponseGate;
    const bool odd = force_odd <= kSymmetryGate
        && displacement_odd <= kDisplacementSymmetryGate
        && !(force_even <= kSymmetryGate
             && displacement_even <= kDisplacementSymmetryGate);
    const bool even = force_even <= kSymmetryGate
        && displacement_even <= kDisplacementSymmetryGate
        && !(force_odd <= kSymmetryGate
             && displacement_odd <= kDisplacementSymmetryGate);

    const char* verdict = "MIXED_POLARITY_RESPONSE";
    if (!finite || !deterministic) verdict = "INVALID_PROTOCOL";
    else if (!detected) verdict = "NO_RESOLVED_RESPONSE";
    else if (odd) verdict = "CHARGE_ODD_RESPONSE";
    else if (even) verdict = "POLARITY_EVEN_SELF_INTERFERENCE";

    const bool linear_amplitude = exponents[0] >= 0.8 && exponents[0] <= 1.2
        && exponents[1] >= 0.8 && exponents[1] <= 1.2;
    const bool quadratic_amplitude = exponents[0] >= 1.8 && exponents[0] <= 2.2
        && exponents[1] >= 1.8 && exponents[1] <= 2.2;
    const char* amplitude_class = linear_amplitude ? "LINEAR_AMPLITUDE"
        : quadratic_amplitude ? "QUADRATIC_AMPLITUDE"
        : "NONPOWER_OR_MIXED_AMPLITUDE";
    const bool rotation_pass = rotation_residuals[0] <= kSymmetryGate
        && rotation_residuals[1] <= kSymmetryGate;
    const bool transverse = transverse_fractions[0] >= 0.90
        && transverse_fractions[1] >= 0.90;
    const bool energy_closed = maximum_energy_closure <= kEnergyGate;

    std::cout << "symmetry,force_odd," << force_odd
              << ",force_even," << force_even
              << ",displacement_odd," << displacement_odd
              << ",displacement_even," << displacement_even << '\n';
    std::cout << "amplitude,exponent_plus," << exponents[0]
              << ",exponent_minus," << exponents[1]
              << ",class," << amplitude_class << '\n';
    std::cout << "polarization,rotation_plus," << rotation_residuals[0]
              << ",rotation_minus," << rotation_residuals[1]
              << ",rotation_class,"
              << (rotation_pass ? "CUBIC_ROTATION_PASS"
                                : "CUBIC_ROTATION_FAIL")
              << ",circular_ratio_plus," << circular_ratios[0]
              << ",circular_ratio_minus," << circular_ratios[1] << '\n';
    std::cout << "direction,transverse_fraction_plus,"
              << transverse_fractions[0]
              << ",transverse_fraction_minus," << transverse_fractions[1]
              << ",class,"
              << (transverse ? "TRANSVERSE_POLARIZATION_DOMINANT"
                             : "NOT_TRANSVERSE_DOMINANT") << '\n';
    std::cout << "energy,max_normalized_inclusion_exclusion_closure,"
              << maximum_energy_closure << ",class,"
              << (energy_closed ? "ACCOUNTED_ENERGY_CLOSED"
                                : "ACCOUNTED_ENERGY_OPEN") << '\n';
    std::cout << "repeat,plus," << repeat_plus_residual
              << ",minus," << repeat_minus_residual
              << ",deterministic," << (deterministic ? "true" : "false")
              << '\n';
    std::cout << "gates,finite," << (finite ? "true" : "false")
              << ",detected," << (detected ? "true" : "false")
              << ",deterministic," << (deterministic ? "true" : "false")
              << '\n';
    std::cout << "verdict," << verdict << '\n';

    return std::string(verdict) == "INVALID_PROTOCOL" ? 1 : 0;
}

