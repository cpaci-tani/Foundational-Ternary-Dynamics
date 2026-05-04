/**
 * Toggle-bisection: which physics toggle drives which feature?
 *
 * Method: full physics minus one toggle at a time. Compare to baseline.
 * Run at L=32 with pure +x flux, single seed (deterministic anyway).
 *
 * Output: which toggle, when removed, changes the string length / color
 * content / matter:antimatter ratio.
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <functional>
#include <tuple>
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"

struct R {
    std::string config;
    int n_total, n_R, n_G, n_B, n_none, n_matter, n_anti;
    std::vector<std::tuple<int,int,int,int,int>> coords; // x,y,z,s,c
};

static R run_one(const std::string& config_name,
                 std::function<void(ftd::RenderBridge&)> setup) {
    const int L = 32;
    ftd::RenderBridge rb(L);
    setup(rb);
    rb.toggles.langevin_seed = 1;
    std::string err;
    if (!rb.toggles.validate(&err)) {
        std::cerr << "  " << config_name << ": INVALID (" << err << ")" << std::endl;
        R r; r.config = config_name + " [INVALID]"; r.n_total = -1;
        return r;
    }
    const int c = L / 2;
    const double A = 5.0 * ftd::K_GENESIS;
    rb.inject_flux(c, c, c, {A, 0.0, 0.0});
    for (int t = 0; t < 200; ++t) rb.tick();

    R r;
    r.config = config_name;
    r.n_total = r.n_R = r.n_G = r.n_B = r.n_none = r.n_matter = r.n_anti = 0;
    const auto& vox = rb.voxels();
    const auto& lat = rb.lattice();
    for (int64_t i = 0; i < lat.total_sites(); ++i) {
        if (vox[i].state == 0) continue;
        ++r.n_total;
        if (vox[i].color == 1) ++r.n_R;
        else if (vox[i].color == 2) ++r.n_G;
        else if (vox[i].color == 3) ++r.n_B;
        else ++r.n_none;
        if (vox[i].state > 0) ++r.n_matter; else ++r.n_anti;
        auto c3 = lat.coord(static_cast<int>(i));
        r.coords.push_back({c3.x, c3.y, c3.z, vox[i].state, vox[i].color});
    }
    return r;
}

// Full-physics setup: defaults + all optional physics (excluding larmor due to langevin conflict)
static void setup_full(ftd::RenderBridge& rb) {
    rb.toggles.color_forces      = true;
    rb.toggles.strong_force      = true;
    rb.toggles.triad_binding     = true;
    rb.toggles.pair_production   = true;
    rb.toggles.exchange_force    = true;
    rb.toggles.latency_field     = true;
    rb.toggles.langevin          = true;
    rb.toggles.langevin_T        = 0.005;
    rb.toggles.langevin_gamma    = 0.02;
}

int main() {
    std::cerr << "[bisection] Full-physics MINUS one toggle at a time, L=32, +x flux ..." << std::endl;

    std::vector<R> results;

    // Baseline: full physics
    results.push_back(run_one("FULL_PHYSICS", setup_full));

    // Engine defaults only
    results.push_back(run_one("DEFAULTS_ONLY", [](ftd::RenderBridge& rb){ /* nothing */ }));

    // Full minus one toggle
    auto minus_one = [&](const std::string& name, std::function<void(ftd::RenderBridge&)> mod) {
        return run_one("FULL_minus_" + name, [mod](ftd::RenderBridge& rb){
            setup_full(rb);
            mod(rb);
        });
    };
    results.push_back(minus_one("color_forces",   [](ftd::RenderBridge& rb){ rb.toggles.color_forces = false; rb.toggles.triad_binding = false; }));
    results.push_back(minus_one("strong_force",   [](ftd::RenderBridge& rb){ rb.toggles.strong_force = false; }));
    results.push_back(minus_one("triad_binding",  [](ftd::RenderBridge& rb){ rb.toggles.triad_binding = false; }));
    results.push_back(minus_one("pair_production",[](ftd::RenderBridge& rb){ rb.toggles.pair_production = false; }));
    results.push_back(minus_one("exchange_force", [](ftd::RenderBridge& rb){ rb.toggles.exchange_force = false; }));
    results.push_back(minus_one("latency_field",  [](ftd::RenderBridge& rb){ rb.toggles.latency_field = false; }));
    results.push_back(minus_one("langevin",       [](ftd::RenderBridge& rb){ rb.toggles.langevin = false; }));

    // Defaults + only one optional toggle (positive bisection)
    auto plus_one = [&](const std::string& name, std::function<void(ftd::RenderBridge&)> mod) {
        return run_one("DEFAULTS_plus_" + name, mod);
    };
    results.push_back(plus_one("color_forces",     [](ftd::RenderBridge& rb){ rb.toggles.color_forces = true; }));
    results.push_back(plus_one("color+triad",      [](ftd::RenderBridge& rb){ rb.toggles.color_forces = true; rb.toggles.triad_binding = true; }));
    results.push_back(plus_one("strong_force",     [](ftd::RenderBridge& rb){ rb.toggles.strong_force = true; }));
    results.push_back(plus_one("pair_production",  [](ftd::RenderBridge& rb){ rb.toggles.pair_production = true; }));
    results.push_back(plus_one("exchange_force",   [](ftd::RenderBridge& rb){ rb.toggles.exchange_force = true; }));

    // Output
    std::cout << "{\n  \"runs\": [\n";
    bool first = true;
    for (const auto& r : results) {
        if (!first) std::cout << ",\n";
        std::cout << "    {\"config\":\"" << r.config << "\""
                  << ",\"n_total\":" << r.n_total
                  << ",\"n_R\":" << r.n_R << ",\"n_G\":" << r.n_G << ",\"n_B\":" << r.n_B
                  << ",\"n_none\":" << r.n_none
                  << ",\"n_matter\":" << r.n_matter << ",\"n_antimatter\":" << r.n_anti
                  << ",\"coords\":[";
        for (size_t i = 0; i < r.coords.size(); ++i) {
            if (i) std::cout << ",";
            auto [x, y, z, s, c] = r.coords[i];
            std::cout << "{\"x\":" << x << ",\"y\":" << y << ",\"z\":" << z
                      << ",\"s\":" << s << ",\"c\":" << c << "}";
        }
        std::cout << "]}";

        std::cerr << "  " << std::left << std::setw(35) << r.config
                  << "  n=" << r.n_total
                  << "  R=" << r.n_R << " G=" << r.n_G << " B=" << r.n_B
                  << " (none=" << r.n_none << ")"
                  << "  matter=" << r.n_matter << " anti=" << r.n_anti << std::endl;
        first = false;
    }
    std::cout << "\n  ]\n}\n";
    std::cerr << "[bisection] DONE" << std::endl;
    return 0;
}
