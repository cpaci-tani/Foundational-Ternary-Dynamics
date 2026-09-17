/**
 * CPU/CUDA parity for FTD-0406 strong_stress_energy.
 *
 * Isolated colour-sector pair: remainder colour force, collision-free
 * Hamiltonian projection, and CIC string T00. Default OFF => golden-neutral.
 * This is a completeness contract, not a derivation.
 */

#include "ftd/constants.h"
#include "ftd/gpu_engine.h"
#include "ftd/render_bridge.h"
#include "ftd/strong_stress_energy.h"

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

namespace {

using ftd::FluxBoundaryMode;
using ftd::RenderBridge;
using ftd::StrongEnergyStepDiagnostics;
using ftd::StrongStressCell;
using ftd::TermToggles;
using ftd::Vec3;
using ftd::Voxel;

int passed = 0;
int failed = 0;

void check(const char* name, bool ok) {
    std::printf("  %s  %s\n", ok ? "PASS" : "FAIL", name);
    ok ? ++passed : ++failed;
}

int index_of(int x, int y, int z, int L) {
    return x * L * L + y * L + z;
}

bool close(double a, double b, double tol = 1e-12) {
    return std::abs(a - b) <= tol;
}

TermToggles isolated_pair() {
    TermToggles t;
    t.disable_all();
    t.forces = true;
    t.movement = true;
    t.color_forces = true;
    t.strong_stress_energy = true;
    return t;
}

void stamp_contract_pair(std::vector<Voxel>& seed, int L) {
    const int y = L / 2;
    const int z = L / 2;
    const int i1 = index_of(8, y, z, L);
    const int i2 = index_of(L - 9, y, z, L);
    seed[static_cast<std::size_t>(i1)].state = 1;
    seed[static_cast<std::size_t>(i1)].color = 1;
    seed[static_cast<std::size_t>(i1)].flux = Vec3{ftd::K_B, 0.0, 0.0};
    seed[static_cast<std::size_t>(i1)].particle_id = 1;
    seed[static_cast<std::size_t>(i2)].state = 1;
    seed[static_cast<std::size_t>(i2)].color = -1;
    seed[static_cast<std::size_t>(i2)].flux = Vec3{0.0, ftd::K_B, 0.0};
    seed[static_cast<std::size_t>(i2)].particle_id = 2;
}

}  // namespace

int main() {
    std::printf("GPU strong-stress-energy parity (FTD-0406)\n");
    constexpr int L = 33;
    std::vector<Voxel> seed(static_cast<std::size_t>(L * L * L));
    stamp_contract_pair(seed, L);
    const TermToggles t = isolated_pair();
    std::string error;
    check("isolated colour sector validates", t.validate(&error));

    RenderBridge cpu(L);
    cpu.force_cpu();
    cpu.toggles.disable_all();
    cpu.toggles = t;
    cpu.voxels() = seed;
    cpu.tick();

    ftd::gpu::GpuEngine gpu(L);
    gpu.graph_capture_enabled = false;
    gpu.toggles.disable_all();
    gpu.toggles = t;
    gpu.upload_from_host(seed);
    gpu.tick();
    std::vector<Voxel> gpu_voxels;
    gpu.sync_to_host(gpu_voxels);

    const int i1 = index_of(8, L / 2, L / 2, L);
    const auto& cv = cpu.voxels()[static_cast<std::size_t>(i1)];
    const auto& gv = gpu_voxels[static_cast<std::size_t>(i1)];
    const auto cfd = cpu.force_diag_at(8, L / 2, L / 2);
    const auto& gfd = gpu.force_diag();
    std::printf("    remainder.x CPU=%.16g GPU=%.16g\n",
                cv.remainder.x, gv.remainder.x);
    std::printf("    velocity.x  CPU=%.16g GPU=%.16g\n",
                cv.velocity.x, gv.velocity.x);
    std::printf("    F_strong.x  CPU=%.16g GPU=%.16g\n",
                cfd.f_strong.x, gfd.strong_x[static_cast<std::size_t>(i1)]);

    check("remainder parity after projected tick",
          close(cv.remainder.x, gv.remainder.x) &&
          close(cv.remainder.y, gv.remainder.y) &&
          close(cv.remainder.z, gv.remainder.z));
    check("velocity parity after projected tick",
          close(cv.velocity.x, gv.velocity.x) &&
          close(cv.velocity.y, gv.velocity.y) &&
          close(cv.velocity.z, gv.velocity.z));
    check("colour-force parity with remainder positions",
          close(cfd.f_strong.x, gfd.strong_x[static_cast<std::size_t>(i1)]) &&
          close(cfd.f_strong.y, gfd.strong_y[static_cast<std::size_t>(i1)]) &&
          close(cfd.f_strong.z, gfd.strong_z[static_cast<std::size_t>(i1)]));
    check("projection residual stays on the frozen surface",
          std::abs(cpu.strong_energy_step_diagnostics().residual) <= 1e-12);

    std::vector<StrongStressCell> gpu_cells;
    gpu.download_strong_stress(gpu_cells);
    const auto& cpu_cells = cpu.strong_stress_cells();
    double t00_err = 0.0;
    for (std::size_t i = 0; i < cpu_cells.size(); ++i) {
        t00_err = std::max(t00_err,
            std::abs(cpu_cells[i].energy_density - gpu_cells[i].energy_density));
    }
    std::printf("    max |T00 CPU-GPU|=%.3g\n", t00_err);
    check("CIC string T00 parity", t00_err <= 1e-12);

    // Non-periodic boundary gate. The projection's pair math
    // (minimum_image_component / minimum_image_displacement) wraps with the
    // periodic minimum image unconditionally, so a non-Periodic flux_boundary
    // would feed it physically wrong separations. CUDA's projection_ok has
    // always required Periodic; the CPU gate did not, so the two backends
    // disagreed on the same profile. Both must now refuse it identically.
    TermToggles dispersal = isolated_pair();
    dispersal.flux_boundary = FluxBoundaryMode::Dispersal;
    std::string dispersal_error;
    const bool dispersal_rejected = !dispersal.validate(&dispersal_error);
    std::printf("    validate(Dispersal) -> %s",
                dispersal_error.empty() ? "<accepted>\n" : dispersal_error.c_str());
    check("validate rejects strong_stress_energy with flux_boundary=Dispersal",
          dispersal_rejected
          && dispersal_error.find(
                 "strong_stress_energy requires flux_boundary=Periodic")
             != std::string::npos);

    RenderBridge cpu_np(L);
    cpu_np.force_cpu();
    cpu_np.toggles.disable_all();
    cpu_np.toggles = dispersal;
    cpu_np.voxels() = seed;
    cpu_np.tick();
    const StrongEnergyStepDiagnostics cpu_np_diag =
        cpu_np.strong_energy_step_diagnostics();

    ftd::gpu::GpuEngine gpu_np(L);
    gpu_np.graph_capture_enabled = false;
    gpu_np.toggles.disable_all();
    gpu_np.toggles = dispersal;
    gpu_np.upload_from_host(seed);
    gpu_np.tick();
    StrongEnergyStepDiagnostics gpu_np_diag;
    gpu_np.download_strong_step_diagnostics(gpu_np_diag);

    std::printf("    projection_failures CPU=%d GPU=%d\n",
                cpu_np_diag.projection_failures, gpu_np_diag.projection_failures);
    std::printf("    projection_events   CPU=%d GPU=%d\n",
                cpu_np_diag.projection_events, gpu_np_diag.projection_events);
    std::printf("    topology_failures   CPU=%d GPU=%d\n",
                cpu_np_diag.topology_failures, gpu_np_diag.topology_failures);
    check("CPU refuses the non-periodic projection",
          cpu_np_diag.projection_failures == 1
          && cpu_np_diag.projection_events == 0
          && cpu_np_diag.topology_failures == 0);
    check("CPU/GPU agree on the non-periodic projection refusal",
          cpu_np_diag.projection_failures == gpu_np_diag.projection_failures
          && cpu_np_diag.projection_events == gpu_np_diag.projection_events
          && cpu_np_diag.topology_failures == gpu_np_diag.topology_failures);

    std::printf("\n%d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
