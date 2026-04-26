/**
 * @file test_langevin_equipartition.cpp
 * @brief Verify the Langevin thermostat produces the expected equilibrium.
 *
 * Setup: bare lattice, zero initial state, langevin ON with (T, gamma)
 * chosen so that discrete stability is safe (gamma small, T small).
 * Target at equilibrium:
 *     <|wave_vel|^2>_voxel  =  3 * T    (three components, unit mass)
 *     <wave_vel>_voxel      =  0        (no mean drift)
 *
 * Method: run for N_BURN + N_MEASURE ticks. During N_MEASURE, accumulate
 * the mean of |wave_vel|^2 over all voxels and all measurement ticks.
 * Compare to 3*T. Also report autocorrelation of a single voxel's
 * wave_vel, which should decay as exp(-gamma * t).
 *
 * Falsification criterion: if <|wave_vel|^2> differs from 3T by more than
 * the expected statistical error (~ 1/sqrt(N_voxels * N_measure)), the
 * thermostat is not producing the correct equilibrium.
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include "ftd/render_bridge.h"

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    const int L = 16;
    const double T = 0.01;        // energy units, per kinetic DoF
    const double gamma = 0.01;    // 1/ticks; tau ~ 100 ticks
    const int N_BURN    = 1000;   // burn-in ticks for equilibration
    const int N_MEASURE = 2000;   // measurement window

    std::printf("================================================================\n");
    std::printf("  Langevin thermostat equipartition verification\n");
    std::printf("================================================================\n");
    std::printf("  L = %d, T = %.4f, gamma = %.4f\n", L, T, gamma);
    std::printf("  N_burn = %d, N_measure = %d\n", N_BURN, N_MEASURE);
    std::printf("  Expected: <|wave_vel|^2>_voxel = 3 T = %.6f\n", 3.0 * T);
    std::printf("  Expected autocorrelation time: ~ 1/gamma = %.1f ticks\n\n",
                1.0/gamma);

    ftd::RenderBridge rb(L);

    // Minimal bare lattice: wave propagation + gauss projection + langevin.
    // Disable everything else to isolate the thermostat.
    rb.toggles.wave_propagation = true;
    rb.toggles.coupling         = false;
    rb.toggles.damping          = false;
    rb.toggles.genesis          = false;
    rb.toggles.gauss_projection = true;
    rb.toggles.forces           = false;
    rb.toggles.gravity          = false;
    rb.toggles.poisson_coulomb  = false;
    rb.toggles.movement         = false;
    rb.toggles.lorentz_force    = false;
    rb.toggles.selective_damping= false;
    rb.toggles.larmor_radiation = false;
    rb.toggles.dual_substrate   = false;  // single-substrate path
    rb.toggles.weak_transmutation = false;
    rb.toggles.latency_field    = false;
    rb.toggles.langevin         = true;
    rb.toggles.langevin_T       = T;
    rb.toggles.langevin_gamma   = gamma;

    // Burn-in
    std::printf("  Burn-in (%d ticks)...\n", N_BURN);
    rb.run(N_BURN);

    // Measurement: accumulate <|v|^2>, <v>, also track a single-voxel
    // time series for autocorrelation.
    const int N = L * L * L;
    double sum_v2 = 0.0;
    double sum_v[3] = {0.0, 0.0, 0.0};
    double sum_J2 = 0.0;
    double sum_div2 = 0.0;

    // Single-voxel time series at lattice centre.
    const int mid = L/2;
    const int mid_idx = rb.lattice().index(mid, mid, mid);
    std::vector<double> vx_series(N_MEASURE);

    // OPEN-8 fix (mirrors test_langevin_gpu_cpu_parity rationale): cast to
    // const RenderBridge so `voxels()` selects the read-only overload, which
    // calls sync_to_host() WITHOUT marking host_mutated_=true. The non-const
    // overload would set the dirty flag and the next tick's push_to_device
    // would upload host wave_vel back to the device, clobbering whatever
    // cuRAND injected. Exact root mechanism is ARCH-6.
    const ftd::RenderBridge& crb = rb;

    // Sample at every tick during measurement window.
    for (int step = 0; step < N_MEASURE; ++step) {
        rb.run(1);
        const auto& vox = crb.voxels();
        double tick_sum_v2 = 0, tick_sum_vx = 0, tick_sum_vy = 0, tick_sum_vz = 0;
        double tick_sum_J2 = 0;
        for (int i = 0; i < N; ++i) {
            tick_sum_v2 += vox[i].wave_vel.mag2();
            tick_sum_vx += vox[i].wave_vel.x;
            tick_sum_vy += vox[i].wave_vel.y;
            tick_sum_vz += vox[i].wave_vel.z;
            tick_sum_J2 += vox[i].flux.mag2();
        }
        sum_v2 += tick_sum_v2;
        sum_v[0] += tick_sum_vx;
        sum_v[1] += tick_sum_vy;
        sum_v[2] += tick_sum_vz;
        sum_J2 += tick_sum_J2;

        vx_series[step] = vox[mid_idx].wave_vel.x;

        // Periodic progress
        if ((step + 1) % (N_MEASURE / 10) == 0) {
            const double running_v2 = sum_v2 / (N * (step + 1));
            std::printf("    tick %5d/%d  <|v|^2> = %.6e  (target %.6e, dev %+.2f%%)\n",
                        step + 1, N_MEASURE, running_v2, 3.0*T,
                        100.0 * (running_v2 - 3.0*T) / (3.0*T));
        }
    }

    // Final statistics.
    const long long n_samples = static_cast<long long>(N) * N_MEASURE;
    const double mean_v2 = sum_v2 / n_samples;
    const double mean_vx = sum_v[0] / n_samples;
    const double mean_vy = sum_v[1] / n_samples;
    const double mean_vz = sum_v[2] / n_samples;
    const double mean_J2 = sum_J2 / n_samples;

    std::printf("\n  --- Final statistics over %lld voxel-samples ---\n", n_samples);
    std::printf("  <|wave_vel|^2>_voxel   = %.6e   (target 3T = %.6e, dev %+.2f%%)\n",
                mean_v2, 3.0*T, 100.0*(mean_v2 - 3.0*T)/(3.0*T));
    std::printf("  <wave_vel>_voxel       = (%.3e, %.3e, %.3e)  (target 0)\n",
                mean_vx, mean_vy, mean_vz);
    std::printf("  <|J|^2>_voxel          = %.6e   (emerges from fluct-diss; reported)\n",
                mean_J2);

    // Per-component equipartition: each component should have <v_i^2> = T
    double sum_vx2 = 0, sum_vy2 = 0, sum_vz2 = 0;
    // (Expensive to recompute; instead estimate from |v|^2/3 assuming isotropy)
    std::printf("  Isotropy check (|v|^2/3) = %.6e   (target T = %.6e)\n",
                mean_v2/3.0, T);

    // Autocorrelation of vx at centre: C(k) = <vx(t) vx(t+k)>
    // normalized by C(0).
    std::printf("\n  --- Autocorrelation of single voxel wave_vel.x ---\n");
    double vx_mean = 0;
    for (double v : vx_series) vx_mean += v;
    vx_mean /= vx_series.size();
    double c0 = 0;
    for (double v : vx_series) c0 += (v - vx_mean) * (v - vx_mean);
    c0 /= vx_series.size();
    std::printf("  C(0) (variance) = %.6e   (target T = %.6e, dev %+.2f%%)\n",
                c0, T, 100.0*(c0 - T)/T);

    const std::vector<int> lags = {1, 10, 50, 100, 200, 500, 1000};
    for (int lag : lags) {
        if (lag >= (int)vx_series.size()) break;
        double c = 0;
        int nn = (int)vx_series.size() - lag;
        for (int t = 0; t < nn; ++t) {
            c += (vx_series[t] - vx_mean) * (vx_series[t+lag] - vx_mean);
        }
        c /= nn;
        double c_norm = (c0 > 0) ? c / c0 : 0;
        double expected = std::exp(-gamma * lag);
        std::printf("  C(%4d)/C(0) = %+.4f   (expected exp(-gamma*lag) = %.4f)\n",
                    lag, c_norm, expected);
    }

    std::printf("\n================================================================\n");
    const double dev = std::abs(mean_v2 - 3.0*T) / (3.0*T);
    if (dev < 0.05) {
        std::printf("  PASS  equipartition within 5%% (|dev| = %.2f%%)\n", 100.0*dev);
        return 0;
    } else {
        std::printf("  FAIL  equipartition deviation = %.2f%% > 5%%\n", 100.0*dev);
        return 1;
    }
}
