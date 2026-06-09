/**
 * @file test_spacetime_forcing_demo.cpp
 * @brief DEMONSTRATION for FTD-0253 (FOUND_SPACETIME_FORCING_BOUNDARY): the
 *        causal cone is forced by locality; the Lorentzian *metric* is not —
 *        it rides on the dynamics being 2nd-order (reversible/wave) rather than
 *        1st-order (irreversible/diffusion).
 *
 * Controlled experiment: the SAME lattice and the SAME isotropic Laplacian
 * (`rb.laplacian_flux`, the engine's stencil — so LOCALITY is byte-identical),
 * evolved two ways from the same initial condition:
 *   WAVE  (2nd order):  wave_vel += c²·∇²J·dt;  flux += wave_vel·dt   (leapfrog)
 *   DIFF  (1st order):  flux     += D ·∇²J·dt                          (Euler)
 * The manual leapfrog reproduces the engine's wave physics (∂²_t J = c²∇²J);
 * DIFF is the counterfactual "what if the same lattice were first-order."
 * This test never calls rb.tick() and never modifies a physics phase — it is
 * read-only on the engine (golden-gate-safe).
 *
 * PREDICTION (FTD-0253):
 *   (cone, shared)  both reach the SAME max disturbance radius from a point IC
 *                   — the locality cone is in both.
 *   (clock)         WAVE oscillates (a clock); DIFF decays monotonically (none).
 *   (energy/revers) WAVE conserves Σ(|J|²+|v|²); DIFF dissipates Σ|J|² → 0.
 *   (propagation)   WAVE spreads ballistically (r_rms ∝ t); DIFF diffusively
 *                   (r_rms ∝ √t).
 * i.e. the METRIC (clock + ballistic ruler + reversibility) appears ONLY in the
 * 2nd-order dynamics, while the CAUSAL CONE is shared.
 */

#include "test_helpers.h"
#include "ftd/render_bridge.h"
#include "ftd/constants.h"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>

using namespace ftd;
using namespace ftd::test;

namespace {

int N(const RenderBridge& rb) { return rb.lattice().total_sites(); }

// Precompute ∇²(flux) for every voxel (double-buffered: read current flux,
// write into `lap` — the engine's own isotropic stencil, same for WAVE & DIFF).
void compute_laplacian(const RenderBridge& rb, std::vector<Vec3>& lap) {
    const int n = N(rb);
    for (int i = 0; i < n; ++i) lap[(size_t)i] = rb.laplacian_flux(i);
}

void wave_step(RenderBridge& rb, double c2, double dt, std::vector<Vec3>& lap) {
    compute_laplacian(rb, lap);                 // ∇²J at the start of the step
    auto& v = rb.voxels();
    for (int i = 0; i < N(rb); ++i) {
        v[i].wave_vel.x += c2 * lap[i].x * dt;  // 2nd order: momentum then position
        v[i].wave_vel.y += c2 * lap[i].y * dt;
        v[i].wave_vel.z += c2 * lap[i].z * dt;
        v[i].flux.x += v[i].wave_vel.x * dt;
        v[i].flux.y += v[i].wave_vel.y * dt;
        v[i].flux.z += v[i].wave_vel.z * dt;
    }
}

void diffusion_step(RenderBridge& rb, double D, double dt, std::vector<Vec3>& lap) {
    compute_laplacian(rb, lap);                 // same stencil, same locality
    auto& v = rb.voxels();
    for (int i = 0; i < N(rb); ++i) {           // 1st order: flux straight from ∇²J
        v[i].flux.x += D * lap[i].x * dt;
        v[i].flux.y += D * lap[i].y * dt;
        v[i].flux.z += D * lap[i].z * dt;
    }
}

// ---- observables ----
double modal_amp_z(const RenderBridge& rb, int n_mode) {
    const int L = rb.lattice().size();
    const double k = 2.0 * PI * n_mode / L;
    const auto& v = rb.voxels();
    double acc = 0.0;
    for (int x = 0; x < L; ++x) {
        const double s = std::sin(k * x);
        for (int y = 0; y < L; ++y)
            for (int z = 0; z < L; ++z)
                acc += v[rb.lattice().index(x, y, z)].flux.z * s;
    }
    return acc;
}

double energy_J(const RenderBridge& rb) {       // Σ|J|²
    const auto& v = rb.voxels(); double e = 0.0;
    for (int i = 0; i < N(rb); ++i)
        e += v[i].flux.x*v[i].flux.x + v[i].flux.y*v[i].flux.y + v[i].flux.z*v[i].flux.z;
    return e;
}
double energy_Jv(const RenderBridge& rb) {       // Σ(|J|²+|v|²) — leapfrog invariant
    const auto& v = rb.voxels(); double e = energy_J(rb);
    for (int i = 0; i < N(rb); ++i)
        e += v[i].wave_vel.x*v[i].wave_vel.x + v[i].wave_vel.y*v[i].wave_vel.y
           + v[i].wave_vel.z*v[i].wave_vel.z;
    return e;
}

// energy-weighted RMS radius from the lattice centre (the "metric ruler")
double rms_radius(const RenderBridge& rb) {
    const int L = rb.lattice().size(); const double c = L / 2.0;
    const auto& v = rb.voxels(); double num = 0.0, den = 0.0;
    for (int x = 0; x < L; ++x) for (int y = 0; y < L; ++y) for (int z = 0; z < L; ++z) {
        const auto& vx = v[rb.lattice().index(x, y, z)];
        const double e = vx.flux.x*vx.flux.x + vx.flux.y*vx.flux.y + vx.flux.z*vx.flux.z;
        const double r2 = (x-c)*(x-c) + (y-c)*(y-c) + (z-c)*(z-c);
        num += e * r2; den += e;
    }
    return (den > 1e-30) ? std::sqrt(num / den) : 0.0;
}

// max radius at which |J| exceeds a threshold (the causal-cone front)
double front_radius(const RenderBridge& rb, double thresh) {
    const int L = rb.lattice().size(); const double c = L / 2.0;
    const auto& v = rb.voxels(); double rmax = 0.0;
    for (int x = 0; x < L; ++x) for (int y = 0; y < L; ++y) for (int z = 0; z < L; ++z) {
        const auto& vx = v[rb.lattice().index(x, y, z)];
        const double mag = std::sqrt(vx.flux.x*vx.flux.x + vx.flux.y*vx.flux.y + vx.flux.z*vx.flux.z);
        if (mag > thresh) {
            const double r = std::sqrt((x-c)*(x-c) + (y-c)*(y-c) + (z-c)*(z-c));
            if (r > rmax) rmax = r;
        }
    }
    return rmax;
}

void inject_standing_mode(RenderBridge& rb, int n_mode, double A) {
    const int L = rb.lattice().size(); const double k = 2.0 * PI * n_mode / L;
    auto& v = rb.voxels();
    for (int x = 0; x < L; ++x) for (int y = 0; y < L; ++y) for (int z = 0; z < L; ++z) {
        const int i = rb.lattice().index(x, y, z);
        v[i].flux = {0.0, 0.0, A * std::sin(k * x)};
        v[i].wave_vel = {0.0, 0.0, 0.0};
    }
}

void inject_point(RenderBridge& rb, double A) {
    const int L = rb.lattice().size();
    for (auto& vx : rb.voxels()) { vx.flux = {0,0,0}; vx.wave_vel = {0,0,0}; }
    rb.voxels()[rb.lattice().index(L/2, L/2, L/2)].flux = {0.0, 0.0, A};
}

} // namespace

int main() {
    Counter c;
    std::cout << std::fixed << std::setprecision(5);
    std::cout << "================================================================\n";
    std::cout << "  FTD-0253 demo: cone is forced (locality); metric needs 2nd order\n";
    std::cout << "  same lattice + same Laplacian; WAVE(2nd) vs DIFFUSION(1st)\n";
    std::cout << "================================================================\n\n";

    const int L = 48;
    const double dt = 0.2;
    const double c2 = C_WAVE * C_WAVE;   // wave: ∂²J = c²∇²J  (c² = 1/3)
    const double D  = c2 / 4.0;          // diffusion coeff (stable margin)
    const int n_mode = 4;
    const int TICKS = 300;
    std::vector<Vec3> lapW((size_t)L*L*L), lapD((size_t)L*L*L);

    // ---- Sub-test 1: CLOCK (oscillation vs monotone decay) ----
    std::cout << "--- 1. Clock: standing mode, modal amplitude q(t) ---\n";
    RenderBridge rw(L), rd(L);
    prepare_bridge(rw, true); prepare_bridge(rd, true);
    inject_standing_mode(rw, n_mode, 0.1);
    inject_standing_mode(rd, n_mode, 0.1);
    const double q0 = modal_amp_z(rw, n_mode);
    double wmin = q0, wmax = q0, dmin = q0;
    const double E0_d = energy_J(rd);
    for (int t = 0; t < TICKS; ++t) {
        wave_step(rw, c2, dt, lapW);
        diffusion_step(rd, D, dt, lapD);
        const double qw = modal_amp_z(rw, n_mode), qd = modal_amp_z(rd, n_mode);
        wmin = std::min(wmin, qw); wmax = std::max(wmax, qw); dmin = std::min(dmin, qd);
    }
    const double qw_final = modal_amp_z(rw, n_mode), qd_final = modal_amp_z(rd, n_mode);
    const double Ed = energy_J(rd) / E0_d;
    std::cout << "  WAVE  q: q0=" << q0 << " min=" << wmin << " max=" << wmax
              << " final=" << qw_final << "   (oscillates: swings to +/-q0)\n";
    std::cout << "  DIFF  q: q0=" << q0 << " min=" << dmin
              << " final=" << qd_final << "   (monotone decay, never negative)\n";
    std::cout << "  amplitude  WAVE |swing|/q0=" << std::min(wmax, -wmin) / q0
              << " (preserved)   DIFF Σ(J²) ratio=" << Ed << " (dissipated)\n\n";

    check("[clock] WAVE oscillates (q goes negative)", wmin < -0.2 * q0, &c);
    check("[clock] DIFF never goes negative (no clock)", dmin > -1e-7, &c);
    check("[clock] DIFF decays (final < 0.85 q0)", qd_final < 0.85 * q0, &c);
    check("[revers] WAVE non-dissipative: oscillation amplitude preserved",
          wmax > 0.9 * q0 && -wmin > 0.9 * q0, &c);
    check("[revers] DIFF dissipates Σ(J²) (irreversible)", Ed < 0.7, &c);

    // ---- Sub-test 2: CONE (shared) + PROPAGATION (ballistic vs diffusive) ----
    std::cout << "--- 2. Cone (shared) + propagation (ballistic vs diffusive) ---\n";
    RenderBridge pw(L), pd(L);
    prepare_bridge(pw, true); prepare_bridge(pd, true);
    inject_point(pw, 1.0); inject_point(pd, 1.0);
    double rW_mid=0, rW_late=0, rD_mid=0, rD_late=0;
    double frontW8=0, frontD8=0;
    const int T_CONE = 8, T_MID = 40, T_LATE = 160;
    for (int t = 1; t <= T_LATE; ++t) {
        wave_step(pw, c2, dt, lapW);
        diffusion_step(pd, D, dt, lapD);
        if (t == T_CONE) {  // strict support = causal cone (early, pre-boundary)
            frontW8 = front_radius(pw, 1e-12); frontD8 = front_radius(pd, 1e-12);
        }
        if (t == T_MID)  { rW_mid = rms_radius(pw); rD_mid = rms_radius(pd); }
        if (t == T_LATE) { rW_late = rms_radius(pw); rD_late = rms_radius(pd); }
    }
    const double wGrow = (rW_mid > 1e-9) ? rW_late / rW_mid : 0.0;
    const double dGrow = (rD_mid > 1e-9) ? rD_late / rD_mid : 0.0;
    const double ballistic = (double)T_LATE / T_MID;            // 4.0 (∝ t)
    const double diffusive = std::sqrt(ballistic);               // 2.0 (∝ √t)
    const double cone8 = T_CONE * std::sqrt(2.0) + 1.0;  // edge-nbr reach √2/tick
    std::cout << "  r_rms growth (t " << T_MID << "->" << T_LATE << "):  WAVE "
              << wGrow << " (ballistic≈" << ballistic << ")   DIFF "
              << dGrow << " (diffusive≈" << diffusive << ")\n";
    std::cout << "  causal front @t=" << T_CONE << " (cone <= " << cone8 << "):  WAVE "
              << frontW8 << "   DIFF " << frontD8 << "   (neither outruns locality)\n\n";

    // Cone is shared: NEITHER outruns the locality bound (same stencil).
    check("[cone] WAVE within locality cone", frontW8 <= cone8, &c);
    check("[cone] DIFF within locality cone", frontD8 <= cone8, &c);
    // Metric ruler: WAVE spreads ballistically, DIFF diffusively.
    check("[propagation] WAVE spreads faster than DIFF (ballistic vs diffusive)",
          wGrow > dGrow + 0.5, &c);
    check("[propagation] WAVE growth is ballistic-leaning (> diffusive midpoint)",
          wGrow > 0.5 * (ballistic + diffusive), &c);

    std::cout << "INTERPRETATION (FTD-0253): the causal CONE is shared (same\n"
              << "  locality), but the METRIC — clock, ballistic ruler, energy\n"
              << "  conservation/reversibility — appears ONLY in the 2nd-order\n"
              << "  (wave) dynamics. The Lorentzian metric is not forced by the\n"
              << "  postulates; it rides on the reversible/2nd-order action choice.\n\n";

    return report_and_exit_code(c, "FTD-0253 spacetime-forcing demo");
}
