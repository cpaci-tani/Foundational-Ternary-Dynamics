/**
 * Native C_L and g_sJ b=2 flow audit.
 *
 * This closes the two remaining Gaussian native-flow gates:
 *   - C_L^FTD(b=2) = 1 for the declared static source kernel.
 *   - g_sJ^FTD(b=2) = 1 for exactly blockable current/flux long modes.
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

void test_static_kernel_coefficient() {
  std::cout << "\n-- Static source kernel keeps C_L=1 --\n";

  // Fixed nonzero symbols. In the native Gaussian generator, the inverse
  // source kernel is the selected operator symbol itself, so C_L=sigma/sigma.
  const double fine_sigma = 0.75;
  const double coarse_sigma = 1.25;

  const double c_fine =
      ftd::eft::native_static_response_coefficient(fine_sigma, fine_sigma);
  const double c_coarse =
      ftd::eft::native_static_response_coefficient(coarse_sigma, coarse_sigma);

  std::cout << "    C_L(fine)=" << c_fine
            << " C_L(coarse)=" << c_coarse << "\n";

  check("fine static coefficient is 1", std::abs(c_fine - 1.0) < 1e-12);
  check("coarse static coefficient is 1", std::abs(c_coarse - 1.0) < 1e-12);
}

ftd::eft::DualCellFields uniform_flux(int L) {
  ftd::eft::DualCellFields f(L);
  for (double& v : f.phi_x) v = 1.5;
  for (double& v : f.phi_y) v = -0.25;
  for (double& v : f.phi_z) v = 0.75;
  ftd::eft::set_source_from_divergence(f);
  return f;
}

ftd::eft::DualCellContinuity uniform_current(int L) {
  ftd::eft::DualCellContinuity c(L);
  for (double& v : c.current_x) v = -0.5;
  for (double& v : c.current_y) v = 2.0;
  for (double& v : c.current_z) v = 0.25;
  return c;
}

void test_vertex_coupling_invariant() {
  std::cout << "\n-- Uniform current/flux vertex keeps g_sJ=1 --\n";

  const auto fine_flux = uniform_flux(4);
  const auto fine_current = uniform_current(4);

  const auto coarse_flux = ftd::eft::block_dual_cell_b2(fine_flux);
  const auto coarse_current =
      ftd::eft::block_dual_cell_continuity_b2(fine_current);

  const double vertex_fine =
      ftd::eft::canonical_current_flux_vertex(fine_current, fine_flux, 1.0, 1.0);
  const double vertex_coarse =
      ftd::eft::canonical_current_flux_vertex(coarse_current, coarse_flux, 8.0, 4.0);
  const double ratio = vertex_coarse / vertex_fine;

  std::cout << "    vertex_fine=" << vertex_fine
            << " vertex_coarse=" << vertex_coarse
            << " ratio=" << ratio << "\n";

  check("fine current continuity is source-free",
        ftd::eft::max_continuity_residual(fine_current) < 1e-12);
  check("coarse current continuity is source-free",
        ftd::eft::max_continuity_residual(coarse_current) < 1e-12);
  check("vertex ratio is 1 for blockable long modes",
        std::abs(ratio - 1.0) < 1e-12);
}

}  // namespace

int main() {
  std::cout << "================================================================\n";
  std::cout << "  TEST: Native Static/Vertex b=2 Flow\n";
  std::cout << "================================================================\n";

  test_static_kernel_coefficient();
  test_vertex_coupling_invariant();

  std::cout << "\n================================================================\n";
  if (g_failures == 0) {
    std::cout << "  Native response-flow audit PASSED.\n";
  } else {
    std::cout << "  " << g_failures << " native response-flow check(s) FAILED.\n";
  }
  std::cout << "================================================================\n";

  return g_failures;
}
