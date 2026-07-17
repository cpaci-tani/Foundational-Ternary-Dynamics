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
#include "ftd/test_telemetry.h"
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

ftd::TermToggles toggles_with_evaporation_only() {
  ftd::TermToggles toggles;
  toggles.disable_all();
  toggles.evaporation = true;
  return toggles;
}

ftd::TermToggles toggles_with_pair_production_only() {
  ftd::TermToggles toggles;
  toggles.disable_all();
  toggles.pair_production = true;
  return toggles;
}

// GCL-6 (2026-07-16, BH-F5 completion): evaporation is stochastic on both
// backends — p = exp(-E_7site/K_MANIFEST²)·K_EVAP_RATE with the shared
// SplitMix64 Evaporation draw — so a zero-flux particle no longer evaporates
// deterministically on the first tick (the pre-fix GPU threshold rule did).
// Tick until the draw fires (deterministic for a fixed langevin_seed; bound
// 200 ⇒ P(no fire) ≈ 1.4e-10 at p = 0.1). Quiet ticks must keep the ledger
// closed with zero reactions; the firing tick gets the full parity battery.
void check_evaporation_ledger_stochastic(const std::string& name,
                                         std::vector<ftd::Voxel> voxels,
                                         const ftd::TermToggles& toggles) {
  constexpr int L = 8;
  constexpr int MAX_TICKS = 200;

  ftd::gpu::GpuEngine gpu(L);
  gpu.toggles = toggles;
  gpu.upload_from_host(voxels);

  std::vector<ftd::Voxel> host_before = std::move(voxels);
  for (int t = 1; t <= MAX_TICKS; ++t) {
    const auto before = state_snapshot(host_before);
    gpu.tick();
    std::vector<ftd::Voxel> host_after;
    gpu.sync_to_host(host_after);
    const auto after = state_snapshot(host_after);
    const auto device = gpu.continuity_step();

    if (before == after) {
      if (ftd::eft::max_continuity_residual(device) >= 1e-12 ||
          ftd::eft::total_reaction_l1(device) != 0.0) {
        check(name + ": quiet-tick ledger clean (tick " + std::to_string(t) +
              ")", false);
        return;
      }
      host_before = std::move(host_after);
      continue;
    }

    std::cout << "    " << name << ": evaporation fired at tick " << t << "\n";
    ftd::eft::DualCellContinuity inferred;
    const auto inferred_report =
        ftd::eft::extract_moore_history_from_snapshots(L, before, after,
                                                       inferred);
    check(name + ": inferred extraction valid", inferred_report.valid);
    check(name + ": expected transport count",
          inferred_report.transported_events == 0);
    check(name + ": expected annihilation count",
          inferred_report.annihilation_pairs == 0);
    check(name + ": expected reaction-site count",
          inferred_report.reaction_sites == 1);
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
    return;
  }
  check(name + ": evaporation fired within 200 ticks", false);
}

ftd::TermToggles toggles_with_weak_transmutation_only() {
  ftd::TermToggles toggles;
  toggles.disable_all();
  toggles.weak_transmutation = true;
  return toggles;
}

}  // namespace

int main() {
  ftd::test::contract({
      "transport/continuity/conservation",
      "[MEASUREMENT]",
      "state snapshots, GPU full-tick ledger, closure domain",
      "movement, genesis, evaporation, pair production, weak transmutation",
      "continuity_residual, current_l1, reaction_l1",
      "periodic L=8 lattice",
      "GPU-first; direct GpuEngine path",
      "device continuity ledger matches host snapshot inference",
      "failure means GPU state-changing channel is not ledgered or parity broke"});

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
    // evaporation flag (introduced 2026-05-05) lets the test exercise the
    // evaporation kernel in isolation without enabling genesis. Pre-flag this
    // used toggles_with_no_state_extensions() and depended on a GPU bug
    // (evaporation ignored its toggle gate); see callstack-audit BH-F6.
    // Stochastic since the BH-F5 completion (2026-07-16) — see the helper's
    // comment; the single-tick certain-death form asserted the retired
    // deterministic GPU threshold rule.
    check_evaporation_ledger_stochastic("GCL-6 evaporation", voxels,
                                        toggles_with_evaporation_only());
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
    rb.toggles.evaporation = true;  // exercise evaporation in isolation (see GCL-6 comment)
    auto& v = rb.voxel_at(3, 3, 3);
    v.state = +1;
    v.particle_id = index(L, 3, 3, 3);

    // Stochastic evaporation (BH-F5 completion, 2026-07-16): tick until the
    // shared SplitMix64 draw fires (deterministic at the fixed default seed;
    // bound 200 ⇒ P(no fire) ≈ 1.4e-10), requiring a closed ledger on the way.
    bool closed_every_tick = true;
    int fired_tick = -1;
    ftd::eft::DualCellContinuity device;
    for (int t = 1; t <= 200; ++t) {
      rb.tick();
      device = rb.continuity_step();
      if (ftd::eft::max_continuity_residual(device) >= 1e-12)
        closed_every_tick = false;
      if (ftd::eft::total_reaction_l1(device) == 1) { fired_tick = t; break; }
    }
    std::cout << "    GCL-9: evaporation fired at tick " << fired_tick << "\n";
    check("GCL-9 bridge ledger has lattice size", device.L == L);
    check("GCL-9 bridge ledger closes on every tick", closed_every_tick);
    check("GCL-9 bridge ledger reaction", fired_tick > 0);
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
