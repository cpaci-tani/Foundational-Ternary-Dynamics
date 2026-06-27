/**
 * @file campaign_mass_gap.cpp
 * @brief FTD-0270 closure swing (P2): the nonlinear-loop native mass gap.
 *
 * Frozen instrument for PREREG_QDYN_MASS_GAP_v1 (LEDGER FTD-0333 on verdict).
 * Pre-registration:
 *   docs/theory/03_derivations/foundational_mechanics/PREREG_QDYN_MASS_GAP_v1.md
 *
 * THE QUESTION (pre-reg §0). The linear native flux is already established
 * massless at k=0 (omega0 = 0): the clock-OFF rest mode is flat to ~4e-15,
 * FTD-0270 measures the massless dispersion s=0.944. That is NOT the question
 * here -- it is the linear control (G1) below. The only genuinely [OPEN] slice:
 *
 *   Does the FULL NONLINEAR genesis<->Gauss back-reaction loop (genesis firing
 *   + kinetic drain + Gauss projection all live) dynamically generate a k=0
 *   restoring oscillation omega0>0 of a manifested resting cluster's flux that
 *   the linear operator analysis structurally cannot see?
 *
 * Prior: CLOSED-NEGATIVE (~70%). A null hardens FTD-0270 [MEASURED -- BOUNDARY].
 *
 * CONFIG (native dynamics, clock OFF), golden-neutral / observation-only.
 *   Toggles ON : wave_propagation, coupling, genesis, gauss_projection.
 *   Toggles OFF: dual_substrate, de_broglie_clock, langevin (T=0).
 *   de_broglie_clock / db_clock_coulomb / symplectic_leapfrog all default OFF
 *   (dead branches in phase_read) -> the golden hash 0xb604d81a3d79366e is
 *   unaffected. This campaign only READS the bridge (flux_at / state) and never
 *   touches the golden harness. Deterministic seed_rng; CPU reference path
 *   (OMP_NUM_THREADS=1) is canonical (the FTD-0267 genesis event counters are
 *   CPU-only). force_cpu() is applied unconditionally.
 *
 * OBSERVABLE (pre-reg §2). Rest-frame flux autocorrelation
 *   C(t) = sum_{i in probe} J_i(0) . J_i(t)
 * over a probe ball (radius 5) centred on the cluster, sampled every tick for
 * T ticks, DC-removed, FFT'd (ftd::power_spectrum), peak-picked above
 * 1e-3 * PSD_max, with the leapfrog frequency correction
 *   omega_phys = (2/dt) * sin(omega_raw / 2).
 * omega0 = the lowest coherent spectral peak. The cluster is at rest, so all
 * spectral content is rest-frame (k=0).
 *
 * TWO WINDOWS (the quiescence discriminator, pre-reg §2/§4):
 *   forming   : genesis active (window starts at injection, t in [0, T)).
 *   quiescent : after the genesis firing rate falls below 10% of its peak; the
 *               window's J(0) is re-baselined at the quiescence onset.
 * A genuine rest-mass gap persists into quiescence; a genesis-relaxation
 * artifact vanishes with the firing rate (omega0^q / omega0^f << 1).
 *
 * LINEAR CONTROL (pre-reg §2/§3 G1): identical readout with genesis +
 * gauss_projection OFF (pure linear wave on the same seeded flux). Must
 * reproduce the established omega0 ~ 0 -- validates the readout / null baseline.
 *
 * GATES (pre-reg §3):
 *   G1 readout validity : omega0_ctrl < 0.01.
 *   G2 instability       : rho = <|J|>(t+1)/<|J|>(t) over the FFT window < 1.0005.
 *                          (rho >~ 1.002 is the FTD-0308 leapfrog blow-up -> INVALID,
 *                           NOT a positive gap.)
 *   G3 cluster formed    : N >= 3 manifested voxels persist through the windows.
 *   G4 determinism       : CPU bit-reproducible across seeds-of-record (run twice).
 *
 * OUTPUT: a machine-readable table, one row per (L, A):
 *   L, A, omega0_forming, omega0_quiescent, omega0_ctrl, rho, g_rate_peak, N,
 *   sharp_forming, sharp_quiescent, t_quiescent
 * emitted to stdout (prefixed "ROW,") and to a CSV under --output-dir.
 *
 * THIS RUNNER REPORTS MEASUREMENTS; IT DOES NOT ADJUDICATE THE VERDICT. The
 * forming/quiescent/control omega0 values feed the §4 discriminator table in a
 * separate ANALYSIS doc AFTER the pre-reg is hash-locked. Banned moves (§7): do
 * not enable the clock, do not tune omega0/coupling/threshold, do not read a G2
 * fail as a gap, do not report "massless at k=0" as the result.
 *
 * CLI:
 *   (no args)                   SMOKE: L=16 A=10 window=256 (CI regression; NOT
 *                               a measurement). This is what `ctest
 *                               campaign_mass_gap` runs — fast, deterministic,
 *                               golden-neutral.
 *   --sweep                     CANONICAL MEASUREMENT: L∈{32,48} × A∈{6,8,10,12,16}
 *                               × window 4096 (the full PREREG_QDYN_MASS_GAP_v1
 *                               §2 protocol). Run BY HAND after the pre-reg lock.
 *   --L=N (repeatable)          lattice sizes        (override either preset)
 *   --A=X (repeatable)          amplitude sweep      (override either preset)
 *   --window=N                  FFT window length T  (override either preset)
 *   --max-ticks=N               hard evolution cap   (default window*4 + 4096)
 *   --probe-radius=R            probe ball radius    (default 5)
 *   --seed=0xHEX|N              RNG seed             (default 0xD0270002)
 *   --dt=X                      time step            (default 1.0)
 *   --output-dir=PATH           CSV directory        (default engine/results/mass_gap_default/)
 *   --tag=S                     CSV/file tag         (default smoke / p2)
 *
 * Run semantics (frozen): the NO-ARG default is the SMOKE — a trivial sanity /
 * CI run, NOT a measurement. The canonical measurement requires the explicit
 * --sweep flag. Explicit --L / --A / --window override whichever preset applies.
 */

#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/spectral.h"
#include "ftd/voxel.h"

#include <algorithm>
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

// ---- Probe ball: lattice indices within Euclidean radius R of the center. ----
std::vector<int> probe_ball(const ftd::RenderBridge& rb, int cx, int cy, int cz, int R) {
    const ftd::Lattice& lat = rb.lattice();
    const int L = lat.size();
    std::vector<int> idx;
    const int R2 = R * R;
    for (int dx = -R; dx <= R; ++dx)
        for (int dy = -R; dy <= R; ++dy)
            for (int dz = -R; dz <= R; ++dz) {
                if (dx * dx + dy * dy + dz * dz > R2) continue;
                const int x = ((cx + dx) % L + L) % L;
                const int y = ((cy + dy) % L + L) % L;
                const int z = ((cz + dz) % L + L) % L;
                idx.push_back(lat.index(x, y, z));
            }
    return idx;
}

// ---- Mean flux magnitude <|J|> over the whole lattice (the G2 rho monitor). --
double mean_flux_magnitude(const ftd::RenderBridge& rb) {
    const auto& vox = rb.voxels();
    double s = 0.0;
    for (const auto& v : vox) s += std::sqrt(v.flux.mag2());
    return vox.empty() ? 0.0 : s / static_cast<double>(vox.size());
}

// ---- Manifested-voxel count (state != 0): the FTD-0273 cluster mass N. -------
int manifested_count(const ftd::RenderBridge& rb) {
    const auto& vox = rb.voxels();
    int n = 0;
    for (const auto& v : vox) if (v.state != 0) ++n;
    return n;
}

// ---- One FFT window's spectral readout. --------------------------------------
//
// Snapshots J(0) over the probe set at window start, then advances `window`
// ticks accumulating C(t) = sum_probe J(0).J(t) and <|J|>(t). DC-removes,
// FFTs, and peak-picks the LOWEST coherent peak above 1e-3*PSD_max. Applies the
// leapfrog frequency correction. Returns omega0 (lattice angular freq), peak
// sharpness (peak_power / total_power in [0,1]), and rho (mean per-tick growth).
struct WindowResult {
    double omega0   = 0.0;   // lowest coherent spectral peak (leapfrog-corrected)
    double sharpness = 0.0;  // peak power / summed power (coherence proxy)
    double rho      = 0.0;   // <|J|>(t+1)/<|J|>(t) averaged over the window (G2)
    double psd_max  = 0.0;   // for diagnostics
    bool   found    = false; // a peak above floor was found
};

WindowResult measure_window(ftd::RenderBridge& rb,
                            const std::vector<int>& probes,
                            int window, double dt) {
    WindowResult r;
    if (window < 4) return r;

    // J(0) reference over the probe set.
    std::vector<ftd::Vec3> J0(probes.size());
    for (size_t p = 0; p < probes.size(); ++p) J0[p] = rb.flux_at(probes[p]);

    std::vector<double> corr;
    corr.reserve(window);
    double prev_mag = mean_flux_magnitude(rb);
    if (prev_mag <= 0.0) prev_mag = 1e-300;
    double rho_accum = 0.0;
    int    rho_count = 0;

    for (int t = 0; t < window; ++t) {
        double ct = 0.0;
        for (size_t p = 0; p < probes.size(); ++p) {
            const ftd::Vec3 J = rb.flux_at(probes[p]);
            ct += J0[p].x * J.x + J0[p].y * J.y + J0[p].z * J.z;
        }
        corr.push_back(ct);

        rb.run(1);

        const double mag = mean_flux_magnitude(rb);
        if (prev_mag > 1e-300) { rho_accum += mag / prev_mag; ++rho_count; }
        prev_mag = (mag > 1e-300) ? mag : 1e-300;
    }
    r.rho = (rho_count > 0) ? rho_accum / rho_count : 0.0;

    // DC removal.
    double mean = 0.0;
    for (double v : corr) mean += v;
    mean /= static_cast<double>(corr.size());
    for (double& v : corr) v -= mean;

    // Power spectrum (radix-2, auto zero-padded to next pow2).
    const std::vector<double> psd = ftd::power_spectrum(corr);
    int Nfft = 1;
    while (Nfft < static_cast<int>(corr.size())) Nfft <<= 1;

    // Noise floor + total power (skip the DC bin 0).
    double psd_max = 0.0, total = 0.0;
    for (size_t i = 1; i < psd.size(); ++i) {
        psd_max = std::max(psd_max, psd[i]);
        total  += psd[i];
    }
    r.psd_max = psd_max;
    if (psd_max <= 0.0) return r;
    const double floor = 1e-3 * psd_max;

    // Lowest-bin local-maximum peak above floor.
    for (size_t i = 1; i + 1 < psd.size(); ++i) {
        if (psd[i] > floor && psd[i] >= psd[i - 1] && psd[i] > psd[i + 1]) {
            const double omega_raw = 2.0 * ftd::PI * static_cast<double>(i)
                                   / static_cast<double>(Nfft);   // rad / step
            // Leapfrog correction: 2*sin(Omega/2) = omega_phys * dt.
            r.omega0    = (2.0 / dt) * std::sin(omega_raw / 2.0);
            r.sharpness = (total > 0.0) ? psd[i] / total : 0.0;
            r.found     = true;
            break;
        }
    }
    return r;
}

// ---- Configure the native nonlinear stack (or the linear control). -----------
//
// native=true  : wave + coupling + genesis + gauss_projection (the P2 loop).
// native=false : wave + coupling only, genesis + gauss OFF (the G1 linear ctrl).
// In BOTH: dual_substrate / de_broglie_clock / langevin OFF; CPU; deterministic.
void configure(ftd::RenderBridge& rb, bool native, std::uint32_t seed) {
    rb.force_cpu();
    rb.set_sor_iterations(150);
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.coupling         = true;
    rb.toggles.genesis          = native;   // OFF in the linear control
    rb.toggles.gauss_projection = native;   // OFF in the linear control
    rb.toggles.dual_substrate   = false;
    rb.toggles.de_broglie_clock = false;    // NATIVE flux: no imposed -omega0^2 J
    rb.toggles.langevin         = false;    // T=0 deterministic rest frame
    rb.seed_rng(seed);
}

struct RunOutput {
    int    L = 0;
    double A = 0.0;
    double omega0_forming   = 0.0;
    double omega0_quiescent = 0.0;
    double omega0_ctrl      = 0.0;
    double rho              = 0.0;   // worst (max) rho across the measured windows
    double g_rate_peak      = 0.0;
    int    N                = 0;
    double sharp_forming    = 0.0;
    double sharp_quiescent  = 0.0;
    int    t_quiescent      = -1;    // tick at which quiescence onset was detected
};

// One (L, A) native run: forming window + locate quiescence + quiescent window.
RunOutput run_native(int L, double A, int window, int max_ticks,
                     int probe_radius, std::uint32_t seed, double dt) {
    RunOutput out;
    out.L = L;
    out.A = A;

    ftd::RenderBridge rb(L);
    configure(rb, /*native=*/true, seed);

    const int c = L / 2;
    rb.inject_flux(c, c, c, {A * ftd::K_GENESIS, 0.0, 0.0});
    const std::vector<int> probes = probe_ball(rb, c, c, c, probe_radius);

    // ---- FORMING window: starts at injection (genesis active). ----
    const WindowResult forming = measure_window(rb, probes, window, dt);
    out.omega0_forming = forming.omega0;
    out.sharp_forming  = forming.sharpness;
    double rho_worst   = forming.rho;

    // Genesis-rate peak observed during forming (the burst lives here; FTD-0267
    // shows the one-shot burst is ~tens of ticks, well inside one window).
    // measure_window already advanced the bridge; we instead track the peak by a
    // light forward scan with the counters AFTER forming, plus the forming-phase
    // peak captured during a quick pre-scan would double-evolve. To keep a single
    // forward evolution we read the cumulative-since-injection peak by re-deriving
    // it from a short diagnostic: advance until the per-tick genesis rate decays
    // below 10% of the running peak, recording the peak as we go.
    double g_peak = 0.0;
    int    t_total = window;      // ticks already advanced inside `forming`
    int    t_quiescent = -1;

    // Walk forward (post-forming) until quiescence onset, capping at max_ticks.
    // The genesis burst is typically already over by the end of the forming
    // window; in that case the very first sampled rate is sub-threshold and the
    // quiescent window begins immediately.
    while (t_total < max_ticks) {
        const long long gev = rb.genesis_events_this_tick();
        const double grate = static_cast<double>(gev);
        if (grate > g_peak) g_peak = grate;
        // Quiescence: firing rate below 10% of peak (and peak established).
        if (g_peak > 0.0 && grate < 0.10 * g_peak) { t_quiescent = t_total; break; }
        if (g_peak == 0.0 && t_total > window + 64) { t_quiescent = t_total; break; }
        rb.run(1);
        ++t_total;
    }
    if (t_quiescent < 0) t_quiescent = t_total;  // hit the cap; measure here anyway
    out.g_rate_peak = g_peak;
    out.t_quiescent = t_quiescent;

    // ---- QUIESCENT window: J(0) re-baselined at quiescence onset. ----
    const WindowResult quiescent = measure_window(rb, probes, window, dt);
    out.omega0_quiescent = quiescent.omega0;
    out.sharp_quiescent  = quiescent.sharpness;
    rho_worst = std::max(rho_worst, quiescent.rho);
    out.rho = rho_worst;

    out.N = manifested_count(rb);
    return out;
}

// One (L, A) linear control: genesis + gauss OFF; same seed, same seeded flux.
double run_control(int L, double A, int window, int probe_radius,
                   std::uint32_t seed, double dt) {
    ftd::RenderBridge rb(L);
    configure(rb, /*native=*/false, seed);
    const int c = L / 2;
    rb.inject_flux(c, c, c, {A * ftd::K_GENESIS, 0.0, 0.0});
    const std::vector<int> probes = probe_ball(rb, c, c, c, probe_radius);
    const WindowResult ctrl = measure_window(rb, probes, window, dt);
    return ctrl.omega0;
}

} // namespace

int main(int argc, char** argv) {
    std::vector<int>    Ls;          // empty => preset fills it
    std::vector<double> As;          // empty => preset fills it
    int window       = -1;           // <0 => preset fills it
    int max_ticks    = -1;           // default: window*4 + 4096 (set after parse)
    int probe_radius = 5;
    std::uint32_t seed = 0xD0270002u;
    double dt        = 1.0;
    bool sweep       = false;        // --sweep => canonical measurement preset
    std::string output_dir = "engine/results/mass_gap_default/";
    std::string tag;                 // empty => preset fills it (smoke / p2)

    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if      (a.rfind("--L=", 0) == 0)            Ls.push_back(std::atoi(a.c_str() + 4));
        else if (a.rfind("--A=", 0) == 0)            As.push_back(std::atof(a.c_str() + 4));
        else if (a.rfind("--window=", 0) == 0)       window = std::atoi(a.c_str() + 9);
        else if (a.rfind("--max-ticks=", 0) == 0)    max_ticks = std::atoi(a.c_str() + 12);
        else if (a.rfind("--probe-radius=", 0) == 0) probe_radius = std::atoi(a.c_str() + 15);
        else if (a.rfind("--seed=", 0) == 0)         seed = static_cast<std::uint32_t>(std::strtoul(a.c_str() + 7, nullptr, 0));
        else if (a.rfind("--dt=", 0) == 0)           dt = std::atof(a.c_str() + 5);
        else if (a == "--sweep")                     sweep = true;
        else if (a.rfind("--output-dir=", 0) == 0)   output_dir = a.substr(13);
        else if (a.rfind("--tag=", 0) == 0)          tag = a.substr(6);
    }

    // Run semantics: the NO-ARG default is the SMOKE (CI regression, NOT a
    // measurement). --sweep opts into the canonical PREREG_QDYN_MASS_GAP_v1 §2
    // protocol. Explicit --L / --A / --window override whichever preset applies.
    if (sweep) {
        // Canonical measurement preset (run by hand after the pre-reg lock).
        if (Ls.empty())  Ls = {32, 48};                       // pre-reg §2
        if (As.empty())  As = {6.0, 8.0, 10.0, 12.0, 16.0};   // pre-reg §2
        if (window < 0)  window = 4096;                        // pre-reg §2
        if (tag.empty()) tag = "p2";
    } else {
        // Smoke preset: trivial sanity / CI run (L=16, A=10, window=256).
        if (Ls.empty())  Ls = {16};
        if (As.empty())  As = {10.0};
        if (window < 0)  window = 256;
        if (tag.empty()) tag = "smoke";
    }
    if (max_ticks < 0) max_ticks = window * 4 + 4096;

    fs::create_directories(output_dir);
    const fs::path out_csv = fs::path(output_dir) / ("mass_gap_" + tag + ".csv");
    std::FILE* f = std::fopen(out_csv.string().c_str(), "w");
    if (!f) {
        std::fprintf(stderr, "ERROR: cannot open %s\n", out_csv.string().c_str());
        return 1;
    }

    std::printf("# campaign_mass_gap (FTD-0270 P2 / PREREG_QDYN_MASS_GAP_v1)\n");
    std::printf("# config: native dynamics, de_broglie_clock OFF; CPU; "
                "seed=0x%X window=%d probe_radius=%d dt=%.3f%s\n",
                seed, window, probe_radius, dt,
                sweep ? "  [--sweep: canonical measurement]" : "  [SMOKE -- not a measurement]");
    std::printf("# toggles ON: wave_propagation,coupling,genesis,gauss_projection ; "
                "OFF: dual_substrate,de_broglie_clock,langevin\n");
    std::printf("# golden-neutral (observation-only); golden hash 0xb604d81a3d79366e unaffected\n");
    std::printf("# columns: L,A,omega0_forming,omega0_quiescent,omega0_ctrl,rho,"
                "g_rate_peak,N,sharp_forming,sharp_quiescent,t_quiescent\n");
    std::fflush(stdout);

    std::fprintf(f, "# campaign_mass_gap FTD-0270 P2 seed=0x%X window=%d probe_radius=%d dt=%.3f%s\n",
                 seed, window, probe_radius, dt, sweep ? " SWEEP" : " SMOKE");
    std::fprintf(f, "L,A,omega0_forming,omega0_quiescent,omega0_ctrl,rho,"
                    "g_rate_peak,N,sharp_forming,sharp_quiescent,t_quiescent\n");

    for (int L : Ls) {
        for (double A : As) {
            const RunOutput r = run_native(L, A, window, max_ticks, probe_radius, seed, dt);
            const double omega0_ctrl = run_control(L, A, window, probe_radius, seed, dt);

            // ROW line is the machine-readable record on stdout.
            std::printf("ROW,%d,%.4f,%.6e,%.6e,%.6e,%.8f,%.1f,%d,%.4e,%.4e,%d\n",
                        r.L, r.A,
                        r.omega0_forming, r.omega0_quiescent, omega0_ctrl,
                        r.rho, r.g_rate_peak, r.N,
                        r.sharp_forming, r.sharp_quiescent, r.t_quiescent);
            std::fflush(stdout);

            std::fprintf(f, "%d,%.4f,%.6e,%.6e,%.6e,%.8f,%.1f,%d,%.4e,%.4e,%d\n",
                         r.L, r.A,
                         r.omega0_forming, r.omega0_quiescent, omega0_ctrl,
                         r.rho, r.g_rate_peak, r.N,
                         r.sharp_forming, r.sharp_quiescent, r.t_quiescent);
            std::fflush(f);
        }
    }

    std::fclose(f);
    std::printf("DONE -> %s\n", out_csv.string().c_str());
    return 0;
}
