/**
 * Native bare-flow audit for the dual-cell blocking map.
 *
 * This tests the first nontrivial RG invariant of the Gaussian native EFT:
 * an exactly blockable uniform flux-density mode keeps the same canonical
 * energy after b=2 finite-volume blocking.
 */

#include "ftd/eft/dual_cell_flow.h"

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

ftd::eft::DualCellFields uniform_flux_field(int L) {
  ftd::eft::DualCellFields f(L);
  for (double& v : f.phi_x) v = 2.0;
  for (double& v : f.phi_y) v = -1.0;
  for (double& v : f.phi_z) v = 0.5;
  ftd::eft::set_source_from_divergence(f);
  return f;
}

ftd::eft::DualCellFields boundary_flux_pair() {
  ftd::eft::DualCellFields f(4);
  f.phi_x[f.index(1, 0, 0)] = 1.0;
  ftd::eft::set_source_from_divergence(f);
  return f;
}

void test_uniform_mode_energy_invariant() {
  std::cout << "\n-- Uniform mode has invariant canonical flux energy --\n";
  const auto fine = uniform_flux_field(4);
  const auto report = ftd::eft::measure_native_b2_flow(fine);

  std::cout << "    fine_E=" << report.flux_energy_fine
            << " coarse_E=" << report.flux_energy_coarse
            << " ratio=" << report.flux_energy_ratio
            << " Qfine=" << report.total_source_fine
            << " Qcoarse=" << report.total_source_coarse << "\n";

  check("source conserved", report.source_conserved);
  check("Gauss preserved", report.gauss_preserved);
  check("uniform-mode energy ratio is 1",
        std::abs(report.flux_energy_ratio - 1.0) < 1e-12);
}

void test_short_mode_energy_decreases_under_blocking() {
  std::cout << "\n-- Short/internal structure is integrated out by blocking --\n";
  auto fine = boundary_flux_pair();
  // Add a fine face internal to the left coarse block. It contributes to fine
  // energy but cancels out of the coarse boundary variables.
  fine.phi_x[fine.index(0, 0, 0)] = 3.0;
  ftd::eft::set_source_from_divergence(fine);

  const auto report = ftd::eft::measure_native_b2_flow(fine);
  std::cout << "    fine_E=" << report.flux_energy_fine
            << " coarse_E=" << report.flux_energy_coarse
            << " ratio=" << report.flux_energy_ratio << "\n";

  check("source conserved with short mode", report.source_conserved);
  check("Gauss preserved with short mode", report.gauss_preserved);
  check("short-mode energy ratio is below 1", report.flux_energy_ratio < 1.0);
}

}  // namespace

int main() {
  std::cout << "================================================================\n";
  std::cout << "  TEST: Native Bare b=2 Flow\n";
  std::cout << "================================================================\n";

  test_uniform_mode_energy_invariant();
  test_short_mode_energy_decreases_under_blocking();

  std::cout << "\n================================================================\n";
  if (g_failures == 0) {
    std::cout << "  Native bare-flow audit PASSED.\n";
  } else {
    std::cout << "  " << g_failures << " native flow check(s) FAILED.\n";
  }
  std::cout << "================================================================\n";

  return g_failures;
}
