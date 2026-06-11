// ==========================================================================
//  engine/src/scenarios/s0_seed.cpp
//
//  Group: s0-seed-* (49 scenarios)
//  JS source: engine/web/js/bridge/scenarios/s0-seed-scenarios.js
//
//  Split out of engine/src/scenarios.cpp (ticket S1). The three internal
//  static helpers seed_lepton / dp / tri moved with this group because
//  they are only used by s0-seed-* scenarios.
// ==========================================================================

#include "ftd/scenarios.h"
#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/voxel.h"

#include "_helpers.h"

#include <cmath>

namespace ftd {

using detail::urand;

// (seed_lepton helper removed audit-4 2026-04-28: only callers were
// s0-seed-{electron, muon, tau} which are now canonical in vacuum.cpp.)

// Helper: _dp from JS — particle + radial flux envelope, optional lock.
static void dp(RenderBridge& rb, int cx, int cy, int cz,
               int st, int sp, int co, double sig, double amp, bool lock) {
    IPF(rb, cx, cy, cz, st, sp, (co >= 0) ? co : 0);
    if (lock) LOCK(rb, cx, cy, cz);
    int sn = (st > 0) ? 1 : -1;
    int eR = CEL(3.0 * sig);
    for (int dz2 = -eR; dz2 <= eR; dz2++) for (int dy2 = -eR; dy2 <= eR; dy2++) for (int dx2 = -eR; dx2 <= eR; dx2++) {
        if (dx2 == 0 && dy2 == 0 && dz2 == 0) continue;
        double r22 = dx2*dx2 + dy2*dy2 + dz2*dz2;
        double rr = std::sqrt(r22);
        if (rr > 3.0 * sig) continue;
        double gg = amp * std::exp(-r22 / (2.0 * sig * sig));
        if (gg < 0.001) continue;
        IF(rb, cx+dx2, cy+dy2, cz+dz2, sn*gg*dx2/rr, sn*gg*dy2/rr, sn*gg*dz2/rr);
    }
}

// Helper: _tri from JS — 3-vertex equilateral triangle in xy-plane.
static void tri(RenderBridge& rb, int cx, int cy, int cz,
                const int charges[3], const int colors[3], int rad, bool lock) {
    for (int k = 0; k < 3; k++) {
        double ang = (2.0 * PI * k) / 3.0;
        int qx = RND(cx + rad * std::cos(ang));
        int qy = RND(cy + rad * std::sin(ang));
        dp(rb, qx, qy, cz, charges[k], (k % 2 == 0) ? 1 : -1, colors[k], 2, K_B * 0.5, lock);
    }
}

bool setup_s0_seed_scenario(RenderBridge& rb, const std::string& name) {
    if (name.rfind("s0-seed-", 0) != 0) return false;
    const int    N    = rb.lattice().size();
    const double midF = (N - 1) * 0.5;
    const int    mc   = RND(midF);

    // Audit-4 2026-04-28: s0-seed-{electron, muon, tau, photon} removed —
    // mirrors of s0-vacuum-{electron, muon, tau, photon} which are now canonical.
    // s0-seed-proton-candidate also removed earlier (audit-3).

    // ── Moore Seeds ──
    if (name == "s0-seed-octahedron") {
        IP(rb, mc, mc, mc, -1);
        const int off[6][3] = {{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
        for (int i = 0; i < 6; i++) IP(rb, mc+off[i][0], mc+off[i][1], mc+off[i][2], +1);
    }
    else if (name == "s0-seed-cuboctahedron") {
        IP(rb, mc, mc, mc, -1);
        const int off[12][3] = {
            {1,1,0},{1,-1,0},{-1,1,0},{-1,-1,0},
            {1,0,1},{1,0,-1},{-1,0,1},{-1,0,-1},
            {0,1,1},{0,1,-1},{0,-1,1},{0,-1,-1}
        };
        for (int i = 0; i < 12; i++) IP(rb, mc+off[i][0], mc+off[i][1], mc+off[i][2], +1);
    }
    else if (name == "s0-seed-stella-octangula") {
        IP(rb, mc, mc, mc, -1);
        const int off[8][3] = {
            {1,1,1},{1,1,-1},{1,-1,1},{1,-1,-1},
            {-1,1,1},{-1,1,-1},{-1,-1,1},{-1,-1,-1}
        };
        for (int i = 0; i < 8; i++) IP(rb, mc+off[i][0], mc+off[i][1], mc+off[i][2], +1);
    }
    else if (name == "s0-seed-moore-cell") {
        // genesis=false (audit-2 2026-04-28): the 27-site geometric seed
        // should stay a 27-site seed. Mirrors JS s0-seed-moore-cell.
        rb.toggles.genesis = false;
        IP(rb, mc, mc, mc, -1);
        for (int dx = -1; dx <= 1; dx++) for (int dy = -1; dy <= 1; dy++) for (int dz = -1; dz <= 1; dz++) {
            if (dx == 0 && dy == 0 && dz == 0) continue;
            IP(rb, mc+dx, mc+dy, mc+dz, +1);
        }
    }
    else if (name == "s0-seed-emergent-ic1") {
        // FTD-0102 / FTD-0107 ic1 (point injection).
        // Inject 10·K_GENESIS flux at lattice center; under the right
        // toggles (genesis + langevin + gauss_projection + wave_propagation),
        // the dynamics produce the emergent 25-voxel L¹-ball-radius-2
        // octahedral bound state. See:
        //   docs/theory/10_eft_program/ANALYSIS_EMERGENT_SPECTRUM_G1.md
        //   docs/theory/08_structural/EXPLR_25_VOXEL_CLUSTER_GEOMETRY.md
        //   docs/theory/08_structural/EXPLR_OCTAHEDRAL_BOUND_STATES.md
        //
        // This scenario sets the required toggles directly so the scenario
        // is self-contained when invoked from the WASM bridge or tests.
        rb.toggles.wave_propagation = true;
        rb.toggles.gauss_projection = true;
        rb.toggles.genesis          = true;
        rb.toggles.langevin         = true;
        rb.toggles.langevin_T       = 0.005;
        rb.toggles.langevin_gamma   = 0.02;
        rb.toggles.dual_substrate   = false;
        IF(rb, mc, mc, mc, 10.0 * K_GENESIS, 0, 0);
    }
    else if (name == "s0-seed-emergent-ic3-collision") {
        // FTD-0102 / FTD-0107 ic3 (two-beam collision).
        // Two opposing flux beams at ±L/4 from centre on the x-axis
        // produce 2 stable bound states of 2-3 voxels each at the
        // collision points. Reproduced 5/5 seeds at L=32 and L=64
        // post-fix (RTX 5090, 2026-04-27).
        rb.toggles.wave_propagation = true;
        rb.toggles.gauss_projection = true;
        rb.toggles.genesis          = true;
        rb.toggles.langevin         = true;
        rb.toggles.langevin_T       = 0.005;
        rb.toggles.langevin_gamma   = 0.02;
        rb.toggles.dual_substrate   = false;
        const int q = N / 4;
        IF(rb, mc - q, mc, mc, +5.0 * K_GENESIS, 0, 0);
        IF(rb, mc + q, mc, mc, -5.0 * K_GENESIS, 0, 0);
    }
    else if (name == "s0-seed-emergent-ic4-subthreshold") {
        // FTD-0102 / FTD-0107 ic4 (sub-threshold injection).
        // 0.5·K_GENESIS at lattice centre — below the K_GENESIS gap.
        // Pre-registered Outcome: 0 manifested voxels across 5/5 seeds
        // (negative control demonstrating the genesis threshold).
        rb.toggles.wave_propagation = true;
        rb.toggles.gauss_projection = true;
        rb.toggles.genesis          = true;
        rb.toggles.langevin         = true;
        rb.toggles.langevin_T       = 0.005;
        rb.toggles.langevin_gamma   = 0.02;
        rb.toggles.dual_substrate   = false;
        IF(rb, mc, mc, mc, 0.5 * K_GENESIS, 0, 0);
    }
    else if (name == "s0-seed-emergent-ic2-thermal-runaway") {
        // FTD-0102 / FTD-0107 ic2 (thermal-driven runaway).
        // No flux injection — only elevated Langevin T = 0.05 (10× the
        // standard ic1/ic3 setting). Demonstrates the unstable-phase
        // regime where pure thermal noise drives runaway genesis.
        // The L=32 seed-4 finite-size escape observed in the post-fix
        // re-measurement lives in this phase-space neighbourhood.
        rb.toggles.wave_propagation = true;
        rb.toggles.gauss_projection = true;
        rb.toggles.genesis          = true;
        rb.toggles.langevin         = true;
        rb.toggles.langevin_T       = 0.05;   // 10× ic1
        rb.toggles.langevin_gamma   = 0.02;
        rb.toggles.dual_substrate   = false;
        // No IF call — thermal noise alone drives the dynamics.
    }
    else if (name == "s0-seed-emergent-ic1-diagonal") {
        // FTD-0110 D3g: body-diagonal injection.
        // Same total amplitude as ic1 (10·K_GENESIS) but distributed along
        // (1,1,1)/√3 instead of +x. The 3-fold rotation about the body
        // diagonal is Z_3 (not Z_4); if the cluster-efficiency ¼ comes from
        // the i-cycle Z_4 about the injection axis, then a body-diagonal
        // injection should give k ≈ 1/3 instead of ¼ — and a cluster size
        // of (1/3)·A² ≈ 33 voxels at A=10 (vs 25 for axial).
        // If k stays at ¼, the structural origin is global (N_base, not
        // axis-specific Z_4).
        rb.toggles.wave_propagation = true;
        rb.toggles.gauss_projection = true;
        rb.toggles.genesis          = true;
        rb.toggles.langevin         = true;
        rb.toggles.langevin_T       = 0.005;
        rb.toggles.langevin_gamma   = 0.02;
        rb.toggles.dual_substrate   = false;
        const double inv_sqrt3 = 1.0 / std::sqrt(3.0);
        const double A = 10.0 * K_GENESIS * inv_sqrt3;
        IF(rb, mc, mc, mc, A, A, A);
    }
    else if (name == "s0-seed-emergent-ic1-isotropic") {
        // FTD-0110 D3h: isotropic 6-axis injection at the canonical
        // ic1 amplitude. Decomposes A·K_GENESIS uniformly across the
        // 6 SC face-neighbour directions of the centre voxel; the
        // resulting bound state should be O_h-symmetric under all
        // cube rotations (no injection-direction breaking the +x/−x
        // asymmetry seen in s0-seed-emergent-ic1).
        rb.toggles.wave_propagation = true;
        rb.toggles.gauss_projection = true;
        rb.toggles.genesis          = true;
        rb.toggles.langevin         = true;
        rb.toggles.langevin_T       = 0.005;
        rb.toggles.langevin_gamma   = 0.02;
        rb.toggles.dual_substrate   = false;
        // Distribute 10·K_GENESIS magnitude across 6 directions: each
        // of the 6 face neighbours of the centre receives a flux pointing
        // outward from the centre with magnitude (10/√6)·K_GENESIS
        // (so |J|² summed across all 6 voxels = 10²·K_GENESIS² as in ic1).
        const double inv_sqrt6 = 1.0 / std::sqrt(6.0);
        const double a = 10.0 * K_GENESIS * inv_sqrt6;
        IF(rb, mc + 1, mc, mc, +a, 0, 0);
        IF(rb, mc - 1, mc, mc, -a, 0, 0);
        IF(rb, mc, mc + 1, mc, 0, +a, 0);
        IF(rb, mc, mc - 1, mc, 0, -a, 0);
        IF(rb, mc, mc, mc + 1, 0, 0, +a);
        IF(rb, mc, mc, mc - 1, 0, 0, -a);
    }
    else if (name == "s0-seed-emergent-ic1-viz") {
        // Clean visualisation of the axial ic1 cluster (dashboard demo).
        // Uses A=20·K_GENESIS instead of the campaign A=10 to compensate
        // for the CPU genesis-drain that suppresses cluster growth in
        // single-threaded WASM (vs GPU's no-drain behaviour). T=0 disables
        // Langevin thermal driving so the cluster is NOT obscured by
        // background thermal genesis. Run ~200 ticks for clearest view.
        rb.toggles.wave_propagation = true;
        rb.toggles.gauss_projection = true;
        rb.toggles.genesis          = true;
        rb.toggles.langevin         = true;
        rb.toggles.langevin_T       = 0.0;
        rb.toggles.langevin_gamma   = 0.02;
        rb.toggles.dual_substrate   = false;
        IF(rb, mc, mc, mc, 20.0 * K_GENESIS, 0, 0);
    }
    else if (name == "s0-seed-cluster-law") {
        // FTD-0269: the genesis-burst N(A) cluster-size law (interactive).
        // Canonical ic1 stack + A=10 point injection (the campaign baseline).
        // The dashboard fire panel re-injects at a user-chosen A to sweep the
        // broken-power law N(A): a geometry-limited cascade inside the 27-block
        // below the knee (A~16), an energy budget N = k_eff*A^2 above it.
        // See docs/theory/.../ANALYSIS_FTD0110_NA_LAW.md (verdict BOUNDARY).
        rb.toggles.wave_propagation = true;
        rb.toggles.gauss_projection = true;
        rb.toggles.genesis          = true;
        rb.toggles.langevin         = true;
        rb.toggles.langevin_T       = 0.005;
        rb.toggles.langevin_gamma   = 0.02;
        rb.toggles.dual_substrate   = false;
        IF(rb, mc, mc, mc, 10.0 * K_GENESIS, 0, 0);
    }
    else if (name == "s0-seed-cluster-law-subknee") {
        // FTD-0269 answer-key: sub-knee regime (A=12). Compact 27-block
        // cascade; GPU campaign cluster ~ 8 voxels (FTD-0261). T=0 for a clean
        // deterministic view (no thermal background).
        rb.toggles.wave_propagation = true;
        rb.toggles.gauss_projection = true;
        rb.toggles.genesis          = true;
        rb.toggles.langevin         = true;
        rb.toggles.langevin_T       = 0.0;
        rb.toggles.langevin_gamma   = 0.02;
        rb.toggles.dual_substrate   = false;
        IF(rb, mc, mc, mc, 12.0 * K_GENESIS, 0, 0);
    }
    else if (name == "s0-seed-cluster-law-knee") {
        // FTD-0269 answer-key: the knee (A=16) — the 27-block escape where the
        // geometry-limited cascade hands off to the energy budget. GPU campaign
        // cluster ~ 21 voxels (FTD-0261). T=0.
        rb.toggles.wave_propagation = true;
        rb.toggles.gauss_projection = true;
        rb.toggles.genesis          = true;
        rb.toggles.langevin         = true;
        rb.toggles.langevin_T       = 0.0;
        rb.toggles.langevin_gamma   = 0.02;
        rb.toggles.dual_substrate   = false;
        IF(rb, mc, mc, mc, 16.0 * K_GENESIS, 0, 0);
    }
    else if (name == "s0-seed-cluster-law-superknee") {
        // FTD-0269 answer-key: super-knee regime (A=40) — bulk-lattice volume
        // expansion, N = k_eff*A^2. GPU campaign cluster ~ 92 voxels (FTD-0261).
        // T=0.
        rb.toggles.wave_propagation = true;
        rb.toggles.gauss_projection = true;
        rb.toggles.genesis          = true;
        rb.toggles.langevin         = true;
        rb.toggles.langevin_T       = 0.0;
        rb.toggles.langevin_gamma   = 0.02;
        rb.toggles.dual_substrate   = false;
        IF(rb, mc, mc, mc, 40.0 * K_GENESIS, 0, 0);
    }
    else if (name == "s0-seed-emergent-ic1-diagonal-viz") {
        // Clean body-diagonal cluster (D3g shape comparison).
        rb.toggles.wave_propagation = true;
        rb.toggles.gauss_projection = true;
        rb.toggles.genesis          = true;
        rb.toggles.langevin         = true;
        rb.toggles.langevin_T       = 0.0;
        rb.toggles.langevin_gamma   = 0.02;
        rb.toggles.dual_substrate   = false;
        const double inv_sqrt3 = 1.0 / std::sqrt(3.0);
        const double A = 20.0 * K_GENESIS * inv_sqrt3;
        IF(rb, mc, mc, mc, A, A, A);
    }
    else if (name == "s0-seed-emergent-ic1-isotropic-viz") {
        // Clean isotropic 6-axis injection (D3h full O_h symmetry view).
        rb.toggles.wave_propagation = true;
        rb.toggles.gauss_projection = true;
        rb.toggles.genesis          = true;
        rb.toggles.langevin         = true;
        rb.toggles.langevin_T       = 0.0;
        rb.toggles.langevin_gamma   = 0.02;
        rb.toggles.dual_substrate   = false;
        const double inv_sqrt6 = 1.0 / std::sqrt(6.0);
        const double a = 20.0 * K_GENESIS * inv_sqrt6;
        IF(rb, mc + 1, mc, mc, +a, 0, 0);
        IF(rb, mc - 1, mc, mc, -a, 0, 0);
        IF(rb, mc, mc + 1, mc, 0, +a, 0);
        IF(rb, mc, mc - 1, mc, 0, -a, 0);
        IF(rb, mc, mc, mc + 1, 0, 0, +a);
        IF(rb, mc, mc, mc - 1, 0, 0, -a);
    }
    // s0-seed-symmetry-regression removed 2026-04-28 (audit removal): engine
    // CI artefact (voxel_uniform() RNG determinism check), not a user-facing
    // physics scenario. Fold into a ctest under engine/tests/ if still needed.

    else if (name == "s0-seed-moore-decomposition") {
        IP(rb, mc, mc, mc, -1);
        const int oct[6][3] = {{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
        for (int i = 0; i < 6; i++) IP(rb, mc+oct[i][0], mc+oct[i][1], mc+oct[i][2], +1);
        const int cub[12][3] = {
            {1,1,0},{1,-1,0},{-1,1,0},{-1,-1,0},
            {1,0,1},{1,0,-1},{-1,0,1},{-1,0,-1},
            {0,1,1},{0,1,-1},{0,-1,1},{0,-1,-1}
        };
        for (int i = 0; i < 12; i++) IP(rb, mc+cub[i][0], mc+cub[i][1], mc+cub[i][2], -1);
        const int stel[8][3] = {
            {1,1,1},{1,1,-1},{1,-1,1},{1,-1,-1},
            {-1,1,1},{-1,1,-1},{-1,-1,1},{-1,-1,-1}
        };
        for (int i = 0; i < 8; i++) IP(rb, mc+stel[i][0], mc+stel[i][1], mc+stel[i][2], +1);
    }
    // ── Composite seeds via dp/tri helpers ──
    // Audit-3 2026-04-28: removed electron-l3, neutrino, quark, antiquark.
    // Audit-4 2026-04-28: removed positron, pion, proton-l4, neutron — all
    // now canonical in vacuum.cpp (s0-vacuum-*).
    else if (name == "s0-seed-hydrogen") {
        const int oR = std::max(4, N / 6);
        const int bR = std::max(2, N / 12);
        const int charges[3] = {+1, +1, -1};
        const int colors[3]  = {1, 2, 3};
        tri(rb, mc, mc, mc, charges, colors, bR, true);
        dp(rb, mc, mc, mc + oR, -1, -1, 0, 2, K_B, false);
    }
    else if (name == "s0-seed-helium") {
        // ⁴He / α-particle (audit 2026-04-28 fix): 2 protons + 2 neutrons
        // at tetrahedral vertices + 2 electrons in 1s² shell. Each nucleon
        // is a 3-quark triad. Mirrors JS s0-seed-helium body.
        const int oR = std::max(3, N / 8);
        const int nR = std::max(2, N / 12);
        const int bR = std::max(1, N / 16);
        const int tet[4][3] = {
            { +nR, +nR, +nR },   // proton 1
            { -nR, -nR, +nR },   // proton 2
            { +nR, -nR, -nR },   // neutron 1
            { -nR, +nR, -nR },   // neutron 2
        };
        const int pCharges[3] = { +1, +1, -1 };
        const int nCharges[3] = { +1, -1, -1 };
        const int colors[3]   = { 1, 2, 3 };
        for (int i = 0; i < 4; ++i) {
            const int* charges = (i < 2) ? pCharges : nCharges;
            tri(rb, mc + tet[i][0], mc + tet[i][1], mc + tet[i][2],
                charges, colors, bR, true);
        }
        dp(rb, mc, mc, mc + oR, -1, +1, 0, 2, K_B * 0.8, false);
        dp(rb, mc, mc, mc - oR, -1, -1, 0, 2, K_B * 0.8, false);
    }
    else if (name == "s0-seed-h2-bond-formation") {
        const int bd = std::max(4, N / 6);
        const int hf = bd / 2;
        const int bR = std::max(1, N / 16);
        const int charges[3] = {+1, +1, -1};
        const int colors[3]  = {1, 2, 3};
        tri(rb, mc - RND(hf * 0.7), mc, mc, charges, colors, bR, true);
        tri(rb, mc + RND(hf * 0.7), mc, mc, charges, colors, bR, true);
        dp(rb, mc, mc, mc + 1, -1, -1, 0, 2, K_B * 0.8, false);
        dp(rb, mc, mc, mc - 1, -1, +1, 0, 2, K_B * 0.8, false);
    }
    else if (name == "s0-seed-spark-of-life") {
        // Honest demo: a nonliving mineral-pore-like boundary, simple
        // precursor charge pairs, and a flux spark that can cross the
        // existing K_GENESIS manifestation threshold.
        const int ringR = std::max(5, N / 8);
        constexpr int ringSites = 16;
        for (int i = 0; i < ringSites; ++i) {
            const double angle = (2.0 * PI * i) / ringSites;
            const int px = RND(mc + ringR * std::cos(angle));
            const int py = RND(mc + ringR * std::sin(angle));
            const int state = (i % 2 == 0) ? +1 : -1;
            IPF(rb, px, py, mc, state, state, 0);
            LOCK(rb, px, py, mc);
        }

        // Four incoming precursor charge pairs, each dressed with
        // sub-threshold flux and nudged inward toward the pore.
        const int precursorR = std::max(ringR + 4, N / 4);
        const double precursorSpeed = 0.12 * C_SPEED;
        for (int k = 0; k < 4; ++k) {
            const double angle = (2.0 * PI * k) / 4.0;
            const double dirX = std::cos(angle);
            const double dirY = std::sin(angle);
            const double tanX = -dirY;
            const double tanY = dirX;
            for (int j = 0; j < 2; ++j) {
                const int side = (j == 0) ? -1 : +1;
                const int state = (j == 0) ? +1 : -1;
                const int px = RND(mc + precursorR * dirX + side * tanX);
                const int py = RND(mc + precursorR * dirY + side * tanY);
                dp(rb, px, py, mc, state, state, ((k + j) % 3) + 1,
                   1.6, K_B * 0.7, false);
                SET_VEL(rb, px, py, mc, -dirX * precursorSpeed,
                        -dirY * precursorSpeed, 0);
            }
        }

        // Central unlocked catalytic triad: color-labelled, not locked, so
        // the ordinary Scale-0 dynamics own its fate.
        const int triR = std::max(2, N / 18);
        const int catalystCharges[3] = {+1, -1, +1};
        const int catalystColors[3] = {1, 2, 3};
        tri(rb, mc, mc, mc, catalystCharges, catalystColors, triR, false);

        // Six-axis deterministic spark. Per-axis amplitude is chosen so
        // the vector-budget scale is 6*K_GENESIS.
        const double spark = 6.0 * K_GENESIS / std::sqrt(6.0);
        IF(rb, mc + 1, mc, mc, +spark, 0, 0);
        IF(rb, mc - 1, mc, mc, -spark, 0, 0);
        IF(rb, mc, mc + 1, mc, 0, +spark, 0);
        IF(rb, mc, mc - 1, mc, 0, -spark, 0);
        IF(rb, mc, mc, mc + 1, 0, 0, +spark);
        IF(rb, mc, mc, mc - 1, 0, 0, -spark);

        // Two daughter pockets stay below K_GENESIS at setup; if they
        // bloom, it is because dynamics feed them.
        auto daughter_pocket = [&](int cx, int cy, int cz, int sign) {
            constexpr int radius = 4;
            constexpr double sigma = 2.0;
            const double amp = 0.75 * K_GENESIS;
            for (int dz = -radius; dz <= radius; ++dz)
            for (int dy = -radius; dy <= radius; ++dy)
            for (int dx = -radius; dx <= radius; ++dx) {
                const int x = cx + dx;
                const int y = cy + dy;
                const int z = cz + dz;
                if (x < 0 || x >= N || y < 0 || y >= N || z < 0 || z >= N) continue;
                const int r2i = dx*dx + dy*dy + dz*dz;
                if (r2i == 0 || r2i > radius * radius) continue;
                const double r2 = static_cast<double>(r2i);
                const double r = std::sqrt(r2);
                const double val = amp * std::exp(-r2 / (2.0 * sigma * sigma));
                if (val < 0.001) continue;
                IF(rb, x, y, z,
                   sign * val * dx / r,
                   sign * val * dy / r,
                   sign * val * dz / r);
            }
        };
        const int daughterR = std::max(ringR + 2, N / 6);
        daughter_pocket(mc - daughterR, mc, mc + 2, +1);
        daughter_pocket(mc + daughterR, mc, mc - 2, -1);
    }
    // ── Quarks (LHC additions) ──
    else if (name == "s0-seed-up-quark" || name == "s0-seed-down-quark" ||
             name == "s0-seed-strange-quark" || name == "s0-seed-charm-quark" ||
             name == "s0-seed-bottom-quark" || name == "s0-seed-top-quark") {
        int charge, color;
        double ampBoost;
        if      (name == "s0-seed-up-quark")      { charge = +1; color = 1; ampBoost = 0.5; }
        else if (name == "s0-seed-down-quark")    { charge = -1; color = 2; ampBoost = 0.5; }
        else if (name == "s0-seed-strange-quark") { charge = -1; color = 3; ampBoost = 0.7; }
        else if (name == "s0-seed-charm-quark")   { charge = +1; color = 1; ampBoost = 1.0; }
        else if (name == "s0-seed-bottom-quark")  { charge = -1; color = 2; ampBoost = 1.4; }
        else                                        { charge = +1; color = 3; ampBoost = 2.5; }
        IPF(rb, mc, mc, mc, charge, (charge > 0) ? +1 : -1, color);
        const double qSig = 1.5;
        const int qR = 4;
        const double qAmp = K_B * ampBoost;
        for (int dz = -qR; dz <= qR; dz++) for (int dy = -qR; dy <= qR; dy++) for (int dx = -qR; dx <= qR; dx++) {
            int r2 = dx*dx + dy*dy + dz*dz;
            if (r2 == 0 || r2 > qR * qR) continue;
            double r = std::sqrt(double(r2));
            double g = qAmp * std::exp(-r2 / (2.0 * qSig * qSig));
            if (g < 1e-3) continue;
            int sign = (charge > 0) ? 1 : -1;
            double axisBias[3] = {0, 0, 0};
            axisBias[color - 1] = 0.5;
            IF(rb, mc + dx, mc + dy, mc + dz,
               sign * g * (dx / r + axisBias[0]),
               sign * g * (dy / r + axisBias[1]),
               sign * g * (dz / r + axisBias[2]));
        }
    }
    // ── Electroweak bosons + Higgs + gluon ──
    // Audit-4 2026-04-28: s0-seed-higgs-boson removed (mirror of s0-vacuum-higgs).
    else if (name == "s0-seed-higgs-field") {
        const double vevAmp = K_B * 0.3;
        const double noise  = K_B * 0.05;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double sx = std::sin(0.19*x + 0.23*y + 0.29*z);
            double sy = std::sin(0.37*x + 0.13*y + 0.17*z);
            double sz = std::sin(0.11*x + 0.31*y + 0.41*z);
            IF(rb, x, y, z, vevAmp + noise*sx, vevAmp + noise*sy, vevAmp + noise*sz);
        }
    }
    // Audit-4 2026-04-28: s0-seed-{w-boson, z-boson} removed —
    // mirrors of s0-vacuum-{w-boson, z-boson} which are now canonical.
    else if (name == "s0-seed-gluon") {
        // genesis=false (audit 2026-04-28): same fix as photon.
        rb.toggles.genesis = false;
        const int sigma = 3;
        const double gAmp = K_B * 2.0;
        const int startX = std::max(4, N / 4);
        const int halfR = 8;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int dx = -halfR; dx <= halfR; dx++) {
            int x = startX + dx;
            if (x < 0 || x >= N) continue;
            double dy = y - midF, dz = z - midF;
            double gg = gAmp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * sigma * sigma));
            if (gg < 1e-6) continue;
            IF(rb, x, y, z, 0, gg, 0);
            IW(rb, x, y, z, gg, 0, 0);
        }
    }
    // ── Process demos ──
    else if (name == "s0-seed-beta-decay") {
        // Audit 2026-04-28: auto-enable required toggles so the decay
        // actually fires by default (mirrors JS s0-seed-beta-decay).
        rb.toggles.weak_transmutation = true;
        rb.toggles.dual_substrate = true;
        const int bdR = std::max(2, N / 10);
        for (int k = 0; k < 3; k++) {
            double ang = (2.0 * PI * k) / 3.0;
            int bx = RND(mc + bdR * std::cos(ang));
            int by = RND(mc + bdR * std::sin(ang));
            int charge = (k == 0) ? +1 : -1;
            IP(rb, bx, by, mc, charge);
        }
        const int leptonR = std::max(4, N / 5);
        IP(rb, mc, mc, mc + leptonR, -1);
        const int nuSig = 2, nuR = 4;
        for (int dz2 = -nuR; dz2 <= nuR; dz2++) for (int dy2 = -nuR; dy2 <= nuR; dy2++) for (int dx2 = -nuR; dx2 <= nuR; dx2++) {
            int r22 = dx2*dx2 + dy2*dy2 + dz2*dz2;
            if (r22 > nuR * nuR) continue;
            double g = K_B * 0.3 * std::exp(-r22 / (2.0 * nuSig * nuSig));
            if (g < 1e-3) continue;
            IF(rb, mc+dx2, mc-leptonR+dy2, mc+dz2, g*0.55, g*0.45, 0);
        }
        rb.toggles.weak_transmutation = true;
        rb.toggles.dual_substrate = true;
    }
    else if (name == "s0-seed-ee-annihilation") {
        const int aSep = std::max(6, N / 3);
        const int half = aSep / 2;
        IP(rb, mc - half, mc, mc, -1);
        SET_VEL(rb, mc - half, mc, mc, +0.3 * C_SPEED, 0, 0);
        IP(rb, mc + half, mc, mc, +1);
        SET_VEL(rb, mc + half, mc, mc, -0.3 * C_SPEED, 0, 0);
        const int aSig = 2, aR = 4;
        for (int pass = 0; pass < 2; pass++) {
            int cx = (pass == 0) ? mc - half : mc + half;
            int sign = (pass == 0) ? -1 : +1;
            for (int dz2 = -aR; dz2 <= aR; dz2++) for (int dy2 = -aR; dy2 <= aR; dy2++) for (int dx2 = -aR; dx2 <= aR; dx2++) {
                int r2 = dx2*dx2 + dy2*dy2 + dz2*dz2;
                if (r2 == 0 || r2 > aR * aR) continue;
                double r = std::sqrt(double(r2));
                double g = K_B * std::exp(-r2 / (2.0 * aSig * aSig));
                if (g < 1e-3) continue;
                IF(rb, cx+dx2, mc+dy2, mc+dz2, sign*g*dx2/r, sign*g*dy2/r, sign*g*dz2/r);
            }
        }
    }
    else if (name == "s0-seed-quark-gluon-plasma") {
        const int qOffset = 2;
        const int dirs[2] = {-qOffset, qOffset};
        int quarkIndex = 0;
        for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
        for (int k = 0; k < 2; k++) {
            int dx = dirs[i], dy = dirs[j], dz = dirs[k];
            const int charge = (quarkIndex % 2 == 0) ? +1 : -1;
            const int color = (quarkIndex % 3) + 1; // R=1, G=2, B=3
            IPF(rb, mc + dx, mc + dy, mc + dz, charge, (charge > 0) ? +1 : -1, color);

            // High thermal random velocity, speed = 0.5 * C_SPEED
            const double theta = urand() * 2.0 * PI;
            const double phi = std::acos(urand() * 2.0 - 1.0);
            const double speed = 0.5 * C_SPEED;
            SET_VEL(rb, mc + dx, mc + dy, mc + dz,
                    speed * std::sin(phi) * std::cos(theta),
                    speed * std::sin(phi) * std::sin(theta),
                    speed * std::cos(phi));

            quarkIndex++;
        }

        // Seed random high-energy gluon flux pulses in a central 8x8x8 region:
        const int pulseR = 4;
        for (int dz = -pulseR; dz <= pulseR; dz++)
        for (int dy = -pulseR; dy <= pulseR; dy++)
        for (int dx = -pulseR; dx <= pulseR; dx++) {
            const int r2 = dx * dx + dy * dy + dz * dz;
            if (r2 > pulseR * pulseR) continue;

            const double amp = K_B * 3.0 * urand();
            const double theta = urand() * 2.0 * PI;
            const double phi = std::acos(urand() * 2.0 - 1.0);

            const double jx = amp * std::sin(phi) * std::cos(theta);
            const double jy = amp * std::sin(phi) * std::sin(theta);
            const double jz = amp * std::cos(phi);

            const double wx = amp * std::sin(phi) * std::cos(theta) * C_SPEED;
            const double wy = amp * std::sin(phi) * std::sin(theta) * C_SPEED;
            const double wz = amp * std::cos(phi) * C_SPEED;

            IF(rb, mc + dx, mc + dy, mc + dz, jx, jy, jz);
            IW(rb, mc + dx, mc + dy, mc + dz, wx, wy, wz);
        }
    }
    else if (name == "s0-seed-gravitational-lensing") {
        // Schwarzschild well at the center:
        const double sHalf = midF, rs = 3.0;
        IP(rb, mc, mc, mc, +1);
        for (int z = 0; z < N; z++)
        for (int y = 0; y < N; y++)
        for (int x = 0; x < N; x++) {
            const double rx = x - sHalf, ry = y - sHalf, rz = z - sHalf;
            const double r = std::max(std::sqrt(rx * rx + ry * ry + rz * rz), 0.5);
            const double mg = G_N * (K_B * rs) / (r * r);
            if (mg < 1e-6) continue;
            IF(rb, x, y, z, -mg * rx / r, -mg * ry / r, -mg * rz / r);
        }

        // Off-axis photon pulse launched at x0 = N/4, propagating in +x:
        const int x0 = N / 4;
        const int offset = std::max(4, N / 6);
        const int y0 = mc + offset;
        const int z0 = mc;

        const int sigma = std::max(2, N / 12);
        const double amp = K_B * 3.0;
        const double lambdaEff = 4.0 * sigma;
        const double k_val = 2.0 * PI / lambdaEff;
        const double cutR = 3.0 * sigma;
        const double cutR2 = cutR * cutR;

        for (int z = 0; z < N; z++)
        for (int y = 0; y < N; y++)
        for (int x = 0; x < N; x++) {
            const int dx = x - x0, dy = y - y0, dz = z - z0;
            const int r2 = dx * dx + dy * dy + dz * dz;
            if (r2 > cutR2) continue;
            double g = std::exp(-r2 / (2.0 * sigma * sigma));
            if (g < 1e-6) continue;
            double phase = k_val * dx;
            double jz = amp * g * std::sin(phase);
            double wz = amp * g * std::cos(phase) * C_SPEED;
            IF(rb, x, y, z, 0, 0, jz);
            IW(rb, x, y, z, wz, 0, 0);
        }
    }
    // ── Level 6: Gauge / Topological ──
    else if (name == "s0-seed-wilson-loop") {
        const int R = std::max(3, N / 8);
        const double wAmp = K_B;
        for (int x = mc - R; x <= mc + R; x++) IF(rb, x, mc - R, mc,  wAmp, 0, 0);
        for (int y = mc - R; y <= mc + R; y++) IF(rb, mc + R, y, mc, 0,  wAmp, 0);
        for (int x = mc + R; x >= mc - R; x--) IF(rb, x, mc + R, mc, -wAmp, 0, 0);
        for (int y = mc + R; y >= mc - R; y--) IF(rb, mc - R, y, mc, 0, -wAmp, 0);
        IP(rb, mc-R, mc-R, mc, +1); IP(rb, mc+R, mc-R, mc, +1);
        IP(rb, mc+R, mc+R, mc, +1); IP(rb, mc-R, mc+R, mc, +1);
    }
    else if (name == "s0-seed-flux-tube") {
        const int ftSep = std::max(6, N / 4), ftH = ftSep / 2;
        IP(rb, mc - ftH, mc, mc, +1);
        IP(rb, mc + ftH, mc, mc, -1);
        const double ftSig = 1.5;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = mc - ftH; x <= mc + ftH; x++) {
            double dy2 = y - mc, dz2 = z - mc;
            double p2 = dy2*dy2 + dz2*dz2;
            double g = K_B * std::exp(-p2 / (2.0 * ftSig * ftSig));
            if (g > 0.001) IF(rb, x, y, z, g, 0, 0);
        }
    }
    else if (name == "s0-seed-monopole") {
        const double mHalf = (N - 1) / 2.0;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double rx = x - mHalf, ry = y - mHalf, rz = z - mHalf;
            double r = std::sqrt(rx*rx + ry*ry + rz*rz);
            if (r < 1.0) r = 1.0;
            double mg = 1.0 / (4.0 * PI * r * r);
            if (mg < 1e-6) continue;
            double rxy = std::sqrt(rx*rx + ry*ry);
            if (rxy < 0.5) { IF(rb, x, y, z, 0, 0, mg); continue; }
            IF(rb, x, y, z, -ry / rxy * mg, rx / rxy * mg, 0);
        }
    }
    else if (name == "s0-seed-instanton") {
        const double iSize = 3.0, iHalf = (N - 1) / 2.0;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double rx = x - iHalf, ry = y - iHalf, rz = z - iHalf;
            double r2 = rx*rx + ry*ry + rz*rz;
            double r = std::sqrt(r2);
            double mg = iSize / (r2 + iSize * iSize);
            if (mg < 1e-6 || r < 0.5) continue;
            IF(rb, x, y, z, mg * rx / r, mg * ry / r, mg * rz / r);
        }
    }
    // ── Level 7: Gravity / Cosmology ──
    else if (name == "s0-seed-schwarzschild") {
        const double sHalf = (N - 1) / 2.0, rs = 3.0;
        IP(rb, mc, mc, mc, +1);
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double rx = x - sHalf, ry = y - sHalf, rz = z - sHalf;
            double r = std::sqrt(rx*rx + ry*ry + rz*rz);
            if (r < 0.5) r = 0.5;
            double mg = K_B * rs / (r * r);
            if (mg < 1e-6) continue;
            IF(rb, x, y, z, -mg * rx / r, -mg * ry / r, -mg * rz / r);
        }
    }
    else if (name == "s0-seed-massive-body") {
        // A dense ball of LOCKED rest mass. Gravity is sourced from REAL manifested
        // mass (rho_mass = M_REST*|state|) by the latency-Poisson solver (enable
        // latency_field), NOT the |J|^2 field-energy proxy. Locked => static body
        // (skipped by movement + evaporation), so it is a stable gravitational source.
        const double sHalf = (N - 1) / 2.0;
        // Small compact body so the latency well is sub-horizon with a visible
        // 1/r tail (a dense large ball saturates to a black hole — that's the
        // schwarzschild scenario). ~33 voxels at L=33 -> latencyMax ~0.5.
        const int R = std::min(2, std::max(1, N / 16));
        const double R2 = static_cast<double>(R) * R;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double rx = x - sHalf, ry = y - sHalf, rz = z - sHalf;
            if (rx*rx + ry*ry + rz*rz > R2) continue;
            IP(rb, x, y, z, +1);
            LOCK(rb, x, y, z);
        }
    }
    else if (name == "s0-seed-gravitational-wave") {
        const int gwWl = std::max(4, N / 4);
        const double gwK = 2.0 * PI / gwWl, gwAmp = 0.1;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double v = gwAmp * std::sin(gwK * x);
            if (std::fabs(v) > 1e-6) IF(rb, x, y, z, 0, v, 0);
        }
    }
    // ── Time-dilation scenarios (2026-06-07) ──
    // Thin reuse mirrors for the Time Observatory panel. Each reproduces an
    // existing gravity seed so the latency well (gravitational clock-slowdown)
    // is real; no new physics. The latency SAMPLER builds dτ/dt from the |J|²
    // flux field, so these reuse FLUX-producing wells (a locked-rest-mass body
    // has zero flux → no proxy latency). Kept in sync with the JS delegating
    // cases in engine/web/js/bridge/scenarios/s0-seed-scenarios.js.
    else if (name == "s0-seed-time-gravity-well" || name == "s0-seed-time-twin-clocks") {
        // == s0-seed-gravitational-wave: a sinusoidal flux well that the
        // field-energy latency solver turns into a measurable dτ/dt field.
        const int gwWl = std::max(4, N / 4);
        const double gwK = 2.0 * PI / gwWl, gwAmp = 0.1;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double v = gwAmp * std::sin(gwK * x);
            if (std::fabs(v) > 1e-6) IF(rb, x, y, z, 0, v, 0);
        }
    }
    else if (name == "s0-seed-time-horizon") {
        // == s0-seed-schwarzschild: seed-bias inflow toward a central mass — a
        // stronger (near-horizon) well for the deepest time-dilation demo.
        const double sHalf = (N - 1) / 2.0, rs = 3.0;
        IP(rb, mc, mc, mc, +1);
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double rx = x - sHalf, ry = y - sHalf, rz = z - sHalf;
            double r = std::sqrt(rx*rx + ry*ry + rz*rz);
            if (r < 0.5) r = 0.5;
            double mg = K_B * rs / (r * r);
            if (mg < 1e-6) continue;
            IF(rb, x, y, z, -mg * rx / r, -mg * ry / r, -mg * rz / r);
        }
    }
    // ── Level 8: Reference frame context / Observer ──
    else if (name == "s0-seed-sloop") {
        const int slR = std::max(3, N / 8);
        const int slN = 12;
        const double slA = K_B;
        for (int i = 0; i < slN; i++) {
            double a = 2.0 * PI * i / slN;
            int px = RND(mc + slR * std::cos(a));
            int py = RND(mc + slR * std::sin(a));
            IP(rb, px, py, mc, +1);
            IF(rb, px, py, mc, -std::sin(a) * slA, std::cos(a) * slA, 0);
        }
    }
    else if (name == "s0-seed-observer-cell") {
        IP(rb, mc, mc, mc, +1);
        const int oct[6][3] = {{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
        for (int i = 0; i < 6; i++) IP(rb, mc+oct[i][0], mc+oct[i][1], mc+oct[i][2], -1);
        const int cub[12][3] = {
            {1,1,0},{1,-1,0},{-1,1,0},{-1,-1,0},
            {1,0,1},{1,0,-1},{-1,0,1},{-1,0,-1},
            {0,1,1},{0,1,-1},{0,-1,1},{0,-1,-1}
        };
        for (int i = 0; i < 12; i++) IP(rb, mc+cub[i][0], mc+cub[i][1], mc+cub[i][2], +1);
        const int stel[8][3] = {
            {1,1,1},{1,1,-1},{1,-1,1},{1,-1,-1},
            {-1,1,1},{-1,1,-1},{-1,-1,1},{-1,-1,-1}
        };
        for (int i = 0; i < 8; i++) IP(rb, mc+stel[i][0], mc+stel[i][1], mc+stel[i][2], -1);
    }
    else if (name == "s0-seed-de-broglie-clock") {
        // FTD-0271: de Broglie internal clock (single-particle pilot wave).
        // A central manifested block carries a uniform flux J0. When the
        // de_broglie_clock toggle is ON (the de-broglie-clock-panel enables it),
        // the Klein-Gordon mass term -omega0^2*J makes the block's flux
        // oscillate at the rest-frame Compton frequency omega0 -- the internal
        // clock. [CONDITIONAL -- DERIVED-GIVEN-IMPOSED-INPUT]: omega0~M_REST is
        // IMPOSED (A0: FTD's native flux is massless); Schrodinger + de Broglie
        // are textbook Klein-Gordon, not an FTD prediction. The block interior
        // is the k=0 rest mode (uniform => Laplacian 0), so the centre voxel
        // oscillates at a clean omega0. genesis/damping OFF so the block
        // persists; the panel drives the clock and reads centre |J|(t).
        rb.toggles.wave_propagation = true;
        rb.toggles.genesis          = false;
        rb.toggles.damping          = false;
        rb.toggles.dual_substrate   = false;
        const int half = 3;            // 7x7x7 central manifested block
        const double J0 = 0.08;
        for (int dx = -half; dx <= half; ++dx)
            for (int dy = -half; dy <= half; ++dy)
                for (int dz = -half; dz <= half; ++dz) {
                    IP(rb, mc + dx, mc + dy, mc + dz, +1);
                    IF(rb, mc + dx, mc + dy, mc + dz, J0, 0, 0);
                }
    }
    return true;
}

}  // namespace ftd
