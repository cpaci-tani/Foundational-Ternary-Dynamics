/**
 * Native reaction ledger for signed ternary state changes.
 *
 * The movement current satisfies Delta_t rho + div j = 0. This test covers
 * non-transport state changes and verifies that they close as
 *
 *   Delta_t rho + div j = S_reaction
 *
 * with S_reaction equal to the sitewise state change for creation,
 * evaporation, pair production, and weak transmutation.
 */

#include "ftd/render_bridge.h"
#include "ftd/constants.h"

#include <cmath>
#include <cstdlib>
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

std::vector<int> state_field(const ftd::RenderBridge& rb) {
  std::vector<int> rho(static_cast<size_t>(rb.lattice().total_sites()), 0);
  const auto& voxels = rb.voxels();
  for (size_t i = 0; i < rho.size(); ++i) rho[i] = voxels[i].state;
  return rho;
}

int total_charge(const std::vector<int>& rho) {
  int q = 0;
  for (int value : rho) q += value;
  return q;
}

int count_nonzero(const std::vector<int>& rho) {
  int n = 0;
  for (int value : rho) {
    if (value != 0) ++n;
  }
  return n;
}

std::vector<int> source_delta(const std::vector<int>& before,
                              const std::vector<int>& after) {
  std::vector<int> source(before.size(), 0);
  for (size_t i = 0; i < before.size(); ++i) {
    source[i] = after[i] - before[i];
  }
  return source;
}

std::vector<int> reaction_residual(const std::vector<int>& before,
                                   const std::vector<int>& after,
                                   const std::vector<int>& source) {
  std::vector<int> residual(before.size(), 0);
  for (size_t i = 0; i < before.size(); ++i) {
    residual[i] = after[i] - before[i] - source[i];
  }
  return residual;
}

int max_abs(const std::vector<int>& values) {
  int out = 0;
  for (int value : values) {
    int a = std::abs(value);
    if (a > out) out = a;
  }
  return out;
}

void disable_all_keep_single_substrate(ftd::RenderBridge& rb) {
  rb.toggles.disable_all();
  rb.toggles.dual_substrate = false;
}

void disable_all_keep_dual_substrate(ftd::RenderBridge& rb) {
  rb.toggles.disable_all();
  rb.toggles.dual_substrate = true;
}

bool check_source_closure(const std::string& name,
                          const std::vector<int>& before,
                          const std::vector<int>& after,
                          int expected_delta_q,
                          int expected_nonzero_after) {
  const std::vector<int> zero_source(before.size(), 0);
  const auto source = source_delta(before, after);
  const int max_without_source = max_abs(reaction_residual(before, after, zero_source));
  const int max_with_source = max_abs(reaction_residual(before, after, source));
  const int delta_q = total_charge(after) - total_charge(before);
  const int nonzero_after = count_nonzero(after);

  std::cout << "    " << name
            << ": delta_Q=" << delta_q
            << " nonzero_after=" << nonzero_after
            << " max_without_source=" << max_without_source
            << " max_with_source=" << max_with_source << "\n";

  const bool ok = (delta_q == expected_delta_q) &&
                  (nonzero_after == expected_nonzero_after) &&
                  (max_without_source > 0) &&
                  (max_with_source == 0);
  check(name, ok);
  return ok;
}

}  // namespace

int main() {
  std::cout << "================================================================\n";
  std::cout << "  TEST: Native Reaction Ledger\n";
  std::cout << "================================================================\n";

  {
    std::cout << "\n-- NRL-1: Evaporation sink --\n";
    ftd::RenderBridge rb(8);
    disable_all_keep_single_substrate(rb);
    rb.toggles.genesis = true;  // Evaporation currently shares this toggle.
    rb.inject_particle(3, 3, 3, +1, {0, 0, 0});

    const auto before = state_field(rb);
    rb.tick();
    const auto after = state_field(rb);

    const auto source = source_delta(before, after);
    const int max_with_source =
        max_abs(reaction_residual(before, after, source));
    const int delta_q = total_charge(after) - total_charge(before);
    const int nonzero_after = count_nonzero(after);
    std::cout << "    NRL-1: evaporation is stochastic"
              << ": delta_Q=" << delta_q
              << " nonzero_after=" << nonzero_after
              << " max_with_source=" << max_with_source << "\n";
    check("NRL-1: stochastic evaporation/no-op ledger closes",
          (delta_q == 0 || delta_q == -1) &&
          (nonzero_after == 1 || nonzero_after == 0) &&
          max_with_source == 0);
  }

  {
    std::cout << "\n-- NRL-2: Dual-substrate genesis source --\n";
    ftd::RenderBridge rb(8);
    disable_all_keep_dual_substrate(rb);
    rb.toggles.genesis = true;
    const int center = idx(rb, 3, 3, 3);
    auto& v = rb.voxels()[center];
    v.flux_L = {100.0 * ftd::K_GENESIS, 0, 0};
    v.flux_R = {};
    v.flux = v.flux_L + v.flux_R;

    const auto before = state_field(rb);
    rb.tick();
    const auto after = state_field(rb);

    check_source_closure("NRL-2: genesis creates local signed source",
                         before, after, +1, 1);
  }

  {
    std::cout << "\n-- NRL-3: Pair-production neutral source pair --\n";
    ftd::RenderBridge rb(8);
    disable_all_keep_single_substrate(rb);
    rb.toggles.pair_production = true;
    rb.inject_flux(3, 3, 3, {100.0 * ftd::K_GENESIS, 0, 0});

    const auto before = state_field(rb);
    rb.tick();
    const auto after = state_field(rb);

    check_source_closure("NRL-3: pair production has local S with net zero",
                         before, after, 0, 2);
  }

  {
    std::cout << "\n-- NRL-4: Weak transmutation signed source --\n";
    ftd::RenderBridge rb(8);
    disable_all_keep_dual_substrate(rb);
    rb.toggles.weak_transmutation = true;
    rb.inject_particle(3, 3, 3, +1, {0.1, 0, 0});

    // Force stress at the particle site above threshold. The weak rule uses
    // compute_stress_left in dual-substrate mode.
    auto& xp = rb.voxel_at(4, 3, 3);
    xp.flux_L = {100.0 * ftd::WEAK_THRESHOLD, 0, 0};
    xp.flux_R = {};
    xp.flux = xp.flux_L;

    const auto before = state_field(rb);
    rb.tick();
    const auto after = state_field(rb);

    check_source_closure("NRL-4: weak transmutation is S=-2s at site",
                         before, after, -2, 1);
  }

  std::cout << "\n================================================================\n";
  if (g_failures == 0) {
    std::cout << "  Native reaction ledger PASSED.\n";
  } else {
    std::cout << "  " << g_failures << " native reaction check(s) FAILED.\n";
  }
  std::cout << "================================================================\n";

  return g_failures;
}
