/**
 * @file benchmark_sm_masses_gpu.cpp
 * @brief GPU exploratory Standard Model hierarchy benchmark
 *
 * Computes the equilibrium field energy for fundamental particles
 * on the ternary lattice and compares ratios against Standard Model reference
 * scales. The Higgs VEV and Weinberg-angle projection below are imposed
 * reference inputs, so this is an exploratory diagnostic rather than a native
 * derivation of the Standard Model mass spectrum.
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <iomanip>

#include "ftd/gpu_engine.h"
#include "ftd/constants.h"

using namespace ftd;

static double run_electron(int L, int WARMUP) {
    int mid = L / 2;
    gpu::GpuEngine gpu(L);

    gpu.toggles.disable_all();
    gpu.toggles.wave_propagation = true;
    gpu.toggles.coupling = true;
    gpu.toggles.damping = true;
    
    // Inject neutral electron-analog (flavor=0, color=0) to measure bare face-field mass
    gpu.inject_particle(mid, mid, mid, +1, {ftd::K_B, 0, 0}, 0, 0);
    
    std::vector<Voxel> voxels;
    gpu.sync_to_host(voxels);
    voxels[mid * L * L + mid * L + mid].locked = true;
    gpu.upload_from_host(voxels);

    gpu.run(WARMUP);
    auto audit = gpu.energy_audit();
    return audit.field_energy + audit.wave_energy; // U(1) face field
}

static double run_proton(int L, int WARMUP) {
    int mid = L / 2;
    gpu::GpuEngine gpu(L);

    gpu.toggles.disable_all();
    gpu.toggles.wave_propagation = true;
    gpu.toggles.coupling = true;
    gpu.toggles.damping = true;
    gpu.toggles.color_forces = true;
    gpu.toggles.strong_force = true;
    
    // Inject Triad (nucleon analog) with color charge
    gpu.inject_particle(mid, mid, mid, +1, {ftd::K_B, 0, 0}, +1, 1);
    gpu.inject_particle(mid+1, mid+1, mid, +1, {ftd::K_B, 0, 0}, +1, 2);
    gpu.inject_particle(mid+1, mid, mid+1, +1, {ftd::K_B, 0, 0}, +1, 3);
    
    auto get_index = [L](int x, int y, int z) {
        return ((x % L + L) % L) * L * L + ((y % L + L) % L) * L + ((z % L + L) % L);
    };

    std::vector<Voxel> voxels;
    gpu.sync_to_host(voxels);
    voxels[get_index(mid, mid, mid)].locked = true;
    voxels[get_index(mid+1, mid+1, mid)].locked = true;
    voxels[get_index(mid+1, mid, mid+1)].locked = true;
    gpu.upload_from_host(voxels);

    gpu.run(WARMUP);
    auto audit = gpu.energy_audit();
    // Total emergent mass = U(1) + SU(3)
    return audit.field_energy + audit.wave_energy + audit.strong_energy;
}

static std::pair<double, double> run_wz_boson(int L, int WARMUP) {
    int mid = L / 2;
    gpu::GpuEngine gpu(L);

    gpu.toggles.disable_all();
    gpu.toggles.wave_propagation = true;
    gpu.toggles.coupling = true;
    gpu.toggles.damping = true;
    
    // Inject particle with flavor to excite the weak (edge) field
    gpu.inject_particle(mid, mid, mid, +1, {ftd::K_B, 0, 0}, 0, 0, 1);
    
    std::vector<Voxel> voxels;
    gpu.sync_to_host(voxels);
    voxels[mid * L * L + mid * L + mid].locked = true;
    gpu.upload_from_host(voxels);

    gpu.run(WARMUP);
    gpu.sync_to_host(voxels);

    // Apply an imposed Higgs VEV reference interaction to weak field energy.
    double weak_mass = 0.0;
    for (const auto& v : voxels) {
        double w_mag = v.flux_weak.mag();
        // Diagnostic model: field density scaled by sqrt(VEV).
        weak_mass += w_mag * w_mag * std::sqrt(ftd::HIGGS_VEV_LATTICE);
    }
    
    // Split into M_Z and M_W via the imposed Weinberg-angle projection.
    double m_z = weak_mass;
    double m_w = m_z * ftd::WZ_MIXING_ANGLE_COS;
    
    return {m_w, m_z};
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  GPU STANDARD MODEL MASS BATTERY\n";
    std::cout << "  Exploratory Face, Edge, and Vertex hierarchy diagnostic\n";
    std::cout << "================================================================\n\n";

    const int WARMUP = 400; 
    int sizes[] = {32, 64};

    for (int L : sizes) {
        std::cout << "--- Lattice Size L=" << L << " ---\n";
        
        double e_mass = run_electron(L, WARMUP);
        double p_mass = run_proton(L, WARMUP);
        auto [w_mass, z_mass] = run_wz_boson(L, WARMUP);
        
        double p_ratio = p_mass / e_mass;
        double w_ratio = w_mass / e_mass;
        double z_ratio = z_mass / e_mass;

        std::cout << "  m_e (Electron) Field Energy : " << e_mass << "\n";
        std::cout << "  m_p (Proton) Field Energy   : " << p_mass << "\n";
        std::cout << "  M_W (W Boson) Field Energy  : " << w_mass << "\n";
        std::cout << "  M_Z (Z Boson) Field Energy  : " << z_mass << "\n\n";
        
        std::cout << "  Emergent Ratio m_p / m_e    : " << p_ratio << "  (SM ref: 1836.15)\n";
        std::cout << "  Emergent Ratio M_W / m_e    : " << w_ratio << "  (SM ref: ~157300)\n";
        std::cout << "  Emergent Ratio M_Z / m_e    : " << z_ratio << "  (SM ref: ~178400)\n\n";
    }

    std::cout << "--- Conclusion ---\n";
    std::cout << "  These runs exercise the GPU field-energy ledgers and geometric\n";
    std::cout << "  stencil factors. The VEV and Weinberg projection are imposed\n";
    std::cout << "  reference inputs, so treat the ratios as exploratory diagnostics,\n";
    std::cout << "  not calibrated mass predictions.\n";
    std::cout << "================================================================\n";
    return 0;
}
