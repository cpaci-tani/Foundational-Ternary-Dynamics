#pragma once
/**
 * @file ftd/eft/manifestation_background.h
 * @brief Forced Poisson manifestation-injection background (Plan B, P2 protocol).
 *
 * Parallel to ftd::eft::prepare_thermal_background in coupling_measurement.h,
 * but instead of thermalizing wave_vel via Langevin, this places N random
 * signed charges at density n = N/L^3 and settles the flux field. Used by
 * the manifestation-scale-flow campaign to measure how density deforms the
 * bare Gaussian fixed point.
 */
#include <cstdint>
#include <cstdio>
#include <memory>
#include <random>
#include <utility>
#include <vector>

#include "ftd/eft/coupling_measurement.h"  // configure_bare_lattice_for_coupling
#include "ftd/render_bridge.h"

namespace ftd {
namespace eft {

/// Produce a manifestation-dressed RenderBridge. Caller owns the result.
///
/// Injects exactly N = floor(n * L^3) locked particles at distinct random
/// sites, with alternating signs so sum_Q = 0 exactly when N is even (if N
/// is odd, the final site is whichever sign closes the ledger, and sum_Q
/// is +-1; the caller should prefer even N).
///
/// Then settles the field for settle_ticks ticks with the bare-lattice
/// toggles (wave_propagation + coupling + gauss_projection), so the flux
/// field is the manifestation background we want to probe on top of.
///
/// Particles are placed on random sites but a rejection step skips any
/// site on the V(r) probe axis y = z = L/2. The test charges at
/// (mid, mid, mid) and (mid+r, mid, mid) sample the Green's function
/// along this line; any background charge anywhere on the axis biases it.
///
/// The BG settles with its own states locked; when the downstream probe
/// (measure_alpha_eff_on_bg, measure_kt_on_bg) creates a measurement
/// bridge, it copies only flux + wave_vel, leaving BG states behind.
/// This gives the clean "fluctuation dressing" of the flux field.
inline std::unique_ptr<RenderBridge> prepare_manifestation_background(
    int L, double density, uint64_t seed,
    int settle_ticks = 200,
    double initial_flux_z = 0.05)
{
    auto rb = std::make_unique<RenderBridge>(L);
    configure_bare_lattice_for_coupling(*rb);
    const int mid = L / 2;
    const int N_target = static_cast<int>(density * static_cast<double>(L) * L * L);

    // Reproducible uniform sampling of distinct sites.
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<int> uni(0, L - 1);

    std::vector<uint8_t> occupied(static_cast<size_t>(L) * L * L, 0);
    std::vector<int> placed_indices;
    placed_indices.reserve(static_cast<size_t>(N_target));
    int placed = 0;
    int attempts = 0;
    const int max_attempts = N_target * 100 + 1000;  // avoid infinite loops at high n
    int8_t next_sign = +1;
    while (placed < N_target && attempts < max_attempts) {
        ++attempts;
        const int x = uni(rng);
        const int y = uni(rng);
        const int z = uni(rng);
        // Reject any site on the V(r) probe axis (y = z = mid). The test charges
        // at (mid, mid, mid) and (mid+r, mid, mid) sample the Green's function
        // along this line; any background charge anywhere on the axis biases it.
        if (y == mid && z == mid) continue;
        const size_t idx = static_cast<size_t>(x) * L * L + static_cast<size_t>(y) * L + z;
        if (occupied[idx]) continue;
        occupied[idx] = 1;
        rb->inject_particle(x, y, z, next_sign,
                            {0.0, 0.0, static_cast<double>(next_sign) * initial_flux_z});
        placed_indices.push_back(rb->lattice().index(x, y, z));
        next_sign = -next_sign;
        ++placed;
    }

    // Apply locked=true to all placed sites in a single batch. Calling voxels()
    // once ensures the GPU->host sync happens before we write, and sets
    // host_mutated_ so the next tick/run pushes the flags back to the device.
    {
        auto& vox = rb->voxels();
        for (int idx : placed_indices) vox[idx].locked = true;
    }

    if (placed < N_target) {
        std::fprintf(stderr,
            "[prepare_manifestation_background] placed %d/%d at L=%d density=%.3g "
            "(guard/collision budget exhausted)\n",
            placed, N_target, L, density);
    }

    // Settle the flux field under bare-lattice dynamics.
    if (settle_ticks > 0) rb->run(settle_ticks);
    return rb;
}

/// Report how many manifestations were actually placed (useful for logging
/// in high-density cases where the guard corridor reduces the count).
inline int count_manifested_sites(const RenderBridge& rb) {
    int c = 0;
    for (const auto& v : rb.voxels()) if (v.state != 0 && v.locked) ++c;
    return c;
}

/// One K_T extraction from a plane-wave dispersion probe on a prepared
/// background. Injects a small-amplitude plane wave along x at wavenumber
/// k = 2*pi*m/L for m in {1, 2, 3}, measures angular frequency omega(k) via
/// zero-crossings of a probe-site's flux.z, then fits K_T from
///     omega^2(k) = K_T * (sigma_18_axial(k) / 3)
/// where sigma_18_axial(kx) = sigma_18(kx, 0, 0) with ky = kz = 0.
///
/// The probe copies bg's flux/wave_vel into a FRESH RenderBridge, so the
/// background is not disturbed and the probe can be repeated.
///
/// Plane-wave dispersion probe for the transverse stiffness K_T on a
/// prepared background. See docstring of measure_kt_on_bg below.
///
/// Backend note: defaults to CPU (force_cpu=true) because a pre-existing
/// GPU stencil bug (kernels_stencil.cu:903 misaligned address) crashes on
/// settle+run with a fresh bridge that inherits empty or near-empty flux.
/// K_T measurement is cheap (few k values, short probes), so CPU cost is
/// minor. Pass force_cpu=false once the GPU bug is root-caused.
struct KtMeasurement {
    std::vector<std::pair<double, double>> k_omega;  // (k, omega) samples
    double K_T_fit = 0.0;
    double r2      = 0.0;
    bool   valid   = false;
};

/// Copy the flux + wave_vel fields (including L/R dual-substrate components)
/// from a settled background bridge `src` into a freshly-constructed bridge
/// `dst`. Used by measure_kt_on_bg and measure_potential_vp to probe a
/// prepared background without disturbing it — the probe runs on its own
/// bridge that inherits the field state but is free to evolve independently.
///
/// Notes:
///  - `dst` must be constructed with the same lattice size as `src`.
///  - `dst.voxels()` is called first, so under GPU backend this triggers a
///    pull from device -> host on `src` (sync_to_host) and marks `dst` as
///    host-mutated (the next tick/run on dst pushes the new field back).
///  - State, velocity, particle_id, locked, etc. are NOT copied — `dst`
///    keeps its own (empty) states. Only the field is transferred.
inline void copy_flux_and_wave_vel(const RenderBridge& src, RenderBridge& dst) {
    const auto& sv = src.voxels();
    auto& dv = dst.voxels();
    const size_t N = sv.size();
    for (size_t i = 0; i < N; ++i) {
        dv[i].flux      = sv[i].flux;
        dv[i].wave_vel  = sv[i].wave_vel;
        dv[i].flux_L    = sv[i].flux_L;
        dv[i].flux_R    = sv[i].flux_R;
        dv[i].wave_vel_L = sv[i].wave_vel_L;
        dv[i].wave_vel_R = sv[i].wave_vel_R;
    }
}

inline double sigma_18_axial(double kx) {
    // sigma_18(kx, 0, 0) with cos(ky) = cos(kz) = 1.
    //   = 4 - (2/3)(cos kx + 2) - (2/3)(2 cos kx + 1)
    return 4.0 - (2.0 / 3.0) * (std::cos(kx) + 2.0)
               - (2.0 / 3.0) * (2.0 * std::cos(kx) + 1.0);
}

inline KtMeasurement measure_kt_on_bg(const RenderBridge& bg,
                                      int n_ticks = 200,
                                      double amplitude = 1e-3,
                                      const std::vector<int>& m_values = {1, 2, 3},
                                      bool force_cpu = true) {
    KtMeasurement out;
    const int L = bg.lattice().size();
    if (L < 8) return out;
    constexpr double kPi = 3.14159265358979323846;

    for (int m : m_values) {
        const double k = 2.0 * kPi * static_cast<double>(m) / static_cast<double>(L);
        RenderBridge rb(L);
        if (force_cpu) rb.force_cpu();
        configure_bare_lattice_for_coupling(rb);
        rb.toggles.langevin = false;
        copy_flux_and_wave_vel(bg, rb);
        // Add a plane-wave perturbation to flux.z along x: J_z += A cos(k x).
        auto& vs = rb.voxels();
        for (int x = 0; x < L; ++x) {
            const double dJ = amplitude * std::cos(k * static_cast<double>(x));
            for (int y = 0; y < L; ++y)
                for (int z = 0; z < L; ++z)
                    vs[rb.lattice().index(x, y, z)].flux.z += dJ;
        }
        // Sample flux.z at a fixed probe site (x=0, y=mid, z=mid) every tick.
        const int mid = L / 2;
        const int probe = rb.lattice().index(0, mid, mid);
        std::vector<double> trace;
        trace.reserve(n_ticks + 1);
        trace.push_back(rb.voxels()[probe].flux.z);
        for (int t = 0; t < n_ticks; ++t) {
            rb.run(1);
            trace.push_back(rb.voxels()[probe].flux.z);
        }
        // Count sign changes. For a pure cos(omega t), period = 2 pi/omega;
        // 2 crossings per period, so crossings = t_total * omega / pi.
        int crossings = 0;
        for (size_t i = 1; i < trace.size(); ++i) {
            if ((trace[i - 1] > 0.0 && trace[i] <= 0.0) ||
                (trace[i - 1] < 0.0 && trace[i] >= 0.0)) ++crossings;
        }
        if (crossings < 2) continue;  // not enough oscillation; skip this k
        const double omega = static_cast<double>(crossings) * kPi
                             / static_cast<double>(n_ticks);
        out.k_omega.emplace_back(k, omega);
    }

    // Fit: omega^2 = K_T * (sigma_18_axial(k) / 3). Least squares in one variable.
    double num = 0.0, den = 0.0, sum_y = 0.0;
    for (const auto& [k, w] : out.k_omega) {
        const double x = sigma_18_axial(k) / 3.0;
        const double y = w * w;
        num += x * y;
        den += x * x;
        sum_y += y;
    }
    if (out.k_omega.size() >= 2 && den > 0.0) {
        out.K_T_fit = num / den;
        const int n = static_cast<int>(out.k_omega.size());
        const double ybar = sum_y / n;
        double ss_tot = 0.0, ss_res = 0.0;
        for (const auto& [k, w] : out.k_omega) {
            const double x = sigma_18_axial(k) / 3.0;
            const double y = w * w;
            const double yhat = out.K_T_fit * x;
            ss_tot += (y - ybar) * (y - ybar);
            ss_res += (y - yhat) * (y - yhat);
        }
        out.r2 = (ss_tot > 0.0) ? 1.0 - ss_res / ss_tot : 0.0;
        out.valid = std::isfinite(out.K_T_fit);
    }
    return out;
}

}  // namespace eft
}  // namespace ftd
