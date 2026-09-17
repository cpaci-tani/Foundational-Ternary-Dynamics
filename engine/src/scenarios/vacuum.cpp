// ==========================================================================
//  engine/src/scenarios/vacuum.cpp
//
//  Group: s0-vacuum-* (15 scenarios)
//  Canonical seed implementation; the former JS mirror is archived.
//  Spec:      engine/web/docs/SPEC_VACUUM_PARTICLE_SCENARIOS.md
//
//  12 of 15 case bodies mirror s0-seed-* injectors verbatim (just renamed);
//  3 neutrino flavors + π⁰ + K± are net-new in this file.
//
//  Parameterization pass (2026-09): every meaningful hardcoded seed input is
//  now exposed via ftd::seed::{real,integer,choice,boolean}. With no
//  ftd::seed::Context installed (the legacy dispatch_scenario() path), every
//  call below returns its literal default unchanged, so behavior, arithmetic
//  order, centering/rounding, and RNG draw ordering are bit-identical to the
//  pre-parameterization file. Exposing an amplitude, radius, sigma, boost
//  factor, or color label as a bounded, labeled input is a preparation-time
//  convenience, not a new physics claim; qualification language in each
//  scenario's comment block is unchanged.
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
// Per-axis construction center, read once per scenario body before any
// placement loop. All 15 s0-vacuum-* bodies previously hardcoded their
// construction center to the lattice midpoint (mc/midF) with no override —
// this is the single shared fix threaded through every call site below.
// Returns both the double-precision center (for envelope/Gaussian math, which
// used the unrounded midF historically) and its rounded integer form (for
// marker placement, which used the rounded mc historically).
struct SourceCenter { double x, y, z; int ix, iy, iz; };

inline SourceCenter read_source_center(int N, double default_center) {
    SourceCenter c;
    c.x = seed::real("source.x", default_center, 0, N - 1,
        "Center x", "Lattice x-coordinate of the construction's center.", "lattice units", 1.0);
    c.y = seed::real("source.y", default_center, 0, N - 1,
        "Center y", "Lattice y-coordinate of the construction's center.", "lattice units", 1.0);
    c.z = seed::real("source.z", default_center, 0, N - 1,
        "Center z", "Lattice z-coordinate of the construction's center.", "lattice units", 1.0);
    c.ix = RND(c.x); c.iy = RND(c.y); c.iz = RND(c.z);
    return c;
}

// The three pion/kaon two-marker scenarios below previously hardcoded their
// separation to the x axis (mc+hf, mc-hf, y/z pinned at center). This
// computes the two marker positions along a caller-selected axis instead.
struct AxisPair { int x0, y0, z0, x1, y1, z1; };
inline AxisPair pair_along_axis(const SourceCenter& c, int half_separation, int axis) {
    AxisPair p{c.ix, c.iy, c.iz, c.ix, c.iy, c.iz};
    if (axis == 1)      { p.y0 += half_separation; p.y1 -= half_separation; }
    else if (axis == 2) { p.z0 += half_separation; p.z1 -= half_separation; }
    else                { p.x0 += half_separation; p.x1 -= half_separation; }
    return p;
}
}  // namespace

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

    // Reused across the marker-polarity seeds below: a ±1 ternary choice.
    static const seed::Options POLARITY = {{-1, "Negative (-1)"}, {1, "Positive (+1)"}};

    if (name == "s0-vacuum-electron") {
        // Scenario ID: s0-vacuum-electron
        // Qualification: one inert negative marker plus a selected inward
        // radial vector template under the source-free wave map. No charge
        // coupling, mass pole, spinor, or electron observable is present.
        // Initial Condition Parameters: see the ftd::seed bindings below.
        configure_free_wave(false);
        const int centerPolarity = seed::choice("constituent0.polarity", -1, POLARITY,
            "Center polarity", "Ternary manifestation state of the central marker.");
        const SourceCenter c = read_source_center(N, midF);
        const int envRadiusDivisor = seed::integer("geometry.envelopeRadiusDivisor", 6, 2, 64,
            "Envelope radius divisor", "N/divisor (floored at 3) sets the radial envelope's outer radius.");
        const double envSigmaFraction = seed::real("geometry.envelopeSigmaFraction", 0.5, 0.05, 2.0,
            "Envelope sigma fraction", "Fraction of the envelope radius used as the Gaussian falloff width.", "fraction", 0.01);
        const double ampMultiplier = seed::real("source.amplitudeMultiplier", 1.5, 0.0, 10.0,
            "Envelope amplitude multiplier", "Multiplies K_B to set the radial envelope's peak amplitude.", "×K_B", 0.05);
        IP(rb, c.ix, c.iy, c.iz, centerPolarity);
        const int envR = std::max(3, N / envRadiusDivisor);
        const double envSigma = envR * envSigmaFraction;
        const double envAmp = K_B * ampMultiplier;
        const double envR2 = envR * envR;
        const int xLo = FLR(c.x) - envR, xHi = CEL(c.x) + envR;
        const int yLo = FLR(c.y) - envR, yHi = CEL(c.y) + envR;
        const int zLo = FLR(c.z) - envR, zHi = CEL(c.z) + envR;
        for (int z = zLo; z <= zHi; ++z)
        for (int y = yLo; y <= yHi; ++y)
        for (int x = xLo; x <= xHi; ++x) {
            double dx = x - c.x, dy = y - c.y, dz = z - c.z;
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
        // Initial Condition Parameters: see the ftd::seed bindings below.
        configure_free_wave(false);
        const double boostDefault = (name == "s0-vacuum-tau") ? 2.25 : 1.80;
        const int centerPolarity = seed::choice("constituent0.polarity", -1, POLARITY,
            "Center polarity", "Ternary manifestation state of the central marker.");
        const int envRadiusDivisor = seed::integer("geometry.envelopeRadiusDivisor", 6, 2, 64,
            "Envelope radius divisor", "N/divisor (floored at 3) sets the radial envelope's outer radius.");
        const double envSigmaFraction = seed::real("geometry.envelopeSigmaFraction", 0.5, 0.05, 2.0,
            "Envelope sigma fraction", "Fraction of the envelope radius used as the Gaussian falloff width.", "fraction", 0.01);
        const SourceCenter c = read_source_center(N, midF);
        const double ampMultiplier = seed::real("source.amplitudeMultiplier", boostDefault, 0.0, 10.0,
            "Envelope amplitude multiplier", "Multiplies K_B to set the radial envelope's peak amplitude; the registered 1.80/2.25 generation-boost reading assumes the default.", "×K_B", 0.05);
        IP(rb, c.ix, c.iy, c.iz, centerPolarity);
        const int envR = std::max(3, N / envRadiusDivisor);
        const double envSigma = envR * envSigmaFraction;
        const double envAmp = K_B * ampMultiplier;
        const double envR2 = envR * envR;
        const int xLo = FLR(c.x) - envR, xHi = CEL(c.x) + envR;
        const int yLo = FLR(c.y) - envR, yHi = CEL(c.y) + envR;
        const int zLo = FLR(c.z) - envR, zHi = CEL(c.z) + envR;
        for (int z = zLo; z <= zHi; ++z)
        for (int y = yLo; y <= yHi; ++y)
        for (int x = xLo; x <= xHi; ++x) {
            double dx = x - c.x, dy = y - c.y, dz = z - c.z;
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
        // Initial Condition Parameters: see the ftd::seed bindings below.
        configure_free_wave(false);
        const int centerPolarity = seed::choice("constituent0.polarity", +1, POLARITY,
            "Center polarity", "Ternary manifestation state of the central marker.");
        const int envRadiusDivisor = seed::integer("geometry.envelopeRadiusDivisor", 6, 2, 64,
            "Envelope radius divisor", "N/divisor (floored at 3) sets the radial envelope's outer radius.");
        const double envSigmaFraction = seed::real("geometry.envelopeSigmaFraction", 0.5, 0.05, 2.0,
            "Envelope sigma fraction", "Fraction of the envelope radius used as the Gaussian falloff width.", "fraction", 0.01);
        const double ampMultiplier = seed::real("source.amplitudeMultiplier", 1.5, 0.0, 10.0,
            "Envelope amplitude multiplier", "Multiplies K_B to set the radial envelope's peak amplitude.", "×K_B", 0.05);
        const SourceCenter c = read_source_center(N, midF);
        IP(rb, c.ix, c.iy, c.iz, centerPolarity);
        const int envR = std::max(3, N / envRadiusDivisor);
        const double envSigma = envR * envSigmaFraction;
        const double envAmp = K_B * ampMultiplier;
        const double envR2 = envR * envR;
        const int xLo = FLR(c.x) - envR, xHi = CEL(c.x) + envR;
        const int yLo = FLR(c.y) - envR, yHi = CEL(c.y) + envR;
        const int zLo = FLR(c.z) - envR, zHi = CEL(c.z) + envR;
        for (int z = zLo; z <= zHi; ++z)
        for (int y = yLo; y <= yHi; ++y)
        for (int x = xLo; x <= xHi; ++x) {
            double dx = x - c.x, dy = y - c.y, dz = z - c.z;
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
        // Initial Condition Parameters: see the ftd::seed bindings below.
        configure_free_wave(false);
        const double boostDefault = (name == "s0-vacuum-antitau") ? 2.25 : 1.80;
        const int centerPolarity = seed::choice("constituent0.polarity", +1, POLARITY,
            "Center polarity", "Ternary manifestation state of the central marker.");
        const int envRadiusDivisor = seed::integer("geometry.envelopeRadiusDivisor", 6, 2, 64,
            "Envelope radius divisor", "N/divisor (floored at 3) sets the radial envelope's outer radius.");
        const double envSigmaFraction = seed::real("geometry.envelopeSigmaFraction", 0.5, 0.05, 2.0,
            "Envelope sigma fraction", "Fraction of the envelope radius used as the Gaussian falloff width.", "fraction", 0.01);
        const double ampMultiplier = seed::real("source.amplitudeMultiplier", boostDefault, 0.0, 10.0,
            "Envelope amplitude multiplier", "Multiplies K_B to set the radial envelope's peak amplitude; the registered 1.80/2.25 generation-boost reading assumes the default.", "×K_B", 0.05);
        const SourceCenter c = read_source_center(N, midF);
        IP(rb, c.ix, c.iy, c.iz, centerPolarity);
        const int envR = std::max(3, N / envRadiusDivisor);
        const double envSigma = envR * envSigmaFraction;
        const double envAmp = K_B * ampMultiplier;
        const double envR2 = envR * envR;
        const int xLo = FLR(c.x) - envR, xHi = CEL(c.x) + envR;
        const int yLo = FLR(c.y) - envR, yHi = CEL(c.y) + envR;
        const int zLo = FLR(c.z) - envR, zHi = CEL(c.z) + envR;
        for (int z = zLo; z <= zHi; ++z)
        for (int y = yLo; y <= yHi; ++y)
        for (int x = xLo; x <= xHi; ++x) {
            double dx = x - c.x, dy = y - c.y, dz = z - c.z;
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
        // Initial Condition Parameters: see the ftd::seed bindings below.
        // Expected Behaviour: Propagating electromagnetic wave packet with genesis disabled to avoid pair production.
        // Verification: shared one-way packet construction; photon identity remains [OPEN].
        configure_free_wave();
        const double centerDivisor = seed::real("geometry.centerDivisor", 4.0, 1.0, 64.0,
            "Packet center divisor", "N/divisor sets the plane packet's x center (floored at 5).", "fraction of N", 0.1);
        const double packetSigma = seed::real("packet.sigma", 3.0, 0.1, 20.0,
            "Packet sigma", "Gaussian falloff width of the plane packet along x.", "lattice units", 0.1);
        const double ampMultiplier = seed::real("source.amplitudeMultiplier", 0.5, 0.0, 10.0,
            "Packet amplitude multiplier", "Multiplies K_B to set the plane packet's peak amplitude.", "×K_B", 0.05);
        const int seed_wave_0_direction = seed::choice("wave0.direction", +1, {{-1,"-x"},{1,"+x"}}, "Wave 1 direction", "Changes the travel direction of wave ingredient 1 along x.");
        const double seed_wave_0_carrierK = seed::real("wave0.carrierK", 0.0, 0.0, PI, "Wave 1 carrier wavenumber", "Sets wave 1 carrier wavenumber of wave ingredient 1; increasing it changes the prepared profile before the first tick.", "radians per cell");
        inject_plane_packet_x(rb, std::max(5.0, N / centerDivisor), packetSigma, K_B * ampMultiplier, seed_wave_0_direction, seed_wave_0_carrierK);
        return true;
    }

    if (name == "s0-vacuum-w-boson") {
        // Scenario ID: s0-vacuum-w-boson
        // Qualification: one inert positive marker and an anisotropic radial
        // vector template. No weak charge, mass pole, polarization
        // representation, or W-boson observable is present.
        // Initial Condition Parameters: see the ftd::seed bindings below.
        configure_free_wave(false);
        const int centerPolarity = seed::choice("constituent0.polarity", +1, POLARITY,
            "Center polarity", "Ternary manifestation state (and tied spin label) of the central marker.");
        const SourceCenter c = read_source_center(N, mc);
        IPF(rb, c.ix, c.iy, c.iz, centerPolarity, centerPolarity, 0);
        const double sigma = seed::real("packet.sigma", 1.8, 0.1, 10.0,
            "Envelope sigma", "Gaussian falloff width of the anisotropic radial envelope.", "lattice units", 0.1);
        const double ampMultiplier = seed::real("source.amplitudeMultiplier", 1.6, 0.0, 10.0,
            "Envelope amplitude multiplier", "Multiplies K_B to set the envelope's peak amplitude.", "×K_B", 0.05);
        const double amp = K_B * ampMultiplier;
        const int eR = seed::integer("geometry.envelopeRadius", 5, 1, 30,
            "Envelope radius", "Integer voxel radius of the anisotropic envelope.", "voxels");
        const double anisotropyFactor = seed::real("source.anisotropyFactor", 1.3, 0.0, 5.0,
            "x-axis anisotropy factor", "Multiplies only the x-component of the outward unit vector before scaling by the envelope amplitude.", "dimensionless", 0.05);
        const double eR2 = eR * eR;
        for (int dz = -eR; dz <= eR; ++dz)
        for (int dy = -eR; dy <= eR; ++dy)
        for (int dx = -eR; dx <= eR; ++dx) {
            double r2 = dx*dx + dy*dy + dz*dz;
            if (r2 == 0 || r2 > eR2) continue;
            double r = std::sqrt(r2);
            double v = amp * std::exp(-r2 / (2.0 * sigma * sigma));
            if (v < 0.001) continue;
            IF(rb, c.ix+dx, c.iy+dy, c.iz+dz, v*anisotropyFactor*dx/r, v*dy/r, v*dz/r);
        }
        return true;
    }

    if (name == "s0-vacuum-w-minus-boson") {
        // Scenario ID: s0-vacuum-w-minus-boson
        // Qualification: charge-sign mirror of s0-vacuum-w-boson (negative
        // marker, every field term sign-flipped). No weak charge, mass pole,
        // polarization representation, or W-boson observable is present.
        // Initial Condition Parameters: see the ftd::seed bindings below.
        configure_free_wave(false);
        const int centerPolarity = seed::choice("constituent0.polarity", -1, POLARITY,
            "Center polarity", "Ternary manifestation state (and tied spin label) of the central marker.");
        const SourceCenter c = read_source_center(N, mc);
        IPF(rb, c.ix, c.iy, c.iz, centerPolarity, centerPolarity, 0);
        const double sigma = seed::real("packet.sigma", 1.8, 0.1, 10.0,
            "Envelope sigma", "Gaussian falloff width of the anisotropic radial envelope.", "lattice units", 0.1);
        const double ampMultiplier = seed::real("source.amplitudeMultiplier", 1.6, 0.0, 10.0,
            "Envelope amplitude multiplier", "Multiplies K_B to set the envelope's peak amplitude.", "×K_B", 0.05);
        const double amp = K_B * ampMultiplier;
        const int eR = seed::integer("geometry.envelopeRadius", 5, 1, 30,
            "Envelope radius", "Integer voxel radius of the anisotropic envelope.", "voxels");
        const double anisotropyFactor = seed::real("source.anisotropyFactor", 1.3, 0.0, 5.0,
            "x-axis anisotropy factor", "Multiplies only the x-component of the outward unit vector before scaling by the envelope amplitude.", "dimensionless", 0.05);
        const double eR2 = eR * eR;
        for (int dz = -eR; dz <= eR; ++dz)
        for (int dy = -eR; dy <= eR; ++dy)
        for (int dx = -eR; dx <= eR; ++dx) {
            double r2 = dx*dx + dy*dy + dz*dz;
            if (r2 == 0 || r2 > eR2) continue;
            double r = std::sqrt(r2);
            double v = amp * std::exp(-r2 / (2.0 * sigma * sigma));
            if (v < 0.001) continue;
            IF(rb, c.ix+dx, c.iy+dy, c.iz+dz, -v*anisotropyFactor*dx/r, -v*dy/r, -v*dz/r);
        }
        return true;
    }

    if (name == "s0-vacuum-z-boson") {
        // Scenario ID: s0-vacuum-z-boson
        // Qualification: an unmanifested inward radial vector template. No
        // neutral current, mass pole, polarization representation, or Z-boson
        // observable is present.
        // Initial Condition Parameters: see the ftd::seed bindings below.
        configure_free_wave(false);
        const double sigma = seed::real("packet.sigma", 2.0, 0.1, 10.0,
            "Envelope sigma", "Gaussian falloff width of the inward radial envelope.", "lattice units", 0.1);
        const double ampMultiplier = seed::real("source.amplitudeMultiplier", 1.8, 0.0, 10.0,
            "Envelope amplitude multiplier", "Multiplies K_B to set the envelope's peak amplitude.", "×K_B", 0.05);
        const double amp = K_B * ampMultiplier;
        const int eR = seed::integer("geometry.envelopeRadius", 6, 1, 30,
            "Envelope radius", "Integer voxel radius of the inward radial envelope.", "voxels");
        const SourceCenter c = read_source_center(N, mc);
        const double eR2 = eR * eR;
        for (int dz = -eR; dz <= eR; ++dz)
        for (int dy = -eR; dy <= eR; ++dy)
        for (int dx = -eR; dx <= eR; ++dx) {
            double r2 = dx*dx + dy*dy + dz*dz;
            if (r2 == 0 || r2 > eR2) continue;
            double r = std::sqrt(r2);
            double v = amp * std::exp(-r2 / (2.0 * sigma * sigma));
            if (v < 0.001) continue;
            IF(rb, c.ix+dx, c.iy+dy, c.iz+dz, -v*dx/r, -v*dy/r, -v*dz/r);
        }
        return true;
    }

    if (name == "s0-vacuum-higgs") {
        // Scenario ID: s0-vacuum-higgs
        // Qualification: an unmanifested equal-component three-vector blob.
        // It is not a scalar field and contains no Higgs potential, mass pole,
        // symmetry breaking, or decay observable.
        // Initial Condition Parameters: see the ftd::seed bindings below.
        configure_free_wave(false);
        const double hSig = seed::real("packet.sigma", 2.0, 0.1, 10.0,
            "Blob sigma", "Gaussian falloff width of the equal-component three-vector blob.", "lattice units", 0.1);
        const double ampMultiplier = seed::real("source.amplitudeMultiplier", 1.2, 0.0, 10.0,
            "Blob amplitude multiplier", "Multiplies K_B to set the blob's peak amplitude.", "×K_B", 0.05);
        const double hAmp = K_B * ampMultiplier;
        const int hR = seed::integer("geometry.envelopeRadius", 6, 1, 30,
            "Blob radius", "Integer voxel radius of the equal-component three-vector blob.", "voxels");
        const SourceCenter c = read_source_center(N, mc);
        const double hR2 = hR * hR;
        for (int dz = -hR; dz <= hR; ++dz)
        for (int dy = -hR; dy <= hR; ++dy)
        for (int dx = -hR; dx <= hR; ++dx) {
            double r2 = dx*dx + dy*dy + dz*dz;
            if (r2 == 0 || r2 > hR2) continue;
            double g = hAmp * std::exp(-r2 / (2.0 * hSig * hSig));
            if (g < 0.001) continue;
            double iso = g / std::sqrt(3.0);
            IF(rb, c.ix+dx, c.iy+dy, c.iz+dz, iso, iso, iso);
        }
        return true;
    }

    if (name == "s0-vacuum-proton") {
        // Scenario ID: s0-vacuum-proton
        // Qualification: unlocked selected-color triad under only the static-
        // dressing force, color force, and movement phases. At L=24 it has
        // 3 sites at tick 8, 1 at tick 16, and none by tick 32.
        // The proton/bound-state identity is closed negative for this setup.
        // Initial Condition Parameters: see the ftd::seed bindings below.
        configure_unlocked_composite_terms(rb);
        const int constituent0Polarity = seed::choice("constituent0.polarity", +1, POLARITY,
            "Constituent 1 polarity", "Ternary manifestation state of the first triad member.");
        const int constituent1Polarity = seed::choice("constituent1.polarity", +1, POLARITY,
            "Constituent 2 polarity", "Ternary manifestation state of the second triad member.");
        const int constituent2Polarity = seed::choice("constituent2.polarity", -1, POLARITY,
            "Constituent 3 polarity", "Ternary manifestation state of the third triad member.");
        const int constituent0Color = seed::integer("constituent0.colorLabel", 1, 1, 3,
            "Constituent 1 color label", "Color label consumed by the active color force between triad members.");
        const int constituent1Color = seed::integer("constituent1.colorLabel", 2, 1, 3,
            "Constituent 2 color label", "Color label consumed by the active color force between triad members.");
        const int constituent2Color = seed::integer("constituent2.colorLabel", 3, 1, 3,
            "Constituent 3 color label", "Color label consumed by the active color force between triad members.");
        const int radiusDivisor = seed::integer("geometry.triadRadiusDivisor", 8, 2, 64,
            "Triad radius divisor", "N/divisor (floored at 2) sets the triad's circumradius.");
        const SourceCenter c = read_source_center(N, mc);
        const double rotationOffset = seed::real("geometry.triadRotationOffset", 0.0, -PI, PI,
            "Triad rotation offset", "Additive angle rotating all three triad vertices about the triad center; 0 reproduces the original fixed 120°-spaced layout.", "radians", 0.01);
        const double dressSigma = seed::real("constituent0.dressSigma", 2.0, 0.1, 10.0,
            "Triad dressing sigma", "Gaussian falloff width of each triad member's dressing (shared by symmetry).", "lattice units", 0.1);
        const double dressAmplitudeMultiplier = seed::real("constituent0.dressAmplitudeMultiplier", 0.5, 0.0, 5.0,
            "Triad dressing amplitude multiplier", "Multiplies K_B to set each triad member's dressing amplitude (shared by symmetry).", "×K_B", 0.05);
        const int charges[3] = {constituent0Polarity, constituent1Polarity, constituent2Polarity};
        const int colors[3]  = {constituent0Color, constituent1Color, constituent2Color};
        const int bR = std::max(2, N / radiusDivisor);
        tri(rb, c.ix, c.iy, c.iz, charges, colors, bR, false, rotationOffset,
            dressSigma, K_B * dressAmplitudeMultiplier);
        return true;
    }

    if (name == "s0-vacuum-neutron") {
        // Scenario ID: s0-vacuum-neutron
        // Qualification: alternate-polarity version of the unlocked selected-
        // color triad. At L=24 it has one surviving site at ticks 8/16/32 and
        // none by tick 64. The neutron/bound-state identity is closed negative.
        // Initial Condition Parameters: see the ftd::seed bindings below.
        configure_unlocked_composite_terms(rb);
        const int constituent0Polarity = seed::choice("constituent0.polarity", +1, POLARITY,
            "Constituent 1 polarity", "Ternary manifestation state of the first triad member.");
        const int constituent1Polarity = seed::choice("constituent1.polarity", -1, POLARITY,
            "Constituent 2 polarity", "Ternary manifestation state of the second triad member.");
        const int constituent2Polarity = seed::choice("constituent2.polarity", -1, POLARITY,
            "Constituent 3 polarity", "Ternary manifestation state of the third triad member.");
        const int constituent0Color = seed::integer("constituent0.colorLabel", 1, 1, 3,
            "Constituent 1 color label", "Color label consumed by the active color force between triad members.");
        const int constituent1Color = seed::integer("constituent1.colorLabel", 2, 1, 3,
            "Constituent 2 color label", "Color label consumed by the active color force between triad members.");
        const int constituent2Color = seed::integer("constituent2.colorLabel", 3, 1, 3,
            "Constituent 3 color label", "Color label consumed by the active color force between triad members.");
        const int radiusDivisor = seed::integer("geometry.triadRadiusDivisor", 8, 2, 64,
            "Triad radius divisor", "N/divisor (floored at 2) sets the triad's circumradius.");
        const SourceCenter c = read_source_center(N, mc);
        const double rotationOffset = seed::real("geometry.triadRotationOffset", 0.0, -PI, PI,
            "Triad rotation offset", "Additive angle rotating all three triad vertices about the triad center; 0 reproduces the original fixed 120°-spaced layout.", "radians", 0.01);
        const double dressSigma = seed::real("constituent0.dressSigma", 2.0, 0.1, 10.0,
            "Triad dressing sigma", "Gaussian falloff width of each triad member's dressing (shared by symmetry).", "lattice units", 0.1);
        const double dressAmplitudeMultiplier = seed::real("constituent0.dressAmplitudeMultiplier", 0.5, 0.0, 5.0,
            "Triad dressing amplitude multiplier", "Multiplies K_B to set each triad member's dressing amplitude (shared by symmetry).", "×K_B", 0.05);
        const int charges[3] = {constituent0Polarity, constituent1Polarity, constituent2Polarity};
        const int colors[3]  = {constituent0Color, constituent1Color, constituent2Color};
        const int bR = std::max(2, N / radiusDivisor);
        tri(rb, c.ix, c.iy, c.iz, charges, colors, bR, false, rotationOffset,
            dressSigma, K_B * dressAmplitudeMultiplier);
        return true;
    }

    if (name == "s0-vacuum-pion-charged") {
        // Scenario ID: s0-vacuum-pion-charged
        // Qualification: unlocked opposite-polarity selected-color pair. Both
        // sites are removed by the movement collision rule by tick 8 at L=24;
        // no bound charged-pion mode survives.
        // Initial Condition Parameters: see the ftd::seed bindings below.
        configure_unlocked_composite_terms(rb);
        const int separationDivisor = seed::integer("geometry.separationDivisor", 8, 2, 64,
            "Separation divisor", "N/divisor (floored at 3) sets the pair's total separation; half that value offsets each marker from center.");
        const int constituent0Polarity = seed::choice("constituent0.polarity", +1, POLARITY,
            "Constituent 1 polarity", "Ternary manifestation state (and tied spin label) of the +x marker.");
        const int constituent1Polarity = seed::choice("constituent1.polarity", -1, POLARITY,
            "Constituent 2 polarity", "Ternary manifestation state (and tied spin label) of the −x marker.");
        const int constituent0Color = seed::integer("constituent0.colorLabel", 1, 1, 3,
            "Constituent 1 color label", "Color label consumed by the active color force between the pair.");
        const int constituent1Color = seed::integer("constituent1.colorLabel", 2, 1, 3,
            "Constituent 2 color label", "Color label consumed by the active color force between the pair.");
        const double dressSigma = seed::real("constituent0.dressSigma", 2.0, 0.1, 10.0,
            "Dressing sigma", "Gaussian falloff width of each marker's dressing (shared by symmetry).", "lattice units", 0.1);
        const double dressAmplitudeMultiplier = seed::real("constituent0.dressAmplitudeMultiplier", 0.5, 0.0, 5.0,
            "Dressing amplitude multiplier", "Multiplies K_B to set each marker's dressing amplitude (shared by symmetry).", "×K_B", 0.05);
        const SourceCenter c = read_source_center(N, mc);
        const int separationAxis = seed::choice("geometry.separationAxis", 0,
            {{0, "x"}, {1, "y"}, {2, "z"}},
            "Separation axis", "Cartesian axis along which the pair is separated; the other two axes stay at the center.");
        const int sp = std::max(3, N / separationDivisor);
        const int hf = sp / 2;
        const double dressAmp = K_B * dressAmplitudeMultiplier;
        const AxisPair pos = pair_along_axis(c, hf, separationAxis);
        // B4 (2026-07-27): place both markers before dressing either -- IPF
        // always zeroes flux at its own center, so dressing second-first would
        // silently discard whichever marker's dressing landed on the other.
        dp_place(rb, pos.x0, pos.y0, pos.z0, constituent0Polarity, constituent0Polarity, constituent0Color, false);
        dp_place(rb, pos.x1, pos.y1, pos.z1, constituent1Polarity, constituent1Polarity, constituent1Color, false);
        dp_dress(rb, pos.x0, pos.y0, pos.z0, constituent0Polarity, dressSigma, dressAmp);
        dp_dress(rb, pos.x1, pos.y1, pos.z1, constituent1Polarity, dressSigma, dressAmp);
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
        // Initial Condition Parameters: see the ftd::seed bindings below.
        // Expected Behaviour: Small-amplitude localized propagating neutral candidate packet.
        // Verification: amplitude-coded candidate only; neutrino identity remains [OPEN].
        const double flavorBoostDefault =
            name == "s0-vacuum-tau-neutrino"  ? 1.6 :
            name == "s0-vacuum-muon-neutrino" ? 1.3 : 1.0;
        const double flavorAmplitudeMultiplier = seed::real("source.flavorAmplitudeMultiplier", flavorBoostDefault, 0.0, 10.0,
            "Flavor amplitude multiplier", "Selected per-flavor scale (1.0/1.3/1.6 for e/mu/tau); no flavor, mass, or oscillation mechanism is represented.", "dimensionless", 0.05);
        const double baseAmplitudeMultiplier = seed::real("source.baseAmplitudeMultiplier", 0.3, 0.0, 5.0,
            "Base amplitude multiplier", "Multiplies K_B, before the flavor multiplier, to set the packet's overall amplitude scale.", "×K_B", 0.01);
        const double centerDivisor = seed::real("geometry.centerDivisor", 4.0, 1.0, 64.0,
            "Packet center divisor", "N/divisor sets the packet's x center (floored at 5).", "fraction of N", 0.1);
        const double packetSigmaX = seed::real("packet.sigma", 2.5, 0.1, 20.0,
            "Packet sigma_x", "Gaussian falloff width of the transverse packet along x.", "lattice units", 0.1);
        const double widthDivisor = seed::real("geometry.widthDivisor", 5.0, 1.0, 64.0,
            "Packet width divisor", "N/divisor sets the transverse packet's sigma_t (floored at 5).", "fraction of N", 0.1);
        const double carrierDivisor = seed::real("geometry.carrierDivisor", 3.0, 1.0, 64.0,
            "Carrier wavenumber divisor", "N/divisor sets the carrier wavenumber's denominator (floored at 8).", "fraction of N", 0.1);
        const double transverseY = seed::real("source.y", midF, 0, N - 1,
            "Transverse center y", "Lattice y-coordinate of the transverse packet center.", "lattice units", 1.0);
        const double transverseZ = seed::real("source.z", midF, 0, N - 1,
            "Transverse center z", "Lattice z-coordinate of the transverse packet center.", "lattice units", 1.0);
        configure_free_wave();
        const int seed_wave_0_direction = seed::choice("wave0.direction", +1, {{-1,"-x"},{1,"+x"}}, "Wave 1 direction", "Changes the travel direction of wave ingredient 1 along x.");
        const double seed_wave_0_phase = seed::real("wave0.phase", 0.0, -PI, PI, "Wave 1 carrier phase", "Sets wave 1 carrier phase of wave ingredient 1; increasing it changes the prepared profile before the first tick.", "radians");
        inject_transverse_packet_x(rb, std::max(5.0, N / centerDivisor), transverseY, transverseZ, packetSigmaX, std::max(5.0, N / widthDivisor), K_B * baseAmplitudeMultiplier * flavorAmplitudeMultiplier, seed_wave_0_direction, 2.0 * PI / std::max(8.0, N / carrierDivisor), seed_wave_0_phase);
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
        // Initial Condition Parameters: see the ftd::seed bindings below.
        const double flavorBoostDefault =
            name == "s0-vacuum-tau-antineutrino"  ? 1.6 :
            name == "s0-vacuum-muon-antineutrino" ? 1.3 : 1.0;
        const double flavorAmplitudeMultiplier = seed::real("source.flavorAmplitudeMultiplier", flavorBoostDefault, 0.0, 10.0,
            "Flavor amplitude multiplier", "Selected per-flavor scale (1.0/1.3/1.6 for e/mu/tau); no flavor, mass, or oscillation mechanism is represented.", "dimensionless", 0.05);
        const double baseAmplitudeMultiplier = seed::real("source.baseAmplitudeMultiplier", 0.3, 0.0, 5.0,
            "Base amplitude multiplier", "Multiplies K_B, before the flavor multiplier, to set the packet's overall amplitude scale.", "×K_B", 0.01);
        const double centerDivisor = seed::real("geometry.centerDivisor", 4.0, 1.0, 64.0,
            "Packet center divisor", "N/divisor sets the packet's x center (floored at 5).", "fraction of N", 0.1);
        const double packetSigmaX = seed::real("packet.sigma", 2.5, 0.1, 20.0,
            "Packet sigma_x", "Gaussian falloff width of the transverse packet along x.", "lattice units", 0.1);
        const double widthDivisor = seed::real("geometry.widthDivisor", 5.0, 1.0, 64.0,
            "Packet width divisor", "N/divisor sets the transverse packet's sigma_t (floored at 5).", "fraction of N", 0.1);
        const double carrierDivisor = seed::real("geometry.carrierDivisor", 3.0, 1.0, 64.0,
            "Carrier wavenumber divisor", "N/divisor sets the carrier wavenumber's denominator (floored at 8).", "fraction of N", 0.1);
        const double transverseY = seed::real("source.y", midF, 0, N - 1,
            "Transverse center y", "Lattice y-coordinate of the transverse packet center.", "lattice units", 1.0);
        const double transverseZ = seed::real("source.z", midF, 0, N - 1,
            "Transverse center z", "Lattice z-coordinate of the transverse packet center.", "lattice units", 1.0);
        configure_free_wave();
        const int seed_wave_0_direction = seed::choice("wave0.direction", -1, {{-1,"-x"},{1,"+x"}}, "Wave 1 direction", "Changes the travel direction of wave ingredient 1 along x.");
        const double seed_wave_0_phase = seed::real("wave0.phase", 0.0, -PI, PI, "Wave 1 carrier phase", "Sets wave 1 carrier phase of wave ingredient 1; increasing it changes the prepared profile before the first tick.", "radians");
        inject_transverse_packet_x(rb, std::max(5.0, N / centerDivisor), transverseY, transverseZ, packetSigmaX, std::max(5.0, N / widthDivisor), K_B * baseAmplitudeMultiplier * flavorAmplitudeMultiplier, seed_wave_0_direction, 2.0 * PI / std::max(8.0, N / carrierDivisor), seed_wave_0_phase);
        return true;
    }

    if (name == "s0-vacuum-pion-neutral") {
        // Scenario ID: s0-vacuum-pion-neutral
        // Qualification: bit-identical alias of s0-vacuum-pion-charged. Both
        // sites are gone by tick 8; no neutral-specific degree of freedom or
        // bound pion mode is present.
        // Initial Condition Parameters: see the ftd::seed bindings below.
        configure_unlocked_composite_terms(rb);
        const int separationDivisor = seed::integer("geometry.separationDivisor", 8, 2, 64,
            "Separation divisor", "N/divisor (floored at 3) sets the pair's total separation; half that value offsets each marker from center.");
        const int constituent0Polarity = seed::choice("constituent0.polarity", +1, POLARITY,
            "Constituent 1 polarity", "Ternary manifestation state (and tied spin label) of the +x marker.");
        const int constituent1Polarity = seed::choice("constituent1.polarity", -1, POLARITY,
            "Constituent 2 polarity", "Ternary manifestation state (and tied spin label) of the −x marker.");
        const int constituent0Color = seed::integer("constituent0.colorLabel", 1, 1, 3,
            "Constituent 1 color label", "Color label consumed by the active color force between the pair.");
        const int constituent1Color = seed::integer("constituent1.colorLabel", 2, 1, 3,
            "Constituent 2 color label", "Color label consumed by the active color force between the pair.");
        const double dressSigma = seed::real("constituent0.dressSigma", 2.0, 0.1, 10.0,
            "Dressing sigma", "Gaussian falloff width of each marker's dressing (shared by symmetry).", "lattice units", 0.1);
        const double dressAmplitudeMultiplier = seed::real("constituent0.dressAmplitudeMultiplier", 0.5, 0.0, 5.0,
            "Dressing amplitude multiplier", "Multiplies K_B to set each marker's dressing amplitude (shared by symmetry).", "×K_B", 0.05);
        const SourceCenter c = read_source_center(N, mc);
        const int separationAxis = seed::choice("geometry.separationAxis", 0,
            {{0, "x"}, {1, "y"}, {2, "z"}},
            "Separation axis", "Cartesian axis along which the pair is separated; the other two axes stay at the center.");
        const int sp = std::max(3, N / separationDivisor);
        const int hf = sp / 2;
        const double dressAmp = K_B * dressAmplitudeMultiplier;
        const AxisPair pos = pair_along_axis(c, hf, separationAxis);
        // B4 (2026-07-27): place both markers before dressing either -- IPF
        // always zeroes flux at its own center, so dressing second-first would
        // silently discard whichever marker's dressing landed on the other.
        dp_place(rb, pos.x0, pos.y0, pos.z0, constituent0Polarity, constituent0Polarity, constituent0Color, false);
        dp_place(rb, pos.x1, pos.y1, pos.z1, constituent1Polarity, constituent1Polarity, constituent1Color, false);
        dp_dress(rb, pos.x0, pos.y0, pos.z0, constituent0Polarity, dressSigma, dressAmp);
        dp_dress(rb, pos.x1, pos.y1, pos.z1, constituent1Polarity, dressSigma, dressAmp);
        return true;
    }

    if (name == "s0-vacuum-kaon-charged") {
        // Scenario ID: s0-vacuum-kaon-charged
        // Qualification: the same unlocked pair with an imposed 1.88 dressing
        // boost. Both sites are gone by tick 8 at L=24; the boost does not
        // produce binding and no kaon flavor or mass mechanism is present.
        // Initial Condition Parameters: see the ftd::seed bindings below.
        configure_unlocked_composite_terms(rb);
        const int separationDivisor = seed::integer("geometry.separationDivisor", 8, 2, 64,
            "Separation divisor", "N/divisor (floored at 3) sets the pair's total separation; half that value offsets each marker from center.");
        const int constituent0Polarity = seed::choice("constituent0.polarity", +1, POLARITY,
            "Constituent 1 polarity", "Ternary manifestation state (and tied spin label) of the +x marker.");
        const int constituent1Polarity = seed::choice("constituent1.polarity", -1, POLARITY,
            "Constituent 2 polarity", "Ternary manifestation state (and tied spin label) of the −x marker.");
        const int constituent0Color = seed::integer("constituent0.colorLabel", 1, 1, 3,
            "Constituent 1 color label", "Color label consumed by the active color force between the pair.");
        const int constituent1Color = seed::integer("constituent1.colorLabel", 2, 1, 3,
            "Constituent 2 color label", "Color label consumed by the active color force between the pair.");
        const double dressSigma = seed::real("constituent0.dressSigma", 2.0, 0.1, 10.0,
            "Dressing sigma", "Gaussian falloff width of each marker's dressing (shared by symmetry).", "lattice units", 0.1);
        const double dressAmplitudeMultiplier = seed::real("constituent0.dressAmplitudeMultiplier", 0.5, 0.0, 5.0,
            "Dressing amplitude multiplier", "Multiplies K_B, before the kaon boost, to set each marker's dressing amplitude (shared by symmetry).", "×K_B", 0.05);
        const double kaonBoostMultiplier = seed::real("source.dressBoostMultiplier", 1.88, 0.0, 10.0,
            "Kaon dressing boost multiplier", "Multiplies the shared dressing amplitude; a boost of 1.0 reduces to the pion dressing.", "dimensionless", 0.01);
        const SourceCenter c = read_source_center(N, mc);
        const int separationAxis = seed::choice("geometry.separationAxis", 0,
            {{0, "x"}, {1, "y"}, {2, "z"}},
            "Separation axis", "Cartesian axis along which the pair is separated; the other two axes stay at the center.");
        const int sp = std::max(3, N / separationDivisor);
        const int hf = sp / 2;
        const double dressAmp = K_B * dressAmplitudeMultiplier * kaonBoostMultiplier;
        const AxisPair pos = pair_along_axis(c, hf, separationAxis);
        // B4 (2026-07-27): place both markers before dressing either -- IPF
        // always zeroes flux at its own center, so dressing second-first would
        // silently discard whichever marker's dressing landed on the other.
        dp_place(rb, pos.x0, pos.y0, pos.z0, constituent0Polarity, constituent0Polarity, constituent0Color, false);
        dp_place(rb, pos.x1, pos.y1, pos.z1, constituent1Polarity, constituent1Polarity, constituent1Color, false);
        dp_dress(rb, pos.x0, pos.y0, pos.z0, constituent0Polarity, dressSigma, dressAmp);
        dp_dress(rb, pos.x1, pos.y1, pos.z1, constituent1Polarity, dressSigma, dressAmp);
        return true;
    }

    return false;
}

}  // namespace ftd
