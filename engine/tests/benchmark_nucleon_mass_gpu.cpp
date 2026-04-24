/**
 * @file benchmark_nucleon_mass_gpu.cpp
 * @brief GPU Dynamical Nucleon Mass Benchmark
 *
 * Tests the triad (nucleon analog) binding energy under physical 
 * fine-structure coupling limits using the CUDA engine.
 *
 * Specifically compares the bare EM configuration vs the full
 * phenomenological QCD (color_forces + strong_force) configuration
 * to verify if the energy audit recovers the 1836 ratio dynamically.
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <iomanip>

#include "ftd/gpu_engine.h"
#include "ftd/constants.h"

using namespace ftd;

static double run_test(int L, int WARMUP, bool use_strong_force, bool is_triad) {
    int mid = L / 2;
    gpu::GpuEngine gpu(L);

    // Manual EFT Configuration
    gpu.toggles.disable_all();
    gpu.toggles.wave_propagation = true;
    gpu.toggles.coupling = true;
    gpu.toggles.damping = true;
    gpu.toggles.poisson_coulomb = false; 

    // OVERRIDES for this specific test
    gpu.toggles.genesis = false;         // Prevent evaporation/pair production
    gpu.toggles.coulomb_charge_coupling = std::sqrt(2.0 * M_PI * ftd::ALPHA);

    if (use_strong_force) {
        gpu.toggles.forces = true;
        gpu.toggles.color_forces = true;
        gpu.toggles.strong_force = true;
        gpu.toggles.movement = true; // Movement is required for forces to integrate, but particles are locked
    }

    auto get_index = [L](int x, int y, int z) {
        return ((x % L + L) % L) * L * L + ((y % L + L) % L) * L + ((z % L + L) % L);
    };

    if (!is_triad) {
        // Single particle (Electron analog)
        gpu.inject_particle(mid, mid, mid, +1, {ftd::K_B, 0, 0}, +1, 0);
        // Lock particle so we only measure field equilibration, not kinetic escape
        std::vector<Voxel> voxels;
        gpu.sync_to_host(voxels);
        voxels[get_index(mid, mid, mid)].locked = true;
        gpu.upload_from_host(voxels);
    } else {
        // Triad (Nucleon analog)
        gpu.inject_particle(mid, mid, mid, +1, {ftd::K_B, 0, 0}, +1, 1);
        gpu.inject_particle(mid+1, mid+1, mid, +1, {ftd::K_B, 0, 0}, +1, 2);
        gpu.inject_particle(mid+1, mid, mid+1, +1, {ftd::K_B, 0, 0}, +1, 3);

        std::vector<Voxel> voxels;
        gpu.sync_to_host(voxels);
        voxels[get_index(mid, mid, mid)].locked = true;
        voxels[get_index(mid+1, mid+1, mid)].locked = true;
        voxels[get_index(mid+1, mid, mid+1)].locked = true;
        gpu.upload_from_host(voxels);
    }

    gpu.run(WARMUP);
    auto audit = gpu.energy_audit();
    return audit.field_energy + audit.wave_energy + audit.strong_energy;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  GPU NUCLEON MASS BATTERY\n";
    std::cout << "  Testing Triad mass under physical α and full QCD forces\n";
    std::cout << "================================================================\n\n";

    const int WARMUP = 400; // Fast warmup for GPU
    int sizes[] = {32, 64};

    for (int L : sizes) {
        std::cout << "--- Lattice Size L=" << L << " ---\n";
        
        // --- 1. Bare EM (No Strong Force) ---
        double e_single_em = run_test(L, WARMUP, false, false);
        double e_triad_em  = run_test(L, WARMUP, false, true);
        double ratio_em = e_triad_em / e_single_em;

        // --- 2. Full QCD (Strong Force Enabled) ---
        double e_single_qcd = run_test(L, WARMUP, true, false);
        double e_triad_qcd  = run_test(L, WARMUP, true, true);
        double ratio_qcd = e_triad_qcd / e_single_qcd;

        std::cout << "  Bare EM Ratio:     " << ratio_em << "  (E_triad=" << e_triad_em << ", E_single=" << e_single_em << ")\n";
        std::cout << "  Full QCD Ratio:    " << ratio_qcd << "  (E_triad=" << e_triad_qcd << ", E_single=" << e_single_qcd << ")\n\n";
    }

    std::cout << "--- Conclusion ---\n";
    std::cout << "  Target Physical m_p/m_e = 1836.15\n";
    std::cout << "  The QCD ratio includes the strong field energy ledger and can\n";
    std::cout << "  differ substantially from the bare EM ratio. Treat this as an\n";
    std::cout << "  exploratory engine diagnostic, not a calibrated mass prediction.\n";
    std::cout << "================================================================\n";

    return 0;
}
