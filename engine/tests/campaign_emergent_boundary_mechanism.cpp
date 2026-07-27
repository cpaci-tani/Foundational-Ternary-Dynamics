/**
 * @file campaign_emergent_boundary_mechanism.cpp
 * @brief FTD-0474 membrane/environment/periodic-boundary discriminator.
 */

#include "ftd/eft/emergent_boundary_observer.h"
#include "ftd/eft/history_event_journal.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <string>
#include <tuple>
#include <vector>

namespace fs = std::filesystem;

namespace {

constexpr int kTicks = 300;
constexpr std::array<int, 6> kSampleTicks{{150, 180, 210, 240, 270, 300}};
constexpr std::array<int, 2> kVolumes{{24, 32}};
constexpr std::array<int, 3> kAmplitudes{{12, 20, 40}};
constexpr std::array<std::uint32_t, 4> kSeeds{{
    0xE0102000u, 0xE0102001u, 0xE0102002u, 0xE0102003u}};

enum class Arm {
    ReactionPeriodic,
    ReactionDispersal,
    ThermalDispersal,
    MobileDispersal,
};

constexpr std::array<Arm, 4> kArms{{
    Arm::ReactionPeriodic,
    Arm::ReactionDispersal,
    Arm::ThermalDispersal,
    Arm::MobileDispersal,
}};

const char* arm_name(Arm arm) {
    switch (arm) {
        case Arm::ReactionPeriodic: return "reaction_periodic";
        case Arm::ReactionDispersal: return "reaction_dispersal";
        case Arm::ThermalDispersal: return "thermal_dispersal";
        case Arm::MobileDispersal: return "mobile_dispersal";
    }
    return "unknown";
}

void configure(ftd::RenderBridge& rb, Arm arm, std::uint32_t seed) {
    rb.force_cpu();
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.genesis = true;
    rb.toggles.dual_substrate = false;
    rb.toggles.reflective_boundary = false;
    rb.toggles.flux_boundary = arm == Arm::ReactionPeriodic
        ? ftd::FluxBoundaryMode::Periodic : ftd::FluxBoundaryMode::Dispersal;
    if (arm == Arm::ThermalDispersal) {
        rb.toggles.langevin = true;
        rb.toggles.langevin_T = 0.005;
        rb.toggles.langevin_gamma = 0.02;
    }
    if (arm == Arm::MobileDispersal) {
        rb.toggles.coupling = true;
        rb.toggles.forces = true;
        rb.toggles.movement = true;
        rb.toggles.gravity = false;
        rb.toggles.poisson_coulomb = false;
        rb.toggles.emergent_forces = false;
        rb.toggles.lorentz_force = false;
        rb.toggles.strong_force = false;
        rb.toggles.color_forces = false;
        rb.toggles.weak_transmutation = false;
        rb.toggles.pair_production = false;
    }
    rb.toggles.langevin_seed = seed;
    rb.seed_rng(seed);
}

void hash_bytes(std::uint64_t& hash, const void* data, std::size_t size) {
    const auto* bytes = static_cast<const unsigned char*>(data);
    for (std::size_t i = 0; i < size; ++i) {
        hash ^= static_cast<std::uint64_t>(bytes[i]);
        hash *= 1099511628211ull;
    }
}

template <typename T>
void hash_value(std::uint64_t& hash, const T& value) {
    hash_bytes(hash, &value, sizeof(value));
}

std::uint64_t selected_state_hash(const ftd::RenderBridge& rb) {
    std::uint64_t hash = 1469598103934665603ull;
    hash_value(hash, rb.current_tick());
    hash_value(hash, rb.physical_time());
    for (const auto& v : rb.voxels()) {
        hash_value(hash, v.state);
        for (const auto& q : {v.flux, v.wave_vel, v.flux_L, v.flux_R,
                             v.wave_vel_L, v.wave_vel_R, v.velocity,
                             v.remainder, v.flux_strong, v.wave_vel_strong,
                             v.flux_weak, v.wave_vel_weak}) {
            hash_value(hash, q.x); hash_value(hash, q.y); hash_value(hash, q.z);
        }
        hash_value(hash, v.latency); hash_value(hash, v.tau);
        hash_value(hash, v.phase); hash_value(hash, v.locked);
        hash_value(hash, v.particle_id); hash_value(hash, v.pair_id);
        hash_value(hash, v.spin); hash_value(hash, v.color);
        hash_value(hash, v.flavor); hash_value(hash, v.accel_mag);
    }
    return hash;
}

double mean(const std::vector<double>& values) {
    if (values.empty()) return 0.0;
    return std::accumulate(values.begin(), values.end(), 0.0)
        / static_cast<double>(values.size());
}

double coefficient_of_variation(const std::vector<double>& values) {
    if (values.empty()) return 0.0;
    const double m = mean(values);
    if (std::abs(m) <= 1e-30) return 0.0;
    double variance = 0.0;
    for (double x : values) variance += (x - m) * (x - m);
    variance /= static_cast<double>(values.size());
    return std::sqrt(variance) / std::abs(m);
}

struct RunResult {
    Arm arm = Arm::ReactionPeriodic;
    int L = 0;
    int amplitude = 0;
    std::uint32_t seed = 0;
    std::vector<ftd::eft::EmergentBoundaryObservation> samples;
    long long genesis_events = 0;
    long long evaporation_events = 0;
    long long movement_events = 0;
    long long annihilation_events = 0;
    bool stable = false;
    bool dynamically_active = false;
    bool mechanically_eligible = false;
    bool finite = true;
    double mean_area = 0.0;
    double mean_laplace = 0.0;
    double mean_interface_fraction = 0.0;
};

bool is_sample_tick(int tick) {
    return std::find(kSampleTicks.begin(), kSampleTicks.end(), tick)
        != kSampleTicks.end();
}

RunResult run_one(Arm arm, int L, int amplitude, std::uint32_t seed) {
    RunResult result;
    result.arm = arm;
    result.L = L;
    result.amplitude = amplitude;
    result.seed = seed;

    ftd::RenderBridge rb(L);
    configure(rb, arm, seed);
    if (!rb.enable_history_journal(true)) result.finite = false;
    const int center = L / 2;
    rb.inject_flux(center, center, center,
                   {amplitude * ftd::K_GENESIS, 0.0, 0.0});

    for (int tick = 1; tick <= kTicks; ++tick) {
        rb.tick();
        if (tick >= kSampleTicks.front()) {
            for (const auto& event : rb.history_events()) {
                switch (event.kind) {
                    case ftd::eft::HistoryEventKind::Genesis:
                        ++result.genesis_events; break;
                    case ftd::eft::HistoryEventKind::Evaporation:
                        ++result.evaporation_events; break;
                    case ftd::eft::HistoryEventKind::Movement:
                        ++result.movement_events; break;
                    case ftd::eft::HistoryEventKind::Annihilation:
                        ++result.annihilation_events; break;
                    default: break;
                }
            }
        }
        rb.clear_history_events();
        if (is_sample_tick(tick)) {
            auto observation = ftd::eft::observe_emergent_boundary(
                rb, center, center, center);
            const std::array<double, 13> scalars{{
                observation.centroid_x, observation.centroid_y,
                observation.centroid_z, observation.centroid_displacement,
                observation.rms_radius, observation.volume_radius,
                observation.area_coefficient,
                observation.radial_traction_jump,
                observation.laplace_coefficient,
                observation.interface_gradient_fraction,
                observation.wave_kinetic_energy,
                static_cast<double>(observation.occupancy),
                static_cast<double>(observation.boundary_sites)}};
            for (double scalar : scalars)
                result.finite = result.finite && std::isfinite(scalar);
            result.samples.push_back(observation);
        }
    }

    std::vector<double> occupancy;
    std::vector<double> radii;
    std::vector<double> areas;
    std::vector<double> laplace;
    std::vector<double> interface_fraction;
    int valid_count = 0;
    int interior_count = 0;
    bool size_gate = true;
    double kinetic_mean = 0.0;
    for (const auto& sample : result.samples) {
        if (sample.valid) ++valid_count;
        if (sample.interior_sites > 0) ++interior_count;
        size_gate = size_gate && sample.occupancy >= 4
            && sample.occupancy <= static_cast<int>(0.01 * L * L * L);
        occupancy.push_back(static_cast<double>(sample.occupancy));
        radii.push_back(sample.rms_radius);
        areas.push_back(sample.area_coefficient);
        laplace.push_back(sample.laplace_coefficient);
        interface_fraction.push_back(sample.interface_gradient_fraction);
        kinetic_mean += sample.wave_kinetic_energy;
    }
    kinetic_mean /= static_cast<double>(result.samples.size());
    result.stable = valid_count == static_cast<int>(kSampleTicks.size())
        && size_gate && coefficient_of_variation(occupancy) <= 0.20
        && coefficient_of_variation(radii) <= 0.15;
    const long long event_count = result.genesis_events + result.evaporation_events
                                + result.movement_events + result.annihilation_events;
    result.dynamically_active = result.stable
        && (event_count > 0 || kinetic_mean > 1e-10);
    result.mechanically_eligible = arm == Arm::MobileDispersal
        && result.dynamically_active && result.movement_events > 0
        && interior_count >= 4;
    result.mean_area = mean(areas);
    result.mean_laplace = mean(laplace);
    result.mean_interface_fraction = mean(interface_fraction);
    return result;
}

bool observer_neutrality() {
    ftd::RenderBridge control(24);
    ftd::RenderBridge observed(24);
    configure(control, Arm::ReactionPeriodic, kSeeds[0]);
    configure(observed, Arm::ReactionPeriodic, kSeeds[0]);
    const int c = 12;
    control.inject_flux(c, c, c, {20.0 * ftd::K_GENESIS, 0, 0});
    observed.inject_flux(c, c, c, {20.0 * ftd::K_GENESIS, 0, 0});
    if (!observed.enable_history_journal(true)) return false;
    for (int tick = 0; tick < 64; ++tick) {
        control.tick();
        observed.tick();
    }
    return selected_state_hash(control) == selected_state_hash(observed)
        && control.rng_state_hash() == observed.rng_state_hash();
}

std::vector<const RunResult*> cell_runs(
        const std::vector<RunResult>& runs, Arm arm, int L, int amplitude) {
    std::vector<const RunResult*> out;
    for (const auto& run : runs)
        if (run.arm == arm && run.L == L && run.amplitude == amplitude)
            out.push_back(&run);
    return out;
}

double stable_dynamic_fraction(const std::vector<RunResult>& runs, Arm arm) {
    int count = 0;
    int passed = 0;
    for (const auto& run : runs) {
        if (run.arm != arm) continue;
        ++count;
        if (run.stable && run.dynamically_active) ++passed;
    }
    return count ? static_cast<double>(passed) / count : 0.0;
}

double relative_difference(double a, double b) {
    const double denom = std::max(std::abs(a), std::abs(b));
    return denom > 0.0 ? std::abs(a - b) / denom : 0.0;
}

}  // namespace

int main() {
    const fs::path output_dir = fs::path(__FILE__).parent_path().parent_path()
                              / "results" / "ftd_0474";
    fs::create_directories(output_dir);
    std::vector<RunResult> runs;
    runs.reserve(kArms.size() * kVolumes.size()
                 * kAmplitudes.size() * kSeeds.size());

    for (Arm arm : kArms)
        for (int L : kVolumes)
            for (int amplitude : kAmplitudes)
                for (std::uint32_t seed : kSeeds) {
                    std::cout << "FTD-0474 arm=" << arm_name(arm)
                              << " L=" << L << " A=" << amplitude
                              << " seed=0x" << std::hex << seed << std::dec
                              << std::endl;
                    runs.push_back(run_one(arm, L, amplitude, seed));
                }

    std::ofstream sample_csv(output_dir / "samples.csv");
    sample_csv << std::setprecision(17)
        << "arm,L,amplitude,seed,sample_tick,valid,N,B,I,centroid_x,centroid_y,"
           "centroid_z,centroid_displacement,rms_radius,volume_radius,"
           "area_coefficient,radial_traction_jump,laplace_coefficient,"
           "interface_gradient_fraction,wave_kinetic_energy\n";
    std::ofstream run_csv(output_dir / "windows_msvc_cpu.csv");
    run_csv << std::setprecision(17)
        << "arm,L,amplitude,seed,stable,dynamically_active,mechanically_eligible,"
           "genesis_events,evaporation_events,movement_events,annihilation_events,"
           "mean_area_coefficient,mean_laplace_coefficient,"
           "mean_interface_gradient_fraction,finite\n";

    int sample_rows = 0;
    bool all_finite = true;
    for (const auto& run : runs) {
        for (std::size_t i = 0; i < run.samples.size(); ++i) {
            const auto& s = run.samples[i];
            sample_csv << arm_name(run.arm) << ',' << run.L << ','
                << run.amplitude << ',' << run.seed << ',' << kSampleTicks[i]
                << ',' << s.valid << ',' << s.occupancy << ','
                << s.boundary_sites << ',' << s.interior_sites << ','
                << s.centroid_x << ',' << s.centroid_y << ',' << s.centroid_z
                << ',' << s.centroid_displacement << ',' << s.rms_radius
                << ',' << s.volume_radius << ',' << s.area_coefficient
                << ',' << s.radial_traction_jump << ',' << s.laplace_coefficient
                << ',' << s.interface_gradient_fraction << ','
                << s.wave_kinetic_energy << '\n';
            ++sample_rows;
        }
        run_csv << arm_name(run.arm) << ',' << run.L << ',' << run.amplitude
            << ',' << run.seed << ',' << run.stable << ','
            << run.dynamically_active << ',' << run.mechanically_eligible << ','
            << run.genesis_events << ',' << run.evaporation_events << ','
            << run.movement_events << ',' << run.annihilation_events << ','
            << run.mean_area << ',' << run.mean_laplace << ','
            << run.mean_interface_fraction << ',' << run.finite << '\n';
        all_finite = all_finite && run.finite;
    }

    bool mobile_cells = true;
    bool reaction_cells = true;
    std::map<int, std::vector<double>> mobile_area_by_L;
    std::map<int, std::vector<double>> mobile_laplace_by_L;
    std::map<int, std::vector<double>> mobile_interface_by_L;
    std::map<int, std::vector<double>> reaction_interface_by_L;
    for (int L : kVolumes) {
        for (int amplitude : kAmplitudes) {
            const auto mobile = cell_runs(runs, Arm::MobileDispersal, L, amplitude);
            const auto reaction = cell_runs(runs, Arm::ReactionDispersal, L, amplitude);
            int mechanical_pass = 0;
            int reaction_pass = 0;
            std::vector<double> cell_area, cell_laplace;
            for (const auto* run : mobile) {
                if (run->mechanically_eligible) ++mechanical_pass;
                if (run->mechanically_eligible) {
                    cell_area.push_back(run->mean_area);
                    cell_laplace.push_back(run->mean_laplace);
                    mobile_interface_by_L[L].push_back(run->mean_interface_fraction);
                }
            }
            for (const auto* run : reaction) {
                if (run->stable && run->dynamically_active) ++reaction_pass;
                if (run->stable && run->dynamically_active)
                    reaction_interface_by_L[L].push_back(run->mean_interface_fraction);
            }
            mobile_cells = mobile_cells && mechanical_pass >= 3;
            reaction_cells = reaction_cells && reaction_pass >= 3;
            mobile_area_by_L[L].push_back(mean(cell_area));
            mobile_laplace_by_L[L].push_back(mean(cell_laplace));
        }
    }

    bool area_gate = true;
    bool laplace_gate = true;
    bool interface_gate = true;
    std::map<int, double> area_volume_mean;
    std::map<int, double> laplace_volume_mean;
    for (int L : kVolumes) {
        area_gate = area_gate
            && coefficient_of_variation(mobile_area_by_L[L]) <= 0.30;
        const auto& lp = mobile_laplace_by_L[L];
        std::vector<double> abs_lp;
        bool positive = true, negative = true, nonzero = true;
        for (double value : lp) {
            positive = positive && value > 0.0;
            negative = negative && value < 0.0;
            nonzero = nonzero && std::abs(value) > 1e-12;
            abs_lp.push_back(std::abs(value));
        }
        laplace_gate = laplace_gate && nonzero && (positive || negative)
            && coefficient_of_variation(abs_lp) <= 0.35;
        interface_gate = interface_gate
            && mean(mobile_interface_by_L[L]) >= 0.50
            && mean(reaction_interface_by_L[L]) >= 0.50;
        area_volume_mean[L] = mean(mobile_area_by_L[L]);
        laplace_volume_mean[L] = mean(abs_lp);
    }
    const bool volume_gate = relative_difference(
            area_volume_mean[kVolumes[0]], area_volume_mean[kVolumes[1]]) <= 0.30
        && relative_difference(laplace_volume_mean[kVolumes[0]],
                               laplace_volume_mean[kVolumes[1]]) <= 0.30;
    const bool membrane = mobile_cells && reaction_cells && area_gate
        && laplace_gate && interface_gate && volume_gate;

    const double periodic_fraction = stable_dynamic_fraction(
        runs, Arm::ReactionPeriodic);
    const double dispersal_fraction = stable_dynamic_fraction(
        runs, Arm::ReactionDispersal);
    const double thermal_fraction = stable_dynamic_fraction(
        runs, Arm::ThermalDispersal);
    const double mobile_fraction = stable_dynamic_fraction(
        runs, Arm::MobileDispersal);

    std::string verdict;
    if (membrane) {
        verdict = "MECHANICAL_MEMBRANE_SUPPORTED";
    } else if (thermal_fraction >= 0.75 && dispersal_fraction <= 0.25) {
        verdict = "EXPLICIT_ENVIRONMENT_SUPPORT_ONLY";
    } else if (periodic_fraction >= 0.75 && dispersal_fraction <= 0.25) {
        verdict = "PERIODIC_RECIRCULATION_SUPPORT_ONLY";
    } else if (periodic_fraction > 0.0 || dispersal_fraction > 0.0
               || thermal_fraction > 0.0) {
        verdict = "REACTION_FRONT_ONLY";
    } else if (periodic_fraction < 0.25 && dispersal_fraction < 0.25
               && thermal_fraction < 0.25 && mobile_fraction < 0.25) {
        verdict = "NO_QUALIFIED_FINITE_STRUCTURE";
    } else {
        verdict = "MIXED_OR_UNRESOLVED_BOUNDARY_MECHANISM";
    }

    const bool neutral = observer_neutrality();
    const bool structural_valid = runs.size() == 96 && sample_rows == 576
        && all_finite && neutral;

    std::ofstream verdict_file(output_dir / "verdict.txt");
    verdict_file << std::setprecision(17)
        << "verdict=" << verdict << '\n'
        << "structural_valid=" << structural_valid << '\n'
        << "observer_neutral=" << neutral << '\n'
        << "run_rows=" << runs.size() << '\n'
        << "sample_rows=" << sample_rows << '\n'
        << "periodic_stable_dynamic_fraction=" << periodic_fraction << '\n'
        << "dispersal_stable_dynamic_fraction=" << dispersal_fraction << '\n'
        << "thermal_stable_dynamic_fraction=" << thermal_fraction << '\n'
        << "mobile_stable_dynamic_fraction=" << mobile_fraction << '\n'
        << "mobile_cells=" << mobile_cells << '\n'
        << "reaction_cells=" << reaction_cells << '\n'
        << "area_gate=" << area_gate << '\n'
        << "laplace_gate=" << laplace_gate << '\n'
        << "interface_gate=" << interface_gate << '\n'
        << "volume_gate=" << volume_gate << '\n';

    std::cout << std::setprecision(8)
              << "FTD-0474 verdict=" << verdict << '\n'
              << "  stable/dynamic fractions periodic=" << periodic_fraction
              << " dispersal=" << dispersal_fraction
              << " thermal=" << thermal_fraction
              << " mobile=" << mobile_fraction << '\n'
              << "  membrane gates mobile_cells=" << mobile_cells
              << " reaction_cells=" << reaction_cells
              << " area=" << area_gate << " laplace=" << laplace_gate
              << " interface=" << interface_gate
              << " volume=" << volume_gate << '\n'
              << "  structural_valid=" << structural_valid
              << " observer_neutral=" << neutral << std::endl;
    return structural_valid ? 0 : 1;
}
