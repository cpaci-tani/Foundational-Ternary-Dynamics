// GPU energy-ledger gap regression (FTD engine, 2026-08-20).
//
// Root cause fixed: GpuBackend::tick() returns early in interactive_gpu_mode_
// without syncing the host AoS shadow (voxels_), so update_energy_ledger_cpu()
// summed stale/zero host data and reported energy_ledger().E_curr = 0 on the
// interactive GPU path (the native app default). The fix sources the ledger
// from the compact device-side energy_audit() reduction on that path.
//
// This test asserts two things:
//   (1) Convention mapping — on a FRESH host shadow (CPU), the ledger's E_curr
//       equals field_energy + wave_energy + particle_ke (+ strong) reconstructed
//       from energy_audit(). This proves the channel mapping the interactive-GPU
//       path relies on is exact (V_cell = 1 makes the audit sums == host sums).
//   (2) The symptom — interactive-GPU energy_ledger().E_curr is now NON-ZERO and
//       agrees with the CPU reference to tight tolerance for the same scenario.

#include "ftd/render_bridge.h"
#include "ftd/backend.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void check(const std::string& name, bool ok) {
    if (!ok) {
        ++failures;
        std::cerr << "FAIL: " << name << '\n';
    }
}

void close(const std::string& name, double a, double b,
           double rel = 1e-9, double abs = 1e-9) {
    const double tol = std::max(abs, rel * std::max(std::abs(a), std::abs(b)));
    const bool ok = std::isfinite(a) && std::isfinite(b) && std::abs(a - b) <= tol;
    if (!ok) {
        ++failures;
        std::cerr << "FAIL: " << name << "  a=" << a << " b=" << b
                  << " |d|=" << std::abs(a - b) << " tol=" << tol << '\n';
    }
}

// Deterministic scenario (no genesis RNG, no movement): standing flux/wave
// pattern plus a few static particles carrying velocity. Field + wave channels
// evolve under the wave equation + damping; particle_ke stays finite. Exercises
// all three ledger channels while remaining bit-reproducible across backends.
void populate(ftd::RenderBridge& rb) {
    const int L = rb.lattice().size();
    auto& voxels = rb.voxels();
    for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
    for (int z = 0; z < L; ++z) {
        const int i = rb.lattice().index(x, y, z);
        auto& v = voxels[static_cast<std::size_t>(i)];
        const double q = static_cast<double>((x * 7 + y * 5 + z * 3) % 17 - 8) / 41.0;
        v.flux = {q + 0.01 * x, -0.7 * q + 0.005 * y, 0.4 * q - 0.003 * z};
        v.wave_vel = {-0.12 * q, 0.08 * q + 0.002 * z, -0.04 * q};
        if ((i % 37) == 0) {
            v.state = (i % 2 == 0) ? 1 : -1;
            v.particle_id = i;
            v.velocity = {0.03 * ((i % 3) - 1), 0.02 * ((i % 5) - 2),
                          0.01 * ((i % 7) - 3)};
        }
    }
}

void configure(ftd::RenderBridge& rb) {
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.coupling = true;
    rb.toggles.damping = true;
    // gauss_projection is deliberately OFF for the cross-backend leg: the CPU
    // path uses the SOR Poisson solver and the GPU path uses the cuFFT solver,
    // which agree only to ~sub-percent (a pre-existing algorithmic difference,
    // unrelated to this fix). Leaving it off isolates the ledger-sourcing
    // mechanism: per-site wave + damping updates are identical arithmetic on
    // both backends (modulo FMA), so E_curr agrees to near machine precision.
    // Pin the controls disable_all() intentionally preserves. Single substrate
    // (populate() writes only flux, not flux_L/R); no genesis RNG; no movement
    // -> particle_ke stays constant. Fully deterministic across backends.
    rb.toggles.dual_substrate = false;
    rb.toggles.genesis = false;
    rb.toggles.movement = false;
}

}  // namespace

int main() {
    constexpr int L = 12;
    constexpr int TICKS = 8;

    // ── CPU reference (fresh host shadow every tick) ────────────────────────
    ftd::RenderBridge cpu(L);
    cpu.force_cpu();
    configure(cpu);
    cpu.seed_rng(1234);
    populate(cpu);
    for (int t = 0; t < TICKS; ++t) cpu.tick();

    const double e_cpu = cpu.energy_ledger().E_curr;

    // (1) Convention mapping: reconstruct E_total from the audit channels on the
    // SAME fresh state and confirm it matches the ledger's E_curr. This is the
    // exact mapping the interactive-GPU path uses (only the energy *source*
    // differs there — device reduction vs host sum).
    const ftd::EnergyAudit a_cpu = cpu.energy_audit();
    double e_recon = a_cpu.field_energy + a_cpu.wave_energy + a_cpu.particle_ke;
    if (cpu.toggles.strong_stress_energy) e_recon += a_cpu.strong_potential_energy;
    check("CPU ledger E_curr is non-trivial", std::abs(e_cpu) > 1e-6);
    close("CPU ledger E_curr == audit(field+wave+ke)", e_cpu, e_recon, 1e-9, 1e-9);
    check("CPU ledger updated once per tick",
          cpu.energy_ledger().updates == static_cast<std::uint64_t>(TICKS));

    // ── Interactive GPU (deferred host mirror) ──────────────────────────────
    ftd::RenderBridge gpu(L);
    gpu.set_interactive_gpu_mode(true);
    const bool is_gpu = gpu.backend_kind() == ftd::Backend::Kind::Gpu;

    configure(gpu);
    gpu.seed_rng(1234);
    populate(gpu);
    for (int t = 0; t < TICKS; ++t) gpu.tick();

    const double e_gpu = gpu.energy_ledger().E_curr;

    if (is_gpu) {
        // (2) The symptom: E_curr is live (non-zero) on the interactive GPU
        // path. Before the fix, GpuBackend::tick() returned without syncing the
        // host shadow and update_energy_ledger_cpu summed zeros -> E_curr = 0.
        check("interactive-GPU ledger E_curr is NON-ZERO (symptom fixed)",
              std::abs(e_gpu) > 1e-6);
        check("interactive-GPU ledger updated once per tick",
              gpu.energy_ledger().updates == static_cast<std::uint64_t>(TICKS));

        // (3) Direct proof the ledger is now device-sourced: on the SAME
        // (unchanged) device state, energy_ledger().E_curr must equal the
        // energy_audit() channel reconstruction. Both come from the identical
        // compact device reduction, so this is near bit-exact.
        const ftd::EnergyAudit a_gpu = gpu.energy_audit();
        double e_recon_gpu = a_gpu.field_energy + a_gpu.wave_energy + a_gpu.particle_ke;
        if (gpu.toggles.strong_stress_energy) e_recon_gpu += a_gpu.strong_potential_energy;
        close("interactive-GPU E_curr == device audit(field+wave+ke)",
              e_gpu, e_recon_gpu, 1e-12, 1e-12);

        // (4) Cross-backend consistency: with the SOR/FFT solver difference
        // removed (gauss off), CPU and interactive-GPU E_curr agree to near
        // machine precision for the same scenario+tick. The chart/readout does
        // not jump when toggling backends.
        close("CPU vs interactive-GPU ledger E_curr parity", e_cpu, e_gpu,
              1e-6, 1e-9);
        std::cerr << "[info] E_curr  cpu=" << e_cpu << "  gpu=" << e_gpu
                  << "  |rel|=" << std::abs(e_cpu - e_gpu)
                             / std::max(std::abs(e_cpu), 1e-300) << '\n';
    } else {
        std::cerr << "[skip] GPU backend unavailable (CPU-only or FTD_FORCE_CPU);"
                     " interactive-GPU parity leg skipped.\n";
    }

    if (failures == 0) {
        std::cout << "PASS: interactive-GPU energy ledger sources E_curr from the "
                     "device audit and matches the CPU reference "
                     "(convention mapping exact, symptom fixed)\n";
    }
    return failures == 0 ? 0 : 1;
}
