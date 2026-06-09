/**
 * @file campaign_time_dilation.cpp
 * @brief CAMPAIGN 2 — Dynamical time dilation: does a moving lattice clock
 *        dilate as √(1−v²) [L²/γ] or 1−v [L¹/FTD-0208]?
 *
 * Arc: .claude/plans/plan-an-intuitive-path-twinkling-gizmo.md (Campaign 2).
 *
 * THE QUESTION. FTD-0208 [CLOSED NEGATIVE] proved the discrete *single-event*
 * budget is linear (L¹): v + dτ/dt ≤ 1 ⇒ dτ/dt = 1−v. The wave-dispersion
 * argument says the *coherent wave* dynamics is second-order, so the dispersion
 * is a sum of squares (L²): ω² = c²k² + m² ⇒ dτ/dt = √(1−v²) (γ) in the IR.
 * These differ by 36% at v=0.5. This campaign measures which law a REAL counted
 * oscillation obeys — refining FTD-0208, not re-deriving γ.
 *
 * HARD DISCIPLINE. NEVER reads `voxel.tau` (it hardcodes √(f²−v²)/√f → circular,
 * the F6/F10 trap). This is pure bare-wave dynamics, fully deterministic.
 *
 * WAVE CLOCK (this file). The lattice flux is massless. An effective rest-mass
 * comes from a fixed TRANSVERSE wavevector k⊥: factoring J ∝ e^{i k⊥·r⊥}·φ(z,t)
 * reduces the 3D massless wave to a 1+1D massive (Klein-Gordon-like) field φ
 * along the motion axis, with m_eff = c·k⊥ and rest frequency ω₀ = ω(k_z=0).
 * For a mode k = n_z·motion + n⊥·transverse we measure the lattice frequency
 * ω(k) by the single-tick Rayleigh-quotient eigenvalue (the campaign_dispersion
 * method, generalized to arbitrary k), the group velocity v_g = dω/dk_z by
 * central difference, and the co-moving (proper) frequency ω_proper = ω − k_z·v_g.
 * Dilation = ω_proper/ω₀; velocity v = v_g/C_WAVE (c_lattice = C_WAVE = 1/√3).
 * Predictions: L²/γ = √(1−v²); L¹ = 1−v. The L²/L¹ VERDICT is drawn post-lock
 * in scripts/exploration/analyze_time_dilation.py — the checks here are
 * SANITY-only (ran correctly, data is physically sane), to preserve the
 * pre-registration (PREREG_DYNAMICAL_TIME_DILATION_v1).
 *
 * Axes covered here: T1 (dilation law, sweep n_z → v), T2 (IR, sweep n⊥ + L),
 * T3 (anisotropy, motion along ⟨100⟩/⟨110⟩/⟨111⟩), T5 (controls: v=0, k⊥=0).
 * The particle clock (T4 cross-check) is a separate planned addition (plan S3).
 *
 * CLI: --L=N (single lattice, for WSL2/CUDA large-L) or default CPU multi-L
 * sweep; --output-dir=PATH; --nperp=LIST; --nzmax=N.
 */

#include "test_helpers.h"
#include "ftd/render_bridge.h"
#include "ftd/constants.h"

#include <cmath>
#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>

using namespace ftd;
using namespace ftd::test;

namespace {

namespace fs = std::filesystem;

struct IVec3 { int x, y, z; };
double imag3(IVec3 a) { return std::sqrt((double)(a.x*a.x + a.y*a.y + a.z*a.z)); }

// Single-tick Rayleigh-quotient eigenvalue ω²(k) for integer wavevector
// (nx,ny,nz) on an L³ periodic lattice. Bare wave only (no Gauss): from rest,
// after one tick wave_vel = c²∇²J = −ω²J, so ω² = −Σ(wv·J)/Σ(J²). Exact for an
// eigenmode (k = 2π·integer/L per component). Polarization (flux.z) is
// irrelevant: the Laplacian is scalar per component, so ω² depends only on k.
double measure_omega2(int L, int nx, int ny, int nz) {
    RenderBridge rb(L);
    prepare_bridge(rb, /*force_cpu=*/true);
    rb.toggles.wave_propagation = true;          // bare dispersion eigenvalue
    const double A = 0.1;
    std::vector<double> Jb((size_t)L * L * L, 0.0);
    for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
            for (int z = 0; z < L; ++z) {
                const int i = rb.lattice().index(x, y, z);
                const double ph = 2.0 * PI * (double)(nx*x + ny*y + nz*z) / L;
                const double j = A * std::sin(ph);
                rb.voxels()[i].flux = {0.0, 0.0, j};
                rb.voxels()[i].wave_vel = {0.0, 0.0, 0.0};
                Jb[(size_t)i] = j;
            }
    rb.tick();
    double num = 0.0, den = 0.0;
    const auto& vox = rb.voxels();
    for (size_t i = 0; i < Jb.size(); ++i) {
        num += vox[i].wave_vel.z * Jb[i];
        den += Jb[i] * Jb[i];
    }
    if (den < 1e-30) return 0.0;
    return -num / den;  // ω²
}

double omega_of(int L, IVec3 k) {
    return std::sqrt(std::max(0.0, measure_omega2(L, k.x, k.y, k.z)));
}

// Measure the dilation curve for a clock moving along `motion` (integer lattice
// direction) with transverse mass quantum n_perp along `tperp`. Emits one CSV
// row per longitudinal quantum n_z. Returns false on a sanity failure.
bool measure_clock(std::FILE* out, const char* dirname, int L,
                   IVec3 motion, IVec3 tperp, int n_perp, int n_z_max,
                   Counter& c) {
    const double mmag = imag3(motion);   // |motion| in lattice units
    const double dk = 2.0 * PI / L;      // wavevector quantum

    // Cap n_z so the wavevector ALONG MOTION stays pre-turnover (k_motion < ~1.2,
    // safely below the v_g peak at k=π/2): beyond it the lattice group velocity
    // falls and v(n_z)/dilation(v) go non-monotone (a lattice artifact, not SR).
    // This keeps the measurement in the clean IR/relativistic regime.
    const double K_MAX = 1.2;
    const int nz_cap = (int)std::floor(K_MAX / (mmag * dk));
    if (nz_cap < n_z_max) n_z_max = nz_cap;
    if (n_z_max < 3) n_z_max = 3;

    // Precompute ω(n_z) for n_z = 0 .. n_z_max+1 (one extra for central diff).
    std::vector<double> w(n_z_max + 2, 0.0);
    for (int nz = 0; nz <= n_z_max + 1; ++nz) {
        IVec3 k = { nz*motion.x + n_perp*tperp.x,
                    nz*motion.y + n_perp*tperp.y,
                    nz*motion.z + n_perp*tperp.z };
        w[nz] = omega_of(L, k);
    }
    const double omega0 = w[0];          // rest frequency (k_z = 0)

    double prev_v = -1.0, prev_dil = 2.0;
    bool monotone_v = true, monotone_dil = true, subluminal = true;
    // Monotonicity is a clean-regime sanity check: enforce it only over the IR
    // sub-range; the final point(s) near the k_motion cap can wobble as lattice
    // corrections set in (that wobble is the T2 signal, not a runner fault).
    const int ir_cut = std::max(2, (int)std::ceil(0.7 * n_z_max));
    for (int nz = 0; nz <= n_z_max; ++nz) {
        const double k_motion = nz * mmag * dk;          // |k| along motion axis
        double vg = 0.0;
        if (nz > 0) vg = (w[nz+1] - w[nz-1]) / (2.0 * mmag * dk);  // central diff
        const double v_norm = vg / C_WAVE;               // v/c
        const double w_proper = w[nz] - k_motion * vg;   // co-moving frequency
        const double dil_meas = (omega0 > 1e-12) ? w_proper / omega0 : 0.0;
        const double dil_L2 = (v_norm < 1.0)
                                  ? std::sqrt(std::max(0.0, 1.0 - v_norm*v_norm))
                                  : 0.0;
        const double dil_L1 = 1.0 - v_norm;

        std::fprintf(out,
            "%s,%d,%d,%d,%.8f,%.8f,%.8f,%.8f,%.8f,%.8f,%.8f,%.8f,%.8f\n",
            dirname, L, n_perp, nz, k_motion, w[nz], omega0, vg, v_norm,
            w_proper, dil_meas, dil_L2, dil_L1);

        if (nz > 0 && nz <= ir_cut) {
            if (v_norm < prev_v - 1e-9) monotone_v = false;
            if (dil_meas > prev_dil + 1e-6) monotone_dil = false;
        }
        if (v_norm >= 1.0) subluminal = false;
        prev_v = v_norm; prev_dil = dil_meas;
    }

    // ---- SANITY checks (structural; NOT the L²/L¹ verdict) ----
    const std::string tag = std::string(dirname) + " L=" + std::to_string(L)
                          + " n⊥=" + std::to_string(n_perp);
    check(("[sane] rest clock oscillates ω₀>0 (" + tag + ")").c_str(),
          omega0 > 1e-9, &c);
    check(("[sane] velocity subluminal v<c (" + tag + ")").c_str(),
          subluminal, &c);
    check(("[sane] velocity rises with momentum (" + tag + ")").c_str(),
          monotone_v, &c);
    check(("[sane] clock slows monotonically with v (" + tag + ")").c_str(),
          monotone_dil, &c);
    return omega0 > 1e-9 && subluminal && monotone_v && monotone_dil;
}

} // namespace

int main(int argc, char** argv) {
    Counter c;

    std::vector<int> Ls = {33, 49, 65};
    int nzmax_override = -1;
    int nperp_fixed = -1;   // v2 IR mode: hold n⊥ FIXED so growing L softens k⊥→0
    std::string output_dir = "engine/results/time_dilation_2026-06-07/";
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a.rfind("--L=", 0) == 0)               Ls = { std::atoi(a.c_str() + 4) };
        else if (a.rfind("--Llist=", 0) == 0) {     // comma-separated IR sweep
            Ls.clear();
            std::string s = a.substr(8);
            for (size_t pos = 0; pos < s.size(); ) {
                size_t comma = s.find(',', pos);
                if (comma == std::string::npos) comma = s.size();
                Ls.push_back(std::atoi(s.substr(pos, comma - pos).c_str()));
                pos = comma + 1;
            }
        }
        else if (a.rfind("--nzmax=", 0) == 0)      nzmax_override = std::atoi(a.c_str() + 8);
        else if (a.rfind("--nperp-fixed=", 0) == 0) nperp_fixed = std::atoi(a.c_str() + 14);
        else if (a.rfind("--output-dir=", 0) == 0) output_dir = a.substr(13);
    }
    fs::create_directories(output_dir);

    std::cout << "================================================================\n";
    std::cout << "  CAMPAIGN 2: dynamical time dilation (wave clock, dispersion)\n";
    std::cout << "  L²/γ = √(1−v²)  vs  L¹ = 1−v   (verdict drawn post-lock)\n";
    std::cout << "================================================================\n";

    const std::string csv_path = output_dir + "wave_clock_dilation.csv";
    std::FILE* out = std::fopen(csv_path.c_str(), "w");
    if (!out) { std::cerr << "cannot open " << csv_path << "\n"; return 2; }
    std::fprintf(out, "direction,L,n_perp,n_z,k_motion,omega,omega0,"
                      "v_g,v_norm,omega_proper,dilation_meas,dilation_L2,dilation_L1\n");

    // T3 anisotropy: motion along ⟨100⟩, ⟨110⟩, ⟨111⟩ with a perpendicular
    // transverse mass direction. (transverse · motion = 0 in each case.)
    struct Dir { const char* name; IVec3 motion; IVec3 tperp; };
    const std::vector<Dir> dirs = {
        { "100", {1,0,0}, {0,1,0} },   // motion x, transverse y
        { "110", {1,1,0}, {0,0,1} },   // motion (1,1,0), transverse z
        { "111", {1,1,1}, {1,-1,0} },  // motion (1,1,1), transverse (1,-1,0)
    };

    // Clock mass (transverse quantum n⊥) scales with L so the velocity sweep
    // stays IR while still resolving low v; n_z (longitudinal momentum) is
    // capped pre-turnover (≈L/5, further capped per-direction inside
    // measure_clock). Heavier masses give finer low-v sampling.
    for (int L : Ls) {
        // Default: masses ∝ L (fine v-sampling at fixed k). v2 IR mode
        // (--nperp-fixed=N): hold n⊥ FIXED so k⊥ = 2π·n⊥/L → 0 as L grows —
        // the genuine IR limit, where a fixed-(n⊥,n_z) point softens with L.
        const std::vector<int> nperps =
            (nperp_fixed > 0) ? std::vector<int>{ nperp_fixed }
                              : std::vector<int>{ std::max(2, L / 16), std::max(4, L / 8) };
        for (const auto& d : dirs) {
            for (int np : nperps) {
                const int n_z_max = (nzmax_override > 0) ? nzmax_override
                                    : (nperp_fixed > 0) ? 3 * np  // K_MAX-capped inside
                                                        : std::min(L / 5, 2 * np);
                std::cout << "  dir<" << d.name << "> L=" << L
                          << " n⊥=" << np << "\n";
                measure_clock(out, d.name, L, d.motion, d.tperp, np, n_z_max, c);
            }
        }
    }
    std::fclose(out);
    std::cout << "\n  wrote " << csv_path << "\n";
    std::cout << "  NEXT (post-lock): analyze_time_dilation.py fits dilation_meas\n"
              << "  vs v_norm against {√(1−v²), 1−v, 1−v²/2, const} per (dir,L,n⊥)\n"
              << "  and extrapolates the IR limit.\n\n";

    return report_and_exit_code(c, "Campaign 2 — time dilation (wave clock, sanity)");
}
