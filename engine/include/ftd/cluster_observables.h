#pragma once
/**
 * Per-cluster observables for the cluster-thermodynamics EXPLORATORY campaign.
 *
 * Exploratory instrumentation — NOT a derived FTD claim, NOT pre-registered.
 * See .claude/plans/lazy-conjuring-marble.md (the "dissipation-N" conjecture).
 *
 * measure_cluster() reduces one connected component (a list of voxel indices)
 * to: cardinality N, field/wave/kinetic/total energy, a flux-alignment
 * coherence R, the organization scalar org = N*R, the centroid, and the summed
 * flux vector (whose direction feeds the inter-cluster detuning proxy).
 *
 * Frozen definitions (per the plan):
 *   R   = |Σ J| / Σ|J|  ∈ (0,1]                 // Kuramoto-style order parameter
 *   org = N · R                                  // "coherent mass"  (fallback: org = N)
 *   detuning = acos( n̂_a · n̂_b )                // 0 = compatible, π = anti-aligned
 */

#include "render_bridge.h"
#include <vector>
#include <cmath>
#include <algorithm>

namespace ftd {

struct ClusterMeasure {
    int    size = 0;        // cardinality N
    double E_field = 0.0;   // Σ |J|²
    double E_wave  = 0.0;   // Σ |wave_vel|²  (thermostat throughput / maintenance-cost proxy)
    double E_kin   = 0.0;   // Σ 0.5 |velocity|²
    double E_total = 0.0;   // 0.5(E_field+E_wave)+E_kin   (mirrors EnergyAudit convention)
    double coherence = 0.0; // R = |Σ J| / Σ|J|  ∈ (0,1]
    double org = 0.0;       // organization = N · R
    Vec3   centroid;        // unweighted mean coord (no periodic wrap; ok for centred clusters)
    Vec3   flux_sum;        // Σ J  (direction = cluster mean-flux axis)
};

inline ClusterMeasure measure_cluster(const RenderBridge& rb,
                                      const std::vector<int>& idxs) {
    const auto& vox = rb.voxels();
    const auto& lat = rb.lattice();
    ClusterMeasure m;
    m.size = static_cast<int>(idxs.size());
    if (m.size == 0) return m;
    double sum_flux_mag = 0.0;
    double sx = 0.0, sy = 0.0, sz = 0.0;
    for (int i : idxs) {
        const Voxel& v = vox[i];
        m.E_field += v.flux.mag2();
        m.E_wave  += v.wave_vel.mag2();
        m.E_kin   += 0.5 * v.velocity.mag2();
        m.flux_sum += v.flux;
        sum_flux_mag += v.flux.mag();
        Coord c = lat.coord(i);
        sx += c.x; sy += c.y; sz += c.z;
    }
    m.E_total = 0.5 * (m.E_field + m.E_wave) + m.E_kin;
    m.coherence = (sum_flux_mag > 1e-12) ? (m.flux_sum.mag() / sum_flux_mag) : 0.0;
    m.org = m.size * m.coherence;
    m.centroid = Vec3(sx / m.size, sy / m.size, sz / m.size);
    return m;
}

// Inter-cluster detuning (the P2 compatibility axis): angle between two
// clusters' mean-flux directions. 0 = aligned/compatible, π = anti-aligned.
inline double detuning_proxy(const Vec3& flux_sum_a, const Vec3& flux_sum_b) {
    double ma = flux_sum_a.mag(), mb = flux_sum_b.mag();
    if (ma < 1e-12 || mb < 1e-12) return 0.0;
    double c = flux_sum_a.dot(flux_sum_b) / (ma * mb);
    c = std::max(-1.0, std::min(1.0, c));
    return std::acos(c);
}

}  // namespace ftd
