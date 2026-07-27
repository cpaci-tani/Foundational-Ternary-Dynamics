/**
 * @file campaign_native_dressed_evaporation_hazard.cpp
 * @brief FTD-0432 exact conditional evaporation-hazard campaign.
 */

#include "ftd/eft/native_evaporation_hazard_observer.h"
#include "ftd/eft/native_reaction_polarity_slow_mode.h"

#include <algorithm>
#include <array>
#include <cmath>
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

constexpr int kTransitions = 32;

struct Args {
    int L = 32;
    std::string backend = "cpu";
    std::string backend_label = "unspecified";
    std::string output = "native_dressed_evaporation_hazard.csv";
};

struct ArmSpec {
    std::string arm;
    int seed = 0;
    std::array<int, 3> direction{};
    int n = 1;
};

struct TransitionRecord {
    int tick = 0;
    ftd::eft::NativeEvaporationHazardObservation hazard{};
    std::complex<double> source_after{};
    long long occupancy_after = 0;
    long long actual_removed = 0;
    long long history_evaporation = 0;
    long long history_other = 0;
};

struct ArmResult {
    ArmSpec spec;
    std::string actual_backend;
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

bool is_coupled(const ArmSpec& spec) {
    return spec.arm == "coupled" || spec.arm == "locked_control";
}

bool is_locked(const ArmSpec& spec) {
    return spec.arm == "locked_control";
}

void configure(ftd::RenderBridge& bridge, const ArmSpec& spec) {
    bridge.toggles.disable_all();
    bridge.toggles.evaporation = true;
    bridge.toggles.wave_propagation = is_coupled(spec);
    bridge.toggles.coupling = is_coupled(spec);
    bridge.toggles.dual_substrate = false;
    bridge.toggles.strict_validation = true;
    bridge.toggles.langevin_seed = static_cast<std::uint64_t>(spec.seed);
    bridge.seed_rng(static_cast<unsigned int>(spec.seed));
}

bool toggle_contract(const ftd::RenderBridge& bridge, const ArmSpec& spec) {
    const auto& t = bridge.toggles;
    return t.evaporation && !t.genesis
        && t.wave_propagation == is_coupled(spec)
        && t.coupling == is_coupled(spec)
        && !t.dual_substrate && !t.gauss_projection
        && !t.matched_gauss_dynamics && !t.damping && !t.movement
        && !t.forces && !t.pair_production && !t.weak_transmutation
        && !t.poisson_coulomb && !t.emergent_forces && !t.langevin
        && !t.symplectic_leapfrog && !t.verlet_wave_integrator
        && !t.latency_field && !t.lorentz_period2_floquet
        && !t.lorentz_bcc_time_floquet;
}

void initialize_square_source(ftd::RenderBridge& bridge, const ArmSpec& spec) {
    const int L = bridge.lattice().size();
    auto& voxels = bridge.voxels();
    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                int phase = spec.n * (
                    spec.direction[0] * x + spec.direction[1] * y
                    + spec.direction[2] * z);
                phase %= L;
                if (phase < 0) phase += L;
                const int index = bridge.lattice().index(x, y, z);
                bridge.set_state(index,
                    static_cast<std::int8_t>(phase < L / 2 ? 1 : -1));
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
                voxel.locked = is_locked(spec);
            }
        }
    }
}

ArmResult run_arm(const Args& args, const ArmSpec& spec) {
    ArmResult out;
    out.spec = spec;
    ftd::RenderBridge bridge(args.L);
    if (args.backend == "cpu") bridge.force_cpu();
    out.actual_backend = bridge.backend_kind() == ftd::Backend::Kind::Gpu
        ? "gpu" : "cpu";
    configure(bridge, spec);
    initialize_square_source(bridge, spec);
    out.toggles_valid = toggle_contract(bridge, spec);
    out.history_required = args.backend == "cpu";
    out.history_enabled = out.history_required
        ? bridge.enable_history_journal(true) : false;

    bool valid = true;
    long long previous_occupancy = bridge.lattice().total_sites();
    for (int tick = 0; tick < kTransitions; ++tick) {
        TransitionRecord record;
        record.tick = tick;
        record.hazard = ftd::eft::observe_native_evaporation_hazard(
            bridge, tick, spec.n, spec.direction);
        bridge.tick();
        const auto after = ftd::eft::measure_native_reaction_mode(
            bridge, tick + 1, spec.n, spec.direction);
        record.source_after = after.source;
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
        valid = valid
            && record.actual_removed >= 0
            && record.hazard.max_site_probability <= 0.1 + 1e-15
            && record.hazard.min_site_probability >= 0.0
            && std::isfinite(record.hazard.source_hazard);
        out.transitions.push_back(record);
    }

    const auto& first = out.transitions.front();
    const auto& last = out.transitions.back();
    const bool initial_valid = first.hazard.occupancy
            == static_cast<long long>(bridge.lattice().total_sites())
        && std::abs(first.hazard.source) >= 0.3;
    const bool backend_valid = out.actual_backend == args.backend;
    if (spec.arm == "isolated") {
        for (const auto& record : out.transitions) {
            valid = valid
                && std::abs(record.hazard.mean_site_probability - 0.1) <= 1e-12
                && std::abs(record.hazard.source_hazard - 0.1) <= 1e-12
                && record.hazard.max_local_energy <= 1e-14;
        }
    } else if (is_locked(spec)) {
        for (const auto& record : out.transitions) {
            valid = valid && record.hazard.eligible_sites == 0
                && record.hazard.expected_removals == 0.0
                && record.actual_removed == 0;
        }
        valid = valid && std::abs(
            last.source_after - first.hazard.source) <= 1e-14;
    } else {
        valid = valid && first.hazard.max_local_energy > 0.0;
    }
    out.structural_valid = initial_valid && valid
        && (!out.history_required || out.history_enabled);
    out.execution_valid = backend_valid && out.toggles_valid
        && out.structural_valid;
    return out;
}

std::vector<ArmSpec> selected_matrix() {
    const std::array<std::pair<std::array<int, 3>, int>, 3> modes{{
        {{{1, 0, 0}}, 1},
        {{{1, 1, 0}}, 2},
        {{{1, 1, 1}}, 3},
    }};
    std::vector<ArmSpec> out;
    for (const auto& mode : modes) {
        for (int seed = 0; seed < 8; ++seed) {
            out.push_back({"isolated", seed, mode.first, mode.second});
            out.push_back({"coupled", seed, mode.first, mode.second});
        }
        out.push_back({"locked_control", 0, mode.first, mode.second});
    }
    return out;
}

void write_header(std::ofstream& out) {
    out << "backend_label,actual_backend,L,arm,seed,dx,dy,dz,n,tick,"
           "source_before_real,source_before_imag,expected_loss_real,"
           "expected_loss_imag,predicted_next_real,predicted_next_imag,"
           "source_after_real,source_after_imag,source_hazard,"
           "expected_removals,removal_variance,actual_removed,"
           "occupancy_before,occupancy_after,eligible_sites,"
           "mean_site_probability,min_site_probability,max_site_probability,"
           "mean_local_energy,max_local_energy,history_evaporation,"
           "history_other,history_required,history_enabled,toggles_valid,"
           "structural_valid,execution_valid\n";
}

void write_arm(std::ofstream& out, const Args& args, const ArmResult& arm) {
    for (const auto& record : arm.transitions) {
        const auto& h = record.hazard;
        out << args.backend_label << ',' << arm.actual_backend << ',' << args.L
            << ',' << arm.spec.arm << ',' << arm.spec.seed << ','
            << arm.spec.direction[0] << ',' << arm.spec.direction[1] << ','
            << arm.spec.direction[2] << ',' << arm.spec.n << ','
            << record.tick << ',' << h.source.real() << ',' << h.source.imag()
            << ',' << h.expected_loss_source.real() << ','
            << h.expected_loss_source.imag() << ','
            << h.predicted_next_source.real() << ','
            << h.predicted_next_source.imag() << ','
            << record.source_after.real() << ',' << record.source_after.imag()
            << ',' << h.source_hazard << ',' << h.expected_removals << ','
            << h.removal_variance << ',' << record.actual_removed << ','
            << h.occupancy << ',' << record.occupancy_after << ','
            << h.eligible_sites << ',' << h.mean_site_probability << ','
            << h.min_site_probability << ',' << h.max_site_probability << ','
            << h.mean_local_energy << ',' << h.max_local_energy << ','
            << record.history_evaporation << ',' << record.history_other << ','
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
    if (args.L != 32) {
        std::cerr << "locked volume is L=32\n";
        return 2;
    }
    if (args.backend != "cpu" && args.backend != "gpu") {
        std::cerr << "backend must be cpu or gpu\n";
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

    const auto matrix = selected_matrix();
    bool execution_valid = true;
    int rows = 0;
    for (std::size_t index = 0; index < matrix.size(); ++index) {
        const auto arm = run_arm(args, matrix[index]);
        write_arm(csv, args, arm);
        rows += static_cast<int>(arm.transitions.size());
        execution_valid = execution_valid && arm.execution_valid;
        if ((index + 1) % 17 == 0 || index + 1 == matrix.size()) {
            std::cout << "completed=" << (index + 1) << '/' << matrix.size()
                      << " mode=" << arm.spec.direction[0]
                      << arm.spec.direction[1] << arm.spec.direction[2]
                      << " n=" << arm.spec.n
                      << " valid=" << arm.execution_valid << '\n';
        }
    }
    std::cout << "FTD-0432 native dressed evaporation hazard"
              << " backend_label=" << args.backend_label
              << " L=" << args.L << " arms=" << matrix.size()
              << " rows=" << rows
              << " execution_valid=" << execution_valid
              << " output=" << args.output << '\n';
    return execution_valid ? 0 : 1;
}
