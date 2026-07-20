/**
 * @file campaign_manifestation_readout_collision.cpp
 * @brief Do genuinely different flux configurations manifest to the
 *        identical discrete (state, color, spin) readout?
 *
 * This is a deterministic extension of an already-locked, already-run
 * protocol -- no new stochastic design choice is being introduced, so
 * this is not pre-registered under LOCK-STD v1 (nothing here could be
 * tuned after seeing results; the seeds and dynamics are byte-identical
 * to campaign_manifestation_seed_diversity.cpp, reused verbatim).
 *
 * Background: manifest_at() (render_bridge_phases/phase_write.cpp) sets
 *   - state  from the sign of a polarity signal (chirality/divergence),
 *   - color  from the DOMINANT AXIS of the live flux vector (fx,fy,fz),
 *   - spin   from the dominant axis of the local flux curl (or a coin
 *            flip if the curl is ~0).
 * Each of these is a many-to-one reduction of continuous information
 * (an axis-magnitude COMPARISON, not the magnitudes themselves) down to
 * one of a handful of discrete values. This campaign asks whether that
 * reduction is actually LOSSY in practice: do the three already-locked
 * seed-diversity seeds (A_baseline amp=3.00, C_hot amp=5.00, E_cold
 * amp=2.15 -- verified this session to differ in locked e_half by a
 * factor of 9.2x) collapse onto the SAME (state,color,spin) triple
 * despite their substantially different flux content at freeze?
 *
 * Honest scope: this does NOT show the engine "forgets" J -- flux is
 * not zeroed by manifest_at, and remains fully readable afterward. The
 * claim under test is narrower and still real: the (state,color,spin)
 * triple ALONE -- the "actualized" record FTD's ontology treats as what
 * manifestation produces -- cannot be inverted to recover which of
 * several substantially different flux configurations produced it, even
 * though nothing here claims the underlying number J is deleted from
 * the engine's memory.
 *
 * OUTPUT: CSV to stdout. stderr carries progress. No verdict tag is
 * assigned here beyond reporting the measured collision/non-collision.
 *
 * Deterministic; CPU-forced.
 *
 * Epistemic status: [MEASUREMENT INSTRUMENT], deterministic extension of
 * an already-locked protocol.
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdio>
#include <vector>

#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"

namespace {

constexpr int kL       = 17;
constexpr int kMaxWait = 200;

struct SeedSpec {
    const char* name;
    double ox, oy, oz;
    double amp, sigma, cutR;
    double known_e_half_prerelax;  // reproducibility target, from this morning's runs
};

// Byte-identical to campaign_manifestation_seed_diversity.cpp / campaign_
// hedgehog_charge_robustness.cpp's three clean arms. known_e_half_prerelax
// values are the e_half measured at the SAME freeze point (before any
// relaxation) in campaign_hedgehog_charge_robustness.cpp's v1.1 run.
const SeedSpec kSeeds[] = {
    {"A_baseline", 0.31, 0.17, 0.07, 3.00, 0.45, 4.0, 1.368676308503},
    {"C_hot",      0.31, 0.17, 0.07, 5.00, 0.45, 4.0, 5.828246462835},
    {"E_cold",     0.31, 0.17, 0.07, 2.15, 0.45, 4.0, 0.540720277788},
};
constexpr int kNSeeds = 3;

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

bool find_manifested_site(const ftd::RenderBridge& rb, int& mx, int& my, int& mz) {
    const int L = rb.lattice().size();
    const auto& v = rb.voxels();
    for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
    for (int z = 0; z < L; ++z) {
        if (v[(x * L + y) * L + z].state != 0) { mx = x; my = y; mz = z; return true; }
    }
    return false;
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

}  // namespace

int main() {
    std::printf("# campaign_manifestation_readout_collision\n");
    std::printf("# L=%d  K_GENESIS=%.10f  K_MANIFEST=%.16f\n", kL, ftd::K_GENESIS, ftd::K_MANIFEST);

    int8_t states[kNSeeds] = {0, 0, 0};
    int8_t colors[kNSeeds] = {0, 0, 0};
    int8_t spins[kNSeeds]  = {0, 0, 0};
    double jmags[kNSeeds]  = {0, 0, 0};
    ftd::Vec3 jvecs[kNSeeds];
    bool valid[kNSeeds] = {false, false, false};

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
        if (fire_tick < 0) {
            std::printf("GATE,%s,VOID_no_manifestation,1\n", spec.name);
            std::fprintf(stderr, "[%s] VOID: no manifestation within %d ticks\n", spec.name, kMaxWait);
            continue;
        }

        const double e = e_half(rb);
        const double e_err = std::fabs(e - spec.known_e_half_prerelax);
        std::printf("GATE,%s,V2_e_half_matches_known,%d\n", spec.name, (e_err < 1e-9) ? 1 : 0);

        int mx, my, mz;
        if (!find_manifested_site(rb, mx, my, mz)) {
            std::printf("GATE,%s,VOID_site_not_found,1\n", spec.name);
            continue;
        }
        const int L = rb.lattice().size();
        const auto& v = rb.voxels()[(mx * L + my) * L + mz];

        states[si] = v.state;
        colors[si] = v.color;
        spins[si]  = v.spin;
        jvecs[si]  = v.flux;
        jmags[si]  = v.flux.mag();
        valid[si]  = true;

        std::printf("ARM,%s,state,%d,color,%d,spin,%d,J,%.10f,%.10f,%.10f,Jmag,%.10f,e_half,%.12f,fire_tick,%d\n",
                    spec.name, (int)v.state, (int)v.color, (int)v.spin,
                    v.flux.x, v.flux.y, v.flux.z, jmags[si], e, fire_tick);
        std::fprintf(stderr, "[%s] state=%d color=%d spin=%d |J|=%.6f e_half=%.6f\n",
                     spec.name, (int)v.state, (int)v.color, (int)v.spin, jmags[si], e);
    }

    // Pairwise comparison report -- the actual question.
    std::printf("# pairwise readout-collision report\n");
    for (int a = 0; a < kNSeeds; ++a) {
        for (int b = a + 1; b < kNSeeds; ++b) {
            if (!valid[a] || !valid[b]) continue;
            const bool same_readout = (states[a] == states[b]) &&
                                       (colors[a] == colors[b]) &&
                                       (spins[a]  == spins[b]);
            const ftd::Vec3 diff = jvecs[a] - jvecs[b];
            const double diff_mag = diff.mag();
            const double dot = jvecs[a].dot(jvecs[b]);
            const double cos_angle = (jmags[a] > 1e-12 && jmags[b] > 1e-12)
                ? std::max(-1.0, std::min(1.0, dot / (jmags[a] * jmags[b])))
                : 1.0;
            const double angle_deg = std::acos(cos_angle) * 180.0 / M_PI;
            std::printf("PAIR,%s,%s,same_readout,%d,|J_a|,%.6f,|J_b|,%.6f,|Ja-Jb|,%.6f,angle_deg,%.4f\n",
                        kSeeds[a].name, kSeeds[b].name, same_readout ? 1 : 0,
                        jmags[a], jmags[b], diff_mag, angle_deg);
            std::fprintf(stderr, "[%s vs %s] same (state,color,spin) = %s ; |J_a|=%.4f |J_b|=%.4f |ΔJ|=%.4f angle=%.2f deg\n",
                         kSeeds[a].name, kSeeds[b].name, same_readout ? "YES" : "no",
                         jmags[a], jmags[b], diff_mag, angle_deg);
        }
    }

    std::printf("# done -- interpretation applied by the analyst\n");
    return 0;
}
