/**
 * Native Moore-layer coupling audit.
 *
 * This is a fixed diagnostic for the engine's current 18-point direct
 * operator. It checks the observed Moore-shell ordering without importing the
 * older alpha/sin^2(theta_W)/alpha_s interpretation.
 *
 * Interpretation:
 *   - SC and FCC are direct channels of the 18-point operator.
 *   - BCC is not a direct stencil channel in G18, but a nonzero BCC shell
 *     response appears after propagation/projection.
 */

#include "ftd/constants.h"
#include "ftd/render_bridge.h"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>

namespace {

int failures = 0;

const char* shell_names[] = {
    "center",
    "SC face shell",
    "FCC edge shell",
    "BCC corner shell",
};

void check(const std::string& name, bool condition) {
  if (condition) {
    std::cout << "  PASS  " << name << "\n";
  } else {
    std::cout << "  FAIL  " << name << "\n";
    ++failures;
  }
}

int moore_shell(int dx, int dy, int dz) {
  return (dx != 0 ? 1 : 0) + (dy != 0 ? 1 : 0) + (dz != 0 ? 1 : 0);
}

void configure_static_source_case(ftd::RenderBridge& rb) {
  rb.toggles.genesis = false;
  rb.toggles.forces = false;
  rb.toggles.lorentz_force = false;
  rb.toggles.movement = false;
  rb.toggles.weak_transmutation = false;
  rb.toggles.color_forces = false;
  rb.toggles.strong_force = false;
  rb.toggles.triad_binding = false;
  rb.toggles.pair_production = false;
  rb.toggles.exchange_force = false;
}

}  // namespace

int main() {
  std::cout << "================================================================\n";
  std::cout << "  TEST: Native Moore-Layer Coupling Audit\n";
  std::cout << "================================================================\n";

  const int L = 16;
  const int cx = L / 2;
  const int cy = L / 2;
  const int cz = L / 2;

  ftd::RenderBridge rb(L);
  configure_static_source_case(rb);
  rb.inject_particle(cx, cy, cz, +1, {0, 0, ftd::K_B});
  rb.voxels()[rb.lattice().index(cx, cy, cz)].locked = true;
  rb.run(500);

  double flux_shell[4] = {0, 0, 0, 0};
  double div_shell[4] = {0, 0, 0, 0};
  int count_shell[4] = {0, 0, 0, 0};

  for (int dx = -1; dx <= 1; ++dx) {
    for (int dy = -1; dy <= 1; ++dy) {
      for (int dz = -1; dz <= 1; ++dz) {
        const int shell = moore_shell(dx, dy, dz);
        const int x = (cx + dx + L) % L;
        const int y = (cy + dy + L) % L;
        const int z = (cz + dz + L) % L;
        const int idx = rb.lattice().index(x, y, z);
        flux_shell[shell] += rb.voxels()[idx].flux.mag();
        div_shell[shell] += std::abs(rb.divergence_flux(idx));
        ++count_shell[shell];
      }
    }
  }

  double mean_flux[4] = {0, 0, 0, 0};
  double mean_div[4] = {0, 0, 0, 0};
  for (int s = 0; s < 4; ++s) {
    mean_flux[s] = flux_shell[s] / static_cast<double>(count_shell[s]);
    mean_div[s] = div_shell[s] / static_cast<double>(count_shell[s]);
  }

  std::cout << "\n  Shell              Count    mean |J|        mean |div J|\n";
  std::cout << "  ---------------------------------------------------------\n";
  for (int s = 0; s < 4; ++s) {
    std::cout << "  " << std::left << std::setw(18) << shell_names[s]
              << std::right << std::setw(5) << count_shell[s] << "    "
              << std::setprecision(10) << std::setw(14) << mean_flux[s]
              << "    " << std::setw(14) << mean_div[s] << "\n";
  }

  const double fcc_over_sc = mean_flux[2] / mean_flux[1];
  const double bcc_over_sc = mean_flux[3] / mean_flux[1];
  const double neighbor_div =
      (div_shell[1] + div_shell[2] + div_shell[3]) /
      static_cast<double>(count_shell[1] + count_shell[2] + count_shell[3]);

  std::cout << "\n  Ratios relative to SC:\n";
  std::cout << "    FCC/SC = " << fcc_over_sc << "\n";
  std::cout << "    BCC/SC = " << bcc_over_sc << "\n";

  check("SC flux exceeds FCC flux", mean_flux[1] > mean_flux[2]);
  check("FCC flux exceeds BCC flux", mean_flux[2] > mean_flux[3]);
  check("BCC shell response is nonzero", mean_flux[3] > 1e-8);
  check("FCC/SC stays near the direct G18 edge-weight response",
        fcc_over_sc > 0.20 && fcc_over_sc < 0.35);
  check("BCC/SC is present but subdominant",
        bcc_over_sc > 0.10 && bcc_over_sc < 0.25);
  check("divergence remains source-centered",
        mean_div[0] > 10.0 * neighbor_div);

  std::cout << "\n================================================================\n";
  if (failures == 0) {
    std::cout << "  Native Moore-layer coupling audit PASSED.\n";
  } else {
    std::cout << "  " << failures << " Moore-layer coupling check(s) FAILED.\n";
  }
  std::cout << "================================================================\n";

  return failures;
}
