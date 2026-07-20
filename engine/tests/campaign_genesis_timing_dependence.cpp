/**
 * @file campaign_genesis_timing_dependence.cpp
 * @brief Is the mass excess a stable property of manifestation, or does it
 *        depend on exactly when (within the stochastic hazard's eligible
 *        window) genesis happens to fire?
 *
 * Implements the measurement registered by
 *   docs/theory/10_eft_program/preregistrations/PREREG_GENESIS_TIMING_DEPENDENCE_v1.md
 * (tag `preregister-genesis-timing-dependence-v1`).
 *
 * THE PRE-REGISTRATION IS CANONICAL. This file is a measurement instrument
 * built to its spec and locked in the same commit; it makes no claim,
 * promotes no tag, and alters no engine physics.
 *
 * Reuses the identical seed and Phase-A construction as the parent
 * campaigns (deterministic RNG stream; T=0 exactly reproduces the already-
 * measured baseline). For each forced delay T in {0,1,2,3,5,8}: 1 normal
 * tick (genesis ON), then (1+T) further ticks with genesis OFF (every
 * other operation -- wave, coupling, gauss, damping -- fires normally; only
 * the manifestation decision is deferred). At that snapshot F_pre(T):
 *   - curl_drained(T)   : one-leapfrog-step curl, wave_vel scaled 0.5 at
 *                         target (matches the real engine's drain exactly)
 *   - curl_undrained(T) : same, unscaled (s=1) -- cross-check vs. the known
 *                         response curve from the parent sweep
 *   - e_half_relaxed(T) : state flipped, wave_vel DRAINED (real genesis
 *                         mechanics), relaxed to the Gauss fixed point --
 *                         the primary observable: what a real, drained
 *                         manifestation event would lock in if it fired at
 *                         delay T.
 *
 * OUTPUT: CSV to stdout (GATE rows for validity, TIMING rows per T).
 * stderr carries progress. NO verdict/CV computation is performed here;
 * the prereg's frozen bands are applied afterward by the analyst.
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

constexpr int    kL             = 17;
constexpr double kSeedOffX = 0.31, kSeedOffY = 0.17, kSeedOffZ = 0.07;
constexpr double kSeedAmp   = 3.0;
constexpr double kSeedSigma = 0.45;
constexpr double kSeedCutR  = 4.0;
constexpr double kResidualGate  = 1e-8;
constexpr int    kResidualCap   = 5000;
constexpr int    kSorItersRelax = 6;

constexpr double kParentCurlAt10 = 1.923346156209e+01;  // T=0 reproducibility (V1)

const int kDelays[] = {0, 1, 2, 3, 5, 8};
constexpr int kNDelays = 6;

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

int relax_to_fixed_point(const ftd::RenderBridge& src, ftd::RenderBridge& out, const char* tag) {
    out.toggles.disable_all();
    out.toggles.gauss_projection = true;
    out.set_sor_iterations(kSorItersRelax);
    const int L = src.lattice().size();
    const auto& sv = src.voxels();
    for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
    for (int z = 0; z < L; ++z) {
        const int i = (x * L + y) * L + z;
        if (sv[i].state != 0) out.set_state(x, y, z, sv[i].state);
        if (sv[i].flux.mag2() > 0.0) out.inject_flux_add(x, y, z, sv[i].flux);
    }
    double prev = e_half(out);
    int apps = 0;
    for (; apps < kResidualCap; ++apps) {
        out.tick();
        const double cur = e_half(out);
        if (!std::isfinite(cur) || cur > 1e12) {
            std::printf("GATE,%s,diverged_at_app,%d\n", tag, apps);
            prev = cur; ++apps; break;
        }
        if (std::fabs(cur - prev) < kResidualGate) { prev = cur; ++apps; break; }
        prev = cur;
    }
    std::printf("GATE,%s,relax_applications,%d\n", tag, apps);
    std::printf("GATE,%s,relax_converged,%d\n", tag, (apps < kResidualCap) ? 1 : 0);
    return apps;
}

// Isolated single-leapfrog-step curl at scaling s, from a bit-identical
// clone of `src` (so callers can measure drained/undrained without
// disturbing `src`'s own state).
double isolated_curl(const ftd::RenderBridge& src, int tx, int ty, int tz, double s) {
    ftd::RenderBridge c(kL);
    c.force_cpu();
    clone_full(src, c);
    c.set_state(tx, ty, tz, +1);
    c.voxel_at(tx, ty, tz).wave_vel = c.voxel_at(tx, ty, tz).wave_vel * s;
    c.toggles.disable_all();
    c.tick();
    return curl_total(c);
}

}  // namespace

int main() {
    std::printf("# campaign_genesis_timing_dependence — PREREG_GENESIS_TIMING_DEPENDENCE_v1\n");
    std::printf("# L=%d  K_GENESIS=%.10f  K_MANIFEST=%.16f  drain=%.4f\n",
                kL, ftd::K_GENESIS, ftd::K_MANIFEST, ftd::K_GENESIS_KINETIC_DRAIN);

    for (int di = 0; di < kNDelays; ++di) {
        const int T = kDelays[di];
        char tag[32];
        std::snprintf(tag, sizeof(tag), "T%d", T);

        ftd::RenderBridge rb(kL);
        rb.force_cpu();
        configure_dynamics(rb);
        int tx = -1, ty = -1, tz = -1;
        const int n_above0 = seed_radial_pulse(rb, tx, ty, tz);
        if (n_above0 != 1) std::printf("GATE,%s,UNEXPECTED_preseed_sites,%d\n", tag, n_above0);

        rb.tick();  // call 1, genesis ON, matches every prior campaign's convention

        rb.toggles.genesis = false;
        for (int i = 0; i < 1 + T; ++i) {
            rb.tick();
            // V2: verify still exactly 1 eligible/unmanifested site every step.
            if (manifested_count(rb) != 0)
                std::printf("GATE,%s,V2_UNEXPECTED_manifested_during_delay,%d\n", tag, i);
        }
        rb.toggles.genesis = true;

        // V2 (final): exactly one site above threshold, still unmanifested.
        int n_above_final = 0;
        bool target_above = false;
        for (const auto& v : rb.voxels()) {
            if (v.flux.mag() > ftd::K_GENESIS) ++n_above_final;
        }
        {
            const auto& tv = rb.voxels()[(tx * kL + ty) * kL + tz];
            target_above = tv.flux.mag() > ftd::K_GENESIS;
        }
        std::printf("GATE,%s,sites_above_threshold,%d\n", tag, n_above_final);
        std::printf("GATE,%s,target_above_threshold,%d\n", tag, target_above ? 1 : 0);
        std::printf("GATE,%s,manifested_pre_flip,%d\n", tag, manifested_count(rb));

        const double fpre_e = e_half(rb);
        const double fpre_curl = curl_total(rb);
        const ftd::Vec3 wv0 = rb.voxel_at(tx, ty, tz).wave_vel;
        std::printf("TIMING,%s,%d,fpre_e_half,%.12f,fpre_curl,%.12e,wv_mag,%.12f\n",
                    tag, T, fpre_e, fpre_curl, wv0.mag());

        const double curl_und = isolated_curl(rb, tx, ty, tz, 1.0);
        const double curl_dr  = isolated_curl(rb, tx, ty, tz, 1.0 - ftd::K_GENESIS_KINETIC_DRAIN);
        std::printf("TIMING,%s,%d,curl_undrained,%.12e,curl_drained,%.12e\n",
                    tag, T, curl_und, curl_dr);
        if (T == 0) {
            std::printf("GATE,V1,curl_undrained_matches_parent,%d\n",
                        (std::fabs(curl_und - kParentCurlAt10) < 1e-6) ? 1 : 0);
        }

        // Primary observable: relax with DRAIN applied (real genesis mechanics).
        ftd::RenderBridge drained_pre(kL);
        drained_pre.force_cpu();
        clone_full(rb, drained_pre);
        drained_pre.set_state(tx, ty, tz, +1);
        drained_pre.voxel_at(tx, ty, tz).wave_vel =
            drained_pre.voxel_at(tx, ty, tz).wave_vel * (1.0 - ftd::K_GENESIS_KINETIC_DRAIN);

        ftd::RenderBridge relaxed(kL);
        relaxed.force_cpu();
        const int apps = relax_to_fixed_point(drained_pre, relaxed, tag);
        const double e_relaxed = e_half(relaxed);
        std::printf("TIMING,%s,%d,e_half_relaxed,%.12f,applications,%d,final_manifested,%d\n",
                    tag, T, e_relaxed, apps, manifested_count(relaxed));
        std::fprintf(stderr, "[T=%d] fpre_curl=%.6e  curl_drained=%.6e  e_half_relaxed=%.6f  (apps=%d)\n",
                     T, fpre_curl, curl_dr, e_relaxed, apps);
    }

    std::printf("# done -- CV + interpretation applied against the prereg's frozen bands by the analyst\n");
    return 0;
}
