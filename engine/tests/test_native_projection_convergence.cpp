/**
 * Native Gauss-projection convergence audit.
 *
 * Fixed SOR ladder, no fitting/search:
 *
 *   iters = {0, 1, 2, 5, 10, 20, 40, 80, 160}
 *
 * The current projection rule corrects void sites and skips particle sites.
 * This audit records void-site and particle-site residuals separately for the
 * same neutral source pair. It verifies the robust native statement:
 *
 *   increasing projection effort changes longitudinal flux and reduces
 *   void-site source residuals relative to no projection.
 */

#include "ftd/render_bridge.h"
#include "ftd/constants.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

static int g_failures = 0;

struct Metrics {
  int sor_iters = 0;
  double rms_all = 0.0;
  double rms_void = 0.0;
  double rms_particle = 0.0;
  double max_void = 0.0;
  double max_particle = 0.0;
  double flux_delta = 0.0;
  int positive_count = 0;
  int negative_count = 0;
  int total_charge = 0;
};

void check(const std::string& name, bool condition) {
  if (condition) {
    std::cout << "  PASS  " << name << "\n";
  } else {
    std::cout << "  FAIL  " << name << "\n";
    ++g_failures;
  }
}

std::vector<ftd::Vec3> flux_field(const ftd::RenderBridge& rb) {
  std::vector<ftd::Vec3> out;
  out.reserve(static_cast<size_t>(rb.lattice().total_sites()));
  for (const auto& v : rb.voxels()) out.push_back(v.flux);
  return out;
}

double flux_delta_norm(const ftd::RenderBridge& rb,
                       const std::vector<ftd::Vec3>& before) {
  double sum_sq = 0.0;
  for (size_t i = 0; i < before.size(); ++i) {
    const auto d = rb.voxels()[i].flux - before[i];
    sum_sq += d.mag2();
  }
  return std::sqrt(sum_sq);
}

void configure_case(ftd::RenderBridge& rb, int sor_iters) {
  rb.toggles.disable_all();
  rb.toggles.dual_substrate = true;
  rb.toggles.gauss_projection = (sor_iters > 0);
  rb.set_sor_iterations(std::max(1, sor_iters));
}

void seed_manifested_pair(ftd::RenderBridge& rb) {
  const int mid = rb.lattice().size() / 2;
  rb.inject_particle(mid - 2, mid, mid, +1, {0, 0, ftd::K_B});
  rb.inject_particle(mid + 2, mid, mid, -1, {0, 0, -ftd::K_B});
}

Metrics measure_after_one_tick(int sor_iters) {
  ftd::RenderBridge rb(16);
  configure_case(rb, sor_iters);
  seed_manifested_pair(rb);
  const auto before_flux = flux_field(rb);
  rb.tick();

  Metrics m;
  m.sor_iters = sor_iters;
  m.flux_delta = flux_delta_norm(rb, before_flux);

  const int n = static_cast<int>(rb.lattice().total_sites());
  double sum_all = 0.0;
  double sum_void = 0.0;
  double sum_particle = 0.0;
  int void_count = 0;
  int particle_count = 0;

  for (int i = 0; i < n; ++i) {
    const int s = rb.voxels()[i].state;
    const double err = rb.divergence_flux(i) - static_cast<double>(s);
    const double err2 = err * err;
    sum_all += err2;
    if (s == 0) {
      sum_void += err2;
      m.max_void = std::max(m.max_void, std::abs(err));
      ++void_count;
    } else {
      sum_particle += err2;
      m.max_particle = std::max(m.max_particle, std::abs(err));
      ++particle_count;
      if (s > 0) ++m.positive_count;
      if (s < 0) ++m.negative_count;
      m.total_charge += s;
    }
  }

  m.rms_all = std::sqrt(sum_all / static_cast<double>(n));
  m.rms_void = std::sqrt(sum_void / static_cast<double>(std::max(1, void_count)));
  m.rms_particle =
      std::sqrt(sum_particle / static_cast<double>(std::max(1, particle_count)));
  return m;
}

}  // namespace

int main() {
  std::cout << "================================================================\n";
  std::cout << "  TEST: Native Projection Convergence\n";
  std::cout << "================================================================\n";

  const std::vector<int> ladder = {0, 1, 2, 5, 10, 20, 40, 80, 160};
  std::vector<Metrics> rows;
  rows.reserve(ladder.size());

  std::cout << "\n-- NPC-1: Fixed neutral pair, SOR ladder --\n";
  for (int iters : ladder) {
    rows.push_back(measure_after_one_tick(iters));
    const auto& m = rows.back();
    std::cout << "    sor=" << m.sor_iters
              << " Q=" << m.total_charge
              << " +=" << m.positive_count
              << " -=" << m.negative_count
              << " rms_void=" << m.rms_void
              << " max_void=" << m.max_void
              << " rms_particle=" << m.rms_particle
              << " max_particle=" << m.max_particle
              << " flux_delta=" << m.flux_delta << "\n";
  }

  const auto& base = rows.front();
  const auto& high = rows.back();
  const auto& mid = rows[5];  // 20 iterations

  check("NPC-1a: source pair preserved",
        high.total_charge == 0 && high.positive_count == 1 && high.negative_count == 1);
  check("NPC-1b: no-projection has zero flux delta", base.flux_delta < 1e-12);
  check("NPC-1c: high projection changes flux", high.flux_delta > 1e-6);
  check("NPC-1d: 20-iteration projection reduces void RMS",
        mid.rms_void < base.rms_void);
  check("NPC-1e: 160-iteration projection reduces void RMS",
        high.rms_void < base.rms_void);
  check("NPC-1f: 160-iteration projection reduces max void residual",
        high.max_void < base.max_void);

  // Particle sites are skipped during projection. Their residual may move
  // indirectly because neighboring void flux changes, but it is not driven to
  // zero by this rule. The audit records this as intended current behavior.
  check("NPC-1g: particle residual remains nonzero under current skip rule",
        high.rms_particle > 0.1);

  std::cout << "\n================================================================\n";
  if (g_failures == 0) {
    std::cout << "  Native projection-convergence audit PASSED.\n";
  } else {
    std::cout << "  " << g_failures << " projection-convergence check(s) FAILED.\n";
  }
  std::cout << "================================================================\n";

  return g_failures;
}

