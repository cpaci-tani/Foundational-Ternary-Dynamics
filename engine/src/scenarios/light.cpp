// ==========================================================================
//  engine/src/scenarios/light.cpp
//
//  Group: light-* (4 scenarios)
//  Canonical seed implementation; the former JS mirror is archived.
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

bool setup_light_scenario(RenderBridge& rb, const std::string& name) {
    if (name.rfind("light-", 0) != 0) return false;
    const int    N     = rb.lattice().size();
    const int    mid   = N / 2;
    const double amp   = 0.15;
    const auto configure_free_wave = [&]() {
        configure_free_wave_terms(rb);
    };

    if (name == "light-rainbow") {
        // Scenario ID: light-rainbow
        // Physical Purpose: Compares native lattice dispersion for three
        // transverse harmonics with different wavelengths.
        // Initial Condition Parameters: Superimposed sinusoidal flux waves of three different frequencies (n=1, 3, 6) across the lattice.
        // Expected Behaviour: The higher-k harmonics accumulate a larger
        // lattice-dispersion phase lag.  "Colour" is only a display label.
        configure_free_wave();
        struct W { int n; int pol; };
        // Propagation is along x, so every polarization must be y or z.
        const W waves[3] = { {1,1}, {3,2}, {6,1} };
        for (int w = 0; w < 3; w++) {
            double k = 2.0 * PI * waves[w].n / N;
            int pol = waves[w].pol;
            for (int x = 0; x < N; x++) for (int y = 0; y < N; y++) for (int z = 0; z < N; z++) {
                double J_val  = amp * std::sin(k * x);
                // phase_read kicks W before phase_write drifts J. Match the
                // same kick-drift time phase used by the packet helper:
                // W = -c D_x J - c^2 Lap(J)/2.  For a uniform-yz harmonic,
                // D_x sin(kx)=sin(k)cos(kx) and
                // Lap_18 sin(kx)=-4 sin^2(k/2)sin(kx).
                double wv_val = -C_WAVE * std::sin(k) * amp * std::cos(k * x)
                              + 2.0 * C_WAVE * C_WAVE
                                * std::sin(k / 2.0) * std::sin(k / 2.0) * J_val;
                double fv[3] = {0,0,0}, wvv[3] = {0,0,0};
                fv[pol] = J_val;
                wvv[pol] = wv_val;
                IF(rb, x, y, z, fv[0], fv[1], fv[2]);
                IW(rb, x, y, z, wvv[0], wvv[1], wvv[2]);
            }
        }
    }
    else if (name == "light-dipole") {
        // Scenario ID: light-dipole
        // Physical Purpose: Visualizes two oppositely directed transverse radiation lobes.
        // Initial Condition Parameters: Divergence-free Gaussian packets, amplitude 0.5.
        // Expected Behaviour: The two lobes separate along +/-x under the native wave map.
        // Verification: dipole-like radiation proxy; not a full Maxwell dipole solution.
        configure_free_wave();
        inject_transverse_packet_x(rb, mid - 2.0, mid, mid, 2.5, 3.0, 0.5, -1);
        inject_transverse_packet_x(rb, mid + 2.0, mid, mid, 2.5, 3.0, 0.5, +1);
    }
    else if (name == "light-two-slit") {
        // Scenario ID: light-two-slit
        // Physical Purpose: Tests interference from two coherent classical
        // transverse sources. There is no material barrier or slit boundary.
        // Initial Condition: Two equal Gaussian sheet packets separated in y.
        // Qualification status: pointwise superposition and both cross-term
        // signs are present, but the fixed L=48 screen gate remains failed
        // because constructive contrast is below the preregistered 5% floor.
        configure_free_wave();
        const int sigma_x = 4;
        const int sigma_y = 2;
        const double sAmp = 0.3;
        const int slit_sep = N / 6;
        const int slit_x   = N / 4;
        const double carrier_k = 2.0 * PI / 8.0;
        inject_sheet_packet_x(rb, slit_x, mid - slit_sep, sigma_x, sigma_y,
                              sAmp, +1, 2, carrier_k);
        inject_sheet_packet_x(rb, slit_x, mid + slit_sep, sigma_x, sigma_y,
                              sAmp, +1, 2, carrier_k);
    }
    else if (name == "light-photon-race") {
        // Scenario ID: light-photon-race
        // Physical Purpose: Compares propagation characteristics of photons/wave packets of different amplitudes.
        // Initial Condition Parameters: Two parallel photon wave packets starting at x_start, one with low amplitude (0.05) and one with high amplitude (0.5).
        // Expected Behaviour: Both packets translate at the same limiting speed despite their amplitude ratio.
        // Verification: common-speed classical-wave comparison.
        configure_free_wave();
        const int sigma = 3;
        const int x_start = N / 4;
        const double pAmps[2] = { 0.05, 0.5 };
        const int transverse_off[2] = { mid - N / 6, mid + N / 6 };
        // Orthogonal polarizations let diagnostics separate the two
        // superposed linear solutions without amplitude leakage.
        inject_sheet_packet_x(rb, x_start, transverse_off[0], sigma, 2.0,
                              pAmps[0], +1, 1);
        inject_sheet_packet_x(rb, x_start, transverse_off[1], sigma, 2.0,
                              pAmps[1], +1, 2);
    }
    else {
        return false;
    }
    return true;
}

}  // namespace ftd
