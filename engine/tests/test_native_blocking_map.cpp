/**
 * Native finite-volume blocking map audit.
 *
 * This test checks the reusable dual-cell blocking API against the native
 * Wilsonian contract:
 *
 *   Q'(X)    = sum fine-cell rho over a 2^3 block
 *   Phi'(F)  = sum fine face fluxes across the coarse face
 *   div Phi' = Q'
 */

#include "ftd/eft/dual_cell_blocking.h"
#include "ftd/test_telemetry.h"

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

void test_gauss_preservation_and_internal_cancellation() {
  std::cout << "\n-- Native B2 block preserves integrated Gauss --\n";
  ftd::eft::DualCellFields fine(4);

  // Internal face inside the left coarse block. It creates equal/opposite
  // fine sources but must not appear as coarse boundary flux.
  fine.phi_x[fine.index(0, 0, 0)] = 3.0;

  // Boundary face between the two coarse x-blocks. This is the only flux
  // that should survive into the coarse x-boundary.
  fine.phi_x[fine.index(1, 0, 0)] = 1.0;

  ftd::eft::set_source_from_divergence(fine);
  check("fine field satisfies div Phi = rho",
        ftd::eft::max_gauss_residual(fine) < 1e-12);
  check("fine total source is neutral", ftd::eft::total_source(fine) == 0);

  const auto coarse = ftd::eft::block_dual_cell_b2(fine);
  check("coarse field satisfies div Phi' = Q'",
        ftd::eft::max_gauss_residual(coarse) < 1e-12);
  check("coarse total source remains neutral", ftd::eft::total_source(coarse) == 0);
  check("left block has Q'=+1", coarse.rho_cell[coarse.index(0, 0, 0)] == 1);
  check("right block has Q'=-1", coarse.rho_cell[coarse.index(1, 0, 0)] == -1);
  check("internal fine face cancels from coarse flux",
        std::abs(coarse.phi_x[coarse.index(0, 0, 0)] - 1.0) < 1e-12);
}

void test_uniform_flux_density_preserved() {
  std::cout << "\n-- Uniform flux density preserved after area rescaling --\n";
  ftd::eft::DualCellFields fine(4);
  for (double& v : fine.phi_x) v = 2.0;
  for (double& v : fine.phi_y) v = -1.0;
  for (double& v : fine.phi_z) v = 0.5;

  ftd::eft::set_source_from_divergence(fine);
  check("uniform fine flux has zero source", ftd::eft::total_source(fine) == 0);

  const auto coarse = ftd::eft::block_dual_cell_b2(fine);
  const double area = 4.0;  // b^2 for b=2
  bool density_ok = true;
  for (size_t i = 0; i < coarse.phi_x.size(); ++i) {
    density_ok = density_ok &&
                 std::abs(coarse.phi_x[i] / area - 2.0) < 1e-12 &&
                 std::abs(coarse.phi_y[i] / area + 1.0) < 1e-12 &&
                 std::abs(coarse.phi_z[i] / area - 0.5) < 1e-12;
  }
  check("coarse integrated flux divided by b^2 equals fine density", density_ok);
  check("coarse uniform field still has zero source",
        ftd::eft::max_gauss_residual(coarse) < 1e-12);
}

void test_invalid_size_returns_empty() {
  std::cout << "\n-- Invalid fine size returns empty field --\n";
  const auto coarse = ftd::eft::block_dual_cell_b2(ftd::eft::DualCellFields(3));
  check("odd-size block request returns L=0", coarse.L == 0);
  check("empty field has no storage", coarse.rho_cell.empty());
}

}  // namespace

int main() {
  ftd::test::contract({
      "blocking/EFT/constraint",
      "[THEOREM] / [MEASUREMENT]",
      "dual-cell source/flux fields, b=2 finite-volume blocking",
      "area rescaling for face flux density",
      "gauss_violation, blocked_operator_moments",
      "periodic dual-cell L=4 -> L=2 block domain",
      "backend-independent finite-volume helper",
      "blocked source and face flux preserve integrated Gauss relation",
      "failure means the native b=2 blocking map violates finite-volume accounting"});

  std::cout << "================================================================\n";
  std::cout << "  TEST: Native Finite-Volume Blocking Map\n";
  std::cout << "================================================================\n";

  test_gauss_preservation_and_internal_cancellation();
  test_uniform_flux_density_preserved();
  test_invalid_size_returns_empty();

  std::cout << "\n================================================================\n";
  if (g_failures == 0) {
    std::cout << "  Native blocking map audit PASSED.\n";
  } else {
    std::cout << "  " << g_failures << " native blocking check(s) FAILED.\n";
  }
  std::cout << "================================================================\n";

  return g_failures;
}
