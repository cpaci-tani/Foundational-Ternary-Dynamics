/**
 * @file benchmark_ewsb_threshold_map.cpp
 * @brief Gap-closure Ticket 4 / Day 2 Thread 1b — EWSB amplitude threshold map.
 *
 * The gap-closure Ticket 4 showed that at amp = 0.80 on L=16, the engine
 * produces a condensate: ⟨|J|⟩ triples and 62 charges emerge spontaneously.
 * This benchmark pins down the threshold by sweeping five amplitudes
 * {0.50, 0.60, 0.70, 0.80, 0.90} on a larger lattice (L=32) with a long
 * run (5000 ticks), producing a deterministic map of amp → {ρ_condensate,
 * N_charges, ⟨|J|⟩_f, stability_fraction}.
 *
 * At the end of each run we dump the post-condensation lattice state so
 * the Thread-3 spectroscopy analysis can extract the flux-flux correlator
 * and mass gap without re-running.
 *
 * Output
 * ------
 *   - CSV to stdout: per-amp per-tick trajectory + per-amp final statistics
 *   - Per-amp binary dump of {state, flux} arrays to
 *     `scripts/benchmarks/results/eft_day2/ewsb_amp_<amp>_final.bin`
 *
 * Runtime estimate: 5 amps × 5000 ticks at L=32 ≈ 25 minutes total.
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "ftd/render_bridge.h"
#include "ftd/voxel.h"

static void configure_ewsb_cold(ftd::RenderBridge& rb) {
    rb.toggles.wave_propagation = true;
    rb.toggles.coupling = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.genesis = true;
    rb.toggles.damping = false;
    rb.toggles.selective_damping = false;
    rb.toggles.larmor_radiation = false;
    rb.toggles.forces = false;
    rb.toggles.lorentz_force = false;
    rb.toggles.color_forces = false;
    rb.toggles.movement = false;
    rb.toggles.poisson_coulomb = false;
    rb.toggles.gravity = false;
    rb.toggles.dual_substrate = false;
    rb.toggles.weak_transmutation = false;
    rb.toggles.strong_force = false;
    rb.toggles.triad_binding = false;
    rb.toggles.pair_production = false;
    rb.toggles.exchange_force = false;
    rb.toggles.latency_field = false;
    rb.toggles.emergent_forces = false;
}

struct SnapshotStats {
    int tick = 0;
    double mean_abs_J = 0.0;
    double max_abs_J = 0.0;
    long long n_plus = 0;
    long long n_minus = 0;
    double total_field_energy = 0.0;
};

static SnapshotStats take_snapshot(const ftd::RenderBridge& rb, int tick) {
    SnapshotStats s;
    s.tick = tick;
    const auto& vox = rb.voxels();
    const int N = rb.lattice().total_sites();
    double sum_J = 0.0, max_J = 0.0, sum_E = 0.0;
    long long np = 0, nm = 0;
    for (int i = 0; i < N; ++i) {
        const double j = std::sqrt(vox[i].flux.dot(vox[i].flux));
        sum_J += j;
        if (j > max_J) max_J = j;
        sum_E += 0.5 * vox[i].flux.dot(vox[i].flux);
        if (vox[i].state > 0) ++np;
        else if (vox[i].state < 0) ++nm;
    }
    s.mean_abs_J = sum_J / static_cast<double>(N);
    s.max_abs_J = max_J;
    s.n_plus = np;
    s.n_minus = nm;
    s.total_field_energy = sum_E;
    return s;
}

static void dump_final_state(const ftd::RenderBridge& rb, const std::string& path) {
    std::ofstream f(path, std::ios::binary);
    if (!f) return;
    const int L = rb.lattice().size();
    const int N = rb.lattice().total_sites();
    const auto& vox = rb.voxels();
    // Header: int32 L, int32 N
    int32_t L32 = static_cast<int32_t>(L);
    int32_t N32 = static_cast<int32_t>(N);
    f.write(reinterpret_cast<const char*>(&L32), sizeof L32);
    f.write(reinterpret_cast<const char*>(&N32), sizeof N32);
    // Per voxel: int8 state, 3 × float flux  (ignore flux_L/R, velocity, etc.)
    for (int i = 0; i < N; ++i) {
        int8_t s = vox[i].state;
        f.write(reinterpret_cast<const char*>(&s), 1);
        float fx = static_cast<float>(vox[i].flux.x);
        float fy = static_cast<float>(vox[i].flux.y);
        float fz = static_cast<float>(vox[i].flux.z);
        f.write(reinterpret_cast<const char*>(&fx), sizeof fx);
        f.write(reinterpret_cast<const char*>(&fy), sizeof fy);
        f.write(reinterpret_cast<const char*>(&fz), sizeof fz);
    }
}

int main(int argc, char** argv) {
    int L = 32;
    int total_ticks = 5000;
    int sample_every = 200;
    std::vector<double> amps = {0.50, 0.60, 0.70, 0.80, 0.90};
    std::string dump_dir = "scripts/benchmarks/results/eft_day2";
    bool quick = false;

    for (int i = 1; i < argc; ++i) {
        std::string s(argv[i]);
        if (s == "--quick") { quick = true; L = 16; total_ticks = 500; }
        else if (s.rfind("--L=", 0) == 0) L = std::atoi(s.c_str() + 4);
        else if (s.rfind("--ticks=", 0) == 0) total_ticks = std::atoi(s.c_str() + 8);
        else if (s.rfind("--dump=", 0) == 0) dump_dir = s.substr(7);
    }

    std::cerr << "================================================================\n";
    std::cerr << "  EWSB Amplitude Threshold Map  (Gap-Closure Ticket 4 / Day 2 T1b)\n";
    std::cerr << "  L = " << L << "  total_ticks = " << total_ticks
              << "  sample every = " << sample_every << "\n";
    std::cerr << "  dump dir = " << dump_dir << "\n";
    std::cerr << "================================================================\n";

    std::cout << "amp,tick,mean_abs_J,max_abs_J,n_plus,n_minus,total_charge,field_energy\n";

    for (double amp : amps) {
        std::cerr << "\n-- amp = " << amp << " --\n";
        ftd::RenderBridge rb(L);
        configure_ewsb_cold(rb);

        // Coordinate-phase "bare vacuum" seed (same pattern as original Phase 4A
        // but with tunable amplitude).
        for (int x = 0; x < L; ++x)
            for (int y = 0; y < L; ++y)
                for (int z = 0; z < L; ++z) {
                    const double phase = (x + 2*y + 3*z) * 0.1;
                    rb.inject_flux(x, y, z,
                                   {amp * std::cos(phase),
                                    amp * std::sin(phase),
                                    amp * std::cos(phase * 2.0)});
                }

        SnapshotStats s0 = take_snapshot(rb, 0);
        std::cout << amp << "," << s0.tick << "," << std::setprecision(10)
                  << s0.mean_abs_J << "," << s0.max_abs_J << ","
                  << s0.n_plus << "," << s0.n_minus << ","
                  << (s0.n_plus + s0.n_minus) << "," << s0.total_field_energy << "\n";

        for (int t = 1; t <= total_ticks; ++t) {
            rb.tick();
            if (t % sample_every == 0 || t == total_ticks) {
                auto s = take_snapshot(rb, t);
                std::cout << amp << "," << s.tick << "," << std::setprecision(10)
                          << s.mean_abs_J << "," << s.max_abs_J << ","
                          << s.n_plus << "," << s.n_minus << ","
                          << (s.n_plus + s.n_minus) << "," << s.total_field_energy << "\n";
                std::cerr << "    tick " << t
                          << "  ⟨|J|⟩=" << s.mean_abs_J
                          << "  N+=" << s.n_plus << "  N-=" << s.n_minus
                          << "  E_field=" << s.total_field_energy << "\n";
            }
        }

        // Final binary dump
        std::ostringstream os;
        os << dump_dir << "/ewsb_amp_" << std::fixed << std::setprecision(2)
           << amp << "_L" << L << "_final.bin";
        dump_final_state(rb, os.str());
        std::cerr << "    dumped: " << os.str() << "\n";
    }

    std::puts("================================================================");
    std::puts("  EWSB threshold map complete.");
    return 0;
}
