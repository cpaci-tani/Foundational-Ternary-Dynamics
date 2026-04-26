/**
 * @file campaign_flux_slice_propagation.cpp
 * @brief 2D flux-slice diagnostic for wave-propagation isotropy.
 *
 * Complementary real-space view of FTD-0092 (Lorentz isotropy / Pillar 3,
 * closed PASS via spectral measurement, δ ∝ k⁴ with R² = 1.000000).
 *
 * Method
 * ------
 * Seed a controlled centred Gaussian flux pulse with J = (φ(r), 0, 0)
 * (every site has flux pointing along +x with magnitude depending only on
 * radial distance from the lattice centre). Propagate under wave-only
 * dynamics (`wave_propagation` + `gauss_projection` ON, everything else
 * OFF). At checkpoints, extract the three central slices:
 *
 *     xy  (z = L/2)    yz  (x = L/2)    xz  (y = L/2)
 *
 * Wave-only dynamics on the 18-pt Moore Laplacian preserves rotational
 * symmetry to O(h²) — at low k·h the wavefront should be radially
 * symmetric in every plane. The two cross-axis planes (xy, xz, yz) of
 * an axially-aligned seed exercise the full 3D stencil; if any two of
 * the three planes disagree, the lattice is not Cauchy-isotropic in
 * real space at that resolution.
 *
 * Output
 * ------
 *   stdout: summary CSV (the per-checkpoint scalars).
 *     plane,tick,wavefront_radius,anisotropy_ratio,plane_energy
 *
 *   slice_<plane>_t<tick>.csv (in cwd): full 2D |J|-magnitude grids,
 *     comma-separated rows. The harness writes them to whatever the
 *     working directory is; the runner script cd's into
 *     engine/results/flux_slices_2026-04-26/ before invoking.
 *
 *   stderr: human-readable progress.
 *
 * Reproducibility
 * ---------------
 * Initial condition is a deterministic Gaussian — no RNG. Wave-only
 * dynamics is deterministic. Same hardware → bit-for-bit identical.
 *
 * Epistemic status
 * ----------------
 *   [DIAGNOSTIC] — no claim is being defended. This produces a
 *   real-space companion view to the Pillar-3 spectral measurement.
 */

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "ftd/render_bridge.h"

namespace {

// ----- seed + run parameters ------------------------------------------------

constexpr int    L_LATTICE   = 48;       // enough room for r ~ L/4 propagation
constexpr double SIGMA       = 3.0;      // Gaussian width (lattice units)
constexpr double AMPLITUDE   = 1.0;
constexpr int    N_TICKS     = 24;       // c_lat = 1/√3 → wavefront travels ~14 voxels
constexpr int    N_CHECKPOINTS = 4;

// ----- helpers --------------------------------------------------------------

// Seed: scalar Gaussian on J_x. Same form as test_moore_laplacian_isotropy.cpp.
void seed_scalar_gaussian(ftd::RenderBridge& rb, double sigma, double amplitude) {
    int N = rb.lattice().size();
    int c = N / 2;
    double cutoff = 4.0 * sigma;
    for (int k = 0; k < N; ++k) {
        for (int j = 0; j < N; ++j) {
            for (int i = 0; i < N; ++i) {
                double dx = i - c, dy = j - c, dz = k - c;
                double r2 = dx*dx + dy*dy + dz*dz;
                if (r2 > cutoff * cutoff) continue;
                double amp = amplitude * std::exp(-0.5 * r2 / (sigma * sigma));
                rb.inject_flux(i, j, k, ftd::Vec3{amp, 0.0, 0.0});
            }
        }
    }
}

enum class Plane { XY, XZ, YZ };

const char* plane_name(Plane p) {
    switch (p) {
        case Plane::XY: return "xy";
        case Plane::XZ: return "xz";
        case Plane::YZ: return "yz";
    }
    return "?";
}

// Extract a 2D |J| slice through the lattice centre.
// XY → fix z = L/2, vary i (x) and j (y); slice[j*N+i] = |J(i,j,L/2)|
// XZ → fix y = L/2, vary i (x) and k (z); slice[k*N+i] = |J(i,L/2,k)|
// YZ → fix x = L/2, vary j (y) and k (z); slice[k*N+j] = |J(L/2,j,k)|
std::vector<double> extract_slice(const ftd::RenderBridge& rb, Plane p) {
    int N = rb.lattice().size();
    int c = N / 2;
    std::vector<double> grid(N * N, 0.0);
    const auto& vox = rb.voxels();
    for (int b = 0; b < N; ++b) {
        for (int a = 0; a < N; ++a) {
            int idx;
            switch (p) {
                case Plane::XY: idx = rb.lattice().index(a, b, c); break;
                case Plane::XZ: idx = rb.lattice().index(a, c, b); break;
                case Plane::YZ: idx = rb.lattice().index(c, a, b); break;
            }
            grid[b * N + a] = vox[idx].flux.mag();
        }
    }
    return grid;
}

void write_slice_csv(const std::string& filename,
                     const std::vector<double>& grid, int N) {
    std::ofstream out(filename);
    if (!out) {
        std::cerr << "  [WARN] could not open " << filename << " for writing\n";
        return;
    }
    out.setf(std::ios::scientific);
    out.precision(6);
    for (int row = 0; row < N; ++row) {
        for (int col = 0; col < N; ++col) {
            if (col > 0) out << ',';
            out << grid[row * N + col];
        }
        out << '\n';
    }
}

// Wavefront radius: distance from centre to the row/col-summed radial
// histogram peak of |J|. Bin step = 1 voxel.
struct PlaneDiagnostic {
    double wavefront_radius;
    double anisotropy_ratio;
    double plane_energy;
};

PlaneDiagnostic compute_plane_diagnostic(const std::vector<double>& grid, int N) {
    int c = N / 2;
    // 1) Radial histogram: bin |J|² by integer radius, take peak bin.
    int n_bins = N;  // generous
    std::vector<double> bin_sum(n_bins, 0.0);
    std::vector<int>    bin_cnt(n_bins, 0);
    for (int b = 0; b < N; ++b) {
        for (int a = 0; a < N; ++a) {
            double da = a - c, db = b - c;
            double r = std::sqrt(da*da + db*db);
            int ir = (int)std::round(r);
            if (ir < 0 || ir >= n_bins) continue;
            bin_sum[ir] += grid[b * N + a];
            bin_cnt[ir] += 1;
        }
    }
    int peak_bin = 0;
    double peak_val = 0.0;
    // Skip bin 0 (centre singularity from initial pulse can dominate at t=0).
    for (int r = 1; r < n_bins; ++r) {
        if (bin_cnt[r] == 0) continue;
        double avg = bin_sum[r] / bin_cnt[r];
        if (avg > peak_val) { peak_val = avg; peak_bin = r; }
    }

    // 2) Anisotropy: along a circle of radius peak_bin, sample 16 angular
    //    points, take max/min ratio of |J|. peak_bin guards against the
    //    flat low-amplitude tail.
    double aniso = 1.0;
    if (peak_bin >= 2) {
        const int n_angles = 16;
        double amax = 0.0, amin = std::numeric_limits<double>::max();
        for (int q = 0; q < n_angles; ++q) {
            constexpr double TWO_PI = 6.283185307179586476925286766559;
            double theta = TWO_PI * q / n_angles;
            double xa = c + peak_bin * std::cos(theta);
            double xb = c + peak_bin * std::sin(theta);
            int ia = (int)std::round(xa);
            int ib = (int)std::round(xb);
            if (ia < 0 || ia >= N || ib < 0 || ib >= N) continue;
            double v = grid[ib * N + ia];
            if (v > amax) amax = v;
            if (v < amin && v > 0.0) amin = v;
        }
        if (amin > 0.0 && std::isfinite(amin)) aniso = amax / amin;
        else aniso = 0.0;  // degenerate
    }

    // 3) In-plane energy: Σ |J|² over the slice.
    double energy = 0.0;
    for (double v : grid) energy += v * v;

    return PlaneDiagnostic{(double)peak_bin, aniso, energy};
}

}  // anonymous namespace

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    std::cerr << "================================================================\n";
    std::cerr << "  Flux-slice propagation diagnostic (FTD-0092 companion)\n";
    std::cerr << "================================================================\n";
    std::cerr << "  L=" << L_LATTICE << "  σ=" << SIGMA << "  amplitude=" << AMPLITUDE << "\n";
    std::cerr << "  N_ticks=" << N_TICKS << "  checkpoints=" << N_CHECKPOINTS << "\n";
    std::cerr << "  Backend: GPU (no force_cpu); single-substrate; wave-only\n\n";

    ftd::RenderBridge rb(L_LATTICE);
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.gauss_projection = true;
    // dual_substrate, langevin, all forces, all manifestation: OFF.

    std::string err;
    if (!rb.toggles.validate(&err)) {
        std::cerr << "[FAIL] toggle validation: " << err << "\n";
        return 1;
    }

    seed_scalar_gaussian(rb, SIGMA, AMPLITUDE);
    std::cerr << "  Seeded scalar Gaussian on J_x.\n";
    std::cerr << "  Backend kind = "
              << (rb.backend_kind() == ftd::Backend::Kind::Gpu ? "GPU" : "CPU")
              << "\n\n";

    // Checkpoint schedule: t = N/4, N/2, 3N/4, N
    std::vector<int> checkpoints;
    for (int q = 1; q <= N_CHECKPOINTS; ++q) {
        checkpoints.push_back((N_TICKS * q) / N_CHECKPOINTS);
    }

    // CSV header
    std::printf("plane,tick,wavefront_radius,anisotropy_ratio,plane_energy\n");

    int last_t = 0;
    const Plane planes[3] = {Plane::XY, Plane::XZ, Plane::YZ};

    for (int cp_tick : checkpoints) {
        int delta = cp_tick - last_t;
        if (delta > 0) {
            std::cerr << "  Running " << delta << " ticks (→ tick " << cp_tick << ")...\n";
            rb.run(delta);
        }
        last_t = cp_tick;

        for (Plane p : planes) {
            auto grid = extract_slice(rb, p);
            char fname[64];
            std::snprintf(fname, sizeof(fname), "slice_%s_t%03d.csv",
                          plane_name(p), cp_tick);
            write_slice_csv(fname, grid, L_LATTICE);

            auto d = compute_plane_diagnostic(grid, L_LATTICE);
            std::printf("%s,%d,%.6f,%.6e,%.6e\n",
                        plane_name(p), cp_tick, d.wavefront_radius,
                        d.anisotropy_ratio, d.plane_energy);
            std::cerr << "    " << plane_name(p)
                      << "  r_wf=" << d.wavefront_radius
                      << "  aniso=" << d.anisotropy_ratio
                      << "  E_plane=" << d.plane_energy << "\n";
        }
    }

    std::cerr << "\n  Done. Per-slice CSVs written to working directory.\n";
    std::cerr << "  Summary CSV emitted on stdout.\n";
    return 0;
}
