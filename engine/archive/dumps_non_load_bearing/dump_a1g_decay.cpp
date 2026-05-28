/**
 * Diagnostic: dump A_{1g} fraction per tick for sub-genesis δ_center IC.
 *
 * If Bridge-I holds analytically, the deterministic wave equation should
 * preserve A_{1g} purity exactly. Yet test_a1g_bridge_i_empirical reports
 * f → 0.149 ≈ 4/27 (random equipartition limit) by tick ~1000.
 *
 * Question: at which tick does f start dropping, and which toggles are
 * responsible?
 *
 * Configurations tested:
 *   A. Bare-bones wave only          (everything off except wave)
 *   B. Wave + damping
 *   C. Wave + gauss_project
 *   D. Wave + dual_substrate
 *   E. Engine defaults
 *
 * Output: 5-column CSV to stdout.
 */

#include "ftd/a1g_projector.h"
#include "ftd/constants.h"
#include "ftd/render_bridge.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

namespace {

struct Config {
    std::string name;
    bool wave;
    bool coupling;
    bool damping;
    bool gauss;
    bool dual_substrate;
    bool poisson_coulomb;
    bool genesis;
    bool selective_damping;
    bool weak_transmutation;
    bool gravity;
    bool forces;
    bool movement;
    bool lorentz;
};

// Minimal toggles — turn off as much as we can.
void apply_config(ftd::RenderBridge& rb, const Config& c) {
    rb.toggles.wave_propagation = c.wave;
    rb.toggles.coupling         = c.coupling;
    rb.toggles.damping          = c.damping;
    rb.toggles.gauss_projection = c.gauss;
    rb.toggles.dual_substrate   = c.dual_substrate;
    rb.toggles.poisson_coulomb  = c.poisson_coulomb;
    rb.toggles.genesis          = c.genesis;
    rb.toggles.selective_damping= c.selective_damping;
    rb.toggles.weak_transmutation = c.weak_transmutation;
    rb.toggles.gravity          = c.gravity;
    rb.toggles.forces           = c.forces;
    rb.toggles.movement         = c.movement;
    rb.toggles.lorentz_force    = c.lorentz;
    rb.toggles.langevin         = false;
}

}  // namespace

int main() {
    constexpr int L = 32;
    const int n_ticks = 200;
    const double A = 0.5 * ftd::K_GENESIS;  // sub-genesis

    struct ConfigEx { Config c; int sor_iters; };
    std::vector<ConfigEx> configs_ex = {
        // wave coupl damp gauss dual pois gen selD weakT grav forc move lorz   sor_iters
        {{"A_wave_only",     true,  false, false, false, false, false, false, false, false, false, false, false, false}, 6},
        {{"C_gauss_def6",    true,  false, false, true,  false, true,  false, false, false, false, false, false, false}, 6},
        {{"C_gauss_iter50",  true,  false, false, true,  false, true,  false, false, false, false, false, false, false}, 50},
        {{"C_gauss_iter500", true,  false, false, true,  false, true,  false, false, false, false, false, false, false}, 500},
        {{"C_gauss_iter5000",true,  false, false, true,  false, true,  false, false, false, false, false, false, false}, 5000},
    };

    std::vector<Config> configs;
    std::vector<int> per_config_iters;
    configs.reserve(configs_ex.size());
    per_config_iters.reserve(configs_ex.size());
    for (auto& ce : configs_ex) {
        configs.push_back(ce.c);
        per_config_iters.push_back(ce.sor_iters);
    }

    std::cout << "tick";
    for (const auto& c : configs) std::cout << "," << c.name;
    std::cout << "\n";

    // Run each config independently, capture per-tick fraction
    std::vector<std::vector<double>> series(configs.size());

    for (std::size_t ci = 0; ci < configs.size(); ++ci) {
        ftd::RenderBridge rb(L);
        apply_config(rb, configs[ci]);
        rb.set_sor_iterations(per_config_iters[ci]);
        std::string err;
        if (!rb.toggles.validate(&err)) {
            std::cerr << "[" << configs[ci].name << "] toggle invalid: " << err << "\n";
            series[ci].assign(static_cast<std::size_t>(n_ticks + 1), -1.0);
            continue;
        }
        const int c = L / 2;
        rb.inject_flux(c, c, c, {A, 0.0, 0.0});

        // tick 0
        {
            auto fr = ftd::compute_a1g_fraction(rb.voxels(), L, c, c, c);
            series[ci].push_back(fr.mean);
        }
        for (int t = 0; t < n_ticks; ++t) {
            rb.tick();
            auto fr = ftd::compute_a1g_fraction(rb.voxels(), L, c, c, c);
            series[ci].push_back(fr.mean);
        }
    }

    // Emit CSV
    for (int t = 0; t <= n_ticks; ++t) {
        std::cout << t;
        for (std::size_t ci = 0; ci < configs.size(); ++ci) {
            std::cout << "," << std::fixed << std::setprecision(8) << series[ci][t];
        }
        std::cout << "\n";
    }
    return 0;
}
