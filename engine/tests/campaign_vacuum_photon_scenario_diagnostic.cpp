/**
 * @file campaign_vacuum_photon_scenario_diagnostic.cpp
 * @brief FTD-0434 exact s0-vacuum-photon production-state diagnostic.
 */

#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/scenarios.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

constexpr int kL = 33;
constexpr int kFinalTick = 24;

struct Args {
    std::string output = "vacuum_photon_scenario_diagnostic.csv";
};

struct Sample {
    int tick = 0;
    long long occupancy = 0;
    long long signed_state = 0;
    std::array<double, 3> flux_component{};
    std::array<double, 3> wave_component{};
    double total_quadratic = 0.0;
    double centroid_x = 0.0;
    double displacement_x = 0.0;
    double width_x = 0.0;
    double divergence_normalized = 0.0;
    double j_dot_curl_normalized = 0.0;
    int best_shift = 0;
    double best_shift_overlap = 0.0;
};

struct ArmResult {
    std::string arm;
    std::string actual_backend;
    bool dispatched = false;
    bool toggles_valid = false;
    bool execution_valid = false;
    double right_moving_residual = 0.0;
    std::array<double, 3> wave_fraction{};
    std::vector<Sample> samples;
    bool translating = false;
    bool nontranslating = false;
    bool projection_dominated = false;
};

Args parse_args(int argc, char** argv) {
    Args args;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
            args.output = argv[++i];
        } else {
            std::cerr << "unknown argument: " << argv[i] << '\n';
        }
    }
    return args;
}

double component(const ftd::Vec3& value, int axis) {
    if (axis == 0) return value.x;
    if (axis == 1) return value.y;
    return value.z;
}

std::vector<double> slice_energy(const ftd::RenderBridge& bridge) {
    std::vector<double> out(kL, 0.0);
    const auto& voxels = bridge.voxels();
    for (int x = 0; x < kL; ++x) {
        for (int y = 0; y < kL; ++y) {
            for (int z = 0; z < kL; ++z) {
                const auto& voxel = voxels[static_cast<std::size_t>(
                    bridge.lattice().index(x, y, z))];
                out[static_cast<std::size_t>(x)] +=
                    voxel.flux.mag2() + voxel.wave_vel.mag2();
            }
        }
    }
    return out;
}

std::pair<int, double> best_shift_overlap(
    const std::vector<double>& initial,
    const std::vector<double>& current) {
    double norm_initial = 0.0;
    double norm_current = 0.0;
    for (int x = 0; x < kL; ++x) {
        norm_initial += initial[static_cast<std::size_t>(x)]
            * initial[static_cast<std::size_t>(x)];
        norm_current += current[static_cast<std::size_t>(x)]
            * current[static_cast<std::size_t>(x)];
    }
    int best = 0;
    double overlap_best = -std::numeric_limits<double>::infinity();
    const double denominator = std::sqrt(norm_initial * norm_current);
    for (int shift = -kL / 2; shift <= kL / 2; ++shift) {
        double dot = 0.0;
        for (int x = 0; x < kL; ++x) {
            int source = (x - shift) % kL;
            if (source < 0) source += kL;
            dot += current[static_cast<std::size_t>(x)]
                * initial[static_cast<std::size_t>(source)];
        }
        const double overlap = denominator > 0.0 ? dot / denominator : 0.0;
        if (overlap > overlap_best) {
            overlap_best = overlap;
            best = shift;
        }
    }
    return {best, overlap_best};
}

double circular_delta(double x, double center) {
    double delta = x - center;
    while (delta > 0.5 * kL) delta -= kL;
    while (delta < -0.5 * kL) delta += kL;
    return delta;
}

Sample measure(const ftd::RenderBridge& bridge,
               int tick,
               double initial_centroid,
               const std::vector<double>& initial_profile) {
    Sample out;
    out.tick = tick;
    const auto& lattice = bridge.lattice();
    const auto& voxels = bridge.voxels();
    std::vector<double> profile = slice_energy(bridge);
    double total_weight = 0.0;
    double cosine = 0.0;
    double sine = 0.0;
    double flux_norm = 0.0;
    double divergence_sq = 0.0;
    double curl_norm = 0.0;
    double helicity = 0.0;

    for (int x = 0; x < kL; ++x) {
        const double weight = profile[static_cast<std::size_t>(x)];
        const double angle = 2.0 * ftd::PI * static_cast<double>(x) / kL;
        total_weight += weight;
        cosine += weight * std::cos(angle);
        sine += weight * std::sin(angle);
    }
    double angle = std::atan2(sine, cosine);
    if (angle < 0.0) angle += 2.0 * ftd::PI;
    out.centroid_x = angle * kL / (2.0 * ftd::PI);
    out.displacement_x = circular_delta(out.centroid_x, initial_centroid);

    for (int x = 0; x < kL; ++x) {
        const double distance = circular_delta(
            static_cast<double>(x), out.centroid_x);
        out.width_x += profile[static_cast<std::size_t>(x)]
            * distance * distance;
    }
    out.width_x = total_weight > 0.0
        ? std::sqrt(out.width_x / total_weight) : 0.0;

    for (std::size_t index = 0; index < lattice.total_sites(); ++index) {
        const auto& voxel = voxels[index];
        if (voxel.state != 0) ++out.occupancy;
        out.signed_state += voxel.state;
        for (int axis_index = 0; axis_index < 3; ++axis_index) {
            const double j = component(voxel.flux, axis_index);
            const double w = component(voxel.wave_vel, axis_index);
            out.flux_component[static_cast<std::size_t>(axis_index)] += j * j;
            out.wave_component[static_cast<std::size_t>(axis_index)] += w * w;
        }
        const double div = bridge.divergence_flux(static_cast<int>(index));
        const auto curl = bridge.curl_flux(static_cast<int>(index));
        flux_norm += voxel.flux.mag2();
        divergence_sq += div * div;
        curl_norm += curl.mag2();
        helicity += voxel.flux.dot(curl);
    }
    for (double value : out.flux_component) out.total_quadratic += value;
    for (double value : out.wave_component) out.total_quadratic += value;
    out.divergence_normalized = std::sqrt(
        divergence_sq / std::max(1e-30, flux_norm));
    out.j_dot_curl_normalized = helicity /
        std::sqrt(std::max(1e-30, flux_norm * curl_norm));
    const auto overlap = best_shift_overlap(initial_profile, profile);
    out.best_shift = overlap.first;
    out.best_shift_overlap = overlap.second;
    return out;
}

void measure_initial_relation(const ftd::RenderBridge& bridge,
                              ArmResult& out) {
    double residual_sq = 0.0;
    double target_sq = 0.0;
    double wave_total = 0.0;
    const auto& lattice = bridge.lattice();
    const auto& voxels = bridge.voxels();
    for (int x = 0; x < kL; ++x) {
        for (int y = 0; y < kL; ++y) {
            for (int z = 0; z < kL; ++z) {
                const int index = lattice.index(x, y, z);
                const double derivative = 0.5 * (
                    voxels[static_cast<std::size_t>(
                        lattice.index(x + 1, y, z))].flux.z
                    - voxels[static_cast<std::size_t>(
                        lattice.index(x - 1, y, z))].flux.z);
                const double target = -ftd::C_WAVE * derivative;
                const double residual = voxels[static_cast<std::size_t>(index)]
                    .wave_vel.z - target;
                residual_sq += residual * residual;
                target_sq += target * target;
                for (int axis = 0; axis < 3; ++axis) {
                    const double value = component(
                        voxels[static_cast<std::size_t>(index)].wave_vel, axis);
                    out.wave_fraction[static_cast<std::size_t>(axis)] +=
                        value * value;
                    wave_total += value * value;
                }
            }
        }
    }
    out.right_moving_residual = std::sqrt(
        residual_sq / std::max(1e-30, target_sq));
    if (wave_total > 0.0)
        for (double& value : out.wave_fraction) value /= wave_total;
}

bool dashboard_toggle_contract(const ftd::RenderBridge& bridge) {
    const auto& t = bridge.toggles;
    return t.wave_propagation && t.coupling && t.damping
        && t.gauss_projection && t.forces && t.movement
        && t.poisson_coulomb && t.selective_damping
        && !t.genesis && !t.evaporation && !t.gravity
        && !t.lorentz_force && !t.larmor_radiation
        && !t.dual_substrate && !t.confinement && !t.color_forces
        && !t.strong_force && !t.exchange_force
        && !t.weak_transmutation && !t.de_broglie_clock
        && !t.pair_production && !t.absorbing_boundary
        && !t.reflective_boundary
        && t.flux_boundary == ftd::FluxBoundaryMode::Dispersal;
}

bool wave_only_toggle_contract(const ftd::RenderBridge& bridge) {
    const auto& t = bridge.toggles;
    return t.wave_propagation && !t.gauss_projection && !t.coupling
        && !t.damping && !t.selective_damping && !t.genesis
        && !t.forces && !t.movement && !t.poisson_coulomb
        && !t.pair_production && !t.weak_transmutation;
}

void configure_dashboard_defaults(ftd::RenderBridge& bridge) {
    // Mirror SCALE0_TOGGLES plus the browser's default non-reflective
    // boundary. Fresh C++ defaults are not the dashboard reset profile.
    bridge.toggles.disable_all();
    auto& t = bridge.toggles;
    t.wave_propagation = true;
    t.coupling = true;
    t.damping = true;
    t.genesis = true;  // The scenario itself switches this off.
    t.gauss_projection = true;
    t.forces = true;
    t.gravity = false;
    t.movement = true;
    t.poisson_coulomb = true;
    t.lorentz_force = false;
    t.selective_damping = true;
    t.larmor_radiation = false;
    t.dual_substrate = false;
    t.confinement = false;
    t.color_forces = false;
    t.strong_force = false;
    t.exchange_force = false;
    t.weak_transmutation = false;
    t.de_broglie_clock = false;
    t.absorbing_boundary = false;
    t.reflective_boundary = false;
    t.flux_boundary = ftd::FluxBoundaryMode::Dispersal;
}

ArmResult run_arm(const std::string& arm) {
    ArmResult out;
    out.arm = arm;
    ftd::RenderBridge bridge(kL);
    bridge.force_cpu();
    if (arm == "dashboard") configure_dashboard_defaults(bridge);
    out.actual_backend = bridge.backend_kind() == ftd::Backend::Kind::Gpu
        ? "gpu" : "cpu";
    out.dispatched = ftd::dispatch_scenario(bridge, "s0-vacuum-photon");
    if (arm == "wave_only") {
        bridge.toggles.disable_all();
        bridge.toggles.wave_propagation = true;
    }
    bridge.toggles.strict_validation = true;
    out.toggles_valid = arm == "dashboard"
        ? dashboard_toggle_contract(bridge)
        : wave_only_toggle_contract(bridge);
    measure_initial_relation(bridge, out);
    const auto initial_profile = slice_energy(bridge);
    const auto first = measure(bridge, 0, 0.0, initial_profile);
    const double initial_centroid = first.centroid_x;
    out.samples.push_back(measure(
        bridge, 0, initial_centroid, initial_profile));
    for (int tick = 1; tick <= kFinalTick; ++tick) {
        bridge.tick();
        out.samples.push_back(measure(
            bridge, tick, initial_centroid, initial_profile));
    }

    bool finite = true;
    bool empty = true;
    for (const auto& sample : out.samples) {
        empty = empty && sample.occupancy == 0 && sample.signed_state == 0;
        finite = finite && std::isfinite(sample.total_quadratic)
            && std::isfinite(sample.centroid_x)
            && std::isfinite(sample.width_x)
            && std::isfinite(sample.divergence_normalized)
            && std::isfinite(sample.best_shift_overlap);
    }
    const auto& initial = out.samples.front();
    const auto& tick1 = out.samples[1];
    const auto& tick20 = out.samples[20];
    const double mean_speed = tick20.displacement_x / 20.0;
    out.translating = tick20.displacement_x >= 8.0
        && std::abs(mean_speed - ftd::C_WAVE) <= 0.2 * ftd::C_WAVE
        && tick20.best_shift_overlap >= 0.8;
    out.nontranslating = (
        std::abs(tick20.displacement_x) < 2.0
        && tick20.width_x >= 1.25 * initial.width_x)
        || (std::abs(static_cast<double>(tick20.best_shift)) < 2.0
            && tick20.best_shift_overlap < 0.8);
    const double divergence_ratio = tick1.divergence_normalized
        / std::max(1e-30, initial.divergence_normalized);
    const double flux_initial = initial.flux_component[0]
        + initial.flux_component[1] + initial.flux_component[2];
    const double flux_tick1 = tick1.flux_component[0]
        + tick1.flux_component[1] + tick1.flux_component[2];
    const double flux_change = std::abs(flux_tick1 - flux_initial)
        / std::max(1e-30, flux_initial);
    out.projection_dominated = arm == "dashboard"
        && divergence_ratio <= 0.01 && flux_change >= 0.01;
    out.execution_valid = out.actual_backend == "cpu" && out.dispatched
        && out.toggles_valid && empty && finite
        && out.samples.size() == static_cast<std::size_t>(kFinalTick + 1);
    return out;
}

void write_header(std::ofstream& out) {
    out << "arm,actual_backend,L,tick,occupancy,signed_state,"
           "flux_x2,flux_y2,flux_z2,wave_x2,wave_y2,wave_z2,"
           "total_quadratic,centroid_x,displacement_x,width_x,"
           "divergence_normalized,j_dot_curl_normalized,best_shift,"
           "best_shift_overlap,right_moving_residual,wave_fraction_x,"
           "wave_fraction_y,wave_fraction_z,dispatched,toggles_valid,"
           "translating_clause,nontranslating_clause,"
           "projection_dominated_clause,execution_valid\n";
}

void write_arm(std::ofstream& out, const ArmResult& arm) {
    for (const auto& sample : arm.samples) {
        out << arm.arm << ',' << arm.actual_backend << ',' << kL << ','
            << sample.tick << ',' << sample.occupancy << ','
            << sample.signed_state;
        for (double value : sample.flux_component) out << ',' << value;
        for (double value : sample.wave_component) out << ',' << value;
        out << ',' << sample.total_quadratic << ',' << sample.centroid_x << ','
            << sample.displacement_x << ',' << sample.width_x << ','
            << sample.divergence_normalized << ','
            << sample.j_dot_curl_normalized << ',' << sample.best_shift << ','
            << sample.best_shift_overlap << ',' << arm.right_moving_residual;
        for (double value : arm.wave_fraction) out << ',' << value;
        out << ',' << (arm.dispatched ? 1 : 0)
            << ',' << (arm.toggles_valid ? 1 : 0)
            << ',' << (arm.translating ? 1 : 0)
            << ',' << (arm.nontranslating ? 1 : 0)
            << ',' << (arm.projection_dominated ? 1 : 0)
            << ',' << (arm.execution_valid ? 1 : 0) << '\n';
    }
}

}  // namespace

int main(int argc, char** argv) {
    const Args args = parse_args(argc, argv);
    const fs::path output_path(args.output);
    if (!output_path.parent_path().empty())
        fs::create_directories(output_path.parent_path());
    std::ofstream csv(output_path);
    if (!csv) {
        std::cerr << "could not open output: " << args.output << '\n';
        return 2;
    }
    csv << std::setprecision(17);
    write_header(csv);

    const auto dashboard = run_arm("dashboard");
    const auto wave_only = run_arm("wave_only");
    write_arm(csv, dashboard);
    write_arm(csv, wave_only);
    const bool valid = dashboard.execution_valid && wave_only.execution_valid;

    for (const auto* arm : {&dashboard, &wave_only}) {
        const auto& initial = arm->samples.front();
        const auto& tick20 = arm->samples[20];
        std::cout << arm->arm
                  << " initial_div=" << initial.divergence_normalized
                  << " right_residual=" << arm->right_moving_residual
                  << " wave_fraction=" << arm->wave_fraction[0] << '/'
                  << arm->wave_fraction[1] << '/' << arm->wave_fraction[2]
                  << " tick20_dx=" << tick20.displacement_x
                  << " width_ratio=" << tick20.width_x / initial.width_x
                  << " shift=" << tick20.best_shift
                  << " overlap=" << tick20.best_shift_overlap
                  << " translating=" << arm->translating
                  << " nontranslating=" << arm->nontranslating
                  << " projection_dominated=" << arm->projection_dominated
                  << " valid=" << arm->execution_valid << '\n';
    }
    std::cout << "FTD-0434 exact vacuum-photon diagnostic rows="
              << dashboard.samples.size() + wave_only.samples.size()
              << " execution_valid=" << valid
              << " output=" << args.output << '\n';
    return valid ? 0 : 1;
}
