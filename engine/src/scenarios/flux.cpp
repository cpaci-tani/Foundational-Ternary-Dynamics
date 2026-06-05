// ==========================================================================
//  engine/src/scenarios/flux.cpp
//
//  Group: flux-* (21 scenarios)
//  JS source: engine/web/js/bridge/scenarios/flux-scenarios.js
//
//  Split out of engine/src/scenarios.cpp (ticket S1). Every scenario body
//  is byte-identical to the pre-split source — see _helpers.h for the
//  shared IF/IW/IP/IPF/SET_VEL/LOCK/SET_SPIN/FLR/CEL/RND primitives
//  and docs/scenarios.h for the group-function contract.
// ==========================================================================

#include "ftd/scenarios.h"
#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/voxel.h"

#include "_helpers.h"

#include <cmath>

namespace ftd {

// Bring detail::urand() into scope for the stochastic scenarios below.
using detail::urand;

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
        // genesis=false (audit-2 2026-04-28): solitons are non-dispersive,
        // not pair-producers. Mirrors JS flux-soliton fix.
        rb.toggles.genesis = false;
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
            double angle = (2.0 * PI * i) / nV;
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
            double angle = (2.0 * PI * k) / 3.0;
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
        const double triAng[3] = { 0, 2 * PI / 3, 4 * PI / 3 };
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
    else if (name == "flux-zero-point") {
        // Zero-Point Energy — the irreducible ground-state floor. Uniform
        // low-amplitude random flux across the WHOLE lattice at 0.3·K_B
        // (≈ 0.08, ~20× below K_GENESIS = N_c·K_B = 1.533), so nothing can
        // manifest. genesis + damping are OFF via config/toggles.js, so the
        // energy-conserving wave dynamics keep a persistent non-zero floor.
        // Mirrors the JS flux-zero-point body (same amplitude); JS↔C++ parity
        // is statistical (both stochastic). Pedagogical, not a ½ℏω derivation.
        const double zpeAmp = K_B * 0.3;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            IF(rb, x, y, z,
               (urand() - 0.5) * zpeAmp,
               (urand() - 0.5) * zpeAmp,
               (urand() - 0.5) * zpeAmp);
        }
    }
    // If we got here and matched none of the cases above, the prefix was
    // "flux-" but the specific name is unknown — silently no-op like JS.
    return true;
}

}  // namespace ftd
