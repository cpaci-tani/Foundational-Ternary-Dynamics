/**
 * Native dual half-shell audit.
 *
 * This fixed audit asks whether the r^2 = 1/2 dual-edge shell has a measurable
 * role in the engine's exact dual-cell Gauss response.
 *
 * Procedure:
 *   1. Seed a neutral +/- source pair.
 *   2. Build exact finite-volume face flux and solve dual-cell Gauss.
 *   3. Reconstruct a cell-centered projected flux from adjacent face fluxes.
 *   4. Sample half-offset shells by trilinear interpolation around each source:
 *        d=1: face centers   r^2 = 1/4
 *        d=2: edge centers   r^2 = 1/2
 *        d=3: corners        r^2 = 3/4
 *
 * No external target value is used.
 */

#include "ftd/constants.h"
#include "ftd/lattice.h"
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

namespace {

static int g_failures = 0;

struct FaceFlux {
  std::vector<double> fx;
  std::vector<double> fy;
  std::vector<double> fz;
};

struct SourceCell {
  int x;
  int y;
  int z;
  int s;
};

struct ShellMetrics {
  int count = 0;
  double sum_mag = 0.0;
  double sum_energy = 0.0;
  double sum_signed_radial = 0.0;
  double max_mag = 0.0;

  double mean_mag() const {
    return count > 0 ? sum_mag / static_cast<double>(count) : 0.0;
  }

  double mean_energy() const {
    return count > 0 ? sum_energy / static_cast<double>(count) : 0.0;
  }

  double mean_signed_radial() const {
    return count > 0 ? sum_signed_radial / static_cast<double>(count) : 0.0;
  }
};

void check(const std::string& name, bool condition) {
  if (condition) {
    std::cout << "  PASS  " << name << "\n";
  } else {
    std::cout << "  FAIL  " << name << "\n";
    ++g_failures;
  }
}

int idx(const ftd::Lattice& lattice, int x, int y, int z) {
  return lattice.index(x, y, z);
}

std::vector<ftd::Voxel> seeded_pair(const ftd::Lattice& lattice,
                                    std::vector<SourceCell>& sources) {
  ftd::RenderBridge rb(lattice.size());
  rb.toggles.disable_all();
  rb.toggles.dual_substrate = true;
  const int mid = lattice.size() / 2;
  sources = {
      {mid - 2, mid, mid, +1},
      {mid + 2, mid, mid, -1},
  };
  rb.inject_particle(sources[0].x, sources[0].y, sources[0].z,
                     +1, {0, 0, ftd::K_B});
  rb.inject_particle(sources[1].x, sources[1].y, sources[1].z,
                     -1, {0, 0, -ftd::K_B});
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
    const int xp = idx(lattice, c.x + 1, c.y, c.z);
    const int yp = idx(lattice, c.x, c.y + 1, c.z);
    const int zp = idx(lattice, c.x, c.y, c.z + 1);
    f.fx[i] = 0.5 * (voxels[i].flux.x + voxels[xp].flux.x);
    f.fy[i] = 0.5 * (voxels[i].flux.y + voxels[yp].flux.y);
    f.fz[i] = 0.5 * (voxels[i].flux.z + voxels[zp].flux.z);
  }
  return f;
}

double div_face_at(const FaceFlux& f, const ftd::Lattice& lattice, int i) {
  const auto c = lattice.coord(i);
  const int xm = idx(lattice, c.x - 1, c.y, c.z);
  const int ym = idx(lattice, c.x, c.y - 1, c.z);
  const int zm = idx(lattice, c.x, c.y, c.z - 1);
  return (f.fx[i] - f.fx[xm]) +
         (f.fy[i] - f.fy[ym]) +
         (f.fz[i] - f.fz[zm]);
}

double lap6_at(const std::vector<double>& phi,
               const ftd::Lattice& lattice,
               int i) {
  const auto& n = lattice.neighbors_6(i);
  return phi[n[0]] + phi[n[1]] + phi[n[2]] +
         phi[n[3]] + phi[n[4]] + phi[n[5]] -
         6.0 * phi[i];
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
    const int xp = idx(lattice, c.x + 1, c.y, c.z);
    const int yp = idx(lattice, c.x, c.y + 1, c.z);
    const int zp = idx(lattice, c.x, c.y, c.z + 1);
    f.fx[i] -= phi[xp] - phi[i];
    f.fy[i] -= phi[yp] - phi[i];
    f.fz[i] -= phi[zp] - phi[i];
  }
}

std::vector<ftd::Vec3> cell_flux_from_face_flux(const FaceFlux& f,
                                                const ftd::Lattice& lattice) {
  const int n = static_cast<int>(lattice.total_sites());
  std::vector<ftd::Vec3> j(static_cast<size_t>(n));
  for (int i = 0; i < n; ++i) {
    const auto c = lattice.coord(i);
    const int xm = idx(lattice, c.x - 1, c.y, c.z);
    const int ym = idx(lattice, c.x, c.y - 1, c.z);
    const int zm = idx(lattice, c.x, c.y, c.z - 1);
    j[i] = {
        0.5 * (f.fx[i] + f.fx[xm]),
        0.5 * (f.fy[i] + f.fy[ym]),
        0.5 * (f.fz[i] + f.fz[zm]),
    };
  }
  return j;
}

ftd::Vec3 trilinear_half_sample(const std::vector<ftd::Vec3>& j,
                                const ftd::Lattice& lattice,
                                const SourceCell& source,
                                int ox,
                                int oy,
                                int oz) {
  std::array<int, 2> xs = {source.x, source.x};
  std::array<int, 2> ys = {source.y, source.y};
  std::array<int, 2> zs = {source.z, source.z};
  int nx = 1, ny = 1, nz = 1;

  if (ox > 0) {
    xs = {source.x, source.x + 1};
    nx = 2;
  } else if (ox < 0) {
    xs = {source.x - 1, source.x};
    nx = 2;
  }

  if (oy > 0) {
    ys = {source.y, source.y + 1};
    ny = 2;
  } else if (oy < 0) {
    ys = {source.y - 1, source.y};
    ny = 2;
  }

  if (oz > 0) {
    zs = {source.z, source.z + 1};
    nz = 2;
  } else if (oz < 0) {
    zs = {source.z - 1, source.z};
    nz = 2;
  }

  ftd::Vec3 out{};
  const double weight = 1.0 / static_cast<double>(nx * ny * nz);
  for (int ix = 0; ix < nx; ++ix) {
    for (int iy = 0; iy < ny; ++iy) {
      for (int iz = 0; iz < nz; ++iz) {
        out += j[idx(lattice, xs[ix], ys[iy], zs[iz])] * weight;
      }
    }
  }
  return out;
}

std::array<ShellMetrics, 4> measure_half_shells(
    const std::vector<ftd::Vec3>& j,
    const ftd::Lattice& lattice,
    const std::vector<SourceCell>& sources) {
  std::array<ShellMetrics, 4> shells{};

  for (const auto& source : sources) {
    for (int ox = -1; ox <= 1; ++ox) {
      for (int oy = -1; oy <= 1; ++oy) {
        for (int oz = -1; oz <= 1; ++oz) {
          const int d = (ox != 0 ? 1 : 0) + (oy != 0 ? 1 : 0) + (oz != 0 ? 1 : 0);
          if (d == 0) continue;

          const ftd::Vec3 sample =
              trilinear_half_sample(j, lattice, source, ox, oy, oz);
          const double mag = sample.mag();
          const double energy = sample.mag2();
          const double norm = 1.0 / std::sqrt(static_cast<double>(d));
          const ftd::Vec3 radial{
              static_cast<double>(ox) * norm,
              static_cast<double>(oy) * norm,
              static_cast<double>(oz) * norm,
          };
          const double signed_radial =
              static_cast<double>(source.s) *
              (sample.x * radial.x + sample.y * radial.y + sample.z * radial.z);

          auto& s = shells[static_cast<size_t>(d)];
          ++s.count;
          s.sum_mag += mag;
          s.sum_energy += energy;
          s.sum_signed_radial += signed_radial;
          s.max_mag = std::max(s.max_mag, mag);
        }
      }
    }
  }
  return shells;
}

void print_shell_table(const std::array<ShellMetrics, 4>& shells) {
  const char* names[4] = {"center", "dual face", "dual edge", "dual corner"};
  const double r2[4] = {0.0, 0.25, 0.5, 0.75};
  std::cout << "\n  shell         r^2     count   mean|J|        mean|J|^2      total|J|^2     mean s*(J.rad)\n";
  std::cout << "  ------------------------------------------------------------------------------------------\n";
  for (int d = 1; d <= 3; ++d) {
    const auto& s = shells[static_cast<size_t>(d)];
    std::cout << "  " << std::left << std::setw(12) << names[d]
              << std::right << std::setw(5) << r2[d]
              << std::setw(9) << s.count
              << "   " << std::scientific << std::setprecision(8)
              << std::setw(13) << s.mean_mag()
              << "   " << std::setw(13) << s.mean_energy()
              << "   " << std::setw(13) << s.sum_energy
              << "   " << std::setw(13) << s.mean_signed_radial()
              << "\n";
  }
}

}  // namespace

int main() {
  std::cout << "================================================================\n";
  std::cout << "  TEST: Native Dual Half-Shell\n";
  std::cout << "================================================================\n";

  ftd::RenderBridge rb_for_lattice(16);
  const auto& lattice = rb_for_lattice.lattice();
  const int n = static_cast<int>(lattice.total_sites());
  const int sor_iters = 2000;
  const double omega = 1.85;

  std::vector<SourceCell> sources;
  const auto voxels = seeded_pair(lattice, sources);
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

  const auto projected_cell_flux =
      cell_flux_from_face_flux(projected_flux, lattice);
  const auto shells = measure_half_shells(projected_cell_flux, lattice, sources);

  std::cout << "\n-- NDH-1: Dual-cell half-offset shell response --\n";
  std::cout << "    source_sum=" << source_sum
            << " lap_residual_rms=" << lap_residual_rms
            << " lap_residual_max=" << lap_residual_max
            << " sor_iters=" << sor_iters
            << " omega=" << omega << "\n";
  print_shell_table(shells);

  const double edge_r2 = 0.5;
  const double face_corner_midpoint = (0.25 + 0.75) * 0.5;
  const double edge_complement = 1.0 - edge_r2;

  const double edge_energy = shells[2].sum_energy;
  const double face_energy = shells[1].sum_energy;
  const double corner_energy = shells[3].sum_energy;
  const double total_energy = face_energy + edge_energy + corner_energy;
  const double edge_fraction =
      total_energy > 0.0 ? edge_energy / total_energy : 0.0;

  std::cout << "\n  derived:\n";
  std::cout << "    edge r^2 - midpoint(face,corner) = "
            << (edge_r2 - face_corner_midpoint) << "\n";
  std::cout << "    edge r^2 - (1-edge r^2)         = "
            << (edge_r2 - edge_complement) << "\n";
  std::cout << "    edge energy fraction             = "
            << edge_fraction << "\n";
  std::cout << "    face:edge:corner total energy    = "
            << face_energy << " : " << edge_energy << " : "
            << corner_energy << "\n";

  check("NDH-1a: neutral source is compatible with periodic solve",
        std::abs(source_sum) < 1e-12);
  check("NDH-1b: dual-cell Poisson solve converged",
        lap_residual_rms < 1e-10 && lap_residual_max < 1e-8);
  check("NDH-1c: half-shell sample counts are fixed",
        shells[1].count == 12 && shells[2].count == 24 && shells[3].count == 16);
  check("NDH-1d: dual-edge shell is exact r^2 self-complement",
        std::abs(edge_r2 - face_corner_midpoint) < 1e-15 &&
        std::abs(edge_r2 - edge_complement) < 1e-15);
  check("NDH-1e: dual-edge shell carries nonzero projected energy",
        edge_energy > 1e-12);
  check("NDH-1f: all dual half-shells carry projected energy",
        face_energy > 1e-12 && corner_energy > 1e-12);
  check("NDH-1g: dual-edge energy is dynamically resolved",
        edge_fraction > 0.05 && edge_fraction < 0.90);

  std::cout << "\n================================================================\n";
  if (g_failures == 0) {
    std::cout << "  Native dual half-shell audit PASSED.\n";
  } else {
    std::cout << "  " << g_failures << " dual half-shell check(s) FAILED.\n";
  }
  std::cout << "================================================================\n";

  return g_failures;
}
