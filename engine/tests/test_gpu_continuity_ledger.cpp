/**
 * GPU-native continuity ledger parity.
 *
 * Verifies that the CUDA tick emits the same native EFT continuity ledger that
 * the host snapshot extractor infers from before/after states.
 */

#include "ftd/constants.h"
#include "ftd/eft/dual_cell_continuity.h"
#include "ftd/gpu_engine.h"
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

int g_failures = 0;

void check(const std::string& name, bool condition) {
  if (condition) {
    std::cout << "  PASS  " << name << "\n";
  } else {
    std::cout << "  FAIL  " << name << "\n";
    ++g_failures;
  }
}

int index(int L, int x, int y, int z) {
  x = (x % L + L) % L;
  y = (y % L + L) % L;
  z = (z % L + L) % L;
  return x * L * L + y * L + z;
}

std::vector<int> state_snapshot(const std::vector<ftd::Voxel>& voxels) {
  std::vector<int> out(voxels.size(), 0);
  for (size_t i = 0; i < voxels.size(); ++i) {
    out[i] = static_cast<int>(voxels[i].state);
  }
  return out;
}

void place(std::vector<ftd::Voxel>& voxels,
           int L, int x, int y, int z,
           int8_t state,
           const ftd::Vec3& flux,
           const ftd::Vec3& velocity = {}) {
  auto& v = voxels[static_cast<size_t>(index(L, x, y, z))];
  v.state = state;
  v.flux = flux;
  v.velocity = velocity;
  v.particle_id = index(L, x, y, z);
}

bool same_vectors(const std::vector<int>& a, const std::vector<int>& b) {
  return a == b;
}

bool same_vectors(const std::vector<double>& a,
                  const std::vector<double>& b,
                  double tol = 1e-12) {
  if (a.size() != b.size()) return false;
  for (size_t i = 0; i < a.size(); ++i) {
    if (std::abs(a[i] - b[i]) > tol) return false;
  }
  return true;
}

void check_ledger_matches(const std::string& name,
                          std::vector<ftd::Voxel> voxels,
                          const ftd::TermToggles& toggles,
                          int expected_transports,
                          int expected_annihilations,
                          int expected_reaction_sites) {
  constexpr int L = 8;
  const auto before = state_snapshot(voxels);

  ftd::gpu::GpuEngine gpu(L);
  gpu.toggles = toggles;
  gpu.upload_from_host(voxels);
  gpu.tick();

  std::vector<ftd::Voxel> after_voxels;
  gpu.sync_to_host(after_voxels);
  const auto after = state_snapshot(after_voxels);

  ftd::eft::DualCellContinuity inferred;
  const auto inferred_report =
      ftd::eft::extract_moore_history_from_snapshots(L, before, after, inferred);
  const auto device = gpu.continuity_step();

  std::cout << "    " << name
            << ": inferred_transports=" << inferred_report.transported_events
            << " inferred_annihilations=" << inferred_report.annihilation_pairs
            << " device_I_l1=" << ftd::eft::total_current_l1(device)
            << " device_SR_l1=" << ftd::eft::total_reaction_l1(device)
            << " device_res=" << ftd::eft::max_continuity_residual(device)
            << "\n";

  check(name + ": inferred extraction valid", inferred_report.valid);
  check(name + ": expected transport count",
        inferred_report.transported_events == expected_transports);
  check(name + ": expected annihilation count",
        inferred_report.annihilation_pairs == expected_annihilations);
  check(name + ": expected reaction-site count",
        inferred_report.reaction_sites == expected_reaction_sites);
  check(name + ": device continuity closes",
        ftd::eft::max_continuity_residual(device) < 1e-12);
  check(name + ": rho_before parity",
        same_vectors(device.rho_before, inferred.rho_before));
  check(name + ": rho_after parity",
        same_vectors(device.rho_after, inferred.rho_after));
  check(name + ": reaction parity",
        same_vectors(device.reaction, inferred.reaction));
  check(name + ": current_x parity",
        same_vectors(device.current_x, inferred.current_x));
  check(name + ": current_y parity",
        same_vectors(device.current_y, inferred.current_y));
  check(name + ": current_z parity",
        same_vectors(device.current_z, inferred.current_z));
}

ftd::TermToggles toggles_with_movement_only() {
  ftd::TermToggles toggles;
  toggles.disable_all();
  toggles.movement = true;
  return toggles;
}

ftd::TermToggles toggles_with_genesis_only() {
  ftd::TermToggles toggles;
  toggles.disable_all();
  toggles.genesis = true;
  return toggles;
}

ftd::TermToggles toggles_with_no_state_extensions() {
  ftd::TermToggles toggles;
  toggles.disable_all();
  return toggles;
}

ftd::TermToggles toggles_with_pair_production_only() {
  ftd::TermToggles toggles;
  toggles.disable_all();
  toggles.pair_production = true;
  return toggles;
}

ftd::TermToggles toggles_with_weak_transmutation_only() {
  ftd::TermToggles toggles;
  toggles.disable_all();
  toggles.weak_transmutation = true;
  return toggles;
}

}  // namespace

int main() {
  std::cout << "================================================================\n";
  std::cout << "  TEST: GPU Native Continuity Ledger\n";
  std::cout << "================================================================\n";

  constexpr int L = 8;

  {
    std::cout << "\n-- GCL-1: x-face positive transport --\n";
    std::vector<ftd::Voxel> voxels(static_cast<size_t>(L * L * L));
    place(voxels, L, 3, 1, 1, +1, {0, 0, ftd::K_B}, {1, 0, 0});
    check_ledger_matches("GCL-1 x transport", voxels,
                         toggles_with_movement_only(), 1, 0, 0);
  }

  {
    std::cout << "\n-- GCL-2: diagonal Moore transport --\n";
    std::vector<ftd::Voxel> voxels(static_cast<size_t>(L * L * L));
    place(voxels, L, 3, 3, 1, +1, {ftd::K_B, ftd::K_B, 0}, {1, 1, 0});
    check_ledger_matches("GCL-2 diagonal transport", voxels,
                         toggles_with_movement_only(), 1, 0, 0);
  }

  {
    std::cout << "\n-- GCL-3: opposite-sign annihilation --\n";
    std::vector<ftd::Voxel> voxels(static_cast<size_t>(L * L * L));
    place(voxels, L, 3, 1, 1, +1, {0, 0, ftd::K_B}, {1, 0, 0});
    place(voxels, L, 4, 1, 1, -1, {0, 0, -ftd::K_B});
    check_ledger_matches("GCL-3 annihilation", voxels,
                         toggles_with_movement_only(), 0, 1, 0);
  }

  {
    std::cout << "\n-- GCL-4: same-sign bounce --\n";
    std::vector<ftd::Voxel> voxels(static_cast<size_t>(L * L * L));
    place(voxels, L, 3, 1, 1, +1, {0, 0, ftd::K_B}, {1, 0, 0});
    place(voxels, L, 4, 1, 1, +1, {0, 0, ftd::K_B});
    check_ledger_matches("GCL-4 bounce", voxels,
                         toggles_with_movement_only(), 0, 0, 0);
  }

  {
    std::cout << "\n-- GCL-5: phase-write genesis reaction --\n";
    std::vector<ftd::Voxel> voxels(static_cast<size_t>(L * L * L));
    voxels[static_cast<size_t>(index(L, 3, 3, 3))].flux = {100.0 * ftd::K_B, 0, 0};
    check_ledger_matches("GCL-5 genesis", voxels,
                         toggles_with_genesis_only(), 0, 0, 1);
  }

  {
    std::cout << "\n-- GCL-6: phase-write evaporation reaction --\n";
    std::vector<ftd::Voxel> voxels(static_cast<size_t>(L * L * L));
    place(voxels, L, 3, 3, 3, +1, {0, 0, 0});
    check_ledger_matches("GCL-6 evaporation", voxels,
                         toggles_with_no_state_extensions(), 0, 0, 1);
  }

  {
    std::cout << "\n-- GCL-7: pair-production reaction pair --\n";
    std::vector<ftd::Voxel> voxels(static_cast<size_t>(L * L * L));
    voxels[static_cast<size_t>(index(L, 3, 3, 3))].flux = {100.0 * ftd::K_B, 0, 0};
    check_ledger_matches("GCL-7 pair production", voxels,
                         toggles_with_pair_production_only(), 0, 0, 2);
  }

  {
    std::cout << "\n-- GCL-8: weak transmutation reaction --\n";
    std::vector<ftd::Voxel> voxels(static_cast<size_t>(L * L * L));
    place(voxels, L, 3, 3, 3, +1, {0, 0, 0});
    voxels[static_cast<size_t>(index(L, 4, 3, 3))].flux = {100.0 * ftd::K_B, 0, 0};
    check_ledger_matches("GCL-8 weak transmutation", voxels,
                         toggles_with_weak_transmutation_only(), 0, 0, 1);
  }

  {
    std::cout << "\n-- GCL-9: RenderBridge exposes GPU ledger --\n";
    ftd::RenderBridge rb(L);
    rb.toggles.disable_all();
    auto& v = rb.voxel_at(3, 3, 3);
    v.state = +1;
    v.particle_id = index(L, 3, 3, 3);
    rb.tick();

    const auto device = rb.continuity_step();
    check("GCL-9 bridge ledger has lattice size", device.L == L);
    check("GCL-9 bridge ledger closes",
          ftd::eft::max_continuity_residual(device) < 1e-12);
    check("GCL-9 bridge ledger reaction",
          ftd::eft::total_reaction_l1(device) == 1);
  }

  std::cout << "\n================================================================\n";
  if (g_failures == 0) {
    std::cout << "  GPU continuity ledger PASSED.\n";
  } else {
    std::cout << "  " << g_failures << " GPU continuity-ledger check(s) FAILED.\n";
  }
  std::cout << "================================================================\n";
  return g_failures;
}
