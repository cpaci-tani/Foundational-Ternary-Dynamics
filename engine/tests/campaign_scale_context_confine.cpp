/**
 * campaign_scale_context_confine.cpp
 *
 * Confinement scan for the scale-context readout admissibility gate.
 *
 * The canonical A=14 Langevin cloud PERCOLATES the box at every L
 * (R_eff ∝ L, ζ ≈ 0.50; see SPEC_SCALE_CONTEXT_READOUT §5.4) because (a) the
 * vacuum is lossless so the coherent flux spreads to fill the box, and (b) the
 * T=0.005 thermal floor exceeds the gate's energy threshold everywhere
 * (f_active → 1). This scan asks: does any confining variant produce a cloud
 * whose R_eff is set by DYNAMICS rather than the box, i.e. a BoundedAdmissible
 * cloud the gate accepts?
 *
 * Levers tried:
 *   - global damping (damping=true, selective_damping=false): localizes the
 *     coherent flux to a finite penetration depth around the source.
 *   - low / zero Langevin T: drops the thermal floor below energy_threshold.
 *   - de_broglie_clock mass term (masses manifested voxels only).
 *
 * Read-only gate; golden hash untouched. This is an EXPLORATORY CPU scan
 * (quick-check); a confirmed admissible config should get a full WSL2/GPU run.
 */
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "ftd/render_bridge.h"
#include "ftd/scale_context.h"
#include "ftd/constants.h"

struct Config {
    std::string name;
    bool   langevin;
    double T;
    double gamma;
    bool   damping;            // global energy dissipation
    bool   selective_damping;  // true => vacuum lossless; false => global damp
    bool   coupling;           // g_c*grad(s) source term (helps sustain clusters)
    bool   de_broglie;
    double omega0;
    double amp;                // injected |J| = amp * K_GENESIS
};

static const char* regime_name(ftd::ScaleRegime r) {
    switch (r) {
        case ftd::ScaleRegime::Indeterminate:     return "Indeterminate";
        case ftd::ScaleRegime::Evaporating:       return "Evaporating";
        case ftd::ScaleRegime::UVLocked:          return "UVLocked";
        case ftd::ScaleRegime::BoundedAdmissible: return "BoundedAdmissible";
        case ftd::ScaleRegime::ShellDominated:    return "ShellDominated";
        case ftd::ScaleRegime::Percolating:       return "Percolating";
    }
    return "?";
}

static const char* status_name(ftd::ReadoutStatus s) {
    switch (s) {
        case ftd::ReadoutStatus::DiagnosticOnly:          return "DiagnosticOnly";
        case ftd::ReadoutStatus::Admissible:              return "ADMISSIBLE";
        case ftd::ReadoutStatus::RejectedScaleContext:    return "REJ_SCALE";
        case ftd::ReadoutStatus::RejectedSelfConfinement: return "REJ_CONFINE";
        case ftd::ReadoutStatus::RejectedNonStationary:   return "REJ_NONSTAT";
    }
    return "?";
}

static ftd::ScaleContextDiagnostics run_config(const Config& c, int L,
                                               int n_therm, int n_record) {
    ftd::RenderBridge rb(L);
    rb.force_cpu();
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.genesis = true;
    rb.toggles.coupling = c.coupling;
    rb.toggles.damping = c.damping;
    rb.toggles.selective_damping = c.selective_damping;
    rb.toggles.de_broglie_clock = c.de_broglie;
    rb.toggles.omega0 = c.omega0;
    rb.toggles.langevin = c.langevin;
    rb.toggles.langevin_T = c.T;
    rb.toggles.langevin_gamma = c.gamma;

    const int cc = L / 2;
    rb.inject_flux(cc, cc, cc, {c.amp * ftd::K_GENESIS, 0.0, 0.0});

    for (int t = 0; t < n_therm; ++t) rb.tick();

    ftd::ScaleContextConfig sc;
    sc.gate_active = true;  // armed, so we get a real verdict
    ftd::ScaleContextTracker tr(sc);
    ftd::ScaleContextDiagnostics d{};
    for (int t = 0; t < n_record; ++t) {
        rb.tick();
        d = tr.ingest(rb);
    }
    return d;
}

int main(int argc, char** argv) {
    int L = 48;
    int n_therm = 1200;
    int n_record = 120;   // > tracker window (64) so it warms up
    if (argc > 1) L = std::atoi(argv[1]);
    if (argc > 2) n_therm = std::atoi(argv[2]);
    if (argc > 3) n_record = std::atoi(argv[3]);

    std::cout << "================================================================\n";
    std::cout << "  Scale-context confinement scan  (L=" << L
              << ", therm=" << n_therm << ", record=" << n_record << ")\n";
    std::cout << "  K_GENESIS=" << ftd::K_GENESIS << "\n";
    std::cout << "================================================================\n";

    // Deterministic amplitude sweep (no Langevin => no thermal floor; global
    // damping localizes the coherent cloud). Larger amplitude => larger genesis
    // cluster => larger R_eff. Goal: land R_eff in the golden window [3, L/4].
    // 'baseline' (Langevin) kept as the Percolating reference.
    std::vector<Config> configs = {
        // name           lang   T       gamma  damp  selDamp coup  dB     w0    amp
        {"det-A26",       false, 0.0,    0.02,  true,  false, true,  false, 1.0,  26.0},
        {"det-A30",       false, 0.0,    0.02,  true,  false, true,  false, 1.0,  30.0},
        {"det-A34",       false, 0.0,    0.02,  true,  false, true,  false, 1.0,  34.0},
        {"det-A38",       false, 0.0,    0.02,  true,  false, true,  false, 1.0,  38.0},
        {"det-A42",       false, 0.0,    0.02,  true,  false, true,  false, 1.0,  42.0},
        {"det-A46",       false, 0.0,    0.02,  true,  false, true,  false, 1.0,  46.0},
    };

    std::cout << std::left << std::setw(14) << "config"
              << std::right << std::setw(9) << "R_eff"
              << std::setw(8) << "kappa"
              << std::setw(9) << "zeta"
              << std::setw(8) << "beta"
              << std::setw(10) << "f_active"
              << std::setw(8) << "conf"
              << std::setw(7) << "stat"
              << "  " << std::left << std::setw(18) << "regime"
              << std::setw(14) << "status" << "\n";
    std::cout << "----------------------------------------------------------------"
              << "------------------------------------\n";

    int n_admissible = 0;
    for (const auto& c : configs) {
        auto d = run_config(c, L, n_therm, n_record);
        if (d.status == ftd::ReadoutStatus::Admissible) ++n_admissible;
        std::cout << std::left << std::setw(14) << c.name
                  << std::right << std::fixed << std::setprecision(3)
                  << std::setw(9) << d.R_eff
                  << std::setw(8) << d.kappa
                  << std::setw(9) << d.zeta
                  << std::setw(8) << d.beta
                  << std::setw(10) << d.active_fraction
                  << std::setw(8) << (d.confinement_fixed_point ? "yes" : "no")
                  << std::setw(7) << (d.stationary ? "yes" : "no")
                  << "  " << std::left << std::setw(18) << regime_name(d.regime)
                  << std::setw(14) << status_name(d.status) << "\n";
    }

    std::cout << "----------------------------------------------------------------"
              << "------------------------------------\n";
    std::cout << "Admissible configs: " << n_admissible << " / " << configs.size() << "\n";
    return 0;
}
