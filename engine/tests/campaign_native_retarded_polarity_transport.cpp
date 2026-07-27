/**
 * @file campaign_native_retarded_polarity_transport.cpp
 * @brief FTD-0430 production-hop retarded polarity-response campaign.
 *
 * A sparse neutral pair moves exactly once through phase_movement.  A locked
 * stationary copy supplies the counterfactual field history.  Both Gauss
 * mechanisms remain disabled.
 */

#include "ftd/eft/native_retarded_polarity_response.h"
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
#include <limits>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

constexpr int kPhaseSamples = 16;
constexpr double kMinDeltaSource = 1e-8;

struct Args {
    int L = 48;
    std::string backend = "cpu";
    std::string backend_label = "unspecified";
    std::string profile = "full";
    std::string output = "native_retarded_polarity_transport.csv";
};

struct ModeResult {
    ftd::eft::NativeRetardedMode mode{};
    double omega = 0.0;
    double exact_response = 0.0;
    ftd::eft::NativeResponseFit fit{};
    double response_relative_error = std::numeric_limits<double>::infinity();
    double residue_ratio = std::numeric_limits<double>::infinity();
    double exact_residue_ratio = std::numeric_limits<double>::infinity();
    double residue_relative_error = std::numeric_limits<double>::infinity();
    int sample_count = 0;
    int final_tau = 0;
    bool pass = false;
};

struct ArmResult {
    int orientation = 1;
    std::string actual_backend;
    long long moving_initial_charge = 0;
    long long moving_final_charge = 0;
    long long stationary_initial_charge = 0;
    long long stationary_final_charge = 0;
    int movement_events = 0;
    int reaction_events = 0;
    bool journal_required = false;
    bool journal_enabled = false;
    bool exact_hop = false;
    bool states_frozen_after_hop = false;
    bool toggles_valid = false;
    bool backend_valid = false;
    double tau0_max_abs = std::numeric_limits<double>::infinity();
    double tau1_max_abs = 0.0;
    double max_outside_cone = std::numeric_limits<double>::infinity();
    int final_support_radius = -1;
    bool causal_pass = false;
    bool execution_valid = false;
    bool advance = false;
    std::vector<ModeResult> modes;
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

std::uint64_t state_hash(const ftd::RenderBridge& bridge) {
    constexpr std::uint64_t offset = 1469598103934665603ull;
    constexpr std::uint64_t prime = 1099511628211ull;
    std::uint64_t hash = offset;
    for (const auto& voxel : bridge.voxels()) {
        hash ^= static_cast<std::uint8_t>(voxel.state + 1);
        hash *= prime;
    }
    return hash;
}

void configure_native_transport_sector(ftd::RenderBridge& bridge) {
    bridge.toggles.disable_all();
    bridge.toggles.wave_propagation = true;
    bridge.toggles.coupling = true;
    bridge.toggles.movement = true;
    bridge.toggles.dual_substrate = false;
    bridge.toggles.strict_validation = true;
}

bool locked_toggles(const ftd::RenderBridge& bridge) {
    const auto& t = bridge.toggles;
    return t.wave_propagation && t.coupling && t.movement
        && !t.dual_substrate && !t.gauss_projection
        && !t.matched_gauss_dynamics && !t.damping && !t.forces
        && !t.genesis && !t.evaporation && !t.pair_production
        && !t.weak_transmutation && !t.poisson_coulomb
        && !t.emergent_forces && !t.langevin && !t.symplectic_leapfrog
        && !t.verlet_wave_integrator && !t.lorentz_period2_floquet
        && !t.lorentz_bcc_time_floquet;
}

struct PairSites {
    int a_old = -1;
    int a_new = -1;
    int b_old = -1;
    int b_new = -1;
};

PairSites pair_sites(const ftd::RenderBridge& bridge) {
    const int L = bridge.lattice().size();
    PairSites sites;
    sites.a_old = bridge.lattice().index(L / 4, L / 2, L / 2);
    sites.a_new = bridge.lattice().index(L / 4 + 1, L / 2, L / 2);
    sites.b_old = bridge.lattice().index(5 * L / 8, L / 2, L / 2);
    sites.b_new = bridge.lattice().index(5 * L / 8 + 1, L / 2, L / 2);
    return sites;
}

void initialize_pair(ftd::RenderBridge& bridge, int orientation, bool move) {
    const int L = bridge.lattice().size();
    const auto sites = pair_sites(bridge);
    bridge.set_state(sites.a_old, static_cast<std::int8_t>(orientation));
    bridge.set_state(sites.b_old, static_cast<std::int8_t>(-orientation));
    const double speed = 0.99 * ftd::C_SPEED;
    for (const int index : {sites.a_old, sites.b_old}) {
        auto& voxel = bridge.voxels()[static_cast<std::size_t>(index)];
        voxel.locked = !move;
        voxel.velocity = move ? ftd::Vec3{speed, 0.0, 0.0} : ftd::Vec3{};
        voxel.remainder = move ? ftd::Vec3{1.0 - speed, 0.0, 0.0}
                               : ftd::Vec3{};
        voxel.flux = {};
        voxel.wave_vel = {};
        voxel.flux_L = {};
        voxel.flux_R = {};
        voxel.wave_vel_L = {};
        voxel.wave_vel_R = {};
    }
    (void)L;
}

bool has_exact_pair(const ftd::RenderBridge& bridge,
                    int first, int second, int orientation) {
    int manifested = 0;
    bool exact = true;
    const auto& voxels = bridge.voxels();
    for (std::size_t index = 0; index < voxels.size(); ++index) {
        if (voxels[index].state != 0) ++manifested;
        const int expected = static_cast<int>(index) == first ? orientation
            : (static_cast<int>(index) == second ? -orientation : 0);
        exact = exact && static_cast<int>(voxels[index].state) == expected;
    }
    return exact && manifested == 2;
}

void lock_moved_pair(ftd::RenderBridge& bridge, const PairSites& sites) {
    for (const int index : {sites.a_new, sites.b_new}) {
        auto& voxel = bridge.voxels()[static_cast<std::size_t>(index)];
        voxel.locked = true;
        voxel.velocity = {};
        voxel.remainder = {};
    }
}

std::vector<int> sample_taus(double slowest_omega) {
    const int final_tau = static_cast<int>(
        std::ceil(4.0 * ftd::PI / slowest_omega));
    std::vector<int> taus{0, 1};
    for (int sample = 1; sample <= kPhaseSamples; ++sample) {
        taus.push_back(std::max(1, static_cast<int>(std::llround(
            static_cast<double>(sample * final_tau)
            / static_cast<double>(kPhaseSamples)))));
    }
    std::sort(taus.begin(), taus.end());
    taus.erase(std::unique(taus.begin(), taus.end()), taus.end());
    return taus;
}

ArmResult run_arm(const Args& args, int orientation) {
    ArmResult out;
    out.orientation = orientation;
    ftd::RenderBridge moving(args.L);
    ftd::RenderBridge stationary(args.L);
    if (args.backend == "cpu") {
        moving.force_cpu();
        stationary.force_cpu();
    }
    out.actual_backend = moving.backend_kind() == ftd::Backend::Kind::Gpu
        ? "gpu" : "cpu";
    const std::string stationary_backend =
        stationary.backend_kind() == ftd::Backend::Kind::Gpu ? "gpu" : "cpu";
    out.backend_valid = out.actual_backend == args.backend
        && stationary_backend == args.backend;
    configure_native_transport_sector(moving);
    configure_native_transport_sector(stationary);
    out.toggles_valid = locked_toggles(moving) && locked_toggles(stationary);

    initialize_pair(moving, orientation, true);
    initialize_pair(stationary, orientation, false);
    const PairSites sites = pair_sites(moving);
    const std::array<int, 4> changed{{
        sites.a_old, sites.a_new, sites.b_old, sites.b_new}};

    out.moving_initial_charge = moving.charge_sum();
    out.stationary_initial_charge = stationary.charge_sum();
    const std::uint64_t stationary_initial_hash = state_hash(stationary);
    out.journal_required = args.backend == "cpu";
    out.journal_enabled = out.journal_required
        ? moving.enable_history_journal(true) : false;

    stationary.tick();
    moving.tick();
    if (out.journal_required) {
        for (const auto& event : moving.history_events()) {
            if (event.kind == ftd::eft::HistoryEventKind::Movement)
                ++out.movement_events;
            else
                ++out.reaction_events;
        }
    }

    out.exact_hop = has_exact_pair(
        moving, sites.a_new, sites.b_new, orientation)
        && has_exact_pair(stationary, sites.a_old, sites.b_old, orientation);
    lock_moved_pair(moving, sites);
    const std::uint64_t moving_post_hop_hash = state_hash(moving);

    auto initial_modes = ftd::eft::measure_native_retarded_modes(
        moving, stationary);
    double slowest_omega = std::numeric_limits<double>::infinity();
    for (const auto& mode : initial_modes)
        slowest_omega = std::min(
            slowest_omega, ftd::eft::native_discrete_pole(mode.k));
    const auto taus = sample_taus(slowest_omega);

    struct ModeWork {
        ModeResult result;
        std::complex<double> locked_source{};
        std::vector<ftd::eft::NativeResponseSample> samples;
        bool source_stable = true;
    };
    std::vector<ModeWork> work(initial_modes.size());
    for (std::size_t index = 0; index < initial_modes.size(); ++index) {
        work[index].result.mode = initial_modes[index];
        work[index].locked_source = initial_modes[index].delta_source;
        work[index].result.omega =
            ftd::eft::native_discrete_pole(initial_modes[index].k);
        work[index].result.exact_response =
            ftd::eft::native_exact_static_response(initial_modes[index].k);
        work[index].samples.push_back({0, initial_modes[index].response});
    }

    auto causal = ftd::eft::measure_native_causal_support(
        moving, stationary, changed, 0);
    out.tau0_max_abs = causal.max_abs;
    out.max_outside_cone = causal.max_outside;
    bool causal_geometry_valid = causal.support_radius <= causal.allowed_radius;

    int current_tau = 0;
    for (std::size_t target_index = 1; target_index < taus.size(); ++target_index) {
        const int target_tau = taus[target_index];
        while (current_tau < target_tau) {
            stationary.tick();
            moving.tick();
            ++current_tau;
        }
        const auto modes = ftd::eft::measure_native_retarded_modes(
            moving, stationary);
        for (std::size_t index = 0; index < work.size(); ++index) {
            work[index].source_stable = work[index].source_stable
                && std::abs(modes[index].delta_source
                            - work[index].locked_source) <= 1e-12;
            work[index].samples.push_back({current_tau, modes[index].response});
            work[index].result.mode = modes[index];
        }
        causal = ftd::eft::measure_native_causal_support(
            moving, stationary, changed, current_tau);
        if (current_tau == 1) out.tau1_max_abs = causal.max_abs;
        out.max_outside_cone = std::max(
            out.max_outside_cone, causal.max_outside);
        out.final_support_radius = causal.support_radius;
        causal_geometry_valid = causal_geometry_valid
            && causal.support_radius <= causal.allowed_radius;
    }

    out.moving_final_charge = moving.charge_sum();
    out.stationary_final_charge = stationary.charge_sum();
    out.states_frozen_after_hop = state_hash(moving) == moving_post_hop_hash
        && state_hash(stationary) == stationary_initial_hash
        && has_exact_pair(moving, sites.a_new, sites.b_new, orientation)
        && has_exact_pair(stationary, sites.a_old, sites.b_old, orientation);
    out.causal_pass = out.tau0_max_abs <= 1e-11
        && out.tau1_max_abs > 1e-10
        && out.max_outside_cone <= 1e-11
        && out.final_support_radius > 2
        && causal_geometry_valid;

    const bool charge_valid = out.moving_initial_charge == 0
        && out.moving_final_charge == 0
        && out.stationary_initial_charge == 0
        && out.stationary_final_charge == 0;
    const bool journal_valid = !out.journal_required
        || (out.journal_enabled && out.movement_events == 2
            && out.reaction_events == 0);
    out.execution_valid = out.backend_valid && out.toggles_valid
        && out.exact_hop && out.states_frozen_after_hop
        && charge_valid && journal_valid && initial_modes.size() == 9;

    out.advance = out.execution_valid && out.causal_pass;
    for (auto& mode : work) {
        mode.result.sample_count = static_cast<int>(mode.samples.size());
        mode.result.final_tau = current_tau;
        mode.result.fit = ftd::eft::fit_native_response(
            mode.samples, mode.result.omega);
        mode.result.response_relative_error = std::abs(
            mode.result.fit.intercept.real() - mode.result.exact_response)
            / std::max(1e-30, std::abs(mode.result.exact_response));
        mode.result.residue_ratio =
            ftd::eft::native_step_residue_ratio(mode.result.fit);
        mode.result.exact_residue_ratio =
            ftd::eft::native_exact_step_residue_ratio(mode.result.omega);
        mode.result.residue_relative_error = std::abs(
            mode.result.residue_ratio - mode.result.exact_residue_ratio)
            / mode.result.exact_residue_ratio;
        mode.result.pass = mode.result.fit.valid && mode.source_stable
            && std::abs(mode.locked_source) >= kMinDeltaSource
            && mode.result.fit.normalized_residual <= 1e-7
            && std::abs(mode.result.fit.intercept.imag())
                <= 1e-7 * std::max(
                    1.0, std::abs(mode.result.fit.intercept.real()))
            && mode.result.response_relative_error <= 1e-6
            && mode.result.residue_relative_error <= 1e-5;
        out.advance = out.advance && mode.result.pass;
        out.modes.push_back(mode.result);
    }
    return out;
}

std::vector<int> selected_orientations(const std::string& profile) {
    return profile == "full" ? std::vector<int>{1, -1}
                             : std::vector<int>{1};
}

void write_header(std::ofstream& out) {
    out << "backend_label,actual_backend,L,profile,orientation,dx,dy,dz,n,"
           "delta_source_real,delta_source_imag,delta_source_abs,omega,"
           "final_tau,samples,z_real,z_imag,z_exact,z_relative_error,"
           "fit_residual,residue_ratio,residue_exact,residue_relative_error,"
           "tau0_max_abs,tau1_max_abs,max_outside_cone,final_support_radius,"
           "movement_events,reaction_events,exact_hop,states_frozen,"
           "toggles_valid,backend_valid,execution_valid,causal_pass,"
           "mode_pass,advance\n";
}

void write_arm(std::ofstream& out, const Args& args, const ArmResult& arm) {
    for (const auto& result : arm.modes) {
        const auto& mode = result.mode;
        out << args.backend_label << ',' << arm.actual_backend << ',' << args.L
            << ',' << args.profile << ',' << arm.orientation << ','
            << mode.direction[0] << ',' << mode.direction[1] << ','
            << mode.direction[2] << ',' << mode.n << ','
            << mode.delta_source.real() << ',' << mode.delta_source.imag() << ','
            << std::abs(mode.delta_source) << ',' << result.omega << ','
            << result.final_tau << ',' << result.sample_count << ','
            << result.fit.intercept.real() << ','
            << result.fit.intercept.imag() << ',' << result.exact_response << ','
            << result.response_relative_error << ','
            << result.fit.normalized_residual << ',' << result.residue_ratio << ','
            << result.exact_residue_ratio << ','
            << result.residue_relative_error << ',' << arm.tau0_max_abs << ','
            << arm.tau1_max_abs << ',' << arm.max_outside_cone << ','
            << arm.final_support_radius << ',' << arm.movement_events << ','
            << arm.reaction_events << ',' << (arm.exact_hop ? 1 : 0) << ','
            << (arm.states_frozen_after_hop ? 1 : 0) << ','
            << (arm.toggles_valid ? 1 : 0) << ','
            << (arm.backend_valid ? 1 : 0) << ','
            << (arm.execution_valid ? 1 : 0) << ','
            << (arm.causal_pass ? 1 : 0) << ','
            << (result.pass ? 1 : 0) << ',' << (arm.advance ? 1 : 0) << '\n';
    }
}

}  // namespace

int main(int argc, char** argv) {
    const Args args = parse_args(argc, argv);
    if ((args.L != 48 && args.L != 96) || (args.L % 8) != 0) {
        std::cerr << "FTD-0430 v2 requires L=48 or L=96\n";
        return 2;
    }
    if (args.backend != "cpu" && args.backend != "gpu") {
        std::cerr << "backend must be cpu or gpu\n";
        return 2;
    }
    if ((args.L == 48 && args.profile != "full")
        || (args.L == 96 && args.profile != "infrared")) {
        std::cerr << "locked v2 profiles are L=48/full and L=96/infrared\n";
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
    bool advance = true;
    int rows = 0;
    const auto orientations = selected_orientations(args.profile);
    for (int orientation : orientations) {
        const auto arm = run_arm(args, orientation);
        write_arm(csv, args, arm);
        execution_valid = execution_valid && arm.execution_valid;
        advance = advance && arm.advance;
        rows += static_cast<int>(arm.modes.size());
        std::cout << "orientation=" << orientation
                  << " backend=" << arm.actual_backend
                  << " execution_valid=" << arm.execution_valid
                  << " causal_pass=" << arm.causal_pass
                  << " advance=" << arm.advance << '\n';
    }

    std::cout << "FTD-0430 native retarded polarity transport"
              << " backend_label=" << args.backend_label
              << " L=" << args.L << " profile=" << args.profile
              << " arms=" << orientations.size() << " rows=" << rows
              << " execution_valid=" << execution_valid
              << " advance=" << advance << " output=" << args.output << '\n';
    return execution_valid ? 0 : 1;
}
