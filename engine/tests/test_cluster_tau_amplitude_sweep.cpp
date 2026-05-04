/**
 * Phase B.3 first deliverable: τ_e(A) curve via engine-default protocol.
 *
 * Following the resolution of Phase B.3 design challenge (see SPEC §5.6 +
 * test_cluster_decay_channels.cpp), the working protocol is engine defaults
 * + position-fixed mask persistence. This test sweeps cluster injection
 * amplitude A across the FTD-0110 SM-particle identifications and reports
 * τ_e(A), the time at which the cluster mask drops below e^(-1).
 *
 * FTD-0110 cluster-mass identification (linear): N(A) ≈ ¼ · (A/K_GENESIS)²
 *
 *   A=10·K_GENESIS → N≈25 voxels (electron-identified, m_e=0.511 MeV/c²)
 *   A=14·K_GENESIS → N≈49 voxels (~muon-identified, m_μ ≈ 207·m_e)
 *   A=20·K_GENESIS → N≈100 voxels (intermediate)
 *   A=30·K_GENESIS → N≈225 voxels
 *   A=42·K_GENESIS → N≈441 voxels (~proton-identified, m_p ≈ 1836·m_e)
 *
 * Note: the FTD-0110 cluster-mass identification is DIMENSIONLESS (cluster
 * size = N·m_e by FTD-0041 convention). The τ_e ratios across A values
 * provide a calibration-INVARIANT measurement comparable to dimensionless
 * SM lifetime ratios (e.g., τ_μ/τ_τ ≈ 7.6×10^6).
 *
 * Pre-registration: this is EXPLORATORY (1 seed per A; no statistical
 * uncertainty quoted). A pre-registered Phase B.3 campaign would seed-
 * sample to extract τ_e(A) ± σ.
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include <unordered_set>
#include <cmath>
#include "ftd/render_bridge.h"
#include "ftd/constants.h"

struct AResult {
    double A_over_KG;
    int initial_mask_size;
    int tau_e_tick;
    double persistence_at_end;
    int n_samples;
};

static std::unordered_set<int> snapshot_mask(const ftd::RenderBridge& rb) {
    std::unordered_set<int> mask;
    const auto& vox = rb.voxels();
    const int64_t total = rb.lattice().total_sites();
    for (int64_t i = 0; i < total; ++i)
        if (vox[i].state != 0) mask.insert(static_cast<int>(i));
    return mask;
}

static double compute_persistence(const ftd::RenderBridge& rb,
                                   const std::unordered_set<int>& mask) {
    if (mask.empty()) return 0.0;
    const auto& vox = rb.voxels();
    int n = 0;
    for (int idx : mask) if (vox[idx].state != 0) ++n;
    return static_cast<double>(n) / mask.size();
}

static AResult run_one(double A_over_KG, int L, int n_warmup,
                       int n_measure, int sample_interval) {
    ftd::RenderBridge rb(L);
    // Engine defaults — DO NOT call disable_all (per Phase B.3 §5.6 resolution).
    // The decay channel `weak_transmutation` is default-ON.

    const int cx = L / 2, cy = L / 2, cz = L / 2;
    const double A = A_over_KG * ftd::K_GENESIS;
    rb.inject_flux(cx, cy, cz, {A, 0.0, 0.0});

    for (int t = 0; t < n_warmup; ++t) rb.tick();
    auto mask = snapshot_mask(rb);

    AResult r;
    r.A_over_KG = A_over_KG;
    r.initial_mask_size = static_cast<int>(mask.size());
    r.tau_e_tick = -1;
    r.persistence_at_end = 1.0;
    r.n_samples = 0;

    for (int t = 1; t <= n_measure; ++t) {
        rb.tick();
        if (t % sample_interval == 0) {
            double p = compute_persistence(rb, mask);
            r.n_samples++;
            if (r.tau_e_tick < 0 && p < std::exp(-1.0)) r.tau_e_tick = t;
            r.persistence_at_end = p;
        }
    }
    return r;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  PHASE B.3 DELIVERABLE: τ_e(A) curve\n";
    std::cout << "================================================================\n\n";

    const int L = 32;
    const int N_WARMUP = 50;
    const int N_MEASURE = 500;
    const int SAMPLE_INTERVAL = 5;

    std::vector<double> A_values = {6.0, 10.0, 14.0, 20.0, 30.0, 42.0, 60.0, 84.0};

    std::cout << "Configuration: L=" << L
              << ", warmup=" << N_WARMUP
              << ", measure=" << N_MEASURE << " ticks, sample every "
              << SAMPLE_INTERVAL << "\n";
    std::cout << "Toggle config: engine defaults (weak_transmutation ON)\n";
    std::cout << "Per FTD-0110 linear: predicted N(A) ≈ ¼ · (A/K_GENESIS)²\n\n";

    std::cout << "  A/K_G    N_pred   N_obs    τ_e (ticks)    p_at_end    notes\n";
    std::cout << "  -----    ------   -----    -----------    --------    -----\n";

    std::vector<AResult> results;
    for (double A : A_values) {
        AResult r = run_one(A, L, N_WARMUP, N_MEASURE, SAMPLE_INTERVAL);
        results.push_back(r);
        int N_pred = static_cast<int>(0.25 * A * A);
        std::cout << std::fixed << std::setprecision(1)
                  << "  " << std::setw(5) << r.A_over_KG << "    "
                  << std::setw(6) << N_pred << "   "
                  << std::setw(5) << r.initial_mask_size << "    "
                  << std::setw(11) << (r.tau_e_tick < 0
                       ? std::string("> ") + std::to_string(N_MEASURE)
                       : std::to_string(r.tau_e_tick)) << "    "
                  << std::fixed << std::setprecision(4) << std::setw(8)
                  << r.persistence_at_end << "    ";

        // Notes
        if (r.A_over_KG <= 6.0) std::cout << "(below electron-identified)";
        else if (std::abs(r.A_over_KG - 10.0) < 0.5) std::cout << "(electron-identified)";
        else if (std::abs(r.A_over_KG - 14.0) < 0.5) std::cout << "(near muon-identified)";
        else if (std::abs(r.A_over_KG - 42.0) < 0.5) std::cout << "(near proton-identified)";
        std::cout << "\n";
    }

    // ----- Ratios -----
    std::cout << "\n--- τ_e ratios (calibration-invariant; comparable to PDG ratios) ---\n";
    for (size_t i = 0; i < results.size(); ++i) {
        for (size_t j = i + 1; j < results.size(); ++j) {
            const auto& a = results[i];
            const auto& b = results[j];
            if (a.tau_e_tick > 0 && b.tau_e_tick > 0) {
                double ratio = static_cast<double>(b.tau_e_tick) / a.tau_e_tick;
                std::cout << "  τ_e(A=" << std::fixed << std::setprecision(1)
                          << b.A_over_KG << ") / τ_e(A=" << a.A_over_KG << ") = "
                          << std::setprecision(3) << ratio
                          << "  (" << b.tau_e_tick << " / " << a.tau_e_tick << ")\n";
            }
        }
    }

    // ----- Verdict -----
    std::cout << "\n--- Phase B.3 deliverable verdict ---\n";
    int decayed_count = 0;
    bool tau_e_grows_with_A = true;
    int prev_tau = -1;
    for (const auto& r : results) {
        if (r.tau_e_tick > 0) {
            ++decayed_count;
            if (prev_tau > 0 && r.tau_e_tick < prev_tau - 5) tau_e_grows_with_A = false;
            prev_tau = r.tau_e_tick;
        }
    }

    std::cout << "  Amplitudes producing observable decay: "
              << decayed_count << " / " << results.size() << "\n";
    std::cout << "  τ_e (mostly) increases with A (heavier clusters last longer): "
              << (tau_e_grows_with_A ? "YES" : "no") << "\n";

    std::cout << "\n  ";
    if (decayed_count >= 3 && tau_e_grows_with_A) {
        std::cout << "[VERDICT] Phase B.3 first deliverable LANDED — clean τ_e(A) curve\n";
        std::cout << "  observed across at least 3 amplitudes, with heavier clusters showing\n";
        std::cout << "  longer lifetimes (consistent with SM intuition: heavier particles\n";
        std::cout << "  more stable per unit mass). Ratios above are the load-bearing\n";
        std::cout << "  comparison targets for PDG ratio testing.\n";
        std::cout << "\n  Next step: pre-registered M-seed campaign to extract τ_e(A) ± σ\n";
        std::cout << "  with statistical uncertainties; then map FTD cluster amplitudes to\n";
        std::cout << "  SM particle identifications and compare ratios to PDG branching/\n";
        std::cout << "  lifetime ratios.\n";
    } else {
        std::cout << "[VERDICT] Phase B.3 deliverable INCOMPLETE — "
                  << decayed_count << " amplitudes decayed; "
                  << (tau_e_grows_with_A ? "monotonic" : "non-monotonic") << " τ_e(A).\n";
        std::cout << "  Need to expand A range or N_MEASURE to capture decay envelope.\n";
    }

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: REPORTED (single-seed; pre-registration deferred)\n";
    std::cout << "================================================================\n";
    return 0;
}
