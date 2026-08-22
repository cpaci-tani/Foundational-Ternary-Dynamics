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
    const int mc = RND((N - 1) / 2.0);   // nearest voxel to the geometric centre
    // Centre the seeded field/packet templates on the MARKER voxel (mc), not the
    // raw geometric centre (N-1)/2. On an EVEN lattice those differ by half a
    // voxel: the marker sprite renders at the voxel centre (mc + 0.5) while a
    // field centred on (N-1)/2 converges at the box centre, so the −1/+1 marker
    // appeared offset from the flux burst it should sit inside. Pinning midF to
    // mc makes the radial field emanate exactly from the marker. (Odd lattices,
    // incl. the golden L=17, already have mc == (N-1)/2, so they are unchanged.)
    const double midF = mc;

    apply_vacuum_environment(rb);

    const auto configure_free_wave = [&](bool gauss = true) {
        configure_free_wave_terms(rb, gauss);
    };
    if (name == "s0-vacuum-electron") {
        // Scenario ID: s0-vacuum-electron
        // Qualification: one inert negative marker plus a selected inward
        // radial vector template under the source-free wave map. No charge
        // coupling, mass pole, spinor, or electron observable is present.
        configure_free_wave(false);
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
        // Qualification: exact 1.2x/1.5x amplitude copies of the electron-
        // labelled vector template, with the same inert negative marker.
        // Their linear trajectories coincide after amplitude normalization;
        // no lepton generation or mass distinction is encoded.
        configure_free_wave(false);
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

    if (name == "s0-vacuum-positron") {
        // Scenario ID: s0-vacuum-positron
        // Qualification: one inert positive marker plus a selected outward
        // radial vector template under the source-free wave map (charge-sign
        // mirror of s0-vacuum-electron). No charge coupling, mass pole,
        // spinor, or positron observable is present.
        configure_free_wave(false);
        IP(rb, mc, mc, mc, +1);
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
            IF(rb, x, y, z, v*dx/r, v*dy/r, v*dz/r);
        }
        return true;
    }

    if (name == "s0-vacuum-antimuon" || name == "s0-vacuum-antitau") {
        // Qualification: exact 1.2x/1.5x amplitude copies of the positron-
        // labelled vector template, with the same inert positive marker
        // (generation-boost mirror of s0-vacuum-{muon,tau}). No lepton
        // generation or mass distinction is encoded.
        configure_free_wave(false);
        const double boost = (name == "s0-vacuum-antitau") ? 2.25 : 1.80;
        IP(rb, mc, mc, mc, +1);
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
            IF(rb, x, y, z, v*dx/r, v*dy/r, v*dz/r);
        }
        return true;
    }

    if (name == "s0-vacuum-photon") {
        // Scenario ID: s0-vacuum-photon
        // Physical Purpose: Seeds a divergence-free transverse photon-candidate packet.
        // Initial Condition Parameters: None.
        // Expected Behaviour: Propagating electromagnetic wave packet with genesis disabled to avoid pair production.
        // Verification: shared one-way packet construction; photon identity remains [OPEN].
        configure_free_wave();
        inject_plane_packet_x(rb, std::max(5.0, N / 4.0), 3.0, K_B * 0.5, +1);
        return true;
    }

    if (name == "s0-vacuum-w-boson") {
        // Scenario ID: s0-vacuum-w-boson
        // Qualification: one inert positive marker and an anisotropic radial
        // vector template. No weak charge, mass pole, polarization
        // representation, or W-boson observable is present.
        configure_free_wave(false);
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

    if (name == "s0-vacuum-w-minus-boson") {
        // Scenario ID: s0-vacuum-w-minus-boson
        // Qualification: charge-sign mirror of s0-vacuum-w-boson (negative
        // marker, every field term sign-flipped). No weak charge, mass pole,
        // polarization representation, or W-boson observable is present.
        configure_free_wave(false);
        IPF(rb, mc, mc, mc, -1, -1, 0);
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
            IF(rb, mc+dx, mc+dy, mc+dz, -v*1.3*dx/r, -v*dy/r, -v*dz/r);
        }
        return true;
    }

    if (name == "s0-vacuum-z-boson") {
        // Scenario ID: s0-vacuum-z-boson
        // Qualification: an unmanifested inward radial vector template. No
        // neutral current, mass pole, polarization representation, or Z-boson
        // observable is present.
        configure_free_wave(false);
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
        // Qualification: an unmanifested equal-component three-vector blob.
        // It is not a scalar field and contains no Higgs potential, mass pole,
        // symmetry breaking, or decay observable.
        configure_free_wave(false);
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
        // Qualification: unlocked selected-color triad under only the static-
        // dressing force, color force, and movement phases. At L=24 it has
        // 3 sites at tick 8, 1 at tick 16, and none by tick 32.
        // The proton/bound-state identity is closed negative for this setup.
        configure_unlocked_composite_terms(rb);
        const int charges[3] = {+1, +1, -1};
        const int colors[3]  = {1, 2, 3};
        const int bR = std::max(2, N / 8);
        tri(rb, mc, mc, mc, charges, colors, bR, false);
        return true;
    }

    if (name == "s0-vacuum-neutron") {
        // Scenario ID: s0-vacuum-neutron
        // Qualification: alternate-polarity version of the unlocked selected-
        // color triad. At L=24 it has one surviving site at ticks 8/16/32 and
        // none by tick 64. The neutron/bound-state identity is closed negative.
        configure_unlocked_composite_terms(rb);
        const int charges[3] = {+1, -1, -1};
        const int colors[3]  = {1, 2, 3};
        const int bR = std::max(2, N / 8);
        tri(rb, mc, mc, mc, charges, colors, bR, false);
        return true;
    }

    if (name == "s0-vacuum-pion-charged") {
        // Scenario ID: s0-vacuum-pion-charged
        // Qualification: unlocked opposite-polarity selected-color pair. Both
        // sites are removed by the movement collision rule by tick 8 at L=24;
        // no bound charged-pion mode survives.
        configure_unlocked_composite_terms(rb);
        const int sp = std::max(3, N / 8);
        const int hf = sp / 2;
        // B4 (2026-07-27): place both markers before dressing either -- IPF
        // always zeroes flux at its own center, so dressing second-first would
        // silently discard whichever marker's dressing landed on the other.
        dp_place(rb, mc + hf, mc, mc, +1, +1, 1, false);
        dp_place(rb, mc - hf, mc, mc, -1, -1, 2, false);
        dp_dress(rb, mc + hf, mc, mc, +1, 2.0, K_B * 0.5);
        dp_dress(rb, mc - hf, mc, mc, -1, 2.0, K_B * 0.5);
        return true;
    }

    if (name == "s0-vacuum-electron-neutrino"
        || name == "s0-vacuum-muon-neutrino"
        || name == "s0-vacuum-tau-neutrino") {
        // Scenario IDs: s0-vacuum-{electron,muon,tau}-neutrino.
        // Qualification target: amplitude independence of one neutral native
        // packet.  The three cases differ only by imposed multipliers
        // 1.0/1.3/1.6; they contain no flavor label, mass term, oscillation,
        // weak interaction, or neutrino-identifying observable.
        // Scenario ID: s0-vacuum-electron-neutrino
        // Physical Purpose: Seeds an electron neutrino in vacuum (nu_e).
        // Initial Condition Parameters: None.
        // Expected Behaviour: Small-amplitude localized propagating neutral candidate packet.
        // Verification: amplitude-coded candidate only; neutrino identity remains [OPEN].
        const double boost =
            name == "s0-vacuum-tau-neutrino"  ? 1.6 :
            name == "s0-vacuum-muon-neutrino" ? 1.3 : 1.0;
        configure_free_wave();
        inject_transverse_packet_x(rb, std::max(5.0, N / 4.0), midF, midF,
                                   2.5, std::max(5.0, N / 5.0), K_B * 0.3 * boost, +1,
                                   2.0 * PI / std::max(8.0, N / 3.0));
        return true;
    }

    if (name == "s0-vacuum-electron-antineutrino"
        || name == "s0-vacuum-muon-antineutrino"
        || name == "s0-vacuum-tau-antineutrino") {
        // Scenario IDs: s0-vacuum-{electron,muon,tau}-antineutrino.
        // Qualification: direction-mirror of s0-vacuum-{electron,muon,tau}-
        // neutrino (same 1.0/1.3/1.6 amplitude code, opposite propagation
        // direction). No flavor label, mass term, oscillation, weak
        // interaction, or antineutrino-identifying observable is present.
        const double boost =
            name == "s0-vacuum-tau-antineutrino"  ? 1.6 :
            name == "s0-vacuum-muon-antineutrino" ? 1.3 : 1.0;
        configure_free_wave();
        inject_transverse_packet_x(rb, std::max(5.0, N / 4.0), midF, midF,
                                   2.5, std::max(5.0, N / 5.0), K_B * 0.3 * boost, -1,
                                   2.0 * PI / std::max(8.0, N / 3.0));
        return true;
    }

    if (name == "s0-vacuum-pion-neutral") {
        // Scenario ID: s0-vacuum-pion-neutral
        // Qualification: bit-identical alias of s0-vacuum-pion-charged. Both
        // sites are gone by tick 8; no neutral-specific degree of freedom or
        // bound pion mode is present.
        configure_unlocked_composite_terms(rb);
        const int sp = std::max(3, N / 8);
        const int hf = sp / 2;
        // B4 (2026-07-27): place both markers before dressing either -- IPF
        // always zeroes flux at its own center, so dressing second-first would
        // silently discard whichever marker's dressing landed on the other.
        dp_place(rb, mc + hf, mc, mc, +1, +1, 1, false);
        dp_place(rb, mc - hf, mc, mc, -1, -1, 2, false);
        dp_dress(rb, mc + hf, mc, mc, +1, 2.0, K_B * 0.5);
        dp_dress(rb, mc - hf, mc, mc, -1, 2.0, K_B * 0.5);
        return true;
    }

    if (name == "s0-vacuum-kaon-charged") {
        // Scenario ID: s0-vacuum-kaon-charged
        // Qualification: the same unlocked pair with an imposed 1.88 dressing
        // boost. Both sites are gone by tick 8 at L=24; the boost does not
        // produce binding and no kaon flavor or mass mechanism is present.
        configure_unlocked_composite_terms(rb);
        const int sp = std::max(3, N / 8);
        const int hf = sp / 2;
        const double kBoost = 1.88;
        // B4 (2026-07-27): place both markers before dressing either -- IPF
        // always zeroes flux at its own center, so dressing second-first would
        // silently discard whichever marker's dressing landed on the other.
        dp_place(rb, mc + hf, mc, mc, +1, +1, 1, false);
        dp_place(rb, mc - hf, mc, mc, -1, -1, 2, false);
        dp_dress(rb, mc + hf, mc, mc, +1, 2.0, K_B * 0.5 * kBoost);
        dp_dress(rb, mc - hf, mc, mc, -1, 2.0, K_B * 0.5 * kBoost);
        return true;
    }

    return false;
}

}  // namespace ftd
