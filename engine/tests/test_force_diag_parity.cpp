// test_force_diag_parity.cpp
//
// CPU-vs-GPU parity test for the force_diag mirror added 2026-04-25.
//
// Background: GpuBackend::sync_to_host() previously left
// RenderBridge::force_diag_ untouched, so any test that called
// force_diag_at(...) after tick() on the GPU backend got default-zero
// values. This test reproduces the access pattern used by
// test_asymptotic_freedom and test_confinement (locked color particles
// at fixed offsets, single tick, read f_strong) and asserts the GPU
// path now reports a non-zero force that matches the CPU path within a
// generous tolerance (FFT vs SOR + f32 vs f64).

#include "ftd/render_bridge.h"
#include <cmath>
#include <cstdio>
#include <iostream>

using namespace ftd;

static int failures = 0;

static void check_close(const char* name, double a, double b, double tol) {
    double diff = std::fabs(a - b);
    if (diff > tol) {
        std::printf("  FAIL  %-44s: |a-b|=%.3e tol=%.3e (a=%.6e, b=%.6e)\n",
                    name, diff, tol, a, b);
        ++failures;
    } else {
        std::printf("  PASS  %-44s: |a-b|=%.3e tol=%.3e\n",
                    name, diff, tol);
    }
}

static void check_nonzero(const char* name, double v) {
    if (std::fabs(v) < 1e-10) {
        std::printf("  FAIL  %-44s: value=%.3e (expected non-zero)\n", name, v);
        ++failures;
    } else {
        std::printf("  PASS  %-44s: value=%.3e\n", name, v);
    }
}

struct Sample {
    Vec3 f_strong_at_target;
    Vec3 f_coulomb_at_target;
};

// Reproduces the geometry of test_asymptotic_freedom::measure_effective_coupling:
// two locked colored particles separated by r along the x axis. Reads forces
// on the second particle.
static Sample run_color_scenario(bool force_cpu_path, int r_sep) {
    const int L = 32;
    int mid = L / 2;
    RenderBridge bridge(L);
    if (force_cpu_path) bridge.force_cpu();

    bridge.toggles.disable_all();
    bridge.toggles.forces = true;
    bridge.toggles.color_forces = true;
    bridge.toggles.strong_force = true;
    // Also enable Poisson Coulomb so the same kernel populates f_coulomb,
    // exercising the second mirror path (phase_forces_kernel writes
    // f_coulomb / f_gravity / f_magnetic; color_force_kernel writes f_strong).
    bridge.toggles.poisson_coulomb = true;
    bridge.toggles.gauss_projection = true;

    // Locked Red particle at center
    bridge.inject_particle(mid, mid, mid, +1, {K_B, 0, 0}, /*spin=*/0, /*color=*/1);
    bridge.voxels()[bridge.lattice().index(mid, mid, mid)].locked = true;

    // Locked Green particle at +r along x
    int tx = mid + r_sep;
    bridge.inject_particle(tx, mid, mid, +1, {0, K_B, 0}, /*spin=*/0, /*color=*/2);
    bridge.voxels()[bridge.lattice().index(tx, mid, mid)].locked = true;

    bridge.tick();

    Sample s;
    auto& fd = bridge.force_diag_at(tx, mid, mid);
    s.f_strong_at_target  = fd.f_strong;
    s.f_coulomb_at_target = fd.f_coulomb;
    return s;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: force_diag CPU/GPU parity (color-force scenario)\n";
    std::cout << "================================================================\n";

    // r_sep = 4 keeps both particles inside the lattice and well within the
    // confinement radius — gives a robust f_strong magnitude without lattice-
    // boundary artifacts.
    const int r_sep = 4;

    Sample gpu = run_color_scenario(/*force_cpu_path=*/false, r_sep);
    Sample cpu = run_color_scenario(/*force_cpu_path=*/true,  r_sep);

    std::printf("CPU f_strong = (%.6e, %.6e, %.6e)  |F|=%.6e\n",
                cpu.f_strong_at_target.x, cpu.f_strong_at_target.y,
                cpu.f_strong_at_target.z, cpu.f_strong_at_target.mag());
    std::printf("GPU f_strong = (%.6e, %.6e, %.6e)  |F|=%.6e\n",
                gpu.f_strong_at_target.x, gpu.f_strong_at_target.y,
                gpu.f_strong_at_target.z, gpu.f_strong_at_target.mag());

    std::printf("CPU f_coulomb = (%.6e, %.6e, %.6e)  |F|=%.6e\n",
                cpu.f_coulomb_at_target.x, cpu.f_coulomb_at_target.y,
                cpu.f_coulomb_at_target.z, cpu.f_coulomb_at_target.mag());
    std::printf("GPU f_coulomb = (%.6e, %.6e, %.6e)  |F|=%.6e\n",
                gpu.f_coulomb_at_target.x, gpu.f_coulomb_at_target.y,
                gpu.f_coulomb_at_target.z, gpu.f_coulomb_at_target.mag());

    // Sanity: both backends must report a non-zero color force.
    check_nonzero("CPU |f_strong| > 0",                  cpu.f_strong_at_target.mag());
    check_nonzero("GPU |f_strong| > 0 (mirror wired)",   gpu.f_strong_at_target.mag());
    check_nonzero("CPU |f_coulomb| > 0",                 cpu.f_coulomb_at_target.mag());
    check_nonzero("GPU |f_coulomb| > 0 (mirror wired)",  gpu.f_coulomb_at_target.mag());

    // Per-component parity (FFT vs SOR + f32 vs f64 — generous tolerance).
    const double tol = 1e-3;
    check_close("f_strong.x parity",
                cpu.f_strong_at_target.x, gpu.f_strong_at_target.x, tol);
    check_close("f_strong.y parity",
                cpu.f_strong_at_target.y, gpu.f_strong_at_target.y, tol);
    check_close("f_strong.z parity",
                cpu.f_strong_at_target.z, gpu.f_strong_at_target.z, tol);

    if (failures != 0) {
        std::printf("\nfailures: %d\n", failures);
        return 1;
    }
    std::printf("\nAll force-diag parity checks passed.\n");
    return 0;
}
