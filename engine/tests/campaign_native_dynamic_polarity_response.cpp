/**
 * @file campaign_native_dynamic_polarity_response.cpp
 * @brief FTD-0429 native long-wavelength polarity-response campaign.
 *
 * The field starts identically zero.  Only the production wave and native
 * state-gradient coupling terms evolve it.  No Gauss solve or matched-field
 * extension is enabled.
 */

#include "ftd/eft/native_dynamic_polarity_response.h"
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
constexpr double kMinSourceMode = 1e-3;

struct Args {
    int L = 32;
    std::string backend = "cpu";
    std::string backend_label = "unspecified";
    std::string profile = "full";
    std::string output = "native_dynamic_polarity_response.csv";
};

struct ArmSpec {
    std::string name;
    std::array<int, 3> direction{};
    int base = 1;
    int duty = 2;
    int orientation = 1;
};

struct ModeResult {
    int n = 0;
    std::complex<double> source{};
    double omega = 0.0;
    int final_tick = 0;
    int sample_count = 0;
    ftd::eft::NativeResponseFit fit{};
    double exact_response = 0.0;
    double relative_error = std::numeric_limits<double>::infinity();
    bool valid = false;
};

struct ArmResult {
    ArmSpec spec;
    std::string actual_backend;
    long long initial_charge = 0;
    long long final_charge = 0;
    std::uint64_t initial_state_hash = 0;
    std::uint64_t final_state_hash = 0;
    bool forbidden_toggles_off = false;
    bool valid = false;
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

void configure_native_linear_sector(ftd::RenderBridge& bridge) {
    bridge.toggles.disable_all();
    bridge.toggles.wave_propagation = true;
    bridge.toggles.coupling = true;
    bridge.toggles.dual_substrate = false;
    bridge.toggles.strict_validation = true;
}

bool forbidden_toggles_are_off(const ftd::RenderBridge& bridge) {
    const auto& t = bridge.toggles;
    return t.wave_propagation && t.coupling && !t.dual_substrate
        && !t.gauss_projection && !t.matched_gauss_dynamics
        && !t.damping && !t.movement && !t.forces && !t.genesis
        && !t.evaporation && !t.pair_production && !t.weak_transmutation
        && !t.poisson_coulomb && !t.emergent_forces;
}

void prepare_source(ftd::RenderBridge& bridge, const ArmSpec& spec) {
    const int L = bridge.lattice().size();
    auto& voxels = bridge.voxels();
    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                const int index = bridge.lattice().index(x, y, z);
                int state = 0;
                const int lane = y + L * z;
                if ((lane % spec.duty) == 0) {
                    int m = spec.direction[0] * x
                        + spec.direction[1] * y
                        + spec.direction[2] * z;
                    m %= L;
                    if (m < 0) m += L;
                    const int phase_bin = (spec.base * m) % L;
                    state = phase_bin < L / 2
                        ? spec.orientation : -spec.orientation;
                }
                bridge.set_state(index, static_cast<std::int8_t>(state));
                auto& voxel = voxels[static_cast<std::size_t>(index)];
                voxel.flux = {};
                voxel.wave_vel = {};
                voxel.flux_L = {};
                voxel.flux_R = {};
                voxel.wave_vel_L = {};
                voxel.wave_vel_R = {};
                voxel.velocity = {};
                voxel.remainder = {};
            }
        }
    }
}

std::vector<int> measured_harmonics(int base) {
    return base == 1 ? std::vector<int>{1, 3} : std::vector<int>{2};
}

ArmResult run_arm(const Args& args, const ArmSpec& spec) {
    ArmResult out;
    out.spec = spec;
    ftd::RenderBridge bridge(args.L);
    if (args.backend == "cpu") bridge.force_cpu();
    out.actual_backend = bridge.backend_kind() == ftd::Backend::Kind::Gpu
        ? "gpu" : "cpu";
    configure_native_linear_sector(bridge);
    prepare_source(bridge, spec);

    out.initial_charge = bridge.charge_sum();
    out.initial_state_hash = state_hash(bridge);
    out.forbidden_toggles_off = forbidden_toggles_are_off(bridge);

    struct ModeWork {
        ModeResult result;
        std::vector<ftd::eft::NativeResponseSample> samples;
    };
    std::vector<ModeWork> work;
    double slowest_omega = std::numeric_limits<double>::infinity();
    for (int n : measured_harmonics(spec.base)) {
        ModeWork mode;
        mode.result.n = n;
        const auto initial = ftd::eft::measure_native_polarity_mode(
            bridge, n, spec.direction);
        mode.result.source = initial.source;
        mode.result.omega = ftd::eft::native_discrete_pole(initial.k);
        mode.result.exact_response =
            ftd::eft::native_exact_static_response(initial.k);
        mode.samples.push_back({0, initial.response});
        slowest_omega = std::min(slowest_omega, mode.result.omega);
        work.push_back(std::move(mode));
    }

    const int final_tick = static_cast<int>(std::ceil(4.0 * ftd::PI / slowest_omega));
    std::vector<int> sample_ticks;
    sample_ticks.reserve(kPhaseSamples);
    for (int sample = 1; sample <= kPhaseSamples; ++sample) {
        const int tick = std::max(1, static_cast<int>(std::llround(
            static_cast<double>(sample * final_tick) /
            static_cast<double>(kPhaseSamples))));
        if (sample_ticks.empty() || sample_ticks.back() != tick)
            sample_ticks.push_back(tick);
    }

    int current_tick = 0;
    for (int target_tick : sample_ticks) {
        while (current_tick < target_tick) {
            bridge.tick();
            ++current_tick;
        }
        for (auto& mode : work) {
            const auto projection = ftd::eft::measure_native_polarity_mode(
                bridge, mode.result.n, spec.direction);
            mode.samples.push_back({current_tick, projection.response});
        }
    }

    out.final_charge = bridge.charge_sum();
    out.final_state_hash = state_hash(bridge);
    const bool state_valid = out.initial_charge == 0 && out.final_charge == 0
        && out.initial_state_hash == out.final_state_hash;
    const bool backend_valid = args.backend == out.actual_backend;

    out.valid = state_valid && backend_valid && out.forbidden_toggles_off;
    for (auto& mode : work) {
        mode.result.final_tick = current_tick;
        mode.result.sample_count = static_cast<int>(mode.samples.size());
        mode.result.fit = ftd::eft::fit_native_response(
            mode.samples, mode.result.omega);
        mode.result.relative_error = std::abs(
            mode.result.fit.intercept.real() - mode.result.exact_response)
            / std::max(1e-30, std::abs(mode.result.exact_response));
        mode.result.valid = mode.result.fit.valid
            && std::abs(mode.result.source) >= kMinSourceMode
            && mode.result.fit.normalized_residual <= 1e-8
            && std::abs(mode.result.fit.intercept.imag())
                <= 1e-8 * std::max(1.0,
                                   std::abs(mode.result.fit.intercept.real()))
            && mode.result.relative_error <= 1e-7;
        out.valid = out.valid && mode.result.valid;
        out.modes.push_back(mode.result);
    }
    return out;
}

std::vector<ArmSpec> campaign_matrix() {
    const std::array<std::array<int, 3>, 3> directions{{
        {1, 0, 0}, {1, 1, 0}, {1, 1, 1}}};
    const std::array<const char*, 3> names{{"100", "110", "111"}};
    std::vector<ArmSpec> out;
    for (std::size_t direction = 0; direction < directions.size(); ++direction) {
        for (int base : {1, 2}) {
            for (int orientation : {1, -1}) {
                out.push_back({
                    std::string("primary_") + names[direction],
                    directions[direction], base, 2, orientation});
            }
        }
    }
    for (int base : {1, 2}) {
        for (int duty : {1, 4}) {
            out.push_back({
                "amplitude_100", {1, 0, 0}, base, duty, 1});
        }
    }
    return out;
}

std::vector<ArmSpec> selected_matrix(const std::string& profile) {
    const auto full = campaign_matrix();
    if (profile == "full") return full;
    std::vector<ArmSpec> infrared;
    for (const auto& arm : full) {
        if (arm.name.rfind("primary_", 0) == 0
            && arm.duty == 2 && arm.orientation == 1) {
            infrared.push_back(arm);
        }
    }
    return infrared;
}

void write_header(std::ofstream& out) {
    out << "backend_label,actual_backend,L,arm,dx,dy,dz,base,duty,orientation,"
           "n,source_real,source_imag,source_abs,omega,final_tick,samples,"
           "z_real,z_imag,z_exact,relative_error,fit_residual,initial_charge,"
           "final_charge,state_unchanged,forbidden_toggles_off,valid\n";
}

void write_arm(std::ofstream& out, const Args& args, const ArmResult& arm) {
    for (const auto& mode : arm.modes) {
        out << args.backend_label << ',' << arm.actual_backend << ',' << args.L
            << ',' << arm.spec.name << ',' << arm.spec.direction[0] << ','
            << arm.spec.direction[1] << ',' << arm.spec.direction[2] << ','
            << arm.spec.base << ',' << arm.spec.duty << ','
            << arm.spec.orientation << ',' << mode.n << ','
            << mode.source.real() << ',' << mode.source.imag() << ','
            << std::abs(mode.source) << ',' << mode.omega << ','
            << mode.final_tick << ',' << mode.sample_count << ','
            << mode.fit.intercept.real() << ',' << mode.fit.intercept.imag()
            << ',' << mode.exact_response << ',' << mode.relative_error << ','
            << mode.fit.normalized_residual << ',' << arm.initial_charge << ','
            << arm.final_charge << ','
            << (arm.initial_state_hash == arm.final_state_hash ? 1 : 0) << ','
            << (arm.forbidden_toggles_off ? 1 : 0) << ','
            << (arm.valid && mode.valid ? 1 : 0) << '\n';
    }
}

}  // namespace

int main(int argc, char** argv) {
    const Args args = parse_args(argc, argv);
    if ((args.L != 32 && args.L != 64) || (args.L % 4) != 0) {
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

    bool valid = true;
    int rows = 0;
    const auto matrix = selected_matrix(args.profile);
    for (const auto& spec : matrix) {
        const ArmResult arm = run_arm(args, spec);
        write_arm(csv, args, arm);
        rows += static_cast<int>(arm.modes.size());
        valid = valid && arm.valid;
        std::cout << spec.name << " d=" << spec.direction[0]
                  << spec.direction[1] << spec.direction[2]
                  << " base=" << spec.base << " duty=" << spec.duty
                  << " sign=" << spec.orientation
                  << " backend=" << arm.actual_backend
                  << " valid=" << arm.valid << '\n';
    }

    std::cout << "FTD-0429 native dynamical polarity response"
              << " backend_label=" << args.backend_label
              << " L=" << args.L << " profile=" << args.profile
              << " arms=" << matrix.size() << " rows=" << rows
              << " valid=" << valid << " output=" << args.output << '\n';
    return valid ? 0 : 1;
}
