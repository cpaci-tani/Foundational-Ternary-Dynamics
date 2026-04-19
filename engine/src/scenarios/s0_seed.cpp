// ==========================================================================
//  engine/src/scenarios/s0_seed.cpp
//
//  Group: s0-seed-* (49 scenarios)
//  JS source: engine/web/js/bridge/scenarios/s0-seed-scenarios.js
//
//  Split out of engine/src/scenarios.cpp (ticket S1). The three internal
//  static helpers seed_lepton / dp / tri moved with this group because
//  they are only used by s0-seed-* scenarios.
// ==========================================================================

#include "ftd/scenarios.h"
#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/voxel.h"

#include "_helpers.h"

#include <cmath>

namespace ftd {

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

}  // namespace ftd
