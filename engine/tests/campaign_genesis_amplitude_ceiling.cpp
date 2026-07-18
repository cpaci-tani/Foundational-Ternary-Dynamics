/**
 * @file campaign_genesis_amplitude_ceiling.cpp
 * @brief Is there a maximum amplitude for coherent light in FTD? (the genesis ceiling)
 *
 * MOTIVATION. Light in FTD is a wave in the CONTINUOUS flux field J (FTD-0298).
 * The flux field carries no axiom-level amplitude cap (unlike the ternary state
 * field s in {-1,0,+1}). But the genesis rule fires when a void site reaches
 * |J| > K_GENESIS, and the manifestation step DRAINS the flux back to the
 * threshold (flux *= max(0, 1 - K_GENESIS/|J|)). So a coherent flux wave should
 * have an effective amplitude ceiling at |J| = K_GENESIS: push past it and the
 * crests stop growing and start shedding into matter (state flips). This is the
 * lattice analogue of a water wave reaching its breaking steepness, or of the
 * QED Schwinger limit (field too strong -> vacuum makes pairs).
 *
 * This campaign drives genesis with a COHERENT TRANSVERSE PLANE WAVE (not the
 * thermal Langevin drive of campaign_genesis_criticality, and not a localized
 * injection). It sweeps the wave amplitude A through K_GENESIS and asks:
 *
 *   THRESHOLD   -- at what A do the first genesis events appear?
 *                  prediction: exactly A = K_GENESIS (a hard local gate).
 *   SHARPNESS   -- is it a clean cliff (zero below, nonzero above)?
 *   WAVELENGTH  -- does the threshold move with wavelength?
 *                  prediction: NO (amplitude and wavelength are orthogonal).
 *   BREAKING    -- above threshold, does the void-field peak get pinned near
 *                  K_GENESIS while matter accumulates (the wave "breaks")?
 *
 * Three pre-stated outcomes (registered in conversation before running):
 *   (1) sharp K_GENESIS-locked, wavelength-invariant threshold -> a forward
 *       prediction of the framework (the coherent-light amplitude ceiling).
 *   (2) smeared crossover not locked to K_GENESIS -> a boundary (soft/dynamical
 *       ceiling).
 *   (3) no triggering by a coherent wave at all -> a sharper boundary
 *       (consistent with FTD-0274: local injections never detonate).
 *
 * The genesis mechanism is bit-exact CPU<->GPU (per-voxel SplitMix64 RNG), and
 * the threshold is a LOCAL gate on |J|, so it is L-independent: a CPU run at
 * modest L is a canonical measurement of the threshold. A WSL2/CUDA large-L
 * confirmation is a cheap follow-up.
 *
 * Stack: wave_propagation + gauss_projection + genesis [+ coupling]. Langevin
 * OFF (pure coherent wave, no thermal noise). dual_substrate OFF.
 *
 * Outputs:
 *   sweep_<tag>.csv : one row per (nodes, amplitude, seed)
 *   traj_<tag>.csv  : per-tick trajectory at one super-threshold amplitude
 *
 * Usage:
 *   campaign_genesis_amplitude_ceiling --cpu --L=32 --ticks=60 --seeds=4 \
 *       --nodes=2,4,8 --amps=0.8,1.0,1.2,1.4,1.5,1.533,1.55,1.7,2.0,2.5,3.0 \
 *       --traj-amp=2.5 --output-dir=PATH --tag=run
 */

#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/cluster_tracker.h"     // ClusterTracker: persistence / lifetime axis
#include "ftd/cluster_observables.h" // measure_cluster() -> coherence R, org = N*R
#include "ftd/correlations.h"        // charge_correlation(), structure_factor()

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

namespace {

constexpr double TWO_PI = 6.283185307179586476925286766559;

struct LatticeStats {
    int    manifested   = 0;     // voxels with state != 0
    double max_void_J   = 0.0;   // max |J| over VOID sites (genesis candidates)
    double flux_energy  = 0.0;   // sum of 0.5|J|^2 over all sites
};

LatticeStats measure(const ftd::RenderBridge& rb) {
    LatticeStats s;
    for (const auto& v : rb.voxels()) {
        const double j = v.flux.mag();
        s.flux_energy += 0.5 * j * j;
        if (v.state != 0) {
            ++s.manifested;
        } else if (j > s.max_void_J) {
            s.max_void_J = j;
        }
    }
    return s;
}

// Transverse standing plane wave: J = (0, A*sin(k x), 0), k = 2*pi*nodes/L.
// Divergence-free (survives Gauss projection); polarized in y, varies along x;
// antinodes reach |J| = A. wave_vel left at 0 -> standing wave whose antinodes
// return to amplitude A every period, giving genesis repeated shots at peak A.
void setup_plane_wave(ftd::RenderBridge& rb, int L, double A, int nodes) {
    const double k = TWO_PI * static_cast<double>(nodes) / static_cast<double>(L);
    for (int x = 0; x < L; ++x) {
        const ftd::Vec3 fv{0.0, A * std::sin(k * static_cast<double>(x)), 0.0};
        for (int y = 0; y < L; ++y)
            for (int z = 0; z < L; ++z)
                rb.inject_flux(x, y, z, fv);
    }
}

// static_field=true holds the injected |J| constant (wave_propagation OFF) to
// isolate the genesis GATE cleanly: onset should be exactly at |J|=K_GENESIS.
// static_field=false lets the coherent wave propagate (the physical "light"
// object) so we can watch it break into matter above the ceiling.
void configure(ftd::RenderBridge& rb, bool coupling_on, bool static_field) {
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = !static_field;
    rb.toggles.gauss_projection = true;
    rb.toggles.genesis          = true;     // single manifestation (the local |J|>K_GENESIS gate)
    rb.toggles.coupling         = coupling_on;
    rb.toggles.dual_substrate   = false;
    rb.toggles.langevin         = false;    // pure coherent wave, no thermal noise
}

std::vector<double> parse_dlist(const std::string& s) {
    std::vector<double> out;
    std::size_t i = 0;
    while (i < s.size()) {
        std::size_t j = s.find(',', i);
        if (j == std::string::npos) j = s.size();
        out.push_back(std::atof(s.substr(i, j - i).c_str()));
        i = j + 1;
    }
    return out;
}

std::vector<int> parse_ilist(const std::string& s) {
    std::vector<int> out;
    for (double d : parse_dlist(s)) out.push_back(static_cast<int>(d));
    return out;
}

// Apply the research overrides to a configured bridge.
void apply_overrides(ftd::RenderBridge& rb, double kg, double km, bool km_temp, double ramp_temp) {
    if (kg > 0.0) rb.genesis_threshold_override = kg;
    rb.manifest_use_temperature = km_temp;
    if (km > 0.0) rb.manifest_scale_override = km;
    if (km_temp) rb.toggles.langevin_T = ramp_temp;  // ramp width (thermostat may be off)
}

int manifested_count(const ftd::RenderBridge& rb) {
    int n = 0;
    for (const auto& v : rb.voxels()) if (v.state != 0) ++n;
    return n;
}

// ── Test 1: FTD-0110 ladder ratio test ──────────────────────────────────────
// Single-voxel injection at A = mult·kg; settle; measure cluster size N. If the
// N(mult) curve is the SAME across different kg, the factor-3 in K_GENESIS is a
// pure unit choice with no physical content for the mass ladder.
void run_ladder(const std::string& csv, int L, int settle, int seeds, bool force_cpu,
                int sor, bool coupling_on, const std::vector<double>& kg_list,
                const std::vector<double>& mults, double km_override, bool km_temp,
                double ramp_temp, std::uint32_t seed_base) {
    std::FILE* f = std::fopen(csv.c_str(), "w");
    if (!f) { std::fprintf(stderr, "cannot open %s\n", csv.c_str()); return; }
    std::fprintf(f, "kg,multiplier,A_inject,settle,seed,cluster_N\n");
    for (double kg : kg_list) {
        std::printf("[ladder kg=%.4f]\n", kg); std::fflush(stdout);
        for (double m : mults) {
            const double A = m * kg;
            long long sum = 0;
            for (int s = 0; s < seeds; ++s) {
                ftd::RenderBridge rb(L);
                if (force_cpu) { rb.force_cpu(); rb.set_sor_iterations(sor); }
                configure(rb, coupling_on, /*static=*/false);
                apply_overrides(rb, kg, km_override, km_temp, ramp_temp);
                rb.seed_rng(seed_base + static_cast<std::uint32_t>(s));
                rb.inject_flux(L / 2, L / 2, L / 2, {A, 0.0, 0.0});
                for (int t = 0; t < settle; ++t) rb.tick();
                const int N = manifested_count(rb);
                std::fprintf(f, "%.4f,%.2f,%.4f,%d,%d,%d\n", kg, m, A, settle, s, N);
                sum += N;
            }
            std::printf("  mult=%.1f  A=%.3f  <N>=%.1f\n", m, A, static_cast<double>(sum) / seeds);
            std::fflush(stdout);
        }
    }
    std::fclose(f);
    std::printf("wrote %s\n", csv.c_str()); std::fflush(stdout);
}

// ── Test 2: EWSB / absorbing-state transition under km=fixed vs km=T ─────────
// Pure thermal drive (Langevin T, no injection). Measures the steady-state
// manifested fraction m(T) for both ramp modes. Tells us whether tying the
// genesis ramp to the real temperature changes the transition.
void run_thermal(const std::string& csv, int L, int equil, int sample, int seeds,
                 bool force_cpu, int sor, bool coupling_on, const std::vector<double>& Ts,
                 double gamma, double km_override, std::uint32_t seed_base) {
    std::FILE* f = std::fopen(csv.c_str(), "w");
    if (!f) { std::fprintf(stderr, "cannot open %s\n", csv.c_str()); return; }
    std::fprintf(f, "ramp_mode,T,seed,manifested,m\n");
    const double N = static_cast<double>(L) * L * L;
    for (int mode = 0; mode < 2; ++mode) {
        const bool km_temp = (mode == 1);
        const char* label = km_temp ? "km=T" : "km=fixed";
        std::printf("[thermal ramp=%s]\n", label); std::fflush(stdout);
        for (double T : Ts) {
            for (int s = 0; s < seeds; ++s) {
                ftd::RenderBridge rb(L);
                if (force_cpu) { rb.force_cpu(); rb.set_sor_iterations(sor); }
                rb.toggles.disable_all();
                rb.toggles.wave_propagation = true;
                rb.toggles.gauss_projection = true;
                rb.toggles.genesis          = true;
                rb.toggles.coupling         = coupling_on;
                rb.toggles.dual_substrate   = false;
                rb.toggles.langevin         = true;
                rb.toggles.langevin_T       = T;
                rb.toggles.langevin_gamma   = gamma;
                rb.manifest_use_temperature = km_temp;          // km = T when mode 1
                if (!km_temp && km_override > 0) rb.manifest_scale_override = km_override;
                rb.seed_rng(seed_base + static_cast<std::uint32_t>(s)
                            + static_cast<std::uint32_t>(T * 1e6));
                for (int t = 0; t < equil; ++t) rb.tick();
                long long man_sum = 0;
                for (int t = 0; t < sample; ++t) { rb.tick(); man_sum += manifested_count(rb); }
                const double m = static_cast<double>(man_sum) / sample / N;
                std::fprintf(f, "%s,%.4f,%d,%lld,%.8f\n", label, T, s, man_sum / sample, m);
                std::printf("  [%s] T=%.4f seed=%d  m=%.5f\n", label, T, s, m);
                std::fflush(stdout);
            }
        }
    }
    std::fclose(f);
    std::printf("wrote %s\n", csv.c_str()); std::fflush(stdout);
}

// ── Test 3: "Does information do creative work?" ─────────────────────────────
// At FIXED total flux energy E = Σ½|J|², does the SPATIAL INFORMATION (order /
// coherence) of the disposition J govern the ORGANIZATION and SURVIVAL of
// manifested matter — beyond raw energy and beyond the |J| amplitude histogram?
// Decisive arm = permutation control: a coherent field vs the SAME field with
// its per-voxel flux Vec3 randomly permuted (identical histogram + energy, zero
// spatial structure). Genesis runs before Gauss projection, so tick-1 yield is
// fixed by the histogram; any later divergence isolates pure spatial information.

// Deterministic LCG for reproducible field construction (the physics RNG stays
// separate via rb.seed_rng). Pattern mirrors campaign_graviton_tt_correlator.
struct Lcg {
    std::uint64_t s;
    explicit Lcg(std::uint64_t seed) : s(seed ? seed : 0x9E3779B97F4A7C15ull) {}
    std::uint32_t next() { s = s * 6364136223846793005ull + 1442695040888963407ull; return static_cast<std::uint32_t>(s >> 32); }
    double uniform() { return next() / 4294967296.0; }   // [0,1)
    double sym()     { return 2.0 * uniform() - 1.0; }    // [-1,1)
};

// Rescale every flux vector so that Σ½|J|² == E_target. Returns the achieved
// energy. MUST run after the field is fully built and before the first tick().
double normalize_energy(ftd::RenderBridge& rb, double E_target) {
    const auto& lat = rb.lattice();
    const int N = static_cast<int>(rb.voxels().size());
    double E_cur = 0.0;
    for (const auto& v : rb.voxels()) E_cur += 0.5 * v.flux.mag2();
    if (E_cur < 1e-300) return 0.0;
    const double scale = std::sqrt(E_target / E_cur);
    std::vector<ftd::Vec3> scaled(N);
    { const auto& vox = rb.voxels();
      for (int i = 0; i < N; ++i) scaled[i] = ftd::Vec3{vox[i].flux.x * scale, vox[i].flux.y * scale, vox[i].flux.z * scale}; }
    for (int i = 0; i < N; ++i) { const ftd::Coord c = lat.coord(i); rb.inject_flux(c.x, c.y, c.z, scaled[i]); }
    double E_new = 0.0;
    for (const auto& v : rb.voxels()) E_new += 0.5 * v.flux.mag2();
    return E_new;
}

// Superposition of M random TRANSVERSE plane-wave modes (≈divergence-free →
// survives Gauss projection). M=1 → one random coherent mode; large M → noise.
void setup_multimode_flux(ftd::RenderBridge& rb, int L, int M, Lcg& rng) {
    const auto& lat = rb.lattice();
    const int N = L * L * L;
    std::vector<ftd::Vec3> field(N);  // default-constructs to {0,0,0}
    for (int m = 0; m < M; ++m) {
        int nx, ny, nz;
        do { nx = static_cast<int>(rng.uniform() * (L / 4));
             ny = static_cast<int>(rng.uniform() * (L / 4));
             nz = static_cast<int>(rng.uniform() * (L / 4)); } while (nx == 0 && ny == 0 && nz == 0);
        const double kx = TWO_PI * nx / L, ky = TWO_PI * ny / L, kz = TWO_PI * nz / L;
        const double kmag = std::sqrt(kx * kx + ky * ky + kz * kz);
        const double khx = kx / kmag, khy = ky / kmag, khz = kz / kmag;
        double rx = rng.sym(), ry = rng.sym(), rz = rng.sym();
        const double rdk = rx * khx + ry * khy + rz * khz;
        double ex = rx - rdk * khx, ey = ry - rdk * khy, ez = rz - rdk * khz;  // transverse component
        double em = std::sqrt(ex * ex + ey * ey + ez * ez);
        if (em < 1e-9) { ex = khy; ey = -khx; ez = 0.0; em = std::sqrt(ex * ex + ey * ey + ez * ez); if (em < 1e-9) { ex = 1; ey = 0; ez = 0; em = 1; } }
        ex /= em; ey /= em; ez /= em;
        const double phase = TWO_PI * rng.uniform();
        for (int x = 0; x < L; ++x)
            for (int y = 0; y < L; ++y)
                for (int z = 0; z < L; ++z) {
                    const double sgn = std::sin(kx * x + ky * y + kz * z + phase);
                    const int idx = lat.index(x, y, z);
                    field[idx].x += ex * sgn; field[idx].y += ey * sgn; field[idx].z += ez * sgn;
                }
    }
    for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
            for (int z = 0; z < L; ++z) { const int idx = lat.index(x, y, z); rb.inject_flux(x, y, z, field[idx]); }
}

// White noise: per-voxel i.i.d. Vec3 (NOT divergence-free — Gauss strips its
// longitudinal part; the energy-retention contrast lives here).
void setup_random_flux(ftd::RenderBridge& rb, int L, Lcg& rng) {
    for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
            for (int z = 0; z < L; ++z)
                rb.inject_flux(x, y, z, {rng.sym(), rng.sym(), rng.sym()});
}

// THE decisive primitive: random permutation of WHOLE per-voxel flux Vec3
// across voxels. Preserves the |J| histogram and Σ½|J|² EXACTLY, destroys all
// spatial structure. (Permute the Vec3 as a unit — never the components.)
void scramble_flux(ftd::RenderBridge& rb, int L, Lcg& rng) {
    const auto& lat = rb.lattice();
    const int N = L * L * L;
    std::vector<ftd::Vec3> f(N);
    { const auto& vox = rb.voxels(); for (int i = 0; i < N; ++i) f[i] = vox[i].flux; }
    for (int i = N - 1; i > 0; --i) { int j = static_cast<int>(rng.uniform() * (i + 1)); if (j > i) j = i; std::swap(f[i], f[j]); }
    for (int i = 0; i < N; ++i) { const ftd::Coord c = lat.coord(i); rb.inject_flux(c.x, c.y, c.z, f[i]); }
}

// One-shot connected-components over manifested voxels (state != 0), grouped by
// SIGN, 6-face connectivity (periodic). Returns voxel-index lists ≥ min_size for
// measure_cluster(). Snapshot organization axis (vs ClusterTracker's time axis).
std::vector<std::vector<int>> connected_components_snapshot(const ftd::RenderBridge& rb, int min_size) {
    const auto& vox = rb.voxels();
    const auto& lat = rb.lattice();
    const int N = static_cast<int>(vox.size());
    std::vector<char> visited(N, 0);
    std::vector<std::vector<int>> comps;
    std::vector<int> stack;
    for (int i = 0; i < N; ++i) {
        if (vox[i].state == 0 || visited[i]) continue;
        const int sign = vox[i].state;
        std::vector<int> comp;
        stack.clear(); stack.push_back(i); visited[i] = 1;
        while (!stack.empty()) {
            const int cur = stack.back(); stack.pop_back();
            comp.push_back(cur);
            for (int n : lat.neighbors_6(cur))
                if (!visited[n] && vox[n].state == sign) { visited[n] = 1; stack.push_back(n); }
        }
        if (static_cast<int>(comp.size()) >= min_size) comps.push_back(std::move(comp));
    }
    return comps;
}

void configure_info(ftd::RenderBridge& rb, bool coupling_on, bool gauss_on) {
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;       // the disposition must evolve
    rb.toggles.gauss_projection = gauss_on;   // ON canonical; OFF = energy-retention control
    rb.toggles.genesis          = true;       // also activates evaporation (survival test)
    rb.toggles.coupling         = coupling_on;
    rb.toggles.dual_substrate   = false;      // single-substrate genesis path
    rb.toggles.langevin         = false;      // test the DISPOSITION's info, not thermal noise
}

void run_info(const std::string& summary_csv, const std::string& traj_csv,
              int L, int settle, int seeds, bool force_cpu, int sor,
              bool coupling_on, bool gauss_on, double A_ref, double E_override,
              const std::vector<int>& modes, std::uint32_t seed_base, bool want_traj) {
    // E_target from the coherent reference (exact parity for every arm).
    double E_target = E_override;
    if (E_target <= 0.0) {
        ftd::RenderBridge tmp(L);
        if (force_cpu) tmp.force_cpu();
        setup_plane_wave(tmp, L, A_ref, 2);
        double E = 0.0; for (const auto& v : tmp.voxels()) E += 0.5 * v.flux.mag2();
        E_target = E;
    }

    std::FILE* f = std::fopen(summary_csv.c_str(), "w");
    if (!f) { std::fprintf(stderr, "cannot open %s\n", summary_csv.c_str()); return; }
    std::fprintf(f, "backend,L,settle,coupling,gauss,arm,M,seed,E_target,E0_measured,E_final,"
                    "genesis_total,evap_total,manifested_final,survival_ratio,max_void_J0,"
                    "n_clusters,largest_cluster,mean_R,total_org,Sk_peak,Sk0,Sk_peak_ratio,"
                    "alive_clusters,max_size_observed,mean_lifetime\n");
    std::FILE* ft = nullptr;
    if (want_traj) {
        ft = std::fopen(traj_csv.c_str(), "w");
        if (ft) std::fprintf(ft, "arm,M,seed,tick,genesis_events,evap_events,manifested,max_void_J,field_energy,alive_clusters,largest_cluster\n");
    }

    const char* backend = force_cpu ? "cpu" : "default";
    std::printf("[info] E_target=%.6f (coherent A_ref=%.3f)  gauss=%s coupling=%s  L=%d settle=%d seeds=%d\n",
                E_target, A_ref, gauss_on ? "on" : "off", coupling_on ? "on" : "off", L, settle, seeds);
    std::fflush(stdout);

    struct Arm { const char* label; int M; };
    std::vector<Arm> arms;
    arms.push_back({"coherent", 1});
    arms.push_back({"scrambled", 1});
    for (int M : modes) arms.push_back({"multimode", M});
    arms.push_back({"white", 0});

    for (const Arm& arm : arms) {
        const std::string lab = arm.label;
        for (int s = 0; s < seeds; ++s) {
            ftd::RenderBridge rb(L);
            if (force_cpu) { rb.force_cpu(); rb.set_sor_iterations(sor); }
            configure_info(rb, coupling_on, gauss_on);
            rb.seed_rng(seed_base + static_cast<std::uint32_t>(s));  // physics RNG identical across arms at fixed seed

            std::uint64_t ah = 1469598103934665603ull;
            for (const char* p = arm.label; *p; ++p) { ah ^= static_cast<std::uint8_t>(*p); ah *= 1099511628211ull; }
            Lcg rng(ah ^ (static_cast<std::uint64_t>(seed_base) << 1)
                       ^ (static_cast<std::uint64_t>(s) * 0x9E3779B97F4A7C15ull)
                       ^ (static_cast<std::uint64_t>(arm.M) << 17));

            if      (lab == "coherent")  setup_plane_wave(rb, L, A_ref, 2);
            else if (lab == "scrambled") setup_plane_wave(rb, L, A_ref, 2);
            else if (lab == "multimode") setup_multimode_flux(rb, L, arm.M, rng);
            else                         setup_random_flux(rb, L, rng);

            const double E0 = normalize_energy(rb, E_target);
            if (lab == "scrambled") scramble_flux(rb, L, rng);  // preserves E + histogram

            const LatticeStats s0 = measure(rb);  // tick-0: all void → max_void_J0 = global max |J|
            ftd::ClusterTracker tracker;

            long long genesis_total = 0, evap_total = 0;
            for (int t = 1; t <= settle; ++t) {
                rb.tick();
                genesis_total += rb.genesis_events_this_tick();
                evap_total    += rb.evaporation_events_this_tick();
                tracker.record(rb);
                if (ft && s == 0) {
                    const LatticeStats st = measure(rb);
                    std::fprintf(ft, "%s,%d,%d,%d,%lld,%lld,%d,%.6f,%.6f,%d,%d\n",
                                 arm.label, arm.M, s, t, rb.genesis_events_this_tick(),
                                 rb.evaporation_events_this_tick(), st.manifested, st.max_void_J,
                                 st.flux_energy, tracker.alive_count(), tracker.max_size_observed());
                }
            }

            const auto comps = connected_components_snapshot(rb, 4);
            int largest = 0; double sumNR = 0.0, sumN = 0.0;
            for (const auto& c : comps) {
                if (static_cast<int>(c.size()) > largest) largest = static_cast<int>(c.size());
                const ftd::ClusterMeasure cm = ftd::measure_cluster(rb, c);
                sumNR += cm.org; sumN += cm.size;
            }
            const double mean_R = (sumN > 0.0) ? (sumNR / sumN) : 0.0;
            const double total_org = sumNR;

            const auto G = ftd::charge_correlation(rb, L / 2);
            const auto Sk = ftd::structure_factor(G, L / 2);
            const double Sk0 = Sk.empty() ? 0.0 : Sk[0];
            double Sk_peak = 0.0; for (std::size_t k = 1; k < Sk.size(); ++k) if (Sk[k] > Sk_peak) Sk_peak = Sk[k];
            const double Sk_ratio = (std::fabs(Sk0) > 1e-12) ? (Sk_peak / Sk0) : 0.0;

            const LatticeStats sf = measure(rb);
            const double survival = (genesis_total > 0) ? static_cast<double>(sf.manifested) / static_cast<double>(genesis_total) : 0.0;

            std::fprintf(f, "%s,%d,%d,%s,%s,%s,%d,%d,%.6f,%.6f,%.6f,%lld,%lld,%d,%.6f,%.6f,%d,%d,%.6f,%.6f,%.6f,%.6f,%.6f,%d,%d,%.4f\n",
                backend, L, settle, coupling_on ? "on" : "off", gauss_on ? "on" : "off",
                arm.label, arm.M, s, E_target, E0, sf.flux_energy,
                genesis_total, evap_total, sf.manifested, survival, s0.max_void_J,
                static_cast<int>(comps.size()), largest, mean_R, total_org, Sk_peak, Sk0, Sk_ratio,
                tracker.alive_count(), tracker.max_size_observed(), tracker.mean_lifetime());

            std::printf("  [%s M=%d s=%d] N=%d surv=%.3f cls=%d max=%d R=%.3f org=%.1f Sk*=%.2f E0=%.2f Ef=%.2f mvj0=%.3f\n",
                        arm.label, arm.M, s, sf.manifested, survival, static_cast<int>(comps.size()),
                        largest, mean_R, total_org, Sk_ratio, E0, sf.flux_energy, s0.max_void_J);
            std::fflush(stdout);
        }
    }
    std::fclose(f);
    if (ft) std::fclose(ft);
    std::printf("wrote %s\n", summary_csv.c_str()); std::fflush(stdout);
}

// ── Test 4: "imagination writes form" — does matter inherit an ARBITRARY form? ─
// An obviously-chosen symbol (the letters FTD) is imprinted into the disposition
// and we ask whether the manifested matter reproduces it. Two levels:
//   L1 (template): |J| high inside the letters, 0 outside  -> matter = letters.
//   L2 (form in information): |J| UNIFORM everywhere (no form in the energy);
//      the letters are encoded ONLY in coherence (aligned flux inside, random
//      outside). Matter manifests everywhere, the energy is blank, but the
//      ORGANIZATION (local sign coherence) should still spell FTD.
// Pure genesis on a STATIC field (wave/gauss/coupling off) so the form is copied
// at maximal fidelity (no wave dispersion). Read-only / golden-neutral.

// The "imagined" form: an ASCII-art glyph ('#'/non-space/non-dot = inside),
// scaled to the lattice and extruded in z. Defaults to the block letters FTD;
// --glyph=FILE loads any other form (a wave, a spiral, an eye, ...).
std::vector<std::string> g_glyph;

void set_default_glyph() {
    g_glyph = {
        "#########  #########  ########.",
        "#########  #########  #########",
        "###......  ...###...  ###....##",
        "###......  ...###...  ###....##",
        "#######..  ...###...  ###....##",
        "#######..  ...###...  ###....##",
        "###......  ...###...  ###....##",
        "###......  ...###...  ###....##",
        "###......  ...###...  #########",
        "###......  ...###...  ########.",
    };
}

void load_glyph(const std::string& path) {
    std::ifstream in(path);
    if (!in) { std::fprintf(stderr, "cannot open glyph %s; using default\n", path.c_str()); return; }
    std::vector<std::string> lines; std::string line; std::size_t w = 0;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        w = std::max(w, line.size()); lines.push_back(line);
    }
    for (auto& l : lines) l.resize(w, ' ');
    if (!lines.empty()) g_glyph = std::move(lines);
}

bool form_FTD(int x, int y, int L) {
    if (g_glyph.empty()) set_default_glyph();
    const int GH = static_cast<int>(g_glyph.size());
    const int GW = static_cast<int>(g_glyph[0].size());
    const double scale = (0.80 * L) / std::max(GW, GH);   // fit the larger dimension
    const double gw = GW * scale, gh = GH * scale;
    const double x0 = (L - gw) * 0.5, y0 = (L - gh) * 0.5;
    const int gx = static_cast<int>((x - x0) / scale);
    const int gy = static_cast<int>((y - y0) / scale);
    if (gx < 0 || gx >= GW || gy < 0 || gy >= GH) return false;
    const char c = g_glyph[gy][gx];
    return !(c == ' ' || c == '.');
}

void run_stamp(const std::string& csv, int L, int settle, double A,
               bool force_cpu, int sor, std::uint32_t seed_base) {
    const int N = L * L * L;
    std::vector<double> mask2d(L * L, 0.0);
    for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
            mask2d[x * L + y] = form_FTD(x, y, L) ? 1.0 : 0.0;

    struct Out { std::vector<double> matter, scoh, fmag; explicit Out(int n) : matter(n, 0), scoh(n, 0), fmag(n, 0) {} };
    std::vector<Out> outs; outs.reserve(2);

    for (int level = 1; level <= 2; ++level) {
        ftd::RenderBridge rb(L);
        if (force_cpu) { rb.force_cpu(); rb.set_sor_iterations(sor); }
        rb.toggles.disable_all();
        rb.toggles.genesis        = true;    // pure genesis on a static field
        rb.toggles.dual_substrate = false;
        rb.seed_rng(seed_base);
        Lcg rng(0x0F7D0000ull ^ (static_cast<std::uint64_t>(level) << 8) ^ seed_base);

        const auto& lat = rb.lattice();
        std::vector<ftd::Vec3> field(N);
        for (int x = 0; x < L; ++x)
            for (int y = 0; y < L; ++y) {
                const bool inside = form_FTD(x, y, L);
                for (int z = 0; z < L; ++z) {
                    ftd::Vec3 J;  // zero
                    if (level == 1) {
                        if (inside) J = ftd::Vec3{0.0, A, 0.0};        // magnitude template; outside 0
                    } else {
                        if (inside) {
                            J = ftd::Vec3{0.0, A, 0.0};                // aligned (coherent) inside
                        } else {
                            double rx = rng.sym(), ry = rng.sym(), rz = rng.sym();
                            double m = std::sqrt(rx * rx + ry * ry + rz * rz);
                            if (m < 1e-9) { rx = 1; ry = 0; rz = 0; m = 1; }
                            J = ftd::Vec3{A * rx / m, A * ry / m, A * rz / m};  // random dir, SAME |J|=A
                        }
                    }
                    field[lat.index(x, y, z)] = J;
                }
            }
        for (int x = 0; x < L; ++x)
            for (int y = 0; y < L; ++y)
                for (int z = 0; z < L; ++z) rb.inject_flux(x, y, z, field[lat.index(x, y, z)]);

        for (int t = 0; t < settle; ++t) rb.tick();

        Out o(L * L);
        const auto& vox = rb.voxels();
        for (int x = 0; x < L; ++x)
            for (int y = 0; y < L; ++y) {
                double m = 0, sc = 0, fm = 0;
                for (int z = 0; z < L; ++z) {
                    const int idx = lat.index(x, y, z);
                    const auto& v = vox[idx];
                    m  += (v.state != 0) ? 1.0 : 0.0;
                    fm += v.flux.mag();
                    if (v.state != 0) {  // local sign coherence: same-sign fraction over manifested 26-nbrs
                        int same = 0, tot = 0;
                        for (int n : lat.neighbors_26(idx))
                            if (vox[n].state != 0) { ++tot; if (vox[n].state == v.state) ++same; }
                        sc += (tot > 0) ? static_cast<double>(same) / tot : 0.0;
                    }
                }
                o.matter[x * L + y] = m / L; o.scoh[x * L + y] = sc / L; o.fmag[x * L + y] = fm / L;
            }
        outs.push_back(std::move(o));
        std::printf("  [stamp L%d] built + %d ticks done\n", level, settle); std::fflush(stdout);
    }

    std::FILE* f = std::fopen(csv.c_str(), "w");
    if (!f) { std::fprintf(stderr, "cannot open %s\n", csv.c_str()); return; }
    std::fprintf(f, "x,y,mask,L1_matter,L1_scoh,L1_fmag,L2_matter,L2_scoh,L2_fmag\n");
    for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y) {
            const int k = x * L + y;
            std::fprintf(f, "%d,%d,%.3f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\n",
                         x, y, mask2d[k], outs[0].matter[k], outs[0].scoh[k], outs[0].fmag[k],
                         outs[1].matter[k], outs[1].scoh[k], outs[1].fmag[k]);
        }
    std::fclose(f);
    std::printf("wrote %s  (FTD stamp, L=%d settle=%d A=%.2f)\n", csv.c_str(), L, settle, A);
    std::fflush(stdout);
}

} // namespace

int main(int argc, char** argv) {
    int L = 32;
    int ticks = 60;
    int seeds = 4;
    std::string nodes_str = "2,4,8";
    std::string amps_str =
        "0.8,1.0,1.2,1.4,1.45,1.5,1.52,1.533,1.55,1.6,1.7,1.85,2.0,2.25,2.5,2.75,3.0";
    double traj_amp = 2.5;
    bool coupling_on = true;
    bool static_field = false;
    bool force_cpu = false;
    int sor_iters = 100;
    std::uint32_t seed_base = 0x9E150000u;
    std::string tag = "amp";
    std::string output_dir = "engine/results/genesis_amplitude_ceiling/";

    // Research overrides (Test the K_GENESIS=N_c·K_MANIFEST and K_MANIFEST=W_SC
    // choices; compile-time values per FTD-0388 since 2026-07-17).
    double kgenesis_override  = -1.0;   // <=0 ⇒ compile-time K_GENESIS (1.516386)
    double kmanifest_override = -1.0;   // <=0 ⇒ compile-time K_MANIFEST (W_SC = 0.505462)
    bool   kmanifest_temp     = false;  // ramp scale = temperature instead of mass
    double ramp_temp          = 0.0;    // the "temperature" value when --kmanifest-temp
    bool   ladder_mode        = false;  // FTD-0110 ratio test (single-voxel injection)
    bool   thermal_mode       = false;  // EWSB / absorbing-state transition (Langevin T)
    std::string kg_list_str   = "0.511,1.533";   // ladder: kg values to compare
    std::string mults_str     = "2,4,6,8,10,12";  // ladder: A = mult·kg
    std::string Ts_str        = "0.02,0.04,0.06,0.08,0.10,0.12,0.14,0.16";  // thermal
    int    equil              = 600;    // thermal equilibration ticks
    int    sample             = 300;    // thermal sampling ticks
    double gamma              = 0.02;   // thermal Langevin friction

    // Test 3 — "does information do creative work?" (info-vs-manifestation)
    bool   info_mode          = false;
    std::string modes_str     = "1,2,4,8,16,32";  // mode-count order axis
    double A_ref              = 2.0;    // reference coherent amplitude (above K_GENESIS=1.533)
    double info_energy        = -1.0;   // explicit fixed total energy (<=0 ⇒ from A_ref coherent)
    int    settle             = 300;    // info settle/measurement ticks
    bool   gauss_on           = true;   // gauss projection (--gauss=off energy-retention control)
    bool   info_traj          = false;  // also emit per-tick trajectory (seed 0)

    // Test 4 — "imagination writes form" (does matter inherit an arbitrary form)
    bool   stamp_mode         = false;
    double stamp_A            = 2.0;    // imprint amplitude (above K_GENESIS=1.533)
    std::string glyph_path;             // --glyph=FILE : ASCII-art form (default FTD)

    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if      (a.rfind("--L=", 0) == 0)            L = std::atoi(a.c_str() + 4);
        else if (a.rfind("--ticks=", 0) == 0)        ticks = std::atoi(a.c_str() + 8);
        else if (a.rfind("--seeds=", 0) == 0)        seeds = std::atoi(a.c_str() + 8);
        else if (a.rfind("--nodes=", 0) == 0)        nodes_str = a.substr(8);
        else if (a.rfind("--amps=", 0) == 0)         amps_str = a.substr(7);
        else if (a.rfind("--traj-amp=", 0) == 0)     traj_amp = std::atof(a.c_str() + 11);
        else if (a == "--coupling=off")              coupling_on = false;
        else if (a == "--coupling=on")               coupling_on = true;
        else if (a == "--static")                    static_field = true;
        else if (a.rfind("--kgenesis=", 0) == 0)     kgenesis_override  = std::atof(a.c_str() + 11);
        else if (a.rfind("--kmanifest=", 0) == 0)    kmanifest_override = std::atof(a.c_str() + 12);
        else if (a == "--kmanifest-temp")            kmanifest_temp     = true;
        else if (a.rfind("--ramp-temp=", 0) == 0)    ramp_temp          = std::atof(a.c_str() + 12);
        else if (a == "--ladder")                    ladder_mode        = true;
        else if (a == "--thermal")                   thermal_mode       = true;
        else if (a.rfind("--kg-list=", 0) == 0)      kg_list_str        = a.substr(10);
        else if (a.rfind("--mults=", 0) == 0)        mults_str          = a.substr(8);
        else if (a.rfind("--Ts=", 0) == 0)           Ts_str             = a.substr(5);
        else if (a.rfind("--equil=", 0) == 0)        equil              = std::atoi(a.c_str() + 8);
        else if (a.rfind("--sample=", 0) == 0)       sample             = std::atoi(a.c_str() + 9);
        else if (a.rfind("--gamma=", 0) == 0)        gamma              = std::atof(a.c_str() + 8);
        else if (a == "--info")                      info_mode          = true;
        else if (a.rfind("--modes=", 0) == 0)        modes_str          = a.substr(8);
        else if (a.rfind("--A-ref=", 0) == 0)        A_ref              = std::atof(a.c_str() + 8);
        else if (a.rfind("--info-energy=", 0) == 0)  info_energy        = std::atof(a.c_str() + 14);
        else if (a.rfind("--settle=", 0) == 0)       settle             = std::atoi(a.c_str() + 9);
        else if (a == "--gauss=off")                 gauss_on           = false;
        else if (a == "--gauss=on")                  gauss_on           = true;
        else if (a == "--info-traj")                 info_traj          = true;
        else if (a == "--stamp")                     stamp_mode         = true;
        else if (a.rfind("--stamp-A=", 0) == 0)      stamp_A            = std::atof(a.c_str() + 10);
        else if (a.rfind("--glyph=", 0) == 0)        glyph_path         = a.substr(8);
        else if (a == "--cpu")                       force_cpu = true;
        else if (a.rfind("--sor=", 0) == 0)          sor_iters = std::atoi(a.c_str() + 6);
        else if (a.rfind("--seed-base=", 0) == 0)    seed_base = static_cast<std::uint32_t>(std::strtoul(a.c_str() + 12, nullptr, 0));
        else if (a.rfind("--tag=", 0) == 0)          tag = a.substr(6);
        else if (a.rfind("--output-dir=", 0) == 0)   output_dir = a.substr(13);
    }

    const std::vector<int>    nodes = parse_ilist(nodes_str);
    const std::vector<double> amps  = parse_dlist(amps_str);

    fs::create_directories(output_dir);

    // Announce the active genesis constants (compile-time + any override).
    std::printf("genesis_amplitude_ceiling\n");
    std::printf("  compile-time:  K_GENESIS=%.4f  K_MANIFEST=%.4f  K_B=%.4f  N_c=%d\n",
                ftd::K_GENESIS, ftd::K_MANIFEST, ftd::K_B, static_cast<int>(ftd::N_C));
    if (kgenesis_override  > 0) std::printf("  OVERRIDE: kgenesis  = %.4f\n", kgenesis_override);
    if (kmanifest_override > 0) std::printf("  OVERRIDE: kmanifest = %.4f\n", kmanifest_override);
    if (kmanifest_temp)         std::printf("  OVERRIDE: ramp scale = temperature (= %.4f)\n", ramp_temp);
    std::fflush(stdout);

    // ── Mode dispatch: ladder (Test 1) / thermal (Test 2) / amplitude sweep ──
    if (ladder_mode) {
        const fs::path csv = fs::path(output_dir) / ("ladder_" + tag + ".csv");
        run_ladder(csv.string(), L, ticks, seeds, force_cpu, sor_iters, coupling_on,
                   parse_dlist(kg_list_str), parse_dlist(mults_str),
                   kmanifest_override, kmanifest_temp, ramp_temp, seed_base);
        return 0;
    }
    if (thermal_mode) {
        const fs::path csv = fs::path(output_dir) / ("thermal_" + tag + ".csv");
        run_thermal(csv.string(), L, equil, sample, seeds, force_cpu, sor_iters, coupling_on,
                    parse_dlist(Ts_str), gamma, kmanifest_override, seed_base);
        return 0;
    }
    if (info_mode) {
        const fs::path s = fs::path(output_dir) / ("info_"      + tag + ".csv");
        const fs::path t = fs::path(output_dir) / ("info_traj_" + tag + ".csv");
        run_info(s.string(), t.string(), L, settle, seeds, force_cpu, sor_iters,
                 coupling_on, gauss_on, A_ref, info_energy, parse_ilist(modes_str),
                 seed_base, info_traj);
        return 0;
    }
    if (stamp_mode) {
        if (!glyph_path.empty()) load_glyph(glyph_path);
        const fs::path csv = fs::path(output_dir) / ("stamp_" + tag + ".csv");
        run_stamp(csv.string(), L, settle, stamp_A, force_cpu, sor_iters, seed_base);
        return 0;
    }

    const fs::path sweep_csv = fs::path(output_dir) / ("sweep_" + tag + ".csv");
    const fs::path traj_csv  = fs::path(output_dir) / ("traj_"  + tag + ".csv");

    std::FILE* fs_sweep = std::fopen(sweep_csv.string().c_str(), "w");
    if (!fs_sweep) { std::fprintf(stderr, "cannot open %s\n", sweep_csv.string().c_str()); return 1; }
    std::fprintf(fs_sweep,
        "backend,mode,L,ticks,coupling,nodes,lambda_voxels,amplitude,seed,"
        "genesis_total,manifested_final,max_void_J_final,flux_energy_final\n");

    std::printf("genesis_amplitude_ceiling\n");
    std::printf("  K_GENESIS  = %.6f  (= N_c*K_MANIFEST = 3*W_SC, FTD-0388; the predicted cliff)\n", ftd::K_GENESIS);
    std::printf("  K_MANIFEST = %.6f  (probability ramp scale above the cliff)\n", ftd::K_MANIFEST);
    std::printf("  K_B        = %.6f\n", ftd::K_B);
    std::printf("  C_SPEED    = %.6f\n", ftd::C_SPEED);
    std::printf("  L=%d ticks=%d seeds=%d nodes=%s mode=%s coupling=%s backend=%s\n",
                L, ticks, seeds, nodes_str.c_str(),
                static_field ? "static" : "wave",
                coupling_on ? "on" : "off", force_cpu ? "cpu" : "default");
    std::fflush(stdout);

    const char* backend = force_cpu ? "cpu" : "default";
    const char* mode = static_field ? "static" : "wave";

    for (int n : nodes) {
        const double lambda = static_cast<double>(L) / static_cast<double>(n);
        std::printf("[nodes=%d  lambda=%.2f voxels]\n", n, lambda);
        std::fflush(stdout);
        for (double A : amps) {
            long long gsum_over_seeds = 0;
            for (int s = 0; s < seeds; ++s) {
                ftd::RenderBridge rb(L);
                if (force_cpu) { rb.force_cpu(); rb.set_sor_iterations(sor_iters); }
                configure(rb, coupling_on, static_field);
                apply_overrides(rb, kgenesis_override, kmanifest_override, kmanifest_temp, ramp_temp);
                rb.seed_rng(seed_base + static_cast<std::uint32_t>(s)
                            + static_cast<std::uint32_t>(n * 1000));
                setup_plane_wave(rb, L, A, n);

                long long genesis_total = 0;
                for (int t = 0; t < ticks; ++t) {
                    rb.tick();
                    genesis_total += rb.genesis_events_this_tick();
                }
                const LatticeStats st = measure(rb);
                gsum_over_seeds += genesis_total;

                std::fprintf(fs_sweep,
                    "%s,%s,%d,%d,%s,%d,%.4f,%.4f,%d,%lld,%d,%.6f,%.6f\n",
                    backend, mode, L, ticks, coupling_on ? "on" : "off",
                    n, lambda, A, s,
                    genesis_total, st.manifested, st.max_void_J, st.flux_energy);
            }
            std::printf("  A=%.4f  <genesis/seed>=%.2f\n",
                        A, static_cast<double>(gsum_over_seeds) / seeds);
            std::fflush(stdout);
        }
    }
    std::fclose(fs_sweep);
    std::printf("wrote %s\n", sweep_csv.string().c_str());
    std::fflush(stdout);

    // ---- Trajectory: one super-threshold amplitude, watch the wave break ----
    const int traj_nodes = nodes.empty() ? 4 : nodes.front();
    std::FILE* ft = std::fopen(traj_csv.string().c_str(), "w");
    if (ft) {
        std::fprintf(ft, "tick,genesis_events,manifested,max_void_J,flux_energy\n");
        ftd::RenderBridge rb(L);
        if (force_cpu) { rb.force_cpu(); rb.set_sor_iterations(sor_iters); }
        configure(rb, coupling_on, static_field);
        apply_overrides(rb, kgenesis_override, kmanifest_override, kmanifest_temp, ramp_temp);
        rb.seed_rng(seed_base + 0xABCDu);
        setup_plane_wave(rb, L, traj_amp, traj_nodes);

        const LatticeStats s0 = measure(rb);
        std::fprintf(ft, "%d,%lld,%d,%.6f,%.6f\n", 0, 0LL, s0.manifested, s0.max_void_J, s0.flux_energy);
        for (int t = 1; t <= ticks; ++t) {
            rb.tick();
            const LatticeStats st = measure(rb);
            std::fprintf(ft, "%d,%lld,%d,%.6f,%.6f\n",
                         t, rb.genesis_events_this_tick(), st.manifested, st.max_void_J, st.flux_energy);
        }
        std::fclose(ft);
        std::printf("wrote %s  (trajectory at A=%.3f, nodes=%d)\n",
                    traj_csv.string().c_str(), traj_amp, traj_nodes);
        std::fflush(stdout);
    }

    return 0;
}
