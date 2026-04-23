/**
 * Native engine-history flow audit.
 *
 * This connects actual RenderBridge reaction ticks to the finite-volume
 * dual-cell continuity ledger. For non-transport reactions we extract
 *
 *   S_reaction = rho_after - rho_before
 *
 * and verify that both fine and b=2-blocked histories satisfy
 *
 *   Delta rho + div I = S_reaction
 *
 * with I=0 for these reaction-only cases.
 */

#include "ftd/constants.h"
#include "ftd/eft/dual_cell_continuity.h"
#include "ftd/render_bridge.h"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

static int g_failures = 0;

void check(const std::string& name, bool condition) {
  if (condition) {
    std::cout << "  PASS  " << name << "\n";
  } else {
    std::cout << "  FAIL  " << name << "\n";
    ++g_failures;
  }
}

int idx(const ftd::RenderBridge& rb, int x, int y, int z) {
  return rb.lattice().index(x, y, z);
}

std::vector<int> state_snapshot(const ftd::RenderBridge& rb) {
  std::vector<int> out(static_cast<size_t>(rb.lattice().total_sites()), 0);
  const auto& voxels = rb.voxels();
  for (size_t i = 0; i < out.size(); ++i) {
    out[i] = static_cast<int>(voxels[i].state);
  }
  return out;
}

ftd::eft::DualCellContinuity reaction_history_from_snapshots(
    int L,
    const std::vector<int>& before,
    const std::vector<int>& after) {
  ftd::eft::DualCellContinuity hist(L);
  for (size_t i = 0; i < before.size(); ++i) {
    hist.rho_before[i] = before[i];
    hist.rho_after[i] = after[i];
    hist.reaction[i] = after[i] - before[i];
  }
  return hist;
}

void disable_all_keep_single_substrate(ftd::RenderBridge& rb) {
  rb.toggles.disable_all();
  rb.toggles.dual_substrate = false;
}

void disable_all_keep_dual_substrate(ftd::RenderBridge& rb) {
  rb.toggles.disable_all();
  rb.toggles.dual_substrate = true;
}

void check_reaction_history(const std::string& name,
                            ftd::RenderBridge& rb,
                            int expected_delta_q,
                            int alternate_expected_delta_q = 999999) {
  const auto before = state_snapshot(rb);
  rb.tick();
  const auto after = state_snapshot(rb);

  const auto fine =
      reaction_history_from_snapshots(rb.lattice().size(), before, after);
  const auto coarse = ftd::eft::block_dual_cell_continuity_b2(fine);

  const int delta_q =
      ftd::eft::total_after(fine) - ftd::eft::total_before(fine);

  std::cout << "    " << name
            << ": delta_Q=" << delta_q
            << " S_fine=" << ftd::eft::total_reaction(fine)
            << " S_coarse=" << ftd::eft::total_reaction(coarse)
            << " fine_res=" << ftd::eft::max_continuity_residual(fine)
            << " coarse_res=" << ftd::eft::max_continuity_residual(coarse)
            << "\n";

  check(name + ": expected signed charge change",
        delta_q == expected_delta_q || delta_q == alternate_expected_delta_q);
  check(name + ": fine reaction ledger closes",
        ftd::eft::max_continuity_residual(fine) < 1e-12);
  check(name + ": blocked reaction ledger closes",
        ftd::eft::max_continuity_residual(coarse) < 1e-12);
  check(name + ": reaction total blocks exactly",
        ftd::eft::total_reaction(coarse) == ftd::eft::total_reaction(fine));
}

}  // namespace

int main() {
  std::cout << "================================================================\n";
  std::cout << "  TEST: Native Engine History b=2 Flow\n";
  std::cout << "================================================================\n";

  {
    std::cout << "\n-- NEH-1: Evaporation sink --\n";
    ftd::RenderBridge rb(8);
    disable_all_keep_single_substrate(rb);
    rb.toggles.genesis = true;
    rb.inject_particle(3, 3, 3, +1, {0, 0, 0});
    check_reaction_history("NEH-1 stochastic evaporation/no-op", rb, -1, 0);
  }

  {
    std::cout << "\n-- NEH-2: Dual-substrate genesis source --\n";
    ftd::RenderBridge rb(8);
    disable_all_keep_dual_substrate(rb);
    rb.toggles.genesis = true;
    auto& v = rb.voxels()[static_cast<size_t>(idx(rb, 3, 3, 3))];
    v.flux_L = {100.0 * ftd::K_GENESIS, 0, 0};
    v.flux_R = {};
    v.flux = v.flux_L + v.flux_R;
    check_reaction_history("NEH-2 genesis", rb, +1);
  }

  {
    std::cout << "\n-- NEH-3: Pair-production neutral source pair --\n";
    ftd::RenderBridge rb(8);
    disable_all_keep_single_substrate(rb);
    rb.toggles.pair_production = true;
    rb.inject_flux(3, 3, 3, {100.0 * ftd::K_GENESIS, 0, 0});
    check_reaction_history("NEH-3 pair production", rb, 0);
  }

  {
    std::cout << "\n-- NEH-4: Weak transmutation signed source --\n";
    ftd::RenderBridge rb(8);
    disable_all_keep_dual_substrate(rb);
    rb.toggles.weak_transmutation = true;
    rb.inject_particle(3, 3, 3, +1, {0.1, 0, 0});
    auto& xp = rb.voxel_at(4, 3, 3);
    xp.flux_L = {100.0 * ftd::WEAK_THRESHOLD, 0, 0};
    xp.flux_R = {};
    xp.flux = xp.flux_L;
    check_reaction_history("NEH-4 weak transmutation", rb, -2);
  }

  std::cout << "\n================================================================\n";
  if (g_failures == 0) {
    std::cout << "  Native engine-history flow audit PASSED.\n";
  } else {
    std::cout << "  " << g_failures << " native engine-history check(s) FAILED.\n";
  }
  std::cout << "================================================================\n";

  return g_failures;
}
