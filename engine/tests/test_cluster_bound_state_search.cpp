/**
 * Phase B.3 (β''): search for a true bound-state cluster regime.
 *
 * The soliton-vs-flooding finding (test_cluster_a10_centroid_drift.cpp)
 * showed that the engine's default toggles produce two regimes neither of
 * which is a true bound state:
 *   - Soliton (A=10): conserved matter + directed motion
 *   - Flooding (A=7, 8): runaway lattice nucleation
 *
 * A true BOUND STATE would have: total_manifested ≈ const + centroid
 * stationary + rms_radius bounded (analogous to a stable particle).
 *
 * This test sweeps toggle configurations adding binding-channel toggles to
 * the engine defaults, and for each, measures the triplet metric at three
 * representative amplitudes (A=7, 10, 14). Each (toggle, A) pair is
 * classified into one of four regimes via the triplet.
 *
 * Toggles to test:
 *   - confinement
 *   - pair_production
 *   - color_forces (requires dual_substrate which is default)
 *   - color_forces + triad_binding
 *   - strong_force
 *   - exchange_force
 *   - latency_field
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <string>
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"

struct TripletMetric {
    int initial_n;
    int mid_n;
    int final_n;
    double initial_centroid_dist;
    double final_centroid_dist;
    double centroid_drift;
    double final_rms;
    std::string regime;
};

static const double TWOPI = 2.0 * 3.14159265358979323846;

static void compute_metric(const ftd::RenderBridge& rb, int inj,
                            int& n_total, double& cx, double& cy, double& cz, double& rms) {
    const auto& vox = rb.voxels();
    const auto& lat = rb.lattice();
    const int L = lat.size();
    const int64_t total = lat.total_sites();

    n_total = 0;
    double sx_x = 0, cx_x = 0, sx_y = 0, cx_y = 0, sx_z = 0, cx_z = 0;
    for (int64_t i = 0; i < total; ++i) {
        if (vox[i].state == 0) continue;
        ++n_total;
        auto c = lat.coord(static_cast<int>(i));
        sx_x += std::sin(TWOPI * c.x / L); cx_x += std::cos(TWOPI * c.x / L);
        sx_y += std::sin(TWOPI * c.y / L); cx_y += std::cos(TWOPI * c.y / L);
        sx_z += std::sin(TWOPI * c.z / L); cx_z += std::cos(TWOPI * c.z / L);
    }
    if (n_total == 0) { cx = cy = cz = rms = 0; return; }

    cx = std::atan2(sx_x, cx_x) * L / TWOPI; if (cx < 0) cx += L;
    cy = std::atan2(sx_y, cx_y) * L / TWOPI; if (cy < 0) cy += L;
    cz = std::atan2(sx_z, cx_z) * L / TWOPI; if (cz < 0) cz += L;

    auto wrap = [L](double d) {
        if (d > L/2.0) d -= L;
        if (d < -L/2.0) d += L;
        return d;
    };

    rms = 0;
    for (int64_t i = 0; i < total; ++i) {
        if (vox[i].state == 0) continue;
        auto c = lat.coord(static_cast<int>(i));
        double dx = wrap(c.x - cx), dy = wrap(c.y - cy), dz = wrap(c.z - cz);
        rms += dx*dx + dy*dy + dz*dz;
    }
    rms = std::sqrt(rms / n_total);
}

enum class TogCfg {
    Defaults, Confinement, PairProd, ColorForces, ColorTriad,
    StrongForce, ExchangeForce, LatencyField
};

static const char* cfg_name(TogCfg c) {
    switch (c) {
        case TogCfg::Defaults: return "defaults";
        case TogCfg::Confinement: return "+confinement";
        case TogCfg::PairProd: return "+pair_production";
        case TogCfg::ColorForces: return "+color_forces";
        case TogCfg::ColorTriad: return "+color+triad";
        case TogCfg::StrongForce: return "+strong_force";
        case TogCfg::ExchangeForce: return "+exchange_force";
        case TogCfg::LatencyField: return "+latency_field";
    }
    return "?";
}

static void apply_cfg(ftd::RenderBridge& rb, TogCfg c) {
    // Keep all engine defaults; ADD the named binding toggle(s).
    switch (c) {
        case TogCfg::Defaults: break;
        case TogCfg::Confinement: rb.toggles.confinement = true; break;
        case TogCfg::PairProd: rb.toggles.pair_production = true; break;
        case TogCfg::ColorForces: rb.toggles.color_forces = true; break;
        case TogCfg::ColorTriad:
            rb.toggles.color_forces = true;
            rb.toggles.triad_binding = true;
            break;
        case TogCfg::StrongForce: rb.toggles.strong_force = true; break;
        case TogCfg::ExchangeForce: rb.toggles.exchange_force = true; break;
        case TogCfg::LatencyField: rb.toggles.latency_field = true; break;
    }
}

static TripletMetric run_one(double A_over_KG, TogCfg cfg) {
    const int L = 32;
    const int N_WARMUP = 50;
    const int N_TRACE = 200;
    const int inj = L / 2;

    ftd::RenderBridge rb(L);
    apply_cfg(rb, cfg);
    std::string err;
    if (!rb.toggles.validate(&err)) {
        TripletMetric m;
        m.initial_n = -1;
        m.regime = std::string("INVALID(") + err + ")";
        return m;
    }
    rb.toggles.langevin_seed = 1;
    rb.inject_flux(inj, inj, inj, {A_over_KG * ftd::K_GENESIS, 0.0, 0.0});
    for (int t = 0; t < N_WARMUP; ++t) rb.tick();

    TripletMetric m;
    int n; double cx, cy, cz, rms;
    compute_metric(rb, inj, n, cx, cy, cz, rms);
    m.initial_n = n;
    auto wrap = [L](double d) { if (d > L/2.0) d -= L; if (d < -L/2.0) d += L; return d; };
    double dx0 = wrap(cx - inj), dy0 = wrap(cy - inj), dz0 = wrap(cz - inj);
    double cx0 = cx, cy0 = cy, cz0 = cz;
    m.initial_centroid_dist = std::sqrt(dx0*dx0 + dy0*dy0 + dz0*dz0);

    for (int t = 0; t < N_TRACE / 2; ++t) rb.tick();
    compute_metric(rb, inj, n, cx, cy, cz, rms);
    m.mid_n = n;

    for (int t = 0; t < N_TRACE / 2; ++t) rb.tick();
    compute_metric(rb, inj, n, cx, cy, cz, rms);
    m.final_n = n;
    double dxf = wrap(cx - cx0), dyf = wrap(cy - cy0), dzf = wrap(cz - cz0);
    m.centroid_drift = std::sqrt(dxf*dxf + dyf*dyf + dzf*dzf);
    double dxfi = wrap(cx - inj), dyfi = wrap(cy - inj), dzfi = wrap(cz - inj);
    m.final_centroid_dist = std::sqrt(dxfi*dxfi + dyfi*dyfi + dzfi*dzfi);
    m.final_rms = rms;

    // Classify
    if (m.final_n > 3 * m.initial_n)         m.regime = "FLOODING";
    else if (m.final_n == 0)                  m.regime = "FULL DECAY";
    else if (m.final_n < m.initial_n / 3)     m.regime = "DECAYING";
    else if (m.centroid_drift > 3.0)          m.regime = "SOLITON";
    else if (m.final_rms > L / 4.0)           m.regime = "DIFFUSING";
    else                                       m.regime = "BOUND";

    return m;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  PHASE B.3 (β''): bound-state search via binding-toggle sweep\n";
    std::cout << "================================================================\n\n";
    std::cout << "Triplet metric: total_manifested + centroid_drift + RMS\n";
    std::cout << "Looking for: (n_total ≈ const) ∧ (centroid stationary) ∧ (rms bounded)\n";
    std::cout << "  → BOUND state regime (analogous to stable particle)\n\n";

    std::vector<TogCfg> configs = {
        TogCfg::Defaults, TogCfg::Confinement, TogCfg::PairProd,
        TogCfg::ColorForces, TogCfg::ColorTriad, TogCfg::StrongForce,
        TogCfg::ExchangeForce, TogCfg::LatencyField
    };
    std::vector<double> A_vals = {7.0, 10.0, 14.0};

    std::cout << std::left << std::setw(20) << "config" << std::right
              << std::setw(7) << "A/K_G"
              << std::setw(8) << "n_init"
              << std::setw(7) << "n_mid"
              << std::setw(8) << "n_final"
              << std::setw(10) << "drift"
              << std::setw(8) << "rms"
              << "  regime\n";
    std::cout << "------------------- ------ ------- ------ ------- --------- -------  -------\n";

    int n_bound_found = 0;
    for (TogCfg cfg : configs) {
        for (double A : A_vals) {
            TripletMetric m = run_one(A, cfg);
            if (m.initial_n < 0) {
                std::cout << std::left << std::setw(20) << cfg_name(cfg) << std::right
                          << std::setw(7) << std::fixed << std::setprecision(1) << A
                          << "  " << m.regime << "\n";
                continue;
            }
            std::cout << std::left << std::setw(20) << cfg_name(cfg) << std::right
                      << std::setw(7) << std::fixed << std::setprecision(1) << A
                      << std::setw(8) << m.initial_n
                      << std::setw(7) << m.mid_n
                      << std::setw(8) << m.final_n
                      << std::setw(10) << std::setprecision(2) << m.centroid_drift
                      << std::setw(8) << std::setprecision(2) << m.final_rms
                      << "  " << m.regime << "\n";
            if (m.regime == "BOUND") ++n_bound_found;
        }
    }

    std::cout << "\n--- Verdict ---\n";
    std::cout << "  BOUND-state regime found: " << n_bound_found << " / "
              << (configs.size() * A_vals.size()) << " configurations\n";
    if (n_bound_found > 0) {
        std::cout << "\n  [VERDICT] Bound-state regime EXISTS for at least one (toggle, A) pair.\n";
        std::cout << "  Phase B has a candidate physical-cluster regime. Next: characterize\n";
        std::cout << "  the lightest bound state and connect to FTD-0110 mass identification.\n";
    } else {
        std::cout << "\n  [VERDICT] NO bound-state regime found in tested (toggle, A) sweep.\n";
        std::cout << "  Either binding requires a more specific toggle combination, or the\n";
        std::cout << "  engine's discrete dynamics genuinely do not support stable bound\n";
        std::cout << "  states under any of these toggles. Substantive finding either way.\n";
    }

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: REPORTED (single-seed binding-toggle sweep)\n";
    std::cout << "================================================================\n";
    return 0;
}
