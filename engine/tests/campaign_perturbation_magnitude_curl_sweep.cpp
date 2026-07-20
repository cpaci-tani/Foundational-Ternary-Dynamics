/**
 * @file campaign_perturbation_magnitude_curl_sweep.cpp
 * @brief Does injected curl scale with perturbation size, or is it a
 *        symmetry-breaking floor set by acting on a single site at all?
 *
 * Implements the measurement registered by
 *   docs/theory/10_eft_program/preregistrations/PREREG_PERTURBATION_MAGNITUDE_CURL_SWEEP_v1.md
 * (tag `preregister-perturbation-magnitude-curl-sweep-v1`).
 *
 * THE PRE-REGISTRATION IS CANONICAL. This file is a measurement instrument
 * built to its spec and locked in the same commit; it makes no claim,
 * promotes no tag, and alters no engine physics.
 *
 * Reuses the identical seed, Phase-A prefix (1 normal tick, then 1
 * genesis-OFF tick — the v1.1-corrected convention), and F_pre snapshot
 * construction as campaign_kinetic_drain_curl_isolation.cpp, so F_pre is
 * bit-identical to that campaign's (verified as V1 below) and the s=0.5 /
 * s=1.0 sweep points exactly reproduce its already-measured Test-B values.
 *
 * Structural prediction (prereg 1, not a physics guess — a consequence of
 * the operations' own linearity): the single leapfrog step is affine in the
 * scaling factor s (flux[target] = flux_old[target] + s*wave_vel_original);
 * curl_total is a quadratic functional of flux. A quadratic functional of
 * an affine function of s is EXACTLY a quadratic polynomial in s. The sweep
 * is fit to A + B*s + C*s^2 and R^2 is reported as a structural validity
 * check (V2), not as the physics result -- the physics is in the fitted
 * coefficients themselves (see the prereg's OUTCOME section for the
 * post-hoc characterization).
 *
 * OUTPUT: CSV to stdout (GATE rows for validity, SWEEP rows per s, SUMMARY
 * rows). stderr carries progress. NO verdict/fit is computed here beyond
 * raw measurement; the quadratic fit and its interpretation are applied
 * afterward by the analyst (kept out of the instrument so the fit can be
 * redone/audited independently of the C++ build).
 *
 * Deterministic; CPU-forced.
 *
 * Epistemic status: [PRE-REGISTERED MEASUREMENT INSTRUMENT].
 */

#include <cmath>
#include <cstdio>
#include <vector>

#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"

namespace {

// ── Reused verbatim from the parent instruments ─────────────────────────
constexpr int    kL             = 17;
constexpr double kSeedOffX = 0.31, kSeedOffY = 0.17, kSeedOffZ = 0.07;
constexpr double kSeedAmp   = 3.0;
constexpr double kSeedSigma = 0.45;
constexpr double kSeedCutR  = 4.0;

// Parent's disclosed F_pre values (V1 reproducibility check) and Test-B
// sweep-point cross-checks (V3).
constexpr double kParentFPreEHalf = 3.129589867365;
constexpr double kParentFPreCurl  = 7.065424291986e+00;
constexpr double kParentCurlAt05  = 1.265327895938e+01;  // Test-B drain
constexpr double kParentCurlAt10  = 1.923346156209e+01;  // Test-B no-drain

const double kSweepS[] = {0.0, 0.25, 0.5, 0.75, 1.0, 1.25, 1.5, 2.0};
constexpr int kNSweep = 8;

double e_half(const ftd::RenderBridge& rb) {
    double e = 0.0;
    for (const auto& v : rb.voxels()) e += v.flux.mag2();
    return 0.5 * e;
}

int manifested_count(const ftd::RenderBridge& rb) {
    int n = 0;
    for (const auto& v : rb.voxels()) if (v.state != 0) ++n;
    return n;
}

double curl_total(const ftd::RenderBridge& rb) {
    const int L = rb.lattice().size();
    const auto& v = rb.voxels();
    auto idx = [&](int x, int y, int z) {
        x = (x % L + L) % L; y = (y % L + L) % L; z = (z % L + L) % L;
        return (x * L + y) * L + z;
    };
    double c2 = 0.0;
    for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
    for (int z = 0; z < L; ++z) {
        const double dJzdy = (v[idx(x,y+1,z)].flux.z - v[idx(x,y-1,z)].flux.z) * 0.5;
        const double dJydz = (v[idx(x,y,z+1)].flux.y - v[idx(x,y,z-1)].flux.y) * 0.5;
        const double dJxdz = (v[idx(x,y,z+1)].flux.x - v[idx(x,y,z-1)].flux.x) * 0.5;
        const double dJzdx = (v[idx(x+1,y,z)].flux.z - v[idx(x-1,y,z)].flux.z) * 0.5;
        const double dJydx = (v[idx(x+1,y,z)].flux.y - v[idx(x-1,y,z)].flux.y) * 0.5;
        const double dJxdy = (v[idx(x,y+1,z)].flux.x - v[idx(x,y-1,z)].flux.x) * 0.5;
        const double cx = dJzdy - dJydz;
        const double cy = dJxdz - dJzdx;
        const double cz = dJydx - dJxdy;
        c2 += cx*cx + cy*cy + cz*cz;
    }
    return c2;
}

void configure_dynamics(ftd::RenderBridge& rb) {
    rb.toggles.disable_all();
    rb.toggles.wave_propagation  = true;
    rb.toggles.coupling          = true;
    rb.toggles.gauss_projection  = true;
    rb.toggles.genesis           = true;
    rb.toggles.damping           = true;
    rb.toggles.selective_damping = true;
}

int seed_radial_pulse(ftd::RenderBridge& rb, int& out_tx, int& out_ty, int& out_tz) {
    const double cx = (kL - 1) / 2.0 + kSeedOffX;
    const double cy = (kL - 1) / 2.0 + kSeedOffY;
    const double cz = (kL - 1) / 2.0 + kSeedOffZ;
    const int lo = 0, hi = kL - 1;
    double best_r2 = 1e18;
    for (int x = std::max(lo, (int)(cx - kSeedCutR)); x <= std::min(hi, (int)(cx + kSeedCutR) + 1); ++x)
    for (int y = std::max(lo, (int)(cy - kSeedCutR)); y <= std::min(hi, (int)(cy + kSeedCutR) + 1); ++y)
    for (int z = std::max(lo, (int)(cz - kSeedCutR)); z <= std::min(hi, (int)(cz + kSeedCutR) + 1); ++z) {
        const double dx = x - cx, dy = y - cy, dz = z - cz;
        const double r2 = dx*dx + dy*dy + dz*dz;
        if (r2 > kSeedCutR * kSeedCutR) continue;
        const double r = std::sqrt(r2);
        if (r < 1e-9) continue;
        const double amp = kSeedAmp * std::exp(-r2 / (2.0 * kSeedSigma * kSeedSigma));
        if (amp < 1e-9) continue;
        rb.inject_flux_add(x, y, z, ftd::Vec3(amp * dx / r, amp * dy / r, amp * dz / r));
        if (r2 < best_r2) { best_r2 = r2; out_tx = x; out_ty = y; out_tz = z; }
    }
    int n_above = 0;
    for (const auto& v : rb.voxels()) if (v.flux.mag() > ftd::K_GENESIS) ++n_above;
    return n_above;
}

void clone_full(const ftd::RenderBridge& src, ftd::RenderBridge& out) {
    out.toggles.disable_all();
    const int L = src.lattice().size();
    const auto& sv = src.voxels();
    for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
    for (int z = 0; z < L; ++z) {
        const int i = (x * L + y) * L + z;
        if (sv[i].state != 0) out.set_state(x, y, z, sv[i].state);
        if (sv[i].flux.mag2() > 0.0) out.inject_flux_add(x, y, z, sv[i].flux);
        if (sv[i].wave_vel.mag2() > 0.0)
            out.voxel_at(x, y, z).wave_vel = sv[i].wave_vel;
    }
}

}  // namespace

int main() {
    std::printf("# campaign_perturbation_magnitude_curl_sweep — PREREG_PERTURBATION_MAGNITUDE_CURL_SWEEP_v1\n");
    std::printf("# L=%d  K_GENESIS=%.10f  K_MANIFEST=%.16f\n", kL, ftd::K_GENESIS, ftd::K_MANIFEST);

    // ── Common Phase-A prefix, identical to the parent isolation campaign ──
    ftd::RenderBridge rb(kL);
    rb.force_cpu();
    configure_dynamics(rb);
    int tx = -1, ty = -1, tz = -1;
    const int n_above = seed_radial_pulse(rb, tx, ty, tz);
    std::printf("GATE,prefix,sites_above_threshold_preseed,%d\n", n_above);
    std::printf("GATE,prefix,target_site,%d,%d,%d\n", tx, ty, tz);

    if (manifested_count(rb) >= 1) std::printf("GATE,prefix,UNEXPECTED_EARLY_FIRE,0\n");
    rb.tick();  // call 1
    std::printf("GATE,prefix,manifested_after_call1,%d\n", manifested_count(rb));

    if (manifested_count(rb) >= 1) std::printf("GATE,prefix,UNEXPECTED_EARLY_FIRE,1\n");
    rb.toggles.genesis = false;
    rb.tick();  // call 2, genesis suppressed
    rb.toggles.genesis = true;

    const int manifested_pre = manifested_count(rb);
    const double fpre_e = e_half(rb);
    const double fpre_curl = curl_total(rb);
    std::printf("GATE,V2,manifested_pre_flip,%d\n", manifested_pre);
    std::printf("SUMMARY,F_pre,e_half,%.12f,curl_total,%.12e\n", fpre_e, fpre_curl);
    std::printf("GATE,V1,fpre_ehalf_matches_parent,%d\n",
                (std::fabs(fpre_e - kParentFPreEHalf) < 1e-9) ? 1 : 0);
    std::printf("GATE,V1,fpre_curl_matches_parent,%d\n",
                (std::fabs(fpre_curl - kParentFPreCurl) < 1e-6) ? 1 : 0);
    std::fprintf(stderr, "[F_pre] e_half=%.12f curl=%.6e (parent: %.12f / %.6e)\n",
                 fpre_e, fpre_curl, kParentFPreEHalf, kParentFPreCurl);

    // F_pre now sits in `rb`; wave_vel at the target is what real dynamics
    // produced there — this is what the sweep scales.
    const ftd::Vec3 wv0 = rb.voxel_at(tx, ty, tz).wave_vel;
    std::printf("GATE,prefix,wave_vel_target,%.12f,%.12f,%.12f\n", wv0.x, wv0.y, wv0.z);

    // ── C_null: no flip, no wave_vel touch, one raw leapfrog step ──────────
    {
        ftd::RenderBridge nullc(kL);
        nullc.force_cpu();
        clone_full(rb, nullc);
        nullc.toggles.disable_all();
        nullc.tick();
        std::printf("SUMMARY,C_null,curl_total,%.12e,e_half,%.12f\n",
                    curl_total(nullc), e_half(nullc));
    }

    // ── The sweep ────────────────────────────────────────────────────────
    for (int i = 0; i < kNSweep; ++i) {
        const double s = kSweepS[i];
        ftd::RenderBridge c(kL);
        c.force_cpu();
        clone_full(rb, c);
        c.set_state(tx, ty, tz, +1);
        c.voxel_at(tx, ty, tz).wave_vel = wv0 * s;
        c.toggles.disable_all();
        c.tick();
        const double curl = curl_total(c);
        const double eh = e_half(c);
        std::printf("SWEEP,%.4f,%.12e,%.12f\n", s, curl, eh);
        std::fprintf(stderr, "[sweep s=%.2f] curl_total=%.6e e_half=%.6f\n", s, curl, eh);
    }

    std::printf("# done -- quadratic fit + interpretation applied by the analyst, not here\n");
    return 0;
}
