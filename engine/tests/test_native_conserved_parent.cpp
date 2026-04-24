/**
 * Native conserved-parent audit for weak transmutation.
 *
 * Weak transmutation changes the signed ternary state:
 *
 *   s -> -s
 *
 * so signed s is not conserved by itself. In dual-substrate mode, however, the
 * engine also swaps the hidden substrates:
 *
 *   J_L <-> J_R
 *
 * This test verifies the resulting parent ledger at the transmuting site:
 *
 *   J = J_L + J_R          conserved
 *   |J_L|^2 + |J_R|^2      conserved
 *   chi -> -chi            chirality parity flips
 *   |chi|                  conserved
 *   s * chi                conserved
 *
 * This is not a QED gauge-charge statement. It is a native FTD statement about
 * dual-substrate parity structure.
 */

#include "ftd/render_bridge.h"
#include "ftd/constants.h"

#include <cmath>
#include <iostream>
#include <string>

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

bool close(double a, double b, double tol = 1e-12) {
  return std::abs(a - b) <= tol;
}

bool close_vec(const ftd::Vec3& a, const ftd::Vec3& b, double tol = 1e-12) {
  return close(a.x, b.x, tol) && close(a.y, b.y, tol) && close(a.z, b.z, tol);
}

double dual_energy(const ftd::Voxel& v) {
  return v.flux_L.mag2() + v.flux_R.mag2() +
         v.wave_vel_L.mag2() + v.wave_vel_R.mag2();
}

void configure_weak_parent_audit(ftd::RenderBridge& rb) {
  rb.toggles.disable_all();
  rb.toggles.dual_substrate = true;
  rb.toggles.weak_transmutation = true;
  rb.seed_rng(20260422);
}

}  // namespace

int main() {
  std::cout << "================================================================\n";
  std::cout << "  TEST: Native Conserved Parent Ledger\n";
  std::cout << "================================================================\n";

  {
    std::cout << "\n-- NCP-1: Dual weak transmutation parent invariants --\n";

    ftd::RenderBridge rb(8);
    configure_weak_parent_audit(rb);

    const int x = 3, y = 3, z = 3;
    const int center = rb.lattice().index(x, y, z);
    rb.inject_particle(x, y, z, +1, {ftd::K_B, 0, 0});

    // Force stress at the particle site above threshold. The weak rule uses
    // compute_stress_left() in dual-substrate mode.
    auto& xp = rb.voxel_at(x + 1, y, z);
    xp.flux_L = {100.0 * ftd::WEAK_THRESHOLD, 0, 0};
    xp.flux_R = {};
    xp.flux = xp.flux_L;

    const auto before = rb.voxels()[center];
    const int s_before = before.state;
    const ftd::Vec3 j_before = before.flux_L + before.flux_R;
    const double e_before = dual_energy(before);
    const double chi_before = before.chirality_density();
    const double align_before = static_cast<double>(s_before) * chi_before;

    rb.tick();

    const auto after = rb.voxels()[center];
    const int s_after = after.state;
    const ftd::Vec3 j_after = after.flux_L + after.flux_R;
    const double e_after = dual_energy(after);
    const double chi_after = after.chirality_density();
    const double align_after = static_cast<double>(s_after) * chi_after;

    std::cout << "    s_before=" << s_before << " s_after=" << s_after << "\n";
    std::cout << "    chi_before=" << chi_before
              << " chi_after=" << chi_after << "\n";
    std::cout << "    s*chi before=" << align_before
              << " after=" << align_after << "\n";
    std::cout << "    dual_energy before=" << e_before
              << " after=" << e_after << "\n";
    std::cout << "    |J_after-J_before|=("
              << (j_after.x - j_before.x) << ", "
              << (j_after.y - j_before.y) << ", "
              << (j_after.z - j_before.z) << ")\n";

    check("NCP-1a: signed state flips", s_before == +1 && s_after == -1);
    check("NCP-1b: chirality parity flips", chi_before > 0.0 && chi_after < 0.0);
    check("NCP-1c: chirality magnitude conserved",
          close(std::abs(chi_before), std::abs(chi_after)));
    check("NCP-1d: state-chirality alignment conserved",
          close(align_before, align_after));
    check("NCP-1e: observable flux J_L+J_R conserved",
          close_vec(j_before, j_after));
    check("NCP-1f: dual-substrate local energy conserved",
          close(e_before, e_after));
  }

  {
    std::cout << "\n-- NCP-2: Same audit for negative initial state --\n";

    ftd::RenderBridge rb(8);
    configure_weak_parent_audit(rb);

    const int x = 3, y = 3, z = 3;
    const int center = rb.lattice().index(x, y, z);
    rb.inject_particle(x, y, z, -1, {ftd::K_B, 0, 0});

    auto& xp = rb.voxel_at(x + 1, y, z);
    xp.flux_L = {100.0 * ftd::WEAK_THRESHOLD, 0, 0};
    xp.flux_R = {};
    xp.flux = xp.flux_L;

    const auto before = rb.voxels()[center];
    const int s_before = before.state;
    const ftd::Vec3 j_before = before.flux_L + before.flux_R;
    const double e_before = dual_energy(before);
    const double chi_before = before.chirality_density();
    const double align_before = static_cast<double>(s_before) * chi_before;

    rb.tick();

    const auto after = rb.voxels()[center];
    const int s_after = after.state;
    const ftd::Vec3 j_after = after.flux_L + after.flux_R;
    const double e_after = dual_energy(after);
    const double chi_after = after.chirality_density();
    const double align_after = static_cast<double>(s_after) * chi_after;

    std::cout << "    s_before=" << s_before << " s_after=" << s_after << "\n";
    std::cout << "    chi_before=" << chi_before
              << " chi_after=" << chi_after << "\n";
    std::cout << "    s*chi before=" << align_before
              << " after=" << align_after << "\n";

    check("NCP-2a: signed state flips", s_before == -1 && s_after == +1);
    check("NCP-2b: chirality parity flips", chi_before < 0.0 && chi_after > 0.0);
    check("NCP-2c: chirality magnitude conserved",
          close(std::abs(chi_before), std::abs(chi_after)));
    check("NCP-2d: state-chirality alignment conserved",
          close(align_before, align_after));
    check("NCP-2e: observable flux J_L+J_R conserved",
          close_vec(j_before, j_after));
    check("NCP-2f: dual-substrate local energy conserved",
          close(e_before, e_after));
  }

  std::cout << "\n================================================================\n";
  if (g_failures == 0) {
    std::cout << "  Native conserved-parent audit PASSED.\n";
  } else {
    std::cout << "  " << g_failures << " conserved-parent check(s) FAILED.\n";
  }
  std::cout << "================================================================\n";

  return g_failures;
}

