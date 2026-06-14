/**
 * Campaign: Wave Sectors (FTD-0299)  [hardened v2 after adversarial pre-reg review]
 *
 * Arm 1 (--arm=light): light-sector dispersion atlas. Injects a scalar flux
 *   standing plane wave along <100>/<110>/<111>, measures omega(k) two ways
 *   (single-tick operator eigenvalue + time-FFT), and ALSO dumps the exact 18-pt
 *   stencil eigenvalue omega_theory per row so the analyzer compares engine vs its
 *   OWN operator symbol (the axial law 2c|sin(k/2)| is only correct on <100>).
 *   With gauss OFF the bare wave operator is a scalar Laplacian per component, so
 *   the polarization is just a non-node probe (dispersion is polarization-free).
 *
 * Arm 2 (--arm=sound): the FTD-0298-SOUND condensate-compression probe. Prepares
 *   a uniform manifested condensate (Langevin above T_up), enables state<->flux
 *   COUPLING (the only channel a collective compression mode can propagate in),
 *   imposes a small longitudinal compression kick, and records THREE per-x density
 *   Fourier modes each tick: energy 1/2|J|^2 (continuous, primary), |J| (continuous),
 *   and the conserved state density Sigma_s (cross-check). A kick=0 CONTROL arm
 *   (same equilibrated condensate, no kick) calibrates the background/breathing
 *   spectrum so a relaxation transient cannot masquerade as a propagating mode.
 *
 * Read-only instrument (campaign_*.cpp, no engine-source change) => golden-neutral.
 * Pre-registration: PREREG_WAVE_SECTORS_v1.md (FTD-0299).
 * Canonical: force_cpu() + OMP_NUM_THREADS=1 for bit-exact reproducibility.
 */
#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/render_bridge_diagnostics.h"
#include "ftd/spectral.h"
#include "ftd/voxel.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

struct Dir {
  const char* name;
  int D[3];
  double e[3];   // transverse non-node probe polarization
  double dmag;   // |D| so |k| = q*|D|
};

const Dir DIRS[3] = {
  { "100", {1, 0, 0}, {0.0, 0.0, 1.0},                                 1.0 },
  { "110", {1, 1, 0}, {0.0, 0.0, 1.0},                                 1.4142135623730951 },
  { "111", {1, 1, 1}, {0.70710678118654752, -0.70710678118654752, 0.0}, 1.7320508075688772 },
};

// Exact 18-pt isotropic-Laplacian eigenvalue -> omega for a plane wave with
// per-component wavenumbers (kx,ky,kz). lambda = (2/3)Sum cos ki
// + (2/3)Sum cos ki cos kj - 4 ; omega = sqrt(-c^2 lambda), c^2 = 1/3.
double omega_theory_18pt(double kx, double ky, double kz) {
  const double cx = std::cos(kx), cy = std::cos(ky), cz = std::cos(kz);
  const double lam = (2.0 / 3.0) * (cx + cy + cz)
                   + (2.0 / 3.0) * (cx * cy + cy * cz + cx * cz) - 4.0;
  const double c2 = ftd::C_WAVE * ftd::C_WAVE;
  return std::sqrt(std::max(0.0, -c2 * lam));
}

void seed_standing_wave(ftd::RenderBridge& rb, int L, const Dir& d, double q, double A) {
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const double ph = q * (d.D[0] * x + d.D[1] * y + d.D[2] * z);
        const double s = std::sin(ph);
        rb.inject_flux(x, y, z, {A * d.e[0] * s, A * d.e[1] * s, A * d.e[2] * s});
      }
}

// ---------------------------------------------------------------------------
// Arm 1 — light-sector dispersion atlas
// ---------------------------------------------------------------------------
void run_light_arm(int L, int ticks, int max_mode, const fs::path& out_dir) {
  fs::create_directories(out_dir);
  const fs::path csv = out_dir / ("wave_sectors_light_L" + std::to_string(L) + ".csv");
  std::FILE* f = std::fopen(csv.string().c_str(), "w");
  if (!f) { std::fprintf(stderr, "cannot open %s\n", csv.string().c_str()); return; }
  std::fprintf(f, "direction,n,kmag,omega_eig,omega_fft,omega_theory,c_eff\n");

  const double AMP = 0.1;
  if (max_mode <= 0) max_mode = L / 4;
  int Nfft = 1;
  while (Nfft < ticks) Nfft <<= 1;

  for (const Dir& d : DIRS) {
    for (int n = 1; n <= max_mode; ++n) {
      const double q = 2.0 * ftd::PI * n / L;
      const double kmag = q * d.dmag;
      const double omega_th =
          omega_theory_18pt(q * d.D[0], q * d.D[1], q * d.D[2]);

      // (a) single-tick operator eigenvalue
      double omega_eig = 0.0;
      {
        ftd::RenderBridge rb(L);
        rb.force_cpu();
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;
        rb.seed_rng(42);
        seed_standing_wave(rb, L, d, q, AMP);
        const int p = rb.lattice().index(1, 0, 0);
        const ftd::Vec3 Jb = rb.flux_at(p);
        const double J_before = Jb.x * d.e[0] + Jb.y * d.e[1] + Jb.z * d.e[2];
        rb.tick();
        const ftd::Vec3 Wa = rb.wave_vel_at(p);
        const double wv_after = Wa.x * d.e[0] + Wa.y * d.e[1] + Wa.z * d.e[2];
        omega_eig = (std::abs(J_before) > 1e-15)
                        ? std::sqrt(std::abs(wv_after / J_before)) : 0.0;
      }

      // (b) leapfrog time-FFT (related by sin(omega_fft/2)=omega_eig/2)
      double omega_fft = 0.0;
      {
        ftd::RenderBridge rb(L);
        rb.force_cpu();
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;
        rb.seed_rng(42);
        seed_standing_wave(rb, L, d, q, AMP);
        const int p = rb.lattice().index(1, 0, 0);
        std::vector<double> series;
        series.reserve(ticks);
        double mean = 0.0;
        for (int t = 0; t < ticks; ++t) {
          const ftd::Vec3 J = rb.flux_at(p);
          const double v = J.x * d.e[0] + J.y * d.e[1] + J.z * d.e[2];
          series.push_back(v);
          mean += v;
          rb.tick();
        }
        mean /= ticks;
        for (double& v : series) v -= mean;  // remove DC before PSD
        const auto psd = ftd::power_spectrum(series);
        int peak = 1;
        double mx = 0.0;
        for (int i = 1; i < static_cast<int>(psd.size()); ++i)
          if (psd[i] > mx) { mx = psd[i]; peak = i; }
        omega_fft = 2.0 * ftd::PI * peak / Nfft;
      }

      const double c_eff = (kmag > 1e-12) ? omega_eig / kmag : 0.0;
      std::fprintf(f, "%s,%d,%.10f,%.10f,%.10f,%.10f,%.10f\n",
                   d.name, n, kmag, omega_eig, omega_fft, omega_th, c_eff);
    }
  }
  std::fclose(f);
  std::printf("[light] L=%d modes=1..%d ticks=%d -> %s\n",
              L, max_mode, ticks, csv.string().c_str());
}

// ---------------------------------------------------------------------------
// Arm 2 helpers
// ---------------------------------------------------------------------------
void base_condensate_toggles(ftd::RenderBridge& rb, double T_cond) {
  rb.force_cpu();
  rb.set_sor_iterations(150);
  rb.toggles.disable_all();
  rb.toggles.wave_propagation = true;
  rb.toggles.gauss_projection = true;
  rb.toggles.genesis = true;
  rb.toggles.coupling = true;        // M9: the s<->J channel a compression mode needs
  rb.toggles.dual_substrate = false; // REQUIRED for Langevin
  rb.toggles.langevin = true;
  rb.toggles.langevin_gamma = 0.02;
  rb.toggles.langevin_T = T_cond;
}

// Record THREE per-x density Fourier modes each tick for the listed modes.
void record_modes(ftd::RenderBridge& rb, int L, const std::vector<int>& modes,
                  int ticks, const char* arm, std::uint32_t seed, std::FILE* f) {
  const double Nvox = static_cast<double>(L) * L * L;
  std::vector<double> ex(L), rx(L), jx(L);
  for (int t = 0; t < ticks; ++t) {
    for (int x = 0; x < L; ++x) {
      double e = 0.0, r = 0.0, j = 0.0;
      for (int y = 0; y < L; ++y)
        for (int z = 0; z < L; ++z) {
          const int idx = rb.lattice().index(x, y, z);
          const ftd::Vec3 J = rb.flux_at(idx);
          const double m2 = J.mag2();
          e += 0.5 * m2;
          j += std::sqrt(m2);
          r += static_cast<double>(rb.state_at(idx));
        }
      ex[x] = e; rx[x] = r; jx[x] = j;
    }
    const double m = rb.energy_audit().manifested_count / Nvox;
    for (int n : modes) {
      const double k = 2.0 * ftd::PI * n / L;
      double ere = 0, eim = 0, rre = 0, rim = 0, jre = 0, jim = 0;
      for (int x = 0; x < L; ++x) {
        const double c = std::cos(k * x), s = std::sin(k * x);
        ere += ex[x] * c; eim -= ex[x] * s;
        rre += rx[x] * c; rim -= rx[x] * s;
        jre += jx[x] * c; jim -= jx[x] * s;
      }
      std::fprintf(f, "%u,%s,%d,%.10f,%d,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f\n",
                   seed, arm, n, k, t, ere, eim, rre, rim, jre, jim, m);
    }
    rb.tick();
  }
}

// ---------------------------------------------------------------------------
// Arm 2 — condensate compression probe (FTD-0298-SOUND)
// ---------------------------------------------------------------------------
void run_sound_arm(int L, int ticks, int seeds, double T_cond, int equil,
                   double kick, int nmodes, const fs::path& out_dir) {
  fs::create_directories(out_dir);
  const fs::path csv = out_dir / ("wave_sectors_sound_L" + std::to_string(L) + ".csv");
  std::FILE* f = std::fopen(csv.string().c_str(), "w");
  if (!f) { std::fprintf(stderr, "cannot open %s\n", csv.string().c_str()); return; }
  std::fprintf(f, "seed,arm,n,k,tick,e_re,e_im,rho_re,rho_im,j_re,j_im,m\n");

  const double Nvox = static_cast<double>(L) * L * L;
  const std::uint32_t seed_base = 0x73E12000u;
  std::vector<int> modes;
  for (int n = 1; n <= nmodes; ++n) modes.push_back(n);

  for (int s = 0; s < seeds; ++s) {
    const std::uint32_t seed = seed_base + static_cast<std::uint32_t>(s) * 2654435761u;

    // CONTROL: equilibrate, no kick, record ALL modes (the background/breathing null)
    {
      ftd::RenderBridge rb(L);
      base_condensate_toggles(rb, T_cond);
      rb.seed_rng(seed);
      for (int t = 0; t < equil; ++t) rb.tick();
      const double m0 = rb.energy_audit().manifested_count / Nvox;
      rb.toggles.langevin = false;  // microcanonical, no kick
      record_modes(rb, L, modes, ticks, "ctrl", seed, f);
      std::printf("[sound] seed=%u ctrl m0=%.3f\n", seed, m0);
    }

    // KICK: per mode, equilibrate (same seed => same condensate), kick at k_n, record
    for (int n : modes) {
      ftd::RenderBridge rb(L);
      base_condensate_toggles(rb, T_cond);
      rb.seed_rng(seed);
      for (int t = 0; t < equil; ++t) rb.tick();
      const double k = 2.0 * ftd::PI * n / L;
      for (int x = 0; x < L; ++x) {
        const double dv = kick * std::sin(k * x);
        for (int y = 0; y < L; ++y)
          for (int z = 0; z < L; ++z)
            rb.inject_wave_vel_add(x, y, z, {dv, 0.0, 0.0});
      }
      rb.toggles.langevin = false;  // microcanonical after the kick
      std::vector<int> one{n};
      record_modes(rb, L, one, ticks, "kick", seed, f);
    }
    std::printf("[sound] seed=%u done (%d kick modes)\n", seed, (int)modes.size());
  }
  std::fclose(f);
  std::printf("[sound] L=%d seeds=%d ticks=%d kick=%.3f Tcond=%.3f -> %s\n",
              L, seeds, ticks, kick, T_cond, csv.string().c_str());
}

}  // namespace

int main(int argc, char** argv) {
  std::string arm = "light";
  int L = 32, ticks = 256, seeds = 5, equil = 600, max_mode = 0, nmodes = 6;
  double T_cond = 0.5, kick = 0.05;
  std::string out = "engine/results/wave_sectors";

  for (int i = 1; i < argc; ++i) {
    const std::string a = argv[i];
    if (a.rfind("--arm=", 0) == 0) arm = a.substr(6);
    else if (a.rfind("--L=", 0) == 0) L = std::atoi(a.c_str() + 4);
    else if (a.rfind("--ticks=", 0) == 0) ticks = std::atoi(a.c_str() + 8);
    else if (a.rfind("--seeds=", 0) == 0) seeds = std::atoi(a.c_str() + 8);
    else if (a.rfind("--equil=", 0) == 0) equil = std::atoi(a.c_str() + 8);
    else if (a.rfind("--modes=", 0) == 0) max_mode = std::atoi(a.c_str() + 8);
    else if (a.rfind("--nmodes=", 0) == 0) nmodes = std::atoi(a.c_str() + 9);
    else if (a.rfind("--Tcond=", 0) == 0) T_cond = std::atof(a.c_str() + 8);
    else if (a.rfind("--kick=", 0) == 0) kick = std::atof(a.c_str() + 7);
    else if (a.rfind("--out=", 0) == 0) out = a.substr(6);
  }

  const fs::path out_dir(out);
  std::printf("campaign_wave_sectors v2: arm=%s L=%d ticks=%d\n", arm.c_str(), L, ticks);

  if (arm == "light") {
    run_light_arm(L, ticks, max_mode, out_dir);
  } else if (arm == "sound") {
    run_sound_arm(L, ticks, seeds, T_cond, equil, kick, nmodes, out_dir);
  } else {
    std::fprintf(stderr, "unknown --arm=%s (use light|sound)\n", arm.c_str());
    return 2;
  }
  return 0;
}
