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
//  quantum-born-rule, quantum-casimir) can call urand() across TU boundaries.
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
constexpr double SCN_PI = 3.14159265358979323846;

}  // namespace
}  // namespace ftd
