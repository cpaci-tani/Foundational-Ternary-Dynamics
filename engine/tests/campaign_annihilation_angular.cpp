/**
 * Campaign: e+e- Annihilation Angular Distribution (QED Scattering) — GPU
 *
 * Validates that the FTD lattice produces the expected angular distribution
 * for e+e- -> gamma gamma annihilation radiation.
 *
 * Uses GpuEngine for CUDA-accelerated simulation on GPU.
 * 128^3 lattice (~2M voxels), production-quality resolution.
 *
 * KEY CHANGE from v1: Uses counter-propagating BEAM initial conditions
 * (inject_particle with directed flux along x-axis) instead of spherical
 * wavepackets (inject_wavepacket). The quadrupolar angular distribution
 * (1 + cos^2 theta) requires directed initial flux — spherically symmetric
 * wavepackets give B ~ -0.19 (near isotropic), not B = 1.
 *
 * Protocol:
 *   1. Create a 128^3 lattice on GPU
 *   2. Place +1 particle at (L/2 - 5, L/2, L/2) with flux (+K_B, 0, 0)
 *      and -1 particle at (L/2 + 5, L/2, L/2) with flux (-K_B, 0, 0)
 *   3. Set initial velocities toward each other along x-axis
 *   4. Run until annihilation, then 55 more ticks for radiation propagation
 *   5. Sync to host and export full flux field CSV
 *   6. Post-process with Python: angular distribution, fit to (1 + B cos^2 theta)
 *
 * Checks:
 *   AA1: Both particles annihilate (manifested_count == 0 post-collision)
 *   AA2: Total charge conserved (charge == 0 throughout)
 *   AA3: Radiation propagates outward (field energy > 0 at T_measure)
 *   AA4: CSV export succeeds
 *
 * Build: cmake -DFTD_ENABLE_CUDA=ON ... && make ftd_annihilation_angular_gpu
 * Run:   ./ftd_annihilation_angular_gpu
 * Post:  python scripts/experiments/analyze_annihilation_angular.py output/annihilation_flux_t<N>.csv
 */

#include "ftd/gpu_engine.h"
#include "ftd/constants.h"
#include <cstdio>
#include <cmath>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>

using namespace ftd;

static int tests_passed = 0;
static int tests_failed = 0;

#define CHECK(cond, msg) do { \
    if (cond) { tests_passed++; std::printf("  PASS: %s\n", msg); } \
    else { tests_failed++; std::printf("  FAIL: %s\n", msg); } \
    fflush(stdout); \
} while(0)

// Export flux field from host voxels to CSV
static bool export_flux_field_csv(const std::vector<Voxel>& voxels, int L,
                                   const std::string& filename) {
    std::ofstream f(filename);
    if (!f.is_open()) return false;
    f << "x,y,z,state,Jx,Jy,Jz,density\n";
    int written = 0;
    for (int z = 0; z < L; ++z) {
        for (int y = 0; y < L; ++y) {
            for (int x = 0; x < L; ++x) {
                int idx = z * L * L + y * L + x;
                const auto& v = voxels[idx];
                // Only write sites with significant flux (sparse output)
                double jmag = std::sqrt(v.flux.x*v.flux.x + v.flux.y*v.flux.y + v.flux.z*v.flux.z);
                if (jmag > 1e-8 || v.state != 0) {
                    f << x << "," << y << "," << z << ","
                      << (int)v.state << ","
                      << v.flux.x << "," << v.flux.y << "," << v.flux.z << ","
                      << jmag << "\n";
                    ++written;
                }
            }
        }
    }
    f.close();
    std::printf("  Exported %d sites to %s\n", written, filename.c_str());
    return true;
}

int main() {
    std::printf("================================================================\n");
    std::printf("  CAMPAIGN: e+e- Annihilation Angular Distribution — GPU (CUDA)\n");
    std::printf("  Initial conditions: counter-propagating BEAMS (directed flux)\n");
    std::printf("================================================================\n");

    // ------------------------------------------------------------------
    // Setup: 128^3 lattice on GPU (production resolution)
    // ------------------------------------------------------------------
    constexpr int L = 128;
    constexpr int HALF = L / 2;
    // Separation of 10 lattice units — this produced annihilation at tick ~1596
    // in the pair production experiment
    constexpr int SEP = 10;
    constexpr int POS_X = HALF - SEP/2;   // +1 particle (electron) at x=59
    constexpr int NEG_X = HALF + SEP/2;   // -1 particle (positron) at x=69
    constexpr int POST_ANNIHILATION_TICKS = 55;
    constexpr int MAX_TICKS = 2500;  // Generous time for Coulomb-driven collision
    // Initial velocity toward each other (same as Rutherford experiment pattern)
    constexpr double V_INIT = 0.3;

    std::printf("\n--- Setup ---\n");
    std::printf("  Lattice size: %d^3 = %d voxels\n", L, L*L*L);
    std::printf("  e+ position:  (%d, %d, %d)  flux = (+K_B, 0, 0)\n", POS_X, HALF, HALF);
    std::printf("  e- position:  (%d, %d, %d)  flux = (-K_B, 0, 0)\n", NEG_X, HALF, HALF);
    std::printf("  Separation:   %d lattice units\n", SEP);
    std::printf("  V_initial:    %.2f (toward each other along x)\n", V_INIT);
    std::printf("  K_B:          %.6f\n", K_B);
    std::printf("  C_SPEED:      %.6f\n", C_SPEED);
    std::printf("  ALPHA:        %.10f (1/%.6f)\n", ALPHA, 1.0/ALPHA);

    // Initialize GPU engine
    gpu::GpuEngine gpu(L);

    // Enable core physics, disable phenomenological extensions
    gpu.toggles.enable_all();
    gpu.toggles.larmor_radiation = false;
    gpu.toggles.color_forces = false;
    gpu.toggles.strong_force = false;
    gpu.toggles.triad_binding = false;
    // pair_production must be ON for annihilation to occur
    gpu.toggles.pair_production = true;
    gpu.toggles.exchange_force = false;
    gpu.toggles.latency_field = false;

    // ------------------------------------------------------------------
    // Inject particles with DIRECTED FLUX (beams, not wavepackets)
    //
    // This is the critical difference: inject_particle places a manifested
    // voxel with a specific flux vector. The +1 particle gets flux pointing
    // in +x (toward the -1 particle), and the -1 gets flux in -x (toward
    // the +1). This creates counter-propagating beams with a well-defined
    // collision axis, which is required for the quadrupolar (1+cos^2 theta)
    // angular distribution.
    // ------------------------------------------------------------------
    gpu.inject_particle(POS_X, HALF, HALF, +1, {K_B, 0.0, 0.0});
    gpu.inject_particle(NEG_X, HALF, HALF, -1, {-K_B, 0.0, 0.0});

    // ------------------------------------------------------------------
    // Set initial velocities toward each other via sync/upload
    // (Same pattern used in GP-EXP-RUTHERFORD experiment)
    // ------------------------------------------------------------------
    std::vector<Voxel> voxels;
    gpu.sync_to_host(voxels);

    // +1 particle at POS_X: velocity in +x direction (toward center)
    {
        int idx = HALF * L * L + HALF * L + POS_X;
        if (voxels[idx].state == +1) {
            voxels[idx].velocity = {V_INIT, 0.0, 0.0};
            std::printf("  Set e+ velocity = (+%.2f, 0, 0)\n", V_INIT);
        } else {
            std::printf("  WARNING: e+ not found at expected position\n");
        }
    }

    // -1 particle at NEG_X: velocity in -x direction (toward center)
    {
        int idx = HALF * L * L + HALF * L + NEG_X;
        if (voxels[idx].state == -1) {
            voxels[idx].velocity = {-V_INIT, 0.0, 0.0};
            std::printf("  Set e- velocity = (-%.2f, 0, 0)\n", V_INIT);
        } else {
            std::printf("  WARNING: e- not found at expected position\n");
        }
    }

    gpu.upload_from_host(voxels);

    auto audit0 = gpu.energy_audit();
    std::printf("  Initial manifested: %d, charge: %d\n",
                audit0.manifested_count, audit0.charge_total);

    // ------------------------------------------------------------------
    // Run simulation on GPU until annihilation
    // ------------------------------------------------------------------
    std::printf("\n--- Running simulation (GPU, counter-propagating beams) ---\n");

    int annihilation_tick = -1;
    bool charge_conserved = true;

    for (int t = 0; t < MAX_TICKS; ++t) {
        gpu.tick();

        // Check every 5 ticks to reduce sync overhead
        if (t % 5 == 0 || annihilation_tick > 0) {
            auto audit = gpu.energy_audit();

            if (audit.charge_total != 0) {
                charge_conserved = false;
            }

            // Detect annihilation
            if (audit.manifested_count == 0 && annihilation_tick < 0) {
                annihilation_tick = t + 1;
                std::printf("  Annihilation detected at tick ~%d\n", annihilation_tick);
            }

            // Progress reporting every 500 ticks
            if (t % 500 == 0 && annihilation_tick < 0) {
                std::printf("  tick %d: manifested=%d charge=%d field_e=%.4e\n",
                            t, audit.manifested_count, audit.charge_total, audit.field_energy);
            }

            // After annihilation, run POST_ANNIHILATION_TICKS more
            if (annihilation_tick > 0 && (t + 1) >= annihilation_tick + POST_ANNIHILATION_TICKS) {
                std::printf("  Radiation propagation complete at tick %d\n", t + 1);
                break;
            }
        }
    }

    int T_measure = annihilation_tick > 0 ? annihilation_tick + POST_ANNIHILATION_TICKS : MAX_TICKS;

    // ------------------------------------------------------------------
    // Final diagnostics and export
    // ------------------------------------------------------------------
    auto audit_final = gpu.energy_audit();

    std::printf("\n--- Final state (tick ~%d) ---\n", T_measure);
    std::printf("  Manifested:     %d\n", audit_final.manifested_count);
    std::printf("  Charge total:   %d\n", audit_final.charge_total);
    std::printf("  Field energy:   %.6e\n", audit_final.field_energy);
    std::printf("  Gauss violation: %.6e\n", audit_final.gauss_violation);

    // Sync full voxel data from GPU to host for CSV export
    gpu.sync_to_host(voxels);

    std::printf("  Synced %zu voxels from GPU to host\n", voxels.size());

    // Export CSV
    std::filesystem::create_directories("output");
    std::string flux_file = "output/annihilation_flux_t" + std::to_string(T_measure) + ".csv";
    bool csv_ok = export_flux_field_csv(voxels, L, flux_file);

    // ------------------------------------------------------------------
    // Checks
    // ------------------------------------------------------------------
    std::printf("\n--- Checks ---\n");

    CHECK(annihilation_tick > 0 && audit_final.manifested_count == 0,
          "AA1: Both particles annihilated (manifested_count == 0)");

    CHECK(charge_conserved && audit_final.charge_total == 0,
          "AA2: Total charge conserved (Q=0 throughout)");

    CHECK(audit_final.field_energy > 1e-10,
          "AA3: Radiation field present (field_energy > 0)");

    CHECK(csv_ok, "AA4: Flux field CSV export succeeded");

    // ------------------------------------------------------------------
    // Summary
    // ------------------------------------------------------------------
    std::printf("\n================================================================\n");
    std::printf("  RESULT: %s (%d passed, %d failed)\n",
                tests_failed == 0 ? "ALL PASSED" : "FAILURES",
                tests_passed, tests_failed);
    std::printf("\n  NEXT STEP: Run the Python analysis script:\n");
    std::printf("    python scripts/experiments/analyze_annihilation_angular.py \\\n");
    std::printf("      %s\n", flux_file.c_str());
    std::printf("  This computes the angular distribution and fits to\n");
    std::printf("  (1 + B cos^2 theta), reporting B and chi-squared.\n");
    std::printf("  QED prediction: B = 1.0\n");
    std::printf("  (Previous wavepacket result: B ~ -0.19 — isotropic, wrong)\n");
    std::printf("  (Expected with beams: B ~ 1.0 — quadrupolar, correct)\n");
    std::printf("================================================================\n");

    return tests_failed;
}
