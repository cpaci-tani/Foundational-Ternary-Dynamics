/**
 * @file campaign_native_dressed_hazard_ir_scaling.cpp
 * @brief FTD-0433 pole-phased dressed evaporation-hazard scaling campaign.
 */

#include "ftd/eft/native_dynamic_polarity_response.h"
#include "ftd/eft/native_evaporation_hazard_observer.h"
#include "ftd/eft/native_reaction_polarity_slow_mode.h"

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

constexpr std::array<int, 7> kGpuVolumes{12, 16, 20, 24, 32, 40, 48};
constexpr std::array<int, 3> kDirection{1, 0, 0};
constexpr int kHarmonic = 1;
constexpr int kSeeds = 8;

struct Args {
    int L = 32;
    std::string backend = "cpu";
    std::string backend_label = "unspecified";
    std::string output = "native_dressed_hazard_ir_scaling.csv";
};

struct PolePhase {
    double omega = 0.0;
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
        } else {
            std::cerr << "unknown argument: " << argv[i] << '\n';
        }
    }
    return args;
}

bool registered_volume(const Args& args) {
    if (args.backend == "cpu") return args.L == 32;
    for (int volume : kGpuVolumes)
        if (args.L == volume) return true;
    return false;
}

PolePhase pole_phase(int L) {
    const double k = 2.0 * ftd::PI / static_cast<double>(L);
    PolePhase out;
    out.omega = ftd::eft::native_discrete_pole({k, 0.0, 0.0});
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
    long long previous_occupancy = initial.occupancy;
    for (int tick = 0; tick <= out.pole.target_transition; ++tick) {
        TransitionRecord record;
        record.tick = tick;
        record.hazard = ftd::eft::observe_native_evaporation_hazard(
            bridge, tick, kHarmonic, kDirection);
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
    out << "backend_label,actual_backend,L,seed,dx,dy,dz,n,tick,omega,"
           "target_transition,phase_error,initial_source_real,"
           "initial_source_imag,initial_occupancy,initial_signed_state,"
           "source_before_real,source_before_imag,expected_loss_real,"
           "expected_loss_imag,predicted_next_real,predicted_next_imag,"
           "source_after_real,source_after_imag,source_hazard,"
           "expected_removals,removal_variance,actual_removed,"
           "occupancy_before,occupancy_after,signed_state_after,"
           "eligible_sites,mean_site_probability,min_site_probability,"
           "max_site_probability,mean_local_energy,max_local_energy,"
           "history_evaporation,history_other,history_required,"
           "history_enabled,toggles_valid,structural_valid,execution_valid\n";
}

void write_arm(std::ofstream& out, const Args& args, const ArmResult& arm) {
    for (const auto& record : arm.transitions) {
        const auto& h = record.hazard;
        out << args.backend_label << ',' << arm.actual_backend << ',' << args.L
            << ',' << arm.seed << ',' << kDirection[0] << ','
            << kDirection[1] << ',' << kDirection[2] << ',' << kHarmonic
            << ',' << record.tick << ',' << arm.pole.omega << ','
            << arm.pole.target_transition << ',' << arm.pole.phase_error << ','
            << arm.initial_source.real() << ',' << arm.initial_source.imag()
            << ',' << arm.initial_occupancy << ','
            << arm.initial_signed_state << ',' << h.source.real() << ','
            << h.source.imag() << ',' << h.expected_loss_source.real() << ','
            << h.expected_loss_source.imag() << ','
            << h.predicted_next_source.real() << ','
            << h.predicted_next_source.imag() << ','
            << record.source_after.real() << ',' << record.source_after.imag()
            << ',' << h.source_hazard << ',' << h.expected_removals << ','
            << h.removal_variance << ',' << record.actual_removed << ','
            << h.occupancy << ',' << record.occupancy_after << ','
            << record.signed_state_after << ',' << h.eligible_sites << ','
            << h.mean_site_probability << ',' << h.min_site_probability << ','
            << h.max_site_probability << ',' << h.mean_local_energy << ','
            << h.max_local_energy << ',' << record.history_evaporation << ','
            << record.history_other << ','
            << (arm.history_required ? 1 : 0) << ','
            << (arm.history_enabled ? 1 : 0) << ','
            << (arm.toggles_valid ? 1 : 0) << ','
            << (arm.structural_valid ? 1 : 0) << ','
            << (arm.execution_valid ? 1 : 0) << '\n';
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
    csv << std::setprecision(17);
    write_header(csv);

    bool execution_valid = true;
    int rows = 0;
    for (int seed = 0; seed < kSeeds; ++seed) {
        const auto arm = run_arm(args, seed);
        write_arm(csv, args, arm);
        rows += static_cast<int>(arm.transitions.size());
        execution_valid = execution_valid && arm.execution_valid;
        std::cout << "completed=" << (seed + 1) << '/' << kSeeds
                  << " target_transition=" << arm.pole.target_transition
                  << " phase_error=" << arm.pole.phase_error
                  << " valid=" << arm.execution_valid << '\n';
    }
    std::cout << "FTD-0433 native dressed hazard IR scaling"
              << " backend_label=" << args.backend_label
              << " L=" << args.L << " arms=" << kSeeds
              << " rows=" << rows
              << " execution_valid=" << execution_valid
              << " output=" << args.output << '\n';
    return execution_valid ? 0 : 1;
}
