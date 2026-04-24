/**
 * Native manifestation ledger for genesis and evaporation.
 *
 * Weak transmutation has a dual-substrate parent invariant. This test asks the
 * same question for genesis and evaporation in the narrow phase-write dynamics:
 * with wave propagation, movement, and Gauss projection off, does a change in
 * signed state s come with a compensating change in local field/chirality data?
 *
 * Current engine result:
 *   - genesis creates s without changing J_L, J_R, J, chi, or dual energy;
 *   - evaporation removes s without changing J_L, J_R, J, chi, or dual energy.
 *
 * Therefore genesis/evaporation are true manifestation/source rules in the
 * current engine, not parent-conserving transformations like weak
 * transmutation.
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

void configure_manifestation_audit(ftd::RenderBridge& rb) {
  rb.toggles.disable_all();
  rb.toggles.dual_substrate = true;
  rb.toggles.genesis = true;
  rb.seed_rng(20260422);
}

}  // namespace

int main() {
  std::cout << "================================================================\n";
  std::cout << "  TEST: Native Manifestation Ledger\n";
  std::cout << "================================================================\n";

  {
    std::cout << "\n-- NML-1: Genesis creates s without field exchange --\n";

    ftd::RenderBridge rb(8);
    configure_manifestation_audit(rb);

    const int center = rb.lattice().index(3, 3, 3);
    auto& v = rb.voxels()[center];
    v.flux_L = {100.0 * ftd::K_GENESIS, 0, 0};
    v.flux_R = {};
    v.wave_vel_L = {};
    v.wave_vel_R = {};
    v.flux = v.flux_L + v.flux_R;
    v.wave_vel = {};

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

    check("NML-1a: genesis creates positive signed state",
          s_before == 0 && s_after == +1);
    check("NML-1b: observable flux unchanged", close_vec(j_before, j_after));
    check("NML-1c: dual energy unchanged", close(e_before, e_after));
    check("NML-1d: chirality unchanged", close(chi_before, chi_after));
    check("NML-1e: state-chirality alignment is created",
          close(align_before, 0.0) && align_after > 0.0);
  }

  {
    std::cout << "\n-- NML-2: Evaporation removes s without field exchange --\n";

    ftd::RenderBridge rb(8);
    configure_manifestation_audit(rb);

    const int x = 3, y = 3, z = 3;
    const int center = rb.lattice().index(x, y, z);
    const double tiny_flux = 1e-5;
    rb.inject_particle(x, y, z, +1, {tiny_flux, 0, 0});

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

    check("NML-2a: evaporation removes signed state",
          s_before == +1 && s_after == 0);
    check("NML-2b: observable flux unchanged", close_vec(j_before, j_after));
    check("NML-2c: dual energy unchanged", close(e_before, e_after));
    check("NML-2d: chirality unchanged", close(chi_before, chi_after));
    check("NML-2e: state-chirality alignment is destroyed",
          align_before > 0.0 && close(align_after, 0.0));
  }

  std::cout << "\n================================================================\n";
  if (g_failures == 0) {
    std::cout << "  Native manifestation ledger PASSED.\n";
  } else {
    std::cout << "  " << g_failures << " manifestation-ledger check(s) FAILED.\n";
  }
  std::cout << "================================================================\n";

  return g_failures;
}

