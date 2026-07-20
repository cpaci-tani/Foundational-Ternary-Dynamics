/**
 * @file campaign_kinetic_drain_curl_isolation.cpp
 * @brief Isolating the transverse-contamination mechanism: is it the
 *        genesis kinetic-drain operation?
 *
 * Implements the measurement registered by
 *   docs/theory/10_eft_program/preregistrations/PREREG_KINETIC_DRAIN_CURL_ISOLATION_v1.md
 * (tag `preregister-kinetic-drain-curl-isolation-v1`).
 *
 * THE PRE-REGISTRATION IS CANONICAL. This file is a measurement instrument
 * built to its spec and locked in the same commit; it makes no claim,
 * promotes no tag, and alters no engine physics.
 *
 * ── Test A — the decisive intervention (prereg §1.A) ────────────────────
 * Reproduces PREREG_GENESIS_ENERGY_LEDGER_v1's G-early trajectory exactly
 * through tick 1 (identical seed, identical deterministic RNG stream). At
 * tick 2 (the manifestation tick), `genesis` is toggled OFF for that ONE
 * tick only — wave/coupling/gauss/damping still run normally — so the
 * engine's own manifest-at + kinetic-drain code path never fires. The site
 * is then flipped to manifested by `rb.set_state(x,y,z,+1)` directly, which
 * (verified by source read: injection.cpp vs. render_bridge.cpp
 * set_state_unlocked) touches ONLY the ternary state field — never flux,
 * never wave_vel. Every other same-tick operation is preserved; only the
 * drain is removed. Frozen, relaxed exactly as the parent campaign's arms.
 *
 * ── Test B — the isolated unit test (prereg §1.B) ────────────────────────
 * From a single bit-identical snapshot (state+flux+wave_vel, captured at
 * the same tick-2-genesis-off point Test A uses), two fresh bridges are
 * cloned. Copy-drain applies set_state(+1) then scales wave_vel at the
 * target site by (1-K_GENESIS_KINETIC_DRAIN) (replicating the engine's own
 * manifest-at drain) then ONE raw leapfrog integration (toggles.disable_
 * all() + one tick() — on a fresh bridge delta_j_ is zero-initialized and
 * never populated since wave_propagation is never enabled on it, so this
 * applies only wave_vel+=0; flux+=wave_vel, the engine's unconditional base
 * leapfrog, nothing else active). Copy-no-drain is identical minus the
 * scaling. curl_total is compared — since the two copies are bit-identical
 * up to the single scaling operation, any difference is causally
 * attributable to the drain step alone.
 *
 * OUTPUT: CSV to stdout (GATE rows for validity, SUMMARY rows for e_half /
 * curl_total per arm/copy). stderr carries progress. NO verdict is computed
 * here; the prereg's outcome map is applied afterward by the analyst.
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

// ── Reused verbatim from PREREG_GENESIS_ENERGY_LEDGER_v1's instrument ──────
constexpr int    kL             = 17;
constexpr double kFrozenEhalf17 = 0.478917129;
constexpr double kResidualGate  = 1e-8;
constexpr int    kResidualCap   = 5000;
constexpr int    kSorItersRelax = 6;
constexpr double kSeedOffX = 0.31, kSeedOffY = 0.17, kSeedOffZ = 0.07;
constexpr double kSeedAmp   = 3.0;
constexpr double kSeedSigma = 0.45;
constexpr double kSeedCutR  = 4.0;

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

// Discrete central-difference curl, summed as |curl(J)|^2 over the lattice.
// A NEW diagnostic for this campaign (not previously measured in this
// program). Periodic wrap via lattice().index() neighbor helpers would be
// cleaner, but at L=17 the seed+dynamics stay well clear of the boundary
// (cutoff 4 from a near-center site), so direct modular indexing is safe
// and avoids adding a new lattice-API dependency for a diagnostic-only tool.
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

// Relax to the Gauss-projector fixed point on a FRESH bridge (the
// stale-delta_j_ runaway fix from the parent campaign — see its own
// disclosure in PREREG_GENESIS_ENERGY_LEDGER_v1 §1).
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
        if (sv[i].state != 0)
            out.set_state(x, y, z, sv[i].state);
        if (sv[i].flux.mag2() > 0.0)
            out.inject_flux_add(x, y, z, sv[i].flux);
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

// Copy state+flux+wave_vel onto a fresh bridge (full clone, for Test B).
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

// Max |flux difference| between two bridges, ignoring the target site
// (V3 gate: the leapfrog step must not perturb any OTHER voxel).
double max_flux_diff_excluding(const ftd::RenderBridge& a, const ftd::RenderBridge& b,
                               int L, int tx, int ty, int tz) {
    const auto& va = a.voxels();
    const auto& vb = b.voxels();
    double m = 0.0;
    for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
    for (int z = 0; z < L; ++z) {
        if (x == tx && y == ty && z == tz) continue;
        const int i = (x * L + y) * L + z;
        const double d = (va[i].flux - vb[i].flux).mag();
        if (d > m) m = d;
    }
    return m;
}

}  // namespace

int main() {
    std::printf("# campaign_kinetic_drain_curl_isolation — PREREG_KINETIC_DRAIN_CURL_ISOLATION_v1\n");
    std::printf("# L=%d  K_GENESIS=%.10f  K_MANIFEST=%.16f  drain=%.4f  W_SC(17) frozen=%.9f\n",
                kL, ftd::K_GENESIS, ftd::K_MANIFEST, ftd::K_GENESIS_KINETIC_DRAIN, kFrozenEhalf17);

    // ── Common Phase-A prefix: seed + ticks 0,1 (genesis ON, no fire) ──────
    ftd::RenderBridge rb(kL);
    rb.force_cpu();
    configure_dynamics(rb);
    int tx = -1, ty = -1, tz = -1;
    const int n_above = seed_radial_pulse(rb, tx, ty, tz);
    std::printf("GATE,prefix,sites_above_threshold_preseed,%d\n", n_above);
    std::printf("GATE,prefix,target_site,%d,%d,%d\n", tx, ty, tz);

    for (int t = 0; t < 2; ++t) {
        if (manifested_count(rb) >= 1) {
            std::printf("GATE,prefix,UNEXPECTED_EARLY_FIRE,%d\n", t);
        }
        rb.tick();
    }
    std::printf("GATE,prefix,manifested_after_tick1,%d\n", manifested_count(rb));

    // ── Tick 2 with genesis OFF: every other operation fires normally ──────
    rb.toggles.genesis = false;
    rb.tick();
    rb.toggles.genesis = true;  // restored (unused henceforth, but tidy)
    const int manifested_pre_flip = manifested_count(rb);
    std::printf("GATE,V2,manifested_pre_manual_flip,%d\n", manifested_pre_flip);
    std::printf("SUMMARY,F_pre,e_half,%.12f,curl_total,%.12e\n", e_half(rb), curl_total(rb));

    // F_pre is now captured in `rb` — used by both Test A and Test B below.

    // ── Test A: manual state flip, NO drain, then relax ─────────────────────
    {
        ftd::RenderBridge nodrain(kL);
        nodrain.force_cpu();
        clone_full(rb, nodrain);
        nodrain.set_state(tx, ty, tz, +1);
        const double e_prerelax = e_half(nodrain);
        const double curl_prerelax = curl_total(nodrain);
        std::printf("SUMMARY,G-nodrain,e_half_prerelax,%.12f,curl_prerelax,%.12e\n",
                    e_prerelax, curl_prerelax);

        ftd::RenderBridge relaxed(kL);
        relaxed.force_cpu();
        const int apps = relax_to_fixed_point(nodrain, relaxed, "G-nodrain");
        const double e = e_half(relaxed);
        std::printf("SUMMARY,G-nodrain,e_half,%.12f,applications,%d,final_manifested,%d\n",
                    e, apps, manifested_count(relaxed));
        std::fprintf(stderr, "[G-nodrain] e_half = %.12f (W_SC=%.9f, G-early was 1.709171333089)\n",
                     e, kFrozenEhalf17);
    }

    // ── Test B: isolated single-leapfrog-step, drain vs no-drain ────────────
    {
        ftd::RenderBridge drain_copy(kL), nodrain_copy(kL);
        drain_copy.force_cpu();
        nodrain_copy.force_cpu();
        clone_full(rb, drain_copy);
        clone_full(rb, nodrain_copy);

        drain_copy.set_state(tx, ty, tz, +1);
        nodrain_copy.set_state(tx, ty, tz, +1);

        // Replicate the engine's own kinetic-drain scaling, at the target
        // site ONLY (phase_write.cpp: v.wave_vel *= (1 - K_GENESIS_KINETIC_DRAIN)).
        drain_copy.voxel_at(tx, ty, tz).wave_vel =
            drain_copy.voxel_at(tx, ty, tz).wave_vel * (1.0 - ftd::K_GENESIS_KINETIC_DRAIN);

        // ONE raw leapfrog integration on each: disable_all() means
        // wave_propagation is off, so delta_j_ is zero-initialized and never
        // populated on these fresh bridges — the single tick() applies only
        // the engine's unconditional base leapfrog (wave_vel+=0; flux+=wave_vel).
        drain_copy.toggles.disable_all();
        nodrain_copy.toggles.disable_all();
        drain_copy.tick();
        nodrain_copy.tick();

        const double curl_drain = curl_total(drain_copy);
        const double curl_nodrain = curl_total(nodrain_copy);
        const double e_drain = e_half(drain_copy);
        const double e_nodrain = e_half(nodrain_copy);
        const double v3_maxdiff = max_flux_diff_excluding(drain_copy, nodrain_copy, kL, tx, ty, tz);

        std::printf("GATE,TestB,v3_max_flux_diff_excl_target,%.12e\n", v3_maxdiff);
        std::printf("SUMMARY,TestB-drain,curl_total,%.12e,e_half,%.12f\n", curl_drain, e_drain);
        std::printf("SUMMARY,TestB-nodrain,curl_total,%.12e,e_half,%.12f\n", curl_nodrain, e_nodrain);
        std::printf("SUMMARY,TestB-ratio,curl_drain_over_nodrain,%.6f\n",
                    (curl_nodrain > 0) ? curl_drain / curl_nodrain : -1.0);
        std::fprintf(stderr, "[TestB] curl_total: drain=%.6e  nodrain=%.6e  ratio=%.4f\n",
                     curl_drain, curl_nodrain, (curl_nodrain > 0) ? curl_drain / curl_nodrain : -1.0);
    }

    std::printf("# done -- verdict applied against the prereg's outcome map by the analyst, not here\n");
    return 0;
}
