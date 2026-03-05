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

namespace ftd {

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

}  // namespace ftd
