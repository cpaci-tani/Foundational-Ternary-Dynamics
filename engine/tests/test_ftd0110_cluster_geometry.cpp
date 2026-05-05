/**
 * Test: FTD-0110 cluster GEOMETRY diagnostic.
 *
 * Following the 2026-05-04 Option A finding (DERIV_FTD0110_NONLINEAR_BRIDGE.md §5),
 * the local-27-block A_{1g} purity that Bridge-II §3.1 invokes is empirically
 * falsified by gauss_projection's non-local Poisson convolution. The mean-cluster
 * size formula N(A) ≈ A²/4 still holds at 5% across the SM-particle test grid.
 *
 * To re-derive N(A) ≈ A²/4 without local A_{1g} purity, we first need to know
 * what KIND of object the cluster is geometrically. The empirical fit
 *     N(A) = (A/(2·K_GENESIS))²
 * is consistent with multiple geometries:
 *     - 2D square of side A/(2·K_GEN)   → N = side²
 *     - 2D disk of radius A/(2·K_GEN·√π) → N = πr²
 *     - thin 3D shell at radius A/(K_GEN·4√π) → N = 4πR²
 *     - 1D string of length A²/4
 *     - 3D cube of side ~ A^{2/3}
 * etc. Each implies a different physical mechanism.
 *
 * This test instruments the existing FTD-0110 amplitude scan to record
 * the bounding box (dx, dy, dz) and principal-axis covariance eigenvalues
 * of the largest cluster, in addition to its voxel count. The output
 * tells us:
 *   - dx vs dy vs dz aspect ratios → 1D / 2D / 3D classification
 *   - λ_1, λ_2, λ_3 (covariance eigenvalues) → linearity / planarity / sphericity
 *
 * Toggles: standard FTD-0110 ic1 set (wave + gauss + genesis + Langevin).
 * IC: δ_centre · A·ê_x at lattice centre (16, 16, 16) on L=32.
 * Ticks: 700 (matches T5b).
 * Amplitudes: A ∈ {10, 15, 20, 30, 50}·K_GENESIS (matches T5b grid).
 * Seeds: 5 per amplitude.
 *
 * Output: per-amplitude mean ± std of bounding box and covariance eigenvalues.
 *
 * Cross-references:
 *   docs/theory/03_derivations/DERIV_FTD0110_NONLINEAR_BRIDGE.md §5.4 Routes
 *   engine/tests/test_emergent_ic1_topology.cpp T5b (per-amplitude N data)
 */

#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <queue>
#include <string>
#include <tuple>
#include <vector>

namespace {

struct ClusterGeometry {
    int n_voxels = 0;
    double cx = 0, cy = 0, cz = 0;
    int dx = 0, dy = 0, dz = 0;       // bounding box extents (max - min)
    double lambda1 = 0, lambda2 = 0, lambda3 = 0;  // covariance eigenvalues, sorted ↓
};

int wrap_diff(int a, int b, int L) {
    int d = a - b;
    if (d > L / 2) d -= L;
    if (d < -L / 2) d += L;
    return d;
}

// Find connected (Moore-26) clusters of manifested voxels and return the
// largest one's geometry. Periodic-wrap aware.
ClusterGeometry largest_cluster_geometry(const ftd::RenderBridge& rb) {
    const int L = rb.lattice().size();
    const int N_total = L * L * L;
    const auto& voxels = rb.voxels();

    auto idx = [L](int x, int y, int z) {
        x = ((x % L) + L) % L;
        y = ((y % L) + L) % L;
        z = ((z % L) + L) % L;
        return x * L * L + y * L + z;
    };

    std::vector<bool> visited(N_total, false);
    std::vector<std::vector<std::tuple<int,int,int>>> clusters_positions;

    for (int z0 = 0; z0 < L; ++z0)
    for (int y0 = 0; y0 < L; ++y0)
    for (int x0 = 0; x0 < L; ++x0) {
        const int i0 = idx(x0, y0, z0);
        if (visited[i0]) continue;
        if (voxels[i0].state == 0) continue;

        std::vector<std::tuple<int,int,int>> positions;
        std::queue<std::tuple<int,int,int>> q;
        q.push({x0, y0, z0});
        visited[i0] = true;

        while (!q.empty()) {
            auto [cx, cy, cz] = q.front(); q.pop();
            positions.push_back({cx, cy, cz});
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
        clusters_positions.push_back(std::move(positions));
    }

    ClusterGeometry out;
    if (clusters_positions.empty()) return out;

    // Largest cluster
    auto it_max = std::max_element(
        clusters_positions.begin(), clusters_positions.end(),
        [](const auto& a, const auto& b){ return a.size() < b.size(); });
    const auto& pos = *it_max;
    out.n_voxels = static_cast<int>(pos.size());

    // Periodic-wrap-aware centroid: use first voxel as reference.
    auto [rx, ry, rz] = pos[0];
    double sx = 0, sy = 0, sz = 0;
    for (auto [x, y, z] : pos) {
        sx += rx + wrap_diff(x, rx, L);
        sy += ry + wrap_diff(y, ry, L);
        sz += rz + wrap_diff(z, rz, L);
    }
    out.cx = sx / out.n_voxels;
    out.cy = sy / out.n_voxels;
    out.cz = sz / out.n_voxels;

    // Bounding box extents (using wrap-relative coordinates)
    int min_x = INT32_MAX, min_y = INT32_MAX, min_z = INT32_MAX;
    int max_x = INT32_MIN, max_y = INT32_MIN, max_z = INT32_MIN;
    for (auto [x, y, z] : pos) {
        int wx = rx + wrap_diff(x, rx, L);
        int wy = ry + wrap_diff(y, ry, L);
        int wz = rz + wrap_diff(z, rz, L);
        if (wx < min_x) min_x = wx;
        if (wy < min_y) min_y = wy;
        if (wz < min_z) min_z = wz;
        if (wx > max_x) max_x = wx;
        if (wy > max_y) max_y = wy;
        if (wz > max_z) max_z = wz;
    }
    out.dx = max_x - min_x;
    out.dy = max_y - min_y;
    out.dz = max_z - min_z;

    // Covariance matrix C[i,j] = (1/N) Σ (v_k - centroid)_i · (v_k - centroid)_j
    double Cxx = 0, Cyy = 0, Czz = 0, Cxy = 0, Cxz = 0, Cyz = 0;
    for (auto [x, y, z] : pos) {
        double wx = rx + wrap_diff(x, rx, L) - out.cx;
        double wy = ry + wrap_diff(y, ry, L) - out.cy;
        double wz = rz + wrap_diff(z, rz, L) - out.cz;
        Cxx += wx * wx; Cyy += wy * wy; Czz += wz * wz;
        Cxy += wx * wy; Cxz += wx * wz; Cyz += wy * wz;
    }
    Cxx /= out.n_voxels; Cyy /= out.n_voxels; Czz /= out.n_voxels;
    Cxy /= out.n_voxels; Cxz /= out.n_voxels; Cyz /= out.n_voxels;

    // Eigenvalues of 3x3 symmetric matrix via closed-form (Smith 1961 trick).
    // For C = [[Cxx, Cxy, Cxz], [Cxy, Cyy, Cyz], [Cxz, Cyz, Czz]],
    // characteristic polynomial: λ³ − tr·λ² + ... = 0.
    // Subtract trace mean for numerical stability.
    double trace = Cxx + Cyy + Czz;
    double m = trace / 3.0;
    double Bxx = Cxx - m, Byy = Cyy - m, Bzz = Czz - m;
    double p2 = Bxx*Bxx + Byy*Byy + Bzz*Bzz + 2.0*(Cxy*Cxy + Cxz*Cxz + Cyz*Cyz);
    double p = std::sqrt(p2 / 6.0);
    double eig[3];
    if (p < 1e-15) {
        eig[0] = eig[1] = eig[2] = m;
    } else {
        // det(B) / p³ = 2·cos(3φ); φ ∈ [0, π/3].
        double Bdet = Bxx*(Byy*Bzz - Cyz*Cyz)
                    - Cxy*(Cxy*Bzz - Cyz*Cxz)
                    + Cxz*(Cxy*Cyz - Byy*Cxz);
        double r = Bdet / (2.0 * p * p * p);
        if (r > 1.0) r = 1.0;
        if (r < -1.0) r = -1.0;
        double phi = std::acos(r) / 3.0;
        eig[0] = m + 2.0 * p * std::cos(phi);
        eig[2] = m + 2.0 * p * std::cos(phi + 2.0 * 3.14159265358979323846 / 3.0);
        eig[1] = trace - eig[0] - eig[2];
    }
    // sort descending
    std::sort(eig, eig + 3, std::greater<double>());
    out.lambda1 = eig[0];
    out.lambda2 = eig[1];
    out.lambda3 = eig[2];
    return out;
}

ClusterGeometry run_axial(int L, std::uint32_t seed, double amp_in_K_GENESIS) {
    ftd::RenderBridge rb(L);
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.genesis          = true;
    rb.toggles.langevin         = true;
    rb.toggles.langevin_T       = 0.005;
    rb.toggles.langevin_gamma   = 0.02;
    rb.toggles.dual_substrate   = false;
    rb.seed_rng(seed);

    rb.inject_flux(L / 2, L / 2, L / 2,
                   {amp_in_K_GENESIS * ftd::K_GENESIS, 0, 0});
    rb.run(700);
    return largest_cluster_geometry(rb);
}

}  // namespace

int main() {
    std::printf("================================================================\n");
    std::printf("  FTD-0110 cluster GEOMETRY diagnostic (axial +x injection, L=32)\n");
    std::printf("  Toggles: wave + gauss + genesis + Langevin (canonical ic1)\n");
    std::printf("  Goal: distinguish 1D/2D/3D cluster shape via bounding box\n");
    std::printf("        and covariance principal axes.\n");
    std::printf("================================================================\n\n");

    constexpr int L = 32;
    constexpr int N_SEEDS = 5;
    const std::vector<double> amps = {10.0, 15.0, 20.0, 30.0, 50.0};

    std::printf("%-7s  %-13s  %-21s  %-29s\n",
                "A/K_G", "N (mean±std)", "bbox (dx,dy,dz) mean", "λ₁/λ₂/λ₃ (mean)");
    std::printf("%-7s  %-13s  %-21s  %-29s\n",
                "-----", "-----------", "-------------------", "----------------");

    for (double A : amps) {
        std::vector<ClusterGeometry> geoms;
        geoms.reserve(N_SEEDS);
        for (int s = 0; s < N_SEEDS; ++s) {
            std::uint32_t seed = 0xE0102000u + static_cast<std::uint32_t>(s);
            geoms.push_back(run_axial(L, seed, A));
        }
        // Aggregate
        double n_mean = 0, n_var = 0;
        double dx_mean = 0, dy_mean = 0, dz_mean = 0;
        double l1_mean = 0, l2_mean = 0, l3_mean = 0;
        for (const auto& g : geoms) {
            n_mean  += g.n_voxels;
            dx_mean += g.dx; dy_mean += g.dy; dz_mean += g.dz;
            l1_mean += g.lambda1; l2_mean += g.lambda2; l3_mean += g.lambda3;
        }
        n_mean /= N_SEEDS;
        dx_mean /= N_SEEDS; dy_mean /= N_SEEDS; dz_mean /= N_SEEDS;
        l1_mean /= N_SEEDS; l2_mean /= N_SEEDS; l3_mean /= N_SEEDS;
        for (const auto& g : geoms) n_var += (g.n_voxels - n_mean) * (g.n_voxels - n_mean);
        double n_std = std::sqrt(n_var / N_SEEDS);

        std::printf("%-7.1f  %5.1f ± %5.1f  (%4.1f, %4.1f, %4.1f)        %5.2f / %5.2f / %5.2f\n",
                    A, n_mean, n_std, dx_mean, dy_mean, dz_mean,
                    l1_mean, l2_mean, l3_mean);
    }

    std::printf("\n");
    std::printf("Interpretation key:\n");
    std::printf("  • dx≈dy≈dz, λ₁≈λ₂≈λ₃   → 3D-isotropic ball\n");
    std::printf("  • dx≪dy≈dz, λ₁≈λ₂ ≫ λ₃ → 2D plane (transverse to x), √N ≈ side\n");
    std::printf("  • dx≫dy≈dz, λ₁ ≫ λ₂≈λ₃ → 1D string (along x)\n");
    std::printf("  • A²/4 hypothesis if 2D: side ≈ A/2; e.g. A=10 → side=5, √N=5 ✓\n");

    return 0;
}
