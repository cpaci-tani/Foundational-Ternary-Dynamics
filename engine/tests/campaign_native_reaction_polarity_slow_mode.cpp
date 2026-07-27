/**
 * @file campaign_native_reaction_polarity_slow_mode.cpp
 * @brief FTD-0431 reaction-aware polarity decay campaign.
 */

#include "ftd/eft/native_reaction_polarity_slow_mode.h"
#include "ftd/render_bridge.h"

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

constexpr int kFinalTick = 16;

struct Args {
    int L = 32;
    std::string backend = "cpu";
    std::string backend_label = "unspecified";
    std::string profile = "full";
    std::string output = "native_reaction_polarity_slow_mode.csv";
};

struct ArmSpec {
    std::string arm;
    int seed = 0;
    std::array<int, 3> direction{};
    int n = 1;
};

struct SampleRecord {
    ftd::eft::NativeReactionModeMeasurement measurement{};
    long long removed_since_last = 0;
    long long cumulative_removed = 0;
    long long history_evaporation = 0;
    long long history_other = 0;
};

struct ArmResult {
    ArmSpec spec;
    std::string actual_backend;
    bool history_required = false;
    bool history_enabled = false;
    bool toggles_valid = false;
    bool event_valid = false;
    bool source_valid = false;
    bool field_valid = false;
    bool execution_valid = false;
    std::vector<SampleRecord> samples;
};

Args parse_args(int argc, char** argv) {
    Args args;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--L") == 0 && i + 1 < argc) {
            args.L = static_cast<int>(std::strtol(argv[++i], nullptr, 10));
        } else if (std::strcmp(argv[i], "--backend") == 0 && i + 1 < argc) {
            args.backend = argv[++i];
        } else if (std::strcmp(argv[i], "--backend-label") == 0 && i + 1 < argc) {
            args.backend_label = argv[++i];
        } else if (std::strcmp(argv[i], "--profile") == 0 && i + 1 < argc) {
            args.profile = argv[++i];
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

bool locked_toggle_contract(const ftd::RenderBridge& bridge,
                            const ArmSpec& spec) {
    const auto& t = bridge.toggles;
    return t.evaporation && !t.genesis
        && t.wave_propagation == is_coupled(spec)
        && t.coupling == is_coupled(spec)
        && !t.dual_substrate && !t.gauss_projection
        && !t.matched_gauss_dynamics && !t.damping && !t.movement
        && !t.forces && !t.pair_production && !t.weak_transmutation
        && !t.poisson_coulomb && !t.emergent_forces && !t.langevin
        && !t.symplectic_leapfrog && !t.verlet_wave_integrator
        && !t.lorentz_period2_floquet && !t.lorentz_bcc_time_floquet;
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
    out.toggles_valid = locked_toggle_contract(bridge, spec);
    out.history_required = args.backend == "cpu";
    out.history_enabled = out.history_required
        ? bridge.enable_history_journal(true) : false;

    SampleRecord initial;
    initial.measurement = ftd::eft::measure_native_reaction_mode(
        bridge, 0, spec.n, spec.direction);
    out.samples.push_back(initial);

    bool monotone = true;
    bool history_valid = true;
    long long cumulative_removed = 0;
    for (int tick = 1; tick <= kFinalTick; ++tick) {
        const long long before = out.samples.back().measurement.occupancy;
        bridge.tick();
        SampleRecord sample;
        sample.measurement = ftd::eft::measure_native_reaction_mode(
            bridge, tick, spec.n, spec.direction);
        sample.removed_since_last = before - sample.measurement.occupancy;
        monotone = monotone && sample.removed_since_last >= 0;
        cumulative_removed += sample.removed_since_last;
        sample.cumulative_removed = cumulative_removed;
        if (out.history_required) {
            for (const auto& event : bridge.history_events()) {
                if (event.kind == ftd::eft::HistoryEventKind::Evaporation)
                    ++sample.history_evaporation;
                else
                    ++sample.history_other;
            }
            history_valid = history_valid
                && sample.history_evaporation == sample.removed_since_last
                && sample.history_other == 0;
        }
        out.samples.push_back(sample);
    }

    const auto& first = out.samples.front().measurement;
    const auto& final = out.samples.back().measurement;
    const bool backend_valid = out.actual_backend == args.backend;
    const bool initial_valid = first.signed_state == 0
        && std::abs(first.source) >= 0.3
        && first.occupancy == bridge.lattice().total_sites();
    const bool activation_valid = is_locked(spec)
        ? cumulative_removed == 0
        : (out.samples[1].removed_since_last > 0 && cumulative_removed > 0);
    const bool locked_source_valid = !is_locked(spec)
        || std::abs(final.source - first.source)
            <= 1e-14 * std::max(1.0, std::abs(first.source));
    out.source_valid = initial_valid && monotone && activation_valid
        && locked_source_valid;

    double max_divergence = 0.0;
    double early_divergence = 0.0;
    for (const auto& sample : out.samples) {
        max_divergence = std::max(
            max_divergence, std::abs(sample.measurement.divergence));
        if (sample.measurement.tick <= 2)
            early_divergence = std::max(
                early_divergence, std::abs(sample.measurement.divergence));
    }
    out.field_valid = spec.arm == "isolated"
        ? max_divergence <= 1e-14
        : early_divergence > 1e-8;
    out.event_valid = history_valid
        && (!out.history_required || out.history_enabled);
    out.execution_valid = backend_valid && out.toggles_valid
        && out.event_valid && out.source_valid && out.field_valid;
    return out;
}

std::vector<ArmSpec> selected_matrix(const std::string& profile) {
    const std::array<std::array<int, 3>, 3> directions{{
        {1, 0, 0}, {1, 1, 0}, {1, 1, 1}}};
    std::vector<ArmSpec> out;
    for (const auto& direction : directions) {
        for (int n : {1, 2, 3}) {
            if (profile == "full") {
                for (int seed = 0; seed < 8; ++seed) {
                    out.push_back({"isolated", seed, direction, n});
                    out.push_back({"coupled", seed, direction, n});
                }
                out.push_back({"locked_control", 0, direction, n});
            } else {
                for (int seed = 0; seed < 8; ++seed)
                    out.push_back({"coupled", seed, direction, n});
            }
        }
    }
    return out;
}

void write_header(std::ofstream& out) {
    out << "backend_label,actual_backend,L,profile,arm,seed,dx,dy,dz,n,tick,"
           "source_real,source_imag,source_abs,div_real,div_imag,div_abs,"
           "occupancy,global_charge,removed_since_last,cumulative_removed,"
           "history_evaporation,history_other,history_required,history_enabled,"
           "toggles_valid,event_valid,source_valid,field_valid,execution_valid\n";
}

void write_arm(std::ofstream& out, const Args& args, const ArmResult& arm) {
    for (const auto& sample : arm.samples) {
        const auto& measurement = sample.measurement;
        out << args.backend_label << ',' << arm.actual_backend << ',' << args.L
            << ',' << args.profile << ',' << arm.spec.arm << ','
            << arm.spec.seed << ',' << arm.spec.direction[0] << ','
            << arm.spec.direction[1] << ',' << arm.spec.direction[2] << ','
            << arm.spec.n << ',' << measurement.tick << ','
            << measurement.source.real() << ',' << measurement.source.imag()
            << ',' << std::abs(measurement.source) << ','
            << measurement.divergence.real() << ','
            << measurement.divergence.imag() << ','
            << std::abs(measurement.divergence) << ','
            << measurement.occupancy << ',' << measurement.signed_state << ','
            << sample.removed_since_last << ',' << sample.cumulative_removed
            << ',' << sample.history_evaporation << ',' << sample.history_other
            << ',' << (arm.history_required ? 1 : 0) << ','
            << (arm.history_enabled ? 1 : 0) << ','
            << (arm.toggles_valid ? 1 : 0) << ','
            << (arm.event_valid ? 1 : 0) << ','
            << (arm.source_valid ? 1 : 0) << ','
            << (arm.field_valid ? 1 : 0) << ','
            << (arm.execution_valid ? 1 : 0) << '\n';
    }
}

}  // namespace

int main(int argc, char** argv) {
    const Args args = parse_args(argc, argv);
    if (args.L != 32 && args.L != 64) {
        std::cerr << "L must be 32 or 64\n";
        return 2;
    }
    if (args.backend != "cpu" && args.backend != "gpu") {
        std::cerr << "backend must be cpu or gpu\n";
        return 2;
    }
    if ((args.L == 32 && args.profile != "full")
        || (args.L == 64 && args.profile != "infrared")) {
        std::cerr << "locked profiles are L=32/full and L=64/infrared\n";
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
    const auto matrix = selected_matrix(args.profile);
    for (std::size_t index = 0; index < matrix.size(); ++index) {
        const auto arm = run_arm(args, matrix[index]);
        write_arm(csv, args, arm);
        rows += static_cast<int>(arm.samples.size());
        execution_valid = execution_valid && arm.execution_valid;
        if ((index + 1) % 18 == 0 || index + 1 == matrix.size()) {
            std::cout << "completed=" << (index + 1) << '/' << matrix.size()
                      << " last_arm=" << arm.spec.arm
                      << " seed=" << arm.spec.seed
                      << " d=" << arm.spec.direction[0]
                      << arm.spec.direction[1] << arm.spec.direction[2]
                      << " n=" << arm.spec.n
                      << " valid=" << arm.execution_valid << '\n';
        }
    }

    std::cout << "FTD-0431 native reaction polarity slow mode"
              << " backend_label=" << args.backend_label
              << " L=" << args.L << " profile=" << args.profile
              << " arms=" << matrix.size() << " rows=" << rows
              << " execution_valid=" << execution_valid
              << " output=" << args.output << '\n';
    return execution_valid ? 0 : 1;
}
