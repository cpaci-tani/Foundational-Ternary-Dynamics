/**
 * Native current-flow audit for finite-volume b=2 blocking.
 *
 * Verifies that the native continuity equation
 *
 *   Delta rho + div I = S_reaction
 *
 * survives b=2 blocking exactly, with integrated current normalization
 * Z_j^FTD(b) = 1 for signed transport.
 */

#include "ftd/eft/dual_cell_continuity.h"

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

void move_x(ftd::eft::DualCellContinuity& f,
            int x, int y, int z,
            int charge) {
  const int src = f.index(x, y, z);
  const int dst = f.index(x + 1, y, z);
  f.rho_before[src] += charge;
  f.rho_after[dst] += charge;
  f.current_x[src] += static_cast<double>(charge);
}

void create_source(ftd::eft::DualCellContinuity& f,
                   int x, int y, int z,
                   int charge) {
  const int i = f.index(x, y, z);
  f.rho_after[i] += charge;
  f.reaction[i] += charge;
}

void test_transport_and_reaction_blocking() {
  std::cout << "\n-- Transport plus reaction continuity blocks exactly --\n";
  ftd::eft::DualCellContinuity fine(4);

  // Crosses the boundary between coarse x-blocks: survives as coarse current.
  move_x(fine, 1, 0, 0, +1);

  // Internal to the left coarse block: affects fine continuity but cancels
  // from the coarse boundary-current variable.
  move_x(fine, 0, 1, 0, +1);

  // A true reaction source inside the second coarse x-block.
  create_source(fine, 2, 2, 0, -1);

  check("fine continuity closes",
        ftd::eft::max_continuity_residual(fine) < 1e-12);

  const auto coarse = ftd::eft::block_dual_cell_continuity_b2(fine);
  check("coarse continuity closes",
        ftd::eft::max_continuity_residual(coarse) < 1e-12);
  check("total before conserved under blocking",
        ftd::eft::total_before(coarse) == ftd::eft::total_before(fine));
  check("total after conserved under blocking",
        ftd::eft::total_after(coarse) == ftd::eft::total_after(fine));
  check("total reaction conserved under blocking",
        ftd::eft::total_reaction(coarse) == ftd::eft::total_reaction(fine));

  const int left = coarse.index(0, 0, 0);
  const int right = coarse.index(1, 0, 0);
  check("coarse boundary current keeps unit transport",
        std::abs(coarse.current_x[left] - 1.0) < 1e-12);
  check("right block receives transported source",
        coarse.rho_after[right] - coarse.rho_before[right] == 1);
}

void test_invalid_size_returns_empty() {
  std::cout << "\n-- Invalid continuity block returns empty field --\n";
  const auto coarse =
      ftd::eft::block_dual_cell_continuity_b2(ftd::eft::DualCellContinuity(3));
  check("odd-size continuity block returns L=0", coarse.L == 0);
  check("empty continuity block has no storage", coarse.rho_before.empty());
}

}  // namespace

int main() {
  std::cout << "================================================================\n";
  std::cout << "  TEST: Native Current b=2 Flow\n";
  std::cout << "================================================================\n";

  test_transport_and_reaction_blocking();
  test_invalid_size_returns_empty();

  std::cout << "\n================================================================\n";
  if (g_failures == 0) {
    std::cout << "  Native current-flow audit PASSED.\n";
  } else {
    std::cout << "  " << g_failures << " native current-flow check(s) FAILED.\n";
  }
  std::cout << "================================================================\n";

  return g_failures;
}
