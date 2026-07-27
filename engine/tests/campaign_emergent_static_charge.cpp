/**
 * @file campaign_emergent_static_charge.cpp
 * @brief FTD-0426 polarity-sourced static-charge discriminator.
 *
 * Canonical protocol:
 *   docs/theory/10_eft_program/preregistrations/
 *     PREREG_EMERGENT_STATIC_CHARGE_v1.md
 *
 * This is a target-blind engine campaign.  It asks whether production
 * transport of one member of a neutral polarity pair creates equal/opposite,
 * radius-stable closed-surface flux around the separated bodies.  It does not
 * identify primitive s with empirical electric charge and does not use the
 * explicitly state-sourced Poisson-Coulomb force as evidence.
 */

#include "ftd/constants.h"
#include "ftd/eft/emergent_charge_surface.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

constexpr std::array<int, 4> kRadii{3, 4, 5, 6};
constexpr int kNeutralTicks = 128;
constexpr int kProjectedTicks = 256;
constexpr int kLiveTicks = 128;
constexpr int kSorIterations = 30;

struct Args {
    int L = 32;
    std::string backend = "cpu";
    std::string output = "emergent_static_charge.csv";
};

Args parse_args(int argc, char** argv) {
    Args args;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--L") == 0 && i + 1 < argc) {
            args.L = static_cast<int>(std::strtol(argv[++i], nullptr, 10));
        } else if (std::strcmp(argv[i], "--backend") == 0 && i + 1 < argc) {
            args.backend = argv[++i];
        } else if (std::strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
            args.output = argv[++i];
        } else {
            std::cerr << "unknown argument: " << argv[i] << '\n';
        }
    }
    return args;
}

struct StageSamples {
    std::string stage;
    std::array<ftd::eft::SurfaceChargeSample, kRadii.size()> body_a{};
    std::array<ftd::eft::SurfaceChargeSample, kRadii.size()> body_b{};
};

struct ArmResult {
    int orientation = 0;
    std::string backend;
    bool valid = true;
    int transport_ticks = 0;
    StageSamples neutral;
    StageSamples projected;
    StageSamples live;
};

struct Means {
    double a = 0.0;
    double b = 0.0;
};

double mean_boundary(
    const std::array<ftd::eft::SurfaceChargeSample, kRadii.size()>& samples) {
    double sum = 0.0;
    for (const auto& sample : samples) sum += sample.boundary_flux;
    return sum / static_cast<double>(samples.size());
}

Means means(const StageSamples& samples) {
    return {mean_boundary(samples.body_a), mean_boundary(samples.body_b)};
}

double plateau_ratio(
    const std::array<ftd::eft::SurfaceChargeSample, kRadii.size()>& samples) {
    double lo = samples.front().boundary_flux;
    double hi = samples.front().boundary_flux;
    for (const auto& sample : samples) {
        lo = std::min(lo, sample.boundary_flux);
        hi = std::max(hi, sample.boundary_flux);
    }
    return (hi - lo) / std::max(1e-15, std::abs(mean_boundary(samples)));
}

double max_abs_boundary(const StageSamples& samples) {
    double out = 0.0;
    for (const auto& sample : samples.body_a)
        out = std::max(out, std::abs(sample.boundary_flux));
    for (const auto& sample : samples.body_b)
        out = std::max(out, std::abs(sample.boundary_flux));
    return out;
}

double max_gauss_residual(const StageSamples& samples) {
    double out = 0.0;
    for (const auto& sample : samples.body_a)
        out = std::max(out, std::abs(sample.gauss_residual));
    for (const auto& sample : samples.body_b)
        out = std::max(out, std::abs(sample.gauss_residual));
    return out;
}

StageSamples sample_stage(const ftd::RenderBridge& rb,
                          const std::string& stage) {
    const int L = rb.lattice().size();
    const int ax = L / 4;
    const int bx = 3 * L / 4;
    const int mid = L / 2;
    StageSamples out;
    out.stage = stage;
    for (std::size_t i = 0; i < kRadii.size(); ++i) {
        out.body_a[i] = ftd::eft::measure_central_cube_charge(
            rb, ax, mid, mid, kRadii[i]);
        out.body_b[i] = ftd::eft::measure_central_cube_charge(
            rb, bx, mid, mid, kRadii[i]);
    }
    return out;
}

bool telescope_valid(const StageSamples& samples) {
    for (const auto& sample : samples.body_a)
        if (!ftd::eft::central_cube_telescope_closes(sample)) return false;
    for (const auto& sample : samples.body_b)
        if (!ftd::eft::central_cube_telescope_closes(sample)) return false;
    return true;
}

void configure_gauss_only(ftd::RenderBridge& rb) {
    rb.toggles.disable_all();
    rb.toggles.gauss_projection = true;
    rb.toggles.dual_substrate = false;
    rb.toggles.coulomb_charge_coupling = 1.0;
}

void configure_transport(ftd::RenderBridge& rb) {
    configure_gauss_only(rb);
    rb.toggles.movement = true;
}

void configure_live(ftd::RenderBridge& rb) {
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.coupling = true;
    rb.toggles.damping = true;
    rb.toggles.selective_damping = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.dual_substrate = false;
    rb.toggles.coulomb_charge_coupling = 1.0;
}

void prime_positive_x_hop(ftd::RenderBridge& rb, int x, int y, int z) {
    auto& mobile = rb.voxel_at(x, y, z);
    const double speed = 0.99 * ftd::C_SPEED;
    mobile.velocity = {speed, 0.0, 0.0};
    mobile.remainder = {1.0 - speed, 0.0, 0.0};
}

ArmResult run_arm(const Args& args, int orientation) {
    ArmResult out;
    out.orientation = orientation;

    ftd::RenderBridge rb(args.L);
    if (args.backend == "cpu") rb.force_cpu();
    out.backend = rb.backend_kind() == ftd::Backend::Kind::Gpu ? "gpu" : "cpu";
    rb.seed_rng(0x0426u);
    rb.set_sor_iterations(kSorIterations);
    configure_gauss_only(rb);

    const int ax = args.L / 4;
    const int bx = 3 * args.L / 4;
    const int mid = args.L / 2;
    const int mobile_y = mid + 2;

    // Two neutral composites.  Only A's -q member is mobile.
    rb.inject_particle(ax, mid, mid, static_cast<int8_t>(orientation), {});
    rb.inject_particle(ax, mobile_y, mid, static_cast<int8_t>(-orientation), {});
    rb.inject_particle(bx, mid, mid, static_cast<int8_t>(orientation), {});
    rb.inject_particle(bx, mid - 2, mid, static_cast<int8_t>(-orientation), {});
    rb.voxel_at(ax, mid, mid).locked = true;
    rb.voxel_at(bx, mid, mid).locked = true;
    rb.voxel_at(bx, mid - 2, mid).locked = true;

    rb.run(kNeutralTicks);
    out.neutral = sample_stage(rb, "neutral");

    configure_transport(rb);
    prime_positive_x_hop(rb, ax, mobile_y, mid);
    bool arrived = false;
    for (int tick = 1; tick <= 4 * args.L; ++tick) {
        rb.tick();
        if (rb.state_at(bx, mobile_y, mid) == -orientation) {
            auto& mobile = rb.voxel_at(bx, mobile_y, mid);
            mobile.locked = true;
            mobile.velocity = {};
            mobile.remainder = {};
            out.transport_ticks = tick;
            arrived = true;
            break;
        }
    }

    out.valid = arrived
        && rb.charge_sum() == 0
        && rb.state_at(ax, mid, mid) == orientation
        && rb.state_at(ax, mobile_y, mid) == 0
        && rb.state_at(bx, mid, mid) == orientation
        && rb.state_at(bx, mid - 2, mid) == -orientation
        && rb.state_at(bx, mobile_y, mid) == -orientation;

    configure_gauss_only(rb);
    rb.run(kProjectedTicks);
    out.projected = sample_stage(rb, "projected");

    configure_live(rb);
    rb.run(kLiveTicks);
    out.live = sample_stage(rb, "live");

    out.valid = out.valid
        && telescope_valid(out.neutral)
        && telescope_valid(out.projected)
        && telescope_valid(out.live);
    return out;
}

void write_samples(std::ofstream& csv, const std::string& backend, int L,
                   const ArmResult& arm, const StageSamples& stage) {
    const auto write_body = [&](const char* body,
                                const auto& samples) {
        for (const auto& sample : samples) {
            csv << backend << ',' << L << ',' << arm.orientation << ','
                << stage.stage << ',' << body << ',' << sample.radius << ','
                << sample.enclosed_sites << ',' << sample.enclosed_polarity << ','
                << sample.mean_polarity << ',' << sample.boundary_flux << ','
                << sample.divergence_sum << ',' << sample.gauss_target << ','
                << sample.telescope_residual << ',' << sample.gauss_residual << ','
                << arm.transport_ticks << ',' << (arm.valid ? 1 : 0) << '\n';
        }
    };
    write_body("A", stage.body_a);
    write_body("B", stage.body_b);
}

bool readout_gate(const ArmResult& arm) {
    const auto projected = means(arm.projected);
    const double q = static_cast<double>(arm.orientation);
    return max_abs_boundary(arm.neutral) <= 0.10
        && projected.a * q > 0.0
        && projected.b * q < 0.0
        && std::min(std::abs(projected.a), std::abs(projected.b)) >= 0.50
        && std::abs(projected.a + projected.b) <= 0.10
        && plateau_ratio(arm.projected.body_a) <= 0.15
        && plateau_ratio(arm.projected.body_b) <= 0.15
        && max_gauss_residual(arm.projected) <= 0.15;
}

bool live_gate(const ArmResult& arm) {
    const auto projected = means(arm.projected);
    const auto live = means(arm.live);
    const double q = static_cast<double>(arm.orientation);
    return live.a * q > 0.0
        && live.b * q < 0.0
        && std::min(std::abs(live.a), std::abs(live.b)) >= 0.50
        && std::abs(live.a + live.b) <= 0.10
        && plateau_ratio(arm.live.body_a) <= 0.15
        && plateau_ratio(arm.live.body_b) <= 0.15
        && max_gauss_residual(arm.live) <= 0.15
        && std::abs(live.a - projected.a) <= 0.10
        && std::abs(live.b - projected.b) <= 0.10;
}

bool mirror_gate(const ArmResult& positive, const ArmResult& negative,
                 bool use_live) {
    const auto p = means(use_live ? positive.live : positive.projected);
    const auto n = means(use_live ? negative.live : negative.projected);
    return std::abs(p.a + n.a) <= 0.10 && std::abs(p.b + n.b) <= 0.10;
}

void print_arm(const ArmResult& arm) {
    const auto neutral = means(arm.neutral);
    const auto projected = means(arm.projected);
    const auto live = means(arm.live);
    std::cout << "arm q=" << arm.orientation
              << " valid=" << arm.valid
              << " transport_ticks=" << arm.transport_ticks
              << " neutral=(" << neutral.a << ',' << neutral.b << ')'
              << " projected=(" << projected.a << ',' << projected.b << ')'
              << " live=(" << live.a << ',' << live.b << ')'
              << " projected_plateau=(" << plateau_ratio(arm.projected.body_a)
              << ',' << plateau_ratio(arm.projected.body_b) << ')'
              << " live_plateau=(" << plateau_ratio(arm.live.body_a)
              << ',' << plateau_ratio(arm.live.body_b) << ')'
              << " projected_gauss_max=" << max_gauss_residual(arm.projected)
              << " live_gauss_max=" << max_gauss_residual(arm.live) << '\n';
}

}  // namespace

int main(int argc, char** argv) {
    const Args args = parse_args(argc, argv);
    if (args.L < 32 || (args.L % 4) != 0) {
        std::cerr << "L must be a multiple of four and at least 32\n";
        return 2;
    }
    if (args.backend != "cpu" && args.backend != "gpu") {
        std::cerr << "backend must be cpu or gpu\n";
        return 2;
    }

    const ArmResult positive = run_arm(args, +1);
    const ArmResult negative = run_arm(args, -1);

    if (positive.backend != negative.backend) {
        std::cerr << "mirror arms selected different execution backends\n";
        return 2;
    }
    const std::string actual_backend = positive.backend;
    const fs::path output_path(args.output);
    if (!output_path.parent_path().empty())
        fs::create_directories(output_path.parent_path());
    std::ofstream csv(output_path);
    if (!csv) {
        std::cerr << "could not open output: " << args.output << '\n';
        return 2;
    }
    csv << std::setprecision(17);
    csv << "backend,L,orientation,stage,body,radius,enclosed_sites,"
           "enclosed_polarity,mean_polarity,boundary_flux,divergence_sum,"
           "gauss_target,telescope_residual,gauss_residual,transport_ticks,valid\n";
    for (const auto* arm : {&positive, &negative}) {
        write_samples(csv, actual_backend, args.L, *arm, arm->neutral);
        write_samples(csv, actual_backend, args.L, *arm, arm->projected);
        write_samples(csv, actual_backend, args.L, *arm, arm->live);
    }

    std::cout << std::setprecision(12);
    std::cout << "FTD-0426 polarity-sourced static-charge campaign\n";
    std::cout << "requested_backend=" << args.backend
              << " actual_backend=" << actual_backend
              << " L=" << args.L << " output=" << args.output << '\n';
    print_arm(positive);
    print_arm(negative);

    const bool valid = positive.valid && negative.valid;
    const bool readout = valid && readout_gate(positive) && readout_gate(negative)
        && mirror_gate(positive, negative, false);
    const bool autonomous = readout && live_gate(positive) && live_gate(negative)
        && mirror_gate(positive, negative, true);

    const char* verdict = !valid ? "D_INVALID_CAMPAIGN"
        : autonomous ? "A_RESTRICTED_LOW_ENERGY_EMERGENT_CHARGE"
        : readout ? "B_SELECTED_GAUSS_CONSTRAINT_REALIZATION"
        : "C_CLOSED_NEGATIVE_STATIC_READOUT";
    std::cout << "readout_gate=" << readout
              << " autonomous_dressing_gate=" << autonomous
              << " verdict=" << verdict << '\n';

    // A scientific negative is a valid campaign result.  Only a broken
    // transport/observer contract fails the executable.
    return valid ? 0 : 1;
}
