// ==========================================================================
//  engine/src/scenarios.cpp
//
//  C++ port of the Scale-0 scenario library that was previously JS-only on
//  the MockBridge. See engine/include/ftd/scenarios.h for the header + the
//  motivation block on why this exists.
//
//  This is a pure, mechanical port from engine/web/js/bridge/scenarios/*.js.
//  Every scenario body is preserved 1-for-1 (same math, same loop shapes,
//  same thresholds) — the only translation work is:
//
//    this._injectFlux(...)      →  rb.inject_flux_add(x,y,z, Vec3(fx,fy,fz))
//    this._injectWaveVel(...)   →  rb.inject_wave_vel_add(x,y,z, Vec3(...))
//    this.injectParticle(...)   →  rb.inject_particle(x,y,z, s, Vec3(0,0,0))
//    this._particles[i].vx = v  →  rb.voxel_at(x,y,z).velocity.x = v
//    this._particles[i].locked  →  rb.voxel_at(x,y,z).locked = true
//    this._toggles.genesis = T  →  rb.toggles.genesis = true
//    Math.random()              →  uniform01(rng) from std::mt19937
//
//  Helpers IF / IW / IP / IPF wrap the inject_* calls to keep the ported
//  code visually aligned with the JS source (one C++ line per JS line).
//
//  Groups ported (83 scenarios total, matching the JS dispatcher layout):
//    flux-*    : 20 scenarios   ← js/bridge/scenarios/flux-scenarios.js
//    light-*   :  4 scenarios   ← js/bridge/scenarios/light-scenarios.js
//    quantum-* :  8 scenarios   ← js/bridge/scenarios/quantum-scenarios.js
//    s0-seed-* : 49 scenarios   ← js/bridge/scenarios/s0-seed-scenarios.js
//    s0-field-*:  8 scenarios   ← js/bridge/scenarios/s0-field-scenarios.js
// ==========================================================================

#include "ftd/scenarios.h"
#include "ftd/render_bridge.h"
#include "ftd/constants.h"
// Vec3 comes from voxel.h (there is no separate vec3.h).
#include "ftd/voxel.h"

#include <cmath>
#include <random>
#include <string>

namespace ftd {
namespace {

// ── Shared RNG ──────────────────────────────────────────────────────
// Used by scenarios that call Math.random() in JS (flux-random-genesis,
// flux-thermalization, flux-vacuum-foam, quantum-born-rule, quantum-casimir).
// Thread-local so each worker thread gets its own state; reset_rng() is
// invoked at the top of dispatch_scenario() below so repeated setupScenario
// calls produce a reproducible sequence within a single process run.
//
// NOTE: JS Math.random() is not seedable, so JS↔C++ parity for the 5
// stochastic scenarios is statistical (same distribution), not bit-exact.
// The fixed seed ensures repeatability within one WASM process run —
// important for snapshot tests.
constexpr std::uint_fast32_t SCN_RNG_SEED = 0xC0DEFACE;
thread_local std::mt19937 g_rng{SCN_RNG_SEED};
thread_local std::uniform_real_distribution<double> g_uniform01{0.0, 1.0};

inline double urand() { return g_uniform01(g_rng); }
inline void reset_rng() {
    g_rng.seed(SCN_RNG_SEED);
    g_uniform01.reset();
}

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

// ==========================================================================
//  Group: flux-* (20 scenarios)
//  JS source: engine/web/js/bridge/scenarios/flux-scenarios.js
// ==========================================================================
bool setup_flux_scenario(RenderBridge& rb, const std::string& name) {
    if (name.rfind("flux-", 0) != 0) return false;
    const int    N     = rb.lattice().size();
    const int    mid   = N / 2;
    const double midF  = (N - 1) * 0.5;
    const double sigma = N / 10.0;
    const double amp   = K_B * 2.0;

    if (name == "flux-pulse") {
        const int pulseR = std::min(CEL(sigma * 3), FLR(midF));
        const int pLo = FLR(midF) - pulseR, pHi = CEL(midF) + pulseR;
        for (int z = pLo; z <= pHi; z++) for (int y = pLo; y <= pHi; y++) for (int x = pLo; x <= pHi; x++) {
            double dx = x - midF, dy = y - midF, dz = z - midF;
            double val = amp * std::exp(-(dx*dx+dy*dy+dz*dz) / (2 * sigma * sigma));
            if (val > 0.001) IF(rb, x, y, z, val, 0, 0);
        }
    }
    else if (name == "flux-dipole") {
        const int off = N / 4;
        const int pLx = FLR(midF) - off, pRx = CEL(midF) + off;
        const int yzLo = FLR(midF) - 4, yzHi = CEL(midF) + 4;
        for (int z = yzLo; z <= yzHi; z++) for (int y = yzLo; y <= yzHi; y++) for (int dx = -4; dx <= 4; dx++) {
            double dy = y - midF, dz = z - midF;
            double val = amp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * 9.0));
            if (val > 0.001) {
                IF(rb, pLx + dx, y, z,  val,  val * 0.5, 0);
                IF(rb, pRx + dx, y, z, -val, -val * 0.5, 0);
            }
        }
    }
    else if (name == "flux-standing") {
        const int off = N / 3;
        const int pLx = FLR(midF) - off, pRx = CEL(midF) + off;
        const int yzLo = FLR(midF) - 4, yzHi = CEL(midF) + 4;
        for (int z = yzLo; z <= yzHi; z++) for (int y = yzLo; y <= yzHi; y++) for (int dx = -4; dx <= 4; dx++) {
            double dy = y - midF, dz = z - midF;
            double val = amp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * 9.0));
            if (val > 0.001) {
                IF(rb, pLx + dx, y, z, val, 0, 0);
                IF(rb, pRx + dx, y, z, val, 0, 0);
            }
        }
    }
    else if (name == "flux-soliton") {
        const int sLo = FLR(midF) - 3, sHi = CEL(midF) + 3;
        for (int z = sLo; z <= sHi; z++) for (int y = sLo; y <= sHi; y++) for (int x = sLo; x <= sHi; x++) {
            double dx = x - midF, dy = y - midF, dz = z - midF;
            double val = amp * 10.0 * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * 4.0));
            if (val > 0.001) IF(rb, x, y, z, val, val, 0);
        }
    }
    else if (name == "flux-cascade") {
        const double bigAmp = K_GENESIS * 3.0;
        const int cLo = FLR(midF) - 3, cHi = CEL(midF) + 3;
        for (int z = cLo; z <= cHi; z++) for (int y = cLo; y <= cHi; y++) for (int x = cLo; x <= cHi; x++) {
            double dx = x - midF, dy = y - midF, dz = z - midF;
            double val = bigAmp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * 4.0));
            if (val > 0.001) IF(rb, x, y, z, val, 0, val * 0.5);
        }
    }
    else if (name == "flux-annihilation") {
        const int off = N / 3;
        const int pL = FLR(midF) - off, pR = CEL(midF) + off;
        const int mc = RND(midF);
        IP(rb, pL, mc, mc,  1);
        IP(rb, pR, mc, mc, -1);
        IP(rb, mc, mc, pL, -1);
        IP(rb, mc, mc, pR,  1);
        const double pushAmp = amp * 2.0;
        const int kLo = FLR(midF) - 3, kHi = CEL(midF) + 3;
        for (int z = kLo; z <= kHi; z++) for (int y = kLo; y <= kHi; y++) for (int x = kLo; x <= kHi; x++) {
            double dy = y - midF, dz = z - midF;
            double dxL = x - pL, dxR = x - pR;
            double valL = pushAmp * std::exp(-(dxL*dxL + dy*dy + dz*dz) / (2.0 * 4.0));
            double valR = pushAmp * std::exp(-(dxR*dxR + dy*dy + dz*dz) / (2.0 * 4.0));
            if (valL > 0.001) IF(rb, x, y, z,  valL, 0, 0);
            if (valR > 0.001) IF(rb, x, y, z, -valR, 0, 0);
            double dzL = z - pL, dzR = z - pR, dx0 = x - mc;
            double valZL = pushAmp * std::exp(-(dx0*dx0 + dy*dy + dzL*dzL) / (2.0 * 4.0));
            double valZR = pushAmp * std::exp(-(dx0*dx0 + dy*dy + dzR*dzR) / (2.0 * 4.0));
            if (valZL > 0.001) IF(rb, x, y, z, 0, 0,  valZL);
            if (valZR > 0.001) IF(rb, x, y, z, 0, 0, -valZR);
        }
    }
    else if (name == "flux-pair-production") {
        const double bigAmp = K_GENESIS * 5.0;
        const int ppLo = FLR(midF) - 4, ppHi = CEL(midF) + 4;
        for (int z = ppLo; z <= ppHi; z++) for (int y = ppLo; y <= ppHi; y++) for (int x = ppLo; x <= ppHi; x++) {
            double dx = x - midF, dy = y - midF, dz = z - midF;
            double val = bigAmp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * 6.0));
            if (val > 0.001) IF(rb, x, y, z, val, val * 0.7, val * 0.3);
        }
    }
    else if (name == "flux-interference") {
        const int q = N / 4;
        const int qL = FLR(midF) - q, qR = CEL(midF) + q;
        const int mc = RND(midF);
        const int sources[4][3] = { {qL, mc, qL}, {qR, mc, qL}, {qL, mc, qR}, {qR, mc, qR} };
        for (int s = 0; s < 4; s++) {
            int sx = sources[s][0], sy = sources[s][1], sz = sources[s][2];
            for (int dz = -4; dz <= 4; dz++) for (int dy = -4; dy <= 4; dy++) for (int dx = -4; dx <= 4; dx++) {
                double val = amp * 1.5 * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * 6.0));
                if (val > 0.001) IF(rb, sx + dx, sy + dy, sz + dz, val, 0, 0);
            }
        }
    }
    else if (name == "flux-vortex") {
        const int vRadius = N / 5;
        const int nV = 24;
        const int mc = RND(midF);
        for (int i = 0; i < nV; i++) {
            double angle = (2.0 * SCN_PI * i) / nV;
            int rx = RND(midF + vRadius * std::cos(angle));
            int rz = RND(midF + vRadius * std::sin(angle));
            double tX = -std::sin(angle) * amp * 2.0;
            double tZ =  std::cos(angle) * amp * 2.0;
            double tY =  amp * 0.5;
            IF(rb, rx, mc,     rz, tX,        tY,        tZ);
            IF(rb, rx, mc + 1, rz, tX * 0.5,  tY * 0.5,  tZ * 0.5);
            IF(rb, rx, mc - 1, rz, tX * 0.5, -tY * 0.5,  tZ * 0.5);
        }
    }
    else if (name == "flux-dual-substrate") {
        const int off = N / 4;
        const int pLx = FLR(midF) - off, pRx = CEL(midF) + off;
        const int yzLo = FLR(midF) - 5, yzHi = CEL(midF) + 5;
        for (int z = yzLo; z <= yzHi; z++) for (int y = yzLo; y <= yzHi; y++) for (int dx = -5; dx <= 5; dx++) {
            double dy = y - midF, dz = z - midF;
            double val = amp * 1.5 * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * 8.0));
            if (val > 0.001) {
                IF(rb, pLx + dx, y, z, val,  val * 0.5, -val * 0.3);
                IF(rb, pRx + dx, y, z, val, -val * 0.5,  val * 0.3);
            }
        }
    }
    else if (name == "flux-random-genesis") {
        const int nPatches = 8;
        const double threshold = K_GENESIS * 2.5;
        for (int p = 0; p < nPatches; p++) {
            int cx = int(urand() * (N - 8)) + 4;
            int cy = int(urand() * (N - 8)) + 4;
            int cz = int(urand() * (N - 8)) + 4;
            double pAmp = threshold * (0.8 + urand() * 0.8);
            for (int dz = -2; dz <= 2; dz++) for (int dy = -2; dy <= 2; dy++) for (int dx = -2; dx <= 2; dx++) {
                double val = pAmp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * 3.0));
                if (val > 0.001) {
                    double sx = (urand() - 0.5) * val;
                    double sy = (urand() - 0.5) * val;
                    double sz = (urand() - 0.5) * val;
                    IF(rb, cx + dx, cy + dy, cz + dz, sx, sy, sz);
                }
            }
        }
    }
    // ── QCD scenarios ──
    else if (name == "flux-meson") {
        const int mOff = std::max(2, N / 8);
        const int mDress = std::max(2, N / 10);
        const int mL = FLR(midF) - mOff, mR = CEL(midF) + mOff;
        const int mc = RND(midF);
        IP(rb, mL, mc, mc,  1);
        SET_VEL(rb, mL, mc, mc, 0, 0.05, 0);
        IP(rb, mR, mc, mc, -1);
        SET_VEL(rb, mR, mc, mc, 0, -0.05, 0);
        const double mesonAmp = K_B * 1.5;
        const double mSigma2 = mDress * mDress;
        const int myzLo = FLR(midF) - mDress, myzHi = CEL(midF) + mDress;
        for (int z = myzLo; z <= myzHi; z++) for (int y = myzLo; y <= myzHi; y++) for (int dx = -mDress; dx <= mDress; dx++) {
            double dy = y - midF, dz = z - midF;
            double val = mesonAmp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2 * mSigma2));
            if (val > 0.001) {
                IF(rb, mL + dx, y, z,  val, 0, 0);
                IF(rb, mR + dx, y, z, -val, 0, 0);
            }
        }
    }
    else if (name == "flux-string-breaking") {
        const int sbOff = std::max(2, N / 10);
        const int sbDress = std::max(2, N / 8);
        const int sbL = FLR(midF) - sbOff, sbR = CEL(midF) + sbOff;
        const int mc = RND(midF);
        IP(rb, sbL, mc, mc,  1);
        SET_VEL(rb, sbL, mc, mc, -0.3, 0, 0);
        IP(rb, sbR, mc, mc, -1);
        SET_VEL(rb, sbR, mc, mc,  0.3, 0, 0);
        const double sbAmp = K_B * 3.0;
        const int sbLo = FLR(midF) - sbDress, sbHi = CEL(midF) + sbDress;
        for (int z = sbLo; z <= sbHi; z++) for (int y = sbLo; y <= sbHi; y++) for (int x = sbLo; x <= sbHi; x++) {
            double dx = x - midF, dy = y - midF, dz = z - midF;
            double val = sbAmp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * sbDress));
            if (val > 0.001) IF(rb, x, y, z, val, val * 0.3, 0);
        }
    }
    else if (name == "flux-baryon") {
        const int bR = N / 6;
        const int mc = RND(midF);
        for (int k = 0; k < 3; k++) {
            double angle = (2.0 * SCN_PI * k) / 3.0;
            int bx = RND(midF + bR * std::cos(angle));
            int bz = RND(midF + bR * std::sin(angle));
            IP(rb, bx, mc, bz, 1);
            SET_VEL(rb, bx, mc, bz, -0.04 * std::sin(angle), 0, 0.04 * std::cos(angle));
        }
        int bSea = std::max(1, bR / 2);
        IP(rb, mc + bSea, mc + bSea, mc, -1);
        const int bLo = FLR(midF) - 3, bHi = CEL(midF) + 3;
        for (int z = bLo; z <= bHi; z++) for (int y = bLo; y <= bHi; y++) for (int x = bLo; x <= bHi; x++) {
            double dx = x - midF, dy = y - midF, dz = z - midF;
            double val = amp * 0.5 * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * 4.0));
            if (val > 0.001) IF(rb, x, y, z, val, 0, val * 0.3);
        }
    }
    else if (name == "flux-nested-standing") {
        const int offX = N / 3, offZ = N / 4;
        const int xL = FLR(midF) - offX, xR = CEL(midF) + offX;
        const int zL = FLR(midF) - offZ, zR = CEL(midF) + offZ;
        const int yzLo = FLR(midF) - 4, yzHi = CEL(midF) + 4;
        for (int z = yzLo; z <= yzHi; z++) for (int y = yzLo; y <= yzHi; y++) for (int dx = -4; dx <= 4; dx++) {
            double dy = y - midF, dz = z - midF;
            double val = amp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * 9.0));
            if (val > 0.001) {
                IF(rb, xL + dx, y, z, val, 0, 0);
                IF(rb, xR + dx, y, z, val, 0, 0);
            }
        }
        for (int x = yzLo; x <= yzHi; x++) for (int y = yzLo; y <= yzHi; y++) for (int dz = -4; dz <= 4; dz++) {
            double dx = x - midF, dy = y - midF;
            double val = amp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * 9.0));
            if (val > 0.001) {
                IF(rb, x, y, zL + dz, 0, 0, val);
                IF(rb, x, y, zR + dz, 0, 0, val);
            }
        }
    }
    // ── Experiment scenarios (from test suite) ──
    else if (name == "flux-cyclotron") {
        const double bAmp = amp * 0.15;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double cx = x - mid, cy = y - mid;
            IF(rb, x, y, z, -bAmp * cy * 0.05, bAmp * cx * 0.05, 0);
        }
        IP(rb, mid, mid, mid, 1);
        for (int d = -3; d <= 3; d++) for (int dy = -3; dy <= 3; dy++) for (int dx = -3; dx <= 3; dx++) {
            double val = amp * std::exp(-(dx*dx + dy*dy + d*d) / (2.0 * 4.0));
            if (val > 0.001) IF(rb, mid + dx, mid + dy, mid + d, val * 0.5, 0, 0);
        }
    }
    else if (name == "flux-screening") {
        const int shellR = N / 5;
        IP(rb, mid, mid, mid, 1);
        const int scOff[6][3] = {
            {shellR,0,0},{-shellR,0,0},{0,shellR,0},{0,-shellR,0},{0,0,shellR},{0,0,-shellR}
        };
        for (int s = 0; s < 6; s++) IP(rb, mid + scOff[s][0], mid + scOff[s][1], mid + scOff[s][2], -1);
        const int scDress = std::max(3, int(shellR * 0.8));
        const int scDress2 = scDress * scDress;
        for (int dz = -scDress; dz <= scDress; dz++) for (int dy = -scDress; dy <= scDress; dy++) for (int dx = -scDress; dx <= scDress; dx++) {
            int r2 = dx*dx + dy*dy + dz*dz;
            if (r2 == 0 || r2 > scDress2) continue;
            double r = std::sqrt(double(r2));
            double val = amp * 0.5 / r;
            IF(rb, mid + dx, mid + dy, mid + dz, val * dx / r, val * dy / r, val * dz / r);
        }
    }
    else if (name == "flux-triad") {
        const int tR = N / 6;
        const double triAng[3] = { 0, 2 * SCN_PI / 3, 4 * SCN_PI / 3 };
        for (int t = 0; t < 3; t++) {
            double angle = triAng[t];
            int px = mid + RND(tR * std::cos(angle));
            int pz = mid + RND(tR * std::sin(angle));
            IP(rb, px, mid, pz, 1);
            for (int dx = -3; dx <= 3; dx++) for (int dy = -3; dy <= 3; dy++) for (int dz = -3; dz <= 3; dz++) {
                double val = amp * 0.5 * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * 4.0));
                if (val > 0.001) {
                    double toCX = (mid - (px + dx));
                    double toCZ = (mid - (pz + dz));
                    double dist = std::sqrt(toCX * toCX + toCZ * toCZ);
                    if (dist < 1.0) dist = 1.0;
                    IF(rb, px + dx, mid + dy, pz + dz, val * toCX / dist, 0, val * toCZ / dist);
                }
            }
        }
    }
    else if (name == "flux-thermalization") {
        const int corner = N / 4;
        const double thermAmp = amp * 3.0;
        for (int dz = -4; dz <= 4; dz++) for (int dy = -4; dy <= 4; dy++) for (int dx = -4; dx <= 4; dx++) {
            double val = thermAmp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * 6.0));
            if (val > 0.001) {
                double rx = (urand() - 0.5) * 2;
                double ry = (urand() - 0.5) * 2;
                double rz2 = (urand() - 0.5) * 2;
                double rLen = std::sqrt(rx * rx + ry * ry + rz2 * rz2);
                if (rLen < 1e-12) rLen = 1;
                IF(rb, corner + dx, corner + dy, corner + dz,
                   val * rx / rLen, val * ry / rLen, val * rz2 / rLen);
            }
        }
    }
    else if (name == "flux-vacuum-foam") {
        const int foamR = N / 3;
        const double foamBase = K_B * 0.9, foamVar = K_B * 0.4;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double dx = x - mid, dy = y - mid, dz = z - mid;
            double r2 = dx * dx + dy * dy + dz * dz;
            if (r2 > foamR * foamR) continue;
            double envelope = std::exp(-r2 / (2.0 * foamR * foamR * 0.5));
            double val = (foamBase + foamVar * urand()) * envelope;
            double rx = (urand() - 0.5) * 2;
            double ry = (urand() - 0.5) * 2;
            double rz2 = (urand() - 0.5) * 2;
            double rLen = std::sqrt(rx * rx + ry * ry + rz2 * rz2);
            if (rLen < 1e-12) rLen = 1;
            IF(rb, x, y, z, val * rx / rLen, val * ry / rLen, val * rz2 / rLen);
        }
    }
    // If we got here and matched none of the cases above, the prefix was
    // "flux-" but the specific name is unknown — silently no-op like JS.
    return true;
}

// ==========================================================================
//  Group: light-* (4 scenarios)
//  JS source: engine/web/js/bridge/scenarios/light-scenarios.js
// ==========================================================================
bool setup_light_scenario(RenderBridge& rb, const std::string& name) {
    if (name.rfind("light-", 0) != 0) return false;
    const int    N     = rb.lattice().size();
    const int    mid   = N / 2;
    const double C_WAVE = 1.0 / std::sqrt(3.0);
    const double amp   = 0.15;

    if (name == "light-rainbow") {
        struct W { int n; int pol; };
        const W waves[3] = { {1,1}, {3,2}, {6,0} };
        for (int w = 0; w < 3; w++) {
            double k = 2.0 * SCN_PI * waves[w].n / N;
            int pol = waves[w].pol;
            for (int x = 0; x < N; x++) for (int y = 0; y < N; y++) for (int z = 0; z < N; z++) {
                double J_val  = amp * std::sin(k * x);
                double wv_val = -2.0 * C_WAVE * std::sin(k / 2.0) * amp * std::cos(k * x);
                double fv[3] = {0,0,0}, wvv[3] = {0,0,0};
                fv[pol] = J_val;
                wvv[pol] = wv_val;
                IF(rb, x, y, z, fv[0], fv[1], fv[2]);
                IW(rb, x, y, z, wvv[0], wvv[1], wvv[2]);
            }
        }
    }
    else if (name == "light-dipole") {
        const int sigma = 3;
        const double dAmp = 0.5;
        for (int x = 0; x < N; x++) for (int y = 0; y < N; y++) for (int z = 0; z < N; z++) {
            double dx = x - mid, dy = y - mid, dz = z - mid;
            double g = dAmp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * sigma * sigma));
            if (g < 1e-6) continue;
            IF(rb, x, y, z, 0, 0, g);
            IW(rb, x, y, z, 0, 0, g);
        }
    }
    else if (name == "light-two-slit") {
        const int sigma = 2;
        const double sAmp = 0.3;
        const int slit_sep = N / 6;
        const int slit_x   = N / 4;
        const int slit_ys[2] = { mid - slit_sep, mid + slit_sep };
        for (int i = 0; i < 2; i++) {
            int sy = slit_ys[i];
            for (int z = 0; z < N; z++) for (int dy = -4; dy <= 4; dy++) for (int dx = -4; dx <= 4; dx++) {
                double g = sAmp * std::exp(-(dx*dx + dy*dy) / (2.0 * sigma * sigma));
                if (g < 1e-6) continue;
                int px = slit_x + dx, py = sy + dy;
                if (px < 0 || px >= N || py < 0 || py >= N) continue;
                IF(rb, px, py, z, 0, 0, g);
                IW(rb, px, py, z, g, 0, 0);
            }
        }
    }
    else if (name == "light-photon-race") {
        const int sigma = 3;
        const int x_start = N / 4;
        const double pAmps[2] = { 0.05, 0.5 };
        const int y_off[2] = { mid - N / 6, mid + N / 6 };
        for (int p = 0; p < 2; p++) {
            for (int x = 0; x < N; x++) {
                double dx = x - x_start;
                double g = pAmps[p] * std::exp(-dx * dx / (2.0 * sigma * sigma));
                if (g < 1e-8) continue;
                for (int y = y_off[p] - 2; y <= y_off[p] + 2; y++)
                for (int z = mid - 2; z <= mid + 2; z++) {
                    if (y < 0 || y >= N || z < 0 || z >= N) continue;
                    IF(rb, x, y, z, 0, 0, g);
                    IW(rb, x, y, z, 0, 0, g);
                }
            }
        }
    }
    return true;
}

// ==========================================================================
//  Group: quantum-* (8 scenarios)
//  JS source: engine/web/js/bridge/scenarios/quantum-scenarios.js
// ==========================================================================
bool setup_quantum_scenario(RenderBridge& rb, const std::string& name) {
    if (name.rfind("quantum-", 0) != 0) return false;
    const int N = rb.lattice().size();
    const int mid = N / 2;

    if (name == "quantum-born-rule") {
        const double sigma = N / 8.0;
        const double amp = K_B * 2.0;
        const double theta = urand() * 2.0 * SCN_PI;
        const int pulseR = std::min(CEL(sigma * 3), mid - 1);
        for (int dz = -pulseR; dz <= pulseR; dz++) for (int dy = -pulseR; dy <= pulseR; dy++) for (int dx = -pulseR; dx <= pulseR; dx++) {
            double val = amp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2 * sigma * sigma));
            if (val > 0.001) IF(rb, mid + dx, mid + dy, mid + dz, val * std::cos(theta), val * std::sin(theta), 0);
        }
        rb.toggles.genesis = true;
    }
    else if (name == "quantum-double-slit") {
        const int sigma = 2;
        const double sAmp = 0.3;
        const int slit_sep = N / 6;
        const int slit_x = N / 4;
        const int slit_ys[2] = { mid - slit_sep, mid + slit_sep };
        for (int i = 0; i < 2; i++) {
            int sy = slit_ys[i];
            for (int z = 0; z < N; z++) for (int dy = -4; dy <= 4; dy++) for (int dx = -4; dx <= 4; dx++) {
                double g = sAmp * std::exp(-(dx*dx + dy*dy) / (2.0 * sigma * sigma));
                if (g < 1e-6) continue;
                int px = slit_x + dx, py = sy + dy;
                if (px < 0 || px >= N || py < 0 || py >= N) continue;
                IF(rb, px, py, z, 0, 0, g);
                IW(rb, px, py, z, g, 0, 0);
            }
        }
        rb.toggles.genesis = true;
        rb.toggles.coupling = false;
    }
    else if (name == "quantum-tunnel") {
        const double sigma = N / 12.0;
        const double amp = K_B * 2.0;
        const int packetX = N / 4;
        const int pulseR = std::min(CEL(sigma * 3), mid - 1);
        for (int dz = -pulseR; dz <= pulseR; dz++) for (int dy = -pulseR; dy <= pulseR; dy++) for (int dx = -pulseR; dx <= pulseR; dx++) {
            double val = amp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2 * sigma * sigma));
            if (val > 0.001) {
                int x = packetX + dx, y = mid + dy, z = mid + dz;
                if (x >= 0 && x < N && y >= 0 && y < N && z >= 0 && z < N) {
                    IF(rb, x, y, z, val, 0, 0);
                    IW(rb, x, y, z, val, 0, 0);
                }
            }
        }
        const int W = 3;
        for (int y = 0; y < N; y++) for (int z = 0; z < N; z++) for (int dx = 0; dx < W; dx++) {
            IP(rb, mid + dx, y, z, 1);
            LOCK(rb, mid + dx, y, z);
        }
    }
    else if (name == "quantum-well") {
        const int wallA = N / 4;
        const int wallB = 3 * N / 4;
        const int boxLength = wallB - wallA;
        for (int y = 0; y < N; y++) for (int z = 0; z < N; z++) {
            IP(rb, wallA, y, z, 1); LOCK(rb, wallA, y, z);
            IP(rb, wallB, y, z, 1); LOCK(rb, wallB, y, z);
        }
        for (int n = 1; n <= 8; n++) {
            double amp_n = K_B * 0.5 / n;
            for (int x = wallA + 1; x < wallB; x++) for (int y = 0; y < N; y++) for (int z = 0; z < N; z++) {
                double val = amp_n * std::sin(n * SCN_PI * (x - wallA) / double(boxLength));
                if (std::fabs(val) > 1e-6) IF(rb, x, y, z, 0, val, 0);
            }
        }
        rb.toggles.genesis = false;
        rb.toggles.damping = false;
    }
    else if (name == "quantum-entangle") {
        const double bigAmp = K_GENESIS * 5.0;
        for (int dz = -4; dz <= 4; dz++) for (int dy = -4; dy <= 4; dy++) for (int dx = -4; dx <= 4; dx++) {
            double val = bigAmp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * 6.0));
            if (val > 0.001) IF(rb, mid + dx, mid + dy, mid + dz, val, val, val);
        }
        rb.toggles.genesis = true;
    }
    else if (name == "quantum-aharonov-bohm") {
        const int R = N / 8;
        for (int z = 0; z < N; z++) for (int dy = -R; dy <= R; dy++) for (int dx = -R; dx <= R; dx++) {
            if (dx * dx + dy * dy > R * R) continue;
            IF(rb, mid + dx, mid + dy, z, 0, 0, K_B * 0.5);
        }
        const int pSigma = 3;
        const double pAmp = K_B * 2.0;
        const int pStartX = N / 4;
        for (int dz = -pSigma; dz <= pSigma; dz++) for (int dy = -pSigma; dy <= pSigma; dy++) for (int dx = -pSigma; dx <= pSigma; dx++) {
            double val = pAmp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * pSigma * pSigma));
            if (val > 0.001) {
                int px = pStartX + dx;
                int ayPos = mid + R + 2 + dy;
                int byPos = mid - R - 2 + dy;
                int pz = mid + dz;
                if (px >= 0 && px < N && pz >= 0 && pz < N) {
                    if (ayPos >= 0 && ayPos < N) { IF(rb, px, ayPos, pz, val, 0, 0); IW(rb, px, ayPos, pz, val, 0, 0); }
                    if (byPos >= 0 && byPos < N) { IF(rb, px, byPos, pz, val, 0, 0); IW(rb, px, byPos, pz, val, 0, 0); }
                }
            }
        }
    }
    else if (name == "quantum-casimir") {
        const int d = 6;
        const int plateA = mid - d / 2, plateB = mid + d / 2;
        for (int y = 0; y < N; y++) for (int z = 0; z < N; z++) {
            IP(rb, plateA, y, z, 1); LOCK(rb, plateA, y, z);
            IP(rb, plateB, y, z, 1); LOCK(rb, plateB, y, z);
        }
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            IF(rb, x, y, z,
               (urand() - 0.5) * K_B * 0.3,
               (urand() - 0.5) * K_B * 0.3,
               (urand() - 0.5) * K_B * 0.3);
        }
        rb.toggles.genesis = false;
    }
    else if (name == "quantum-zeno") {
        const double sigma = N / 10.0;
        const double amp = K_GENESIS * 1.2;
        const int pulseR = std::min(CEL(sigma * 3), mid - 1);
        for (int dz = -pulseR; dz <= pulseR; dz++) for (int dy = -pulseR; dy <= pulseR; dy++) for (int dx = -pulseR; dx <= pulseR; dx++) {
            double val = amp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2 * sigma * sigma));
            if (val > 0.001) IF(rb, mid + dx, mid + dy, mid + dz, val, val, val);
        }
        rb.toggles.genesis = true;
    }
    return true;
}

// ==========================================================================
//  Group: s0-field-* (8 scenarios)
//  JS source: engine/web/js/bridge/scenarios/s0-field-scenarios.js
// ==========================================================================
bool setup_s0_field_scenario(RenderBridge& rb, const std::string& name) {
    if (name.rfind("s0-field-", 0) != 0) return false;
    const int    N    = rb.lattice().size();
    const double midF = (N - 1) * 0.5;
    const int    mc   = RND(midF);
    const double cSpeed = 1.0 / std::sqrt(3.0);

    if (name == "s0-field-plane-wave") {
        const double wl  = N / 4.0;
        const double amp = K_B * 2.0;
        const double k   = 2.0 * SCN_PI / wl;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double phase = k * x;
            double jz = amp * std::sin(phase);
            double wz = amp * std::cos(phase) * cSpeed;
            if (std::fabs(jz) > 1e-12 || std::fabs(wz) > 1e-12) {
                IF(rb, x, y, z, 0, 0, jz);
                IW(rb, x, y, z, wz, 0, 0);
            }
        }
    }
    else if (name == "s0-field-standing-wave") {
        const double wl  = N / 4.0;
        const double amp = K_B * 2.0;
        const double k   = 2.0 * SCN_PI / wl;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double jz = amp * std::sin(k * x);
            if (std::fabs(jz) > 1e-12) IF(rb, x, y, z, 0, 0, jz);
        }
    }
    else if (name == "s0-field-uniform-e") {
        const double eMag = 0.1;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            IW(rb, x, y, z, -eMag, 0, 0);
        }
    }
    else if (name == "s0-field-uniform-b") {
        const double bMag = 0.05;
        const double half = (N - 1) / 2.0;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double rx = x - half, ry = y - half;
            double jx = -bMag * ry / 2, jy = bMag * rx / 2;
            if (std::fabs(jx) > 1e-12 || std::fabs(jy) > 1e-12) IF(rb, x, y, z, jx, jy, 0);
        }
    }
    else if (name == "s0-field-photon-pulse") {
        const int sigma = std::max(3, N / 8);
        const double amp = K_B * 2.0;
        const double lambdaEff = 4.0 * sigma;
        const double k = 2.0 * SCN_PI / lambdaEff;
        const double cutR = 3.0 * sigma;
        const double cutR2 = cutR * cutR;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double dx = x - mc, dy = y - mc, dz = z - mc;
            double r2 = dx * dx + dy * dy + dz * dz;
            if (r2 > cutR2) continue;
            double g = std::exp(-r2 / (2.0 * sigma * sigma));
            if (g < 1e-6) continue;
            double phase = k * dx;
            double jz = amp * g * std::sin(phase);
            double wz = amp * g * std::cos(phase) * cSpeed;
            IF(rb, x, y, z, 0, 0, jz);
            IW(rb, x, y, z, wz, 0, 0);
        }
    }
    else if (name == "s0-field-electric-dipole") {
        const int sep  = std::max(2, N / 8);
        const int half = sep / 2;
        const int px = mc + half, nx = mc - half;
        IP(rb, px, mc, mc, +1);
        IP(rb, nx, mc, mc, -1);
        const double alpha_amp = ALPHA / (4.0 * SCN_PI);
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double jx = 0, jy = 0, jz = 0;
            double dx1 = x - px, dy1 = y - mc, dz1 = z - mc;
            double r2_1 = dx1*dx1 + dy1*dy1 + dz1*dz1 + 1.0;
            double f1 = alpha_amp / r2_1;
            jx += f1 * dx1; jy += f1 * dy1; jz += f1 * dz1;
            double dx2 = x - nx, dy2 = y - mc, dz2 = z - mc;
            double r2_2 = dx2*dx2 + dy2*dy2 + dz2*dz2 + 1.0;
            double f2 = -alpha_amp / r2_2;
            jx += f2 * dx2; jy += f2 * dy2; jz += f2 * dz2;
            double mag = std::sqrt(jx*jx + jy*jy + jz*jz);
            if (mag > 1e-6) IF(rb, x, y, z, jx, jy, jz);
        }
    }
    else if (name == "s0-field-magnetic-dipole") {
        const int loopR = std::max(3, N / 8);
        const double amp = K_B;
        const int nAngles = std::max(36, loopR * 8);
        for (int i = 0; i < nAngles; i++) {
            double theta = 2.0 * SCN_PI * i / nAngles;
            int lx = RND(mc + loopR * std::cos(theta));
            int ly = RND(mc + loopR * std::sin(theta));
            double tx = -std::sin(theta) * amp;
            double ty =  std::cos(theta) * amp;
            for (int z = 0; z < N; z++) IF(rb, lx, ly, z, tx, ty, 0);
        }
    }
    else if (name == "s0-field-vortex-line") {
        const double gamma = K_B * 4.0;
        const double half = (N - 1) / 2.0;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double rx = x - half, ry = y - half;
            double r = std::sqrt(rx * rx + ry * ry);
            if (r < 1.0) r = 1.0;
            double mag = gamma / (2.0 * SCN_PI * r);
            if (mag < 1e-6) continue;
            IF(rb, x, y, z, -mag * ry / r, mag * rx / r, 0);
        }
    }
    return true;
}

// ==========================================================================
//  Group: s0-seed-* (49 scenarios)
//  JS source: engine/web/js/bridge/scenarios/s0-seed-scenarios.js
// ==========================================================================

// Helper for s0-seed-electron/muon/tau — identical topology, amp differs.
static void seed_lepton(RenderBridge& rb, int mc, double midF, int N, double boost) {
    IP(rb, mc, mc, mc, -1);
    const int envR = std::max(3, N / 6);
    const double envSigma = envR / 2.0;
    const double envAmp = K_B * boost;
    const double envR2 = envR * envR;
    const int eLo = FLR(midF) - envR, eHi = CEL(midF) + envR;
    for (int z = eLo; z <= eHi; z++) for (int y = eLo; y <= eHi; y++) for (int x = eLo; x <= eHi; x++) {
        double dx = x - midF, dy = y - midF, dz = z - midF;
        double r2 = dx*dx + dy*dy + dz*dz;
        if (r2 < 0.25 || r2 > envR2) continue;
        double r = std::sqrt(r2);
        double val = envAmp * std::exp(-r2 / (2.0 * envSigma * envSigma));
        if (val < 0.001) continue;
        IF(rb, x, y, z, -val*dx/r, -val*dy/r, -val*dz/r);
    }
}

// Helper: _dp from JS — particle + radial flux envelope, optional lock.
static void dp(RenderBridge& rb, int cx, int cy, int cz,
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

// Helper: _tri from JS — 3-vertex equilateral triangle in xy-plane.
static void tri(RenderBridge& rb, int cx, int cy, int cz,
                const int charges[3], const int colors[3], int rad, bool lock) {
    for (int k = 0; k < 3; k++) {
        double ang = (2.0 * SCN_PI * k) / 3.0;
        int qx = RND(cx + rad * std::cos(ang));
        int qy = RND(cy + rad * std::sin(ang));
        dp(rb, qx, qy, cz, charges[k], (k % 2 == 0) ? 1 : -1, colors[k], 2, K_B * 0.5, lock);
    }
}

bool setup_s0_seed_scenario(RenderBridge& rb, const std::string& name) {
    if (name.rfind("s0-seed-", 0) != 0) return false;
    const int    N    = rb.lattice().size();
    const double midF = (N - 1) * 0.5;
    const int    mc   = RND(midF);

    if (name == "s0-seed-electron") {
        seed_lepton(rb, mc, midF, N, 1.5);
    }
    else if (name == "s0-seed-muon" || name == "s0-seed-tau") {
        double boost = (name == "s0-seed-tau") ? 2.25 : 1.80;
        seed_lepton(rb, mc, midF, N, boost);
    }
    else if (name == "s0-seed-photon") {
        const int sigma = 3;
        const double pAmp = K_B * 2.0;
        const int pStartX = std::max(4, N / 4);
        const int halfR = 8;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int dx = -halfR; dx <= halfR; dx++) {
            int x = pStartX + dx;
            if (x < 0 || x >= N) continue;
            double dy = y - midF, dz = z - midF;
            double g = pAmp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * sigma * sigma));
            if (g < 1e-6) continue;
            IF(rb, x, y, z, 0, 0, g);
            IW(rb, x, y, z, g, 0, 0);
        }
    }
    else if (name == "s0-seed-proton-candidate") {
        const int bR = std::max(2, N / 8);
        for (int k = 0; k < 3; k++) {
            double angle = (2.0 * SCN_PI * k) / 3.0;
            int bx = RND(midF + bR * std::cos(angle));
            int bz = RND(midF + bR * std::sin(angle));
            IP(rb, bx, mc, bz, 1);
        }
        const int envR = std::max(3, N / 5);
        const double envSigma = envR / 2.0;
        const double envAmp = K_B * 1.0;
        const double envR2 = envR * envR;
        const int eLo = std::max(0, FLR(midF) - envR);
        const int eHi = std::min(N - 1, CEL(midF) + envR);
        for (int z = eLo; z <= eHi; z++) for (int y = eLo; y <= eHi; y++) for (int x = eLo; x <= eHi; x++) {
            double dx = x - midF, dy = y - midF, dz = z - midF;
            double r2 = dx*dx + dy*dy + dz*dz;
            if (r2 < 0.25 || r2 > envR2) continue;
            double r = std::sqrt(r2);
            double val = envAmp * std::exp(-r2 / (2.0 * envSigma * envSigma));
            if (val < 0.001) continue;
            IF(rb, x, y, z, val*dx/r, val*dy/r, val*dz/r);
        }
    }
    // ── Moore Seeds ──
    else if (name == "s0-seed-octahedron") {
        IP(rb, mc, mc, mc, -1);
        const int off[6][3] = {{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
        for (int i = 0; i < 6; i++) IP(rb, mc+off[i][0], mc+off[i][1], mc+off[i][2], +1);
    }
    else if (name == "s0-seed-cuboctahedron") {
        IP(rb, mc, mc, mc, -1);
        const int off[12][3] = {
            {1,1,0},{1,-1,0},{-1,1,0},{-1,-1,0},
            {1,0,1},{1,0,-1},{-1,0,1},{-1,0,-1},
            {0,1,1},{0,1,-1},{0,-1,1},{0,-1,-1}
        };
        for (int i = 0; i < 12; i++) IP(rb, mc+off[i][0], mc+off[i][1], mc+off[i][2], +1);
    }
    else if (name == "s0-seed-stella-octangula") {
        IP(rb, mc, mc, mc, -1);
        const int off[8][3] = {
            {1,1,1},{1,1,-1},{1,-1,1},{1,-1,-1},
            {-1,1,1},{-1,1,-1},{-1,-1,1},{-1,-1,-1}
        };
        for (int i = 0; i < 8; i++) IP(rb, mc+off[i][0], mc+off[i][1], mc+off[i][2], +1);
    }
    else if (name == "s0-seed-moore-cell") {
        IP(rb, mc, mc, mc, -1);
        for (int dx = -1; dx <= 1; dx++) for (int dy = -1; dy <= 1; dy++) for (int dz = -1; dz <= 1; dz++) {
            if (dx == 0 && dy == 0 && dz == 0) continue;
            IP(rb, mc+dx, mc+dy, mc+dz, +1);
        }
    }
    else if (name == "s0-seed-moore-decomposition") {
        IP(rb, mc, mc, mc, -1);
        const int oct[6][3] = {{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
        for (int i = 0; i < 6; i++) IP(rb, mc+oct[i][0], mc+oct[i][1], mc+oct[i][2], +1);
        const int cub[12][3] = {
            {1,1,0},{1,-1,0},{-1,1,0},{-1,-1,0},
            {1,0,1},{1,0,-1},{-1,0,1},{-1,0,-1},
            {0,1,1},{0,1,-1},{0,-1,1},{0,-1,-1}
        };
        for (int i = 0; i < 12; i++) IP(rb, mc+cub[i][0], mc+cub[i][1], mc+cub[i][2], -1);
        const int stel[8][3] = {
            {1,1,1},{1,1,-1},{1,-1,1},{1,-1,-1},
            {-1,1,1},{-1,1,-1},{-1,-1,1},{-1,-1,-1}
        };
        for (int i = 0; i < 8; i++) IP(rb, mc+stel[i][0], mc+stel[i][1], mc+stel[i][2], +1);
    }
    // ── Level 3-5: composite seeds via dp/tri helpers ──
    else if (name == "s0-seed-electron-l3") {
        dp(rb, mc, mc, mc, -1, -1, 0, std::max(3, N / 10), K_B * 1.5, false);
    }
    else if (name == "s0-seed-positron") {
        dp(rb, mc, mc, mc, +1, +1, 0, std::max(3, N / 10), K_B * 1.5, false);
    }
    else if (name == "s0-seed-neutrino") {
        const int sig = 2, eR = 6;
        for (int dz2 = -eR; dz2 <= eR; dz2++) for (int dy2 = -eR; dy2 <= eR; dy2++) for (int dx2 = -eR; dx2 <= eR; dx2++) {
            double r22 = dx2*dx2 + dy2*dy2 + dz2*dz2;
            if (r22 > eR * eR) continue;
            double gg = K_B * 0.3 * std::exp(-r22 / (2.0 * sig * sig));
            if (gg < 0.001) continue;
            IF(rb, mc+dx2, mc+dy2, mc+dz2, gg * 0.55, gg * 0.45, 0);
        }
    }
    else if (name == "s0-seed-quark") {
        dp(rb, mc, mc, mc, +1, +1, 1, 2, K_B * 0.5, false);
    }
    else if (name == "s0-seed-antiquark") {
        dp(rb, mc, mc, mc, -1, -1, 1, 2, K_B * 0.5, false);
    }
    else if (name == "s0-seed-pion") {
        int sp = std::max(3, N / 8), hf = sp / 2;
        dp(rb, mc + hf, mc, mc, +1, +1, 1, 2, K_B * 0.5, true);
        dp(rb, mc - hf, mc, mc, -1, -1, 1, 2, K_B * 0.5, true);
    }
    else if (name == "s0-seed-proton-l4") {
        const int bR = std::max(2, N / 8);
        const int charges[3] = {+1, +1, -1};
        const int colors[3]  = {1, 2, 3};
        tri(rb, mc, mc, mc, charges, colors, bR, true);
    }
    else if (name == "s0-seed-neutron") {
        const int bR = std::max(2, N / 8);
        const int charges[3] = {+1, -1, -1};
        const int colors[3]  = {1, 2, 3};
        tri(rb, mc, mc, mc, charges, colors, bR, true);
    }
    else if (name == "s0-seed-hydrogen") {
        const int oR = std::max(4, N / 6);
        const int bR = std::max(2, N / 12);
        const int charges[3] = {+1, +1, -1};
        const int colors[3]  = {1, 2, 3};
        tri(rb, mc, mc, mc, charges, colors, bR, true);
        dp(rb, mc, mc, mc + oR, -1, -1, 0, 2, K_B, false);
    }
    else if (name == "s0-seed-helium") {
        const int oR = std::max(3, N / 8);
        dp(rb, mc, mc, mc,       +1, 0, 0, 2, K_B * 3.0, true);
        dp(rb, mc, mc, mc + oR,  -1, +1, 0, 2, K_B * 0.8, false);
        dp(rb, mc, mc, mc - oR,  -1, -1, 0, 2, K_B * 0.8, false);
    }
    else if (name == "s0-seed-h2-molecule") {
        const int bd = std::max(4, N / 6), hf = bd / 2;
        const int oR = std::max(3, N / 8), bR = std::max(1, N / 16);
        const int charges[3] = {+1, +1, -1};
        const int colors[3]  = {1, 2, 3};
        tri(rb, mc - hf, mc, mc, charges, colors, bR, true);
        dp(rb, mc - hf, mc, mc + oR, -1, -1, 0, 2, K_B * 0.8, false);
        tri(rb, mc + hf, mc, mc, charges, colors, bR, true);
        dp(rb, mc + hf, mc, mc + oR, -1, +1, 0, 2, K_B * 0.8, false);
    }
    // ── Quarks (LHC additions) ──
    else if (name == "s0-seed-up-quark" || name == "s0-seed-down-quark" ||
             name == "s0-seed-strange-quark" || name == "s0-seed-charm-quark" ||
             name == "s0-seed-bottom-quark" || name == "s0-seed-top-quark") {
        int charge, color;
        double ampBoost;
        if      (name == "s0-seed-up-quark")      { charge = +1; color = 1; ampBoost = 0.5; }
        else if (name == "s0-seed-down-quark")    { charge = -1; color = 2; ampBoost = 0.5; }
        else if (name == "s0-seed-strange-quark") { charge = -1; color = 3; ampBoost = 0.7; }
        else if (name == "s0-seed-charm-quark")   { charge = +1; color = 1; ampBoost = 1.0; }
        else if (name == "s0-seed-bottom-quark")  { charge = -1; color = 2; ampBoost = 1.4; }
        else                                        { charge = +1; color = 3; ampBoost = 2.5; }
        IPF(rb, mc, mc, mc, charge, (charge > 0) ? +1 : -1, color);
        const double qSig = 1.5;
        const int qR = 4;
        const double qAmp = K_B * ampBoost;
        for (int dz = -qR; dz <= qR; dz++) for (int dy = -qR; dy <= qR; dy++) for (int dx = -qR; dx <= qR; dx++) {
            int r2 = dx*dx + dy*dy + dz*dz;
            if (r2 == 0 || r2 > qR * qR) continue;
            double r = std::sqrt(double(r2));
            double g = qAmp * std::exp(-r2 / (2.0 * qSig * qSig));
            if (g < 1e-3) continue;
            int sign = (charge > 0) ? 1 : -1;
            double axisBias[3] = {0, 0, 0};
            axisBias[color - 1] = 0.5;
            IF(rb, mc + dx, mc + dy, mc + dz,
               sign * g * (dx / r + axisBias[0]),
               sign * g * (dy / r + axisBias[1]),
               sign * g * (dz / r + axisBias[2]));
        }
    }
    // ── Electroweak bosons + Higgs + gluon ──
    else if (name == "s0-seed-higgs-boson") {
        const double hSig = 2.0, hAmp = K_B * 1.2;
        const int hR = 6;
        for (int dz = -hR; dz <= hR; dz++) for (int dy = -hR; dy <= hR; dy++) for (int dx = -hR; dx <= hR; dx++) {
            int r2 = dx*dx + dy*dy + dz*dz;
            if (r2 == 0 || r2 > hR * hR) continue;
            double g = hAmp * std::exp(-r2 / (2.0 * hSig * hSig));
            if (g < 1e-3) continue;
            double iso = g / std::sqrt(3.0);
            IF(rb, mc + dx, mc + dy, mc + dz, iso, iso, iso);
        }
    }
    else if (name == "s0-seed-higgs-field") {
        const double vevAmp = K_B * 0.3;
        const double noise  = K_B * 0.05;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double sx = std::sin(0.19*x + 0.23*y + 0.29*z);
            double sy = std::sin(0.37*x + 0.13*y + 0.17*z);
            double sz = std::sin(0.11*x + 0.31*y + 0.41*z);
            IF(rb, x, y, z, vevAmp + noise*sx, vevAmp + noise*sy, vevAmp + noise*sz);
        }
    }
    else if (name == "s0-seed-w-boson") {
        IP(rb, mc, mc, mc, +1);
        SET_SPIN(rb, mc, mc, mc, +1);
        const double wSig = 1.8, wAmp = K_B * 1.6;
        const int wR = 5;
        for (int dz = -wR; dz <= wR; dz++) for (int dy = -wR; dy <= wR; dy++) for (int dx = -wR; dx <= wR; dx++) {
            int r2 = dx*dx + dy*dy + dz*dz;
            if (r2 == 0 || r2 > wR * wR) continue;
            double r = std::sqrt(double(r2));
            double g = wAmp * std::exp(-r2 / (2.0 * wSig * wSig));
            if (g < 1e-3) continue;
            IF(rb, mc + dx, mc + dy, mc + dz, g * (1.3 * dx / r), g * (dy / r), g * (dz / r));
        }
    }
    else if (name == "s0-seed-z-boson") {
        const double zSig = 2.0, zAmp = K_B * 1.8;
        const int zR = 6;
        for (int dz = -zR; dz <= zR; dz++) for (int dy = -zR; dy <= zR; dy++) for (int dx = -zR; dx <= zR; dx++) {
            int r2 = dx*dx + dy*dy + dz*dz;
            if (r2 == 0 || r2 > zR * zR) continue;
            double r = std::sqrt(double(r2));
            double g = zAmp * std::exp(-r2 / (2.0 * zSig * zSig));
            if (g < 1e-3) continue;
            IF(rb, mc + dx, mc + dy, mc + dz, -g * dx / r, -g * dy / r, -g * dz / r);
        }
    }
    else if (name == "s0-seed-gluon") {
        const int sigma = 3;
        const double gAmp = K_B * 2.0;
        const int startX = std::max(4, N / 4);
        const int halfR = 8;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int dx = -halfR; dx <= halfR; dx++) {
            int x = startX + dx;
            if (x < 0 || x >= N) continue;
            double dy = y - midF, dz = z - midF;
            double gg = gAmp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * sigma * sigma));
            if (gg < 1e-6) continue;
            IF(rb, x, y, z, 0, gg, 0);
            IW(rb, x, y, z, gg, 0, 0);
        }
    }
    // ── Process demos ──
    else if (name == "s0-seed-beta-decay") {
        const int bdR = std::max(2, N / 10);
        for (int k = 0; k < 3; k++) {
            double ang = (2.0 * SCN_PI * k) / 3.0;
            int bx = RND(mc + bdR * std::cos(ang));
            int by = RND(mc + bdR * std::sin(ang));
            int charge = (k == 0) ? +1 : -1;
            IP(rb, bx, by, mc, charge);
        }
        const int leptonR = std::max(4, N / 5);
        IP(rb, mc, mc, mc + leptonR, -1);
        const int nuSig = 2, nuR = 4;
        for (int dz2 = -nuR; dz2 <= nuR; dz2++) for (int dy2 = -nuR; dy2 <= nuR; dy2++) for (int dx2 = -nuR; dx2 <= nuR; dx2++) {
            int r22 = dx2*dx2 + dy2*dy2 + dz2*dz2;
            if (r22 > nuR * nuR) continue;
            double g = K_B * 0.3 * std::exp(-r22 / (2.0 * nuSig * nuSig));
            if (g < 1e-3) continue;
            IF(rb, mc+dx2, mc-leptonR+dy2, mc+dz2, g*0.55, g*0.45, 0);
        }
        rb.toggles.weak_transmutation = true;
        rb.toggles.dual_substrate = true;
    }
    else if (name == "s0-seed-ee-annihilation") {
        const int aSep = std::max(6, N / 3);
        const int half = aSep / 2;
        IP(rb, mc - half, mc, mc, -1);
        SET_VEL(rb, mc - half, mc, mc, +0.3 * C_SPEED, 0, 0);
        IP(rb, mc + half, mc, mc, +1);
        SET_VEL(rb, mc + half, mc, mc, -0.3 * C_SPEED, 0, 0);
        const int aSig = 2, aR = 4;
        for (int pass = 0; pass < 2; pass++) {
            int cx = (pass == 0) ? mc - half : mc + half;
            int sign = (pass == 0) ? -1 : +1;
            for (int dz2 = -aR; dz2 <= aR; dz2++) for (int dy2 = -aR; dy2 <= aR; dy2++) for (int dx2 = -aR; dx2 <= aR; dx2++) {
                int r2 = dx2*dx2 + dy2*dy2 + dz2*dz2;
                if (r2 == 0 || r2 > aR * aR) continue;
                double r = std::sqrt(double(r2));
                double g = K_B * std::exp(-r2 / (2.0 * aSig * aSig));
                if (g < 1e-3) continue;
                IF(rb, cx+dx2, mc+dy2, mc+dz2, sign*g*dx2/r, sign*g*dy2/r, sign*g*dz2/r);
            }
        }
    }
    // ── Level 6: Gauge / Topological ──
    else if (name == "s0-seed-wilson-loop") {
        const int R = std::max(3, N / 8);
        const double wAmp = K_B;
        for (int x = mc - R; x <= mc + R; x++) IF(rb, x, mc - R, mc,  wAmp, 0, 0);
        for (int y = mc - R; y <= mc + R; y++) IF(rb, mc + R, y, mc, 0,  wAmp, 0);
        for (int x = mc + R; x >= mc - R; x--) IF(rb, x, mc + R, mc, -wAmp, 0, 0);
        for (int y = mc + R; y >= mc - R; y--) IF(rb, mc - R, y, mc, 0, -wAmp, 0);
        IP(rb, mc-R, mc-R, mc, +1); IP(rb, mc+R, mc-R, mc, +1);
        IP(rb, mc+R, mc+R, mc, +1); IP(rb, mc-R, mc+R, mc, +1);
    }
    else if (name == "s0-seed-flux-tube") {
        const int ftSep = std::max(6, N / 4), ftH = ftSep / 2;
        IP(rb, mc - ftH, mc, mc, +1);
        IP(rb, mc + ftH, mc, mc, -1);
        const double ftSig = 1.5;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = mc - ftH; x <= mc + ftH; x++) {
            double dy2 = y - mc, dz2 = z - mc;
            double p2 = dy2*dy2 + dz2*dz2;
            double g = K_B * std::exp(-p2 / (2.0 * ftSig * ftSig));
            if (g > 0.001) IF(rb, x, y, z, g, 0, 0);
        }
    }
    else if (name == "s0-seed-monopole") {
        const double mHalf = (N - 1) / 2.0;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double rx = x - mHalf, ry = y - mHalf, rz = z - mHalf;
            double r = std::sqrt(rx*rx + ry*ry + rz*rz);
            if (r < 1.0) r = 1.0;
            double mg = 1.0 / (4.0 * SCN_PI * r * r);
            if (mg < 1e-6) continue;
            double rxy = std::sqrt(rx*rx + ry*ry);
            if (rxy < 0.5) { IF(rb, x, y, z, 0, 0, mg); continue; }
            IF(rb, x, y, z, -ry / rxy * mg, rx / rxy * mg, 0);
        }
    }
    else if (name == "s0-seed-instanton") {
        const double iSize = 3.0, iHalf = (N - 1) / 2.0;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double rx = x - iHalf, ry = y - iHalf, rz = z - iHalf;
            double r2 = rx*rx + ry*ry + rz*rz;
            double r = std::sqrt(r2);
            double mg = iSize / (r2 + iSize * iSize);
            if (mg < 1e-6 || r < 0.5) continue;
            IF(rb, x, y, z, mg * rx / r, mg * ry / r, mg * rz / r);
        }
    }
    // ── Level 7: Gravity / Cosmology ──
    else if (name == "s0-seed-schwarzschild") {
        const double sHalf = (N - 1) / 2.0, rs = 3.0;
        IP(rb, mc, mc, mc, +1);
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double rx = x - sHalf, ry = y - sHalf, rz = z - sHalf;
            double r = std::sqrt(rx*rx + ry*ry + rz*rz);
            if (r < 0.5) r = 0.5;
            double mg = K_B * rs / (r * r);
            if (mg < 1e-6) continue;
            IF(rb, x, y, z, -mg * rx / r, -mg * ry / r, -mg * rz / r);
        }
    }
    else if (name == "s0-seed-frw-patch") {
        int frwStride = RND(1.0 / std::cbrt(0.01));
        int frwSign = 1;
        for (int z = 0; z < N; z += frwStride) for (int y = 0; y < N; y += frwStride) for (int x = 0; x < N; x += frwStride) {
            IP(rb, x, y, z, frwSign);
            frwSign = -frwSign;
        }
    }
    else if (name == "s0-seed-gravitational-wave") {
        const int gwWl = std::max(4, N / 4);
        const double gwK = 2.0 * SCN_PI / gwWl, gwAmp = 0.1;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double v = gwAmp * std::sin(gwK * x);
            if (std::fabs(v) > 1e-6) IF(rb, x, y, z, 0, v, 0);
        }
    }
    // ── Level 8: Consciousness / Observer ──
    else if (name == "s0-seed-sloop") {
        const int slR = std::max(3, N / 8);
        const int slN = 12;
        const double slA = K_B;
        for (int i = 0; i < slN; i++) {
            double a = 2.0 * SCN_PI * i / slN;
            int px = RND(mc + slR * std::cos(a));
            int py = RND(mc + slR * std::sin(a));
            IP(rb, px, py, mc, +1);
            IF(rb, px, py, mc, -std::sin(a) * slA, std::cos(a) * slA, 0);
        }
    }
    else if (name == "s0-seed-observer-cell") {
        IP(rb, mc, mc, mc, +1);
        const int oct[6][3] = {{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
        for (int i = 0; i < 6; i++) IP(rb, mc+oct[i][0], mc+oct[i][1], mc+oct[i][2], -1);
        const int cub[12][3] = {
            {1,1,0},{1,-1,0},{-1,1,0},{-1,-1,0},
            {1,0,1},{1,0,-1},{-1,0,1},{-1,0,-1},
            {0,1,1},{0,1,-1},{0,-1,1},{0,-1,-1}
        };
        for (int i = 0; i < 12; i++) IP(rb, mc+cub[i][0], mc+cub[i][1], mc+cub[i][2], +1);
        const int stel[8][3] = {
            {1,1,1},{1,1,-1},{1,-1,1},{1,-1,-1},
            {-1,1,1},{-1,1,-1},{-1,-1,1},{-1,-1,-1}
        };
        for (int i = 0; i < 8; i++) IP(rb, mc+stel[i][0], mc+stel[i][1], mc+stel[i][2], -1);
    }
    return true;
}

// ==========================================================================
//  Dispatcher — matches JS runSetupScenario contract.
// ==========================================================================
bool dispatch_scenario(RenderBridge& rb, const std::string& name) {
    // Reset the stochastic RNG so each setupScenario call produces a
    // reproducible sequence. Without this, the thread_local distribution
    // state from a previous scenario (e.g. flux-random-genesis) would leak
    // into the next stochastic scenario called in the same process.
    reset_rng();

    // Try each group in order; first matching prefix wins.
    if (setup_flux_scenario(rb, name))     return true;
    if (setup_light_scenario(rb, name))    return true;
    if (setup_quantum_scenario(rb, name))  return true;
    if (setup_s0_seed_scenario(rb, name))  return true;
    if (setup_s0_field_scenario(rb, name)) return true;
    return false;
}

}  // namespace ftd
