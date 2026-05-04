/**
 * Complete-physics-lattice test: all FTD physics toggles ON.
 *
 * Per user observation: emergent behavior comes from the SUM of all physics,
 * not from isolated toggle subsets. Prior tests used `+color+triad only`
 * which is a stripped-down config. This test enables every physics-relevant
 * toggle simultaneously to let the engine's full emergent dynamics run.
 *
 * Toggle config (FULL PHYSICS):
 *   Defaults ON:      wave, coupling, damping, genesis, gauss, forces,
 *                     gravity, poisson_coulomb, movement, lorentz,
 *                     selective_damping, dual_substrate, weak_transmutation
 *   Adding:           color_forces, strong_force, triad_binding,
 *                     pair_production, exchange_force, larmor_radiation,
 *                     latency_field, langevin (T=0.005, FTD-0107 baseline)
 *   NOT enabling:     emergent_forces (conflicts with poisson_coulomb),
 *                     exact_dual_gauss, confinement, strict_validation,
 *                     emergent_forces
 *
 * Test: pure +x, +y, +z flux injection at L=32, 64. Multi-seed.
 * Compare to +color+triad-only baseline.
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"

struct VoxData { int x, y, z; int8_t state; int8_t color; };

static std::vector<VoxData> get_manifested(const ftd::RenderBridge& rb) {
    const auto& vox = rb.voxels();
    const auto& lat = rb.lattice();
    const int64_t total = lat.total_sites();
    std::vector<VoxData> out;
    for (int64_t i = 0; i < total; ++i) {
        if (vox[i].state != 0) {
            auto c = lat.coord(static_cast<int>(i));
            out.push_back({c.x, c.y, c.z, vox[i].state, vox[i].color});
        }
    }
    return out;
}

// Configure the engine for complete physics
static bool setup_full_physics(ftd::RenderBridge& rb, bool enable_full) {
    if (enable_full) {
        // Keep all defaults ON; ADD the optional physics
        rb.toggles.color_forces      = true;
        rb.toggles.strong_force      = true;
        rb.toggles.triad_binding     = true;
        rb.toggles.pair_production   = true;
        rb.toggles.exchange_force    = true;
        // larmor_radiation conflicts with langevin (both modify wave_vel)
        // Keeping langevin (FTD-0107 canonical baseline thermal bath)
        rb.toggles.latency_field     = true;
        rb.toggles.langevin          = true;
        rb.toggles.langevin_T        = 0.005;
        rb.toggles.langevin_gamma    = 0.02;
    }
    // else: pure defaults
    std::string err;
    if (!rb.toggles.validate(&err)) {
        std::cerr << "[full-physics] TOGGLE INVALID: " << err << std::endl;
        return false;
    }
    return true;
}

int main(int argc, char** argv) {
    bool full_physics = true;
    if (argc > 1 && std::string(argv[1]) == "--defaults") full_physics = false;
    std::cerr << "[full-physics] Mode: " << (full_physics ? "FULL PHYSICS" : "DEFAULTS ONLY") << std::endl;

    std::vector<int> L_vals = {32, 64, 128};
    std::vector<int> seeds = {1, 2};
    std::vector<char> axes = {'x', 'y', 'z'};

    std::cout << "{\n  \"meta\": {\n    \"mode\": \""
              << (full_physics ? "full_physics" : "defaults") << "\",\n"
              << "    \"K_GENESIS\": " << ftd::K_GENESIS << "\n  },\n  \"runs\": [\n";

    bool first = true;
    for (int L : L_vals) {
        for (char axis : axes) {
            for (int seed : seeds) {
                std::cerr << "  L=" << L << " axis=" << axis << " seed=" << seed << " ..." << std::flush;

                ftd::RenderBridge rb(L);
                if (!setup_full_physics(rb, full_physics)) {
                    std::cerr << " SKIP" << std::endl;
                    continue;
                }
                rb.toggles.langevin_seed = static_cast<uint32_t>(seed);
                const int c = L / 2;
                const double A = 5.0 * ftd::K_GENESIS;
                double fx = 0.0, fy = 0.0, fz = 0.0;
                if (axis == 'x') fx = A;
                else if (axis == 'y') fy = A;
                else fz = A;
                rb.inject_flux(c, c, c, {fx, fy, fz});
                for (int t = 0; t < 200; ++t) rb.tick();

                auto coords = get_manifested(rb);
                int n_R = 0, n_G = 0, n_B = 0, n_none = 0;
                int n_matter = 0, n_anti = 0;
                for (const auto& v : coords) {
                    if (v.color == 1) ++n_R;
                    else if (v.color == 2) ++n_G;
                    else if (v.color == 3) ++n_B;
                    else ++n_none;
                    if (v.state > 0) ++n_matter;
                    else if (v.state < 0) ++n_anti;
                }
                std::cerr << " n=" << coords.size()
                          << " (R=" << n_R << ",G=" << n_G << ",B=" << n_B
                          << ",none=" << n_none << ",matter=" << n_matter
                          << ",anti=" << n_anti << ")" << std::endl;

                if (!first) std::cout << ",\n";
                std::cout << "    {\"L\":" << L << ",\"axis\":\"" << axis << "\",\"seed\":" << seed
                          << ",\"n_total\":" << coords.size()
                          << ",\"n_matter\":" << n_matter << ",\"n_antimatter\":" << n_anti
                          << ",\"color_R\":" << n_R << ",\"color_G\":" << n_G
                          << ",\"color_B\":" << n_B << ",\"color_none\":" << n_none
                          << ",\"coords\":[";
                for (size_t i = 0; i < coords.size(); ++i) {
                    if (i) std::cout << ",";
                    std::cout << "{\"x\":" << coords[i].x << ",\"y\":" << coords[i].y
                              << ",\"z\":" << coords[i].z
                              << ",\"s\":" << static_cast<int>(coords[i].state)
                              << ",\"c\":" << static_cast<int>(coords[i].color) << "}";
                }
                std::cout << "]}";
                first = false;
            }
        }
    }
    std::cout << "\n  ]\n}\n";
    std::cerr << "[full-physics] DONE" << std::endl;
    return 0;
}
