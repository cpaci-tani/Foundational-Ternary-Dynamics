// ==========================================================================
//  engine/src/scenarios/light.cpp
//
//  Group: light-* (4 scenarios)
//  Canonical seed implementation; the former JS mirror is archived.
//
//  Split out of engine/src/scenarios.cpp (ticket S1).
//
//  Universal scenario seeding (2026-09-16): every meaningful hardcoded
//  construction literal below is now bound through ftd::seed::{real,
//  integer,choice} (declared in ftd/scenario_seed.h). With no
//  ftd::seed::Context installed, record() returns each default_value
//  unchanged and never touches the RNG, so the legacy no-context dispatch
//  path is byte-identical to the pre-seeding source. Grouping convention:
//    source.*       — absolute placement coordinates of a packet center
//    geometry.*      — widths, separations, wavenumbers describing shape
//    packet.*        — shared field amplitude of a single-source scenario
//    harmonicN.*      — per-harmonic mode number / transverse polarization
//                       for the fixed 3-harmonic light-rainbow superposition
//    constituentN.*   — per-packet amplitude/polarization for a scenario
//                       with more than one independently seeded packet
//  Fixed-law constants (C_WAVE) and structural counts/signs that define a
//  scenario's declared identity (light-rainbow's fixed count of 3
//  harmonics; light-dipole's antisymmetric +/-1 direction pair; both
//  light-two-slit sources sharing +1 direction so they stay mutually
//  coherent) are left hardcoded per FTD-0371/SCOPE_CONSUMPTION_PROGRAM
//  discipline: exposing an amplitude, width, or mode number is not a
//  physics claim, but redefining a scenario's named structure would be.
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
        const double sharedAmp = seed::real("packet.amplitude", amp, 0.0, amp * 40.0,
            "Harmonic amplitude", "Shared peak amplitude of all three seeded sinusoidal harmonics.");
        const int modeMax = std::max(N - 1, 8);
        // Propagation is along x, so every polarization must be y or z.
        const seed::Options polChoices = {{1.0, "y"}, {2.0, "z"}};
        const int n0 = seed::integer("harmonic0.modeNumber", 1, 1, modeMax,
            "Harmonic 0 mode number", "Integer wavenumber index n (k = 2*pi*n/N) of the first seeded harmonic.");
        const int p0 = seed::choice("harmonic0.polarization", 1, polChoices,
            "Harmonic 0 polarization", "Transverse axis (y or z) carrying the first harmonic's field.");
        const int n1 = seed::integer("harmonic1.modeNumber", 3, 1, modeMax,
            "Harmonic 1 mode number", "Integer wavenumber index n (k = 2*pi*n/N) of the second seeded harmonic.");
        const int p1 = seed::choice("harmonic1.polarization", 2, polChoices,
            "Harmonic 1 polarization", "Transverse axis (y or z) carrying the second harmonic's field.");
        const int n2 = seed::integer("harmonic2.modeNumber", 6, 1, modeMax,
            "Harmonic 2 mode number", "Integer wavenumber index n (k = 2*pi*n/N) of the third seeded harmonic.");
        const int p2 = seed::choice("harmonic2.polarization", 1, polChoices,
            "Harmonic 2 polarization", "Transverse axis (y or z) carrying the third harmonic's field.");
        const double amul0 = seed::real("harmonic0.amplitudeMultiplier", 1.0, 0.0, 4.0,
            "Harmonic 0 amplitude multiplier", "Multiplier on the shared peak amplitude applied to the first harmonic only.");
        const double amul1 = seed::real("harmonic1.amplitudeMultiplier", 1.0, 0.0, 4.0,
            "Harmonic 1 amplitude multiplier", "Multiplier on the shared peak amplitude applied to the second harmonic only.");
        const double amul2 = seed::real("harmonic2.amplitudeMultiplier", 1.0, 0.0, 4.0,
            "Harmonic 2 amplitude multiplier", "Multiplier on the shared peak amplitude applied to the third harmonic only.");
        struct WA { int n; int pol; double amul; };
        const WA waves[3] = { {n0, p0, amul0}, {n1, p1, amul1}, {n2, p2, amul2} };
        for (int w = 0; w < 3; w++) {
            double k = 2.0 * PI * waves[w].n / N;
            int pol = waves[w].pol;
            double harmAmp = sharedAmp * waves[w].amul;
            for (int x = 0; x < N; x++) for (int y = 0; y < N; y++) for (int z = 0; z < N; z++) {
                double J_val  = harmAmp * std::sin(k * x);
                // phase_read kicks W before phase_write drifts J. Match the
                // same kick-drift time phase used by the packet helper:
                // W = -c D_x J - c^2 Lap(J)/2.  For a uniform-yz harmonic,
                // D_x sin(kx)=sin(k)cos(kx) and
                // Lap_18 sin(kx)=-4 sin^2(k/2)sin(kx).
                double wv_val = -C_WAVE * std::sin(k) * harmAmp * std::cos(k * x)
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
        const double offset = seed::real("geometry.offset", 2.0, 0.0, std::max(N * 0.5, 64.0),
            "Lobe half-separation", "Half the distance (lattice sites) between the two opposite-polarity packet centers, measured along x from the lattice center.");
        const double x0 = seed::real("source.x", mid, 0.0, N - 1.0,
            "Pair center x", "Lattice x-coordinate about which the two opposite-polarity packet centers are symmetrically offset.");
        const double y0 = seed::real("source.y", mid, 0.0, N - 1.0,
            "Packet start y", "Lattice y-coordinate shared by both packet centers.");
        const double z0 = seed::real("source.z", mid, 0.0, N - 1.0,
            "Packet start z", "Lattice z-coordinate shared by both packet centers.");
        const double sx = seed::real("packet.sigmaX", 2.5, 1.0, std::max(N * 0.5, 64.0),
            "Packet width", "Gaussian width (lattice sites) of each packet along the direction of travel.");
        const double st = seed::real("packet.sigmaT", 3.0, 1.0, std::max(N * 0.5, 64.0),
            "Packet transverse width", "Gaussian width (lattice sites) of each packet in the transverse plane.");
        const double pAmp = seed::real("packet.amplitude", 0.5, 0.0, 0.5 * 40.0,
            "Packet amplitude", "Peak amplitude of the seeded curl-potential building each divergence-free transverse packet.");
        const int seed_wave_0_direction = seed::choice("wave0.direction", -1, {{-1,"-x"},{1,"+x"}}, "Wave 1 direction", "Changes the travel direction of wave ingredient 1 along x.");
        const double seed_wave_0_carrierK = seed::real("wave0.carrierK", 0.0, 0.0, PI, "Wave 1 carrier wavenumber", "Sets wave 1 carrier wavenumber of wave ingredient 1; increasing it changes the prepared profile before the first tick.", "radians per cell");
        const double seed_wave_0_phase = seed::real("wave0.phase", 0.0, -PI, PI, "Wave 1 carrier phase", "Sets wave 1 carrier phase of wave ingredient 1; increasing it changes the prepared profile before the first tick.", "radians");
        inject_transverse_packet_x(rb, x0 - offset, y0, z0, sx, st, pAmp, seed_wave_0_direction, seed_wave_0_carrierK, seed_wave_0_phase);
        const int seed_wave_1_direction = seed::choice("wave1.direction", +1, {{-1,"-x"},{1,"+x"}}, "Wave 2 direction", "Changes the travel direction of wave ingredient 2 along x.");
        const double seed_wave_1_carrierK = seed::real("wave1.carrierK", 0.0, 0.0, PI, "Wave 2 carrier wavenumber", "Sets wave 2 carrier wavenumber of wave ingredient 2; increasing it changes the prepared profile before the first tick.", "radians per cell");
        const double seed_wave_1_phase = seed::real("wave1.phase", 0.0, -PI, PI, "Wave 2 carrier phase", "Sets wave 2 carrier phase of wave ingredient 2; increasing it changes the prepared profile before the first tick.", "radians");
        inject_transverse_packet_x(rb, x0 + offset, y0, z0, sx, st, pAmp, seed_wave_1_direction, seed_wave_1_carrierK, seed_wave_1_phase);
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
        const double sigma_x = seed::real("geometry.sigmaX", 4.0, 1.0, std::max(N * 0.5, 64.0),
            "Source width", "Gaussian width (lattice sites) of each slit source along x.");
        const double sigma_y = seed::real("geometry.sigmaY", 2.0, 1.0, std::max(N * 0.5, 64.0),
            "Source transverse width", "Gaussian width (lattice sites) of each slit source along the transverse (separation) axis.");
        const double sAmp = seed::real("packet.amplitude", 0.3, 0.0, 0.3 * 40.0,
            "Source amplitude", "Shared peak amplitude of both coherent slit sources.");
        const int slit_sep = seed::integer("geometry.slitSeparation", N / 6, 0, std::max(N, 64),
            "Slit half-separation", "Half the distance (lattice sites) between the two slit sources, measured from the lattice center along y.");
        const double slit_x = seed::real("source.x", double(N / 4), 0.0, N - 1.0,
            "Source x position", "Shared lattice x-coordinate of both slit sources.");
        const double slit_y = seed::real("source.y", mid, 0.0, N - 1.0,
            "Slit pair center y", "Lattice y-coordinate about which the two slit sources are symmetrically offset.");
        const double carrier_k = seed::real("packet.carrierK", 2.0 * PI / 8.0, 0.0, PI,
            "Carrier wavenumber", "Spatial wavenumber of the cosine carrier riding inside each source envelope.");
        const int pol = seed::choice("packet.polarization", 2, {{1.0, "y"}, {2.0, "z"}},
            "Source polarization", "Transverse axis (y or z) carrying both sources' field, held equal so the two remain mutually coherent.");
        const int seed_wave_0_direction = seed::choice("wave0.direction", +1, {{-1,"-x"},{1,"+x"}}, "Wave 1 direction", "Changes the travel direction of wave ingredient 1 along x.");
        const double seed_wave_0_phase = seed::real("wave0.phase", 0.0, -PI, PI, "Wave 1 carrier phase", "Sets wave 1 carrier phase of wave ingredient 1; increasing it changes the prepared profile before the first tick.", "radians");
        inject_sheet_packet_x(rb, slit_x, slit_y - slit_sep, sigma_x, sigma_y, sAmp, seed_wave_0_direction, pol, carrier_k, seed_wave_0_phase);
        const int seed_wave_1_direction = seed::choice("wave1.direction", +1, {{-1,"-x"},{1,"+x"}}, "Wave 2 direction", "Changes the travel direction of wave ingredient 2 along x.");
        const double seed_wave_1_phase = seed::real("wave1.phase", 0.0, -PI, PI, "Wave 2 carrier phase", "Sets wave 2 carrier phase of wave ingredient 2; increasing it changes the prepared profile before the first tick.", "radians");
        inject_sheet_packet_x(rb, slit_x, slit_y + slit_sep, sigma_x, sigma_y, sAmp, seed_wave_1_direction, pol, carrier_k, seed_wave_1_phase);
    }
    else if (name == "light-photon-race") {
        // Scenario ID: light-photon-race
        // Physical Purpose: Compares propagation characteristics of photons/wave packets of different amplitudes.
        // Initial Condition Parameters: Two parallel photon wave packets starting at x_start, one with low amplitude (0.05) and one with high amplitude (0.5).
        // Expected Behaviour: Both packets translate at the same limiting speed despite their amplitude ratio.
        // Verification: common-speed classical-wave comparison.
        configure_free_wave();
        const double sigma = seed::real("geometry.sigma", 3.0, 1.0, std::max(N * 0.5, 64.0),
            "Source width", "Gaussian width (lattice sites) shared by both packets.");
        const double x_start = seed::real("source.x", double(N / 4), 0.0, N - 1.0,
            "Packet start x", "Shared lattice x-coordinate of both packet centers.");
        const int track_offset = seed::integer("geometry.trackOffset", N / 6, 0, std::max(N, 64),
            "Track half-separation", "Half the distance (lattice sites) between the two parallel tracks, measured from the lattice center along y.");
        const double track_y = seed::real("source.y", mid, 0.0, N - 1.0,
            "Track pair center y", "Lattice y-coordinate about which the two parallel tracks are symmetrically offset.");
        // Orthogonal polarizations let diagnostics separate the two
        // superposed linear solutions without amplitude leakage.
        const seed::Options polChoices = {{1.0, "y"}, {2.0, "z"}};
        const double amp0 = seed::real("constituent0.amplitude", 0.05, 0.0, 0.5 * 40.0,
            "Packet 0 amplitude", "Peak amplitude of the low-amplitude packet.");
        const int pol0 = seed::choice("constituent0.polarization", 1, polChoices,
            "Packet 0 polarization", "Transverse axis (y or z) carrying the low-amplitude packet's field.");
        const double amp1 = seed::real("constituent1.amplitude", 0.5, 0.0, 0.5 * 40.0,
            "Packet 1 amplitude", "Peak amplitude of the high-amplitude packet.");
        const int pol1 = seed::choice("constituent1.polarization", 2, polChoices,
            "Packet 1 polarization", "Transverse axis (y or z) carrying the high-amplitude packet's field.");
        const int seed_wave_0_direction = seed::choice("wave0.direction", +1, {{-1,"-x"},{1,"+x"}}, "Wave 1 direction", "Changes the travel direction of wave ingredient 1 along x.");
        const double seed_wave_0_carrierK = seed::real("wave0.carrierK", 0.0, 0.0, PI, "Wave 1 carrier wavenumber", "Sets wave 1 carrier wavenumber of wave ingredient 1; increasing it changes the prepared profile before the first tick.", "radians per cell");
        const double seed_wave_0_phase = seed::real("wave0.phase", 0.0, -PI, PI, "Wave 1 carrier phase", "Sets wave 1 carrier phase of wave ingredient 1; increasing it changes the prepared profile before the first tick.", "radians");
        const double seed_wave_0_sigmaTransverse = seed::real("wave0.sigmaTransverse", 2.0, 1.0, std::max(64.0, double(N)), "Wave 1 transverse width", "Sets wave 1 transverse width of wave ingredient 1; increasing it changes the prepared profile before the first tick.", "cells");
        inject_sheet_packet_x(rb, x_start, track_y - track_offset, sigma, seed_wave_0_sigmaTransverse, amp0, seed_wave_0_direction, pol0, seed_wave_0_carrierK, seed_wave_0_phase);
        const int seed_wave_1_direction = seed::choice("wave1.direction", +1, {{-1,"-x"},{1,"+x"}}, "Wave 2 direction", "Changes the travel direction of wave ingredient 2 along x.");
        const double seed_wave_1_carrierK = seed::real("wave1.carrierK", 0.0, 0.0, PI, "Wave 2 carrier wavenumber", "Sets wave 2 carrier wavenumber of wave ingredient 2; increasing it changes the prepared profile before the first tick.", "radians per cell");
        const double seed_wave_1_phase = seed::real("wave1.phase", 0.0, -PI, PI, "Wave 2 carrier phase", "Sets wave 2 carrier phase of wave ingredient 2; increasing it changes the prepared profile before the first tick.", "radians");
        const double seed_wave_1_sigmaTransverse = seed::real("wave1.sigmaTransverse", 2.0, 1.0, std::max(64.0, double(N)), "Wave 2 transverse width", "Sets wave 2 transverse width of wave ingredient 2; increasing it changes the prepared profile before the first tick.", "cells");
        inject_sheet_packet_x(rb, x_start, track_y + track_offset, sigma, seed_wave_1_sigmaTransverse, amp1, seed_wave_1_direction, pol1, seed_wave_1_carrierK, seed_wave_1_phase);
    }
    else {
        return false;
    }
    return true;
}

}  // namespace ftd
