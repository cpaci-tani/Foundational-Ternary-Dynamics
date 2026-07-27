/**
 * FTD-0428 run-of-record campaign: integrated matched Maxwell/Gauss branch.
 */

#include "ftd/render_bridge.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

namespace {

constexpr double kGaussTolerance = 1e-9;
constexpr double kIdentityTolerance = 1e-10;

struct Args {
    int L = 32;
    std::string backend = "unknown";
    std::string output = "matched_maxwell_integration.csv";
};

Args parse_args(int argc, char** argv) {
    Args args;
    for (int i = 1; i < argc; ++i) {
        const std::string key = argv[i];
        if (key == "--L" && i + 1 < argc) args.L = std::stoi(argv[++i]);
        else if (key == "--backend-label" && i + 1 < argc)
            args.backend = argv[++i];
        else if (key == "--output" && i + 1 < argc)
            args.output = argv[++i];
    }
    return args;
}

void configure(ftd::RenderBridge& bridge, bool movement) {
    bridge.force_cpu();
    bridge.toggles.disable_all();
    bridge.toggles.movement = movement;
    bridge.toggles.matched_gauss_dynamics = true;
    bridge.toggles.strict_validation = true;
}

int periodic_distance(int a, int b, int L) {
    const int raw = std::abs(a - b);
    return std::min(raw, L - raw);
}

int support_radius(const ftd::eft::MatchedGaussDynamics& state,
                   int cx, int cy, int cz, double threshold = 1e-18) {
    int radius = 0;
    const auto& electric = state.electric();
    const auto& magnetic = state.magnetic_half();
    const int L = electric.L;
    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                const int i = electric.index(x, y, z);
                const double magnitude =
                    std::abs(electric.x[static_cast<std::size_t>(i)]) +
                    std::abs(electric.y[static_cast<std::size_t>(i)]) +
                    std::abs(electric.z[static_cast<std::size_t>(i)]) +
                    std::abs(magnetic.x[static_cast<std::size_t>(i)]) +
                    std::abs(magnetic.y[static_cast<std::size_t>(i)]) +
                    std::abs(magnetic.z[static_cast<std::size_t>(i)]);
                if (magnitude <= threshold) continue;
                radius = std::max(radius, std::max({
                    periodic_distance(x, cx, L),
                    periodic_distance(y, cy, L),
                    periodic_distance(z, cz, L)}));
            }
        }
    }
    return radius;
}

int find_state(const ftd::RenderBridge& bridge, int polarity) {
    const auto& voxels = bridge.voxels();
    for (std::size_t i = 0; i < voxels.size(); ++i)
        if (voxels[i].state == polarity) return static_cast<int>(i);
    return -1;
}

void write_header(std::ofstream& out) {
    out << "backend,L,arm,polarity,direction,radius,ticks,init_valid,"
           "init_iterations,solver_residual,gauss_max,curl_residual,"
           "surface_flux,surface_error,minimum_energy,string_energy,"
           "modified_energy_initial,energy_drift_max,current_l1,reaction_l1,"
           "stationary_current_max,voxel_sync_max,support_initial,"
           "support_tick12,support_final,support_excess,valid\n";
}

void write_row(std::ofstream& out, const Args& args,
               const std::string& arm, int polarity,
               const std::string& direction, int radius, int ticks,
               const ftd::eft::MatchedMinimumEnergyResult& init,
               double gauss_max, double surface_flux, double surface_error,
               double string_energy, double modified_energy_initial,
               double energy_drift_max, double current_l1, int reaction_l1,
               double stationary_current_max, double voxel_sync_max,
               int support_initial, int support_tick12, int support_final,
               int support_excess, bool valid) {
    out << args.backend << ',' << args.L << ',' << arm << ',' << polarity << ','
        << direction << ',' << radius << ',' << ticks << ','
        << (init.valid ? 1 : 0) << ',' << init.iterations << ','
        << init.solver_residual << ',' << gauss_max << ','
        << init.curl_adjoint_residual << ',' << surface_flux << ','
        << surface_error << ',' << init.electric_energy << ','
        << string_energy << ',' << modified_energy_initial << ','
        << energy_drift_max << ',' << current_l1 << ',' << reaction_l1 << ','
        << stationary_current_max << ',' << voxel_sync_max << ','
        << support_initial << ',' << support_tick12 << ',' << support_final
        << ',' << support_excess << ',' << (valid ? 1 : 0) << '\n';
}

bool run_static(const Args& args, std::ofstream& out) {
    ftd::RenderBridge bridge(args.L);
    configure(bridge, false);
    const std::array<int, 3> source{
        args.L / 4, args.L / 4, args.L / 4};
    const std::array<int, 3> sink{
        3 * args.L / 4 - 1, 3 * args.L / 4 - 1, 3 * args.L / 4 - 1};
    bridge.set_state(source[0], source[1], source[2], +1);
    bridge.set_state(sink[0], sink[1], sink[2], -1);
    const auto init = bridge.initialize_matched_gauss_dynamics();

    ftd::eft::MatchedFaceFlux string_field(args.L);
    const bool string_valid = ftd::eft::seed_dipole_path(
        string_field, bridge.lattice().index(source[0], source[1], source[2]),
        bridge.lattice().index(sink[0], sink[1], sink[2]), +1.0);
    const double string_energy = ftd::eft::quadratic_energy(string_field);
    const double initial_energy = bridge.matched_gauss_state().modified_energy(
        ftd::C_SPEED, bridge.dt());
    double drift = 0.0;
    double gauss = init.gauss_residual;
    double sync = bridge.matched_gauss_voxel_sync_residual();
    bool valid = init.valid && init.converged && init.iterations <= 12 * args.L
        && init.solver_residual <= 1e-12 &&
        init.gauss_residual <= kGaussTolerance &&
        init.curl_adjoint_residual <= kIdentityTolerance && string_valid &&
        init.electric_energy < string_energy;
    for (int tick = 0; tick < 32; ++tick) {
        bridge.tick();
        const auto& step = bridge.matched_gauss_state().last_step();
        valid = valid && step.valid;
        gauss = std::max(gauss, step.gauss_residual);
        drift = std::max(drift, std::abs(step.energy_after - initial_energy));
        sync = std::max(sync, bridge.matched_gauss_voxel_sync_residual());
    }
    valid = valid && gauss <= kGaussTolerance &&
        drift <= 1e-10 * std::max(1.0, std::abs(initial_energy)) &&
        sync <= 1e-12;

    for (int radius = 2; radius <= 5; ++radius) {
        const auto surface = ftd::eft::measure_face_cube_charge(
            bridge.matched_gauss_state().electric(), source[0], source[1],
            source[2], radius);
        const double surface_error = std::max(
            std::abs(surface.boundary_flux - 1.0),
            std::abs(surface.telescope_residual));
        const bool row_valid = valid && surface_error <= kGaussTolerance;
        write_row(out, args, "static_dressing", +1, "diagonal", radius, 32,
                  init, gauss, surface.boundary_flux, surface_error,
                  string_energy, initial_energy, drift, 0.0, 0, 0.0, sync,
                  0, 0, 0, 0, row_valid);
        valid = valid && row_valid;
    }
    return valid;
}

bool run_movement_arm(const Args& args, std::ofstream& out,
                      int polarity, int axis) {
    ftd::RenderBridge bridge(args.L);
    configure(bridge, true);
    std::array<int, 3> source{args.L / 2, args.L / 2, args.L / 2};
    std::array<int, 3> sink{3 * args.L / 4, 3 * args.L / 4,
                            3 * args.L / 4};
    source[axis] = args.L / 4;
    bridge.inject_particle(source[0], source[1], source[2],
                           static_cast<std::int8_t>(polarity), {});
    bridge.inject_particle(sink[0], sink[1], sink[2],
                           static_cast<std::int8_t>(-polarity), {});
    bridge.voxel_at(sink[0], sink[1], sink[2]).locked = true;
    auto& mobile = bridge.voxel_at(source[0], source[1], source[2]);
    const double speed = 0.99 * ftd::C_SPEED;
    if (axis == 0) {
        mobile.velocity.x = speed;
        mobile.remainder.x = 1.0 - speed;
    } else if (axis == 1) {
        mobile.velocity.y = speed;
        mobile.remainder.y = 1.0 - speed;
    } else {
        mobile.velocity.z = speed;
        mobile.remainder.z = 1.0 - speed;
    }
    const auto init = bridge.initialize_matched_gauss_dynamics();
    double gauss = init.gauss_residual;
    double current = 0.0;
    double stationary_current = 0.0;
    int reaction = 0;
    double sync = bridge.matched_gauss_voxel_sync_residual();
    bool valid = init.valid && init.iterations <= 12 * args.L;
    for (int tick = 1; tick <= 20; ++tick) {
        if (tick == 13) {
            const int mobile_index = find_state(bridge, polarity);
            if (mobile_index < 0) {
                valid = false;
            } else {
                auto& voxel = bridge.voxels()[static_cast<std::size_t>(mobile_index)];
                voxel.locked = true;
                voxel.velocity = {};
                voxel.remainder = {};
            }
        }
        bridge.tick();
        const auto& step = bridge.matched_gauss_state().last_step();
        valid = valid && step.valid;
        gauss = std::max(gauss, step.gauss_residual);
        current += step.transport.current_l1;
        if (tick > 12)
            stationary_current = std::max(
                stationary_current, step.transport.current_l1);
        reaction += step.transport.reaction_l1;
        sync = std::max(sync, bridge.matched_gauss_voxel_sync_residual());
    }
    const int final_mobile = find_state(bridge, polarity);
    ftd::eft::MatchedSurfaceCharge surface;
    double surface_error = std::numeric_limits<double>::infinity();
    if (final_mobile >= 0) {
        const auto coordinate = bridge.lattice().coord(final_mobile);
        surface = ftd::eft::measure_face_cube_charge(
            bridge.matched_gauss_state().electric(), coordinate.x,
            coordinate.y, coordinate.z, 2);
        surface_error = std::max(
            std::abs(surface.boundary_flux - polarity),
            std::abs(surface.telescope_residual));
    }
    valid = valid && current >= 5.0 && reaction == 0 &&
        stationary_current == 0.0 && surface_error <= kGaussTolerance &&
        gauss <= kGaussTolerance && sync <= 1e-12;
    const std::string direction = axis == 0 ? "+x" : axis == 1 ? "+y" : "+z";
    write_row(out, args, "movement", polarity, direction, 2, 20, init,
              gauss, surface.boundary_flux, surface_error, 0.0, 0.0, 0.0,
              current, reaction, stationary_current, sync,
              0, 0, 0, 0, valid);
    return valid;
}

bool run_wave(const Args& args, std::ofstream& out) {
    ftd::RenderBridge bridge(args.L);
    configure(bridge, false);
    const auto init = bridge.initialize_matched_gauss_dynamics();
    const int c = args.L / 2;
    const bool injected = bridge.inject_matched_transverse_edge_potential(
        c, c, c, 2, 1e-3);
    const int initial_support = support_radius(
        bridge.matched_gauss_state(), c, c, c);
    const double initial_energy = bridge.matched_gauss_state().modified_energy(
        ftd::C_SPEED, bridge.dt());
    double drift = 0.0;
    double gauss = 0.0;
    double sync = bridge.matched_gauss_voxel_sync_residual();
    int final_support = initial_support;
    int support_tick12 = initial_support;
    int support_excess = 0;
    bool valid = init.valid && injected && initial_energy > 0.0;
    for (int tick = 1; tick <= 32; ++tick) {
        bridge.tick();
        const auto& step = bridge.matched_gauss_state().last_step();
        valid = valid && step.valid;
        gauss = std::max(gauss, step.gauss_residual);
        drift = std::max(drift, std::abs(step.energy_after - initial_energy));
        sync = std::max(sync, bridge.matched_gauss_voxel_sync_residual());
        final_support = support_radius(bridge.matched_gauss_state(), c, c, c);
        if (tick == 12) support_tick12 = final_support;
        support_excess = std::max(
            support_excess, final_support - (initial_support + tick));
    }
    valid = valid && gauss <= kIdentityTolerance &&
        drift <= 1e-10 * std::max(1.0, std::abs(initial_energy)) &&
        sync <= 1e-12 && support_excess <= 0 &&
        support_tick12 >= initial_support + 3;
    write_row(out, args, "transverse_wave", 0, "edge-z", 0, 32, init,
              gauss, 0.0, 0.0, 0.0, initial_energy, drift, 0.0, 0, 0.0, sync,
              initial_support, support_tick12, final_support, support_excess,
              valid);
    return valid;
}

}  // namespace

int main(int argc, char** argv) {
    const Args args = parse_args(argc, argv);
    if (args.L < 16 || args.L % 2 != 0) {
        std::cerr << "L must be even and >=16\n";
        return 2;
    }
    std::ofstream out(args.output);
    if (!out) {
        std::cerr << "cannot open output: " << args.output << '\n';
        return 2;
    }
    out << std::setprecision(17);
    write_header(out);

    int failures = run_static(args, out) ? 0 : 1;
    for (int polarity : {+1, -1})
        for (int axis = 0; axis < 3; ++axis)
            if (!run_movement_arm(args, out, polarity, axis)) ++failures;
    if (!run_wave(args, out)) ++failures;

    std::cout << "FTD-0428 backend=" << args.backend << " L=" << args.L
              << " failures=" << failures << " output=" << args.output << '\n';
    return failures == 0 ? 0 : 1;
}
