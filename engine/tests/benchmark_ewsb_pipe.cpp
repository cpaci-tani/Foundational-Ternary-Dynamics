/**
 * @file benchmark_ewsb_pipe.cpp
 * @brief EWSB amplitude-threshold map — Phase E port of benchmark_ewsb_threshold_map.
 *
 * Replaces the 193-LOC direct benchmark with a ~50-LOC pipeline version.
 * Runs a sweep of initial flux amplitudes on L=32 and records the EWSB
 * condensate signature (⟨|J|⟩, state counts, charge imbalance, field
 * energy) via a single EwsbCondensateCount observable per amplitude.
 *
 * CLI:
 *   --quick     reduced ticks/amps for fast smoke test
 *   --cpu       force CPU backend (default on non-CUDA builds)
 *   --gpu       force GPU backend (default on CUDA builds)
 *   --L=N       lattice size (default 32)
 *   --ticks=N   ticks per amplitude (default 5000, quick: 500)
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "ftd/term_toggles.h"
#include "ftd/sim/backend_cpu.h"
#include "ftd/sim/pipeline.h"
#include "ftd/sim/observables/ewsb_condensate_count.h"

#ifdef FTD_ENABLE_CUDA
#  include "ftd/sim/backend_gpu.h"
#endif

using namespace ftd::sim;

enum class Backend { Cpu, Gpu };

static ftd::TermToggles ewsb_toggles() {
    ftd::TermToggles t{};
    t.wave_propagation = true;
    t.coupling = true;
    t.gauss_projection = true;
    t.genesis = true;       // the central toggle — lets manifestation fire
    t.damping = false;
    t.selective_damping = false;
    t.larmor_radiation = false;
    t.forces = false;
    t.lorentz_force = false;
    t.color_forces = false;
    t.movement = false;
    t.poisson_coulomb = false;
    t.dual_substrate = false;
    t.weak_transmutation = false;
    t.latency_field = false;
    return t;
}

template <typename B>
static void run_one_amp(int L, double amp, int n_ticks, int sample_every) {
    Pipeline<B> p(L);
    p.set_toggles(ewsb_toggles());

    // Coordinate-phase seed (matches benchmark_ewsb_threshold_map.cpp)
    for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
            for (int z = 0; z < L; ++z) {
                const double phase = (x + 2*y + 3*z) * 0.1;
                p.inject_flux(x, y, z,
                              {amp * std::cos(phase),
                               amp * std::sin(phase),
                               amp * std::cos(phase * 2.0)});
            }

    auto obs = std::make_shared<EwsbCondensateCount<B>>();
    // Measure at tick 0 (initial), then every `sample_every` ticks.
    obs->measure(p.state());
    p.observe_every(sample_every, obs);

    std::cerr << "  amp=" << amp << " L=" << L << " ticks=" << n_ticks << "\n";
    p.run(n_ticks);

    for (const auto& s : obs->history()) {
        std::cout << amp << "," << s.tick << ","
                  << std::setprecision(10) << s.mean_abs_J << ","
                  << std::setprecision(10) << s.field_energy << ","
                  << s.n_plus << "," << s.n_minus << "," << s.n_zero << ","
                  << s.imbalance() << "\n";
    }
    const auto& last = obs->history().back();
    std::cerr << "    tick " << last.tick
              << "  ⟨|J|⟩=" << last.mean_abs_J
              << "  N+=" << last.n_plus << "  N-=" << last.n_minus
              << "  imbalance=" << last.imbalance() << "\n";
}

int main(int argc, char** argv) {
    int L = 32;
    int n_ticks = 5000;
    int sample_every = 200;
    std::vector<double> amps = {0.50, 0.60, 0.70, 0.80, 0.90};
    bool quick = false;
    Backend backend = Backend::Cpu;
#ifdef FTD_ENABLE_CUDA
    backend = Backend::Gpu;
#endif

    for (int i = 1; i < argc; ++i) {
        std::string s = argv[i];
        if (s == "--quick") { quick = true; L = 16; n_ticks = 500; amps = {0.15, 0.80}; }
        else if (s == "--cpu") backend = Backend::Cpu;
        else if (s == "--gpu") backend = Backend::Gpu;
        else if (s.rfind("--L=", 0) == 0) L = std::atoi(s.c_str() + 4);
        else if (s.rfind("--ticks=", 0) == 0) n_ticks = std::atoi(s.c_str() + 8);
    }

    std::cerr << "================================================================\n";
    std::cerr << "  Pipeline EWSB amplitude threshold map\n";
    std::cerr << "  backend=" << (backend == Backend::Gpu ? "gpu" : "cpu")
              << " L=" << L << " ticks=" << n_ticks << (quick ? " (quick)" : "") << "\n";
    std::cerr << "================================================================\n";

    std::cout << "amp,tick,mean_abs_J,field_energy,n_plus,n_minus,n_zero,imbalance\n";

    for (double amp : amps) {
        std::cerr << "\n-- amp = " << amp << " --\n";
        if (backend == Backend::Gpu) {
#ifdef FTD_ENABLE_CUDA
            run_one_amp<BackendGpu>(L, amp, n_ticks, sample_every);
#else
            std::cerr << "ERROR: --gpu requested but FTD_ENABLE_CUDA not defined\n";
            return 1;
#endif
        } else {
            run_one_amp<BackendCpu>(L, amp, n_ticks, sample_every);
        }
    }
    return 0;
}
