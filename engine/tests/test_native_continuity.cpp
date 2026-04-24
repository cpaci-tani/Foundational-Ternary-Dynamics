/**
 * Native continuity audit for signed ternary state transport.
 *
 * This is narrower than test_continuity.cpp. It checks the local lattice
 * equation
 *
 *   rho(t+1) - rho(t) + div j = S
 *
 * for the engine's native movement rule. With genesis/pair-production/
 * transmutation disabled, isolated moves and annihilation events have S=0
 * when j is the signed one-link transport current. Creation, evaporation,
 * pair production, and weak transmutation are classified as source/reaction
 * terms rather than transport.
 */

#include "ftd/render_bridge.h"
#include "ftd/constants.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {

struct LinkCurrent {
  int from;
  int to;
  int q;
};

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
  for (size_t i = 0; i < rho.size(); ++i) {
    rho[i] = voxels[i].state;
  }
  return rho;
}

int total_charge(const std::vector<int>& rho) {
  int q = 0;
  for (int value : rho) q += value;
  return q;
}

std::vector<int> continuity_residual(const std::vector<int>& before,
                                     const std::vector<int>& after,
                                     const std::vector<LinkCurrent>& links,
                                     const std::vector<int>& source = {}) {
  std::vector<int> div(before.size(), 0);
  for (const auto& link : links) {
    div[static_cast<size_t>(link.from)] += link.q;
    div[static_cast<size_t>(link.to)] -= link.q;
  }

  std::vector<int> residual(before.size(), 0);
  for (size_t i = 0; i < before.size(); ++i) {
    const int s = source.empty() ? 0 : source[i];
    residual[i] = after[i] - before[i] + div[i] - s;
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

std::vector<int> delta_source(const std::vector<int>& before,
                              const std::vector<int>& after) {
  std::vector<int> source(before.size(), 0);
  for (size_t i = 0; i < before.size(); ++i) {
    source[i] = after[i] - before[i];
  }
  return source;
}

void configure_movement_only(ftd::RenderBridge& rb) {
  rb.toggles.disable_all();
  rb.toggles.movement = true;
  rb.toggles.dual_substrate = false;
}

bool run_transport_case(const std::string& name,
                        ftd::RenderBridge& rb,
                        const std::vector<LinkCurrent>& expected_links) {
  const auto before = state_field(rb);
  rb.tick();
  const auto after = state_field(rb);
  const auto residual = continuity_residual(before, after, expected_links);
  const int max_r = max_abs(residual);
  const int q_before = total_charge(before);
  const int q_after = total_charge(after);

  std::cout << "    " << name
            << ": Q0=" << q_before
            << " Q1=" << q_after
            << " max|rho_dot+div_j|=" << max_r << "\n";

  const bool ok = (max_r == 0) && (q_before == q_after);
  check(name, ok);
  return ok;
}

}  // namespace

int main() {
  std::cout << "================================================================\n";
  std::cout << "  TEST: Native Local Continuity Audit\n";
  std::cout << "================================================================\n";

  {
    std::cout << "\n-- NC-1: Positive face transport --\n";
    ftd::RenderBridge rb(8);
    configure_movement_only(rb);
    rb.inject_particle(2, 2, 2, +1, {0, 0, ftd::K_B});
    rb.voxel_at(2, 2, 2).velocity = {1, 0, 0};
    run_transport_case("NC-1: + charge one face step",
                       rb,
                       {{idx(rb, 2, 2, 2), idx(rb, 3, 2, 2), +1}});
  }

  {
    std::cout << "\n-- NC-2: Negative diagonal Moore transport --\n";
    ftd::RenderBridge rb(8);
    configure_movement_only(rb);
    rb.inject_particle(2, 3, 2, -1, {0, 0, -ftd::K_B});
    rb.voxel_at(2, 3, 2).velocity = {1, -1, 1};
    run_transport_case("NC-2: - charge one diagonal step",
                       rb,
                       {{idx(rb, 2, 3, 2), idx(rb, 3, 2, 3), -1}});
  }

  {
    std::cout << "\n-- NC-3: Multiple isolated signed transports --\n";
    ftd::RenderBridge rb(10);
    configure_movement_only(rb);
    rb.inject_particle(2, 2, 2, +1, {0, 0, ftd::K_B});
    rb.inject_particle(7, 7, 7, -1, {0, 0, -ftd::K_B});
    rb.voxel_at(2, 2, 2).velocity = {1, 0, 0};
    rb.voxel_at(7, 7, 7).velocity = {0, -1, 0};
    run_transport_case("NC-3: two independent signed currents",
                       rb,
                       {{idx(rb, 2, 2, 2), idx(rb, 3, 2, 2), +1},
                        {idx(rb, 7, 7, 7), idx(rb, 7, 6, 7), -1}});
  }

  {
    std::cout << "\n-- NC-4: Opposite-sign annihilation as signed transport --\n";
    ftd::RenderBridge rb(8);
    configure_movement_only(rb);
    rb.inject_particle(2, 2, 2, +1, {0, 0, ftd::K_B});
    rb.inject_particle(3, 2, 2, -1, {0, 0, -ftd::K_B});
    rb.voxel_at(2, 2, 2).velocity = {1, 0, 0};
    run_transport_case("NC-4: + current neutralizes target - charge",
                       rb,
                       {{idx(rb, 2, 2, 2), idx(rb, 3, 2, 2), +1}});
  }

  {
    std::cout << "\n-- NC-5: Same-sign collision bounce has no state current --\n";
    ftd::RenderBridge rb(8);
    configure_movement_only(rb);
    rb.inject_particle(2, 2, 2, +1, {0, 0, ftd::K_B});
    rb.inject_particle(3, 2, 2, +1, {0, 0, ftd::K_B});
    rb.voxel_at(2, 2, 2).velocity = {1, 0, 0};
    run_transport_case("NC-5: same-sign bounce leaves rho unchanged", rb, {});
  }

  {
    std::cout << "\n-- NC-6: Reaction/source classification --\n";
    ftd::RenderBridge rb(8);
    const size_t n = static_cast<size_t>(rb.lattice().total_sites());
    std::vector<int> before(n, 0);
    std::vector<int> after_pair(n, 0);
    after_pair[static_cast<size_t>(idx(rb, 1, 1, 1))] = +1;
    after_pair[static_cast<size_t>(idx(rb, 2, 1, 1))] = -1;

    auto no_transport_pair = continuity_residual(before, after_pair, {});
    auto pair_source = delta_source(before, after_pair);
    auto sourced_pair = continuity_residual(before, after_pair, {}, pair_source);
    std::cout << "    pair creation: total_delta_Q="
              << total_charge(after_pair) - total_charge(before)
              << " max_without_source=" << max_abs(no_transport_pair)
              << " max_with_source=" << max_abs(sourced_pair) << "\n";
    check("NC-6a: pair production is source/reaction, not transport",
          total_charge(after_pair) == 0 &&
          max_abs(no_transport_pair) == 1 &&
          max_abs(sourced_pair) == 0);

    std::vector<int> before_flip(n, 0);
    std::vector<int> after_flip(n, 0);
    before_flip[static_cast<size_t>(idx(rb, 4, 4, 4))] = +1;
    after_flip[static_cast<size_t>(idx(rb, 4, 4, 4))] = -1;
    std::cout << "    transmutation + -> -: total_delta_Q="
              << total_charge(after_flip) - total_charge(before_flip) << "\n";
    check("NC-6b: weak transmutation is signed-charge nonconserving",
          total_charge(after_flip) - total_charge(before_flip) == -2);
  }

  std::cout << "\n================================================================\n";
  if (g_failures == 0) {
    std::cout << "  Native continuity audit PASSED.\n";
  } else {
    std::cout << "  " << g_failures << " native continuity check(s) FAILED.\n";
  }
  std::cout << "================================================================\n";

  return g_failures;
}

