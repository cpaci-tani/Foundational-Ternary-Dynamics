/**
 * @file campaign_hedgehog_charge_robustness.cpp
 * @brief Is the hedgehog topological charge of the flux field robust
 *        across birth circumstances that produced a 9.2x energy spread?
 *
 * Implements the measurement registered by
 *   docs/theory/10_eft_program/preregistrations/framework_boundary_imports_consumption/PREREG_HEDGEHOG_CHARGE_ROBUSTNESS_v1.md
 * (tag `preregister-hedgehog-charge-robustness-v1`).
 *
 * THE PRE-REGISTRATION IS CANONICAL. This file is a measurement
 * instrument built to its spec and locked in the same commit; it makes
 * no claim, promotes no tag, and alters no engine physics.
 *
 * Reuses the three seeds from campaign_manifestation_seed_diversity.cpp
 * that passed every gate cleanly (A_baseline, C_hot, E_cold), plus a
 * fourth synthetic-charge reference arm reproducing the FTD-0388
 * self-energy pinning setup. At freeze time (or, for the synthetic arm,
 * at the relaxed fixed point), computes the Berg-Luscher discrete
 * topological (hedgehog) charge Q on the octahedral Moore-shell (the 6
 * face-neighbors) around the relevant voxel, using the formula
 * validated pre-lock in scripts/exploration/validate_hedgehog_charge.py.
 *
 * OUTPUT: CSV to stdout (GATE rows for validity, ARM rows for results).
 * stderr carries progress. NO verdict computed here; the prereg's frozen
 * bands are applied afterward by the analyst.
 *
 * Deterministic; CPU-forced.
 *
 * Epistemic status: [PRE-REGISTERED MEASUREMENT INSTRUMENT].
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdio>
#include <vector>

#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"

namespace {

constexpr int    kL            = 17;
constexpr int    kMaxWait      = 200;
constexpr double kResidualGate = 1e-8;
constexpr int    kResidualCap  = 5000;
constexpr int    kSorItersRelax = 6;
constexpr double kFieldFloor   = 1e-6;  // V1: |J| below this -> direction undefined

struct SeedSpec {
    const char* name;
    double ox, oy, oz;
    double amp, sigma, cutR;
    double known_e_half;  // V2 reproducibility target -- see v1.1 amendment
};

// Only the three arms that passed every gate cleanly in
// PREREG_MANIFESTATION_SEED_DIVERSITY_v1 (B_position VOIDed on no
// manifestation, D_broad failed its own V1 -- neither is reused here).
//
// v1.1 (see PREREG_HEDGEHOG_CHARGE_ROBUSTNESS_v1.md amendment): this
// instrument measures Q at FREEZE time, i.e. before relaxation -- the
// correct V2 target is therefore the original campaign's
// `e_half_prerelax` field, not its final (post-relaxation) `e_half`.
// v1 wrongly used the post-relaxation values; corrected here.
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

// -------------------------------------------------------------------
// Berg-Luscher discrete topological charge on FTD's octahedral
// (Moore k=1) shell -- validated pre-lock in
// scripts/exploration/validate_hedgehog_charge.py (6/6 known-answer
// test cases: rotation-invariant, magnitude-invariant, correct sign,
// correct null on trivial fields).
// -------------------------------------------------------------------

double solid_angle(const ftd::Vec3& ni, const ftd::Vec3& nj, const ftd::Vec3& nk) {
    const double numerator = ni.dot(ftd::Vec3::cross(nj, nk));
    const double denominator = 1.0 + ni.dot(nj) + nj.dot(nk) + nk.dot(ni);
    return 2.0 * std::atan2(numerator, denominator);
}

struct HedgehogResult {
    bool   valid;
    double Q;
    double rms_angular_deviation_deg;  // vs. pure-radial reference, diagnostic only
};

// The 6 face-neighbor offsets, in a fixed order, with the outward-consistent
// octahedron triangulation from validate_hedgehog_charge.py: (x,z,y) order
// for an odd number of minus signs, (x,y,z) order for even -- verified by
// direct cross-product check there, ported verbatim.
HedgehogResult hedgehog_charge(const ftd::RenderBridge& rb, int cx, int cy, int cz) {
    const int L = rb.lattice().size();
    const auto& v = rb.voxels();
    auto idx = [&](int x, int y, int z) { return (x * L + y) * L + z; };

    // Boundary safety: need all 6 face-neighbors in-bounds.
    if (cx < 1 || cx > L - 2 || cy < 1 || cy > L - 2 || cz < 1 || cz > L - 2) {
        return {false, 0.0, 0.0};
    }

    const ftd::Vec3 raw[6] = {
        v[idx(cx + 1, cy, cz)].flux,  // +x
        v[idx(cx - 1, cy, cz)].flux,  // -x
        v[idx(cx, cy + 1, cz)].flux,  // +y
        v[idx(cx, cy - 1, cz)].flux,  // -y
        v[idx(cx, cy, cz + 1)].flux,  // +z
        v[idx(cx, cy, cz - 1)].flux,  // -z
    };
    const ftd::Vec3 ideal_dir[6] = {
        {1,0,0}, {-1,0,0}, {0,1,0}, {0,-1,0}, {0,0,1}, {0,0,-1},
    };

    ftd::Vec3 n[6];
    for (int i = 0; i < 6; ++i) {
        const double mag = raw[i].mag();
        if (mag < kFieldFloor) return {false, 0.0, 0.0};
        n[i] = raw[i] * (1.0 / mag);
    }
    // indices: 0=+x 1=-x 2=+y 3=-y 4=+z 5=-z
    // Build the 8 octahedron faces ALGORITHMICALLY, mirroring
    // scripts/exploration/validate_hedgehog_charge.py::_face() line for
    // line (same sx/sy/sz loop nesting, same "odd minus-count -> swap
    // y/z" parity rule) rather than a hand-transcribed lookup table --
    // a hand-copied table for exactly this construction already produced
    // one wrong index during development; recomputing from the same
    // algorithm as the validated reference removes that failure mode.
    double total = 0.0;
    for (int sx = 0; sx < 2; ++sx)        // 0 = '+', 1 = '-'
    for (int sy = 0; sy < 2; ++sy)
    for (int sz = 0; sz < 2; ++sz) {
        const int idx_x = (sx == 0) ? 0 : 1;
        const int idx_y = (sy == 0) ? 2 : 3;
        const int idx_z = (sz == 0) ? 4 : 5;
        const int n_minus = sx + sy + sz;
        total += (n_minus % 2 == 1)
            ? solid_angle(n[idx_x], n[idx_z], n[idx_y])   // odd parity swap
            : solid_angle(n[idx_x], n[idx_y], n[idx_z]);  // even parity
    }
    const double Q = total / (4.0 * M_PI);

    double sum_sq_dev = 0.0;
    for (int i = 0; i < 6; ++i) {
        double c = n[i].dot(ideal_dir[i]);
        c = std::max(-1.0, std::min(1.0, c));
        const double dev_deg = std::acos(c) * 180.0 / M_PI;
        sum_sq_dev += dev_deg * dev_deg;
    }
    const double rms_dev = std::sqrt(sum_sq_dev / 6.0);

    return {true, Q, rms_dev};
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
        if (!std::isfinite(cur) || cur > 1e12) { prev = cur; ++apps; break; }
        if (std::fabs(cur - prev) < kResidualGate) { prev = cur; ++apps; break; }
        prev = cur;
    }
    std::printf("GATE,%s,relax_applications,%d\n", tag, apps);
    std::printf("GATE,%s,V3_relax_converged,%d\n", tag, (apps < kResidualCap) ? 1 : 0);
    return apps;
}

}  // namespace

int main() {
    std::printf("# campaign_hedgehog_charge_robustness -- PREREG_HEDGEHOG_CHARGE_ROBUSTNESS_v1\n");
    std::printf("# L=%d  K_GENESIS=%.10f  K_MANIFEST=%.16f\n", kL, ftd::K_GENESIS, ftd::K_MANIFEST);

    // --- Arms A/C/E: dynamical seeds, measure Q at freeze ---
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
        const double e_err = std::fabs(e - spec.known_e_half);
        std::printf("GATE,%s,V2_e_half_matches_known,%d\n", spec.name, (e_err < 1e-9) ? 1 : 0);
        std::printf("GATE,%s,e_half_measured,%.12f,e_half_known,%.12f\n", spec.name, e, spec.known_e_half);

        int mx, my, mz;
        if (!find_manifested_site(rb, mx, my, mz)) {
            std::printf("GATE,%s,VOID_site_not_found,1\n", spec.name);
            continue;
        }
        const auto hh = hedgehog_charge(rb, mx, my, mz);
        std::printf("GATE,%s,V1_field_defined_on_shell,%d\n", spec.name, hh.valid ? 1 : 0);
        if (!hh.valid) {
            std::printf("GATE,%s,VOID_field_undefined_on_shell,1\n", spec.name);
            std::fprintf(stderr, "[%s] VOID: |J| below floor at a shell vertex\n", spec.name);
            continue;
        }
        std::printf("ARM,%s,Q,%.10f,rms_angular_deviation_deg,%.6f,e_half,%.12f,fire_tick,%d\n",
                    spec.name, hh.Q, hh.rms_angular_deviation_deg, e, fire_tick);
        std::fprintf(stderr, "[%s] Q=%.6f  rms_dev=%.3f deg  e_half=%.6f  fire_tick=%d\n",
                     spec.name, hh.Q, hh.rms_angular_deviation_deg, e, fire_tick);
    }

    // --- Arm S: synthetic single charge, Gauss-projection-only relaxation
    //     (FTD-0388 self-energy pinning setup) ---
    {
        const int cx = kL / 2, cy = kL / 2, cz = kL / 2;
        ftd::RenderBridge seed(kL);
        seed.force_cpu();
        seed.toggles.disable_all();
        seed.set_state(cx, cy, cz, 1);

        ftd::RenderBridge relaxed(kL);
        relaxed.force_cpu();
        relax_to_fixed_point(seed, relaxed, "S_synthetic");

        const double e = e_half(relaxed);
        const auto hh = hedgehog_charge(relaxed, cx, cy, cz);
        std::printf("GATE,S_synthetic,V1_field_defined_on_shell,%d\n", hh.valid ? 1 : 0);
        if (!hh.valid) {
            std::printf("GATE,S_synthetic,VOID_field_undefined_on_shell,1\n");
            std::fprintf(stderr, "[S_synthetic] VOID: |J| below floor at a shell vertex\n");
        } else {
            std::printf("ARM,S_synthetic,Q,%.10f,rms_angular_deviation_deg,%.6f,e_half,%.12f,fire_tick,-1\n",
                        hh.Q, hh.rms_angular_deviation_deg, e);
            std::fprintf(stderr, "[S_synthetic] Q=%.6f  rms_dev=%.3f deg  e_half=%.6f\n",
                         hh.Q, hh.rms_angular_deviation_deg, e);
        }
    }

    std::printf("# done -- frozen-band verdict applied against PREREG_HEDGEHOG_CHARGE_ROBUSTNESS_v1 by the analyst\n");
    return 0;
}
