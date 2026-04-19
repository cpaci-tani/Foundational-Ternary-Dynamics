/**
 * @file test_sim_parity.cpp
 * @brief Phase C exit gate — GPU vs CPU parity on 3 reference observables.
 *
 * Strategy: for each observable, run identical pipeline programs on
 * BackendCpu and BackendGpu (same L, same seed, same scenario, same
 * tick count) and compare result_host() values.
 *
 * Pre-registered bars (SPEC plan):
 *   - After 1 tick (or measurement before any dynamics):  |Δ| / |CPU|  ≤  1e-3  (permille)
 *   - After long runs (≥ 500 ticks):                      |Δ| / |CPU|  ≤  1e-2  (1%)
 *
 * The second bar is looser because fp32 vs fp64, reduction-order
 * divergence, and chaotic dynamics combine to drift the two backends
 * apart over many ticks. Both bars are honest — neither is tight
 * enough to force bit-exactness.
 *
 * Skip behaviour: if FTD_ENABLE_CUDA is undefined, this file compiles
 * to a stub `main` that prints SKIP and returns 0 (so CMake always has
 * a green CTest entry). On CUDA builds the full parity sweep runs.
 */

#include <cmath>
#include <cstdio>
#include <memory>

#include "ftd/sim/pipeline.h"
#include "ftd/sim/backend_cpu.h"

#ifdef FTD_ENABLE_CUDA
#  include "ftd/sim/backend_gpu.h"
#endif

#include "ftd/sim/observables/total_field_energy.h"
#include "ftd/sim/observables/mean_abs_flux.h"
#include "ftd/sim/observables/state_histogram.h"

using namespace ftd::sim;

static int g_failures = 0;
static void check(const char* name, bool ok, const char* detail = nullptr) {
    if (ok) std::printf("  PASS  %s\n", name);
    else {
        std::printf("  FAIL  %s%s%s\n", name,
                    detail ? "  " : "", detail ? detail : "");
        ++g_failures;
    }
}

#ifdef FTD_ENABLE_CUDA

/// Configure two pipelines identically. Returns toggles used for the run.
static ftd::TermToggles canonical_scenario() {
    ftd::TermToggles t{};
    t.wave_propagation = true;
    t.gauss_projection = true;
    t.coupling = true;
    t.damping = false;
    t.selective_damping = false;
    t.genesis = false;
    t.larmor_radiation = false;
    t.forces = false;
    t.lorentz_force = false;
    t.color_forces = false;
    t.movement = false;
    t.poisson_coulomb = false;
    t.gravity = false;
    return t;
}

/// Seed a Gaussian flux pulse at the lattice centre.
template <typename Pipe>
static void seed_gaussian_pulse(Pipe& p, double amp = 1.0, double sigma = 2.0) {
    const int L = p.L();
    const int mid = L / 2;
    for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
            for (int z = 0; z < L; ++z) {
                const double dx = x - mid, dy = y - mid, dz = z - mid;
                const double r2 = dx*dx + dy*dy + dz*dz;
                const double g = amp * std::exp(-r2 / (2.0 * sigma * sigma));
                p.inject_flux(x, y, z, {g, g * 0.5, -g * 0.3});
            }
}

// Parity Check 1: TotalFieldEnergy at tick 0 (pre-dynamics, should match exactly)
static void par1_energy_initial() {
    std::puts("\n--- PAR1: TotalFieldEnergy at tick 0 (expect 1e-3 permille) ---");
    const int L = 16;
    Pipeline<BackendCpu> p_cpu(L);
    Pipeline<BackendGpu> p_gpu(L);

    p_cpu.set_toggles(canonical_scenario());
    p_gpu.set_toggles(canonical_scenario());
    seed_gaussian_pulse(p_cpu);
    seed_gaussian_pulse(p_gpu);

    auto obs_cpu = std::make_shared<TotalFieldEnergy<BackendCpu>>();
    auto obs_gpu = std::make_shared<TotalFieldEnergy<BackendGpu>>();
    // Measure directly (no run) — both observe the initial seeded state
    obs_cpu->measure(p_cpu.state());
    obs_gpu->measure(p_gpu.state());

    const double e_cpu = obs_cpu->result_host();
    const double e_gpu = obs_gpu->result_host();
    const double rel = std::abs(e_cpu - e_gpu) / std::max(std::abs(e_cpu), 1e-30);

    char buf[160];
    std::snprintf(buf, sizeof buf, "(CPU=%.6e GPU=%.6e rel=%.3e)",
                  e_cpu, e_gpu, rel);
    check("PAR1 TotalFieldEnergy parity ≤ 1e-3 at tick 0", rel < 1e-3, buf);
}

// Parity Check 2: MeanAbsFlux at tick 0
static void par2_mean_abs_flux_initial() {
    std::puts("\n--- PAR2: MeanAbsFlux at tick 0 (expect 1e-3 permille) ---");
    const int L = 16;
    Pipeline<BackendCpu> p_cpu(L);
    Pipeline<BackendGpu> p_gpu(L);
    p_cpu.set_toggles(canonical_scenario());
    p_gpu.set_toggles(canonical_scenario());
    seed_gaussian_pulse(p_cpu);
    seed_gaussian_pulse(p_gpu);

    auto obs_cpu = std::make_shared<MeanAbsFlux<BackendCpu>>();
    auto obs_gpu = std::make_shared<MeanAbsFlux<BackendGpu>>();
    obs_cpu->measure(p_cpu.state());
    obs_gpu->measure(p_gpu.state());

    const double m_cpu = obs_cpu->result_host();
    const double m_gpu = obs_gpu->result_host();
    const double rel = std::abs(m_cpu - m_gpu) / std::max(std::abs(m_cpu), 1e-30);

    char buf[160];
    std::snprintf(buf, sizeof buf, "(CPU=%.6e GPU=%.6e rel=%.3e)", m_cpu, m_gpu, rel);
    check("PAR2 MeanAbsFlux parity ≤ 1e-3 at tick 0", rel < 1e-3, buf);
}

// Parity Check 3: StateHistogram exact match after particle injections
static void par3_state_histogram_exact() {
    std::puts("\n--- PAR3: StateHistogram exact match (integer counts) ---");
    const int L = 16;
    Pipeline<BackendCpu> p_cpu(L);
    Pipeline<BackendGpu> p_gpu(L);
    p_cpu.set_toggles(canonical_scenario());
    p_gpu.set_toggles(canonical_scenario());
    // Inject same particles on both backends
    p_cpu.inject_particle(3, 3, 3, +1, {0.0, 0.0, 0.0});
    p_cpu.inject_particle(5, 5, 5, +1, {0.0, 0.0, 0.0});
    p_cpu.inject_particle(7, 7, 7, -1, {0.0, 0.0, 0.0});
    p_gpu.inject_particle(3, 3, 3, +1, {0.0, 0.0, 0.0});
    p_gpu.inject_particle(5, 5, 5, +1, {0.0, 0.0, 0.0});
    p_gpu.inject_particle(7, 7, 7, -1, {0.0, 0.0, 0.0});

    auto obs_cpu = std::make_shared<StateHistogram<BackendCpu>>();
    auto obs_gpu = std::make_shared<StateHistogram<BackendGpu>>();
    obs_cpu->measure(p_cpu.state());
    obs_gpu->measure(p_gpu.state());

    const auto c_cpu = obs_cpu->result_host();
    const auto c_gpu = obs_gpu->result_host();

    char buf[192];
    std::snprintf(buf, sizeof buf,
                  "(CPU: n+=%lld n-=%lld; GPU: n+=%lld n-=%lld)",
                  c_cpu.n_plus, c_cpu.n_minus, c_gpu.n_plus, c_gpu.n_minus);
    check("PAR3 StateHistogram n_plus matches",   c_cpu.n_plus == c_gpu.n_plus, buf);
    check("PAR3 StateHistogram n_minus matches",  c_cpu.n_minus == c_gpu.n_minus, buf);
    check("PAR3 StateHistogram n_zero matches",   c_cpu.n_zero == c_gpu.n_zero, buf);
}

// Parity Check 4: TotalFieldEnergy after 100 ticks of free-field evolution
static void par4_energy_after_100() {
    std::puts("\n--- PAR4: TotalFieldEnergy after 100 ticks (expect ≤1% drift) ---");
    const int L = 16;
    Pipeline<BackendCpu> p_cpu(L);
    Pipeline<BackendGpu> p_gpu(L);
    p_cpu.set_toggles(canonical_scenario());
    p_gpu.set_toggles(canonical_scenario());
    seed_gaussian_pulse(p_cpu);
    seed_gaussian_pulse(p_gpu);

    auto obs_cpu = std::make_shared<TotalFieldEnergy<BackendCpu>>();
    auto obs_gpu = std::make_shared<TotalFieldEnergy<BackendGpu>>();
    p_cpu.observe_at(100, obs_cpu);
    p_gpu.observe_at(100, obs_gpu);
    p_cpu.run(100);
    p_gpu.run(100);

    const double e_cpu = obs_cpu->result_host();
    const double e_gpu = obs_gpu->result_host();
    const double rel = std::abs(e_cpu - e_gpu) / std::max(std::abs(e_cpu), 1e-30);

    char buf[160];
    std::snprintf(buf, sizeof buf, "(CPU=%.6e GPU=%.6e rel=%.3e)", e_cpu, e_gpu, rel);
    check("PAR4 TotalFieldEnergy parity ≤ 1e-2 after 100 ticks", rel < 1e-2, buf);
}

// Parity Check 5: TotalFieldEnergy after 500 ticks (the long-run 1% bar)
static void par5_energy_after_500() {
    std::puts("\n--- PAR5: TotalFieldEnergy after 500 ticks (long-run 1% bar) ---");
    const int L = 16;
    Pipeline<BackendCpu> p_cpu(L);
    Pipeline<BackendGpu> p_gpu(L);
    p_cpu.set_toggles(canonical_scenario());
    p_gpu.set_toggles(canonical_scenario());
    seed_gaussian_pulse(p_cpu);
    seed_gaussian_pulse(p_gpu);

    auto obs_cpu = std::make_shared<TotalFieldEnergy<BackendCpu>>();
    auto obs_gpu = std::make_shared<TotalFieldEnergy<BackendGpu>>();
    p_cpu.observe_at(500, obs_cpu);
    p_gpu.observe_at(500, obs_gpu);
    p_cpu.run(500);
    p_gpu.run(500);

    const double e_cpu = obs_cpu->result_host();
    const double e_gpu = obs_gpu->result_host();
    const double rel = std::abs(e_cpu - e_gpu) / std::max(std::abs(e_cpu), 1e-30);

    char buf[160];
    std::snprintf(buf, sizeof buf, "(CPU=%.6e GPU=%.6e rel=%.3e)", e_cpu, e_gpu, rel);
    check("PAR5 TotalFieldEnergy parity ≤ 1e-2 after 500 ticks", rel < 1e-2, buf);
}

int main() {
    std::puts("================================================================");
    std::puts("  Sim Pipeline — Phase C GPU/CPU Parity Tests (CUDA enabled)");
    std::puts("================================================================");
    par1_energy_initial();
    par2_mean_abs_flux_initial();
    par3_state_histogram_exact();
    par4_energy_after_100();
    par5_energy_after_500();
    std::puts("\n----------------------------------------------------------------");
    if (g_failures == 0) {
        std::puts("  All Phase-C parity checks PASS");
        return 0;
    }
    std::printf("  %d Phase-C parity check(s) FAILED\n", g_failures);
    return 1;
}

#else  // !FTD_ENABLE_CUDA

int main() {
    std::puts("================================================================");
    std::puts("  Sim Pipeline — Phase C Parity Tests");
    std::puts("  SKIP: FTD_ENABLE_CUDA not defined; no GPU backend to test.");
    std::puts("================================================================");
    return 0;  // not a failure
}

#endif
