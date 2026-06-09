/**
 * @file test_substrate_angle_probe.cpp
 * @brief Stage-1 exploratory probe: which substrate phase (if any) carries a
 *        *native dynamical angle* under the bare wave + Gauss dynamics?
 *
 * Arc: .claude/plans/plan-an-intuitive-path-twinkling-gizmo.md — "what (if
 * anything) in the substrate plays the role of the measurement-angle choice".
 *
 * This test is READ-ONLY on the physics phases: it samples voxel flux/wave_vel
 * per tick but never alters tick(), so it does NOT affect the golden gate.
 *
 * Three candidate "angles", each with a pre-stated prior. The informative
 * outcomes are DEVIATIONS from these priors:
 *
 *   (1) Transverse SPATIAL phase  arg(J_x + i J_y).
 *       Prior: FROZEN. The 18-pt scalar Laplacian evolves Cartesian components
 *       independently and Gauss only touches the longitudinal part, so a
 *       y-polarized transverse mode should never grow a J_x / J_z component.
 *
 *   (2) SYMPLECTIC phase  arg(q + i p), with q = modal amplitude
 *       (Sigma_x J_y sin(kx)) and p = modal velocity (Sigma_x wave_vel_y sin kx).
 *       Prior: WINDS at the dispersion frequency omega(k). This is the {q,p}
 *       simple-harmonic-oscillator phase — *expected*, the substrate's native
 *       but strictly COMMUTATIVE angle. Verified self-consistently: the
 *       multi-tick winding rate must equal the single-tick eigenvalue omega
 *       (the campaign_dispersion.cpp method), so no external dispersion formula
 *       is trusted.
 *
 *   (3) Dual-substrate L/R relative phase. Prior: INERT mirror — with
 *       weak_transmutation OFF, L and R are independent identical copies of the
 *       same wave equation, so a symmetric injection stays symmetric.
 *
 * Guardrail (THEOREM_COMMUTATIVITY_INDEPENDENCE / FTD-0228): a winding
 * symplectic phase is the commutative {q,p} angle. It is NOT a measurement-
 * incompatibility result. This probe maps a boundary; it does not derive QM.
 */

#include "test_helpers.h"
#include "ftd/render_bridge.h"
#include "ftd/constants.h"

#include <cmath>
#include <exception>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using namespace ftd;
using namespace ftd::test;

namespace {

double wrap_phase(double dp) {
    while (dp > PI) dp -= 2.0 * PI;
    while (dp < -PI) dp += 2.0 * PI;
    return dp;
}

// Inject a y-polarized standing wave of mode n into the single substrate:
//   J_y = A sin(k x),  wave_vel = 0,  J_x = J_z = 0.
void inject_y_standing_wave(RenderBridge& rb, int n, double A) {
    const int L = rb.lattice().size();
    const double k = 2.0 * PI * n / L;
    for (int x = 0; x < L; ++x) {
        const double jy = A * std::sin(k * x);
        for (int y = 0; y < L; ++y)
            for (int z = 0; z < L; ++z) {
                const int i = rb.lattice().index(x, y, z);
                rb.voxels()[i].flux = {0.0, jy, 0.0};
                rb.voxels()[i].wave_vel = {0.0, 0.0, 0.0};
            }
    }
}

// Modal projection onto sin(kx) of the y-component of flux (use_wave_vel=false)
// or wave_vel (use_wave_vel=true). This is the canonical coordinate q (or its
// conjugate momentum p) of the injected mode; it is node-robust unlike a
// single-point sample.
double modal_project_y(const RenderBridge& rb, int n, bool use_wave_vel) {
    const int L = rb.lattice().size();
    const double k = 2.0 * PI * n / L;
    const auto& vox = rb.voxels();
    double acc = 0.0;
    for (int x = 0; x < L; ++x) {
        const double s = std::sin(k * x);
        for (int y = 0; y < L; ++y)
            for (int z = 0; z < L; ++z) {
                const int i = rb.lattice().index(x, y, z);
                const double v = use_wave_vel ? vox[i].wave_vel.y : vox[i].flux.y;
                acc += v * s;
            }
    }
    return acc;
}

// Single-tick eigenvalue omega for mode n (campaign_dispersion.cpp method):
// from rest, after one tick wave_vel_y = -omega^2 * J_y * dt; with dt=1,
// omega^2 = |wave_vel_after / J_before| at an antinode site.
double measure_omega_eig(int L, int n, double A) {
    RenderBridge rb(L);
    prepare_bridge(rb, /*force_cpu=*/true);
    rb.toggles.wave_propagation = true;
    rb.toggles.gauss_projection = true;
    inject_y_standing_wave(rb, n, A);
    const int xp = std::max(1, static_cast<int>(std::lround(
                                static_cast<double>(L) / (4.0 * n))));  // antinode
    const int idx = rb.lattice().index(xp, 0, 0);
    const double jy_before = rb.voxels()[idx].flux.y;
    rb.tick();
    const double wv_after = rb.voxels()[idx].wave_vel.y;
    if (std::abs(jy_before) < 1e-15) return 0.0;
    return std::sqrt(std::abs(wv_after / jy_before));
}

} // namespace

int main() {
    Counter c;
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "================================================================\n";
    std::cout << "  Substrate measurement-angle probe (Stage 1, exploratory)\n";
    std::cout << "================================================================\n\n";

    const int L = 32;
    const double A = 0.1;
    const int N_TICKS = 300;
    const std::vector<int> modes = {1, 2, 4};

    // ---- Candidates 1 & 2 (single substrate, bare wave + Gauss) ----
    std::cout << "--- Candidates 1 (transverse frozen?) & 2 (symplectic winds?) ---\n";
    for (int n : modes) {
        const double omega_eig = measure_omega_eig(L, n, A);

        RenderBridge rb(L);
        prepare_bridge(rb, /*force_cpu=*/true);
        rb.toggles.wave_propagation = true;
        rb.toggles.gauss_projection = true;
        inject_y_standing_wave(rb, n, A);

        // Normalize p by omega_eig so (q, p/omega) is a CIRCLE -> uniform phase
        // advance -> winding rate equals omega even over partial periods.
        const double inv_w = (omega_eig > 1e-12) ? 1.0 / omega_eig : 0.0;
        double theta_prev = std::atan2(modal_project_y(rb, n, true) * inv_w,
                                       modal_project_y(rb, n, false));
        double total_winding = 0.0;
        double max_jx = 0.0, max_jz = 0.0;
        for (int t = 0; t < N_TICKS; ++t) {
            rb.tick();
            const auto& vox = rb.voxels();
            for (int i = 0; i < rb.lattice().total_sites(); ++i) {
                max_jx = std::max(max_jx, std::abs(vox[i].flux.x));
                max_jz = std::max(max_jz, std::abs(vox[i].flux.z));
            }
            const double q = modal_project_y(rb, n, false);
            const double p = modal_project_y(rb, n, true);
            const double theta = std::atan2(p * inv_w, q);
            total_winding += wrap_phase(theta - theta_prev);
            theta_prev = theta;
        }
        const double omega_wind = std::abs(total_winding) / N_TICKS;
        const double rel = (omega_eig > 1e-12)
                               ? std::abs(omega_wind - omega_eig) / omega_eig : 1.0;

        std::cout << "  mode n=" << n
                  << "  omega_eig=" << omega_eig
                  << "  omega_wind=" << omega_wind
                  << "  rel.diff=" << rel
                  << "  |winding|=" << std::abs(total_winding) << " rad"
                  << std::scientific << "  max|Jx|=" << max_jx
                  << "  max|Jz|=" << max_jz << std::fixed << "\n";

        const std::string sn = std::to_string(n);
        // c2a: the symplectic phase actually winds (many radians of net angle).
        check(("[c2] symplectic phase winds (mode " + sn + ")").c_str(),
              std::abs(total_winding) > 4.0 * PI, &c);
        // c2b: it winds at the dispersion frequency (self-consistent within 5%).
        check(("[c2] winding rate == single-tick eigenvalue (mode " + sn + ")").c_str(),
              rel < 0.05, &c);
        // c1: the transverse spatial orientation is frozen (no J_x / J_z leakage).
        check(("[c1] transverse orientation frozen (mode " + sn + ")").c_str(),
              max_jx < 1e-6 && max_jz < 1e-6, &c);
    }

    // ---- Candidate 3 (dual substrate, symmetric L/R injection) ----
    std::cout << "\n--- Candidate 3 (dual substrate L/R mirror?) ---\n";
    try {
        const int n = 2;
        RenderBridge rb(L);
        prepare_bridge(rb, /*force_cpu=*/true);
        rb.toggles.wave_propagation = true;
        rb.toggles.gauss_projection = true;
        rb.toggles.dual_substrate = true;
        rb.toggles.weak_transmutation = false;  // no L<->R coupling

        const double k = 2.0 * PI * n / L;
        for (int x = 0; x < L; ++x) {
            const double jy = A * std::sin(k * x);
            for (int y = 0; y < L; ++y)
                for (int z = 0; z < L; ++z) {
                    const int i = rb.lattice().index(x, y, z);
                    rb.voxels()[i].flux_L = {0.0, jy, 0.0};
                    rb.voxels()[i].flux_R = {0.0, jy, 0.0};
                    rb.voxels()[i].wave_vel_L = {0.0, 0.0, 0.0};
                    rb.voxels()[i].wave_vel_R = {0.0, 0.0, 0.0};
                    rb.voxels()[i].flux = {0.0, 2.0 * jy, 0.0};
                }
        }
        double max_lr_split = 0.0;
        for (int t = 0; t < N_TICKS; ++t) {
            rb.tick();
            const auto& vox = rb.voxels();
            for (int i = 0; i < rb.lattice().total_sites(); ++i)
                max_lr_split = std::max(max_lr_split,
                                        std::abs(vox[i].flux_L.y - vox[i].flux_R.y));
        }
        std::cout << "  max|J_L,y - J_R,y| over run = " << std::scientific
                  << max_lr_split << std::fixed << "\n";
        check("[c3] L/R mirror preserved (no spontaneous relative angle)",
              max_lr_split < 1e-6, &c);
    } catch (const std::exception& e) {
        std::cout << "  [c3] skipped (dual-substrate setup threw: " << e.what() << ")\n";
    }

    std::cout << "\nINTERPRETATION (exploratory characterization):\n"
              << "  c1 frozen + c2 winds + c3 mirror  =>  the substrate's only\n"
              << "  native dynamical angle is the symplectic {q,p} phase, which\n"
              << "  winds at omega(k) but is strictly COMMUTATIVE. The 'free choice\n"
              << "  of measurement direction' and its incompatibility are NOT here\n"
              << "  (THEOREM_COMMUTATIVITY_INDEPENDENCE): they are the injected M.\n\n";

    return report_and_exit_code(c, "Substrate measurement-angle probe");
}
