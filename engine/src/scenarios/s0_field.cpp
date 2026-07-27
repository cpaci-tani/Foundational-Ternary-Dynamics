// ==========================================================================
//  engine/src/scenarios/s0_field.cpp
//
//  Group: s0-field-* (9 scenarios)
//  JS source: engine/web/js/bridge/scenarios/s0-field-scenarios.js
//
//  Split out of engine/src/scenarios.cpp (ticket S1).
// ==========================================================================

#include "ftd/scenarios.h"
#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/voxel.h"

#include "_helpers.h"

#include <cmath>

namespace ftd {

bool setup_s0_field_scenario(RenderBridge& rb, const std::string& name) {
    if (name.rfind("s0-field-", 0) != 0) return false;
    const int    N    = rb.lattice().size();
    const double midF = (N - 1) * 0.5;
    const int    mc   = RND(midF);

    if (name == "s0-field-plane-wave") {
        // Scenario ID: s0-field-plane-wave
        // Physical Purpose: Exact traveling eigenmode of the native linear
        // kick-drift wave operator; no electromagnetic identity is assumed.
        // Initial Condition: mode n=4, z polarization, +x propagation.
        // Expected Behaviour: J_z=A sin(kx-omega*t) at the exact lattice pole.
        configure_free_wave_terms(rb, false);
        inject_plane_harmonic_x(rb, 4, K_B * 2.0, +1);
    }
    else if (name == "s0-field-standing-wave") {
        // Scenario ID: s0-field-standing-wave
        // Physical Purpose: Exact standing eigenmode of the native linear
        // kick-drift wave operator; no cavity or material boundary is implied.
        // Initial Condition: mode n=4, z polarization, exact pre-kick phase.
        // Expected Behaviour: J_z=A sin(kx) cos(omega*t), with fixed nodes.
        configure_free_wave_terms(rb, false);
        inject_standing_harmonic_x(rb, 4, K_B * 2.0);
    }
    else if (name == "s0-field-uniform-e") {
        configure_static_seed_terms(rb);
        // Scenario ID: s0-field-uniform-e
        // Physical Purpose: Establishes a uniform electric field.
        // Initial Condition Parameters: None.
        // Expected Behaviour: Spatially uniform vector potential background along x.
        // Discrepancy: the phrase "flux background" is misleading - the behaviour
        // test asserts flux.mag2() == 0.0 for this scenario. What is seeded is the
        // potential, not a nonzero J.
        // genesis=false (audit-2 2026-04-28): static uniform E shouldn't
        // fill the lattice with manifested particles. Mirrors JS.
        rb.toggles.genesis = false;
        const double eMag = 0.1;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            IW(rb, x, y, z, -eMag, 0, 0);
        }
    }
    else if (name == "s0-field-uniform-b") {
        configure_static_seed_terms(rb);
        // Scenario ID: s0-field-uniform-b
        // Physical Purpose: Establishes a uniform magnetic field.
        // Initial Condition Parameters: None.
        // Expected Behaviour: Rotational flux field pattern representing a uniform magnetic field.
        // Discrepancy: None.
        const double bMag = 0.05;
        const double half = (N - 1) / 2.0;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double rx = x - half, ry = y - half;
            double jx = -bMag * ry / 2, jy = bMag * rx / 2;
            if (std::fabs(jx) > 1e-12 || std::fabs(jy) > 1e-12) IF(rb, x, y, z, jx, jy, 0);
        }
    }
    else if (name == "s0-field-photon-pulse") {
        // Scenario ID: s0-field-photon-pulse
        // Physical Purpose: Tests a broad transverse packet as a photon candidate.
        // Initial Condition Parameters: None.
        // Qualification: closed negative for the current seed. At L=48 over
        // 20 ticks its energy centroid speed is 0.462 (not C_SPEED=0.577) and
        // its width grows by 1.646, failing the preregistered speed/coherence
        // gates. The initial field remains exactly transverse and unmanifested.
        configure_free_wave_terms(rb);
        const int sigma = std::max(3, N / 8);
        inject_transverse_packet_x(rb, mc, mc, mc, sigma, std::max(6.0, N / 4.0),
                                   K_B * 2.0, +1,
                                   2.0 * PI / (4.0 * sigma));
    }
    else if (name == "s0-field-thomson-scattering") {
        // Scenario ID: s0-field-thomson-scattering
        // Physical Purpose: Locked-source linear-superposition null test.
        // The four-arm observatory finds no interaction residual or recoil, so
        // the Thomson-scattering interpretation is closed for this profile.
        configure_locked_coupled_field_terms(rb);

        IP(rb, mc, mc, mc, -1);
        rb.voxels()[rb.lattice().index(mc, mc, mc)].locked = true;

        const int mode_n = 4;
        const double amp = 0.05;
        const double k = 2.0 * PI * static_cast<double>(mode_n) / static_cast<double>(N);
        const double omega = lattice_harmonic_omega(k);
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            const double jy = amp * std::sin(k * x);
            const double wy = amp * ((1.0 - std::cos(omega)) * std::sin(k*x)
                                     - std::sin(omega) * std::cos(k*x));
            if (std::fabs(jy) > 1e-12) IF(rb, x, y, z, 0, jy, 0);
            if (std::fabs(wy) > 1e-12) IW(rb, x, y, z, 0, wy, 0);
        }
    }
    else if (name == "s0-field-thomson-unlocked-recoil") {
        // Scenario ID: s0-field-thomson-unlocked-recoil
        // Physical Purpose: Native flux-gradient recoil probe with one mobile
        // negative-polarity manifested site in a transverse lattice wave.
        // Initial Condition Parameters: None.
        // Expected Behaviour: The native emergent-force path produces a
        // deterministic beam-minus-no-beam displacement.  This is not a
        // Thomson cross-section or QED-scattering claim.
        // Discrepancy: The legacy native force path has no resolved recoil;
        // the response depends on the selected emergent-forces extension.
        configure_emergent_recoil_terms(rb);

        IP(rb, mc, mc, mc, -1);
        rb.voxels()[rb.lattice().index(mc, mc, mc)].locked = false;

        const int mode_n = 4;
        const double amp = 0.05;
        const double k = 2.0 * PI * static_cast<double>(mode_n) / static_cast<double>(N);
        const double omega = 2.0 * std::asin(C_SPEED * std::fabs(std::sin(k * 0.5)));
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            const double jy = amp * std::sin(k * x);
            const double wy = amp * ((1.0 - std::cos(omega)) * std::sin(k*x)
                                     - std::sin(omega) * std::cos(k*x));
            if (std::fabs(jy) > 1e-12) IF(rb, x, y, z, 0, jy, 0);
            if (std::fabs(wy) > 1e-12) IW(rb, x, y, z, 0, wy, 0);
        }
    }
    else if (name == "s0-field-spacetime-forcing-boundary") {
        // Scenario ID: s0-field-spacetime-forcing-boundary
        // Physical Purpose: Native point-response locality-cone probe.
        // This is only the production wave map. The diffusion comparison in
        // the legacy demo is a counterfactual and is not an engine scenario.
        configure_free_wave_terms(rb, false);
        IF(rb, mc, mc, mc, 0.0, 0.0, 1.0);
        IW(rb, mc, mc, mc, 0.0, 0.0, 1.0);
    }
    else if (name == "s0-field-electric-dipole") {
        // Scenario ID: s0-field-electric-dipole
        // Physical Purpose: Imposed softened opposite-source Coulomb-shaped
        // flux profile. This is imported initial data, not emergent EM.
        configure_static_seed_terms(rb);
        const int sep  = std::max(2, N / 8);
        const int half = sep / 2;
        const int px = mc + half, nx = mc - half;
        IP(rb, px, mc, mc, +1);
        IP(rb, nx, mc, mc, -1);
        const double alpha_amp = ALPHA / (4.0 * PI);
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double jx = 0, jy = 0, jz = 0;
            double dx1 = x - px, dy1 = y - mc, dz1 = z - mc;
            double r2_1 = dx1*dx1 + dy1*dy1 + dz1*dz1 + 1.0;
            double f1 = alpha_amp / std::pow(r2_1, 1.5);
            jx += f1 * dx1; jy += f1 * dy1; jz += f1 * dz1;
            double dx2 = x - nx, dy2 = y - mc, dz2 = z - mc;
            double r2_2 = dx2*dx2 + dy2*dy2 + dz2*dz2 + 1.0;
            double f2 = -alpha_amp / std::pow(r2_2, 1.5);
            jx += f2 * dx2; jy += f2 * dy2; jz += f2 * dz2;
            double mag = std::sqrt(jx*jx + jy*jy + jz*jz);
            if (mag > 1e-6) IF(rb, x, y, z, jx, jy, jz);
        }
    }
    else if (name == "s0-field-magnetic-dipole") {
        // Scenario ID: s0-field-magnetic-dipole
        // Physical Purpose: Imposed softened dipole vector-potential ansatz
        // A = mu x r / (r^2 + a^2)^(3/2), with mu parallel to +z.
        // It is not a native derivation of magnetism or a material current loop.
        configure_static_seed_terms(rb);
        const double half = (N - 1) / 2.0;
        const double mu_amp = K_B / (4.0 * PI);
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            const double rx = x - half, ry = y - half, rz = z - half;
            const double denom = std::pow(rx*rx + ry*ry + rz*rz + 1.0, 1.5);
            const double ax = -mu_amp * ry / denom;
            const double ay =  mu_amp * rx / denom;
            if (std::hypot(ax, ay) > 1e-8) IF(rb, x, y, z, ax, ay, 0.0);
        }
    }
    else if (name == "s0-field-vortex-line") {
        // Scenario ID: s0-field-vortex-line
        // Physical Purpose: Imposed azimuthal 1/r vector profile about the
        // z-axis. No electromagnetic, fluid, or quantized-vortex identity.
        configure_static_seed_terms(rb);
        const double gamma = K_B * 4.0;
        const double half = (N - 1) / 2.0;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double rx = x - half, ry = y - half;
            double r = std::sqrt(rx * rx + ry * ry);
            if (r < 1.0) r = 1.0;
            double mag = gamma / (2.0 * PI * r);
            if (mag < 1e-6) continue;
            IF(rb, x, y, z, -mag * ry / r, mag * rx / r, 0);
        }
    }

    else if (name == "s0-field-rf-lattice-wave") {
        // Scenario ID: s0-field-rf-lattice-wave
        // Physical Purpose: Selected n=1 transverse lattice harmonic.
        // Initial Condition Parameters: None.
        // Expected Behaviour: exact n=1 plane-average kick-drift pole.
        // Discrepancy: no mapping to SI radio frequency.
        // Ported from JS seedSpectrumComparator (RF lane): modeN=1, sigmaFrac=0.12,
        // amp=0.034, phase=0, y-component, waveSpeed=C_SPEED.
        configure_free_wave_terms(rb, false);
        {
            const double sigmaFrac = 0.12, amp_w = 0.034, ph0 = 0.0;
            const double sigma  = std::max(1.15, N * sigmaFrac);
            const int    modeN  = std::max(1, std::min(N / 2 - 1, 1));
            const double k      = 2.0 * PI * modeN / N;
            const double omega  = 2.0 * std::asin(C_SPEED * std::abs(std::sin(k / 2.0)));
            const double cut    = sigma * 2.4, cut2 = cut * cut;
            const int zlo = std::max(0,   (int)std::floor(mc - cut));
            const int zhi = std::min(N-1, (int)std::ceil (mc + cut));
            const int ylo = std::max(0,   (int)std::floor(mc - cut));
            const int yhi = std::min(N-1, (int)std::ceil (mc + cut));
            for (int z = zlo; z <= zhi; z++)
            for (int y = ylo; y <= yhi; y++)
            for (int x = 0;   x < N;    x++) {
                const double dy = y - mc, dz = z - mc;
                const double r2 = dy*dy + dz*dz;
                if (r2 > cut2) continue;
                const double g  = std::exp(-r2 / (2.0 * sigma * sigma));
                if (g < 1e-4) continue;
                const double ph = k * x + ph0;
                const double j  = amp_w * g * std::sin(ph);
                const double w  = amp_w * g * ((1.0 - std::cos(omega)) * std::sin(ph)
                                                - std::sin(omega) * std::cos(ph));
                if (std::fabs(j) > 1e-12) IF(rb, x, y, z, 0, j, 0);
                if (std::fabs(w) > 1e-12) IW(rb, x, y, z, 0, w, 0);
            }
        }
    }
    else if (name == "s0-field-light-lattice-wave") {
        // Scenario ID: s0-field-light-lattice-wave
        // Physical Purpose: Selected n=6 transverse lattice harmonic.
        // Initial Condition Parameters: None.
        // Expected Behaviour: exact n=6 plane-average kick-drift pole.
        // Discrepancy: no mapping to SI light, color, or photon identity.
        // Ported from JS seedSpectrumComparator (light lane): modeN=6, sigmaFrac=0.10,
        // amp=0.032, phase=PI*0.15, y-component, waveSpeed=C_SPEED.
        configure_free_wave_terms(rb, false);
        {
            const double sigmaFrac = 0.10, amp_w = 0.032, ph0 = PI * 0.15;
            const double sigma  = std::max(1.15, N * sigmaFrac);
            const int    modeN  = std::max(1, std::min(N / 2 - 1, 6));
            const double k      = 2.0 * PI * modeN / N;
            const double omega  = 2.0 * std::asin(C_SPEED * std::abs(std::sin(k / 2.0)));
            const double cut    = sigma * 2.4, cut2 = cut * cut;
            const int zlo = std::max(0,   (int)std::floor(mc - cut));
            const int zhi = std::min(N-1, (int)std::ceil (mc + cut));
            const int ylo = std::max(0,   (int)std::floor(mc - cut));
            const int yhi = std::min(N-1, (int)std::ceil (mc + cut));
            for (int z = zlo; z <= zhi; z++)
            for (int y = ylo; y <= yhi; y++)
            for (int x = 0;   x < N;    x++) {
                const double dy = y - mc, dz = z - mc;
                const double r2 = dy*dy + dz*dz;
                if (r2 > cut2) continue;
                const double g  = std::exp(-r2 / (2.0 * sigma * sigma));
                if (g < 1e-4) continue;
                const double ph = k * x + ph0;
                const double j  = amp_w * g * std::sin(ph);
                const double w  = amp_w * g * ((1.0 - std::cos(omega)) * std::sin(ph)
                                                - std::sin(omega) * std::cos(ph));
                if (std::fabs(j) > 1e-12) IF(rb, x, y, z, 0, j, 0);
                if (std::fabs(w) > 1e-12) IW(rb, x, y, z, 0, w, 0);
            }
        }
    }
    else if (name == "s0-field-sound-lattice-wave") {
        // Scenario ID: s0-field-sound-lattice-wave
        // Physical Purpose: Closed-negative test of a selected c/8 longitudinal seed.
        // Initial Condition Parameters: None.
        // Expected Behaviour: native C_SPEED pole despite the slow seed momentum.
        // Discrepancy: no acoustic medium exists in the frozen vector-wave sector.
        // Ported from JS seedSpectrumComparator (sound lane): modeN=4, sigmaFrac=0.11,
        // amp=0.030, phase=PI*0.10, x-component (longitudinal), waveSpeed=C_SPEED/8.
        configure_free_wave_terms(rb, false);
        {
            // SOUND_PROXY_SPEED: c/8 is a pedagogical PROXY, not a real acoustic
            // eigenmode. FTD has no sound — the single flux sector re-propagates
            // this wave at c = 1/sqrt(3) (declared [BOUNDARY], FTD-0298/0299); the
            // slow appearance is an initial-condition/visual artifact only.
            const double SOUND_PROXY_SPEED = C_SPEED / 8.0;
            const double sigmaFrac = 0.11, amp_w = 0.030, ph0 = PI * 0.10;
            const double sigma  = std::max(1.15, N * sigmaFrac);
            const int    modeN  = std::max(1, std::min(N / 2 - 1, 4));
            const double k      = 2.0 * PI * modeN / N;
            const double seedOmega = 2.0 * SOUND_PROXY_SPEED * std::abs(std::sin(k / 2.0));
            const double cut    = sigma * 2.4, cut2 = cut * cut;
            const int zlo = std::max(0,   (int)std::floor(mc - cut));
            const int zhi = std::min(N-1, (int)std::ceil (mc + cut));
            const int ylo = std::max(0,   (int)std::floor(mc - cut));
            const int yhi = std::min(N-1, (int)std::ceil (mc + cut));
            for (int z = zlo; z <= zhi; z++)
            for (int y = ylo; y <= yhi; y++)
            for (int x = 0;   x < N;    x++) {
                const double dy = y - mc, dz = z - mc;
                const double r2 = dy*dy + dz*dz;
                if (r2 > cut2) continue;
                const double g  = std::exp(-r2 / (2.0 * sigma * sigma));
                if (g < 1e-4) continue;
                const double ph = k * x + ph0;
                const double j  = amp_w * g * std::sin(ph);
                const double w  = -seedOmega * amp_w * g * std::cos(ph);
                if (std::fabs(j) > 1e-12) IF(rb, x, y, z, j, 0, 0);
                if (std::fabs(w) > 1e-12) IW(rb, x, y, z, w, 0, 0);
            }
        }
    }
    else if (name == "s0-field-sound-collision") {
        // Scenario ID: s0-field-sound-collision
        // Physical Purpose: Collision setup for two longitudinal proxy packets.
        // Initial Condition Parameters: None.
        // Expected Behaviour: substantial overlap with exact linear
        // superposition and no collision residual.
        // Discrepancy: no acoustic medium, physical sound identity, or
        // nonlinear collision interaction.
        // Ported from JS seedSpectrumComparator (sound-collision: 2 lanes).
        // Left pulse: pulseCenterOffsetFrac=-0.25, right-going (speedMult=+1).
        // Right pulse: pulseCenterOffsetFrac=+0.25, left-going (speedMult=-1).
        configure_free_wave_terms(rb, false);
        {
            // SOUND_PROXY_SPEED: c/8 is a pedagogical PROXY, not a real acoustic
            // eigenmode. FTD has no sound — the single flux sector re-propagates
            // this wave at c = 1/sqrt(3) (declared [BOUNDARY], FTD-0298/0299); the
            // slow appearance is an initial-condition/visual artifact only.
            const double SOUND_PROXY_SPEED = C_SPEED / 8.0;
            const double pulseFrac = 0.15, sigmaFrac = 0.11, amp_w = 0.030;
            const double sigma      = std::max(1.15, N * sigmaFrac);
            const double pulseSigma = std::max(1.5, N * pulseFrac * 0.5);
            const int    modeN  = std::max(1, std::min(N / 2 - 1, 4));
            const double k      = 2.0 * PI * modeN / N;
            const double seedOmega = 2.0 * SOUND_PROXY_SPEED * std::abs(std::sin(k / 2.0));
            const double cut    = sigma * 2.4, cut2 = cut * cut;
            struct Lane { double offsetFrac; double speedMult; };
            const Lane lanes[2] = {{-0.25, +1.0}, {+0.25, -1.0}};
            for (const auto& lane : lanes) {
                const double centerX = midF + lane.offsetFrac * N;
                const int zlo = std::max(0,   (int)std::floor(mc - cut));
                const int zhi = std::min(N-1, (int)std::ceil (mc + cut));
                const int ylo = std::max(0,   (int)std::floor(mc - cut));
                const int yhi = std::min(N-1, (int)std::ceil (mc + cut));
                for (int z = zlo; z <= zhi; z++)
                for (int y = ylo; y <= yhi; y++)
                for (int x = 0;   x < N;    x++) {
                    const double dy = y - mc, dz = z - mc;
                    const double r2 = dy*dy + dz*dz;
                    if (r2 > cut2) continue;
                    const double dx  = x - centerX;
                    const double gx  = std::exp(-(dx * dx) / (2.0 * pulseSigma * pulseSigma));
                    const double g   = gx * std::exp(-r2 / (2.0 * sigma * sigma));
                    if (g < 1e-4) continue;
                    const double ph = k * x;
                    const double j  = amp_w * g * std::sin(ph);
                    const double w  = lane.speedMult * (-seedOmega * amp_w * g * std::cos(ph));
                    if (std::fabs(j) > 1e-12) IF(rb, x, y, z, j, 0, 0);
                    if (std::fabs(w) > 1e-12) IW(rb, x, y, z, w, 0, 0);
                }
            }
        }
    }
    return true;
}

}  // namespace ftd
