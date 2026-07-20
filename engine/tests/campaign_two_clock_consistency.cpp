/**
 * @file campaign_two_clock_consistency.cpp
 * @brief Does the substrate's DECAY clock dilate like its PROPER-TIME clock?
 *
 * Implements the measurement registered by
 *   docs/theory/10_eft_program/preregistrations/PREREG_TWO_CLOCK_CONSISTENCY_v1.md
 * (tag `preregister-two-clock-consistency-v1`).
 *
 * THE PRE-REGISTRATION IS CANONICAL. This file is a measurement instrument
 * built to its spec and locked in the same commit; it makes no claim,
 * promotes no tag, and alters no engine physics.
 *
 * FROZEN-FIELD DESIGN (prereg §2). Toggles: evaporation + forces + gravity +
 * latency_field ON; wave_propagation, coupling, gauss_projection, damping,
 * selective_damping, movement, genesis OFF. Consequences:
 *   - flux is exactly what we inject and never evolves (wave_vel == 0 and
 *     delta_j_ is zero on a fresh bridge when phase_read is skipped), so
 *     E_local is CONSTANT per voxel => the hazard
 *       p = K_EVAP_RATE * exp(-E_local / K_MANIFEST^2)
 *     is an exact constant, identical for every test voxel by construction;
 *   - movement off => test voxels cannot fall in; their latency is constant;
 *   - genesis off, evaporation on => no new matter contaminates the counts;
 *   - the locked ball contributes ONLY latency (no flux with coupling off).
 *
 * PAIRED ARMS: M (mass ball -> latency well) and F (no mass -> flat). Same
 * test-voxel indices, same dressing, same per-voxel RNG streams
 * (voxel_uniform is keyed on (seed, index, tick, stream)), so under a
 * latency-blind hazard the two arms decay VOXEL-FOR-VOXEL IDENTICALLY:
 * n_diff == 0 is a bit-level discriminator, far sharper than a rate fit.
 *
 * OUTPUT: CSV to stdout — GATE rows (validity), COHORT rows (per test voxel:
 * shell, latency, E_local), SURV rows (per arm, shell, tick: survivors), and
 * DIFF rows (per-voxel decay-tick comparison M vs F). stderr carries
 * progress. NO verdict is computed here; §4 is applied by the analyst.
 *
 * Epistemic status: [PRE-REGISTERED MEASUREMENT INSTRUMENT].
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdio>
#include <map>
#include <vector>

#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"

namespace {

// ── Pre-registered fixed parameters (prereg §2) ────────────────────────────
constexpr int    kL        = 96;
constexpr int    kSor      = 20;
constexpr int    kMassR    = 3;
constexpr int    kEquil    = 200;   // latency warm-start, before test voxels
constexpr int    kSettle   = 3;     // latency re-converge after injection
constexpr int    kTicks    = 400;   // measurement window
constexpr double kETarget  = 0.588; // E_local target => p_pred = 1.0e-2 / tick
constexpr int    kMinSep   = 2;     // test voxels never share an E_local nbhd
const int        kShells[] = {5, 7, 10, 14, 20, 30};
constexpr int    kNShells  = 6;
constexpr int    kFibPerShell = 600;  // candidate points before separation cull

struct TestVoxel {
    int idx = -1, x = 0, y = 0, z = 0, shell = 0;
    double latency = 0.0, e_local = 0.0;
    int decay_tick = -1;             // -1 = survived the window
};

inline int lat_index(int x, int y, int z) { return (x * kL + y) * kL + z; }

// E_local exactly as phase_write computes it: site + 6 face neighbours.
double e_local_at(const ftd::RenderBridge& rb, int i) {
    const auto& vox = rb.voxels();
    double e = vox[i].flux.mag2() + vox[i].wave_vel.mag2();
    for (int n : rb.lattice().neighbors_6(i))
        e += vox[n].flux.mag2() + vox[n].wave_vel.mag2();
    return e;
}

void configure_toggles(ftd::RenderBridge& rb) {
    rb.toggles.disable_all();
    rb.toggles.evaporation   = true;   // decay alone (test isolation)
    rb.toggles.forces        = true;
    rb.toggles.gravity       = true;
    rb.toggles.latency_field = true;   // the gravitational sector of record
    // genesis / wave_propagation / coupling / gauss_projection / damping /
    // selective_damping / movement stay OFF (frozen field, fixed positions).
}

int seed_mass(ftd::RenderBridge& rb) {
    const int c = kL / 2;
    int n = 0;
    for (int z = c - kMassR; z <= c + kMassR; ++z)
    for (int y = c - kMassR; y <= c + kMassR; ++y)
    for (int x = c - kMassR; x <= c + kMassR; ++x) {
        const int dx = x - c, dy = y - c, dz = z - c;
        if (dx*dx + dy*dy + dz*dz > kMassR * kMassR) continue;
        rb.inject_particle(x, y, z,
                           static_cast<int8_t>(((x + y + z) & 1) ? +1 : -1),
                           ftd::Vec3(0, 0, 0));
        rb.voxel_at(x, y, z).locked = true;
        ++n;
    }
    return n;
}

// Spherical-Fibonacci shell layout, lattice-rounded, min-separation culled.
// Deterministic; identical in both arms (depends on nothing but geometry).
std::vector<TestVoxel> plan_test_voxels() {
    const int c = kL / 2;
    std::vector<TestVoxel> out;
    std::vector<std::vector<int>> placed;  // x,y,z of accepted sites
    const double ga = M_PI * (3.0 - std::sqrt(5.0));  // golden angle
    for (int s = 0; s < kNShells; ++s) {
        const int r = kShells[s];
        for (int k = 0; k < kFibPerShell; ++k) {
            const double t  = (kFibPerShell == 1) ? 0.0
                            : 1.0 - 2.0 * k / static_cast<double>(kFibPerShell - 1);
            const double ph = std::acos(t);
            const double th = ga * k;
            const int x = c + static_cast<int>(std::lround(r * std::sin(ph) * std::cos(th)));
            const int y = c + static_cast<int>(std::lround(r * std::sin(ph) * std::sin(th)));
            const int z = c + static_cast<int>(std::lround(r * std::cos(ph)));
            if (x < 2 || x >= kL - 2 || y < 2 || y >= kL - 2 || z < 2 || z >= kL - 2) continue;
            const int dx = x - c, dy = y - c, dz = z - c;
            if (dx*dx + dy*dy + dz*dz <= (kMassR + 1) * (kMassR + 1)) continue;  // not in the ball
            bool clash = false;
            for (const auto& p : placed) {
                const int ax = std::abs(p[0] - x), ay = std::abs(p[1] - y), az = std::abs(p[2] - z);
                if (ax <= kMinSep && ay <= kMinSep && az <= kMinSep) { clash = true; break; }
            }
            if (clash) continue;
            placed.push_back({x, y, z});
            TestVoxel tv;
            tv.idx = lat_index(x, y, z);
            tv.x = x; tv.y = y; tv.z = z; tv.shell = r;
            out.push_back(tv);
        }
    }
    return out;
}

// One arm. Returns the cohort with per-voxel decay ticks filled in.
std::vector<TestVoxel> run_arm(const char* arm, bool with_mass,
                               const std::vector<TestVoxel>& plan) {
    std::fprintf(stderr, "[%s] building L=%d (mass=%d)\n", arm, kL, with_mass ? 1 : 0);
    ftd::RenderBridge rb(kL);
    rb.force_cpu();
    rb.set_sor_iterations(kSor);
    configure_toggles(rb);
    if (with_mass) {
        const int n = seed_mass(rb);
        std::fprintf(stderr, "[%s] locked ball: %d voxels\n", arm, n);
    }
    rb.run(kEquil);                       // latency warm-start, no test voxels yet

    // Inject the test cohort: unlocked +1, dressed at its OWN site only so
    // E_local == kETarget exactly and identically for every test voxel.
    const double amp = std::sqrt(kETarget);
    for (const auto& tv : plan) {
        rb.inject_particle(tv.x, tv.y, tv.z, +1, ftd::Vec3(0, 0, 0));
        rb.inject_flux_add(tv.x, tv.y, tv.z, ftd::Vec3(amp, 0, 0));
    }
    rb.run(kSettle);                      // latency re-converge

    // Cohort snapshot: survivors of the settle window, with L and E_local.
    std::vector<TestVoxel> cohort;
    for (const auto& tv : plan) {
        if (rb.voxels()[tv.idx].state == 0) continue;   // died during settle
        TestVoxel c = tv;
        c.latency = rb.voxels()[tv.idx].latency;
        c.e_local = e_local_at(rb, tv.idx);
        cohort.push_back(c);
    }
    std::fprintf(stderr, "[%s] cohort = %zu of %zu planned\n",
                 arm, cohort.size(), plan.size());

    // V2 input: flux snapshot at cohort time.
    std::vector<ftd::Vec3> flux0(cohort.size());
    for (size_t i = 0; i < cohort.size(); ++i) flux0[i] = rb.voxels()[cohort[i].idx].flux;

    // Measurement window: record survivors per shell per tick.
    std::map<int, std::vector<int>> surv;   // shell -> per-tick survivor count
    for (int s = 0; s < kNShells; ++s) surv[kShells[s]] = std::vector<int>();

    for (int t = 0; t <= kTicks; ++t) {
        std::map<int, int> alive;
        for (size_t i = 0; i < cohort.size(); ++i) {
            const bool a = rb.voxels()[cohort[i].idx].state != 0;
            if (a) alive[cohort[i].shell]++;
            else if (cohort[i].decay_tick < 0) cohort[i].decay_tick = t;
        }
        for (int s = 0; s < kNShells; ++s)
            surv[kShells[s]].push_back(alive[kShells[s]]);
        if (t < kTicks) rb.tick();
        if (t % 100 == 0) std::fprintf(stderr, "[%s] t=%d\n", arm, t);
    }

    // V2: did the frozen field actually stay frozen at the test sites?
    double max_dj = 0.0;
    for (size_t i = 0; i < cohort.size(); ++i) {
        const double d = (rb.voxels()[cohort[i].idx].flux - flux0[i]).mag();
        if (d > max_dj) max_dj = d;
    }
    std::printf("GATE,%s,max_flux_drift,%.6e\n", arm, max_dj);

    // Emit cohort + survival series.
    for (const auto& c : cohort)
        std::printf("COHORT,%s,%d,%d,%d,%d,%.9e,%.9e,%d\n",
                    arm, c.shell, c.x, c.y, c.z, c.latency, c.e_local, c.decay_tick);
    for (int s = 0; s < kNShells; ++s) {
        const auto& v = surv[kShells[s]];
        for (size_t t = 0; t < v.size(); ++t)
            std::printf("SURV,%s,%d,%zu,%d\n", arm, kShells[s], t, v[t]);
    }
    return cohort;
}

}  // namespace

int main() {
    std::printf("# campaign_two_clock_consistency — PREREG_TWO_CLOCK_CONSISTENCY_v1\n");
    std::printf("# K_MANIFEST=%.16f  K_EVAP_RATE=%.4f  E_target=%.6f\n",
                ftd::K_MANIFEST, ftd::K_EVAP_RATE, kETarget);
    const double p_pred = ftd::K_EVAP_RATE
                        * std::exp(-kETarget / (ftd::K_MANIFEST * ftd::K_MANIFEST));
    std::printf("# p_pred = %.9e per tick (uniform across all test voxels by design)\n", p_pred);
    std::printf("# COHORT: arm,shell,x,y,z,latency,e_local,decay_tick | SURV: arm,shell,tick,alive\n");
    std::printf("GATE,ALL,p_pred,%.9e\n", p_pred);

    const auto plan = plan_test_voxels();
    std::fprintf(stderr, "[plan] %zu test voxels across %d shells\n", plan.size(), kNShells);

    const auto m = run_arm("M", /*with_mass=*/true,  plan);
    const auto f = run_arm("F", /*with_mass=*/false, plan);

    // Bit-level discriminator: per-voxel decay-tick identity between arms.
    std::map<int, int> n_diff, n_pair;
    std::map<int, double> lat_sum;
    size_t im = 0, iff = 0;
    while (im < m.size() && iff < f.size()) {
        if (m[im].idx < f[iff].idx) { ++im; continue; }
        if (f[iff].idx < m[im].idx) { ++iff; continue; }
        n_pair[m[im].shell]++;
        lat_sum[m[im].shell] += m[im].latency;
        if (m[im].decay_tick != f[iff].decay_tick) {
            n_diff[m[im].shell]++;
            std::printf("DIFF,%d,%d,%d,%d,%.9e\n",
                        m[im].shell, m[im].idx, m[im].decay_tick, f[iff].decay_tick,
                        m[im].latency);
        }
        ++im; ++iff;
    }
    for (int s = 0; s < kNShells; ++s) {
        const int sh = kShells[s];
        const int np = n_pair[sh];
        const double lbar = np ? lat_sum[sh] / np : 0.0;
        std::printf("PAIRSUM,%d,%d,%d,%.9e,%.9e\n",
                    sh, np, n_diff[sh], lbar, std::sqrt(std::max(0.0, 1.0 - lbar * lbar)));
    }
    std::printf("# done — verdict applied against PREREG §4 by the analyst, not here\n");
    return 0;
}
