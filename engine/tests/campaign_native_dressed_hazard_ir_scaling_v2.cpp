/**
 * @file campaign_native_dressed_hazard_ir_scaling_v2.cpp
 * @brief FTD-0436 phase-corrected dressed evaporation-hazard scaling campaign.
 *
 * Extends FTD-0433 (outcome C) per PREREG_NATIVE_DRESSED_HAZARD_IR_SCALING_v2:
 *   - volumes L in {48, 64, 96, 128, 192}, seeds 0..7 (CPU repro at L=48);
 *   - transitions recorded through t* + 2 (phase-interpolation bracket);
 *   - per-x-plane decomposition of the expected-loss projection at t*,
 *     with a closure gate against the locked FTD-0432 observer (G9).
 *
 * The FTD-0432 observer and the v1 campaign are reused/untouched; the plane
 * pass recomputes the identical production probability and must sum to the
 * observer's expected_loss_source to 1e-12 relative.
 */

#include "ftd/constants.h"
#include "ftd/eft/native_dynamic_polarity_response.h"
#include "ftd/eft/native_evaporation_hazard_observer.h"
#include "ftd/eft/native_reaction_polarity_slow_mode.h"
#include "ftd/proper_time_rate.h"

#include <array>
#include <cmath>
#include <complex>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

// Amendment A1 (2026-07-24, pre-data): primary backend is WSL2 CUDA for
// L in {48, 64, 96} and Windows/MSVC CPU for L in {128, 192}; the CPU
// L=48 reproduction remains. See PREREG_..._v2.md Amendment A1.
constexpr std::array<int, 3> kGpuVolumes{48, 64, 96};
constexpr std::array<int, 3> kCpuVolumes{48, 128, 192};
constexpr int kCpuVolume = 48;  // CPU/CUDA cross-check volume
constexpr std::array<int, 3> kDirection{1, 0, 0};
constexpr int kHarmonic = 1;
constexpr int kSeeds = 8;
constexpr int kBracket = 2;  // record through t* + kBracket

struct Args {
    int L = kCpuVolume;
    std::string backend = "cpu";
    std::string backend_label = "unspecified";
    std::string output = "native_dressed_hazard_ir_scaling_v2.csv";
    std::string plane_output = "native_dressed_hazard_ir_scaling_v2_planes.csv";
};

struct PolePhase {
    double omega = 0.0;
    double tau = 0.0;             // exact continuous transition pi/omega - 1
    int target_transition = -1;
    double phase_error = 0.0;
};

struct TransitionRecord {
    int tick = 0;
    ftd::eft::NativeEvaporationHazardObservation hazard{};
    std::complex<double> source_after{};
    long long signed_state_after = 0;
    long long occupancy_after = 0;
    long long actual_removed = 0;
    long long history_evaporation = 0;
    long long history_other = 0;
    double plane_closure_rel = 0.0;   // |sum(planes) - loss| / max(|loss|, eps)
};

struct ArmResult {
    int seed = 0;
    std::string actual_backend;
    PolePhase pole{};
    std::complex<double> initial_source{};
    long long initial_occupancy = 0;
    long long initial_signed_state = 0;
    bool history_required = false;
    bool history_enabled = false;
    bool toggles_valid = false;
    bool structural_valid = false;
    bool execution_valid = false;
    std::vector<TransitionRecord> transitions;
    // per-x-plane Re/Im of (1/N) * sum_{yz} s*p*e^{-ikx}, captured at t*
    std::vector<std::complex<double>> plane_loss_at_tstar;
};

Args parse_args(int argc, char** argv) {
    Args args;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--L") == 0 && i + 1 < argc) {
            args.L = static_cast<int>(std::strtol(argv[++i], nullptr, 10));
        } else if (std::strcmp(argv[i], "--backend") == 0 && i + 1 < argc) {
            args.backend = argv[++i];
        } else if (std::strcmp(argv[i], "--backend-label") == 0
                   && i + 1 < argc) {
            args.backend_label = argv[++i];
        } else if (std::strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
            args.output = argv[++i];
        } else if (std::strcmp(argv[i], "--plane-output") == 0
                   && i + 1 < argc) {
            args.plane_output = argv[++i];
        } else {
            std::cerr << "unknown argument: " << argv[i] << '\n';
        }
    }
    return args;
}

bool registered_volume(const Args& args) {
    if (args.backend == "cpu") {
        for (int volume : kCpuVolumes)
            if (args.L == volume) return true;
        return false;
    }
    for (int volume : kGpuVolumes)
        if (args.L == volume) return true;
    return false;
}

PolePhase pole_phase(int L) {
    const double k = 2.0 * ftd::PI / static_cast<double>(L);
    PolePhase out;
    out.omega = ftd::eft::native_discrete_pole({k, 0.0, 0.0});
    out.tau = ftd::PI / out.omega - 1.0;
    out.target_transition = static_cast<int>(std::llround(
        ftd::PI / out.omega)) - 1;
    out.phase_error = std::abs(
        static_cast<double>(out.target_transition + 1) * out.omega
        - ftd::PI);
    return out;
}

void configure(ftd::RenderBridge& bridge, int seed) {
    bridge.toggles.disable_all();
    bridge.toggles.evaporation = true;
    bridge.toggles.wave_propagation = true;
    bridge.toggles.coupling = true;
    bridge.toggles.dual_substrate = false;
    bridge.toggles.strict_validation = true;
    bridge.toggles.langevin_seed = static_cast<std::uint64_t>(seed);
    bridge.seed_rng(static_cast<unsigned int>(seed));
}

bool toggle_contract(const ftd::RenderBridge& bridge) {
    const auto& t = bridge.toggles;
    return t.evaporation && !t.genesis && t.wave_propagation && t.coupling
        && !t.dual_substrate && !t.gauss_projection
        && !t.matched_gauss_dynamics && !t.damping && !t.movement
        && !t.forces && !t.pair_production && !t.weak_transmutation
        && !t.poisson_coulomb && !t.emergent_forces && !t.langevin
        && !t.symplectic_leapfrog && !t.verlet_wave_integrator
        && !t.latency_field && !t.lorentz_period2_floquet
        && !t.lorentz_bcc_time_floquet;
}

void initialize_square_source(ftd::RenderBridge& bridge) {
    const int L = bridge.lattice().size();
    auto& voxels = bridge.voxels();
    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                const int index = bridge.lattice().index(x, y, z);
                bridge.set_state(index,
                    static_cast<std::int8_t>(x < L / 2 ? 1 : -1));
                auto& voxel = voxels[static_cast<std::size_t>(index)];
                voxel.flux = {};
                voxel.wave_vel = {};
                voxel.flux_L = {};
                voxel.flux_R = {};
                voxel.wave_vel_L = {};
                voxel.wave_vel_R = {};
                voxel.velocity = {};
                voxel.remainder = {};
                voxel.latency = 0.0;
                voxel.locked = false;
            }
        }
    }
}

/**
 * Per-x-plane decomposition of the expected-loss projection.
 *
 * Recomputes the FTD-0432 observer's site probability with the IDENTICAL
 * formula (predicted post-write fields from prepare_delta_j, seven-site
 * energy, proper-time-scaled clamped Boltzmann rate) and accumulates
 * (1/N) * sum_{yz} s_i * p_i * e^{-ik x} per x-plane. The caller must
 * verify closure against observation.expected_loss_source (gate G9);
 * closure certifies the reimplementation is exact.
 */
std::vector<std::complex<double>> plane_loss_decomposition(
    ftd::RenderBridge& bridge, int n, std::array<int, 3> direction) {
    bridge.prepare_delta_j();
    const auto& delta = bridge.delta_j();
    const auto& lattice = bridge.lattice();
    const auto& voxels =
        static_cast<const ftd::RenderBridge&>(bridge).voxels();
    const int L = lattice.size();
    const std::size_t total = lattice.total_sites();
    const double inverse_total = 1.0 / static_cast<double>(total);
    const double unit = 2.0 * ftd::PI * static_cast<double>(n)
        / static_cast<double>(L);

    std::vector<ftd::Vec3> predicted_flux(total);
    std::vector<ftd::Vec3> predicted_velocity(total);
    for (std::size_t index = 0; index < total; ++index) {
        predicted_velocity[index] = voxels[index].wave_vel + delta[index];
        predicted_flux[index] = voxels[index].flux
            + predicted_velocity[index];
    }

    std::vector<std::complex<double>> planes(static_cast<std::size_t>(L));
    for (int x = 0; x < L; ++x) {
        const double angle = unit * static_cast<double>(x)
            * static_cast<double>(direction[0]);
        const std::complex<double> phase{std::cos(angle), -std::sin(angle)};
        std::complex<double> acc{0.0, 0.0};
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                const int index = lattice.index(x, y, z);
                const auto& voxel = voxels[static_cast<std::size_t>(index)];
                if (voxel.state == 0 || voxel.locked) continue;
                double local_energy =
                    predicted_flux[static_cast<std::size_t>(index)].mag2()
                    + predicted_velocity[
                        static_cast<std::size_t>(index)].mag2();
                for (int neighbor : lattice.neighbors_6(index)) {
                    local_energy +=
                        predicted_flux[
                            static_cast<std::size_t>(neighbor)].mag2()
                        + predicted_velocity[
                            static_cast<std::size_t>(neighbor)].mag2();
                }
                const double dtau = ftd::proper_time_rate(
                    voxel.latency, voxel.speed() * voxel.speed());
                const double probability = std::clamp(
                    ftd::K_EVAP_RATE * dtau
                        * std::exp(-local_energy
                            / (ftd::K_MANIFEST * ftd::K_MANIFEST)),
                    0.0, ftd::K_EVAP_RATE);
                acc += static_cast<double>(voxel.state) * probability
                    * phase;
            }
        }
        planes[static_cast<std::size_t>(x)] = acc * inverse_total;
    }
    return planes;
}

ArmResult run_arm(const Args& args, int seed) {
    ArmResult out;
    out.seed = seed;
    out.pole = pole_phase(args.L);
    ftd::RenderBridge bridge(args.L);
    if (args.backend == "cpu") bridge.force_cpu();
    out.actual_backend = bridge.backend_kind() == ftd::Backend::Kind::Gpu
        ? "gpu" : "cpu";
    configure(bridge, seed);
    initialize_square_source(bridge);
    out.toggles_valid = toggle_contract(bridge);
    out.history_required = args.backend == "cpu";
    out.history_enabled = out.history_required
        ? bridge.enable_history_journal(true) : false;

    const auto initial = ftd::eft::measure_native_reaction_mode(
        bridge, 0, kHarmonic, kDirection);
    out.initial_source = initial.source;
    out.initial_occupancy = initial.occupancy;
    out.initial_signed_state = initial.signed_state;

    bool valid = out.pole.omega > 0.0
        && out.pole.target_transition >= 0
        && out.pole.phase_error <= 0.5 * out.pole.omega + 1e-14;
    const int last_transition = out.pole.target_transition + kBracket;
    long long previous_occupancy = initial.occupancy;
    for (int tick = 0; tick <= last_transition; ++tick) {
        TransitionRecord record;
        record.tick = tick;
        record.hazard = ftd::eft::observe_native_evaporation_hazard(
            bridge, tick, kHarmonic, kDirection);

        if (tick == out.pole.target_transition) {
            out.plane_loss_at_tstar =
                plane_loss_decomposition(bridge, kHarmonic, kDirection);
            std::complex<double> sum{0.0, 0.0};
            for (const auto& p : out.plane_loss_at_tstar) sum += p;
            const double denom = std::max(
                std::abs(record.hazard.expected_loss_source), 1e-300);
            record.plane_closure_rel =
                std::abs(sum - record.hazard.expected_loss_source) / denom;
            valid = valid && record.plane_closure_rel <= 1e-12;
        }

        bridge.tick();
        const auto after = ftd::eft::measure_native_reaction_mode(
            bridge, tick + 1, kHarmonic, kDirection);
        record.source_after = after.source;
        record.signed_state_after = after.signed_state;
        record.occupancy_after = after.occupancy;
        record.actual_removed = previous_occupancy - after.occupancy;
        previous_occupancy = after.occupancy;

        if (out.history_required) {
            for (const auto& event : bridge.history_events()) {
                if (event.kind == ftd::eft::HistoryEventKind::Evaporation)
                    ++record.history_evaporation;
                else
                    ++record.history_other;
            }
            valid = valid
                && record.history_evaporation == record.actual_removed
                && record.history_other == 0;
        }

        const double before_projection = (
            record.hazard.source * std::conj(out.initial_source)).real();
        const double after_projection = (
            record.source_after * std::conj(out.initial_source)).real();
        valid = valid
            && record.actual_removed >= 0
            && record.hazard.occupancy == previous_occupancy
                + record.actual_removed
            && record.hazard.min_site_probability >= 0.0
            && record.hazard.max_site_probability <= 0.1 + 1e-15
            && record.hazard.expected_removals >= 0.0
            && record.hazard.removal_variance >= 0.0
            && std::isfinite(record.hazard.expected_removals)
            && std::isfinite(record.hazard.removal_variance)
            && std::isfinite(record.hazard.source_hazard)
            && before_projection > 0.0 && after_projection > 0.0;
        out.transitions.push_back(record);
    }

    const bool initial_valid = out.initial_occupancy
            == static_cast<long long>(bridge.lattice().total_sites())
        && out.initial_signed_state == 0
        && std::abs(out.initial_source) >= 0.3;
    const bool backend_valid = out.actual_backend == args.backend;
    out.structural_valid = initial_valid && valid
        && (!out.history_required || out.history_enabled);
    out.execution_valid = backend_valid && out.toggles_valid
        && out.structural_valid;
    return out;
}

void write_header(std::ofstream& out) {
    out << "backend_label,actual_backend,L,seed,dx,dy,dz,n,tick,omega,tau,"
           "target_transition,phase_error,initial_source_real,"
           "initial_source_imag,initial_occupancy,initial_signed_state,"
           "source_before_real,source_before_imag,expected_loss_real,"
           "expected_loss_imag,predicted_next_real,predicted_next_imag,"
           "source_after_real,source_after_imag,source_hazard,"
           "expected_removals,removal_variance,actual_removed,"
           "occupancy_before,occupancy_after,signed_state_after,"
           "eligible_sites,mean_site_probability,min_site_probability,"
           "max_site_probability,mean_local_energy,max_local_energy,"
           "plane_closure_rel,history_evaporation,history_other,"
           "history_required,history_enabled,toggles_valid,structural_valid,"
           "execution_valid\n";
}

void write_arm(std::ofstream& out, const Args& args, const ArmResult& arm) {
    for (const auto& record : arm.transitions) {
        const auto& h = record.hazard;
        out << args.backend_label << ',' << arm.actual_backend << ','
            << args.L << ',' << arm.seed << ',' << kDirection[0] << ','
            << kDirection[1] << ',' << kDirection[2] << ',' << kHarmonic
            << ',' << record.tick << ',' << arm.pole.omega << ','
            << arm.pole.tau << ',' << arm.pole.target_transition << ','
            << arm.pole.phase_error << ',' << arm.initial_source.real()
            << ',' << arm.initial_source.imag() << ','
            << arm.initial_occupancy << ',' << arm.initial_signed_state
            << ',' << h.source.real() << ',' << h.source.imag() << ','
            << h.expected_loss_source.real() << ','
            << h.expected_loss_source.imag() << ','
            << h.predicted_next_source.real() << ','
            << h.predicted_next_source.imag() << ','
            << record.source_after.real() << ','
            << record.source_after.imag() << ',' << h.source_hazard << ','
            << h.expected_removals << ',' << h.removal_variance << ','
            << record.actual_removed << ',' << h.occupancy << ','
            << record.occupancy_after << ',' << record.signed_state_after
            << ',' << h.eligible_sites << ',' << h.mean_site_probability
            << ',' << h.min_site_probability << ','
            << h.max_site_probability << ',' << h.mean_local_energy << ','
            << h.max_local_energy << ',' << record.plane_closure_rel << ','
            << record.history_evaporation << ',' << record.history_other
            << ',' << (arm.history_required ? 1 : 0) << ','
            << (arm.history_enabled ? 1 : 0) << ','
            << (arm.toggles_valid ? 1 : 0) << ','
            << (arm.structural_valid ? 1 : 0) << ','
            << (arm.execution_valid ? 1 : 0) << '\n';
    }
}

void write_plane_header(std::ofstream& out) {
    out << "backend_label,actual_backend,L,seed,tick,x,"
           "plane_loss_real,plane_loss_imag\n";
}

void write_arm_planes(std::ofstream& out, const Args& args,
                      const ArmResult& arm) {
    for (std::size_t x = 0; x < arm.plane_loss_at_tstar.size(); ++x) {
        out << args.backend_label << ',' << arm.actual_backend << ','
            << args.L << ',' << arm.seed << ','
            << arm.pole.target_transition << ',' << x << ','
            << arm.plane_loss_at_tstar[x].real() << ','
            << arm.plane_loss_at_tstar[x].imag() << '\n';
    }
}

}  // namespace

int main(int argc, char** argv) {
    const Args args = parse_args(argc, argv);
    if (args.backend != "cpu" && args.backend != "gpu") {
        std::cerr << "backend must be cpu or gpu\n";
        return 2;
    }
    if (!registered_volume(args)) {
        std::cerr << "unregistered backend/volume pair\n";
        return 2;
    }

    const fs::path output_path(args.output);
    if (!output_path.parent_path().empty())
        fs::create_directories(output_path.parent_path());
    std::ofstream csv(output_path);
    if (!csv) {
        std::cerr << "could not open output: " << args.output << '\n';
        return 2;
    }
    const fs::path plane_path(args.plane_output);
    if (!plane_path.parent_path().empty())
        fs::create_directories(plane_path.parent_path());
    std::ofstream plane_csv(plane_path);
    if (!plane_csv) {
        std::cerr << "could not open plane output: " << args.plane_output
                  << '\n';
        return 2;
    }
    csv << std::setprecision(17);
    plane_csv << std::setprecision(17);
    write_header(csv);
    write_plane_header(plane_csv);

    bool execution_valid = true;
    int rows = 0;
    for (int seed = 0; seed < kSeeds; ++seed) {
        const auto arm = run_arm(args, seed);
        write_arm(csv, args, arm);
        write_arm_planes(plane_csv, args, arm);
        rows += static_cast<int>(arm.transitions.size());
        execution_valid = execution_valid && arm.execution_valid;
        std::cout << "completed=" << (seed + 1) << '/' << kSeeds
                  << " target_transition=" << arm.pole.target_transition
                  << " tau=" << arm.pole.tau
                  << " phase_error=" << arm.pole.phase_error
                  << " valid=" << arm.execution_valid << '\n';
    }
    std::cout << "FTD-0436 native dressed hazard IR scaling v2"
              << " backend_label=" << args.backend_label
              << " L=" << args.L << " arms=" << kSeeds
              << " rows=" << rows
              << " execution_valid=" << execution_valid
              << " output=" << args.output << '\n';
    return execution_valid ? 0 : 1;
}
