/**
 * @file campaign_manifestation_seed_diversity.cpp
 * @brief Is genesis's locked energy stable across genuinely different birth
 *        circumstances, or does it depend on how the particle was made?
 *
 * Implements the measurement registered by
 *   docs/theory/10_eft_program/preregistrations/PREREG_MANIFESTATION_SEED_DIVERSITY_v1.md
 * (tag `preregister-manifestation-seed-diversity-v1`).
 *
 * THE PRE-REGISTRATION IS CANONICAL. This file is a measurement instrument
 * built to its spec and locked in the same commit; it makes no claim,
 * promotes no tag, and alters no engine physics.
 *
 * Five diverse seeds (position, amplitude, width), each run through fully
 * NATURAL dynamics (no genesis-off tricks, no manual state flips) until
 * manifestation fires on its own -- exactly the original G-early protocol
 * from PREREG_GENESIS_ENERGY_LEDGER_v1's instrument, repeated across
 * circumstance instead of held fixed. Seed A reproduces that campaign's
 * known result exactly (V4).
 *
 * OUTPUT: CSV to stdout (GATE rows for validity, SEED rows for results).
 * stderr carries progress. NO CV/verdict computed here; the prereg's
 * frozen bands are applied afterward by the analyst.
 *
 * Deterministic; CPU-forced.
 *
 * Epistemic status: [PRE-REGISTERED MEASUREMENT INSTRUMENT].
 */

#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>

#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"

namespace {

constexpr int    kL           = 17;
constexpr int    kMaxWait     = 200;
constexpr double kResidualGate = 1e-8;
constexpr int    kResidualCap  = 5000;
constexpr int    kSorItersRelax = 6;
constexpr double kKnownSeedAEHalf = 1.709171333089;  // genesis-energy-ledger G-early

struct SeedSpec {
    const char* name;
    double ox, oy, oz;   // offset from lattice center (8,8,8) at L=17
    double amp, sigma, cutR;
};

const SeedSpec kSeeds[] = {
    {"A_baseline", 0.31,  0.17,  0.07, 3.00, 0.45, 4.0},
    {"B_position", -3.21, 4.13, -2.09, 3.00, 0.45, 4.0},
    {"C_hot",      0.31,  0.17,  0.07, 5.00, 0.45, 4.0},
    {"D_broad",    0.31,  0.17,  0.07, 3.00, 0.75, 4.0},
    // amp=1.85 was checked before locking and found SUB-threshold at this
    // offset/sigma (peak 1.342 < K_GENESIS=1.516 -- would never fire at
    // all); corrected to 2.15 (margin +0.044, p~=8.3%/tick, mean wait ~12
    // ticks) before the lock commit -- this arithmetic check, not a run,
    // is the "instrument validation" this campaign's design permits per
    // FTD's determinism (same discipline as every prior campaign today).
    {"E_cold",     0.31,  0.17,  0.07, 2.15, 0.45, 4.0},
};
constexpr int kNSeeds = 5;

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

void configure_dynamics(ftd::RenderBridge& rb) {
    rb.toggles.disable_all();
    rb.toggles.wave_propagation  = true;
    rb.toggles.coupling          = true;
    rb.toggles.gauss_projection  = true;
    rb.toggles.genesis           = true;
    rb.toggles.damping           = true;
    rb.toggles.selective_damping = true;
}

int seed_radial_pulse(ftd::RenderBridge& rb, const SeedSpec& s) {
    const double cx = (kL - 1) / 2.0 + s.ox;
    const double cy = (kL - 1) / 2.0 + s.oy;
    const double cz = (kL - 1) / 2.0 + s.oz;
    const int lo = 0, hi = kL - 1;
    for (int x = std::max(lo, (int)(cx - s.cutR)); x <= std::min(hi, (int)(cx + s.cutR) + 1); ++x)
    for (int y = std::max(lo, (int)(cy - s.cutR)); y <= std::min(hi, (int)(cy + s.cutR) + 1); ++y)
    for (int z = std::max(lo, (int)(cz - s.cutR)); z <= std::min(hi, (int)(cz + s.cutR) + 1); ++z) {
        const double dx = x - cx, dy = y - cy, dz = z - cz;
        const double r2 = dx*dx + dy*dy + dz*dz;
        if (r2 > s.cutR * s.cutR) continue;
        const double r = std::sqrt(r2);
        if (r < 1e-9) continue;
        const double amp = s.amp * std::exp(-r2 / (2.0 * s.sigma * s.sigma));
        if (amp < 1e-9) continue;
        rb.inject_flux_add(x, y, z, ftd::Vec3(amp * dx / r, amp * dy / r, amp * dz / r));
    }
    int n_above = 0;
    for (const auto& v : rb.voxels()) if (v.flux.mag() > ftd::K_GENESIS) ++n_above;
    return n_above;
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

}  // namespace

int main() {
    std::printf("# campaign_manifestation_seed_diversity — PREREG_MANIFESTATION_SEED_DIVERSITY_v1\n");
    std::printf("# L=%d  K_GENESIS=%.10f  K_MANIFEST=%.16f\n", kL, ftd::K_GENESIS, ftd::K_MANIFEST);

    for (int si = 0; si < kNSeeds; ++si) {
        const SeedSpec& spec = kSeeds[si];
        ftd::RenderBridge rb(kL);
        rb.force_cpu();
        configure_dynamics(rb);
        const int n_above = seed_radial_pulse(rb, spec);
        std::printf("GATE,%s,V1_sites_above_threshold_preseed,%d\n", spec.name, n_above);

        int fire_tick = -1;
        for (int t = 0; t <= kMaxWait; ++t) {
            if (manifested_count(rb) >= 1) { fire_tick = t; break; }
            rb.tick();
        }
        std::printf("GATE,%s,fire_tick,%d\n", spec.name, fire_tick);
        std::printf("GATE,%s,V2_manifested_at_fire,%d\n", spec.name, manifested_count(rb));

        if (fire_tick < 0) {
            std::printf("GATE,%s,VOID_no_manifestation,1\n", spec.name);
            std::fprintf(stderr, "[%s] VOID: no manifestation within %d ticks\n", spec.name, kMaxWait);
            continue;
        }

        const double e_prerelax = e_half(rb);
        ftd::RenderBridge relaxed(kL);
        relaxed.force_cpu();
        const int apps = relax_to_fixed_point(rb, relaxed, spec.name);
        const double e = e_half(relaxed);
        std::printf("SEED,%s,e_half,%.12f,e_half_prerelax,%.12f,applications,%d,"
                    "fire_tick,%d,final_manifested,%d\n",
                    spec.name, e, e_prerelax, apps, fire_tick, manifested_count(relaxed));
        std::fprintf(stderr, "[%s] fire_tick=%d  e_half=%.12f\n", spec.name, fire_tick, e);

        if (std::strcmp(spec.name, "A_baseline") == 0) {
            std::printf("GATE,V4,seedA_matches_known,%d\n",
                        (std::fabs(e - kKnownSeedAEHalf) < 1e-9) ? 1 : 0);
        }
    }

    std::printf("# done -- CV + interpretation applied against the prereg's frozen bands by the analyst\n");
    return 0;
}
