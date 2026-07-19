/**
 * @file campaign_mass_gap_v2.cpp
 * @brief FTD-0270 closure swing (P2), v2 instrument — the nonlinear-loop
 *        native mass gap, rebuilt per the FTD-0333 postmortem and the
 *        FTD-0337 mechanism correction.
 *
 * Frozen instrument for PREREG_QDYN_MASS_GAP_v2 (LEDGER row minted by the
 * controller on verdict). Pre-registration:
 *   docs/theory/03_derivations/foundational_mechanics/PREREG_QDYN_MASS_GAP_v2.md
 * Predecessor: PREREG_QDYN_MASS_GAP_v1 + ANALYSIS_QDYN_MASS_GAP_v1 (FTD-0333,
 * verdict INVALID: G2 instability on all runs, G1 control mis-calibrated,
 * L=32 flooding). FTD-0337 corrected the instability mechanism: BARE-WAVE
 * leapfrog amplitude growth (a discretization signature, shrinking ~dt² on
 * the dt-honoring symplectic path), NOT a parametric KG-well instability.
 *
 * THE QUESTION (unchanged from v1 §0): does the FULL NONLINEAR genesis<->Gauss
 * back-reaction loop (genesis firing + kinetic drain + Gauss projection all
 * live) dynamically generate a k=0 restoring oscillation omega0>0 of a
 * manifested resting cluster's flux that the linear operator analysis
 * structurally cannot see? The linear massless baseline (omega0=0 at k=0) is
 * ESTABLISHED and is NOT the question (banned move #1).
 *
 * THE THREE v1 LESSONS IMPLEMENTED HERE (FTD-0333 §5):
 *  1. E1 STABLE INTEGRATOR: toggles.verlet_wave_integrator (new, default OFF,
 *     golden-neutral) — velocity-Verlet KDK bare-wave update honoring dt<1.
 *     Canonical sweep runs it at dt=0.5 (well inside the CFL margin).
 *     PLUS an injection-subtracted growth metric (below) so G2 isolates
 *     numerical instability from physical genesis/coupling injection.
 *  2. k=0-ISOLATING CONTROL: the uniform-J (spatial-mean) mode Jbar(t) is
 *     tracked separately in every run. Under the periodic-lattice dynamics
 *     Jbar is conserved by the wave term, the Gauss projection (a lattice
 *     gradient has zero mean), and the coupling source (zero-mean lattice
 *     differences) — so the control's k0 series must be FLAT (G1a null
 *     channel); the control's probe-ball series must ring in the massless
 *     dispersing-wavepacket band (G1b positive channel). v1's mistake was
 *     gating the control on the probe series against an omega ~ 0
 *     expectation.
 *  3. NON-FLOODING SETUP: L in {48, 64} with the stable-island amplitudes
 *     A in {9, 9.5, 13} (v1: L=32 flooded to N=L^3 at every swept A).
 *
 * GROWTH METRICS (G2, recalibrated per FTD-0337):
 *   G2a (bare-wave stability control): same seeded flux, ONLY wave_propagation
 *       ON, same integrator/dt. drift_bare = mean_t (E(t+1)-E(t))/E(t) with
 *       E = ½Σ(|J|²+|wave_vel|²). This is zero-injection by construction, so
 *       any drift is the integrator. (v1's <|J|> ratio rho conflated packet
 *       spreading — which raises Σ|J| at fixed Σ|J|² — with instability;
 *       rho is still reported for continuity but NOT gated.)
 *   G2b (native, injection-subtracted): over the QUIESCENT window,
 *       drift_adj = mean_t (E(t+1)-E(t)-P_c(t))/E(t), where
 *       P_c(t) = Σ_{sites adjacent to manifested} wave_vel·(G_C·∇s)·dt is the
 *       first-order coupling-work estimate (the curl(s·v) source is exactly
 *       zero here: forces OFF ⇒ all velocities stay 0).
 *
 * CONFIG (native dynamics, clock OFF), golden-neutral / observation-only:
 *   Toggles ON : wave_propagation, coupling, genesis, gauss_projection,
 *                verlet_wave_integrator (the E1 instrument; --integrator).
 *   Toggles OFF: dual_substrate, de_broglie_clock, langevin, damping, forces.
 *   The verlet toggle is default-OFF in the engine; enabling it HERE does not
 *   touch the golden profile. Golden hash 0xb604d81a3d79366e unaffected.
 *   CPU reference path canonical (OMP_NUM_THREADS=1 recommended); force_cpu().
 *
 * OBSERVABLES: probe-ball autocorrelation C(t) = Σ_probe J(0)·J(t) (v1
 * continuity; the local detector) and the uniform-mode series
 * C0(t) = Jbar(0)·Jbar(t) (k=0-pure channel), both DC-removed, FFT'd,
 * peak-picked above 1e-3·PSD_max with the leapfrog frequency correction
 * omega_phys = (2/dt)·sin(omega_raw/2). Two windows (forming / quiescent,
 * quiescence = genesis rate < 10% of its peak), quiescent J(0) re-baselined.
 *
 * THIS RUNNER REPORTS MEASUREMENTS; IT DOES NOT ADJUDICATE THE VERDICT.
 * Gates and the FORCED / CLOSED-NEGATIVE / INVALID outcome map are frozen in
 * PREREG_QDYN_MASS_GAP_v2.md §3-§4. Banned moves (v1 §7) all carry over.
 *
 * CLI:
 *   (no args)                 SMOKE: L=16 A=10 window=256 dt=0.5 verlet (CI
 *                             sanity; NOT a measurement).
 *   --sweep                   CANONICAL: L∈{48,64} × A∈{9,9.5,13} × window
 *                             4096 × dt=0.5 × verlet.
 *   --L=N / --A=X (repeat.)   preset overrides
 *   --window=N                FFT window length
 *   --max-ticks=N             hard cap (default window*4 + 4096)
 *   --probe-radius=R          probe ball radius (default 5)
 *   --seed=0xHEX|N            RNG seed (default 0xD0270002; determinism gate
 *                             G4 = re-run bit-identical + second seed run)
 *   --dt=X                    time step (default 0.5; honored by verlet/leapfrog)
 *   --integrator=S            verlet | leapfrog | legacy (default verlet)
 *   --output-dir=PATH         CSV directory (default engine/results/mass_gap_v2/)
 *   --tag=S                   CSV tag (default smoke / v2)
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
#include <set>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

enum class Integrator { Legacy, Leapfrog, Verlet };

const char* integrator_name(Integrator ig) {
    switch (ig) {
        case Integrator::Legacy:   return "legacy";
        case Integrator::Leapfrog: return "leapfrog";
        case Integrator::Verlet:   return "verlet";
    }
    return "?";
}

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
                idx.push_back(lat.index(cx + dx, cy + dy, cz + dz));
            }
    return idx;
}

// ---- One whole-lattice pass: mean-|J|, field energy, uniform-mode vector. ----
struct LatticeScalars {
    double mean_absJ = 0.0;   // <|J|>          (v1 rho continuity metric)
    double energy    = 0.0;   // ½Σ(|J|²+|wv|²) (G2 metric)
    ftd::Vec3 jbar;           // spatial mean of J (uniform / k=0 mode)
};

LatticeScalars lattice_scalars(const ftd::RenderBridge& rb) {
    const auto& vox = rb.voxels();
    LatticeScalars s;
    double sum_abs = 0.0, e = 0.0;
    ftd::Vec3 jsum;
    for (const auto& v : vox) {
        sum_abs += std::sqrt(v.flux.mag2());
        e += v.flux.mag2() + v.wave_vel.mag2();
        jsum += v.flux;
    }
    const double n = static_cast<double>(vox.size());
    s.mean_absJ = (n > 0) ? sum_abs / n : 0.0;
    s.energy = 0.5 * e;
    s.jbar = jsum * ((n > 0) ? 1.0 / n : 0.0);
    return s;
}

int manifested_count(const ftd::RenderBridge& rb) {
    const auto& vox = rb.voxels();
    int n = 0;
    for (const auto& v : vox) if (v.state != 0) ++n;
    return n;
}

// ---- First-order coupling-work estimate P_c(t) (see header). Read-only. ----
// wave_vel · (G_C·∇s) · dt summed over sites where ∇s can be nonzero: the
// manifested sites and their 6 axis neighbors. The curl(s·v) source is exactly
// zero in this config (forces OFF ⇒ velocities identically 0).
double coupling_power(const ftd::RenderBridge& rb, double dt) {
    const ftd::Lattice& lat = rb.lattice();
    const int L = lat.size();
    const auto& active = rb.ordered_active_indices();
    if (active.empty()) return 0.0;
    std::set<int> sites;
    for (int idx : active) {
        const int x = idx / (L * L), y = (idx / L) % L, z = idx % L;
        sites.insert(idx);
        sites.insert(lat.index(x + 1, y, z));
        sites.insert(lat.index(x - 1, y, z));
        sites.insert(lat.index(x, y + 1, z));
        sites.insert(lat.index(x, y - 1, z));
        sites.insert(lat.index(x, y, z + 1));
        sites.insert(lat.index(x, y, z - 1));
    }
    const auto& vox = rb.voxels();
    double p = 0.0;
    for (int idx : sites) {
        // Coupling source is −g_c·∇s (Term 2 sign amendment 2026-07-18);
        // the injected power is P = v · source.
        const ftd::Vec3 gs = rb.gradient_state(idx) * (-ftd::G_C);
        p += vox[idx].wave_vel.dot(gs);
    }
    return p * dt;
}

// ---- One FFT window's spectral + stability readout. -------------------------
struct SeriesPeak {
    double omega0    = 0.0;
    double sharpness = 0.0;
    bool   found     = false;
};

SeriesPeak pick_peak(std::vector<double> series, double dt) {
    SeriesPeak r;
    if (series.size() < 8) return r;
    double mean = 0.0;
    for (double v : series) mean += v;
    mean /= static_cast<double>(series.size());
    for (double& v : series) v -= mean;

    const std::vector<double> psd = ftd::power_spectrum(series);
    int Nfft = 1;
    while (Nfft < static_cast<int>(series.size())) Nfft <<= 1;

    double psd_max = 0.0, total = 0.0;
    for (size_t i = 1; i < psd.size(); ++i) {
        psd_max = std::max(psd_max, psd[i]);
        total  += psd[i];
    }
    if (psd_max <= 0.0) return r;
    const double floor = 1e-3 * psd_max;
    for (size_t i = 1; i + 1 < psd.size(); ++i) {
        if (psd[i] > floor && psd[i] >= psd[i - 1] && psd[i] > psd[i + 1]) {
            const double omega_raw = 2.0 * ftd::PI * static_cast<double>(i)
                                   / static_cast<double>(Nfft);
            r.omega0    = (2.0 / dt) * std::sin(omega_raw / 2.0);
            r.sharpness = (total > 0.0) ? psd[i] / total : 0.0;
            r.found     = true;
            break;
        }
    }
    return r;
}

struct WindowResult {
    SeriesPeak probe;         // probe-ball autocorrelation peak
    SeriesPeak k0;            // uniform-J (k=0) mode peak
    double rho       = 0.0;   // mean <|J|>(t+1)/<|J|>(t)   (v1 continuity; NOT gated)
    double drift     = 0.0;   // mean (E(t+1)-E(t))/E(t)    (raw energy drift)
    double drift_adj = 0.0;   // mean (E(t+1)-E(t)-P_c)/E(t) (injection-subtracted)
    double g_peak    = 0.0;   // peak genesis rate observed inside the window
};

WindowResult measure_window(ftd::RenderBridge& rb, const std::vector<int>& probes,
                            int window, double dt, bool track_coupling) {
    WindowResult r;
    if (window < 8) return r;

    std::vector<ftd::Vec3> J0(probes.size());
    for (size_t p = 0; p < probes.size(); ++p) J0[p] = rb.flux_at(probes[p]);
    LatticeScalars s0 = lattice_scalars(rb);
    const ftd::Vec3 jbar0 = s0.jbar;

    std::vector<double> corr_probe, corr_k0;
    corr_probe.reserve(window);
    corr_k0.reserve(window);

    double rho_acc = 0.0, drift_acc = 0.0, drift_adj_acc = 0.0;
    int    rho_n = 0;
    LatticeScalars prev = s0;

    for (int t = 0; t < window; ++t) {
        double ct = 0.0;
        for (size_t p = 0; p < probes.size(); ++p) {
            const ftd::Vec3 J = rb.flux_at(probes[p]);
            ct += J0[p].dot(J);
        }
        corr_probe.push_back(ct);
        corr_k0.push_back(jbar0.dot(prev.jbar));

        const double pc = track_coupling ? coupling_power(rb, dt) : 0.0;
        rb.run(1);
        const double gev = static_cast<double>(rb.genesis_events_this_tick());
        if (gev > r.g_peak) r.g_peak = gev;

        const LatticeScalars cur = lattice_scalars(rb);
        if (prev.mean_absJ > 1e-300) { rho_acc += cur.mean_absJ / prev.mean_absJ; ++rho_n; }
        if (prev.energy > 1e-300) {
            drift_acc     += (cur.energy - prev.energy) / prev.energy;
            drift_adj_acc += (cur.energy - prev.energy - pc) / prev.energy;
        }
        prev = cur;
    }
    r.rho       = (rho_n > 0) ? rho_acc / rho_n : 0.0;
    r.drift     = drift_acc / window;
    r.drift_adj = drift_adj_acc / window;
    r.probe = pick_peak(corr_probe, dt);
    r.k0    = pick_peak(corr_k0, dt);
    return r;
}

// ---- Configure the stack. mode: 0=native, 1=linear control, 2=bare wave. ----
void configure(ftd::RenderBridge& rb, int mode, Integrator ig,
               std::uint32_t seed, double dt) {
    rb.force_cpu();
    rb.set_sor_iterations(150);
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.coupling         = (mode <= 1);   // OFF in the bare-wave control
    rb.toggles.genesis          = (mode == 0);
    rb.toggles.gauss_projection = (mode == 0);
    rb.toggles.dual_substrate   = false;
    rb.toggles.de_broglie_clock = false;   // NATIVE flux: no imposed -omega0^2 J
    rb.toggles.langevin         = false;   // T=0 deterministic rest frame
    rb.toggles.damping          = false;
    rb.toggles.verlet_wave_integrator = (ig == Integrator::Verlet);
    rb.toggles.symplectic_leapfrog    = (ig == Integrator::Leapfrog);
    rb.set_dt(dt);                          // AFTER toggles: dt<1 honored by E1
    rb.seed_rng(seed);
}

struct RunOutput {
    int    L = 0;
    double A = 0.0;
    // native
    WindowResult forming, quiescent;
    int    t_quiescent = -1;
    double g_rate_peak = 0.0;
    int    N_final     = 0;
    bool   flooded     = false;
    // controls
    SeriesPeak ctrl_probe, ctrl_k0;
    double rho_bare = 0.0, drift_bare = 0.0;
};

RunOutput run_case(int L, double A, int window, int max_ticks, int probe_radius,
                   std::uint32_t seed, double dt, Integrator ig) {
    RunOutput out;
    out.L = L;
    out.A = A;
    const int c = L / 2;

    // ---- NATIVE run: forming window -> quiescence walk -> quiescent window. ----
    {
        ftd::RenderBridge rb(L);
        configure(rb, /*mode=*/0, ig, seed, dt);
        rb.inject_flux(c, c, c, {A * ftd::K_GENESIS, 0.0, 0.0});
        const std::vector<int> probes = probe_ball(rb, c, c, c, probe_radius);

        out.forming = measure_window(rb, probes, window, dt, /*track_coupling=*/true);
        double g_peak = out.forming.g_peak;

        int t_total = window;
        int t_quiescent = -1;
        while (t_total < max_ticks) {
            const double grate = static_cast<double>(rb.genesis_events_this_tick());
            if (grate > g_peak) g_peak = grate;
            if (g_peak > 0.0 && grate < 0.10 * g_peak) { t_quiescent = t_total; break; }
            if (g_peak == 0.0 && t_total > window + 64) { t_quiescent = t_total; break; }
            rb.run(1);
            ++t_total;
        }
        if (t_quiescent < 0) t_quiescent = t_total;
        out.t_quiescent = t_quiescent;

        out.quiescent = measure_window(rb, probes, window, dt, /*track_coupling=*/true);
        if (out.quiescent.g_peak > g_peak) g_peak = out.quiescent.g_peak;
        out.g_rate_peak = g_peak;

        out.N_final = manifested_count(rb);
        const long long L3 = static_cast<long long>(L) * L * L;
        out.flooded = (out.N_final >= L3 / 2);
    }

    // ---- LINEAR CONTROL (G1): wave + coupling, genesis + gauss OFF. ----
    {
        ftd::RenderBridge rb(L);
        configure(rb, /*mode=*/1, ig, seed, dt);
        rb.inject_flux(c, c, c, {A * ftd::K_GENESIS, 0.0, 0.0});
        const std::vector<int> probes = probe_ball(rb, c, c, c, probe_radius);
        const WindowResult ctrl = measure_window(rb, probes, window, dt, false);
        out.ctrl_probe = ctrl.probe;
        out.ctrl_k0    = ctrl.k0;
    }

    // ---- BARE-WAVE STABILITY CONTROL (G2a): wave only, zero injection. ----
    {
        ftd::RenderBridge rb(L);
        configure(rb, /*mode=*/2, ig, seed, dt);
        rb.inject_flux(c, c, c, {A * ftd::K_GENESIS, 0.0, 0.0});
        const std::vector<int> probes = probe_ball(rb, c, c, c, probe_radius);
        const WindowResult bare = measure_window(rb, probes, window, dt, false);
        out.rho_bare   = bare.rho;
        out.drift_bare = bare.drift;
    }

    return out;
}

} // namespace

int main(int argc, char** argv) {
    std::vector<int>    Ls;
    std::vector<double> As;
    int window       = -1;
    int max_ticks    = -1;
    int probe_radius = 5;
    std::uint32_t seed = 0xD0270002u;
    double dt        = 0.5;
    Integrator ig    = Integrator::Verlet;
    bool sweep       = false;
    std::string output_dir = "engine/results/mass_gap_v2/";
    std::string tag;

    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if      (a.rfind("--L=", 0) == 0)            Ls.push_back(std::atoi(a.c_str() + 4));
        else if (a.rfind("--A=", 0) == 0)            As.push_back(std::atof(a.c_str() + 4));
        else if (a.rfind("--window=", 0) == 0)       window = std::atoi(a.c_str() + 9);
        else if (a.rfind("--max-ticks=", 0) == 0)    max_ticks = std::atoi(a.c_str() + 12);
        else if (a.rfind("--probe-radius=", 0) == 0) probe_radius = std::atoi(a.c_str() + 15);
        else if (a.rfind("--seed=", 0) == 0)         seed = static_cast<std::uint32_t>(std::strtoul(a.c_str() + 7, nullptr, 0));
        else if (a.rfind("--dt=", 0) == 0)           dt = std::atof(a.c_str() + 5);
        else if (a.rfind("--integrator=", 0) == 0) {
            const std::string v = a.substr(13);
            if      (v == "verlet")   ig = Integrator::Verlet;
            else if (v == "leapfrog") ig = Integrator::Leapfrog;
            else if (v == "legacy")   ig = Integrator::Legacy;
            else { std::fprintf(stderr, "ERROR: unknown --integrator=%s\n", v.c_str()); return 1; }
        }
        else if (a == "--sweep")                     sweep = true;
        else if (a.rfind("--output-dir=", 0) == 0)   output_dir = a.substr(13);
        else if (a.rfind("--tag=", 0) == 0)          tag = a.substr(6);
    }

    if (sweep) {
        if (Ls.empty())  Ls = {48, 64};              // non-flooding (v1 lesson 3)
        if (As.empty())  As = {9.0, 9.5, 13.0};      // stable islands (v1 lesson 3)
        if (window < 0)  window = 4096;
        if (tag.empty()) tag = "v2";
    } else {
        if (Ls.empty())  Ls = {16};
        if (As.empty())  As = {10.0};
        if (window < 0)  window = 256;
        if (tag.empty()) tag = "smoke";
    }
    if (max_ticks < 0) max_ticks = window * 4 + 4096;
    if (ig == Integrator::Legacy) dt = 1.0;          // legacy path clamps dt to 1

    fs::create_directories(output_dir);
    const fs::path out_csv = fs::path(output_dir) / ("mass_gap_v2_" + tag + ".csv");
    std::FILE* f = std::fopen(out_csv.string().c_str(), "w");
    if (!f) {
        std::fprintf(stderr, "ERROR: cannot open %s\n", out_csv.string().c_str());
        return 1;
    }

    int Nfft = 1;
    while (Nfft < window) Nfft <<= 1;
    const double fft_floor = (2.0 / dt) * std::sin(ftd::PI / Nfft);

    std::printf("# campaign_mass_gap_v2 (FTD-0270 P2 v2 / PREREG_QDYN_MASS_GAP_v2)\n");
    std::printf("# v1 lessons: E1 verlet integrator + injection-subtracted G2 + k0 control + stable islands\n");
    std::printf("# config: native, clock OFF; CPU; seed=0x%X window=%d probe_radius=%d dt=%.3f integrator=%s%s\n",
                seed, window, probe_radius, dt, integrator_name(ig),
                sweep ? "  [--sweep: canonical measurement]" : "  [SMOKE -- not a measurement]");
    std::printf("# toggles ON: wave_propagation,coupling,genesis,gauss_projection,%s ; "
                "OFF: dual_substrate,de_broglie_clock,langevin,damping,forces\n",
                (ig == Integrator::Verlet) ? "verlet_wave_integrator"
                : (ig == Integrator::Leapfrog) ? "symplectic_leapfrog" : "(default integrator)");
    std::printf("# golden-neutral (verlet toggle default-OFF in engine); golden hash 0xb604d81a3d79366e unaffected\n");
    std::printf("# fft_floor(omega_phys, lowest nonzero bin) = %.6e\n", fft_floor);
    const char* cols =
        "L,A,dt,integrator,"
        "omega0_probe_f,sharp_probe_f,omega0_probe_q,sharp_probe_q,"
        "omega0_k0_f,omega0_k0_q,"
        "omega0_probe_ctrl,sharp_probe_ctrl,omega0_k0_ctrl,"
        "rho_native_worst,drift_adj_q,drift_raw_q,rho_bare,drift_bare,"
        "g_rate_peak,N_final,t_quiescent,flooded,fft_floor";
    std::printf("# columns: %s\n", cols);
    std::fflush(stdout);

    std::fprintf(f, "# campaign_mass_gap_v2 FTD-0270 P2v2 seed=0x%X window=%d probe_radius=%d dt=%.3f integrator=%s%s\n",
                 seed, window, probe_radius, dt, integrator_name(ig), sweep ? " SWEEP" : " SMOKE");
    std::fprintf(f, "%s\n", cols);

    for (int L : Ls) {
        for (double A : As) {
            const RunOutput r = run_case(L, A, window, max_ticks, probe_radius, seed, dt, ig);
            const double rho_worst = std::max(r.forming.rho, r.quiescent.rho);
            char line[1024];
            std::snprintf(line, sizeof(line),
                "%d,%.4f,%.3f,%s,"
                "%.6e,%.4e,%.6e,%.4e,"
                "%.6e,%.6e,"
                "%.6e,%.4e,%.6e,"
                "%.8f,%.6e,%.6e,%.8f,%.6e,"
                "%.1f,%d,%d,%d,%.6e",
                r.L, r.A, dt, integrator_name(ig),
                r.forming.probe.omega0, r.forming.probe.sharpness,
                r.quiescent.probe.omega0, r.quiescent.probe.sharpness,
                r.forming.k0.omega0, r.quiescent.k0.omega0,
                r.ctrl_probe.omega0, r.ctrl_probe.sharpness, r.ctrl_k0.omega0,
                rho_worst, r.quiescent.drift_adj, r.quiescent.drift,
                r.rho_bare, r.drift_bare,
                r.g_rate_peak, r.N_final, r.t_quiescent, r.flooded ? 1 : 0, fft_floor);
            std::printf("ROW,%s\n", line);
            std::fflush(stdout);
            std::fprintf(f, "%s\n", line);
            std::fflush(f);
        }
    }

    std::fclose(f);
    std::printf("DONE -> %s\n", out_csv.string().c_str());
    return 0;
}
