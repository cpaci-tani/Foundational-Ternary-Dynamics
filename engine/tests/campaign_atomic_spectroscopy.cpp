/**
 * Campaign: Atomic Spectroscopy — hydrogen-1s on the engine (FTD-0281 rung 1)
 *
 * FIRST RUNG of engine-native atomic spectroscopy: does the engine's OWN
 * Coulomb-clocked flux field ring at the bound 1s frequency predicted by the
 * validated lattice operator? This is an ENGINE↔OPERATOR consistency check —
 * two independent engine code paths (the live db_clock_coulomb leapfrog, and
 * the dumped static phi_C field fed to the Python eigensolver) — NOT a claim
 * that "FTD derives hydrogen". The FTD-0270 / FC-1 quantum-dynamics ceiling and
 * the linear-dispersion caveat (FTD-0270) stand; the result class is
 * [CONDITIONAL — DERIVED-GIVEN-IMPOSED-INPUT] (the de Broglie clock ω₀ and the
 * scalar-potential coupling are [IMPOSED], FTD-0271/0281).
 *
 * METHOD.
 *   A LOCKED +1 charge at lattice center sources the engine's own Gauss/Coulomb
 *   potential phi_C via poisson_coulomb. The de_broglie_clock gives the flux a
 *   rest frequency ω₀; the EXISTING db_clock_coulomb toggle applies
 *       ω_eff²(r) = ω₀² + 2 ω₀ V(r),   V = −phi_C    (phase_read.cpp:195-197)
 *   so the leapfrog integrates J'' = −ω_eff² J and flux normal modes ring at
 *       ω_n = √(ω₀² + a_n)
 *   where a_n are the eigenvalues of the second-order operator
 *       A = −c² L18 + 2 ω₀ V        (c² = 1/3, the engine wave operator).
 *   Bound states sit BELOW ω₀ (a_n < 0). We drop a spherical Gaussian FLUX
 *   wavepacket (state stays 0 everywhere except the locked nucleus), record the
 *   shell-autocorrelation C(t) = Σ_probe J(0)·J(t) over a ball around center,
 *   FFT → peaks = levels. The ground peak (lowest non-DC) is the 1s envelope.
 *
 * THE TOGGLE STACK (the db_clock_coulomb contract; see term_toggles.h
 * TOGGLE_SPECS row + validate()): disable_all(), then wave_propagation,
 * poisson_coulomb, de_broglie_clock, db_clock_coulomb, gauss_projection ON;
 * dual_substrate=false, forces=false; omega0=1.5 (FTD-0278 Leg-1 record).
 * db_clock_coulomb is CPU-only and requires EXACTLY these deps. force_cpu() +
 * set_sor_iterations(≥40).
 *
 * GOLDEN-NEUTRAL. Read-only instrument (campaign_*.cpp, no engine-source
 * change; the db_clock_coulomb coupling already exists and is default-OFF).
 * Canonical: force_cpu() + OMP_NUM_THREADS=1 for bit-exact reproducibility.
 *
 * OUTPUT (two CSVs in --out dir):
 *   atomic_spectroscopy_Ct_L<L>.csv   : tick, corr  (the C(t) time series)
 *   atomic_spectroscopy_phiC_L<L>.csv : x, y, z, phi  (the static phi_C field)
 * plus a console FFT peak list and the engine ground ω₀ readout.
 *
 * Usage:
 *   campaign_atomic_spectroscopy --L 32 --ticks 8192 --omega0 1.5 \
 *       --sor 60 --sigma 3.0 --out engine/build_wsl/atomic_spectroscopy
 */
#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/spectral.h"
#include "ftd/voxel.h"
#ifdef FTD_ENABLE_CUDA
#include "ftd/gpu_engine.h"   // device-probe fast path (FTD-0281 rung-b)
#endif

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

struct Args {
  int L = 32;
  int ticks = 8192;
  double omega0 = 1.5;
  int sor = 60;
  double sigma = 3.0;
  // Probe ball radius for the shell-autocorrelation. Default 0 ⇒ derive from
  // sigma (3σ ball, which captures the wavepacket support and overlaps the
  // spherically symmetric 1s ground state most strongly).
  double probe_radius = 0.0;
  // Gaussian flux wavepacket peak amplitude (lattice flux units).
  double amp = 0.05;
  // Symplectic-leapfrog substep. The plain leapfrog hardcodes dt=1 and is
  // UNSTABLE for the full operator spectrum at ω₀=1.5: the high-k vacuum wave
  // modes have ω_eff² = ω₀² + c²·8 ≈ 4.92 ⇒ ω_eff·dt ≈ 2.22 > 2 (leapfrog CFL).
  // The symplectic integrator (engine's own, default-OFF ⇒ golden-neutral)
  // permits dt<1; dt=0.5 gives ω_eff_max·dt ≈ 1.11, comfortably stable, while
  // keeping the FTD-0278 Leg-1 ω₀=1.5 record. The FFT then measures Ω [rad/step]
  // related to the physical ω_eff by Ω = 2·arcsin(ω_eff·dt/2); we invert this.
  double dt = 0.5;
  std::string out = "atomic_spectroscopy";
  // Backend selection (GPU port, 2026-06-20). "cpu" forces the CPU RenderBridge
  // (the rung-(a) anchor); "gpu" uses the default CUDA backend so the 5090 runs
  // the de-Broglie-clock spectroscopy. On a non-CUDA build "gpu" silently falls
  // back to CPU (RenderBridge has no GPU backend to switch to).
  std::string backend = "cpu";
  // Device-probe fast path (GPU only): drive GpuEngine::tick() directly and
  // compute C(t) via the on-device shell-autocorrelation, avoiding the per-tick
  // full-lattice download that RenderBridge::tick() performs. Required for large
  // L (the download is 1.3 GB/tick at L=256). 1 = on (default for GPU backend),
  // 0 = off (use the RenderBridge per-tick path, identical numbers, slow).
  int device_probe = 1;
  // Packet-center offset along +x (lattice units). 0 = centered (spherically
  // symmetric, excites 1s/2s only — the 2p triplet has a node at center and is
  // not populated). A nonzero offset breaks the symmetry so the probe overlaps
  // the off-center (2p-like) bound states, testing whether the engine FFT can
  // resolve a SECOND bound line when one is actually excited (FTD-0281 rung-b).
  int offset = 0;
  // Nuclear charge Z (FTD-0281 helium extension, 2026-06-20). Z=1 = hydrogen
  // (the rung-(a)/(b) anchor); Z=2 = He+ (a 2× DEEPER Coulomb well, so the 1s
  // is more bound and the levels are MORE separated). Z scales the Gauss-law
  // source charge coupling: source = div(J) − Z·(state − mean_charge), so the
  // engine's own Coulomb potential φ_C scales linearly φ_C → Z·φ_C (well depth
  // ×Z). The mean-charge subtraction keeps the periodic Poisson problem
  // solvable (net source still sums to zero). Z=1 reproduces the default
  // physics EXACTLY (the regression gate). Wired via
  // toggles.coulomb_charge_coupling = Z (the Phase-H knob, poisson_solvers.cpp:164).
  double Z = 1.0;
};

double argd(const char* v) { return std::strtod(v, nullptr); }
int    argi(const char* v) { return static_cast<int>(std::strtol(v, nullptr, 10)); }

Args parse_args(int argc, char** argv) {
  Args a;
  for (int i = 1; i < argc; ++i) {
    auto eq = [&](const char* k) { return std::strcmp(argv[i], k) == 0; };
    if (eq("--L") && i + 1 < argc)            a.L = argi(argv[++i]);
    else if (eq("--ticks") && i + 1 < argc)   a.ticks = argi(argv[++i]);
    else if (eq("--omega0") && i + 1 < argc)  a.omega0 = argd(argv[++i]);
    else if (eq("--sor") && i + 1 < argc)     a.sor = argi(argv[++i]);
    else if (eq("--sigma") && i + 1 < argc)   a.sigma = argd(argv[++i]);
    else if (eq("--radius") && i + 1 < argc)  a.probe_radius = argd(argv[++i]);
    else if (eq("--amp") && i + 1 < argc)     a.amp = argd(argv[++i]);
    else if (eq("--dt") && i + 1 < argc)      a.dt = argd(argv[++i]);
    else if (eq("--out") && i + 1 < argc)     a.out = argv[++i];
    else if (eq("--backend") && i + 1 < argc) a.backend = argv[++i];
    else if (eq("--device-probe") && i + 1 < argc) a.device_probe = argi(argv[++i]);
    else if (eq("--offset") && i + 1 < argc)  a.offset = argi(argv[++i]);
    else if (eq("--Z") && i + 1 < argc)       a.Z = argd(argv[++i]);
    else std::fprintf(stderr, "unknown/ignored arg: %s\n", argv[i]);
  }
  return a;
}

// Configure the preregistered db_clock_coulomb single-substrate no-force
// profile. Mirrors test_db_clock_coulomb.cpp::configure_valid_profile + the
// FTD-0278 Leg-1 omega0=1.5 record.
void configure_profile(ftd::RenderBridge& rb, const Args& a) {
  // Backend: "cpu" forces the CPU RenderBridge (rung-(a) anchor); "gpu" leaves
  // the default backend (CUDA on a CUDA build) so the de-Broglie-clock KG term
  // runs on the GPU phase_read kernel (GPU port, 2026-06-20). The KG term and
  // the pre-read FFT Coulomb solve are now in both backends, so the same toggle
  // stack produces the same C(t) up to FFT-vs-SOR float precision.
  if (a.backend == "cpu") rb.force_cpu();
  rb.seed_rng(0x0281u);
  rb.set_sor_iterations(a.sor);

  rb.toggles.disable_all();
  rb.toggles.wave_propagation = true;
  rb.toggles.poisson_coulomb  = true;
  rb.toggles.de_broglie_clock = true;
  rb.toggles.db_clock_coulomb = true;
  rb.toggles.gauss_projection = true;
  rb.toggles.dual_substrate   = false;
  rb.toggles.forces           = false;
  rb.toggles.omega0           = a.omega0;
  // Nuclear charge Z (helium extension). Scales the COULOMB Poisson source
  // (coulomb_source_scale, consumed by solve_coulomb_poisson) so phi_C → Z·phi_C
  // ⇒ the db_clock_coulomb well depth is ×Z (Z=2 = He+). NOTE: this is the
  // Coulomb-solve scale, NOT coulomb_charge_coupling (which only scales the
  // Gauss flux-projection source phi_, a different buffer that never reaches the
  // KG term). Non-bulk double config field, survives disable_all(); Z=1
  // reproduces the hydrogen anchor exactly. Honored on CPU and GPU.
  rb.toggles.coulomb_source_scale = a.Z;
  // Symplectic leapfrog with dt<1 for CFL stability at ω₀=1.5 (see Args::dt).
  // set_dt() only honors dt<1 when symplectic_leapfrog is on (render_bridge.cpp).
  rb.toggles.symplectic_leapfrog = true;
  rb.set_dt(a.dt);
}

// Drop a spherical Gaussian FLUX wavepacket centered at (c,c,c). We deliberately
// use inject_flux_add (NOT inject_wavepacket, which would set a second state±1
// seed). The flux is polarized along +x with a Gaussian envelope; the operator
// A acts per-component (delta_j -= flux*omega_eff²; the wave Laplacian is also
// per-component), so a single-component drop excites the same scalar spectrum.
void seed_gaussian_flux_packet(ftd::RenderBridge& rb, int L, int c,
                               double sigma, double amp) {
  const double inv2s2 = 1.0 / (2.0 * sigma * sigma);
  // 4σ box around center covers the envelope to ~3e-4 of the peak.
  const int half = static_cast<int>(std::ceil(4.0 * sigma));
  for (int dx = -half; dx <= half; ++dx)
    for (int dy = -half; dy <= half; ++dy)
      for (int dz = -half; dz <= half; ++dz) {
        const int x = c + dx, y = c + dy, z = c + dz;
        if (x < 0 || x >= L || y < 0 || y >= L || z < 0 || z >= L) continue;
        const double r2 = double(dx * dx + dy * dy + dz * dz);
        const double w = amp * std::exp(-r2 * inv2s2);
        rb.inject_flux_add(x, y, z, ftd::Vec3{w, 0.0, 0.0});
      }
}

// Build the probe index set: a ball of radius R around center.
std::vector<int> probe_ball(const ftd::RenderBridge& rb, int L, int c, double R) {
  std::vector<int> idxs;
  const int half = static_cast<int>(std::ceil(R));
  const double R2 = R * R;
  for (int dx = -half; dx <= half; ++dx)
    for (int dy = -half; dy <= half; ++dy)
      for (int dz = -half; dz <= half; ++dz) {
        if (double(dx * dx + dy * dy + dz * dz) > R2) continue;
        const int x = c + dx, y = c + dy, z = c + dz;
        if (x < 0 || x >= L || y < 0 || y >= L || z < 0 || z >= L) continue;
        idxs.push_back(rb.lattice().index(x, y, z));
      }
  return idxs;
}

void dump_phi_csv(const ftd::RenderBridge& rb, int L, const fs::path& csv,
                  const std::vector<double>& phi) {
  std::FILE* f = std::fopen(csv.string().c_str(), "w");
  if (!f) { std::fprintf(stderr, "cannot open %s\n", csv.string().c_str()); return; }
  std::fprintf(f, "x,y,z,phi\n");
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const int idx = rb.lattice().index(x, y, z);
        std::fprintf(f, "%d,%d,%d,%.12e\n", x, y, z, phi[idx]);
      }
  std::fclose(f);
  std::printf("[phiC] dumped static Coulomb field -> %s\n", csv.string().c_str());
}

}  // namespace

int main(int argc, char** argv) {
  const Args a = parse_args(argc, argv);
  const int L = a.L;
  const int c = L / 2;
  const double R = (a.probe_radius > 0.0) ? a.probe_radius : 3.0 * a.sigma;

  fs::create_directories(a.out);
  const fs::path ct_csv =
      fs::path(a.out) / ("atomic_spectroscopy_Ct_L" + std::to_string(L) + ".csv");
  const fs::path phi_csv =
      fs::path(a.out) / ("atomic_spectroscopy_phiC_L" + std::to_string(L) + ".csv");

  std::printf("================================================================\n");
  std::printf("FTD-0281 — atomic spectroscopy (ENGINE)  Z=%.3g (%s)\n",
              a.Z, (a.Z == 1.0) ? "hydrogen-1s" : "He+/ion well");
  std::printf("L=%d  ticks=%d  omega0=%.4f  sor=%d  sigma=%.3f  probeR=%.3f  dt=%.3f  backend=%s  Z=%.3g\n",
              L, a.ticks, a.omega0, a.sor, a.sigma, R, a.dt, a.backend.c_str(), a.Z);
  std::printf("================================================================\n");

  ftd::RenderBridge rb(L);
  configure_profile(rb, a);

  std::printf("[backend] requested=%s  active=%s\n", a.backend.c_str(),
              rb.backend_kind() == ftd::Backend::Kind::Gpu ? "GPU" : "CPU");

  // Validate the toggle stack explicitly (the contract gate).
  {
    std::string err;
    const bool ok = rb.toggles.validate(&err);
    std::printf("[gate] toggle profile valid=%d %s\n", ok ? 1 : 0, err.c_str());
    if (!ok) { std::fprintf(stderr, "INVALID toggle profile — abort\n"); return 2; }
  }

  // LOCKED +1 nucleus at center (no flux of its own). inject_particle sets
  // state=+1; with forces=false the charge stays fixed ⇒ phi_C is static.
  rb.inject_particle(c, c, c, +1, ftd::Vec3{0.0, 0.0, 0.0});

  // Spherical Gaussian FLUX wavepacket (the probe field that will ring).
  seed_gaussian_flux_packet(rb, L, c, a.sigma, a.amp);

  // One warm-up solve so phi_coulomb_ is populated from the fixed source before
  // we snapshot the reference field and J(0). (tick() pre-solves phi_C when
  // db_clock_coulomb is on; we want phi_C_ref captured AFTER the first solve.)
  rb.tick();

  // Reference static field after first solve.
  std::vector<double> phi_ref = rb.phi_coulomb();  // copy

  // Probe set and J(0) snapshot (captured AFTER the warm-up tick so the
  // autocorrelation reference is the post-solve clocked field).
  const std::vector<int> probes = probe_ball(rb, L, c, R);
  const int center_idx = rb.lattice().index(c, c, c);
  std::printf("[probe] ball radius=%.3f -> %zu probe voxels; phi_C(center)=%+.6e\n",
              R, probes.size(), phi_ref[center_idx]);
  // Z-scaling verification: φ_C should scale linearly with Z (well depth ×Z).
  // Report φ_C(center)/Z so the Z=1 vs Z=2 runs can be compared directly.
  std::printf("[Z-well] Z=%.3g  phi_C(center)=%+.6e  phi_C(center)/Z=%+.6e\n",
              a.Z, phi_ref[center_idx], phi_ref[center_idx] / a.Z);

  std::vector<ftd::Vec3> J0(probes.size());
  for (size_t p = 0; p < probes.size(); ++p) J0[p] = rb.flux_at(probes[p]);

  // --- main recording loop: C(t) = Σ_probe J(0)·J(t), plus phi_C drift check ---
  std::vector<double> corr;
  corr.reserve(a.ticks);
  double max_phi_drift = 0.0;
  const int drift_check_ticks = 20;

  // Device-probe fast path: on the GPU backend, drive GpuEngine::tick() directly
  // and compute C(t) with the on-device shell-autocorrelation (no per-tick
  // full-lattice download). The numbers are identical to the RenderBridge path
  // up to the deterministic fixed-order host sum (same probe order). phi_C is
  // static (db_clock_coulomb, forces off), so the drift check is skipped here;
  // the warm-up phi_ref above already pins it. Validated against the slow path
  // at L=32/64 (bit-near, <1e-9 on C(t)).
  bool used_device_probe = false;
#ifdef FTD_ENABLE_CUDA
  if (a.device_probe && rb.backend_kind() == ftd::Backend::Kind::Gpu) {
    if (auto* gpu = rb.gpu_engine_ptr()) {
      used_device_probe = true;
      gpu->toggles = rb.toggles;          // pin the (static) toggle stack
      gpu->set_dt(a.dt);                  // honor the symplectic sub-step on device
      gpu->spectro_set_probes(probes);    // captures J(0) on device (post warm-up)
      std::printf("[device-probe] on — GPU shell-autocorrelation, %zu probes; "
                  "no per-tick full-lattice download\n", probes.size());
      const int report_every = (a.ticks >= 8) ? a.ticks / 8 : 1;
      for (int t = 0; t < a.ticks; ++t) {
        corr.push_back(gpu->spectro_autocorr());
        gpu->tick();
        if (((t + 1) % report_every) == 0)
          std::printf("[device-probe] tick %d/%d\n", t + 1, a.ticks), std::fflush(stdout);
      }
    }
  }
#endif

  if (!used_device_probe) {
    for (int t = 0; t < a.ticks; ++t) {
      double ct = 0.0;
      for (size_t p = 0; p < probes.size(); ++p) {
        const ftd::Vec3 J = rb.flux_at(probes[p]);
        ct += J0[p].x * J.x + J0[p].y * J.y + J0[p].z * J.z;
      }
      corr.push_back(ct);

      // phi_C static check over the first ~20 ticks: ||phi_C(t) − phi_C(0)||_∞.
      if (t < drift_check_ticks) {
        const auto& phi_now = rb.phi_coulomb();
        double d = 0.0;
        for (size_t i = 0; i < phi_now.size(); ++i)
          d = std::max(d, std::abs(phi_now[i] - phi_ref[i]));
        max_phi_drift = std::max(max_phi_drift, d);
      }
      rb.tick();
    }
    std::printf("[phiC-static] max ||phi_C(t)-phi_C(0)||_inf over first %d ticks = %.3e\n",
                drift_check_ticks, max_phi_drift);
  }

  // Dump the static phi_C field once. We dump phi_ref (captured right after the
  // warm-up solve); phi_C is static for this profile (db_clock_coulomb + forces
  // off), confirmed by the drift check on the slow path. On the device-probe
  // path the host phi_coulomb_ is not re-synced each tick, so phi_ref is the
  // authoritative static field to dump.
  dump_phi_csv(rb, L, phi_csv, phi_ref);

  // Write C(t).
  {
    std::FILE* f = std::fopen(ct_csv.string().c_str(), "w");
    if (!f) { std::fprintf(stderr, "cannot open %s\n", ct_csv.string().c_str()); return 1; }
    std::fprintf(f, "tick,corr\n");
    for (int t = 0; t < a.ticks; ++t) std::fprintf(f, "%d,%.12e\n", t, corr[t]);
    std::fclose(f);
    std::printf("[Ct] wrote %d-sample autocorrelation -> %s\n",
                a.ticks, ct_csv.string().c_str());
  }

  // --- FFT the C(t) series, report peaks and the engine ground frequency ---
  // Remove DC (mean) before the PSD so the bound mode is not buried under the
  // static offset of the autocorrelation.
  double mean = 0.0;
  for (double v : corr) mean += v;
  mean /= corr.size();
  std::vector<double> series = corr;
  for (double& v : series) v -= mean;

  const auto psd = ftd::power_spectrum(series);
  int Nfft = 1;
  while (Nfft < static_cast<int>(series.size())) Nfft <<= 1;

  // List the top peaks (local maxima above a noise floor).
  // Raw bin frequency Omega = 2π·bin/Nfft [rad/step]. The symplectic leapfrog
  // discretizes a cos(Ω·n) mode whose physical frequency satisfies
  //   2·sin(Ω/2) = ω_phys·dt   ⇒   ω_phys = (2/dt)·sin(Ω/2).
  // We report BOTH; the operator predicts ω_phys, so the BOUND test and the
  // analyzer comparison use ω_phys (= the de Broglie / KG mode frequency).
  double psd_max = 0.0;
  for (size_t i = 1; i < psd.size(); ++i) psd_max = std::max(psd_max, psd[i]);
  const double floor = 1e-3 * psd_max;
  const double inv_dt2 = 2.0 / a.dt;

  struct Peak { int bin; double omega_raw; double omega_phys; double power; };
  std::vector<Peak> peaks;
  for (int i = 1; i + 1 < static_cast<int>(psd.size()); ++i) {
    if (psd[i] > floor && psd[i] >= psd[i - 1] && psd[i] > psd[i + 1]) {
      const double om_raw = 2.0 * ftd::PI * i / Nfft;
      const double om_phys = inv_dt2 * std::sin(0.5 * om_raw);
      peaks.push_back({i, om_raw, om_phys, psd[i]});
    }
  }
  std::printf("----------------------------------------------------------------\n");
  std::printf("[FFT] Nfft=%d  dt=%.3f  peaks above %.2e (1e-3 of max):\n",
              Nfft, a.dt, floor);
  std::printf("      omega_phys[rad/t]  Omega_raw[rad/step]  bin     power\n");
  int printed = 0;
  for (const auto& pk : peaks) {
    std::printf("      %+.6f          %+.6f         %5d   %.4e%s\n",
                pk.omega_phys, pk.omega_raw, pk.bin, pk.power,
                (pk.omega_phys < a.omega0) ? "   [BOUND]" : "");
    if (++printed >= 12) break;
  }

  // Engine ground frequency = lowest-bin peak (the bound 1s envelope rings at
  // the smallest nonzero frequency below omega0). Reported on the physical axis.
  double omega_ground = 0.0;
  if (!peaks.empty()) omega_ground = peaks.front().omega_phys;  // peaks in bin order
  const bool bound = omega_ground > 0.0 && omega_ground < a.omega0;

  std::printf("----------------------------------------------------------------\n");
  std::printf("[RESULT] omega0 (clock)               = %.6f rad/tick\n", a.omega0);
  std::printf("[RESULT] engine ground peak omega_1s  = %.6f rad/tick  %s\n",
              omega_ground, bound ? "[BELOW omega0 -> BOUND]" : "[NOT below omega0]");
  if (bound) {
    std::printf("[RESULT] binding (omega0 - omega_1s)  = %.6f rad/tick\n",
                a.omega0 - omega_ground);
  }
  std::printf("================================================================\n");
  std::printf("Analyze with: python scripts/exploration/analyze_atomic_spectroscopy.py "
              "--ct %s --phi %s --omega0 %.4f --dt %.4f\n",
              ct_csv.string().c_str(), phi_csv.string().c_str(), a.omega0, a.dt);

  return 0;
}
