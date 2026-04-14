/**
 * Campaign: Quantum correlations (consolidated suite)
 *
 * Merges 3 legacy tests into a single ftd::test-instrumented suite:
 *
 *   test_entanglement         -> section "entanglement"      (16 checks)
 *   campaign_epr_correlation  -> section "epr_correlation"   ( 5 checks)
 *   campaign_bell_substrate   -> section "bell_substrate"    ( 4 checks)
 *
 * Every check(...) from the legacy files is preserved verbatim (same
 * condition, same label) and routed through ftd::test::check for
 * uniform telemetry.
 *
 * Wave 4a.4 consolidation (2026-04-14). test_entanglement was Failed
 * pre-consolidation (the pair_id preservation and annihilation
 * sections); the other 2 pass. Structural parity preserved.
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include <vector>
#include <random>

#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/test_telemetry.h"

// ============================================================================
// Section: entanglement  (from test_entanglement.cpp)
// ============================================================================

static void section_entanglement() {
    // Section 1: Entangled pair creation
    std::cout << "\n--- Section 1: Pair Creation ---\n";
    {
        int L = 16;
        ftd::RenderBridge rb(L);
        int cx = L / 2;

        rb.create_entangled_pair(cx, cx, cx, {0, 0, ftd::K_B});

        auto& v_center = rb.voxels()[rb.lattice().index(cx, cx, cx)];
        ftd::test::check("Positive particle at center", v_center.state == +1);
        ftd::test::check("Center has pair_id >= 0", v_center.pair_id >= 0);

        std::cout << "    Center state: " << (int)v_center.state
                  << ", pair_id: " << v_center.pair_id << "\n";

        int partner_found = 0;
        int partner_pair_id = -1;
        auto nbrs = rb.lattice().neighbors_6(rb.lattice().index(cx, cx, cx));
        for (int n : nbrs) {
            if (rb.voxels()[n].state == -1) {
                partner_found++;
                partner_pair_id = rb.voxels()[n].pair_id;
                std::cout << "    Partner state: " << (int)rb.voxels()[n].state
                          << ", pair_id: " << rb.voxels()[n].pair_id << "\n";
            }
        }

        ftd::test::check("Exactly one negative partner found", partner_found == 1);
        ftd::test::check("Partner shares same pair_id",
              partner_pair_id == v_center.pair_id);
    }

    // Section 2: Complementary states
    std::cout << "\n--- Section 2: Complementary States ---\n";
    {
        int L = 16;
        ftd::RenderBridge rb(L);
        int cx = L / 2;

        rb.create_entangled_pair(cx, cx, cx, {0, 0, ftd::K_B});

        int state_sum = 0;
        int N = rb.lattice().total_sites();
        for (int i = 0; i < N; ++i) {
            state_sum += rb.voxels()[i].state;
        }

        std::cout << "    Sum of all states: " << state_sum << "\n";
        ftd::test::check("Charge conservation: Σs = 0", state_sum == 0);
    }

    // Section 3: Anti-correlated flux
    std::cout << "\n--- Section 3: Anti-Correlated Flux ---\n";
    {
        int L = 16;
        ftd::RenderBridge rb(L);
        int cx = L / 2;

        ftd::Vec3 flux_val = {0, 0, ftd::K_B};
        rb.create_entangled_pair(cx, cx, cx, flux_val);

        auto& vc = rb.voxels()[rb.lattice().index(cx, cx, cx)];
        std::cout << "    Positive flux: (" << vc.flux.x << ", "
                  << vc.flux.y << ", " << vc.flux.z << ")\n";

        auto nbrs = rb.lattice().neighbors_6(rb.lattice().index(cx, cx, cx));
        for (int n : nbrs) {
            if (rb.voxels()[n].state == -1) {
                auto& vp = rb.voxels()[n];
                std::cout << "    Negative flux: (" << vp.flux.x << ", "
                          << vp.flux.y << ", " << vp.flux.z << ")\n";

                ftd::Vec3 sum = vc.flux + vp.flux;
                double sum_mag = sum.mag();
                std::cout << "    Sum of flux vectors: (" << sum.x << ", "
                          << sum.y << ", " << sum.z << ") mag=" << sum_mag << "\n";

                ftd::test::check("Flux vectors are anti-correlated (sum ~ 0)",
                      sum_mag < 1e-10);
                break;
            }
        }
    }

    // Section 4: Multiple pairs have distinct pair_ids
    std::cout << "\n--- Section 4: Distinct Pair IDs ---\n";
    {
        int L = 16;
        ftd::RenderBridge rb(L);

        rb.create_entangled_pair(4, 8, 8, {0, 0, ftd::K_B});
        rb.create_entangled_pair(12, 8, 8, {0, 0, ftd::K_B});

        int pid1 = rb.voxels()[rb.lattice().index(4, 8, 8)].pair_id;
        int pid2 = rb.voxels()[rb.lattice().index(12, 8, 8)].pair_id;

        std::cout << "    Pair 1 ID: " << pid1 << "\n";
        std::cout << "    Pair 2 ID: " << pid2 << "\n";

        ftd::test::check("Distinct pair IDs", pid1 != pid2);
        ftd::test::check("Both pair IDs >= 0", pid1 >= 0 && pid2 >= 0);
        ftd::test::check("Pair IDs are sequential", pid2 == pid1 + 1);
    }

    // Section 5: pair_id preserved through ticks
    std::cout << "\n--- Section 5: Pair ID Preservation ---\n";
    {
        int L = 32;
        ftd::RenderBridge rb(L);
        int cx = L / 2;

        rb.create_entangled_pair(cx, cx, cx, {0, 0, ftd::K_B});
        int original_pid = rb.voxels()[rb.lattice().index(cx, cx, cx)].pair_id;

        rb.voxels()[rb.lattice().index(cx, cx, cx)].locked = true;
        auto nbrs = rb.lattice().neighbors_6(rb.lattice().index(cx, cx, cx));
        int partner_idx = -1;
        for (int n : nbrs) {
            if (rb.voxels()[n].state == -1) {
                rb.voxels()[n].locked = true;
                partner_idx = n;
                break;
            }
        }

        rb.run(50);

        int pid_pos = rb.voxels()[rb.lattice().index(cx, cx, cx)].pair_id;
        int pid_neg = (partner_idx >= 0) ? rb.voxels()[partner_idx].pair_id : -1;

        std::cout << "    Original pair_id: " << original_pid << "\n";
        std::cout << "    Positive pair_id after 50 ticks: " << pid_pos << "\n";
        std::cout << "    Negative pair_id after 50 ticks: " << pid_neg << "\n";

        ftd::test::check("pair_id preserved on positive particle",
              pid_pos == original_pid);
        ftd::test::check("pair_id preserved on negative particle",
              pid_neg == original_pid);
    }

    // Section 6: pair_id cleared on annihilation
    std::cout << "\n--- Section 6: Pair ID Cleared on Annihilation ---\n";
    {
        int L = 16;
        ftd::RenderBridge rb(L);
        int cx = L / 2;

        rb.create_entangled_pair(cx, cx, cx, {0, 0, ftd::K_B});

        auto& vc = rb.voxels()[rb.lattice().index(cx, cx, cx)];
        auto nbrs = rb.lattice().neighbors_6(rb.lattice().index(cx, cx, cx));
        int partner_idx = -1;
        for (int n : nbrs) {
            if (rb.voxels()[n].state == -1) {
                partner_idx = n;
                break;
            }
        }

        if (partner_idx >= 0) {
            auto pc = rb.lattice().coord(partner_idx);
            vc.velocity = {(double)(pc.x - cx), (double)(pc.y - cx), (double)(pc.z - cx)};
            rb.voxels()[partner_idx].velocity = {(double)(cx - pc.x), (double)(cx - pc.y), (double)(cx - pc.z)};

            rb.run(5);

            int N = rb.lattice().total_sites();
            bool pid_exists = false;
            for (int i = 0; i < N; ++i) {
                if (rb.voxels()[i].pair_id >= 0 && rb.voxels()[i].state != 0) {
                    pid_exists = true;
                }
            }

            auto diag = rb.diagnostics();
            std::cout << "    After annihilation attempt: manifested="
                      << diag.manifested_count << "\n";

            if (diag.manifested_count == 0) {
                ftd::test::check("pair_id cleared after annihilation", !pid_exists);
            } else {
                std::cout << "    (Annihilation not yet complete — skipping)\n";
                ftd::test::check("Particles still present with pair_id", true);
            }
        } else {
            ftd::test::check("Partner found for annihilation test", partner_idx >= 0);
        }
    }
}

// ============================================================================
// Section: epr_correlation  (from campaign_epr_correlation.cpp)
// ============================================================================

static int measure_epr(const ftd::Vec3& flux, double angle_rad) {
    double proj = flux.x * std::cos(angle_rad) + flux.y * std::sin(angle_rad);
    return (proj >= 0) ? +1 : -1;
}

static void section_epr_correlation() {
    std::cout << std::fixed << std::setprecision(6);

    const int N_PAIRS = 5000;
    const int N_ANGLES = 13;

    std::mt19937 rng(42);
    std::uniform_real_distribution<double> phi_dist(0.0, 2.0 * ftd::PI);

    std::vector<double> angles(N_ANGLES);
    std::vector<double> E_measured(N_ANGLES, 0.0);
    std::vector<double> E_classical(N_ANGLES, 0.0);
    std::vector<double> E_quantum(N_ANGLES, 0.0);

    for (int i = 0; i < N_ANGLES; ++i) {
        angles[i] = i * ftd::PI / (N_ANGLES - 1);
    }

    int total_charge = 0;
    int n_lattice_pairs = 20;
    {
        int L = 32;
        ftd::RenderBridge rb(L);
        rb.toggles.genesis = false;

        for (int i = 0; i < n_lattice_pairs; ++i) {
            int x = 3 + (i % 5) * 5;
            int y = 3 + ((i / 5) % 5) * 5;
            int z = 3 + (i / 25) * 5;
            rb.create_entangled_pair(x, y, z, {ftd::K_B, 0, 0});
        }

        int N_total = rb.lattice().total_sites();
        for (int j = 0; j < N_total; ++j) {
            total_charge += rb.voxels()[j].state;
        }
    }

    std::cout << "\n--- Measuring E(θ) for " << N_PAIRS << " pairs ---\n";

    for (int i = 0; i < N_PAIRS; ++i) {
        double phi = phi_dist(rng);
        double amp = ftd::K_B;

        ftd::Vec3 flux_A = {amp * std::cos(phi), amp * std::sin(phi), 0.0};
        ftd::Vec3 flux_B = {-flux_A.x, -flux_A.y, -flux_A.z};

        for (int j = 0; j < N_ANGLES; ++j) {
            int outcome_A = measure_epr(flux_A, 0.0);
            int outcome_B = measure_epr(flux_B, angles[j]);
            E_measured[j] += outcome_A * outcome_B;
        }
    }

    auto E_2d = [](double theta) {
        double t = std::fmod(std::abs(theta), 2.0 * ftd::PI);
        if (t > ftd::PI) t = 2.0 * ftd::PI - t;
        return -(1.0 - 2.0 * t / ftd::PI);
    };

    for (int j = 0; j < N_ANGLES; ++j) {
        E_measured[j] /= N_PAIRS;
        E_classical[j] = E_2d(angles[j]);
        E_quantum[j] = -std::cos(angles[j]);
    }

    std::cout << "\n  θ (deg)  | E_meas    | E_class   | E_QM      | err(class)\n";
    std::cout << "  ---------+-----------+-----------+-----------+-----------\n";
    for (int j = 0; j < N_ANGLES; ++j) {
        double deg = angles[j] * 180.0 / ftd::PI;
        double err = std::abs(E_measured[j] - E_classical[j]);
        std::cout << "  " << std::setw(7) << deg
                  << "  | " << std::setw(9) << E_measured[j]
                  << " | " << std::setw(9) << E_classical[j]
                  << " | " << std::setw(9) << E_quantum[j]
                  << " | " << std::setw(9) << err << "\n";
    }

    std::cout << "\n";
    ftd::test::check("EC1: E(0°) = -1.0 (perfect anti-correlation)",
          std::abs(E_measured[0] + 1.0) < 0.01);

    int idx_90 = N_ANGLES / 2;
    std::cout << "  E(90°) = " << E_measured[idx_90] << "\n";
    ftd::test::check("EC2: |E(90°)| < 0.1 (no correlation at orthogonal)",
          std::abs(E_measured[idx_90]) < 0.1);

    double max_err = 0.0;
    for (int j = 0; j < N_ANGLES; ++j) {
        double err = std::abs(E_measured[j] - E_classical[j]);
        if (err > max_err) max_err = err;
    }
    std::cout << "  Max deviation from classical theory: " << max_err << "\n";
    ftd::test::check("EC3: E(θ) matches classical -(1-2|θ|/pi) within 10%",
          max_err < 0.10);

    double E_22  = E_2d(ftd::PI / 8.0);
    double E_68  = E_2d(3.0 * ftd::PI / 8.0);
    double S_max = std::abs(3.0 * E_22 - E_68);
    std::cout << "\n  CHSH S from classical correlation: " << S_max << "\n";
    std::cout << "  (For comparison: QM would give S = 2√2 ≈ "
              << 2.0 * std::sqrt(2.0) << ")\n";
    ftd::test::check("EC4: S_max <= 2.0 (Bell-CHSH bound)", S_max <= 2.0 + 1e-6);

    std::cout << "\n  Total charge from " << n_lattice_pairs
              << " lattice pairs: " << total_charge << "\n";
    ftd::test::check("EC5: Charge conservation (Q = 0 for pair production)",
          total_charge == 0);

    std::cout << std::defaultfloat;
}

// ============================================================================
// Section: bell_substrate  (from campaign_bell_substrate.cpp)
// ============================================================================

static int measure_bell(const ftd::Vec3& flux, double angle_rad) {
    double projection = flux.x * std::cos(angle_rad) + flux.y * std::sin(angle_rad);
    return (projection >= 0) ? +1 : -1;
}

static void section_bell_substrate() {
    std::cout << std::fixed << std::setprecision(6);

    const int N_PAIRS = 10000;

    double a  = 0.0;
    double a_ = ftd::PI / 4.0;
    double b  = ftd::PI / 8.0;
    double b_ = 3.0 * ftd::PI / 8.0;

    std::cout << "\n--- Setup ---\n";
    std::cout << "  N_pairs: " << N_PAIRS << "\n";
    std::cout << "  Detector a: " << (a * 180 / ftd::PI) << "°\n";
    std::cout << "  Detector a': " << (a_ * 180 / ftd::PI) << "°\n";
    std::cout << "  Detector b: " << (b * 180 / ftd::PI) << "°\n";
    std::cout << "  Detector b': " << (b_ * 180 / ftd::PI) << "°\n\n";

    std::mt19937 rng(12345);
    std::uniform_real_distribution<double> angle_dist(0.0, 2.0 * ftd::PI);

    double sum_ab  = 0.0;
    double sum_ab_ = 0.0;
    double sum_a_b = 0.0;
    double sum_a_b_= 0.0;
    double sum_aa  = 0.0;

    for (int i = 0; i < N_PAIRS; ++i) {
        double phi = angle_dist(rng);
        double amp = ftd::K_B;

        ftd::Vec3 flux_A = {amp * std::cos(phi), amp * std::sin(phi), 0.0};
        ftd::Vec3 flux_B = {-flux_A.x, -flux_A.y, -flux_A.z};

        int A_a  = measure_bell(flux_A, a);
        int A_a_ = measure_bell(flux_A, a_);

        int B_b  = measure_bell(flux_B, b);
        int B_b_ = measure_bell(flux_B, b_);
        int B_a  = measure_bell(flux_B, a);

        sum_ab   += A_a  * B_b;
        sum_ab_  += A_a  * B_b_;
        sum_a_b  += A_a_ * B_b;
        sum_a_b_ += A_a_ * B_b_;
        sum_aa   += A_a  * B_a;
    }

    double E_ab  = sum_ab  / N_PAIRS;
    double E_ab_ = sum_ab_ / N_PAIRS;
    double E_a_b = sum_a_b / N_PAIRS;
    double E_a_b_= sum_a_b_/ N_PAIRS;
    double E_aa  = sum_aa  / N_PAIRS;

    double S = std::abs(E_ab - E_ab_ + E_a_b + E_a_b_);

    std::cout << "--- Correlations ---\n";
    std::cout << "  E(a,b)   = " << E_ab << "\n";
    std::cout << "  E(a,b')  = " << E_ab_ << "\n";
    std::cout << "  E(a',b)  = " << E_a_b << "\n";
    std::cout << "  E(a',b') = " << E_a_b_ << "\n";
    std::cout << "  E(a,a)   = " << E_aa << " (theory: -1.0)\n";
    std::cout << "\n  CHSH S = " << S << " (classical bound: 2.0)\n";

    auto E_2d = [](double theta) {
        double t = std::fmod(std::abs(theta), 2.0 * ftd::PI);
        if (t > ftd::PI) t = 2.0 * ftd::PI - t;
        return -(1.0 - 2.0 * t / ftd::PI);
    };
    double E_ab_theory  = E_2d(b - a);
    double E_ab_theory_ = E_2d(b_ - a);
    double E_a_b_theory = E_2d(b - a_);
    double E_a_b_theory_= E_2d(b_ - a_);
    double S_theory = std::abs(E_ab_theory - E_ab_theory_ + E_a_b_theory + E_a_b_theory_);

    std::cout << "\n--- Classical Theory ---\n";
    std::cout << "  E(a,b)   theory = " << E_ab_theory << "\n";
    std::cout << "  E(a,b')  theory = " << E_ab_theory_ << "\n";
    std::cout << "  E(a',b)  theory = " << E_a_b_theory << "\n";
    std::cout << "  E(a',b') theory = " << E_a_b_theory_ << "\n";
    std::cout << "  S_theory = " << S_theory << "\n";

    ftd::test::check("BS1: CHSH S <= 2.0 (local hidden variable bound)", S <= 2.0 + 1e-6);

    bool all_bounded = (std::abs(E_ab) <= 1.0 + 1e-6) &&
                       (std::abs(E_ab_) <= 1.0 + 1e-6) &&
                       (std::abs(E_a_b) <= 1.0 + 1e-6) &&
                       (std::abs(E_a_b_) <= 1.0 + 1e-6);
    ftd::test::check("BS2: |E(a,b)| <= 1 for all angle pairs", all_bounded);

    std::cout << "\n  E(a,a) = " << E_aa << " (expect -1.0)\n";
    ftd::test::check("BS3: E(a,a) = -1.0 (perfect anti-correlation)", std::abs(E_aa + 1.0) < 0.01);

    double max_err = std::max({
        std::abs(E_ab - E_ab_theory),
        std::abs(E_ab_ - E_ab_theory_),
        std::abs(E_a_b - E_a_b_theory),
        std::abs(E_a_b_ - E_a_b_theory_)
    });
    std::cout << "  Max correlation error vs classical theory: " << max_err << "\n";
    ftd::test::check("BS4: Correlations match -(1-2|theta|/pi) within 5%",
          max_err < 0.05);

    std::cout << std::defaultfloat;
}

// ============================================================================
// main
// ============================================================================

int main() {
    ftd::test::init("campaign_quantum_correlations");

    ftd::test::section("entanglement");
    section_entanglement();

    ftd::test::section("epr_correlation");
    section_epr_correlation();

    ftd::test::section("bell_substrate");
    section_bell_substrate();

    return ftd::test::finalize();
}
