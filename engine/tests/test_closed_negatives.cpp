/**
 * @file test_closed_negatives.cpp
 * @brief Regression guards for closed-negative ledger claims.
 *
 * The audit (test-orchestrator, 2026-04-25) flagged that closed-negative
 * findings (FTD-0050, 0061, 0071, 0072, 0073, 0075) have NO regression tests.
 * If a future engine change drifts to make them positive again, they would
 * be silently re-discovered and re-celebrated.
 *
 * This test pins the most consequential closed-negatives at the engine level:
 *
 *   CN-1. Mode-erasure on 2³ block (FTD-0061, 0073): site-local 0-form
 *         state-field readout under genesis+movement collapses the
 *         anticommutator {ê_i, ê_j} to a uniform c·𝟙 — diagonal AND
 *         off-diagonal both equal a constant (NOT Cl(3,0) Clifford).
 *
 *   CN-2. Flux 1-form separable algebra (FTD-0074): bilinear axial
 *         readouts under site-local dynamics produce
 *         {L_f, L_g} ∝ (δ_f + δ_g) split — separable tensor, NOT
 *         Cl(3,0) Clifford (which requires δ_fg).
 *
 * If these signatures change (engine drift → emergent Clifford structure),
 * the test FAILS and the closed-negative claim must be re-audited rather
 * than silently reopened.
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <array>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "ftd/render_bridge.h"
#include "ftd/constants.h"

namespace {

inline int chi(int v_mask, int x, int y, int z) {
    int dot = ((v_mask >> 0) & 1) * x
            + ((v_mask >> 1) & 1) * y
            + ((v_mask >> 2) & 1) * z;
    return (dot & 1) ? -1 : +1;
}

double wh_coef_state(const std::vector<ftd::Voxel>& vox, int L, int v_mask) {
    double sum = 0.0;
    for (int x = 0; x < 2; ++x)
    for (int y = 0; y < 2; ++y)
    for (int z = 0; z < 2; ++z) {
        const int i = x * L * L + y * L + z;
        sum += static_cast<double>(vox[i].state) * static_cast<double>(chi(v_mask, x, y, z));
    }
    return sum / 8.0;
}

double wh_coef_flux(const std::vector<ftd::Voxel>& vox, int L,
                    int component, int v_mask) {
    double sum = 0.0;
    for (int x = 0; x < 2; ++x)
    for (int y = 0; y < 2; ++y)
    for (int z = 0; z < 2; ++z) {
        const int i = x * L * L + y * L + z;
        const auto& f = vox[i].flux;
        const double fc = (component == 0) ? f.x
                          : (component == 1) ? f.y : f.z;
        sum += fc * static_cast<double>(chi(v_mask, x, y, z));
    }
    return sum / 8.0;
}

void inject_wh_mode(ftd::RenderBridge& rb, int v_mask, int axis, double A) {
    for (int x = 0; x < 2; ++x)
    for (int y = 0; y < 2; ++y)
    for (int z = 0; z < 2; ++z) {
        const double s = static_cast<double>(chi(v_mask, x, y, z));
        ftd::Vec3 dF{0, 0, 0};
        if (axis == 0) dF.x = A * s;
        if (axis == 1) dF.y = A * s;
        if (axis == 2) dF.z = A * s;
        rb.inject_flux_add(x, y, z, dF);
    }
}

// Run the canonical FTD-0061 protocol: genesis+movement, two WH injections,
// readout the state-field (0-form). Returns 8-coefficient WH spectrum after
// each injection ordering.
struct StateAlgebra {
    // T[fi][gi][v_mask] = WH coefficient on state field after injecting
    // mode fi, then mode gi.
    std::array<std::array<std::array<double, 8>, 3>, 3> T{};
};

StateAlgebra measure_state_anticommutator(int L, double A) {
    StateAlgebra out;
    const std::array<int, 3> w1_mask = { 0b001, 0b010, 0b100 };

    for (int fi = 0; fi < 3; ++fi)
    for (int gi = 0; gi < 3; ++gi) {
        ftd::RenderBridge rb(L);
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;
        rb.toggles.gauss_projection = true;
        rb.toggles.genesis          = true;
        rb.toggles.movement         = true;
        const int lo = (fi < gi) ? fi : gi;
        const int hi = (fi < gi) ? gi : fi;
        rb.seed_rng(0xCD0517D0u + 100u * lo + hi);

        inject_wh_mode(rb, w1_mask[fi], fi, A);
        rb.run(1);
        inject_wh_mode(rb, w1_mask[gi], gi, A);
        rb.run(1);

        const auto& vox = rb.voxels();
        for (int v = 0; v < 8; ++v) {
            out.T[fi][gi][v] = wh_coef_state(vox, L, v);
        }
    }
    return out;
}

bool test_mode_erasure(const StateAlgebra& alg) {
    // FTD-0061 / 0073 mode-erasure signature: the WH IDENTITY component (v=0)
    // of the anticommutator collapses to the SAME constant (~+2) for every
    // pair (i,j) — diagonal AND off-diagonal alike. Weight-1 modes (the ones
    // we'd expect Clifford to populate for diagonal pairs and zero for
    // off-diagonal) are uniformly zero.
    //
    // Clifford would require: diag identity = +2, off-diag identity = 0,
    // diag weight-1 = 0, off-diag weight-1 = +2 on the matching index.
    // FTD's actual signature: identity = +2 everywhere, weight-1 = 0.
    //
    // If the engine drifts to give Clifford structure, identity-mode mass
    // will diverge between diagonal and off-diagonal pairs — that's the
    // regression signal.

    double ident_diag_avg = 0.0, ident_off_avg = 0.0;
    int diag_n = 0, off_n = 0;
    for (int fi = 0; fi < 3; ++fi)
    for (int gi = fi; gi < 3; ++gi) {
        const double ac_ident = alg.T[fi][gi][0] + alg.T[gi][fi][0];
        if (fi == gi) { ident_diag_avg += ac_ident; ++diag_n; }
        else          { ident_off_avg  += ac_ident; ++off_n; }
    }
    ident_diag_avg /= std::max(1, diag_n);
    ident_off_avg  /= std::max(1, off_n);

    // Weight-1 mode total mass (sum over 3 axial weight-1 modes for all 6
    // pair combinations). Should be near zero for the mode-erasure signature.
    double w1_total_mass = 0.0;
    const std::array<int, 3> w1_mask = { 0b001, 0b010, 0b100 };
    for (int fi = 0; fi < 3; ++fi)
    for (int gi = fi; gi < 3; ++gi) {
        for (int axis = 0; axis < 3; ++axis) {
            const int v = w1_mask[axis];
            w1_total_mass += std::abs(alg.T[fi][gi][v] + alg.T[gi][fi][v]);
        }
    }

    std::printf("  CN-1 mode-erasure on 2^3 state field (FTD-0061 / 0073):\n");
    std::printf("    identity-mode anticommutator: diag avg = %+.4f, off-diag avg = %+.4f\n",
                ident_diag_avg, ident_off_avg);
    std::printf("    weight-1-mode total mass     : %.4f (Clifford would be ~6 for off-diag pairs)\n",
                w1_total_mass);

    const double ident_ratio = std::abs(ident_off_avg) / (std::abs(ident_diag_avg) + 1e-9);
    std::printf("    |off|/|diag| ratio on identity mode = %.3f\n", ident_ratio);
    std::printf("    (mode erasure: ratio ~1; Clifford would give ratio ~0)\n");

    // Pass condition: identity ratio is close to 1 (mode erasure intact)
    // AND weight-1 mass is near zero (no Clifford weight-1 structure).
    const bool ident_collapses = (ident_ratio > 0.5 && ident_ratio < 2.0);
    const bool w1_suppressed   = (w1_total_mass < 1.0);
    const bool no_go_holds     = ident_collapses && w1_suppressed;

    if (!no_go_holds) {
        std::printf("    FAIL: mode-erasure signature broken.\n");
        if (!ident_collapses)
            std::printf("          identity-mode ratio %.3f outside [0.5, 2.0]\n", ident_ratio);
        if (!w1_suppressed)
            std::printf("          weight-1 total mass %.3f exceeds 1.0 — Clifford grade emerging\n",
                        w1_total_mass);
        std::printf("          FTD-0061 / 0073 must be re-audited.\n");
        return false;
    }
    std::printf("    PASS: mode-erasure no-go signature intact.\n");
    return true;
}

bool test_flux_separable(int L, double A) {
    // FTD-0074: flux 1-form readout under site-local dynamics produces
    // separable-tensor algebra, NOT Clifford. Off-diagonal anticommutator
    // pairs split mass between the two active axes (≈ -14·δ_f + -14·δ_g)
    // rather than concentrating on identity (Clifford requires δ_fg).
    const std::array<int, 3> w1_mask = { 0b001, 0b010, 0b100 };
    std::array<std::array<std::array<std::array<double, 8>, 3>, 3>, 3> T{};

    for (int fi = 0; fi < 3; ++fi)
    for (int gi = 0; gi < 3; ++gi) {
        ftd::RenderBridge rb(L);
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;
        rb.toggles.gauss_projection = true;
        rb.toggles.genesis          = true;
        rb.toggles.movement         = true;
        const int lo = (fi < gi) ? fi : gi;
        const int hi = (fi < gi) ? gi : fi;
        rb.seed_rng(0xCE0517D0u + 100u * lo + hi);

        inject_wh_mode(rb, w1_mask[fi], fi, A);
        rb.run(1);
        inject_wh_mode(rb, w1_mask[gi], gi, A);
        rb.run(1);

        const auto& vox = rb.voxels();
        for (int axis = 0; axis < 3; ++axis)
        for (int v = 0; v < 8; ++v) {
            T[fi][gi][axis][v] = wh_coef_flux(vox, L, axis, v);
        }
    }

    std::printf("\n  CN-2 flux-link separable algebra (FTD-0074):\n");

    // Check that off-diagonal anticommutator (e.g., {1,2}) has mass on BOTH
    // axes f and g (not just one) — the separable signature.
    int separable_pairs = 0;
    int total_off_pairs = 0;
    for (int fi = 0; fi < 3; ++fi)
    for (int gi = fi + 1; gi < 3; ++gi) {
        ++total_off_pairs;
        // Anticommutator of L_axis on its natural mode for both active axes
        const double ac_fi = T[fi][gi][fi][w1_mask[fi]] + T[gi][fi][fi][w1_mask[fi]];
        const double ac_gi = T[fi][gi][gi][w1_mask[gi]] + T[gi][fi][gi][w1_mask[gi]];
        // Third axis (uninjected) — should be near zero
        const int third = 3 - fi - gi;
        const double ac_third = T[fi][gi][third][w1_mask[third]]
                              + T[gi][fi][third][w1_mask[third]];

        std::printf("    pair (%d,%d): ac on axes %d=%.3f, %d=%.3f, %d (third)=%.3f\n",
                    fi+1, gi+1, fi, ac_fi, gi, ac_gi, third, ac_third);

        // Separable signature: BOTH ac_fi and ac_gi are non-trivial
        // (significant relative to third axis). Clifford signature would
        // require ac_fi ≈ 0 AND ac_gi ≈ 0 (off-diagonal anticommutator
        // vanishes on plaquette grade).
        const bool both_active = std::abs(ac_fi) > 1.0 && std::abs(ac_gi) > 1.0;
        const bool third_suppressed = std::abs(ac_third) < std::abs(ac_fi) * 0.5;
        if (both_active && third_suppressed) ++separable_pairs;
    }
    std::printf("    separable pairs: %d / %d\n", separable_pairs, total_off_pairs);
    const bool pass = (separable_pairs == total_off_pairs);
    if (!pass) {
        std::printf("    FAIL: flux 1-form is no longer separable on axial bilinears.\n");
        std::printf("          FTD-0074 must be re-audited.\n");
        return false;
    }
    std::printf("    PASS: separable-tensor signature intact.\n");
    return true;
}

} // namespace

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("================================================================\n");
    std::printf("  Closed-Negative Regression Guards\n");
    std::printf("================================================================\n");
    std::printf("  Pins the closed-negative claims FTD-0061, 0073, 0074 by\n");
    std::printf("  asserting the engine STILL exhibits the no-go signature.\n");
    std::printf("  If any of these flips to a Clifford signature, the test\n");
    std::printf("  FAILS — flagging that the closed-negative needs re-audit\n");
    std::printf("  rather than silent reopening.\n\n");

    // Note: the FTD-0061/0073 ledger measurement was on GPU. We do NOT force
    // single-threaded CPU here — the no-go signature is intrinsic to the
    // dynamics, not the backend, and we want this test to track the same
    // path the original measurement used.

    const int    L = 8;
    const double A = 10.0;

    int failures = 0;
    if (!test_mode_erasure(measure_state_anticommutator(L, A))) ++failures;
    if (!test_flux_separable(L, A)) ++failures;

    std::printf("\n================================================================\n");
    if (failures == 0) {
        std::printf("  RESULT: all closed-negative signatures intact (PASS)\n");
    } else {
        std::printf("  RESULT: %d closed-negative regression(s) detected\n", failures);
        std::printf("          Engine has drifted; affected ledger rows need re-audit.\n");
    }
    std::printf("================================================================\n");
    return failures == 0 ? 0 : 1;
}
