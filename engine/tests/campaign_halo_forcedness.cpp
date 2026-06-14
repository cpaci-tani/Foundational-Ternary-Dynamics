/**
 * @file campaign_halo_forcedness.cpp
 * @brief FTD-0300: is the single-particle self-field HALO EXPONENT forced by the
 *        dynamics, or tuned by engine calibration constants?
 *
 * The engine's self-field halo flux falls as |J|(r) ~ r^p (the canonical GPU
 * L=128 value reported in DERIV_DARK_SECTOR_DYNAMICS.md §4.1 is p ≈ -0.69, fit
 * over r∈[7,23]). This is the SHAPE that any downstream dark-matter rotation
 * curve would inherit. FTD-0269 showed the N(A) law's *shape* was forced but its
 * *calibration* was tuned (kinetic drain, γ) — a [BOUNDARY]. This campaign asks
 * the same question of the halo exponent.
 *
 * METHODOLOGY (mirrors test_gpu_shell_battery's halo setup):
 *   Deterministic arm (genesis+movement OFF, locked +1 particle, no Langevin):
 *     bit-deterministic, no seeds. inject_particle(C,C,C,+1,{0,0,K_B}) then lock
 *     the center voxel, run --ticks, measure the radial |J|(r) profile.
 *     Sweeps --Ls × --selective × --stencil.
 *   Langevin arm (--arm=langevin): genesis+langevin ON, supercritical injection
 *     A·K_GENESIS; the ONLY arm where kinetic_drain is active (drain fires inside
 *     the genesis branch). Seeded. Sweeps --kinetic-drain × seeds.
 *
 * The VERDICT reads only the EXPONENT (computed in Python by
 * analyze_halo_forcedness.py over the frozen window r∈[7,23]); halo AMPLITUDE
 * (norm, J_peak, E_field) is report-only — that excludes the amplitude/shape
 * confound. The console exponent printed here is a convenience echo, not the
 * frozen verdict. Each row records ftd::DAMPING/G_C/K_B so compile-time-swept
 * builds (run_halo_constant_sweeps.py) are self-identifying.
 *
 * OBSERVATION-ONLY: read-only measurement campaign; changes no physics. At the
 * default constants it does not affect the golden gate (new TU, no edits to any
 * phase_*.cpp/kernel/constant default).
 *
 * Output (engine/results/halo_forcedness/):
 *   halo_forcedness_<tag>.csv         — one row per cell (summary)
 *   halo_forcedness_shells_<tag>.csv  — long-form per-shell ⟨|J|⟩(r)
 *
 * Usage:
 *   campaign_halo_forcedness --arm=det --Ls=64,96,128,160 --selective=on \
 *       --stencil=full --ticks=4000 --cpu --sor=150 --knob=L --tag=v1
 *   campaign_halo_forcedness --arm=langevin --L=128 --gamma=0.02 --T=0.005 \
 *       --A=10 --kinetic-drain=0.25,0.5,0.75 --seeds=5 --ticks=2000 --cpu --tag=v1
 */

#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/sublattice.h"
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

constexpr int   FIT_LO = 7;    // frozen radial-fit window (mirrors the doc r∈[7,23])
constexpr int   FIT_HI = 23;
constexpr int   MAX_R  = 90;   // covers L=160 (half-diagonal of the central axis)

template <typename T>
std::vector<T> parse_list(const std::string& s) {
    std::vector<T> out; std::size_t i = 0;
    while (i < s.size()) {
        std::size_t j = s.find(',', i); if (j == std::string::npos) j = s.size();
        out.push_back(static_cast<T>(std::atof(s.substr(i, j - i).c_str()))); i = j + 1;
    }
    return out;
}

std::vector<std::string> parse_str_list(const std::string& s) {
    std::vector<std::string> out; std::size_t i = 0;
    while (i < s.size()) {
        std::size_t j = s.find(',', i); if (j == std::string::npos) j = s.size();
        out.push_back(s.substr(i, j - i)); i = j + 1;
    }
    return out;
}

ftd::BccStencilMode stencil_from(const std::string& s) {
    if (s == "sc")  return ftd::BccStencilMode::SC;
    if (s == "fcc") return ftd::BccStencilMode::FCC;
    if (s == "bcc") return ftd::BccStencilMode::BCC;
    return ftd::BccStencilMode::FULL;
}

// Radial profile of |J| around (cx,cy,cz): per integer shell, the mean flux
// magnitude, the site count, and the shell field energy. Plus summary scalars.
struct Profile {
    std::vector<double> avg_J;        // [0..MAX_R]
    std::vector<double> energy_at_r;  // [0..MAX_R] = Σ ½|J|²
    std::vector<long>   count;        // [0..MAX_R]
    double r_eff = 0.0, J_peak = 0.0, E_field = 0.0;
    int    r_shell = MAX_R;           // 1% of ⟨|J|⟩(r=1) boundary
    // log-log LSQ of ⟨|J|⟩ ~ norm·r^exponent over [FIT_LO, FIT_HI]
    double norm = 0.0, exponent = 0.0, r2 = 0.0;
};

Profile radial_profile(const std::vector<ftd::Voxel>& v, int L, int cx, int cy, int cz) {
    Profile p;
    p.avg_J.assign(MAX_R + 1, 0.0);
    p.energy_at_r.assign(MAX_R + 1, 0.0);
    p.count.assign(MAX_R + 1, 0);
    std::vector<double> flux_sum(MAX_R + 1, 0.0);
    double sum_r2_j2 = 0.0, sum_j2 = 0.0, total_E = 0.0, J_peak = 0.0;

    for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
    for (int z = 0; z < L; ++z) {
        const double dx = x - cx, dy = y - cy, dz = z - cz;
        const double r2 = dx*dx + dy*dy + dz*dz;
        const int ri = static_cast<int>(std::lround(std::sqrt(r2)));
        const int idx = x*L*L + y*L + z;
        const double j2 = v[idx].flux.mag2();
        const double jmag = std::sqrt(j2);
        if (ri <= MAX_R) {
            flux_sum[ri] += jmag;
            p.energy_at_r[ri] += 0.5 * j2;
            p.count[ri]++;
        }
        sum_r2_j2 += r2 * j2;
        sum_j2 += j2;
        total_E += 0.5 * j2;
        if (jmag > J_peak) J_peak = jmag;
    }

    p.J_peak = J_peak;
    p.E_field = total_E;
    p.r_eff = (sum_j2 > 1e-30) ? std::sqrt(sum_r2_j2 / sum_j2) : 0.0;
    for (int r = 0; r <= MAX_R; ++r)
        if (p.count[r] > 0) p.avg_J[r] = flux_sum[r] / p.count[r];

    if (p.avg_J[1] > 0) {
        const double thr = 0.01 * p.avg_J[1];
        for (int r = 2; r <= MAX_R; ++r) if (p.avg_J[r] < thr) { p.r_shell = r; break; }
    }

    // log-log fit over the frozen window
    std::vector<double> xs, ys;
    for (int r = FIT_LO; r <= FIT_HI && r <= MAX_R; ++r)
        if (p.count[r] > 0 && p.avg_J[r] > 0) {
            xs.push_back(std::log((double)r));
            ys.push_back(std::log(p.avg_J[r]));
        }
    if (xs.size() >= 2) {
        const int n = (int)xs.size();
        double sx = 0, sy = 0, sxx = 0, sxy = 0;
        for (int i = 0; i < n; ++i) { sx += xs[i]; sy += ys[i]; sxx += xs[i]*xs[i]; sxy += xs[i]*ys[i]; }
        const double denom = n*sxx - sx*sx;
        if (std::fabs(denom) > 1e-30) {
            p.exponent = (n*sxy - sx*sy) / denom;
            const double b = (sy - p.exponent*sx) / n;
            p.norm = std::exp(b);
            const double ybar = sy / n;
            double ss_tot = 0, ss_res = 0;
            for (int i = 0; i < n; ++i) {
                ss_tot += (ys[i]-ybar)*(ys[i]-ybar);
                const double yhat = b + p.exponent*xs[i];
                ss_res += (ys[i]-yhat)*(ys[i]-yhat);
            }
            p.r2 = (ss_tot > 0) ? 1.0 - ss_res/ss_tot : 0.0;
        }
    }
    return p;
}

} // namespace

int main(int argc, char** argv) {
    std::string arm = "det";
    int L = 128, ticks = 4000, seeds = 1, sor = 150;
    std::string Ls_str, selective_str = "on", stencil_str = "full";
    std::string drains_str = "0.5";
    double charge_coupling = 1.0, gamma = 0.02, T = 0.005, A = 10.0;
    bool force_cpu = false;
    std::string toggleset = "minimal";   // minimal | full(canonical shell-battery)
    bool absorbing = false;              // absorbing_boundary: lossless field exits box vs wraps
    std::string knob = "baseline", knob_value = "", tag = "v1";
    std::string output_dir = "engine/results/halo_forcedness/";
    std::uint32_t seed_base = 0x4A100000u;

    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if      (a.rfind("--arm=", 0) == 0)            arm = a.substr(6);
        else if (a.rfind("--L=", 0) == 0)              L = std::atoi(a.c_str() + 4);
        else if (a.rfind("--Ls=", 0) == 0)             Ls_str = a.substr(5);
        else if (a.rfind("--ticks=", 0) == 0)          ticks = std::atoi(a.c_str() + 8);
        else if (a.rfind("--selective=", 0) == 0)      selective_str = a.substr(12);
        else if (a.rfind("--stencil=", 0) == 0)        stencil_str = a.substr(10);
        else if (a.rfind("--toggles=", 0) == 0)        toggleset = a.substr(10);
        else if (a.rfind("--absorbing=", 0) == 0)      absorbing = (a.substr(12) == "on");
        else if (a.rfind("--charge-coupling=", 0) == 0) charge_coupling = std::atof(a.c_str() + 18);
        else if (a.rfind("--kinetic-drain=", 0) == 0)  drains_str = a.substr(16);
        else if (a.rfind("--gamma=", 0) == 0)          gamma = std::atof(a.c_str() + 8);
        else if (a.rfind("--T=", 0) == 0)              T = std::atof(a.c_str() + 4);
        else if (a.rfind("--A=", 0) == 0)              A = std::atof(a.c_str() + 4);
        else if (a.rfind("--seeds=", 0) == 0)          seeds = std::atoi(a.c_str() + 8);
        else if (a.rfind("--sor=", 0) == 0)            sor = std::atoi(a.c_str() + 6);
        else if (a == "--cpu")                         force_cpu = true;
        else if (a.rfind("--knob=", 0) == 0)           knob = a.substr(7);
        else if (a.rfind("--value=", 0) == 0)          knob_value = a.substr(8);
        else if (a.rfind("--tag=", 0) == 0)            tag = a.substr(6);
        else if (a.rfind("--output-dir=", 0) == 0)     output_dir = a.substr(13);
    }

    std::vector<int> Ls;
    if (!Ls_str.empty()) for (double d : parse_list<double>(Ls_str)) Ls.push_back((int)d);
    else Ls.push_back(L);
    const std::vector<std::string> selectives = parse_str_list(selective_str);
    const std::vector<std::string> stencils   = parse_str_list(stencil_str);
    const std::vector<double> drains          = parse_list<double>(drains_str);

    fs::create_directories(output_dir);
    const fs::path sum_csv   = fs::path(output_dir) / ("halo_forcedness_" + tag + ".csv");
    const fs::path shell_csv = fs::path(output_dir) / ("halo_forcedness_shells_" + tag + ".csv");
    // append if present so multi-invocation sweeps (run_halo_constant_sweeps.py) accumulate
    const bool sum_exists = fs::exists(sum_csv);
    std::FILE* fs_ = std::fopen(sum_csv.string().c_str(), "a");
    std::FILE* fl_ = std::fopen(shell_csv.string().c_str(), "a");
    if (!fs_ || !fl_) { std::fprintf(stderr, "cannot open output CSVs in %s\n", output_dir.c_str()); return 1; }
    if (!sum_exists) {
        std::fprintf(fs_, "arm,knob,value,L,backend,ticks,seed,selective,stencil,"
                          "toggleset,absorbing,charge_coupling,kinetic_drain,"
                          "DAMPING,G_C,K_B,K_GENESIS,"
                          "norm,exponent,r2,J_peak,r_eff,r_shell,E_field\n");
        std::fprintf(fl_, "arm,knob,value,L,seed,r,avg_J,n_sites,energy_at_r\n");
    }
    std::fflush(fs_); std::fflush(fl_);

    const char* backend = force_cpu ? "cpu" : "default";
    std::printf("halo_forcedness: arm=%s Ls=%s ticks=%d selective=%s stencil=%s "
                "backend=%s sor=%d knob=%s DAMPING=%.6g G_C=%.6g K_B=%.6g\n",
                arm.c_str(), Ls_str.empty() ? std::to_string(L).c_str() : Ls_str.c_str(),
                ticks, selective_str.c_str(), stencil_str.c_str(), backend, sor,
                knob.c_str(), (double)ftd::DAMPING, (double)ftd::G_C, (double)ftd::K_B);
    std::fflush(stdout);

    auto emit = [&](const std::string& a_name, int Lc, int seed,
                    const std::string& sel, const std::string& st,
                    double drain, const Profile& p) {
        std::fprintf(fs_,
            "%s,%s,%s,%d,%s,%d,%d,%s,%s,%s,%d,%.6g,%.4g,%.8g,%.8g,%.8g,%.8g,"
            "%.8g,%.6f,%.6f,%.8g,%.4f,%d,%.8g\n",
            a_name.c_str(), knob.c_str(), knob_value.c_str(), Lc, backend, ticks, seed,
            sel.c_str(), st.c_str(), toggleset.c_str(), absorbing ? 1 : 0,
            charge_coupling, drain,
            (double)ftd::DAMPING, (double)ftd::G_C, (double)ftd::K_B, (double)ftd::K_GENESIS,
            p.norm, p.exponent, p.r2, p.J_peak, p.r_eff, p.r_shell, p.E_field);
        std::fflush(fs_);
        for (int r = 1; r <= MAX_R && r <= Lc/2; ++r) {
            if (p.count[r] <= 0) continue;
            std::fprintf(fl_, "%s,%s,%s,%d,%d,%d,%.10g,%ld,%.10g\n",
                a_name.c_str(), knob.c_str(), knob_value.c_str(), Lc, seed,
                r, p.avg_J[r], p.count[r], p.energy_at_r[r]);
        }
        std::fflush(fl_);
        std::printf("  %-8s L=%-3d sel=%-3s st=%-4s drain=%.3f seed=%-2d  "
                    "p=%+.4f  R²=%.4f  norm=%.4g  r_eff=%.2f  r_sh=%d\n",
                    a_name.c_str(), Lc, sel.c_str(), st.c_str(), drain, seed,
                    p.exponent, p.r2, p.norm, p.r_eff, p.r_shell);
        std::fflush(stdout);
    };

    if (arm == "langevin") {
        // Genesis+Langevin arm — the only place kinetic_drain is active.
        for (double drain : drains) {
            for (int s = 0; s < seeds; ++s) {
                const int C = L / 2;
                ftd::RenderBridge rb(L);
                if (force_cpu) { rb.force_cpu(); rb.set_sor_iterations(sor); }
                rb.toggles.disable_all();
                rb.toggles.wave_propagation = true;
                rb.toggles.gauss_projection = true;
                rb.toggles.genesis          = true;
                rb.toggles.coupling         = true;
                rb.toggles.dual_substrate   = false;
                rb.toggles.langevin         = true;
                rb.toggles.langevin_gamma   = gamma;
                rb.toggles.langevin_T       = T;
                rb.toggles.kinetic_drain    = drain;
                rb.toggles.bcc_stencil      = stencil_from(stencils.front());
                rb.toggles.coulomb_charge_coupling = charge_coupling;
                rb.seed_rng(seed_base + (std::uint32_t)s * 2654435761u);
                rb.inject_flux(C, C, C, {A * ftd::K_GENESIS, 0, 0});
                rb.run(ticks);
                const Profile p = radial_profile(rb.voxels(), L, C, C, C);
                emit("langevin", L, s, selectives.front(), stencils.front(), drain, p);
            }
        }
    } else {
        // Deterministic locked-particle arm (bit-deterministic; seed = -1).
        for (int Lc : Ls) {
            const int C = Lc / 2;
            for (const std::string& sel : selectives) {
                for (const std::string& st : stencils) {
                    ftd::RenderBridge rb(Lc);
                    if (force_cpu) { rb.force_cpu(); rb.set_sor_iterations(sor); }
                    if (toggleset == "full") {        // canonical shell-battery stack
                        rb.toggles.enable_all();
                        rb.toggles.genesis  = false;
                        rb.toggles.movement = false;
                    } else {                          // minimal: only |J|-building physics
                        rb.toggles.disable_all();     // (forces/Poisson/weak do not touch |J|)
                        rb.toggles.wave_propagation = true;
                        rb.toggles.coupling         = true;
                        rb.toggles.damping          = true;
                        rb.toggles.gauss_projection = true;
                        rb.toggles.dual_substrate   = true;
                    }
                    rb.toggles.selective_damping  = (sel == "on");
                    rb.toggles.absorbing_boundary = absorbing;
                    rb.toggles.bcc_stencil = stencil_from(st);
                    rb.toggles.coulomb_charge_coupling = charge_coupling;
                    rb.inject_particle(C, C, C, +1, {0, 0, ftd::K_B}, 0, 0);
                    rb.voxel_at(C, C, C).locked = true;
                    rb.run(ticks);
                    const Profile p = radial_profile(rb.voxels(), Lc, C, C, C);
                    emit("det", Lc, -1, sel, st, /*drain=*/0.5, p);
                }
            }
        }
    }

    std::fclose(fs_); std::fclose(fl_);
    std::printf("wrote %s and %s\n", sum_csv.string().c_str(), shell_csv.string().c_str());
    return 0;
}
