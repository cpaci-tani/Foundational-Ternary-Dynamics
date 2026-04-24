/**
 * Native source-core fork audit.
 *
 * This is a test-local comparison. It does not change the production engine.
 *
 * Fork under test:
 *
 *   A. current core-boundary rule:
 *        apply Gauss correction only at void sites; skip manifested sites.
 *
 *   B. experimental include-source rule:
 *        apply the same Gauss correction at all sites, including sources.
 *
 * Both variants use the same fixed Poisson/SOR solve on the same neutral source
 * pair. The goal is not to choose by closeness to any external value, but to
 * expose what each native rule does to void residuals, source-core residuals,
 * flux, and chirality.
 */

#include "ftd/render_bridge.h"
#include "ftd/poisson_solvers.h"
#include "ftd/constants.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

static int g_failures = 0;

struct Metrics {
  double rms_all = 0.0;
  double rms_void = 0.0;
  double rms_particle = 0.0;
  double max_void = 0.0;
  double max_particle = 0.0;
  double flux_delta = 0.0;
  double particle_flux_delta = 0.0;
  double chi_abs_sum = 0.0;
  double state_chi_sum = 0.0;
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

double divergence_flux_at(const std::vector<ftd::Voxel>& voxels,
                          const ftd::Lattice& lattice,
                          int idx) {
  const auto& n = lattice.neighbors_6(idx);
  double div = 0.0;
  div += (voxels[n[0]].flux.x - voxels[n[1]].flux.x) * 0.5;
  div += (voxels[n[2]].flux.y - voxels[n[3]].flux.y) * 0.5;
  div += (voxels[n[4]].flux.z - voxels[n[5]].flux.z) * 0.5;
  return div;
}

ftd::Vec3 grad_phi_at(const std::vector<double>& phi,
                      const ftd::Lattice& lattice,
                      int idx) {
  const auto& n = lattice.neighbors_6(idx);
  return {
      (phi[n[0]] - phi[n[1]]) * 0.5,
      (phi[n[2]] - phi[n[3]]) * 0.5,
      (phi[n[4]] - phi[n[5]]) * 0.5,
  };
}

std::vector<ftd::Vec3> flux_field(const std::vector<ftd::Voxel>& voxels) {
  std::vector<ftd::Vec3> out;
  out.reserve(voxels.size());
  for (const auto& v : voxels) out.push_back(v.flux);
  return out;
}

double flux_delta_norm(const std::vector<ftd::Voxel>& voxels,
                       const std::vector<ftd::Vec3>& before) {
  double sum_sq = 0.0;
  for (size_t i = 0; i < voxels.size(); ++i) {
    const auto d = voxels[i].flux - before[i];
    sum_sq += d.mag2();
  }
  return std::sqrt(sum_sq);
}

std::vector<ftd::Voxel> seeded_pair(const ftd::Lattice& lattice) {
  ftd::RenderBridge rb(lattice.size());
  rb.toggles.disable_all();
  rb.toggles.dual_substrate = true;
  const int mid = lattice.size() / 2;
  rb.inject_particle(mid - 2, mid, mid, +1, {0, 0, ftd::K_B});
  rb.inject_particle(mid + 2, mid, mid, -1, {0, 0, -ftd::K_B});
  return rb.voxels();
}

void apply_projection_variant(std::vector<ftd::Voxel>& voxels,
                              const ftd::Lattice& lattice,
                              bool include_source_sites,
                              int sor_iters) {
  const int n = static_cast<int>(lattice.total_sites());
  std::vector<double> phi(static_cast<size_t>(n), 0.0);
  std::vector<double> source(static_cast<size_t>(n), 0.0);

  for (int i = 0; i < n; ++i) {
    source[i] = divergence_flux_at(voxels, lattice, i) -
                static_cast<double>(voxels[i].state);
  }

  for (int iter = 0; iter < sor_iters; ++iter) {
    ftd::sor_sweep_18pt(phi, source, lattice, ftd::SOR_OMEGA);
  }

  for (int i = 0; i < n; ++i) {
    if (!include_source_sites && voxels[i].state != 0) continue;
    const ftd::Vec3 grad_phi = grad_phi_at(phi, lattice, i);
    voxels[i].flux -= grad_phi;

    const ftd::Vec3 half = grad_phi * 0.5;
    voxels[i].flux_L -= half;
    voxels[i].flux_R -= half;
  }
}

Metrics measure(const std::vector<ftd::Voxel>& voxels,
                const ftd::Lattice& lattice,
                const std::vector<ftd::Vec3>& before_flux) {
  Metrics m;
  const int n = static_cast<int>(lattice.total_sites());
  double sum_all = 0.0;
  double sum_void = 0.0;
  double sum_particle = 0.0;
  int void_count = 0;
  int particle_count = 0;

  for (int i = 0; i < n; ++i) {
    const int s = voxels[i].state;
    const double err = divergence_flux_at(voxels, lattice, i) -
                       static_cast<double>(s);
    const double err2 = err * err;
    sum_all += err2;
    m.chi_abs_sum += std::abs(voxels[i].chirality_density());
    m.state_chi_sum += static_cast<double>(s) * voxels[i].chirality_density();

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
      const auto d = voxels[i].flux - before_flux[i];
      m.particle_flux_delta += d.mag2();
    }
  }

  m.rms_all = std::sqrt(sum_all / static_cast<double>(n));
  m.rms_void = std::sqrt(sum_void / static_cast<double>(std::max(1, void_count)));
  m.rms_particle =
      std::sqrt(sum_particle / static_cast<double>(std::max(1, particle_count)));
  m.flux_delta = flux_delta_norm(voxels, before_flux);
  m.particle_flux_delta = std::sqrt(m.particle_flux_delta);
  return m;
}

void print_metrics(const std::string& name, const Metrics& m) {
  std::cout << "    " << name
            << ": Q=" << m.total_charge
            << " +=" << m.positive_count
            << " -=" << m.negative_count
            << " rms_all=" << m.rms_all
            << " rms_void=" << m.rms_void
            << " max_void=" << m.max_void
            << " rms_particle=" << m.rms_particle
            << " max_particle=" << m.max_particle
            << " flux_delta=" << m.flux_delta
            << " particle_flux_delta=" << m.particle_flux_delta
            << " chi_abs_sum=" << m.chi_abs_sum
            << " state_chi_sum=" << m.state_chi_sum
            << "\n";
}

}  // namespace

int main() {
  std::cout << "================================================================\n";
  std::cout << "  TEST: Native Source-Core Fork\n";
  std::cout << "================================================================\n";

  ftd::RenderBridge rb_for_lattice(16);
  const auto& lattice = rb_for_lattice.lattice();
  const int sor_iters = 80;

  auto base_voxels = seeded_pair(lattice);
  const auto before_flux = flux_field(base_voxels);

  auto skip_voxels = base_voxels;
  apply_projection_variant(skip_voxels, lattice, false, sor_iters);

  auto include_voxels = base_voxels;
  apply_projection_variant(include_voxels, lattice, true, sor_iters);

  const auto base = measure(base_voxels, lattice, before_flux);
  const auto skip = measure(skip_voxels, lattice, before_flux);
  const auto include = measure(include_voxels, lattice, before_flux);

  std::cout << "\n-- NSC-1: Same source pair, two core prescriptions --\n";
  print_metrics("base", base);
  print_metrics("skip_source_sites", skip);
  print_metrics("include_source_sites", include);

  check("NSC-1a: source pair preserved in skip variant",
        skip.total_charge == 0 && skip.positive_count == 1 && skip.negative_count == 1);
  check("NSC-1b: source pair preserved in include variant",
        include.total_charge == 0 && include.positive_count == 1 && include.negative_count == 1);
  check("NSC-1c: skip variant changes flux", skip.flux_delta > 1e-6);
  check("NSC-1d: include variant changes flux", include.flux_delta > 1e-6);
  check("NSC-1e: skip variant improves void residual",
        skip.rms_void < base.rms_void);
  check("NSC-1f: include variant further improves void residual",
        include.rms_void < skip.rms_void);
  check("NSC-1g: include variant leaves particle residual unchanged",
        std::abs(include.rms_particle - skip.rms_particle) < 1e-12);

  // The current divergence operator samples neighboring fluxes. Changing the
  // source site's own stored flux therefore does not repair the source-core
  // residual; it only changes the field value that adjacent void sites sample.
  check("NSC-1h: skip-source rule preserves source-core flux",
        skip.particle_flux_delta < 1e-12);
  check("NSC-1i: include-source rule changes source-core flux",
        include.particle_flux_delta > 1e-6);
  check("NSC-1j: skip-source rule preserves chirality ledger",
        std::abs(skip.state_chi_sum - base.state_chi_sum) < 1e-12);
  check("NSC-1k: include-source rule preserves chirality ledger",
        std::abs(include.state_chi_sum - base.state_chi_sum) < 1e-12);

  std::cout << "\n================================================================\n";
  if (g_failures == 0) {
    std::cout << "  Native source-core fork audit PASSED.\n";
  } else {
    std::cout << "  " << g_failures << " source-core fork check(s) FAILED.\n";
  }
  std::cout << "================================================================\n";

  return g_failures;
}
