#pragma once
/**
 * Correlation Function Infrastructure
 *
 * Physics justification: Correlation functions are the fundamental
 * observables of field theories. C(r) = <J(x)·J(x+r)> reveals the
 * spatial structure of the flux field — propagation range, screening
 * length, bound state size. The charge correlator G(r) = <s(x)s(x+r)>
 * detects confinement (exponential decay) vs deconfinement (power law).
 * The structure factor S(k) = FT[C(r)] gives the momentum-space
 * representation needed for dispersion relations.
 *
 * These are standard lattice field theory measurements, not imposed
 * external physics. They measure what the engine already produces.
 */

#include <vector>
#include <cmath>
#include <algorithm>
#include "render_bridge.h"
#include "sublattice.h"

namespace ftd {

// Displacement geometry for sublattice-aware correlators.
//   AXIS:      ±x̂, ±ŷ, ±ẑ at distance r            (legacy axis-aligned)
//   FACE_DIAG: 12 face-diagonal unit vectors (FCC neighbor directions) at r·√2
//   BODY_DIAG: 8 body-diagonal unit vectors (BCC neighbor directions) at r·√3
// Body-diagonal correlators are required by PROTOCOL_BCC_SUBLATTICE_SPECTRUM
// to pick out the master-quadratic spectrum on σ_BCC.
enum class DisplacementMode : uint8_t {
    AXIS      = 0,
    FACE_DIAG = 1,
    BODY_DIAG = 2
};

// Spatial flux-flux correlator: C(r) = <J(x) · J(x+r)> averaged over all x
// Returns C[r] for r = 0, 1, ..., max_r-1
// Averaged over all directions and all sites for isotropy
inline std::vector<double> spatial_flux_correlation(
    const RenderBridge& rb, int max_r = -1)
{
    const auto& lat = rb.lattice();
    const auto& vox = rb.voxels();
    int L = lat.size();
    if (max_r < 0 || max_r > L / 2) max_r = L / 2;

    std::vector<double> C(max_r, 0.0);
    std::vector<int> counts(max_r, 0);

    // For each site, correlate with sites at displacement +r along each axis
    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                int idx0 = lat.index(x, y, z);
                const Vec3& J0 = vox[idx0].flux;

                for (int r = 0; r < max_r; ++r) {
                    // Average over 3 spatial axes for isotropy
                    // +x direction
                    int idx_x = lat.index(lat.wrap(x + r), y, z);
                    double dot_x = J0.dot(vox[idx_x].flux);

                    // +y direction
                    int idx_y = lat.index(x, lat.wrap(y + r), z);
                    double dot_y = J0.dot(vox[idx_y].flux);

                    // +z direction
                    int idx_z = lat.index(x, y, lat.wrap(z + r));
                    double dot_z = J0.dot(vox[idx_z].flux);

                    C[r] += dot_x + dot_y + dot_z;
                    counts[r] += 3;
                }
            }
        }
    }

    // Normalize
    for (int r = 0; r < max_r; ++r) {
        if (counts[r] > 0) C[r] /= counts[r];
    }
    return C;
}

// Charge-charge correlator: G(r) = <s(x) · s(x+r)> averaged over all x
// Reveals confinement: exponential decay → screening, power law → deconfined
inline std::vector<double> charge_correlation(
    const RenderBridge& rb, int max_r = -1)
{
    const auto& lat = rb.lattice();
    const auto& vox = rb.voxels();
    int L = lat.size();
    if (max_r < 0 || max_r > L / 2) max_r = L / 2;

    std::vector<double> G(max_r, 0.0);
    std::vector<int> counts(max_r, 0);

    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                int idx0 = lat.index(x, y, z);
                int8_t s0 = vox[idx0].state;

                for (int r = 0; r < max_r; ++r) {
                    int idx_x = lat.index(lat.wrap(x + r), y, z);
                    int idx_y = lat.index(x, lat.wrap(y + r), z);
                    int idx_z = lat.index(x, y, lat.wrap(z + r));

                    G[r] += static_cast<double>(s0) * vox[idx_x].state;
                    G[r] += static_cast<double>(s0) * vox[idx_y].state;
                    G[r] += static_cast<double>(s0) * vox[idx_z].state;
                    counts[r] += 3;
                }
            }
        }
    }

    for (int r = 0; r < max_r; ++r) {
        if (counts[r] > 0) G[r] /= counts[r];
    }
    return G;
}

// Density-density correlator: D(r) = <ρ(x)·ρ(x+r)> - <ρ>²
// The connected correlator reveals density fluctuations and clustering
inline std::vector<double> density_correlation(
    const RenderBridge& rb, int max_r = -1)
{
    const auto& lat = rb.lattice();
    const auto& vox = rb.voxels();
    int L = lat.size();
    int N = lat.total_sites();
    if (max_r < 0 || max_r > L / 2) max_r = L / 2;

    // Compute mean density
    double rho_mean = 0.0;
    for (int i = 0; i < N; ++i) rho_mean += vox[i].density();
    rho_mean /= N;

    std::vector<double> D(max_r, 0.0);
    std::vector<int> counts(max_r, 0);

    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                int idx0 = lat.index(x, y, z);
                double rho0 = vox[idx0].density();

                for (int r = 0; r < max_r; ++r) {
                    int idx_x = lat.index(lat.wrap(x + r), y, z);
                    int idx_y = lat.index(x, lat.wrap(y + r), z);
                    int idx_z = lat.index(x, y, lat.wrap(z + r));

                    D[r] += rho0 * vox[idx_x].density();
                    D[r] += rho0 * vox[idx_y].density();
                    D[r] += rho0 * vox[idx_z].density();
                    counts[r] += 3;
                }
            }
        }
    }

    for (int r = 0; r < max_r; ++r) {
        if (counts[r] > 0) D[r] = D[r] / counts[r] - rho_mean * rho_mean;
    }
    return D;
}

// Structure factor S(k) via discrete cosine transform of C(r)
// S(k_n) = C(0) + 2 * sum_{r=1}^{R-1} C(r) * cos(2πnr/R)
// Returns S[n] for n = 0, 1, ..., num_k-1
inline std::vector<double> structure_factor(
    const std::vector<double>& Cr, int num_k = -1)
{
    int R = static_cast<int>(Cr.size());
    if (num_k < 0 || num_k > R) num_k = R;

    std::vector<double> Sk(num_k, 0.0);
    const double TWO_PI = 2.0 * 3.14159265358979323846;

    for (int n = 0; n < num_k; ++n) {
        double sum = Cr[0];
        for (int r = 1; r < R; ++r) {
            sum += 2.0 * Cr[r] * std::cos(TWO_PI * n * r / R);
        }
        Sk[n] = sum;
    }
    return Sk;
}

// Temporal autocorrelation from a time series of scalar measurements.
// C(tau) = <x(t) · x(t+tau)> - <x>²
// Input: vector of scalar measurements at successive ticks.
inline std::vector<double> temporal_autocorrelation(
    const std::vector<double>& series, int max_tau = -1)
{
    int T = static_cast<int>(series.size());
    if (max_tau < 0 || max_tau > T / 2) max_tau = T / 2;

    // Mean
    double mean = 0.0;
    for (double v : series) mean += v;
    mean /= T;

    std::vector<double> C(max_tau, 0.0);
    for (int tau = 0; tau < max_tau; ++tau) {
        double sum = 0.0;
        int count = T - tau;
        for (int t = 0; t < count; ++t) {
            sum += (series[t] - mean) * (series[t + tau] - mean);
        }
        C[tau] = sum / count;
    }
    return C;
}

// === Cluster A (FTD-0093): sublattice + diagonal correlators ===

// Spatial flux-flux correlator restricted to a parity sub-lattice and
// (optionally) along a non-axial displacement direction. Required for
// the BCC band-edge spectrum measurement (PROTOCOL_BCC_SUBLATTICE_SPECTRUM
// §2). Mode AXIS reproduces axis-aligned displacements; FACE_DIAG averages
// over 12 face-diagonal unit displacements; BODY_DIAG averages over the 8
// body-diagonal unit displacements (the BCC neighbor set).
//
// `r` runs over INTEGER step counts; physical distance is r·1, r·√2, r·√3
// for AXIS, FACE_DIAG, BODY_DIAG respectively. The caller is responsible
// for that conversion if needed.
inline std::vector<double> spatial_flux_correlation_sublattice(
    const RenderBridge& rb,
    SiteClass site_filter,
    DisplacementMode mode = DisplacementMode::AXIS,
    int max_r = -1)
{
    const auto& lat = rb.lattice();
    const auto& vox = rb.voxels();
    int L = lat.size();
    if (max_r < 0 || max_r > L / 2) max_r = L / 2;

    // Displacement unit vectors per mode.
    struct Disp { int dx, dy, dz; };
    std::vector<Disp> dirs;
    switch (mode) {
        case DisplacementMode::AXIS:
            dirs = { {1,0,0}, {0,1,0}, {0,0,1} };
            break;
        case DisplacementMode::FACE_DIAG:
            // 12 unit vectors of the form (±1,±1,0) etc. Averaging over the
            // POSITIVE half (6 directions) suffices since the flux-flux
            // correlator is symmetric under r → -r.
            dirs = { {1,1,0},{1,-1,0},{1,0,1},{1,0,-1},{0,1,1},{0,1,-1} };
            break;
        case DisplacementMode::BODY_DIAG:
            // 8 corner directions; same parity argument → 4 representative dirs.
            dirs = { {1,1,1},{1,1,-1},{1,-1,1},{1,-1,-1} };
            break;
    }

    std::vector<double> C(max_r, 0.0);
    std::vector<long long> counts(max_r, 0);

    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                if (!site_matches_filter(classify_voxel(x, y, z), site_filter)) continue;
                int idx0 = lat.index(x, y, z);
                const Vec3& J0 = vox[idx0].flux;

                for (int r = 0; r < max_r; ++r) {
                    for (const auto& d : dirs) {
                        int xr = lat.wrap(x + r * d.dx);
                        int yr = lat.wrap(y + r * d.dy);
                        int zr = lat.wrap(z + r * d.dz);
                        // Optional second-leg filter: if user passed a non-ALL filter,
                        // also require the receiver voxel match. This isolates same-class
                        // BCC↔BCC correlations from BCC↔neighbor leakage.
                        if (!site_matches_filter(classify_voxel(xr, yr, zr), site_filter)) continue;
                        int idx_r = lat.index(xr, yr, zr);
                        C[r] += J0.dot(vox[idx_r].flux);
                        ++counts[r];
                    }
                }
            }
        }
    }

    for (int r = 0; r < max_r; ++r) {
        if (counts[r] > 0) C[r] /= static_cast<double>(counts[r]);
    }
    return C;
}

// Sum of flux-energy on selected sublattice. Building block for the
// spectrum-extraction time series ψ(t) = Σ_{i ∈ class} |J(i,t)|².
// (Cheaper than calling spatial_flux_correlation_sublattice when only the
// total is needed.)
inline double sum_flux_energy_sublattice(const RenderBridge& rb, SiteClass site_filter) {
    const auto& lat = rb.lattice();
    const auto& vox = rb.voxels();
    const int L = lat.size();
    double total = 0.0;
    for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
            for (int z = 0; z < L; ++z) {
                if (!site_matches_filter(classify_voxel(x, y, z), site_filter)) continue;
                int idx = lat.index(x, y, z);
                total += vox[idx].flux.mag2();
            }
    return total;
}

}  // namespace ftd
