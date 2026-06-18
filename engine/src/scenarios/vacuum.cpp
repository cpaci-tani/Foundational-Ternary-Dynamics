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


bool setup_vacuum_scenario(RenderBridge& rb, const std::string& name) {
    if (name.compare(0, 10, "s0-vacuum-") != 0) return false;

    const int N = rb.lattice().size();   // ← rb.lattice().size() confirmed from s0_seed.cpp line 73
    const double midF = (N - 1) / 2.0;
    const int mc = RND(midF);

    apply_vacuum_environment(rb);

    if (name == "s0-vacuum-electron") {
        // Scenario ID: s0-vacuum-electron
        // Physical Purpose: Seeds a physical electron in vacuum (e-).
        // Initial Condition Parameters: None.
        // Expected Behaviour: Central -1 charge surrounded by inward-pointing Coulomb dressing flux.
        // Discrepancy: None.
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
        // Scenario ID: s0-vacuum-tau
        // Physical Purpose: Seeds a physical tau lepton in vacuum (tau-).
        // Initial Condition Parameters: None.
        // Expected Behaviour: Central -1 charge with heavily boosted Coulomb dressing flux.
        // Discrepancy: None.
        // Scenario ID: s0-vacuum-muon
        // Physical Purpose: Seeds a physical muon in vacuum (mu-).
        // Initial Condition Parameters: None.
        // Expected Behaviour: Central -1 charge with boosted Coulomb dressing flux.
        // Discrepancy: None.
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
        // Scenario ID: s0-vacuum-photon
        // Physical Purpose: Seeds a physical photon in vacuum.
        // Initial Condition Parameters: None.
        // Expected Behaviour: Propagating electromagnetic wave packet with genesis disabled to avoid pair production.
        // Discrepancy: None.
        // genesis=false (audit 2026-04-28): a free EM wave should not pair-produce.
        rb.toggles.genesis = false;
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
        // Scenario ID: s0-vacuum-w-boson
        // Physical Purpose: Seeds a charged W boson in vacuum (W+/-).
        // Initial Condition Parameters: None.
        // Expected Behaviour: Localized +1 charge with a short-range heavy dressing field.
        // Discrepancy: None.
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
        // Scenario ID: s0-vacuum-z-boson
        // Physical Purpose: Seeds a neutral Z boson in vacuum (Z0).
        // Initial Condition Parameters: None.
        // Expected Behaviour: Localized neutral heavy dressing field without central charge.
        // Discrepancy: None.
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
        // Scenario ID: s0-vacuum-higgs
        // Physical Purpose: Seeds a physical Higgs boson in vacuum (H).
        // Initial Condition Parameters: None.
        // Expected Behaviour: Localized isotropic scalar dressing flux.
        // Discrepancy: None.
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
        // Scenario ID: s0-vacuum-proton
        // Physical Purpose: Seeds a physical proton in vacuum (p).
        // Initial Condition Parameters: None.
        // Expected Behaviour: A three-quark triad (+1, +1, -1) forming a stable bound baryon.
        // Discrepancy: None.
        const int charges[3] = {+1, +1, -1};
        const int colors[3]  = {1, 2, 3};
        const int bR = std::max(2, N / 8);
        tri(rb, mc, mc, mc, charges, colors, bR, true);
        return true;
    }

    if (name == "s0-vacuum-neutron") {
        // Scenario ID: s0-vacuum-neutron
        // Physical Purpose: Seeds a physical neutron in vacuum (n).
        // Initial Condition Parameters: None.
        // Expected Behaviour: A three-quark triad (+1, -1, -1) forming a stable bound baryon.
        // Discrepancy: None.
        const int charges[3] = {+1, -1, -1};
        const int colors[3]  = {1, 2, 3};
        const int bR = std::max(2, N / 8);
        tri(rb, mc, mc, mc, charges, colors, bR, true);
        return true;
    }

    if (name == "s0-vacuum-pion-charged") {
        // Scenario ID: s0-vacuum-pion-charged
        // Physical Purpose: Seeds a physical charged pion in vacuum (pi+/-).
        // Initial Condition Parameters: None.
        // Expected Behaviour: A bound quark-antiquark meson pair with charges (+1, -1).
        // Discrepancy: None.
        const int sp = std::max(3, N / 8);
        const int hf = sp / 2;
        dp(rb, mc + hf, mc, mc, +1, +1, 1, 2.0, K_B * 0.5, true);
        dp(rb, mc - hf, mc, mc, -1, -1, 1, 2.0, K_B * 0.5, true);
        return true;
    }

    if (name == "s0-vacuum-electron-neutrino"
        || name == "s0-vacuum-muon-neutrino"
        || name == "s0-vacuum-tau-neutrino") {
        // Scenario ID: s0-vacuum-tau-neutrino
        // Physical Purpose: Seeds a tau neutrino in vacuum (nu_tau).
        // Initial Condition Parameters: None.
        // Expected Behaviour: Large-amplitude localized propagating neutral wave packet.
        // Discrepancy: None.
        // Scenario ID: s0-vacuum-muon-neutrino
        // Physical Purpose: Seeds a muon neutrino in vacuum (nu_mu).
        // Initial Condition Parameters: None.
        // Expected Behaviour: Intermediate-amplitude localized propagating neutral wave packet.
        // Discrepancy: None.
        // Scenario ID: s0-vacuum-electron-neutrino
        // Physical Purpose: Seeds an electron neutrino in vacuum (nu_e).
        // Initial Condition Parameters: None.
        // Expected Behaviour: Small-amplitude localized propagating neutral wave packet.
        // Discrepancy: None.
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
            IW(rb, mc+dx2, mc+dy2, mc+dz2, gg*0.55, gg*0.45, 0.0);
        }
        return true;
    }

    if (name == "s0-vacuum-pion-neutral") {
        // Scenario ID: s0-vacuum-pion-neutral
        // Physical Purpose: Seeds a physical neutral pion in vacuum (pi0).
        // Initial Condition Parameters: None.
        // Expected Behaviour: A bound quark-antiquark meson pair with neutral charges.
        // Discrepancy: None.
        const int sp = std::max(3, N / 8);
        const int hf = sp / 2;
        dp(rb, mc + hf, mc, mc, 0, +1, 1, 2.0, K_B * 0.5, true);
        dp(rb, mc - hf, mc, mc, 0, -1, 1, 2.0, K_B * 0.5, true);
        return true;
    }

    if (name == "s0-vacuum-kaon-charged") {
        // Scenario ID: s0-vacuum-kaon-charged
        // Physical Purpose: Seeds a physical charged kaon in vacuum (K+/-).
        // Initial Condition Parameters: None.
        // Expected Behaviour: A bound quark-antiquark meson pair with boosted mass energy.
        // Discrepancy: None.
        const int sp = std::max(3, N / 8);
        const int hf = sp / 2;
        const double kBoost = 1.88;
        dp(rb, mc + hf, mc, mc, +1, +1, 1, 2.0, K_B * 0.5 * kBoost, true);
        dp(rb, mc - hf, mc, mc, -1, -1, 1, 2.0, K_B * 0.5 * kBoost, true);
        return true;
    }

    return true;  // matched the prefix but no body — keeps the dispatcher from
                  // falling through to s0-seed-*.
}

}  // namespace ftd
