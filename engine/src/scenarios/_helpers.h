#pragma once
// ==========================================================================
//  engine/src/scenarios/_helpers.h
//
//  Private (non-installed) helper header shared by the split scenario
//  group files (flux.cpp, light.cpp, quantum.cpp, s0_seed.cpp, s0_field.cpp).
//
//  All symbols live in an anonymous namespace so each translation unit gets
//  its own internal-linkage copy — this keeps the split ABI-clean and lets
//  each .cpp evolve independently without touching a public header.
//
//  Origin: these helpers previously lived in engine/src/scenarios.cpp.
//  They were extracted verbatim as part of ticket S1 (scenarios.cpp split).
//
//  The shared RNG (urand / reset_rng / SCN_RNG_SEED) is DEFINED in
//  engine/src/scenarios.cpp next to dispatch_scenario() (which resets it
//  before each run) but DECLARED here in ftd::detail so the stochastic
//  scenarios (flux-random-genesis, flux-thermalization, flux-vacuum-foam,
//  flux-zero-point, quantum-born-rule, quantum-casimir) can call urand()
//  across TU boundaries.
// ==========================================================================

#include "ftd/render_bridge.h"
#include "ftd/voxel.h"

#include <cmath>

namespace ftd {

// ── Shared stochastic RNG (defined in scenarios.cpp) ───────────────
// External linkage; lives in ftd::detail so it doesn't pollute the
// public ftd namespace while still being reachable from sibling TUs.
namespace detail {
double urand();
void   reset_scenario_rng();
}  // namespace detail

namespace {

// ── Injection helpers (match JS argument order) ────────────────────
inline void IF(RenderBridge& rb, int x, int y, int z, double fx, double fy, double fz) {
    rb.inject_flux_add(x, y, z, Vec3(fx, fy, fz));
}
inline void IW(RenderBridge& rb, int x, int y, int z, double wx, double wy, double wz) {
    rb.inject_wave_vel_add(x, y, z, Vec3(wx, wy, wz));
}
inline void IP(RenderBridge& rb, int x, int y, int z, int state) {
    rb.inject_particle(x, y, z, static_cast<int8_t>(state), Vec3(0, 0, 0));
}
inline void IPF(RenderBridge& rb, int x, int y, int z, int state, int spin, int color) {
    rb.inject_particle(x, y, z, static_cast<int8_t>(state), Vec3(0, 0, 0),
                       static_cast<int8_t>(spin), static_cast<int8_t>(color));
}

// Mutate a just-injected particle at (x,y,z).
inline void SET_VEL(RenderBridge& rb, int x, int y, int z, double vx, double vy, double vz) {
    rb.voxel_at(x, y, z).velocity = Vec3(vx, vy, vz);
}
inline void LOCK(RenderBridge& rb, int x, int y, int z) {
    rb.voxel_at(x, y, z).locked = true;
}
inline void SET_SPIN(RenderBridge& rb, int x, int y, int z, int spin) {
    rb.voxel_at(x, y, z).spin = static_cast<int8_t>(spin);
}

// ── Math shims to keep the ported JS readable ──────────────────────
inline int    FLR(double d) { return static_cast<int>(std::floor(d)); }
inline int    CEL(double d) { return static_cast<int>(std::ceil(d)); }
inline int    RND(double d) { return static_cast<int>(std::round(d)); }

// ── Common scenario injection harnesses ─────────────────────────────
inline void dp(RenderBridge& rb, int cx, int cy, int cz,
               int st, int sp, int co, double sig, double amp, bool lock) {
    IPF(rb, cx, cy, cz, st, sp, (co >= 0) ? co : 0);
    if (lock) LOCK(rb, cx, cy, cz);
    int sn = (st > 0) ? 1 : -1;
    int eR = CEL(3.0 * sig);
    for (int dz2 = -eR; dz2 <= eR; dz2++) for (int dy2 = -eR; dy2 <= eR; dy2++) for (int dx2 = -eR; dx2 <= eR; dx2++) {
        if (dx2 == 0 && dy2 == 0 && dz2 == 0) continue;
        double r22 = dx2*dx2 + dy2*dy2 + dz2*dz2;
        double rr = std::sqrt(r22);
        if (rr > 3.0 * sig) continue;
        double gg = amp * std::exp(-r22 / (2.0 * sig * sig));
        if (gg < 0.001) continue;
        IF(rb, cx+dx2, cy+dy2, cz+dz2, sn*gg*dx2/rr, sn*gg*dy2/rr, sn*gg*dz2/rr);
    }
}

inline void tri(RenderBridge& rb, int cx, int cy, int cz,
                const int charges[3], const int colors[3], int rad, bool lock) {
    for (int k = 0; k < 3; k++) {
        double ang = (2.0 * PI * k) / 3.0;
        int qx = RND(cx + rad * std::cos(ang));
        int qy = RND(cy + rad * std::sin(ang));
        dp(rb, qx, qy, cz, charges[k], (k % 2 == 0) ? 1 : -1, colors[k], 2, 0.511 * 0.5, lock);
    }
}
// π lives in ftd:: via `using ontic::PI;` in ftd/constants.h — every
// scenario .cpp already includes constants.h, so call sites use `PI`
// directly without re-defining a SCN_PI alias here.

// Vacuum environment — mirror of JS applyVacuumEnvironment(bridge, ctx).
// In v1 this is a no-op (RenderBridge::reset() is invoked by the caller);
// kept as the single extension point for a future absorbing_boundary toggle.
inline void apply_vacuum_environment(RenderBridge& rb) {
    (void)rb;
}

}  // namespace
}  // namespace ftd
