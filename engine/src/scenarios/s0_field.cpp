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
        // Physical Purpose: Establishes a planar electromagnetic wave.
        // Initial Condition Parameters: None.
        // Expected Behaviour: Plane wave propagating along the x-axis with transverse oscillations.
        // Discrepancy: None.
        const double wl  = N / 4.0;
        const double amp = K_B * 2.0;
        const double k   = 2.0 * PI / wl;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double phase = k * x;
            double jz = amp * std::sin(phase);
            double wz = amp * std::cos(phase) * C_SPEED;
            if (std::fabs(jz) > 1e-12 || std::fabs(wz) > 1e-12) {
                IF(rb, x, y, z, 0, 0, jz);
                IW(rb, x, y, z, wz, 0, 0);
            }
        }
    }
    else if (name == "s0-field-standing-wave") {
        // Scenario ID: s0-field-standing-wave
        // Physical Purpose: Establishes a planar electromagnetic standing wave.
        // Initial Condition Parameters: None.
        // Expected Behaviour: Stationary sinusoidal node-antinode pattern in the transverse flux.
        // Discrepancy: None.
        const double wl  = N / 4.0;
        const double amp = K_B * 2.0;
        const double k   = 2.0 * PI / wl;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double jz = amp * std::sin(k * x);
            if (std::fabs(jz) > 1e-12) IF(rb, x, y, z, 0, 0, jz);
        }
    }
    else if (name == "s0-field-uniform-e") {
        // Scenario ID: s0-field-uniform-e
        // Physical Purpose: Establishes a uniform electric field.
        // Initial Condition Parameters: None.
        // Expected Behaviour: Spatially uniform vector potential flux background pointing along x-axis.
        // Discrepancy: None.
        // genesis=false (audit-2 2026-04-28): static uniform E shouldn't
        // fill the lattice with manifested particles. Mirrors JS.
        rb.toggles.genesis = false;
        const double eMag = 0.1;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            IW(rb, x, y, z, -eMag, 0, 0);
        }
    }
    else if (name == "s0-field-uniform-b") {
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
        // Physical Purpose: Seeds a propagating photon pulse packet.
        // Initial Condition Parameters: None.
        // Expected Behaviour: Coherent localized wave packet propagating at light speed along the x-axis.
        // Discrepancy: None.
        const int sigma = std::max(3, N / 8);
        const double amp = K_B * 2.0;
        const double lambdaEff = 4.0 * sigma;
        const double k = 2.0 * PI / lambdaEff;
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
            double wz = amp * g * std::cos(phase) * C_SPEED;
            IF(rb, x, y, z, 0, 0, jz);
            IW(rb, x, y, z, wz, 0, 0);
        }
    }
    else if (name == "s0-field-thomson-scattering") {
        // Scenario ID: s0-field-thomson-scattering
        // Physical Purpose: Simulates classical electromagnetic Thomson scattering off a locked charge.
        // Initial Condition Parameters: None.
        // Expected Behaviour: Plane wave scattering off a stationary central electron.
        // Discrepancy: None.
        // Fixed electron-like scattering observatory:
        // one locked negative charge at the center plus a y-polarized plane
        // wave propagating along +x. This is a visualization/instrument setup,
        // not a derivation of alpha.
        rb.toggles.wave_propagation = true;
        rb.toggles.coupling = true;
        rb.toggles.damping = false;
        rb.toggles.genesis = false;
        rb.toggles.gauss_projection = false;
        rb.toggles.forces = false;
        rb.toggles.movement = false;
        rb.toggles.poisson_coulomb = false;

        IP(rb, mc, mc, mc, -1);
        rb.voxels()[rb.lattice().index(mc, mc, mc)].locked = true;

        const int mode_n = 4;
        const double amp = 0.05;
        const double k = 2.0 * PI * static_cast<double>(mode_n) / static_cast<double>(N);
        const double omega = 2.0 * C_SPEED * std::fabs(std::sin(k * 0.5));
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            const double jy = amp * std::sin(k * x);
            const double wy = -omega * amp * std::cos(k * x);
            if (std::fabs(jy) > 1e-12) IF(rb, x, y, z, 0, jy, 0);
            if (std::fabs(wy) > 1e-12) IW(rb, x, y, z, 0, wy, 0);
        }
    }
    else if (name == "s0-field-thomson-unlocked-recoil") {
        // Scenario ID: s0-field-thomson-unlocked-recoil
        // Physical Purpose: Simulates Thomson scattering with an unlocked recoiling charge.
        // Initial Condition Parameters: None.
        // Expected Behaviour: Plane wave scattering off a mobile central electron which recoils under emergent forces.
        // Discrepancy: None.
        // FTD-0288 visual companion: one unlocked negative charge plus the
        // same plane wave, with the native emergent flux-gradient force path.
        rb.toggles.wave_propagation = true;
        rb.toggles.coupling = true;
        rb.toggles.damping = false;
        rb.toggles.genesis = false;
        rb.toggles.gauss_projection = false;
        rb.toggles.forces = true;
        rb.toggles.movement = true;
        rb.toggles.poisson_coulomb = false;
        rb.toggles.lorentz_force = false;
        rb.toggles.emergent_forces = true;

        IP(rb, mc, mc, mc, -1);
        rb.voxels()[rb.lattice().index(mc, mc, mc)].locked = false;

        const int mode_n = 4;
        const double amp = 0.05;
        const double k = 2.0 * PI * static_cast<double>(mode_n) / static_cast<double>(N);
        const double omega = 2.0 * C_SPEED * std::fabs(std::sin(k * 0.5));
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            const double jy = amp * std::sin(k * x);
            const double wy = -omega * amp * std::cos(k * x);
            if (std::fabs(jy) > 1e-12) IF(rb, x, y, z, 0, jy, 0);
            if (std::fabs(wy) > 1e-12) IW(rb, x, y, z, 0, wy, 0);
        }
    }
    else if (name == "s0-field-spacetime-forcing-boundary") {
        // Scenario ID: s0-field-spacetime-forcing-boundary
        // Physical Purpose: Models spacetime forcing boundary conditions (FTD-0253).
        // Initial Condition Parameters: None.
        // Expected Behaviour: Forcing of localized flux at the center.
        // Discrepancy: None.
        // FTD-0253 visible seed: the wave half of
        // test_spacetime_forcing_demo.cpp. The diffusion half is a
        // counterfactual shown in engine/web/demos/spacetime-forcing-boundary.html.
        IF(rb, mc, mc, mc, 0.0, 0.0, 1.0);
        IW(rb, mc, mc, mc, 0.0, 0.0, 1.0);
    }
    else if (name == "s0-field-electric-dipole") {
        // Scenario ID: s0-field-electric-dipole
        // Physical Purpose: Establishes a static electric dipole field.
        // Initial Condition Parameters: None.
        // Expected Behaviour: Dipole field lines connecting a positive and negative charge.
        // Discrepancy: None.
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
        // Scenario ID: s0-field-magnetic-dipole
        // Physical Purpose: Establishes a static magnetic dipole field.
        // Initial Condition Parameters: None.
        // Expected Behaviour: Circulating loop flux representing a magnetic dipole.
        // Discrepancy: None.
        const int loopR = std::max(3, N / 8);
        const double amp = K_B;
        const int nAngles = std::max(36, loopR * 8);
        for (int i = 0; i < nAngles; i++) {
            double theta = 2.0 * PI * i / nAngles;
            int lx = RND(mc + loopR * std::cos(theta));
            int ly = RND(mc + loopR * std::sin(theta));
            double tx = -std::sin(theta) * amp;
            double ty =  std::cos(theta) * amp;
            for (int z = 0; z < N; z++) IF(rb, lx, ly, z, tx, ty, 0);
        }
    }
    else if (name == "s0-field-vortex-line") {
        // Scenario ID: s0-field-vortex-line
        // Physical Purpose: Models an electromagnetic or fluid vortex line.
        // Initial Condition Parameters: None.
        // Expected Behaviour: Tangential rotational flux line about the z-axis.
        // Discrepancy: None.
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
        // Physical Purpose: Long-wavelength RF wave demonstration.
        // Initial Condition Parameters: None.
        // Expected Behaviour: Large scale wave propagating across lattice.
        // Discrepancy: None.
        // Ported from JS seedSpectrumComparator (RF lane): modeN=1, sigmaFrac=0.12,
        // amp=0.034, phase=0, y-component, waveSpeed=C_SPEED.
        rb.toggles.wave_propagation  = true;
        rb.toggles.coupling          = false;
        rb.toggles.damping           = false;
        rb.toggles.selective_damping = false;
        rb.toggles.genesis           = false;
        rb.toggles.gauss_projection  = false;
        rb.toggles.forces            = false;
        rb.toggles.movement          = false;
        rb.toggles.poisson_coulomb   = false;
        rb.toggles.lorentz_force     = false;
        {
            const double sigmaFrac = 0.12, amp_w = 0.034, ph0 = 0.0;
            const double sigma  = std::max(1.15, N * sigmaFrac);
            const int    modeN  = std::max(1, std::min(N / 2 - 1, 1));
            const double k      = 2.0 * PI * modeN / N;
            const double omega  = 2.0 * C_SPEED * std::abs(std::sin(k / 2.0));
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
                const double w  = -omega * amp_w * g * std::cos(ph);
                if (std::fabs(j) > 1e-12) IF(rb, x, y, z, 0, j, 0);
                if (std::fabs(w) > 1e-12) IW(rb, x, y, z, 0, w, 0);
            }
        }
    }
    else if (name == "s0-field-light-lattice-wave") {
        // Scenario ID: s0-field-light-lattice-wave
        // Physical Purpose: Optical wavelength lattice wave demonstration.
        // Initial Condition Parameters: None.
        // Expected Behaviour: Optical frequency waves.
        // Discrepancy: None.
        // Ported from JS seedSpectrumComparator (light lane): modeN=6, sigmaFrac=0.10,
        // amp=0.032, phase=PI*0.15, y-component, waveSpeed=C_SPEED.
        rb.toggles.wave_propagation  = true;
        rb.toggles.coupling          = false;
        rb.toggles.damping           = false;
        rb.toggles.selective_damping = false;
        rb.toggles.genesis           = false;
        rb.toggles.gauss_projection  = false;
        rb.toggles.forces            = false;
        rb.toggles.movement          = false;
        rb.toggles.poisson_coulomb   = false;
        rb.toggles.lorentz_force     = false;
        {
            const double sigmaFrac = 0.10, amp_w = 0.032, ph0 = PI * 0.15;
            const double sigma  = std::max(1.15, N * sigmaFrac);
            const int    modeN  = std::max(1, std::min(N / 2 - 1, 6));
            const double k      = 2.0 * PI * modeN / N;
            const double omega  = 2.0 * C_SPEED * std::abs(std::sin(k / 2.0));
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
                const double w  = -omega * amp_w * g * std::cos(ph);
                if (std::fabs(j) > 1e-12) IF(rb, x, y, z, 0, j, 0);
                if (std::fabs(w) > 1e-12) IW(rb, x, y, z, 0, w, 0);
            }
        }
    }
    else if (name == "s0-field-sound-lattice-wave") {
        // Scenario ID: s0-field-sound-lattice-wave
        // Physical Purpose: Acoustic wavelength wave demonstration.
        // Initial Condition Parameters: None.
        // Expected Behaviour: Very low frequency wave behavior.
        // Discrepancy: None.
        // Ported from JS seedSpectrumComparator (sound lane): modeN=4, sigmaFrac=0.11,
        // amp=0.030, phase=PI*0.10, x-component (longitudinal), waveSpeed=C_SPEED/8.
        rb.toggles.wave_propagation  = true;
        rb.toggles.coupling          = false;
        rb.toggles.damping           = false;
        rb.toggles.selective_damping = false;
        rb.toggles.genesis           = false;
        rb.toggles.gauss_projection  = false;
        rb.toggles.forces            = false;
        rb.toggles.movement          = false;
        rb.toggles.poisson_coulomb   = false;
        rb.toggles.lorentz_force     = false;
        {
            const double SOUND_SPEED = C_SPEED / 8.0;
            const double sigmaFrac = 0.11, amp_w = 0.030, ph0 = PI * 0.10;
            const double sigma  = std::max(1.15, N * sigmaFrac);
            const int    modeN  = std::max(1, std::min(N / 2 - 1, 4));
            const double k      = 2.0 * PI * modeN / N;
            const double omega  = 2.0 * SOUND_SPEED * std::abs(std::sin(k / 2.0));
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
                const double w  = -omega * amp_w * g * std::cos(ph);
                if (std::fabs(j) > 1e-12) IF(rb, x, y, z, j, 0, 0);
                if (std::fabs(w) > 1e-12) IW(rb, x, y, z, w, 0, 0);
            }
        }
    }
    else if (name == "s0-field-sound-collision") {
        // Scenario ID: s0-field-sound-collision
        // Physical Purpose: Collision of two sound wave pulses.
        // Initial Condition Parameters: None.
        // Expected Behaviour: Interference and beat frequencies of sound waves.
        // Discrepancy: None.
        // Ported from JS seedSpectrumComparator (sound-collision: 2 lanes).
        // Left pulse: pulseCenterOffsetFrac=-0.25, right-going (speedMult=+1).
        // Right pulse: pulseCenterOffsetFrac=+0.25, left-going (speedMult=-1).
        rb.toggles.wave_propagation  = true;
        rb.toggles.coupling          = false;
        rb.toggles.damping           = false;
        rb.toggles.selective_damping = false;
        rb.toggles.genesis           = false;
        rb.toggles.gauss_projection  = false;
        rb.toggles.forces            = false;
        rb.toggles.movement          = false;
        rb.toggles.poisson_coulomb   = false;
        rb.toggles.lorentz_force     = false;
        {
            const double SOUND_SPEED = C_SPEED / 8.0;
            const double pulseFrac = 0.15, sigmaFrac = 0.11, amp_w = 0.030;
            const double sigma      = std::max(1.15, N * sigmaFrac);
            const double pulseSigma = std::max(1.5, N * pulseFrac * 0.5);
            const int    modeN  = std::max(1, std::min(N / 2 - 1, 4));
            const double k      = 2.0 * PI * modeN / N;
            const double omega  = 2.0 * SOUND_SPEED * std::abs(std::sin(k / 2.0));
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
                    const double w  = lane.speedMult * (-omega * amp_w * g * std::cos(ph));
                    if (std::fabs(j) > 1e-12) IF(rb, x, y, z, j, 0, 0);
                    if (std::fabs(w) > 1e-12) IW(rb, x, y, z, w, 0, 0);
                }
            }
        }
    }
    return true;
}

}  // namespace ftd
