/**
 * Native Moore temporal-layer audit.
 *
 * This fixed audit separates direct stencil support from causal-shell timing.
 * With the current G18 operator, SC and FCC are one-tick direct channels while
 * BCC/corner sites are not direct neighbors of the Laplacian. A BCC response
 * can still appear through multi-step propagation.
 */

#include "ftd/render_bridge.h"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>

namespace {

int failures = 0;

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

void configure_pure_wave(ftd::RenderBridge& rb) {
  rb.toggles.disable_all();
  rb.toggles.wave_propagation = true;
}

struct ShellMeans {
  double center = 0.0;
  double sc = 0.0;
  double fcc = 0.0;
  double bcc = 0.0;
};

ShellMeans measure_shells(const ftd::RenderBridge& rb, int cx, int cy, int cz) {
  double sum[4] = {0, 0, 0, 0};
  int count[4] = {0, 0, 0, 0};

  const int L = rb.lattice().size();
  for (int dx = -1; dx <= 1; ++dx) {
    for (int dy = -1; dy <= 1; ++dy) {
      for (int dz = -1; dz <= 1; ++dz) {
        const int shell = moore_shell(dx, dy, dz);
        const int x = (cx + dx + L) % L;
        const int y = (cy + dy + L) % L;
        const int z = (cz + dz + L) % L;
        const int idx = rb.lattice().index(x, y, z);
        sum[shell] += rb.voxels()[idx].flux.mag();
        ++count[shell];
      }
    }
  }

  ShellMeans m;
  m.center = sum[0] / static_cast<double>(count[0]);
  m.sc = sum[1] / static_cast<double>(count[1]);
  m.fcc = sum[2] / static_cast<double>(count[2]);
  m.bcc = sum[3] / static_cast<double>(count[3]);
  return m;
}

}  // namespace

int main() {
  std::cout << "================================================================\n";
  std::cout << "  TEST: Native Moore Temporal-Layer Audit\n";
  std::cout << "================================================================\n";

  const int L = 16;
  const int c = L / 2;
  ftd::RenderBridge rb(L);
  configure_pure_wave(rb);
  rb.inject_flux(c, c, c, {0, 0, 1.0});

  ShellMeans by_tick[7];
  by_tick[0] = measure_shells(rb, c, c, c);
  for (int t = 1; t <= 6; ++t) {
    rb.tick();
    by_tick[t] = measure_shells(rb, c, c, c);
  }

  std::cout << "\n  tick       center             SC            FCC            BCC\n";
  std::cout << "  ----------------------------------------------------------------\n";
  for (int t = 0; t <= 6; ++t) {
    std::cout << "  " << std::setw(4) << t
              << "  " << std::scientific << std::setprecision(8)
              << std::setw(14) << by_tick[t].center
              << "  " << std::setw(14) << by_tick[t].sc
              << "  " << std::setw(14) << by_tick[t].fcc
              << "  " << std::setw(14) << by_tick[t].bcc << "\n";
  }

  check("t=0 starts localized at center",
        by_tick[0].center > 0.0 &&
        by_tick[0].sc < 1e-14 &&
        by_tick[0].fcc < 1e-14 &&
        by_tick[0].bcc < 1e-14);
  check("t=1 reaches direct SC shell", by_tick[1].sc > 1e-8);
  check("t=1 reaches direct FCC shell", by_tick[1].fcc > 1e-8);
  check("t=1 does not reach BCC corner shell directly", by_tick[1].bcc < 1e-14);
  check("BCC appears through propagation after t=1", by_tick[2].bcc > 1e-8);
  check("BCC remains subdominant at early ticks",
        by_tick[2].bcc < by_tick[2].sc && by_tick[2].bcc < by_tick[2].fcc);

  std::cout << "\n================================================================\n";
  if (failures == 0) {
    std::cout << "  Native Moore temporal-layer audit PASSED.\n";
  } else {
    std::cout << "  " << failures << " temporal-layer check(s) FAILED.\n";
  }
  std::cout << "================================================================\n";

  return failures;
}
