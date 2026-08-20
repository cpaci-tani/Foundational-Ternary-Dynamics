/**
 * @file test_dyadic_recurrence_probe.cpp
 * @brief [EXPLORATORY] Target-blind C3 geometry -> engine recurrence probe.
 *
 * This instrument deliberately separates three questions:
 *   1. Does the atlas curve close as a spatial parametrized loop? (yes, exact)
 *   2. Does a tangent flux tube made from that loop recur under the engine?
 *   3. Do the current strong/confinement toggles change a field-only loop?
 *
 * It is not a mass fit.  It contains no particle mass, CODATA value, alpha^n,
 * or adjustable search.  The spatial embedding and observation ticks are
 * frozen below.  A negative recurrence result is a measured boundary, not a
 * failing unit-test condition; only instrument correctness gates can fail.
 */

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "ftd/backend.h"
#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"

namespace {

constexpr int LATTICE_SIZE = 41;
constexpr int CURVE_SAMPLES = 2048;
constexpr double SPATIAL_SCALE = 1.5;
constexpr double TUBE_SIGMA = 0.85;
constexpr double LIFT_AMPLITUDE = 1729.0 / 800.0;  // 0.7 * (19/4) * 0.65
constexpr std::array<int, 5> OBSERVATION_TICKS{{0, 1, 2, 4, 8}};

enum class Channel { Primary, Strong };

struct CurveSample {
    ftd::Vec3 position;
    ftd::Vec3 tangent;
    double ds = 0.0;
};

struct Metrics {
    int tick = 0;
    double recurrence_error = 0.0;
    double shape_overlap = 0.0;
    double energy_ratio = 0.0;
    double local_fraction = 0.0;
    int manifested = 0;
};

struct RunResult {
    std::string name;
    std::vector<Metrics> metrics;
    std::vector<ftd::Vec3> final_field;
    std::vector<ftd::Vec3> final_velocity;
    std::vector<int8_t> final_state;
    ftd::Backend::Kind backend = ftd::Backend::Kind::Cpu;
};

struct SeedData {
    std::vector<ftd::Vec3> field;
    std::vector<uint8_t> local_mask;
    double norm2 = 0.0;
};

ftd::Vec3 c3_position(double t) {
    return {
        SPATIAL_SCALE * (std::cos(t) + 0.5 * std::cos(2.0 * t)
                       + 0.5 * std::cos(4.0 * t) + 0.375 * std::cos(8.0 * t)),
        SPATIAL_SCALE * (2.0 * std::sin(t) - std::sin(2.0 * t)
                       + std::sin(4.0 * t) - 0.75 * std::sin(8.0 * t)),
        SPATIAL_SCALE * LIFT_AMPLITUDE * std::sin(t)
    };
}

ftd::Vec3 c3_derivative(double t) {
    return {
        SPATIAL_SCALE * (-std::sin(t) - std::sin(2.0 * t)
                       - 2.0 * std::sin(4.0 * t) - 3.0 * std::sin(8.0 * t)),
        SPATIAL_SCALE * (2.0 * std::cos(t) - 2.0 * std::cos(2.0 * t)
                       + 4.0 * std::cos(4.0 * t) - 6.0 * std::cos(8.0 * t)),
        SPATIAL_SCALE * LIFT_AMPLITUDE * std::cos(t)
    };
}

double vec_distance(const ftd::Vec3& a, const ftd::Vec3& b) {
    return (a - b).mag();
}

std::vector<CurveSample> sample_curve() {
    std::vector<CurveSample> samples;
    samples.reserve(CURVE_SAMPLES);
    const double dt = 2.0 * ftd::PI / static_cast<double>(CURVE_SAMPLES);
    const double center = 0.5 * static_cast<double>(LATTICE_SIZE - 1);

    for (int j = 0; j < CURVE_SAMPLES; ++j) {
        const double t = dt * static_cast<double>(j);
        const auto p0 = c3_position(t);
        const auto d = c3_derivative(t);
        const double speed = d.mag();
        CurveSample sample;
        sample.position = {center + p0.x, center + p0.y, center + p0.z};
        sample.tangent = speed > 0.0 ? d * (1.0 / speed) : ftd::Vec3{};
        sample.ds = speed * dt;
        samples.push_back(sample);
    }
    return samples;
}

SeedData build_tangent_tube(const ftd::Lattice& lattice,
                            const std::vector<CurveSample>& samples) {
    SeedData seed;
    seed.field.assign(lattice.total_sites(), ftd::Vec3{});
    seed.local_mask.assign(lattice.total_sites(), 0);

    const int reach = static_cast<int>(std::ceil(3.0 * TUBE_SIGMA));
    const double inv_two_sigma2 = 1.0 / (2.0 * TUBE_SIGMA * TUBE_SIGMA);
    const double mask_radius2 = std::pow(2.5 * TUBE_SIGMA, 2);

    for (const auto& sample : samples) {
        const int px = static_cast<int>(std::floor(sample.position.x));
        const int py = static_cast<int>(std::floor(sample.position.y));
        const int pz = static_cast<int>(std::floor(sample.position.z));
        for (int x = px - reach; x <= px + reach; ++x) {
            for (int y = py - reach; y <= py + reach; ++y) {
                for (int z = pz - reach; z <= pz + reach; ++z) {
                    if (x < 0 || y < 0 || z < 0 || x >= lattice.size()
                        || y >= lattice.size() || z >= lattice.size()) {
                        continue;
                    }
                    const double dx = static_cast<double>(x) - sample.position.x;
                    const double dy = static_cast<double>(y) - sample.position.y;
                    const double dz = static_cast<double>(z) - sample.position.z;
                    const double r2 = dx * dx + dy * dy + dz * dz;
                    const int idx = lattice.index(x, y, z);
                    if (r2 <= mask_radius2) seed.local_mask[idx] = 1;
                    const double weight = std::exp(-r2 * inv_two_sigma2) * sample.ds;
                    seed.field[idx] += sample.tangent * weight;
                }
            }
        }
    }

    double raw_norm2 = 0.0;
    for (const auto& value : seed.field) raw_norm2 += value.mag2();
    const double scale = raw_norm2 > 0.0 ? 1.0 / std::sqrt(raw_norm2) : 0.0;
    seed.norm2 = 0.0;
    for (auto& value : seed.field) {
        value = value * scale;
        seed.norm2 += value.mag2();
    }
    return seed;
}

void configure_primary(ftd::RenderBridge& rb, bool strong_flags) {
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.strict_validation = true;
    rb.toggles.dual_substrate = false;
    rb.toggles.damping = false;
    rb.toggles.selective_damping = false;
    rb.toggles.genesis = false;
    rb.toggles.evaporation = false;
    rb.toggles.coupling = false;
    rb.toggles.forces = false;
    rb.toggles.movement = false;
    rb.toggles.color_forces = strong_flags;
    rb.toggles.strong_force = strong_flags;
    rb.toggles.confinement = strong_flags;
}

void configure_strong(ftd::RenderBridge& rb) {
    rb.toggles.disable_all();
    rb.toggles.strict_validation = true;
    rb.toggles.dual_substrate = false;
    rb.toggles.damping = false;
    rb.toggles.selective_damping = false;
    rb.toggles.genesis = false;
    rb.toggles.evaporation = false;
    rb.toggles.coupling = false;
    rb.toggles.wave_propagation = false;
    rb.toggles.gauss_projection = false;
    rb.toggles.forces = false;
    rb.toggles.movement = false;
    rb.toggles.strong_force = true;
    rb.toggles.color_forces = true;
    rb.toggles.confinement = true;
}

void install_seed(ftd::RenderBridge& rb, const SeedData& seed, Channel channel) {
    auto& voxels = rb.voxels();
    for (std::size_t i = 0; i < voxels.size(); ++i) {
        auto& voxel = voxels[i];
        voxel.state = 0;
        voxel.flux = {};
        voxel.wave_vel = {};
        voxel.flux_L = {};
        voxel.flux_R = {};
        voxel.wave_vel_L = {};
        voxel.wave_vel_R = {};
        voxel.flux_strong = {};
        voxel.wave_vel_strong = {};
        if (channel == Channel::Primary) voxel.flux = seed.field[i];
        else voxel.flux_strong = seed.field[i];
    }
}

const ftd::Vec3& field_for(const ftd::Voxel& voxel, Channel channel) {
    return channel == Channel::Primary ? voxel.flux : voxel.flux_strong;
}

const ftd::Vec3& velocity_for(const ftd::Voxel& voxel, Channel channel) {
    return channel == Channel::Primary ? voxel.wave_vel : voxel.wave_vel_strong;
}

Metrics measure(const ftd::RenderBridge& rb, const SeedData& seed,
                Channel channel, int tick) {
    const auto& voxels = rb.voxels();
    double distance2 = 0.0;
    double field_norm2 = 0.0;
    double full_norm2 = 0.0;
    double local_norm2 = 0.0;
    double overlap = 0.0;
    int manifested = 0;

    for (std::size_t i = 0; i < voxels.size(); ++i) {
        const auto& field = field_for(voxels[i], channel);
        const auto& velocity = velocity_for(voxels[i], channel);
        const auto delta = field - seed.field[i];
        distance2 += delta.mag2() + velocity.mag2();
        field_norm2 += field.mag2();
        const double local_energy = field.mag2() + velocity.mag2();
        full_norm2 += local_energy;
        if (seed.local_mask[i]) local_norm2 += local_energy;
        overlap += field.dot(seed.field[i]);
        if (voxels[i].state != 0) ++manifested;
    }

    Metrics result;
    result.tick = tick;
    result.recurrence_error = std::sqrt(distance2 / seed.norm2);
    result.shape_overlap = field_norm2 > 0.0
        ? overlap / std::sqrt(field_norm2 * seed.norm2) : 0.0;
    result.energy_ratio = full_norm2 / seed.norm2;
    result.local_fraction = full_norm2 > 0.0 ? local_norm2 / full_norm2 : 0.0;
    result.manifested = manifested;
    return result;
}

RunResult run_protocol(const std::string& name, const SeedData& seed,
                       Channel channel, bool strong_flags) {
    ftd::RenderBridge rb(LATTICE_SIZE);
    if (channel == Channel::Primary) configure_primary(rb, strong_flags);
    else configure_strong(rb);
    install_seed(rb, seed, channel);

    RunResult result;
    result.name = name;
    result.backend = rb.backend_kind();
    result.metrics.push_back(measure(rb, seed, channel, 0));
    for (int tick = 1; tick <= OBSERVATION_TICKS.back(); ++tick) {
        rb.tick();
        if (std::find(OBSERVATION_TICKS.begin(), OBSERVATION_TICKS.end(), tick)
            != OBSERVATION_TICKS.end()) {
            result.metrics.push_back(measure(rb, seed, channel, tick));
        }
    }

    const auto& voxels = static_cast<const ftd::RenderBridge&>(rb).voxels();
    result.final_field.reserve(voxels.size());
    result.final_velocity.reserve(voxels.size());
    result.final_state.reserve(voxels.size());
    for (const auto& voxel : voxels) {
        result.final_field.push_back(field_for(voxel, channel));
        result.final_velocity.push_back(velocity_for(voxel, channel));
        result.final_state.push_back(voxel.state);
    }
    return result;
}

double max_state_difference(const RunResult& a, const RunResult& b) {
    double maximum = 0.0;
    if (a.final_field.size() != b.final_field.size()) {
        return std::numeric_limits<double>::infinity();
    }
    for (std::size_t i = 0; i < a.final_field.size(); ++i) {
        maximum = std::max(maximum, vec_distance(a.final_field[i], b.final_field[i]));
        maximum = std::max(maximum, vec_distance(a.final_velocity[i], b.final_velocity[i]));
        if (a.final_state[i] != b.final_state[i]) {
            return std::numeric_limits<double>::infinity();
        }
    }
    return maximum;
}

void emit_rows(std::ostream& out, const RunResult& result) {
    for (const auto& row : result.metrics) {
        out << result.name << ',' << row.tick << ','
            << std::setprecision(17) << row.recurrence_error << ','
            << row.shape_overlap << ',' << row.energy_ratio << ','
            << row.local_fraction << ',' << row.manifested << '\n';
    }
}

bool finite_metrics(const RunResult& result) {
    return std::all_of(result.metrics.begin(), result.metrics.end(),
        [](const Metrics& m) {
            return std::isfinite(m.recurrence_error)
                && std::isfinite(m.shape_overlap)
                && std::isfinite(m.energy_ratio)
                && std::isfinite(m.local_fraction);
        });
}

}  // namespace

int main() {
    std::cout << "FTD dyadic C3 recurrence probe [EXPLORATORY]\n";
    std::cout << "Frozen atlas word: (+,-,+,-), amplitudes=(1,1/2,1/2,3/8), beta=2\n";
    std::cout << "No mass target or numerical parameter search is present.\n\n";

    int failures = 0;
    auto gate = [&](const char* label, bool pass) {
        std::cout << (pass ? "PASS  " : "FAIL  ") << label << '\n';
        if (!pass) ++failures;
    };

    const auto p0 = c3_position(0.0);
    const auto p1 = c3_position(2.0 * ftd::PI);
    const auto d0 = c3_derivative(0.0);
    const auto d1 = c3_derivative(2.0 * ftd::PI);
    const double closure_error = vec_distance(p0, p1);
    const double tangent_closure_error = vec_distance(d0, d1);
    std::cout << std::setprecision(17)
              << "Geometric period = 2*pi; position closure error = " << closure_error
              << "; tangent closure error = " << tangent_closure_error << '\n';
    std::cout << "Exact planar signed area = 3*pi/4 (atlas theorem; not engine energy).\n";
    gate("C3 lifted position and tangent close", closure_error < 1e-12
         && tangent_closure_error < 1e-12);

    ftd::Lattice geometry_lattice(LATTICE_SIZE);
    const auto curve = sample_curve();
    const auto seed = build_tangent_tube(geometry_lattice, curve);
    std::cout << "Normalized seed sum|J|^2 = " << seed.norm2
              << " (field energy = " << 0.5 * seed.norm2 << ")\n";
    gate("Target-blind tangent tube has finite unit norm",
         std::isfinite(seed.norm2) && std::abs(seed.norm2 - 1.0) < 1e-12);

    const auto native = run_protocol("primary-native", seed, Channel::Primary, false);
    const auto native_repeat = run_protocol("primary-native-repeat", seed, Channel::Primary, false);
    const auto flagged = run_protocol("primary-strong-flags", seed, Channel::Primary, true);
    const auto strong = run_protocol("strong-channel", seed, Channel::Strong, true);
    const auto strong_repeat = run_protocol("strong-channel-repeat", seed, Channel::Strong, true);

    const double deterministic_delta = max_state_difference(native, native_repeat);
    const double flag_delta = max_state_difference(native, flagged);
    const double strong_deterministic_delta = max_state_difference(strong, strong_repeat);
    gate("Duplicate native runs are bit-identical", deterministic_delta == 0.0);
    gate("Strong/confinement flags do not alter a field-only primary loop",
         flag_delta == 0.0);
    gate("All measured channels remain finite", finite_metrics(native)
         && finite_metrics(flagged) && finite_metrics(strong)
         && finite_metrics(strong_repeat));
    gate("Genesis-off protocols remain unmanifested",
         std::all_of(native.metrics.begin(), native.metrics.end(),
                     [](const Metrics& m) { return m.manifested == 0; })
         && std::all_of(strong.metrics.begin(), strong.metrics.end(),
                        [](const Metrics& m) { return m.manifested == 0; })
         && std::all_of(strong_repeat.metrics.begin(), strong_repeat.metrics.end(),
                        [](const Metrics& m) { return m.manifested == 0; }));

    std::cout << (strong_deterministic_delta == 0.0 ? "PASS  " : "INVALID  ")
              << "Strong-channel duplicate determinism; max delta = "
              << strong_deterministic_delta << '\n';

    std::ofstream csv("engine/results/dyadic_recurrence_probe.csv");
    csv << "protocol,tick,recurrence_error,shape_overlap,energy_ratio,local_fraction,manifested\n";
    emit_rows(csv, native);
    emit_rows(csv, flagged);
    emit_rows(csv, strong);
    csv.close();

    std::cout << "\nprotocol,tick,recurrence_error,shape_overlap,energy_ratio,local_fraction\n";
    auto print = [](const RunResult& result) {
        for (const auto& row : result.metrics) {
            std::cout << result.name << ',' << row.tick << ','
                      << std::setprecision(8) << row.recurrence_error << ','
                      << row.shape_overlap << ',' << row.energy_ratio << ','
                      << row.local_fraction << '\n';
        }
    };
    print(native);
    print(flagged);
    print(strong);

    std::cout << "\nBackend: "
              << (native.backend == ftd::Backend::Kind::Gpu ? "GPU" : "CPU") << '\n';
    std::cout << "Native repeat max delta: " << deterministic_delta << '\n';
    std::cout << "Primary strong-flag max delta: " << flag_delta << '\n';
    std::cout << "Strong-channel repeat max delta: " << strong_deterministic_delta << '\n';
    std::cout << "Strong-channel verdict: "
              << (strong_deterministic_delta == 0.0
                    ? "reproducible measurement"
                    : "INVALID-NONDETERMINISTIC (do not infer recurrence)")
              << '\n';
    std::cout << "CSV: engine/results/dyadic_recurrence_probe.csv\n";
    std::cout << "Verdict is determined from the printed metrics; non-recurrence is not a test failure.\n";
    return failures == 0 ? 0 : 1;
}
