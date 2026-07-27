/**
 * @file campaign_matched_gauss_transport.cpp
 * @brief FTD-0427 projection-free matched-current campaign.
 */

#include "ftd/constants.h"
#include "ftd/eft/dual_cell_continuity.h"
#include "ftd/eft/matched_gauss_transport.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <array>
#include <cmath>
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

constexpr int kMovingTicks = 12;
constexpr int kStationaryTicks = 8;
constexpr std::array<int, 3> kRadii{2, 3, 4};
constexpr double kTolerance = 1e-12;

struct Direction {
    const char* name;
    int axis;
    int sign;
};

constexpr std::array<Direction, 6> kDirections{{
    {"+x", 0, +1}, {"-x", 0, -1},
    {"+y", 1, +1}, {"-y", 1, -1},
    {"+z", 2, +1}, {"-z", 2, -1},
}};

struct Args {
    int L = 32;
    std::string backend_label = "windows_msvc_cpu";
    std::string output = "matched_gauss_transport.csv";
};

Args parse_args(int argc, char** argv) {
    Args out;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--L") == 0 && i + 1 < argc) {
            out.L = static_cast<int>(std::strtol(argv[++i], nullptr, 10));
        } else if (std::strcmp(argv[i], "--backend-label") == 0 && i + 1 < argc) {
            out.backend_label = argv[++i];
        } else if (std::strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
            out.output = argv[++i];
        } else {
            std::cerr << "unknown argument: " << argv[i] << '\n';
        }
    }
    return out;
}

std::vector<int> state_snapshot(const ftd::RenderBridge& rb) {
    std::vector<int> out(
        static_cast<std::size_t>(rb.lattice().total_sites()), 0);
    const auto& voxels = rb.voxels();
    for (std::size_t i = 0; i < out.size(); ++i)
        out[i] = static_cast<int>(voxels[i].state);
    return out;
}

int state_sum(const std::vector<int>& rho) {
    return std::accumulate(rho.begin(), rho.end(), 0);
}

int count_nonzero(const std::vector<int>& rho) {
    return static_cast<int>(std::count_if(
        rho.begin(), rho.end(), [](int value) { return value != 0; }));
}

int find_state(const std::vector<int>& rho, int value) {
    const auto it = std::find(rho.begin(), rho.end(), value);
    return it == rho.end() ? -1 : static_cast<int>(it - rho.begin());
}

struct ArmSummary {
    bool valid = true;
    int movement_events = 0;
    int reaction_l1 = 0;
    double max_transport_residual = 0.0;
    double max_curl_divergence = 0.0;
    double max_gauss_residual = 0.0;
    double max_telescope_residual = 0.0;
    double max_surface_error = 0.0;
    double max_plateau = 0.0;
    double min_curl_l1 = INFINITY;
    double max_stationary_current_l1 = 0.0;
};

void set_axis(ftd::Vec3& value, int axis, double component) {
    if (axis == 0) value.x = component;
    if (axis == 1) value.y = component;
    if (axis == 2) value.z = component;
}

std::array<int, 3> arm_coordinate(int L, const Direction& direction,
                                  bool source) {
    std::array<int, 3> c{L / 2, L / 2, L / 2};
    if (direction.sign > 0) {
        c[direction.axis] = source ? L / 4 : 3 * L / 4 - 1;
    } else {
        c[direction.axis] = source ? 3 * L / 4 : L / 4 + 1;
    }
    return c;
}

void prime_motion(ftd::RenderBridge& rb, const std::array<int, 3>& source,
                  const Direction& direction) {
    auto& mobile = rb.voxel_at(source[0], source[1], source[2]);
    const double component =
        static_cast<double>(direction.sign) * 0.99 * ftd::C_SPEED;
    set_axis(mobile.velocity, direction.axis, component);
    const double primed = component > 0.0 ? 1.0 - component
                                         : -1.0 - component;
    set_axis(mobile.remainder, direction.axis, primed);
}

void write_sample(std::ofstream& csv, const Args& args, int orientation,
                  const Direction& direction, const char* stage, int tick,
                  const ftd::eft::DualCellHistoryExtraction& extraction,
                  const ftd::eft::MatchedTransportUpdate& update,
                  double curl_l1, double curl_divergence,
                  double gauss_residual, int total_state,
                  const ftd::eft::MatchedSurfaceCharge& surface,
                  double plateau, bool valid) {
    csv << args.backend_label << ',' << args.L << ',' << orientation << ','
        << direction.name << ',' << stage << ',' << tick << ','
        << extraction.transported_events << ','
        << extraction.annihilation_pairs << ','
        << extraction.reaction_sites << ',' << update.reaction_l1 << ','
        << update.current_l1 << ',' << update.transport_residual << ','
        << curl_l1 << ',' << curl_divergence << ',' << gauss_residual << ','
        << total_state << ',' << surface.radius << ','
        << surface.boundary_flux << ',' << surface.divergence_sum << ','
        << surface.telescope_residual << ',' << plateau << ','
        << (valid ? 1 : 0) << '\n';
}

ArmSummary run_arm(const Args& args, int orientation,
                   const Direction& direction, std::ofstream& csv) {
    ArmSummary out;
    ftd::RenderBridge rb(args.L);
    rb.force_cpu();
    rb.seed_rng(0x0427u);
    rb.toggles.disable_all();
    rb.toggles.movement = true;
    rb.toggles.dual_substrate = false;

    const auto source = arm_coordinate(args.L, direction, true);
    const auto sink = arm_coordinate(args.L, direction, false);
    rb.inject_particle(source[0], source[1], source[2],
                       static_cast<std::int8_t>(orientation), {});
    rb.inject_particle(sink[0], sink[1], sink[2],
                       static_cast<std::int8_t>(-orientation), {});
    rb.voxel_at(sink[0], sink[1], sink[2]).locked = true;
    prime_motion(rb, source, direction);

    ftd::eft::MatchedFaceFlux field(args.L);
    const int source_index = rb.lattice().index(source[0], source[1], source[2]);
    const int sink_index = rb.lattice().index(sink[0], sink[1], sink[2]);
    out.valid = ftd::eft::seed_dipole_path(
        field, source_index, sink_index, static_cast<double>(orientation));
    const auto challenge =
        ftd::eft::make_transverse_challenge(args.L, 1e-3);
    const auto challenge_curl = ftd::eft::matched_curl(challenge);
    const double curl_divergence =
        ftd::eft::max_divergence(challenge_curl);
    out.max_curl_divergence = curl_divergence;

    auto before = state_snapshot(rb);
    out.valid = out.valid && !rb.toggles.gauss_projection &&
        state_sum(before) == 0 && count_nonzero(before) == 2 &&
        ftd::eft::max_gauss_residual(field, before) <= kTolerance;

    const int total_ticks = kMovingTicks + kStationaryTicks;
    for (int tick = 1; tick <= total_ticks; ++tick) {
        if (tick == kMovingTicks + 1) {
            const int mobile_index = find_state(before, orientation);
            if (mobile_index >= 0) {
                auto c = rb.lattice().coord(mobile_index);
                auto& mobile = rb.voxel_at(c.x, c.y, c.z);
                mobile.locked = true;
                mobile.velocity = {};
                mobile.remainder = {};
            } else {
                out.valid = false;
            }
        }

        rb.tick();
        const auto after = state_snapshot(rb);
        ftd::eft::DualCellContinuity history;
        const auto extraction =
            ftd::eft::extract_moore_history_from_snapshots(
                args.L, before, after, history);
        const auto update =
            ftd::eft::apply_conservative_current(field, history, kTolerance);
        const double curl_l1 =
            ftd::eft::apply_transverse_curl(field, challenge);
        const double gauss_residual =
            ftd::eft::max_gauss_residual(field, after);

        out.movement_events += extraction.transported_events;
        out.reaction_l1 += update.reaction_l1;
        out.max_transport_residual = std::max(
            out.max_transport_residual, update.transport_residual);
        out.max_curl_divergence = std::max(
            out.max_curl_divergence, curl_divergence);
        out.max_gauss_residual = std::max(
            out.max_gauss_residual, gauss_residual);
        out.min_curl_l1 = std::min(out.min_curl_l1, curl_l1);
        if (tick > kMovingTicks)
            out.max_stationary_current_l1 = std::max(
                out.max_stationary_current_l1, update.current_l1);

        const int mobile_index = find_state(after, orientation);
        const bool step_valid = extraction.valid && update.valid &&
            extraction.annihilation_pairs == 0 &&
            extraction.reaction_sites == 0 && update.reaction_l1 == 0 &&
            state_sum(after) == 0 && count_nonzero(after) == 2 &&
            mobile_index >= 0 && !rb.toggles.gauss_projection;
        out.valid = out.valid && step_valid;

        std::array<ftd::eft::MatchedSurfaceCharge, kRadii.size()> surfaces{};
        double mean = 0.0;
        if (mobile_index >= 0) {
            const auto c = rb.lattice().coord(mobile_index);
            for (std::size_t i = 0; i < kRadii.size(); ++i) {
                surfaces[i] = ftd::eft::measure_face_cube_charge(
                    field, c.x, c.y, c.z, kRadii[i]);
                mean += surfaces[i].boundary_flux;
            }
            mean /= static_cast<double>(surfaces.size());
        }
        double lo = surfaces.front().boundary_flux;
        double hi = lo;
        for (const auto& surface : surfaces) {
            lo = std::min(lo, surface.boundary_flux);
            hi = std::max(hi, surface.boundary_flux);
            out.max_telescope_residual = std::max(
                out.max_telescope_residual,
                std::abs(surface.telescope_residual));
            out.max_surface_error = std::max(
                out.max_surface_error,
                std::abs(surface.boundary_flux -
                         static_cast<double>(orientation)));
        }
        const double plateau = (hi - lo) / std::max(1.0, std::abs(mean));
        out.max_plateau = std::max(out.max_plateau, plateau);

        const char* stage = tick <= kMovingTicks ? "moving" : "stationary";
        for (const auto& surface : surfaces) {
            write_sample(csv, args, orientation, direction, stage, tick,
                         extraction, update, curl_l1, curl_divergence,
                         gauss_residual, state_sum(after), surface, plateau,
                         step_valid);
        }
        before = after;
    }

    out.valid = out.valid && out.movement_events >= 5 &&
        out.reaction_l1 == 0 &&
        out.max_transport_residual <= kTolerance &&
        out.max_curl_divergence <= kTolerance &&
        out.max_gauss_residual <= kTolerance &&
        out.max_telescope_residual <= kTolerance &&
        out.max_surface_error <= kTolerance &&
        out.max_plateau <= kTolerance &&
        out.min_curl_l1 > 0.0 &&
        out.max_stationary_current_l1 == 0.0;
    return out;
}

}  // namespace

int main(int argc, char** argv) {
    const Args args = parse_args(argc, argv);
    if ((args.L != 32 && args.L != 64) || args.output.empty()) {
        std::cerr << "L must be 32 or 64 and output must be nonempty\n";
        return 2;
    }

    const fs::path output(args.output);
    if (!output.parent_path().empty())
        fs::create_directories(output.parent_path());
    std::ofstream csv(output);
    if (!csv) {
        std::cerr << "cannot open output " << args.output << '\n';
        return 2;
    }
    csv << std::setprecision(17);
    csv << "backend,L,orientation,direction,stage,tick,transported_events,"
           "annihilation_pairs,reaction_sites,reaction_l1,current_l1,"
           "transport_residual,curl_l1,curl_divergence,gauss_residual,"
           "total_state,radius,boundary_flux,divergence_sum,"
           "telescope_residual,plateau,valid\n";

    bool all_pass = true;
    std::cout << std::setprecision(17);
    std::cout << "FTD-0427 projection-free matched Gauss transport\n";
    std::cout << "backend=" << args.backend_label << " L=" << args.L
              << " gauss_projection=off output=" << args.output << '\n';
    for (int orientation : {+1, -1}) {
        for (const auto& direction : kDirections) {
            const auto arm = run_arm(args, orientation, direction, csv);
            all_pass = all_pass && arm.valid;
            std::cout << "arm q=" << orientation
                      << " direction=" << direction.name
                      << " pass=" << arm.valid
                      << " moves=" << arm.movement_events
                      << " reaction_l1=" << arm.reaction_l1
                      << " transport=" << arm.max_transport_residual
                      << " divcurl=" << arm.max_curl_divergence
                      << " gauss=" << arm.max_gauss_residual
                      << " telescope=" << arm.max_telescope_residual
                      << " surface=" << arm.max_surface_error
                      << " plateau=" << arm.max_plateau
                      << " curl_l1_min=" << arm.min_curl_l1
                      << " stationary_current="
                      << arm.max_stationary_current_l1 << '\n';
        }
    }
    std::cout << "verdict="
              << (all_pass
                      ? "A_SELECTED_LOCAL_PROJECTION_FREE_TRANSPORT"
                      : "B_OR_C_MATCHED_TRANSPORT_GATE_FAIL")
              << '\n';
    return all_pass ? 0 : 1;
}
