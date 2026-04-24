/**
 * Native full-tick source-response audit.
 *
 * Manifestation creates signed s without local field exchange. With Gauss
 * projection enabled, the next rule should convert that source pattern into a
 * longitudinal flux response. Because the current projection leaves particle
 * sites untouched and corrects void sites, this test compares void-site Gauss
 * residuals for identical source configurations with projection OFF vs ON.
 */

#include "ftd/render_bridge.h"
#include "ftd/constants.h"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

static int g_failures = 0;

struct ResidualMetrics {
  double rms_all = 0.0;
  double rms_void = 0.0;
  double max_void = 0.0;
  int void_count = 0;
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

int idx(const ftd::RenderBridge& rb, int x, int y, int z) {
  return rb.lattice().index(x, y, z);
}

ResidualMetrics residual_metrics(const ftd::RenderBridge& rb) {
  ResidualMetrics m;
  const int n = static_cast<int>(rb.lattice().total_sites());
  double sum_sq_all = 0.0;
  double sum_sq_void = 0.0;
  for (int i = 0; i < n; ++i) {
    const int s = rb.voxels()[i].state;
    const double err = rb.divergence_flux(i) - static_cast<double>(s);
    sum_sq_all += err * err;
    if (s == 0) {
      sum_sq_void += err * err;
      m.max_void = std::max(m.max_void, std::abs(err));
      ++m.void_count;
    } else if (s > 0) {
      ++m.positive_count;
      m.total_charge += s;
    } else {
      ++m.negative_count;
      m.total_charge += s;
    }
  }
  m.rms_all = std::sqrt(sum_sq_all / static_cast<double>(n));
  m.rms_void = std::sqrt(sum_sq_void / static_cast<double>(std::max(1, m.void_count)));
  return m;
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

void configure_projection_case(ftd::RenderBridge& rb, bool gauss_on) {
  rb.toggles.disable_all();
  rb.toggles.dual_substrate = true;
  rb.toggles.gauss_projection = gauss_on;
  rb.set_sor_iterations(80);
}

void seed_manifested_pair(ftd::RenderBridge& rb) {
  const int mid = rb.lattice().size() / 2;
  rb.inject_particle(mid - 2, mid, mid, +1, {0, 0, ftd::K_B});
  rb.inject_particle(mid + 2, mid, mid, -1, {0, 0, -ftd::K_B});
}

void seed_genesis_pair(ftd::RenderBridge& rb) {
  const int mid = rb.lattice().size() / 2;
  rb.toggles.genesis = true;

  auto& pos = rb.voxels()[idx(rb, mid - 2, mid, mid)];
  pos.flux_L = {100.0 * ftd::K_GENESIS, 0, 0};
  pos.flux_R = {};
  pos.flux = pos.flux_L + pos.flux_R;

  auto& neg = rb.voxels()[idx(rb, mid + 2, mid, mid)];
  neg.flux_L = {};
  neg.flux_R = {100.0 * ftd::K_GENESIS, 0, 0};
  neg.flux = neg.flux_L + neg.flux_R;
}

bool run_pair_projection_case(const std::string& label,
                              void (*seed)(ftd::RenderBridge&)) {
  ftd::RenderBridge off(16);
  configure_projection_case(off, false);
  seed(off);
  const auto off_flux_before = flux_field(off);
  off.tick();
  const auto off_metrics = residual_metrics(off);
  const double off_flux_delta = flux_delta_norm(off, off_flux_before);

  ftd::RenderBridge on(16);
  configure_projection_case(on, true);
  seed(on);
  const auto on_flux_before = flux_field(on);
  on.tick();
  const auto on_metrics = residual_metrics(on);
  const double on_flux_delta = flux_delta_norm(on, on_flux_before);

  std::cout << "    " << label << "\n";
  std::cout << "      off: Q=" << off_metrics.total_charge
            << " +=" << off_metrics.positive_count
            << " -=" << off_metrics.negative_count
            << " rms_void=" << off_metrics.rms_void
            << " max_void=" << off_metrics.max_void
            << " flux_delta=" << off_flux_delta << "\n";
  std::cout << "       on: Q=" << on_metrics.total_charge
            << " +=" << on_metrics.positive_count
            << " -=" << on_metrics.negative_count
            << " rms_void=" << on_metrics.rms_void
            << " max_void=" << on_metrics.max_void
            << " flux_delta=" << on_flux_delta << "\n";

  const bool same_sources =
      off_metrics.total_charge == 0 &&
      on_metrics.total_charge == 0 &&
      off_metrics.positive_count == 1 &&
      off_metrics.negative_count == 1 &&
      on_metrics.positive_count == 1 &&
      on_metrics.negative_count == 1;

  check(label + ": source pair is net neutral and preserved", same_sources);
  check(label + ": projection changes flux field", on_flux_delta > 1e-6);
  check(label + ": no-projection leaves flux field unchanged", off_flux_delta < 1e-12);
  check(label + ": projection reduces void RMS residual",
        on_metrics.rms_void < off_metrics.rms_void);
  check(label + ": projection reduces max void residual",
        on_metrics.max_void < off_metrics.max_void);

  return same_sources &&
         on_flux_delta > 1e-6 &&
         off_flux_delta < 1e-12 &&
         on_metrics.rms_void < off_metrics.rms_void &&
         on_metrics.max_void < off_metrics.max_void;
}

}  // namespace

int main() {
  std::cout << "================================================================\n";
  std::cout << "  TEST: Native Source Response\n";
  std::cout << "================================================================\n";

  std::cout << "\n-- NSR-1: Fixed manifested source pair --\n";
  run_pair_projection_case("NSR-1", seed_manifested_pair);

  std::cout << "\n-- NSR-2: Genesis-created source pair --\n";
  run_pair_projection_case("NSR-2", seed_genesis_pair);

  std::cout << "\n================================================================\n";
  if (g_failures == 0) {
    std::cout << "  Native source-response audit PASSED.\n";
  } else {
    std::cout << "  " << g_failures << " source-response check(s) FAILED.\n";
  }
  std::cout << "================================================================\n";

  return g_failures;
}

