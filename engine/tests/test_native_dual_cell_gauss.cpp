/**
 * Native dual-cell Gauss audit.
 *
 * This is a test-local finite-volume comparison. It does not change the
 * production engine.
 *
 * The current engine stores J on voxel records and evaluates div(J) by central
 * differences. That is equivalent to using averaged face fluxes, where the
 * source site's own stored J cancels out of div(J) at that site. This test
 * builds the explicit dual-cell version:
 *
 *   - s lives inside a cell.
 *   - face fluxes live on oriented cell boundaries.
 *   - div_face(F) is flux out of the cell boundary.
 *
 * The audit asks whether a true face-centered correction can satisfy Gauss at
 * the source cell, unlike simply changing the collocated source-site J storage.
 */

#include "ftd/render_bridge.h"
#include "ftd/lattice.h"
#include "ftd/voxel.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

namespace {

static int g_failures = 0;

struct FaceFlux {
  std::vector<double> fx;  // oriented +x face of each cell
  std::vector<double> fy;  // oriented +y face of each cell
  std::vector<double> fz;  // oriented +z face of each cell
};

struct Metrics {
  double rms_all = 0.0;
  double rms_void = 0.0;
  double rms_particle = 0.0;
  double max_all = 0.0;
  double max_void = 0.0;
  double max_particle = 0.0;
  double flux_delta = 0.0;
  double residual_sum = 0.0;
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

std::vector<ftd::Voxel> seeded_pair(const ftd::Lattice& lattice) {
  ftd::RenderBridge rb(lattice.size());
  rb.toggles.disable_all();
  rb.toggles.dual_substrate = true;
  const int mid = lattice.size() / 2;
  rb.inject_particle(mid - 2, mid, mid, +1, {0, 0, ftd::K_B});
  rb.inject_particle(mid + 2, mid, mid, -1, {0, 0, -ftd::K_B});
  return rb.voxels();
}

FaceFlux face_flux_from_cell_flux(const std::vector<ftd::Voxel>& voxels,
                                  const ftd::Lattice& lattice) {
  const int n = static_cast<int>(lattice.total_sites());
  FaceFlux f;
  f.fx.resize(static_cast<size_t>(n));
  f.fy.resize(static_cast<size_t>(n));
  f.fz.resize(static_cast<size_t>(n));

  for (int i = 0; i < n; ++i) {
    const auto c = lattice.coord(i);
    const int xp = lattice.index(c.x + 1, c.y, c.z);
    const int yp = lattice.index(c.x, c.y + 1, c.z);
    const int zp = lattice.index(c.x, c.y, c.z + 1);
    f.fx[i] = 0.5 * (voxels[i].flux.x + voxels[xp].flux.x);
    f.fy[i] = 0.5 * (voxels[i].flux.y + voxels[yp].flux.y);
    f.fz[i] = 0.5 * (voxels[i].flux.z + voxels[zp].flux.z);
  }
  return f;
}

double face_flux_delta_norm(const FaceFlux& a, const FaceFlux& b) {
  double sum_sq = 0.0;
  for (size_t i = 0; i < a.fx.size(); ++i) {
    const double dx = a.fx[i] - b.fx[i];
    const double dy = a.fy[i] - b.fy[i];
    const double dz = a.fz[i] - b.fz[i];
    sum_sq += dx * dx + dy * dy + dz * dz;
  }
  return std::sqrt(sum_sq);
}

double div_face_at(const FaceFlux& f, const ftd::Lattice& lattice, int idx) {
  const auto c = lattice.coord(idx);
  const int xm = lattice.index(c.x - 1, c.y, c.z);
  const int ym = lattice.index(c.x, c.y - 1, c.z);
  const int zm = lattice.index(c.x, c.y, c.z - 1);
  return (f.fx[idx] - f.fx[xm]) +
         (f.fy[idx] - f.fy[ym]) +
         (f.fz[idx] - f.fz[zm]);
}

double lap6_at(const std::vector<double>& phi,
               const ftd::Lattice& lattice,
               int idx) {
  const auto& n = lattice.neighbors_6(idx);
  return phi[n[0]] + phi[n[1]] + phi[n[2]] +
         phi[n[3]] + phi[n[4]] + phi[n[5]] -
         6.0 * phi[idx];
}

void subtract_mean(std::vector<double>& phi) {
  const double sum = std::accumulate(phi.begin(), phi.end(), 0.0);
  const double mean = sum / static_cast<double>(phi.size());
  for (double& x : phi) x -= mean;
}

std::vector<double> solve_dual_cell_phi(const std::vector<double>& source,
                                        const ftd::Lattice& lattice,
                                        int sor_iters,
                                        double omega) {
  const int n = static_cast<int>(lattice.total_sites());
  std::vector<double> phi(static_cast<size_t>(n), 0.0);

  for (int iter = 0; iter < sor_iters; ++iter) {
    for (int parity = 0; parity < 2; ++parity) {
      for (int i = 0; i < n; ++i) {
        const auto c = lattice.coord(i);
        if (((c.x + c.y + c.z) & 1) != parity) continue;
        const auto& nbrs = lattice.neighbors_6(i);
        const double neighbor_sum =
            phi[nbrs[0]] + phi[nbrs[1]] + phi[nbrs[2]] +
            phi[nbrs[3]] + phi[nbrs[4]] + phi[nbrs[5]];
        const double next_phi = (neighbor_sum - source[i]) / 6.0;
        phi[i] = (1.0 - omega) * phi[i] + omega * next_phi;
      }
    }
    if ((iter + 1) % 50 == 0) subtract_mean(phi);
  }
  subtract_mean(phi);
  return phi;
}

void apply_dual_cell_projection(FaceFlux& f,
                                const std::vector<double>& phi,
                                const ftd::Lattice& lattice) {
  const int n = static_cast<int>(lattice.total_sites());
  for (int i = 0; i < n; ++i) {
    const auto c = lattice.coord(i);
    const int xp = lattice.index(c.x + 1, c.y, c.z);
    const int yp = lattice.index(c.x, c.y + 1, c.z);
    const int zp = lattice.index(c.x, c.y, c.z + 1);
    f.fx[i] -= phi[xp] - phi[i];
    f.fy[i] -= phi[yp] - phi[i];
    f.fz[i] -= phi[zp] - phi[i];
  }
}

Metrics measure_face_gauss(const FaceFlux& f,
                           const std::vector<ftd::Voxel>& voxels,
                           const ftd::Lattice& lattice,
                           const FaceFlux& before) {
  Metrics m;
  const int n = static_cast<int>(lattice.total_sites());
  double sum_all = 0.0;
  double sum_void = 0.0;
  double sum_particle = 0.0;
  int void_count = 0;
  int particle_count = 0;

  for (int i = 0; i < n; ++i) {
    const int s = voxels[i].state;
    const double err = div_face_at(f, lattice, i) - static_cast<double>(s);
    const double abs_err = std::abs(err);
    const double err2 = err * err;
    sum_all += err2;
    m.residual_sum += err;
    m.max_all = std::max(m.max_all, abs_err);

    if (s == 0) {
      sum_void += err2;
      m.max_void = std::max(m.max_void, abs_err);
      ++void_count;
    } else {
      sum_particle += err2;
      m.max_particle = std::max(m.max_particle, abs_err);
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
  m.flux_delta = face_flux_delta_norm(f, before);
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
            << " max_all=" << m.max_all
            << " flux_delta=" << m.flux_delta
            << " residual_sum=" << m.residual_sum
            << "\n";
}

}  // namespace

int main() {
  std::cout << "================================================================\n";
  std::cout << "  TEST: Native Dual-Cell Gauss\n";
  std::cout << "================================================================\n";

  ftd::RenderBridge rb_for_lattice(16);
  const auto& lattice = rb_for_lattice.lattice();
  const int n = static_cast<int>(lattice.total_sites());
  const int sor_iters = 2000;
  const double omega = 1.85;

  const auto voxels = seeded_pair(lattice);
  const FaceFlux base_flux = face_flux_from_cell_flux(voxels, lattice);

  std::vector<double> source(static_cast<size_t>(n), 0.0);
  for (int i = 0; i < n; ++i) {
    source[i] = div_face_at(base_flux, lattice, i) -
                static_cast<double>(voxels[i].state);
  }

  const double source_sum = std::accumulate(source.begin(), source.end(), 0.0);
  const auto phi = solve_dual_cell_phi(source, lattice, sor_iters, omega);

  double lap_residual_max = 0.0;
  double lap_residual_rms = 0.0;
  for (int i = 0; i < n; ++i) {
    const double r = lap6_at(phi, lattice, i) - source[i];
    lap_residual_max = std::max(lap_residual_max, std::abs(r));
    lap_residual_rms += r * r;
  }
  lap_residual_rms = std::sqrt(lap_residual_rms / static_cast<double>(n));

  FaceFlux projected_flux = base_flux;
  apply_dual_cell_projection(projected_flux, phi, lattice);

  const auto base = measure_face_gauss(base_flux, voxels, lattice, base_flux);
  const auto projected =
      measure_face_gauss(projected_flux, voxels, lattice, base_flux);

  std::cout << "\n-- NDC-1: Dual-cell finite-volume source pair --\n";
  std::cout << "    source_sum=" << source_sum
            << " lap_residual_rms=" << lap_residual_rms
            << " lap_residual_max=" << lap_residual_max
            << " sor_iters=" << sor_iters
            << " omega=" << omega << "\n";
  print_metrics("base_face_flux", base);
  print_metrics("dual_cell_projected", projected);

  check("NDC-1a: neutral source is compatible with periodic solve",
        std::abs(source_sum) < 1e-12);
  check("NDC-1b: dual-cell Poisson solve converged",
        lap_residual_rms < 1e-10 && lap_residual_max < 1e-8);
  check("NDC-1c: source pair preserved",
        projected.total_charge == 0 &&
        projected.positive_count == 1 &&
        projected.negative_count == 1);
  check("NDC-1d: dual-cell projection changes face flux",
        projected.flux_delta > 1e-6);
  check("NDC-1e: dual-cell projection improves all-site residual",
        projected.rms_all < base.rms_all);
  check("NDC-1f: dual-cell projection improves source-cell residual",
        projected.rms_particle < base.rms_particle);
  check("NDC-1g: dual-cell residual is near zero on all cells",
        projected.rms_all < 1e-10 && projected.max_all < 1e-8);

  std::cout << "\n================================================================\n";
  if (g_failures == 0) {
    std::cout << "  Native dual-cell Gauss audit PASSED.\n";
  } else {
    std::cout << "  " << g_failures << " dual-cell Gauss check(s) FAILED.\n";
  }
  std::cout << "================================================================\n";

  return g_failures;
}
