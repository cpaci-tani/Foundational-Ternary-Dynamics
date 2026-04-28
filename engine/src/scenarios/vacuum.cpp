// ==========================================================================
//  engine/src/scenarios/vacuum.cpp
//
//  Group: s0-vacuum-* (15 scenarios)
//  JS source: engine/web/js/bridge/scenarios/vacuum-scenarios.js
//  Spec:      engine/web/docs/SPEC_VACUUM_PARTICLE_SCENARIOS.md
//
//  12 of 15 case bodies mirror s0-seed-* injectors verbatim (just renamed);
//  3 neutrino flavors + π⁰ + K± are net-new in this file.
// ==========================================================================

#include "ftd/scenarios.h"
#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/voxel.h"

#include "_helpers.h"

#include <cmath>
#include <string>

namespace ftd {

// Local helpers — minimal versions of the JS injectDressedParticle / injectTriad.
namespace {

static void v_dp(RenderBridge& rb, int cx, int cy, int cz,
                 int st, int sp, int co, double sig, double amp, bool lock) {
    IPF(rb, cx, cy, cz, st, sp, (co >= 0) ? co : 0);
    if (lock) LOCK(rb, cx, cy, cz);
    int sn = (st > 0) ? 1 : -1;  // matches JS injectDressedParticle: state=0 → sn=-1 (inward)
    int eR = CEL(3.0 * sig);
    for (int dz2 = -eR; dz2 <= eR; ++dz2)
    for (int dy2 = -eR; dy2 <= eR; ++dy2)
    for (int dx2 = -eR; dx2 <= eR; ++dx2) {
        if (dx2 == 0 && dy2 == 0 && dz2 == 0) continue;
        double r22 = dx2*dx2 + dy2*dy2 + dz2*dz2;
        double rr  = std::sqrt(r22);
        if (rr > 3.0 * sig) continue;
        double gg = amp * std::exp(-r22 / (2.0 * sig * sig));
        if (gg < 0.001) continue;
        IF(rb, cx+dx2, cy+dy2, cz+dz2, sn*gg*dx2/rr, sn*gg*dy2/rr, sn*gg*dz2/rr);
    }
}

static void v_tri(RenderBridge& rb, int cx, int cy, int cz,
                  const int charges[3], const int colors[3], int rad, bool lock) {
    static const double angs[3] = {0.0, 2.0 * PI / 3.0, 4.0 * PI / 3.0};
    for (int k = 0; k < 3; ++k) {
        int qx = RND(cx + rad * std::cos(angs[k]));
        int qy = RND(cy + rad * std::sin(angs[k]));
        v_dp(rb, qx, qy, cz, charges[k], (k % 2 == 0) ? 1 : -1, colors[k],
             2.0, K_B * 0.5, lock);
    }
}

}  // namespace

bool setup_vacuum_scenario(RenderBridge& rb, const std::string& name) {
    if (name.compare(0, 10, "s0-vacuum-") != 0) return false;

    const int N = rb.lattice().size();   // ← rb.lattice().size() confirmed from s0_seed.cpp line 73
    const double midF = (N - 1) / 2.0;
    const int mc = RND(midF);

    apply_vacuum_environment(rb);

    if (name == "s0-vacuum-electron") {
        IP(rb, mc, mc, mc, -1);
        const int envR = std::max(3, N / 6);
        const double envSigma = envR / 2.0;
        const double envAmp = K_B * 1.5;
        const double envR2 = envR * envR;
        const int eLo = FLR(midF) - envR, eHi = CEL(midF) + envR;
        for (int z = eLo; z <= eHi; ++z)
        for (int y = eLo; y <= eHi; ++y)
        for (int x = eLo; x <= eHi; ++x) {
            double dx = x - midF, dy = y - midF, dz = z - midF;
            double r2 = dx*dx + dy*dy + dz*dz;
            if (r2 < 0.25 || r2 > envR2) continue;
            double r = std::sqrt(r2);
            double v = envAmp * std::exp(-r2 / (2.0 * envSigma * envSigma));
            if (v < 0.001) continue;
            IF(rb, x, y, z, -v*dx/r, -v*dy/r, -v*dz/r);
        }
        return true;
    }

    if (name == "s0-vacuum-muon" || name == "s0-vacuum-tau") {
        const double boost = (name == "s0-vacuum-tau") ? 2.25 : 1.80;
        IP(rb, mc, mc, mc, -1);
        const int envR = std::max(3, N / 6);
        const double envSigma = envR / 2.0;
        const double envAmp = K_B * boost;
        const double envR2 = envR * envR;
        const int eLo = FLR(midF) - envR, eHi = CEL(midF) + envR;
        for (int z = eLo; z <= eHi; ++z)
        for (int y = eLo; y <= eHi; ++y)
        for (int x = eLo; x <= eHi; ++x) {
            double dx = x - midF, dy = y - midF, dz = z - midF;
            double r2 = dx*dx + dy*dy + dz*dz;
            if (r2 < 0.25 || r2 > envR2) continue;
            double r = std::sqrt(r2);
            double v = envAmp * std::exp(-r2 / (2.0 * envSigma * envSigma));
            if (v < 0.001) continue;
            IF(rb, x, y, z, -v*dx/r, -v*dy/r, -v*dz/r);
        }
        return true;
    }

    if (name == "s0-vacuum-photon") {
        const double sigma = 3.0;
        const double pAmp = K_B * 2.0;
        const int pStartX = std::max(4, N / 4);
        const int halfR = 8;
        for (int z = 0; z < N; ++z)
        for (int y = 0; y < N; ++y)
        for (int dx = -halfR; dx <= halfR; ++dx) {
            int x = pStartX + dx;
            if (x < 0 || x >= N) continue;
            double dy = y - midF, dz = z - midF;
            double r2 = dx*dx + dy*dy + dz*dz;
            double g = pAmp * std::exp(-r2 / (2.0 * sigma * sigma));
            if (g < 1e-6) continue;
            IF(rb, x, y, z, 0.0, 0.0, g);
            IW(rb, x, y, z, g, 0.0, 0.0);
        }
        return true;
    }

    if (name == "s0-vacuum-w-boson") {
        IPF(rb, mc, mc, mc, +1, +1, 0);
        const double sigma = 1.8;
        const double amp = K_B * 1.6;
        const int eR = 5;
        const double eR2 = eR * eR;
        for (int dz = -eR; dz <= eR; ++dz)
        for (int dy = -eR; dy <= eR; ++dy)
        for (int dx = -eR; dx <= eR; ++dx) {
            double r2 = dx*dx + dy*dy + dz*dz;
            if (r2 == 0 || r2 > eR2) continue;
            double r = std::sqrt(r2);
            double v = amp * std::exp(-r2 / (2.0 * sigma * sigma));
            if (v < 0.001) continue;
            IF(rb, mc+dx, mc+dy, mc+dz, v*1.3*dx/r, v*dy/r, v*dz/r);
        }
        return true;
    }

    if (name == "s0-vacuum-z-boson") {
        const double sigma = 2.0;
        const double amp = K_B * 1.8;
        const int eR = 6;
        const double eR2 = eR * eR;
        for (int dz = -eR; dz <= eR; ++dz)
        for (int dy = -eR; dy <= eR; ++dy)
        for (int dx = -eR; dx <= eR; ++dx) {
            double r2 = dx*dx + dy*dy + dz*dz;
            if (r2 == 0 || r2 > eR2) continue;
            double r = std::sqrt(r2);
            double v = amp * std::exp(-r2 / (2.0 * sigma * sigma));
            if (v < 0.001) continue;
            IF(rb, mc+dx, mc+dy, mc+dz, -v*dx/r, -v*dy/r, -v*dz/r);
        }
        return true;
    }

    if (name == "s0-vacuum-higgs") {
        const double hSig = 2.0, hAmp = K_B * 1.2;
        const int hR = 6;
        const double hR2 = hR * hR;
        for (int dz = -hR; dz <= hR; ++dz)
        for (int dy = -hR; dy <= hR; ++dy)
        for (int dx = -hR; dx <= hR; ++dx) {
            double r2 = dx*dx + dy*dy + dz*dz;
            if (r2 == 0 || r2 > hR2) continue;
            double g = hAmp * std::exp(-r2 / (2.0 * hSig * hSig));
            if (g < 0.001) continue;
            double iso = g / std::sqrt(3.0);
            IF(rb, mc+dx, mc+dy, mc+dz, iso, iso, iso);
        }
        return true;
    }

    if (name == "s0-vacuum-proton") {
        const int charges[3] = {+1, +1, -1};
        const int colors[3]  = {1, 2, 3};
        const int bR = std::max(2, N / 8);
        v_tri(rb, mc, mc, mc, charges, colors, bR, true);
        return true;
    }

    if (name == "s0-vacuum-neutron") {
        const int charges[3] = {+1, -1, -1};
        const int colors[3]  = {1, 2, 3};
        const int bR = std::max(2, N / 8);
        v_tri(rb, mc, mc, mc, charges, colors, bR, true);
        return true;
    }

    if (name == "s0-vacuum-pion-charged") {
        const int sp = std::max(3, N / 8);
        const int hf = sp / 2;
        v_dp(rb, mc + hf, mc, mc, +1, +1, 1, 2.0, K_B * 0.5, true);
        v_dp(rb, mc - hf, mc, mc, -1, -1, 1, 2.0, K_B * 0.5, true);
        return true;
    }

    if (name == "s0-vacuum-electron-neutrino"
        || name == "s0-vacuum-muon-neutrino"
        || name == "s0-vacuum-tau-neutrino") {
        const double boost =
            name == "s0-vacuum-tau-neutrino"  ? 1.6 :
            name == "s0-vacuum-muon-neutrino" ? 1.3 : 1.0;
        const double sig = 2.0;
        const int eR = 6;
        const double eR2 = eR * eR;
        for (int dz2 = -eR; dz2 <= eR; ++dz2)
        for (int dy2 = -eR; dy2 <= eR; ++dy2)
        for (int dx2 = -eR; dx2 <= eR; ++dx2) {
            double r22 = dx2*dx2 + dy2*dy2 + dz2*dz2;
            if (r22 > eR2) continue;
            double gg = K_B * 0.3 * boost * std::exp(-r22 / (2.0 * sig * sig));
            if (gg < 0.001) continue;
            IF(rb, mc+dx2, mc+dy2, mc+dz2, gg*0.55, gg*0.45, 0.0);
        }
        return true;
    }

    if (name == "s0-vacuum-pion-neutral") {
        const int sp = std::max(3, N / 8);
        const int hf = sp / 2;
        v_dp(rb, mc + hf, mc, mc, 0, +1, 1, 2.0, K_B * 0.5, true);
        v_dp(rb, mc - hf, mc, mc, 0, -1, 1, 2.0, K_B * 0.5, true);
        return true;
    }

    if (name == "s0-vacuum-kaon-charged") {
        const int sp = std::max(3, N / 8);
        const int hf = sp / 2;
        const double kBoost = 1.88;
        v_dp(rb, mc + hf, mc, mc, +1, +1, 1, 2.0, K_B * 0.5 * kBoost, true);
        v_dp(rb, mc - hf, mc, mc, -1, -1, 1, 2.0, K_B * 0.5 * kBoost, true);
        return true;
    }

    return true;  // matched the prefix but no body — keeps the dispatcher from
                  // falling through to s0-seed-*.
}

}  // namespace ftd
