/**
 * @file test_emergent_ic1_topology.cpp
 * @brief Regression + topology verification for the canonical ic1 cluster.
 *
 * RE-BASELINED 2026-06-10 (FTD-0260 resolution, owner decision): the
 * historical FTD-0107 expectation (25-voxel cluster at A=10) was a
 * measurement of the pre-correction April/May 2026 stack. Accumulated
 * deliberate engine corrections changed the canonical ic1 steady state;
 * the current canonical stack yields 3-8 voxels at A=10 (both backends).
 * T1/T2/T3 pins below carry the new bands, with the historical values
 * preserved in comments; the historical record itself lives in
 * FOUND_LATTICE_SPACING_GAUGE_FREEDOM.md section 6.5 (stack-pinned) and
 * LEDGER rows FTD-0110/FTD-0260. The original header follows.
 *
 * Two regression checks (confirming existing measurements):
 *   T1: Cluster count and voxel count match FTD-0107 at L=32.
 *       Expected: 1 cluster of exactly 25 voxels, 5/5 seeds.
 *   T2: Same at L=16 (smoke; faster bind-time).
 *
 * One structural-hypothesis check (verifies EXPLR_25_VOXEL_CLUSTER_GEOMETRY.md):
 *   T3: The 25 manifested voxel positions form the L¹-ball of radius 2
 *       centered on the injection point. Specifically, the multiset of
 *       (|dx| + |dy| + |dz|) values across the 25 voxels equals
 *       {0:1, 1:6, 2:18}, matching the centered octahedral number O(2).
 *       The 8 BCC corners at L¹=3 (i.e. (±1,±1,±1)) are NOT in the cluster.
 *
 * Pre-registration scope: the cluster-count regression is verifying
 * already-measured findings (FTD-0102 / FTD-0107). The L¹-ball-topology
 * check is a NEW positional measurement of an already-published
 * hypothesis (EXPLR_25_VOXEL_CLUSTER_GEOMETRY.md §3 "STRUCTURAL
 * HYPOTHESIS"). Per the user's pre-registration scope decision
 * (regression tests don't need new pre-reg; only NEW measurements with
 * untested predictions do), this test confirms a published hypothesis
 * exactly — no candidate values are adjusted post-hoc.
 *
 * Returns 0 on PASS, non-zero on FAIL.
 */

#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/scenarios.h"
#include "ftd/voxel.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <queue>
#include <tuple>
#include <vector>

namespace {

struct ClusterInfo {
    int voxel_count = 0;
    double cx = 0, cy = 0, cz = 0;       // centroid (voxel coords)
    int charge_sum = 0;
    std::vector<std::tuple<int,int,int>> positions;
};

// BFS over Moore-26 neighbors of manifested voxels, returns clusters.
std::vector<ClusterInfo> detect_clusters(const ftd::RenderBridge& rb) {
    const int L = rb.lattice().size();
    const int N = L * L * L;
    const auto& voxels = rb.voxels();

    auto idx = [L](int x, int y, int z) {
        x = ((x % L) + L) % L;
        y = ((y % L) + L) % L;
        z = ((z % L) + L) % L;
        return x * L * L + y * L + z;
    };

    std::vector<bool> visited(N, false);
    std::vector<ClusterInfo> clusters;

    for (int z0 = 0; z0 < L; ++z0)
    for (int y0 = 0; y0 < L; ++y0)
    for (int x0 = 0; x0 < L; ++x0) {
        const int i0 = idx(x0, y0, z0);
        if (visited[i0]) continue;
        if (voxels[i0].state == 0) continue;

        ClusterInfo c;
        std::queue<std::tuple<int,int,int>> q;
        q.push({x0, y0, z0});
        visited[i0] = true;

        while (!q.empty()) {
            auto [cx, cy, cz] = q.front(); q.pop();
            const int ci = idx(cx, cy, cz);
            const auto& v = voxels[ci];
            c.voxel_count++;
            c.cx += cx; c.cy += cy; c.cz += cz;
            c.charge_sum += v.state;
            c.positions.push_back({cx, cy, cz});
            // 26-neighbor Moore expansion
            for (int dz = -1; dz <= 1; ++dz)
            for (int dy = -1; dy <= 1; ++dy)
            for (int dx = -1; dx <= 1; ++dx) {
                if (dx == 0 && dy == 0 && dz == 0) continue;
                int nx = cx + dx, ny = cy + dy, nz = cz + dz;
                int ni = idx(nx, ny, nz);
                if (visited[ni]) continue;
                if (voxels[ni].state == 0) continue;
                visited[ni] = true;
                q.push({nx, ny, nz});
            }
        }
        if (c.voxel_count > 0) {
            c.cx /= c.voxel_count;
            c.cy /= c.voxel_count;
            c.cz /= c.voxel_count;
            clusters.push_back(std::move(c));
        }
    }
    return clusters;
}

// Compute (signed) lattice difference with periodic wrap (smallest magnitude).
int wrap_diff(int a, int b, int L) {
    int d = a - b;
    if (d > L / 2) d -= L;
    if (d < -L / 2) d += L;
    return d;
}

// Same as run_ic1 but with body-diagonal injection (instead of axial +x).
// Used by T8 to test the Z_4 (face-axis) vs Z_3 (body-diagonal) origin
// of the cluster-efficiency coefficient k.
void run_ic1_diagonal_amplitude(int L, std::uint32_t seed, double amp_in_K_GENESIS,
                                std::vector<ClusterInfo>& out_clusters) {
    ftd::RenderBridge rb(L);
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.genesis          = true;
    rb.toggles.langevin         = true;
    rb.toggles.coupling         = true;
    rb.toggles.langevin_T       = 0.005;
    rb.toggles.langevin_gamma   = 0.02;
    rb.toggles.dual_substrate   = false;
    rb.seed_rng(seed);

    const double inv_sqrt3 = 1.0 / std::sqrt(3.0);
    const double comp = amp_in_K_GENESIS * ftd::K_GENESIS * inv_sqrt3;
    rb.inject_flux(L / 2, L / 2, L / 2, {comp, comp, comp});

    rb.run(700);

    out_clusters = detect_clusters(rb);
}

// Same as run_ic1 but with explicit injection amplitude in units of K_GENESIS.
// Used by T5 to test the O(r*) flux-radius selection hypothesis.
void run_ic1_amplitude(int L, std::uint32_t seed, double amp_in_K_GENESIS,
                       std::vector<ClusterInfo>& out_clusters) {
    ftd::RenderBridge rb(L);
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.genesis          = true;
    rb.toggles.langevin         = true;
    rb.toggles.coupling         = true;
    rb.toggles.langevin_T       = 0.005;
    rb.toggles.langevin_gamma   = 0.02;
    rb.toggles.dual_substrate   = false;
    rb.seed_rng(seed);

    rb.inject_flux(L / 2, L / 2, L / 2,
                   {amp_in_K_GENESIS * ftd::K_GENESIS, 0, 0});

    rb.run(700);

    out_clusters = detect_clusters(rb);
}

void run_ic1(int L, std::uint32_t seed, std::vector<ClusterInfo>& out_clusters) {
    run_ic1_amplitude(L, seed, 10.0, out_clusters);
}

bool t1_cluster_count_at_L(int L, int n_seeds, int expected_voxels, double tol_voxels) {
    int passing_seeds = 0;
    int total_voxels_observed = 0;
    int n_observed_clusters = 0;
    std::vector<int> per_seed_voxel_counts;
    for (int s = 0; s < n_seeds; ++s) {
        std::uint32_t seed = 0xE0102000u + static_cast<std::uint32_t>(s);
        std::vector<ClusterInfo> clusters;
        run_ic1(L, seed, clusters);

        // Find the largest cluster (the bound state)
        int largest = 0;
        int total = 0;
        for (const auto& c : clusters) {
            total += c.voxel_count;
            if (c.voxel_count > largest) largest = c.voxel_count;
        }
        per_seed_voxel_counts.push_back(largest);
        total_voxels_observed += total;
        n_observed_clusters += static_cast<int>(clusters.size());
        if (std::abs(largest - expected_voxels) <= tol_voxels) {
            passing_seeds++;
        }
        std::printf("    seed=%d: %zu cluster(s), largest=%d voxels, total_manifest=%d\n",
                    s, clusters.size(), largest, total);
    }
    std::printf("  T1@L=%d: passing_seeds=%d/%d (largest cluster within ±%g of %d voxels)\n",
                L, passing_seeds, n_seeds, tol_voxels, expected_voxels);
    return passing_seeds == n_seeds;
}

bool t3_l1_ball_topology(int L, std::uint32_t seed) {
    std::vector<ClusterInfo> clusters;
    run_ic1(L, seed, clusters);
    if (clusters.empty()) {
        std::printf("  T3: FAIL — no cluster formed\n");
        return false;
    }
    // Pick the largest cluster
    auto& c = *std::max_element(clusters.begin(), clusters.end(),
        [](const ClusterInfo& a, const ClusterInfo& b) {
            return a.voxel_count < b.voxel_count;
        });
    if (c.voxel_count == 0) {
        std::printf("  T3: FAIL — empty cluster\n");
        return false;
    }
    // Find centroid in integer lattice coords. The injection point is L/2 at
    // injection time; the cluster centroid floats by ≤1 voxel under Langevin.
    const int cx0 = L / 2, cy0 = L / 2, cz0 = L / 2;

    // Bin voxels by L¹ distance from injection point
    // Per-orbit decomposition under (L¹, L∞) classification:
    //   center   : (L¹=0, L∞=0)  — 1 site
    //   SC       : (L¹=1, L∞=1)  — 6 sites (face1 of Moore-1)
    //   FCC      : (L¹=2, L∞=1)  — 12 sites (edge of Moore-1)
    //   BCC      : (L¹=3, L∞=1)  — 8 sites (corner of Moore-1)
    //   face2    : (L¹=2, L∞=2)  — 6 sites (Moore-2 axial extension)
    //   edge2    : (L¹=3, L∞=2)  — 24 sites (Moore-2 face-edge)
    //   far      : everything else
    std::map<int, int> l1_histogram;
    int n_center=0, n_SC=0, n_FCC=0, n_BCC=0, n_face2=0, n_edge2=0, n_other=0;
    for (auto [x, y, z] : c.positions) {
        int dx = wrap_diff(x, cx0, L);
        int dy = wrap_diff(y, cy0, L);
        int dz = wrap_diff(z, cz0, L);
        int l1 = std::abs(dx) + std::abs(dy) + std::abs(dz);
        int linf = std::max({std::abs(dx), std::abs(dy), std::abs(dz)});
        l1_histogram[l1]++;
        if (l1 == 0)            n_center++;
        else if (l1 == 1)       n_SC++;
        else if (l1 == 2 && linf == 1)  n_FCC++;
        else if (l1 == 2 && linf == 2)  n_face2++;
        else if (l1 == 3 && linf == 1)  n_BCC++;
        else if (l1 == 3 && linf == 2)  n_edge2++;
        else                    n_other++;
    }

    std::printf("  T3@L=%d (seed 0x%x): largest cluster has %d voxels\n",
                L, seed, c.voxel_count);
    std::printf("  L¹ histogram:\n");
    for (auto [l1, n] : l1_histogram) {
        std::printf("       L¹=%d: %d voxels\n", l1, n);
    }
    std::printf("  Per-orbit decomposition (Moore sub-stencil classification):\n");
    std::printf("       center (L¹=0)               : %d  [hypothesis: 1 ]\n", n_center);
    std::printf("       SC face1 (L¹=1, L∞=1)       : %d  [hypothesis: 6 ]\n", n_SC);
    std::printf("       FCC edge (L¹=2, L∞=1)       : %d  [hypothesis: 12]\n", n_FCC);
    std::printf("       face2 axis (L¹=2, L∞=2)     : %d  [hypothesis: 6 ]\n", n_face2);
    std::printf("       BCC corner (L¹=3, L∞=1)     : %d  [hypothesis: 0 ]\n", n_BCC);
    std::printf("       edge2 (L¹=3, L∞=2)          : %d  [hypothesis: 0 ]\n", n_edge2);
    std::printf("       other (L¹≥4 or L∞≥3)        : %d  [hypothesis: 0 ]\n", n_other);
    std::printf("\n  ================================================================\n");
    std::printf("  HYPOTHESIS COMPARISON (informational; not asserted):\n");
    std::printf("  ----------------------------------------------------------------\n");
    std::printf("  L¹-ball-radius-2 prediction: {1, 6, 12, 6, 0, 0, 0} (25 voxels)\n");
    std::printf("  Per EXPLR_25_VOXEL_CLUSTER_GEOMETRY.md §3 — REFUTED by this measurement:\n");
    std::printf("    - hypothesis predicted BCC corners = 0; measured = %d (REFUTES)\n", n_BCC);
    std::printf("    - hypothesis predicted FCC edges = 12; measured = %d (off by %d)\n", n_FCC, n_FCC - 12);
    std::printf("    - hypothesis predicted face2 = 6;    measured = %d (off by %d)\n", n_face2, n_face2 - 6);
    std::printf("  The 25-voxel count is correct but the SHAPE is closer to Moore-1+center\n");
    std::printf("  with partial FCC edges + partial face2, NOT the L¹-ball-radius-2.\n");
    std::printf("  Polytope-duality interpretation in EXPLR_OCTAHEDRAL_BOUND_STATES.md\n");
    std::printf("  needs corrigendum: the cluster INCLUDES BCC corners, contradicting the\n");
    std::printf("  cluster-on-SC+FCC vs algebra-on-BCC complementarity reading.\n");
    std::printf("  ================================================================\n\n");
    // T4 (2026-04-27 evening): dump per-voxel coordinates relative to injection
    // centre so the actual topology can be analysed structurally. The orbit
    // counts above hide the directional information — which 7 of the 12 FCC
    // edges are present? which 3 of the 6 face2 voxels? Emit a sorted (dx,dy,dz)
    // list so we can see selection rules across seeds.
    std::printf("  Per-voxel relative coordinates (dx, dy, dz from injection centre):\n");
    std::vector<std::tuple<int,int,int>> rel;
    for (auto [x, y, z] : c.positions) {
        int dx = wrap_diff(x, cx0, L);
        int dy = wrap_diff(y, cy0, L);
        int dz = wrap_diff(z, cz0, L);
        rel.emplace_back(dx, dy, dz);
    }
    std::sort(rel.begin(), rel.end());
    for (auto [dx, dy, dz] : rel) {
        int l1 = std::abs(dx) + std::abs(dy) + std::abs(dz);
        int linf = std::max({std::abs(dx), std::abs(dy), std::abs(dz)});
        std::printf("    (%+d, %+d, %+d)  L1=%d L_inf=%d\n", dx, dy, dz, l1, linf);
    }
    std::printf("\n");
    bool any_bcc_corner = (n_BCC > 0);
    (void)any_bcc_corner;  // observation, not assertion criterion

    // Acceptance criteria (only ROBUST invariants; the L¹-ball-2 hypothesis
    // has been refuted, so we no longer assert that specific shape):
    //   1. Cluster count within the current-stack regression band
    //      (RE-BASELINED 2026-06-10 per FTD-0260: historical band was
    //      23–27 around the pre-correction stack's 25; current canonical
    //      stack gives 3–5 at this seed/config; new band 2–10)
    //   2. The cluster is connected (BFS guarantees this by construction)
    //   3. No voxels at L¹ ≥ 4 (cluster is localised; no runaway)
    //   4. Center voxel is present (the injection point manifested)
    bool count_ok = (c.voxel_count >= 2 && c.voxel_count <= 10);
    bool localised = true;
    for (auto [l1, n] : l1_histogram) {
        if (l1 >= 4 && n > 0) { localised = false; break; }
    }
    bool center_ok = (n_center == 1);
    std::printf("  Robust-invariant checks:\n");
    std::printf("    cluster count in band 2-10 (pre-2026-06 stack: 23-27): %s\n", count_ok ? "PASS" : "FAIL");
    std::printf("    No voxels at L¹ >= 4 (localised):  %s\n", localised ? "PASS" : "FAIL");
    std::printf("    Center voxel manifested:           %s\n", center_ok ? "PASS" : "FAIL");
    return count_ok && localised && center_ok;
}

}  // namespace

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    std::printf("================================================================\n");
    std::printf("  FTD-0107 EMERGENT-IC1 TOPOLOGY REGRESSION TEST\n");
    std::printf("  Reproduces FTD-0102/FTD-0107 cluster count and verifies\n");
    std::printf("  L¹-ball-radius-2 topology hypothesis from\n");
    std::printf("  EXPLR_25_VOXEL_CLUSTER_GEOMETRY.md\n");
    std::printf("================================================================\n");

    bool all_pass = true;

    // T2 first (faster smoke): L=16 should still produce the bound state.
    // RE-BASELINED 2026-06-10 (FTD-0260 resolution, owner decision): the
    // historical expectation was 25 voxels (tol=10) — a measurement of the
    // pre-correction April/May stack. Accumulated engine corrections since
    // changed the canonical ic1 steady state; the CURRENT canonical stack
    // gives 3–8 voxels at A=10 across seeds and backends (CPU 8/6/4,
    // GPU 3/5/4 measured 2026-06-10). New pin: 6 ± 4.
    std::printf("\n[T2] Smoke: ic1 at L=16, 3 seeds, expect ~6-voxel cluster (tol=4; pre-2026-06 stack: 25)\n");
    bool t2 = t1_cluster_count_at_L(16, 3, 6, 4);
    std::printf("  T2 verdict: %s\n", t2 ? "PASS" : "FAIL");
    all_pass &= t2;

    // T1: L=32, the FTD-0102 baseline regression pin.
    // RE-BASELINED 2026-06-10 (FTD-0260 resolution): historical pin was
    // 25 voxels (tol=2), stack-pinned to the pre-correction engine. Current
    // canonical stack: 3–5 voxels at A=10 (CPU 4/5/4, GPU 3/5/4). New pin:
    // 4 ± 2. The historical 25-voxel record is preserved in
    // FOUND_LATTICE_SPACING_GAUGE_FREEDOM.md §6.5 (marked stack-pinned)
    // and in the LEDGER FTD-0110/FTD-0260 rows.
    std::printf("\n[T1] FTD-0102 regression: ic1 at L=32, 3 seeds, expect 4 voxels (tol=2; pre-2026-06 stack: 25)\n");
    bool t1 = t1_cluster_count_at_L(32, 3, 4, 2);
    std::printf("  T1 verdict: %s\n", t1 ? "PASS" : "FAIL");
    all_pass &= t1;

    // T3: L¹-ball-radius-2 topology check at L=32, single seed
    std::printf("\n[T3] L¹-ball-radius-2 topology check at L=32, seed 0xE0102000\n");
    bool t3 = t3_l1_ball_topology(32, 0xE0102000u);
    std::printf("  T3 verdict: %s\n", t3 ? "PASS" : "FAIL");
    all_pass &= t3;

    // T4 (2026-04-27 evening): multi-seed inclusion-frequency analysis.
    // For each of N_SEEDS seeds, run ic1 to terminal state, dump cluster
    // voxels, and tally per-voxel inclusion frequency. Voxels with
    // frequency = 1.0 form the "deterministic core"; voxels with
    // frequency in (0, 1) form the "stochastic shell". Tests whether the
    // 25-count is selected by a count/conservation argument vs a shape
    // argument: if the core has size 25 and the shell is empty, the
    // cluster shape is fully deterministic; if the core is smaller and
    // the shell averages to (25 - |core|), the count is conserved but
    // the shape is stochastic.
    {
        std::printf("\n[T4] Multi-seed inclusion-frequency analysis at L=32, 5 seeds\n");
        const int L = 32;
        const int N_seeds = 5;
        std::map<std::tuple<int,int,int>, int> tally;
        std::vector<int> per_seed_counts;
        for (int s = 0; s < N_seeds; ++s) {
            std::vector<ClusterInfo> clusters;
            run_ic1(L, 0xE0102000u + s, clusters);
            if (clusters.empty()) continue;
            auto& c = *std::max_element(clusters.begin(), clusters.end(),
                [](const ClusterInfo& a, const ClusterInfo& b) {
                    return a.voxel_count < b.voxel_count;
                });
            per_seed_counts.push_back(c.voxel_count);
            for (auto [x, y, z] : c.positions) {
                int dx = wrap_diff(x, L/2, L);
                int dy = wrap_diff(y, L/2, L);
                int dz = wrap_diff(z, L/2, L);
                tally[{dx, dy, dz}]++;
            }
        }
        std::printf("  Per-seed cluster sizes:");
        for (int n : per_seed_counts) std::printf(" %d", n);
        std::printf("\n");
        // Bin by frequency
        int n_always = 0, n_majority = 0, n_minority = 0, n_once = 0;
        for (auto& [pos, count] : tally) {
            if (count == N_seeds) n_always++;
            else if (count >= N_seeds / 2 + 1) n_majority++;
            else if (count >= 2) n_minority++;
            else n_once++;
        }
        std::printf("  Per-voxel inclusion frequency across %d seeds:\n", N_seeds);
        std::printf("    always   (%d/%d): %d voxels  [deterministic core]\n",
                    N_seeds, N_seeds, n_always);
        std::printf("    majority (%d-%d/%d): %d voxels\n",
                    N_seeds/2 + 1, N_seeds - 1, N_seeds, n_majority);
        std::printf("    minority (2-%d/%d): %d voxels\n",
                    N_seeds/2, N_seeds, n_minority);
        std::printf("    once     (1/%d): %d voxels  [stochastic outliers]\n",
                    N_seeds, n_once);
        std::printf("    total distinct voxels seen: %zu\n", tally.size());
        // Print "always" set explicitly — that's the structural skeleton
        std::printf("\n  Deterministic core (voxels in ALL %d seeds):\n", N_seeds);
        std::vector<std::tuple<int,int,int>> core;
        for (auto& [pos, count] : tally) {
            if (count == N_seeds) core.push_back(pos);
        }
        std::sort(core.begin(), core.end());
        for (auto [dx, dy, dz] : core) {
            int l1 = std::abs(dx) + std::abs(dy) + std::abs(dz);
            int linf = std::max({std::abs(dx), std::abs(dy), std::abs(dz)});
            std::printf("    (%+d, %+d, %+d)  L1=%d L_inf=%d\n", dx, dy, dz, l1, linf);
        }
        std::printf("\n");
    }

    // T5 (2026-04-27 evening): O(r*) flux-radius selection hypothesis.
    // Prediction: cluster size at injection amplitude A·K_GENESIS scales as
    // the centered-octahedral number O(r*) = (2r*+1)(2r*²+2r*+3)/3 where r*
    // is the lattice-Green's-function threshold radius for K_GENESIS.
    //
    //   A = 1.5 → r* = 1 → O(1) = 7   (or sub-threshold → 0)
    //   A = 5   → between O(1)=7 and O(2)=25
    //   A = 10  → r* = 2 → O(2) = 25  (FTD-0107 baseline; confirmed)
    //   A = 20  → between O(2)=25 and O(3)=63
    //   A = 30  → r* = 3 → O(3) = 63
    //
    // If the cluster sizes match O(r*) closely, the count is selected by a
    // discrete-radius argument and the structural origin of 25 is identified.
    // If they don't, a different mechanism is at work.
    // T7 (2026-04-27 evening): tau-amplitude verification at L=80.
    // R = m_τ/m_e = 3477. Predicted A = 2·√3477 ≈ 117.9·K_GENESIS.
    // Predicted cluster size N ≈ ¼·A² = 3477 voxels (or k(A)·A² with k ≈ 0.21
    // by extrapolation from T5b). Cluster radius ≈ (3·3477/4π)^(1/3) ≈ 9.4
    // voxels — well-contained in L=80.
    //
    // PASS criterion (preregistered): mean cluster size within 30% of 3477,
    // 5/5 seeds non-runaway. Empirical k(A) interpolation: cluster ~ 2900-3200
    // expected (vs 3477 from naive ¼·A²).
    {
        std::printf("\n[T7] Tau verification at L=80, 5 seeds: predicted N=3477 at A=117.9·K_GENESIS\n");
        const double A_tau = 2.0 * std::sqrt(3477.0);  // ≈ 117.93
        const int L = 80;
        const int N_seeds = 5;
        std::vector<int> sizes;
        for (int s = 0; s < N_seeds; ++s) {
            std::vector<ClusterInfo> clusters;
            run_ic1_amplitude(L, 0xE0102000u + s, A_tau, clusters);
            int largest = 0;
            for (auto& c : clusters) if (c.voxel_count > largest) largest = c.voxel_count;
            sizes.push_back(largest);
            std::printf("    seed %d: cluster_size=%d\n", s, largest);
        }
        double mean = 0.0;
        for (int n : sizes) mean += n;
        mean /= sizes.size();
        double var = 0.0;
        for (int n : sizes) var += (n - mean) * (n - mean);
        double std_dev = std::sqrt(var / sizes.size());
        double k_emp = mean / (A_tau * A_tau);
        std::printf("    mean=%.1f ± %.1f, k_emp=%.4f, predicted_naive=%.0f, Δ=%.1f%%\n",
                    mean, std_dev, k_emp, 0.25*A_tau*A_tau, 100.0*std::abs(mean - 3477.0)/3477.0);
        std::printf("\n");
    }

    // T6 (2026-04-27 evening): SM-particle identification verification.
    // If cluster-size ↔ mass/m_e identification holds and N ≈ ¼·A²,
    // then for particle X with mass m_X = R·m_e, the predicted injection
    // amplitude is A = 2·√R·K_GENESIS, and the predicted cluster size is R.
    //
    // Test the muon prediction: m_μ/m_e = 207 → A = 2·√207 ≈ 28.77.
    // Test the proton prediction: m_p/m_e = 1836 → A = 2·√1836 ≈ 85.7.
    //
    // PASS criterion (preregistered loose): predicted size within 30% of
    // measured mean across 5 seeds. The ¼·A² scaling already has 5%
    // RMS error so 30% leaves headroom for Langevin variance.
    //
    // STRICT failure criterion: measured size diverges by >2× from
    // prediction (e.g., runaway, or scaling broken at this amplitude).
    {
        std::printf("\n[T6] SM-particle cluster-size predictions at L=32 (then L=64 if size > 100)\n");
        struct SMProbe { const char* name; int R; };
        std::vector<SMProbe> particles = {
            {"electron", 1},
            {"muon",     207},
            {"pion",     273},
            {"kaon",     974},
            {"proton",   1836},
        };
        const int N_seeds = 5;
        for (auto& p : particles) {
            double A = 2.0 * std::sqrt(static_cast<double>(p.R));
            // Pick L that comfortably contains the predicted cluster
            int L = (p.R <= 100) ? 32 : (p.R <= 1500 ? 48 : 64);
            std::vector<int> sizes;
            for (int s = 0; s < N_seeds; ++s) {
                std::vector<ClusterInfo> clusters;
                run_ic1_amplitude(L, 0xE0102000u + s, A, clusters);
                int largest = 0;
                for (auto& c : clusters) if (c.voxel_count > largest) largest = c.voxel_count;
                sizes.push_back(largest);
            }
            double mean = 0.0;
            for (int n : sizes) mean += n;
            mean /= sizes.size();
            double dev = 100.0 * std::abs(mean - p.R) / p.R;
            std::printf("  %-9s R=%-6d A=%5.2f L=%2d  measured=%6.1f  Δ=%5.1f%%  (",
                        p.name, p.R, A, L, mean, dev);
            for (int n : sizes) std::printf(" %d", n);
            std::printf(" )\n");
        }
        std::printf("\n");
    }

    {
        std::printf("\n[T5b] Cluster-size vs amplitude: 9 amplitudes × 5 seeds at L=32\n");
        std::printf("        Hypothesis: N(A) ≈ k · A² with k ≈ 0.25 (energy balance |J|² ∝ A²)\n");
        std::printf("        %-8s %-12s %-12s %-10s %-10s\n",
                    "A/K_GEN", "mean ± std", "min..max", "k=N/A²", "predict A²/4");
        const std::vector<double> amps = {0.5, 1.5, 3.0, 5.0, 10.0, 15.0, 20.0, 30.0, 50.0};
        const int N_seeds = 5;
        for (double A : amps) {
            std::vector<int> sizes;
            for (int s = 0; s < N_seeds; ++s) {
                std::vector<ClusterInfo> clusters;
                run_ic1_amplitude(32, 0xE0102000u + s, A, clusters);
                int largest = 0;
                for (auto& c : clusters) if (c.voxel_count > largest) largest = c.voxel_count;
                sizes.push_back(largest);
            }
            double mean = 0.0;
            for (int n : sizes) mean += n;
            mean /= sizes.size();
            double var = 0.0;
            for (int n : sizes) var += (n - mean) * (n - mean);
            double std_dev = std::sqrt(var / sizes.size());
            int mn = *std::min_element(sizes.begin(), sizes.end());
            int mx = *std::max_element(sizes.begin(), sizes.end());
            double k = mean / (A * A);
            double predicted = A * A / 4.0;
            std::printf("        %-8.2f %5.1f ± %4.1f   %3d..%-3d    %6.3f    %.1f\n",
                        A, mean, std_dev, mn, mx, k, predicted);
        }
        std::printf("\n");
    }

    // T8 (2026-04-27 evening): D3g — body-diagonal injection amplitude sweep.
    // Tests Z_4 (face-axis, k=¼) vs Z_3 (body-diagonal, k=⅓) discrimination
    // of the cluster-efficiency origin.
    //
    // Hypothesis A (Z_4 / rotation-cycle origin): k_diag ≈ 1/3 ≈ 0.333.
    //   The rotation about the body diagonal (1,1,1)/√3 is Z_3, not Z_4.
    //   If the cluster-efficiency ¼ comes from the Z_4 face-rotation cycle,
    //   then a body-diagonal injection rotates through Z_3 instead, giving
    //   k = 1/|Z_3| = 1/3. Cluster size at A=10 would be ~33 instead of 25.
    //
    // Hypothesis B (N_base / global integer origin): k_diag ≈ 1/4 ≈ 0.250.
    //   N_base = 4 is a global lattice property (= mult(A_{1g}) in 27-site
    //   O_h decomposition); doesn't depend on injection direction.
    //   Cluster size at A=10 would still be ~25.
    {
        std::printf("\n[T8] D3g — body-diagonal injection at L=32, 5 amplitudes × 5 seeds\n");
        std::printf("       Z_4 reading predicts k_diag ≈ 1/3 = 0.333\n");
        std::printf("       N_base reading predicts k_diag ≈ 1/4 = 0.250\n");
        std::printf("       %-8s %-12s %-12s %-10s %-10s %-12s\n",
                    "A/K_GEN", "axial(T5b)", "diagonal", "k_diag", "Δ_Z4", "Δ_Nbase");
        const std::vector<double> amps = {10.0, 15.0, 20.0, 30.0, 50.0};
        const int N_seeds = 5;
        // Reference axial values from T5b (above): A=10:25.2, A=15:50.4, A=20:93.4, A=30:235.8, A=50:554.0
        const std::vector<double> axial_ref = {25.2, 50.4, 93.4, 235.8, 554.0};
        for (size_t i = 0; i < amps.size(); ++i) {
            double A = amps[i];
            std::vector<int> sizes;
            for (int s = 0; s < N_seeds; ++s) {
                std::vector<ClusterInfo> clusters;
                run_ic1_diagonal_amplitude(32, 0xE0102000u + s, A, clusters);
                int largest = 0;
                for (auto& c : clusters) if (c.voxel_count > largest) largest = c.voxel_count;
                sizes.push_back(largest);
            }
            double mean = 0.0;
            for (int n : sizes) mean += n;
            mean /= sizes.size();
            double k_diag = mean / (A * A);
            double dist_Z4   = std::abs(k_diag - 1.0/3.0);
            double dist_Nbase = std::abs(k_diag - 1.0/4.0);
            std::printf("        %-8.2f %5.1f        %5.1f        %6.3f    %.3f      %.3f%s\n",
                        A, axial_ref[i], mean, k_diag, dist_Z4, dist_Nbase,
                        (dist_Z4 < dist_Nbase) ? "    [Z4]" : "    [Nbase]");
        }
        std::printf("\n  Verdict: nearest-prediction tag in last column. If consistently [Z4],\n");
        std::printf("           cluster-efficiency origin is rotation-cycle around injection axis.\n");
        std::printf("           If consistently [Nbase], origin is global lattice integer.\n");
        std::printf("\n");
    }

    std::printf("\n================================================================\n");
    std::printf("  Overall: %s\n", all_pass ? "PASS" : "FAIL");
    std::printf("================================================================\n");
    return all_pass ? 0 : 1;
}
