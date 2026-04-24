/**
 * Native Moore-shell Gauss audit.
 *
 * This is a fixed, test-local operator comparison. It does not change the
 * production engine and it does not search weights.
 *
 * The source is a single internal voxel. The boundary is compared at three
 * fixed shell levels:
 *
 *   G6:  cubic face shell.
 *   G18: face + edge shell with the engine's 18-point isotropic weights.
 *   G26_equal_layer: full Moore shell with an explicit BCC/corner channel and
 *                    equal small-k contribution from the 6, 12, and 8 layers.
 *   G26_iso_mid:     nonzero-BCC midpoint of the fourth-order isotropic Moore
 *                    family.
 *   G26_iso_corner:  BCC endpoint of the fourth-order isotropic Moore family.
 *
 * The equal-layer G26 weights use equal Moore-layer small-k contribution:
 *
 *   face layer   contributes 1/3 of |k|^2
 *   edge layer   contributes 1/3 of |k|^2
 *   corner layer contributes 1/3 of |k|^2
 *
 * This is a declared [SELECTION], not a tuned value. The isotropic G26 weights
 * are also fixed declarations from the analytic one-parameter family:
 *
 *   a = 1/3 + 4c, b = 1/6 - 2c, 0 <= c <= 1/12.
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

struct Direction {
  int dx;
  int dy;
  int dz;
  double weight;
  int shell;
};

struct ShellFlux {
  std::vector<Direction> dirs;
  std::vector<std::vector<double>> link;
};

struct Metrics {
  double rms_all = 0.0;
  double rms_void = 0.0;
  double rms_particle = 0.0;
  double max_all = 0.0;
  double max_void = 0.0;
  double max_particle = 0.0;
  double flux_delta = 0.0;
  double face_delta = 0.0;
  double edge_delta = 0.0;
  double corner_delta = 0.0;
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

std::vector<Direction> dirs_g6() {
  return {
      {1, 0, 0, 1.0, 6},
      {0, 1, 0, 1.0, 6},
      {0, 0, 1, 1.0, 6},
  };
}

std::vector<Direction> edge_positive_half(double weight) {
  return {
      {1, 1, 0, weight, 12},
      {1, -1, 0, weight, 12},
      {1, 0, 1, weight, 12},
      {1, 0, -1, weight, 12},
      {0, 1, 1, weight, 12},
      {0, 1, -1, weight, 12},
  };
}

std::vector<Direction> corner_positive_half(double weight) {
  return {
      {1, 1, 1, weight, 8},
      {1, 1, -1, weight, 8},
      {1, -1, 1, weight, 8},
      {1, -1, -1, weight, 8},
  };
}

std::vector<Direction> dirs_g18() {
  std::vector<Direction> dirs = {
      {1, 0, 0, 1.0 / 3.0, 6},
      {0, 1, 0, 1.0 / 3.0, 6},
      {0, 0, 1, 1.0 / 3.0, 6},
  };
  auto edges = edge_positive_half(1.0 / 6.0);
  dirs.insert(dirs.end(), edges.begin(), edges.end());
  return dirs;
}

std::vector<Direction> dirs_g26_equal_layer() {
  std::vector<Direction> dirs = {
      {1, 0, 0, 1.0 / 3.0, 6},
      {0, 1, 0, 1.0 / 3.0, 6},
      {0, 0, 1, 1.0 / 3.0, 6},
  };
  auto edges = edge_positive_half(1.0 / 12.0);
  auto corners = corner_positive_half(1.0 / 12.0);
  dirs.insert(dirs.end(), edges.begin(), edges.end());
  dirs.insert(dirs.end(), corners.begin(), corners.end());
  return dirs;
}

std::vector<Direction> dirs_g26_isotropic(double corner_weight) {
  const double c = corner_weight;
  const double a = 1.0 / 3.0 + 4.0 * c;
  const double b = 1.0 / 6.0 - 2.0 * c;
  std::vector<Direction> dirs = {
      {1, 0, 0, a, 6},
      {0, 1, 0, a, 6},
      {0, 0, 1, a, 6},
  };
  auto edges = edge_positive_half(b);
  auto corners = corner_positive_half(c);
  dirs.insert(dirs.end(), edges.begin(), edges.end());
  dirs.insert(dirs.end(), corners.begin(), corners.end());
  return dirs;
}

int shifted_index(const ftd::Lattice& lattice, int idx, const Direction& d) {
  const auto c = lattice.coord(idx);
  return lattice.index(c.x + d.dx, c.y + d.dy, c.z + d.dz);
}

int shifted_index_negative(const ftd::Lattice& lattice, int idx, const Direction& d) {
  const auto c = lattice.coord(idx);
  return lattice.index(c.x - d.dx, c.y - d.dy, c.z - d.dz);
}

double dir_norm2(const Direction& d) {
  return static_cast<double>(d.dx * d.dx + d.dy * d.dy + d.dz * d.dz);
}

double dot_dir(const ftd::Vec3& v, const Direction& d) {
  return v.x * static_cast<double>(d.dx) +
         v.y * static_cast<double>(d.dy) +
         v.z * static_cast<double>(d.dz);
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

ShellFlux shell_flux_from_cell_flux(const std::vector<ftd::Voxel>& voxels,
                                    const ftd::Lattice& lattice,
                                    const std::vector<Direction>& dirs) {
  const int n = static_cast<int>(lattice.total_sites());
  ShellFlux out;
  out.dirs = dirs;
  out.link.resize(dirs.size(), std::vector<double>(static_cast<size_t>(n), 0.0));

  for (size_t a = 0; a < dirs.size(); ++a) {
    const auto& d = dirs[a];
    const double norm2 = dir_norm2(d);
    for (int i = 0; i < n; ++i) {
      const int ip = shifted_index(lattice, i, d);
      const ftd::Vec3 avg = (voxels[i].flux + voxels[ip].flux) * 0.5;
      out.link[a][i] = dot_dir(avg, d) / norm2;
    }
  }
  return out;
}

double div_shell_at(const ShellFlux& flux, const ftd::Lattice& lattice, int idx) {
  double div = 0.0;
  for (size_t a = 0; a < flux.dirs.size(); ++a) {
    const auto& d = flux.dirs[a];
    const int im = shifted_index_negative(lattice, idx, d);
    div += d.weight * (flux.link[a][idx] - flux.link[a][im]);
  }
  return div;
}

double shell_lap_at(const std::vector<double>& phi,
                    const std::vector<Direction>& dirs,
                    const ftd::Lattice& lattice,
                    int idx) {
  double lap = 0.0;
  for (const auto& d : dirs) {
    const int ip = shifted_index(lattice, idx, d);
    const int im = shifted_index_negative(lattice, idx, d);
    lap += d.weight * (phi[ip] + phi[im] - 2.0 * phi[idx]);
  }
  return lap;
}

void subtract_mean(std::vector<double>& phi) {
  const double sum = std::accumulate(phi.begin(), phi.end(), 0.0);
  const double mean = sum / static_cast<double>(phi.size());
  for (double& x : phi) x -= mean;
}

std::vector<double> solve_shell_phi(const std::vector<double>& source,
                                    const std::vector<Direction>& dirs,
                                    const ftd::Lattice& lattice,
                                    int sor_iters,
                                    double omega) {
  const int n = static_cast<int>(lattice.total_sites());
  std::vector<double> phi(static_cast<size_t>(n), 0.0);
  double weight_sum = 0.0;
  for (const auto& d : dirs) weight_sum += d.weight;
  const double diagonal = 2.0 * weight_sum;

  for (int iter = 0; iter < sor_iters; ++iter) {
    for (int parity = 0; parity < 2; ++parity) {
      for (int i = 0; i < n; ++i) {
        const auto c = lattice.coord(i);
        if (((c.x + c.y + c.z) & 1) != parity) continue;

        double neighbor_sum = 0.0;
        for (const auto& d : dirs) {
          const int ip = shifted_index(lattice, i, d);
          const int im = shifted_index_negative(lattice, i, d);
          neighbor_sum += d.weight * (phi[ip] + phi[im]);
        }

        const double next_phi = (neighbor_sum - source[i]) / diagonal;
        phi[i] = (1.0 - omega) * phi[i] + omega * next_phi;
      }
    }
    if ((iter + 1) % 50 == 0) subtract_mean(phi);
  }
  subtract_mean(phi);
  return phi;
}

void apply_shell_projection(ShellFlux& flux,
                            const std::vector<double>& phi,
                            const ftd::Lattice& lattice) {
  const int n = static_cast<int>(lattice.total_sites());
  for (size_t a = 0; a < flux.dirs.size(); ++a) {
    const auto& d = flux.dirs[a];
    for (int i = 0; i < n; ++i) {
      const int ip = shifted_index(lattice, i, d);
      flux.link[a][i] -= phi[ip] - phi[i];
    }
  }
}

double shell_flux_delta_norm(const ShellFlux& flux,
                             const ShellFlux& before,
                             int shell_filter) {
  double sum_sq = 0.0;
  for (size_t a = 0; a < flux.dirs.size(); ++a) {
    if (shell_filter != 0 && flux.dirs[a].shell != shell_filter) continue;
    for (size_t i = 0; i < flux.link[a].size(); ++i) {
      const double d = flux.link[a][i] - before.link[a][i];
      sum_sq += d * d;
    }
  }
  return std::sqrt(sum_sq);
}

Metrics measure_shell_gauss(const ShellFlux& flux,
                            const std::vector<ftd::Voxel>& voxels,
                            const ftd::Lattice& lattice,
                            const ShellFlux& before) {
  Metrics m;
  const int n = static_cast<int>(lattice.total_sites());
  double sum_all = 0.0;
  double sum_void = 0.0;
  double sum_particle = 0.0;
  int void_count = 0;
  int particle_count = 0;

  for (int i = 0; i < n; ++i) {
    const int s = voxels[i].state;
    const double err = div_shell_at(flux, lattice, i) - static_cast<double>(s);
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
  m.flux_delta = shell_flux_delta_norm(flux, before, 0);
  m.face_delta = shell_flux_delta_norm(flux, before, 6);
  m.edge_delta = shell_flux_delta_norm(flux, before, 12);
  m.corner_delta = shell_flux_delta_norm(flux, before, 8);
  return m;
}

double sigma_shell(const std::vector<Direction>& dirs,
                   double kx,
                   double ky,
                   double kz) {
  double sigma = 0.0;
  for (const auto& d : dirs) {
    const double kd = kx * d.dx + ky * d.dy + kz * d.dz;
    sigma += 2.0 * d.weight * (1.0 - std::cos(kd));
  }
  return sigma;
}

void print_symbol(const std::string& name,
                  const std::vector<Direction>& dirs,
                  double q) {
  const double inv_sqrt2 = 1.0 / std::sqrt(2.0);
  const double inv_sqrt3 = 1.0 / std::sqrt(3.0);
  const double s_axis = sigma_shell(dirs, q, 0.0, 0.0) / (q * q);
  const double s_face = sigma_shell(dirs, q * inv_sqrt2, q * inv_sqrt2, 0.0) /
                        (q * q);
  const double s_body = sigma_shell(dirs,
                                    q * inv_sqrt3,
                                    q * inv_sqrt3,
                                    q * inv_sqrt3) / (q * q);
  std::cout << "    " << name
            << " symbol(q=" << q << "): axis=" << s_axis
            << " face_diag=" << s_face
            << " body_diag=" << s_body
            << "\n";
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
            << " face_delta=" << m.face_delta
            << " edge_delta=" << m.edge_delta
            << " corner_delta=" << m.corner_delta
            << " residual_sum=" << m.residual_sum
            << "\n";
}

void run_operator(const std::string& name,
                  const std::vector<Direction>& dirs,
                  const std::vector<ftd::Voxel>& voxels,
                  const ftd::Lattice& lattice) {
  const int n = static_cast<int>(lattice.total_sites());
  const int sor_iters = 2500;
  const double omega = 1.80;

  const auto base_flux = shell_flux_from_cell_flux(voxels, lattice, dirs);
  std::vector<double> source(static_cast<size_t>(n), 0.0);
  for (int i = 0; i < n; ++i) {
    source[i] = div_shell_at(base_flux, lattice, i) -
                static_cast<double>(voxels[i].state);
  }

  const double source_sum = std::accumulate(source.begin(), source.end(), 0.0);
  const auto phi = solve_shell_phi(source, dirs, lattice, sor_iters, omega);

  double lap_residual_max = 0.0;
  double lap_residual_rms = 0.0;
  for (int i = 0; i < n; ++i) {
    const double r = shell_lap_at(phi, dirs, lattice, i) - source[i];
    lap_residual_max = std::max(lap_residual_max, std::abs(r));
    lap_residual_rms += r * r;
  }
  lap_residual_rms = std::sqrt(lap_residual_rms / static_cast<double>(n));

  auto projected_flux = base_flux;
  apply_shell_projection(projected_flux, phi, lattice);

  const auto base = measure_shell_gauss(base_flux, voxels, lattice, base_flux);
  const auto projected =
      measure_shell_gauss(projected_flux, voxels, lattice, base_flux);

  std::cout << "\n-- NMG-" << name << ": fixed Moore-shell operator --\n";
  std::cout << "    dirs=" << dirs.size()
            << " source_sum=" << source_sum
            << " lap_residual_rms=" << lap_residual_rms
            << " lap_residual_max=" << lap_residual_max
            << " sor_iters=" << sor_iters
            << " omega=" << omega << "\n";
  print_symbol(name, dirs, 1.0e-3);
  print_symbol(name, dirs, 2.0 * 3.14159265358979323846 / 16.0);
  print_metrics("base_shell_flux", base);
  print_metrics("projected_shell_flux", projected);

  check("NMG-" + name + "a: neutral source is compatible with periodic solve",
        std::abs(source_sum) < 1e-12);
  check("NMG-" + name + "b: shell Poisson solve converged",
        lap_residual_rms < 1e-10 && lap_residual_max < 1e-8);
  check("NMG-" + name + "c: shell projection changes boundary flux",
        projected.flux_delta > 1e-6);
  check("NMG-" + name + "d: shell projection improves source-cell residual",
        projected.rms_particle < base.rms_particle);
  check("NMG-" + name + "e: shell residual is near zero on all cells",
        projected.rms_all < 1e-10 && projected.max_all < 1e-8);
  check("NMG-" + name + "f: small-k symbol is normalized",
        std::abs(sigma_shell(dirs, 1.0e-3, 0.0, 0.0) / 1.0e-6 - 1.0) < 1e-6);

  if (name.rfind("G26", 0) == 0) {
    check("NMG-" + name + "g: BCC/corner channel participates",
          projected.corner_delta > 1e-6);
  }
}

}  // namespace

int main() {
  std::cout << "================================================================\n";
  std::cout << "  TEST: Native Moore-Shell Gauss\n";
  std::cout << "================================================================\n";

  ftd::RenderBridge rb_for_lattice(16);
  const auto& lattice = rb_for_lattice.lattice();
  const auto voxels = seeded_pair(lattice);

  std::cout << "\n-- Fixed operators --\n";
  std::cout << "    G6:  3 positive face directions, weight 1\n";
  std::cout << "    G18: face weight 1/3, edge weight 1/6\n";
  std::cout << "    G26_equal_layer: face 1/3, edge 1/12, corner 1/12\n";
  std::cout << "         equal-layer Moore normalization; not a fitted value.\n";
  std::cout << "    G26_iso_mid: face 1/2, edge 1/12, corner 1/24\n";
  std::cout << "         midpoint of the fourth-order isotropic Moore family.\n";
  std::cout << "    G26_iso_corner: face 2/3, edge 0, corner 1/12\n";
  std::cout << "         BCC endpoint of the fourth-order isotropic Moore family.\n";

  run_operator("G6", dirs_g6(), voxels, lattice);
  run_operator("G18", dirs_g18(), voxels, lattice);
  run_operator("G26_equal_layer", dirs_g26_equal_layer(), voxels, lattice);
  run_operator("G26_iso_mid", dirs_g26_isotropic(1.0 / 24.0), voxels, lattice);
  run_operator("G26_iso_corner", dirs_g26_isotropic(1.0 / 12.0), voxels, lattice);

  std::cout << "\n================================================================\n";
  if (g_failures == 0) {
    std::cout << "  Native Moore-shell Gauss audit PASSED.\n";
  } else {
    std::cout << "  " << g_failures << " Moore-shell Gauss check(s) FAILED.\n";
  }
  std::cout << "================================================================\n";

  return g_failures;
}
