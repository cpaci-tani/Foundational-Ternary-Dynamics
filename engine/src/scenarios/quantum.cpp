// ==========================================================================
//  engine/src/scenarios/quantum.cpp
//
//  Group: quantum-* (8 scenarios)
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
//    source.*   — absolute placement coordinates of a packet/marker/wall
//    geometry.* — widths, separations, radii, sigmas, amplitude cutoffs
//    packet.*   — field amplitudes and phase of a seeded flux/wave vector
//    field.*    — an imposed external field strength (e.g. solenoid flux)
//    harmonic.* — mode number of a single seeded standing/traveling eigenmode
//  Fixed-law constants and structural counts/orderings that define a
//  scenario's declared identity (quantum-well's fixed n=1..8 standing-wave
//  basis; quantum-eraser's diagonal checkerboard parity; quantum-entangle's
//  fixed pair location at the lattice center; the universal 3-sigma Gaussian
//  support radius) are left hardcoded per FTD-0371/SCOPE_CONSUMPTION_PROGRAM
//  discipline: exposing an amplitude, width, or position is not a physics
//  claim, but redefining a scenario's named structure would be.
// ==========================================================================

#include "ftd/scenarios.h"
#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/voxel.h"

#include "_helpers.h"

#include <cmath>

namespace ftd {

using detail::urand;

bool setup_quantum_scenario(RenderBridge& rb, const std::string& name) {
    if (name.rfind("quantum-", 0) != 0) return false;
    const int N = rb.lattice().size();
    const int mid = N / 2;
    const auto configure_free_wave = [&]() {
        configure_free_wave_terms(rb);
    };

    if (name == "quantum-born-rule") {
        // Scenario ID: quantum-born-rule
        // Physical Purpose: Measures the native one-tick genesis response to a fixed graded envelope.
        // Initial Condition Parameters: None.
        // Expected Behaviour: Sites manifest with the engine's threshold/excess probability law.
        // Verification: exact selected genesis-response cohort; not a Born-rule or collapse test.
        configure_genesis_gate_terms(rb);
        const double sigma = seed::real("geometry.sigma", N / 8.0, 0.5, std::max(N * 0.5, 64.0),
            "Envelope width", "Gaussian width (lattice sites) of the seeded genesis envelope.");
        const double amp = seed::real("packet.amplitude", K_B * 2.0, 0.0, K_B * 40.0,
            "Envelope peak amplitude", "Peak flux/wave-velocity magnitude at the center of the seeded envelope.");
        // Fixed orientation makes the native C++ and browser fallback profiles
        // exactly reproducible. Genesis depends on magnitude, so random phase
        // added no information to this single-envelope response probe.
        const double theta = seed::real("packet.phase", PI / 7.0, -PI, PI,
            "In-plane phase", "Angle (radians) splitting the envelope's peak magnitude between the x- and y-flux components.");
        const double cutoff = seed::real("geometry.cutoff", 0.001, 0.0, 1.0,
            "Amplitude cutoff", "Sites where the Gaussian envelope falls below this magnitude are left unwritten.");
        const int cx = seed::integer("source.x", mid, 0, N - 1,
            "Envelope center x", "Lattice x-coordinate of the seeded genesis envelope's center.");
        const int cy = seed::integer("source.y", mid, 0, N - 1,
            "Envelope center y", "Lattice y-coordinate of the seeded genesis envelope's center.");
        const int cz = seed::integer("source.z", mid, 0, N - 1,
            "Envelope center z", "Lattice z-coordinate of the seeded genesis envelope's center.");
        const int pulseR = std::min(CEL(sigma * 3), mid - 1);
        for (int dz = -pulseR; dz <= pulseR; dz++) for (int dy = -pulseR; dy <= pulseR; dy++) for (int dx = -pulseR; dx <= pulseR; dx++) {
            double val = amp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2 * sigma * sigma));
            if (val > cutoff) { IF(rb, cx + dx, cy + dy, cz + dz, val * std::cos(theta), val * std::sin(theta), 0); IW(rb, cx + dx, cy + dy, cz + dz, val * std::cos(theta), val * std::sin(theta), 0); }
        }
        // B2 (2026-07-27): SCOPED OUT of the remove_wave_mean fix, deliberately.
        // One-tick genesis-threshold probe (ticked once in test_scenario_
        // behavior.cpp); the drift remove_wave_mean guards against needs many
        // ticks to accumulate. Measured cost of applying it anyway: it shifts
        // the exact one-tick manifested count away from its deterministic
        // baseline (36 -> 32) with no corresponding benefit at this usage
        // pattern. Left unprojected; revisit if usage changes to continuous
        // ticking.
    }
    else if (name == "quantum-double-slit") {
        // Scenario ID: quantum-double-slit
        // Physical Purpose: Simulates classical coherent two-source interference on the lattice.
        // Initial Condition Parameters: None.
        // Expected Behaviour: exact classical two-source superposition. At the
        // qualified L=48, t=20 screen the cross term is constructive but has no
        // destructive band, closing the double-slit-fringe interpretation for
        // this seed. There is no single-particle impact accumulator.
        configure_free_wave();
        const int sigma = seed::integer("geometry.sigma", 2, 1, std::max(N, 64),
            "Source width", "Gaussian width (lattice sites) of each slit source, used both along x and along the transverse (separation) axis.");
        const double sAmp = seed::real("packet.amplitude", 0.3, 0.0, 0.3 * 40.0,
            "Source amplitude", "Shared peak amplitude of both coherent slit sources.");
        const int slit_sep = seed::integer("geometry.slitSeparation", N / 6, 0, std::max(N, 64),
            "Slit half-separation", "Half the distance (lattice sites) between the two slit sources, measured from the lattice center along y.");
        const int slit_x = seed::integer("source.x", N / 4, 0, N - 1,
            "Source x position", "Shared lattice x-coordinate of both slit sources.");
        const int seed_wave_0_direction = seed::choice("wave0.direction", +1, {{-1,"-x"},{1,"+x"}}, "Wave 1 direction", "Changes the travel direction of wave ingredient 1 along x.");
        const double seed_wave_0_carrierK = seed::real("wave0.carrierK", 0.0, 0.0, PI, "Wave 1 carrier wavenumber", "Sets wave 1 carrier wavenumber of wave ingredient 1; increasing it changes the prepared profile before the first tick.", "radians per cell");
        const double seed_wave_0_phase = seed::real("wave0.phase", 0.0, -PI, PI, "Wave 1 carrier phase", "Sets wave 1 carrier phase of wave ingredient 1; increasing it changes the prepared profile before the first tick.", "radians");
        const int seed_wave_0_polarization = seed::choice("wave0.polarization", 2, {{1,"y"},{2,"z"}}, "Wave 1 polarization", "Selects the supported transverse field axis of wave ingredient 1.");
        const double seed_wave_0_sigmaTransverse = seed::real("wave0.sigmaTransverse", sigma, 1.0, std::max(64.0, double(N)), "Wave 1 transverse width", "Sets wave 1 transverse width of wave ingredient 1; increasing it changes the prepared profile before the first tick.", "cells");
        inject_sheet_packet_x(rb, slit_x, mid - slit_sep, sigma, seed_wave_0_sigmaTransverse, sAmp, seed_wave_0_direction, seed_wave_0_polarization, seed_wave_0_carrierK, seed_wave_0_phase);
        const int seed_wave_1_direction = seed::choice("wave1.direction", +1, {{-1,"-x"},{1,"+x"}}, "Wave 2 direction", "Changes the travel direction of wave ingredient 2 along x.");
        const double seed_wave_1_carrierK = seed::real("wave1.carrierK", 0.0, 0.0, PI, "Wave 2 carrier wavenumber", "Sets wave 2 carrier wavenumber of wave ingredient 2; increasing it changes the prepared profile before the first tick.", "radians per cell");
        const double seed_wave_1_phase = seed::real("wave1.phase", 0.0, -PI, PI, "Wave 2 carrier phase", "Sets wave 2 carrier phase of wave ingredient 2; increasing it changes the prepared profile before the first tick.", "radians");
        const int seed_wave_1_polarization = seed::choice("wave1.polarization", 2, {{1,"y"},{2,"z"}}, "Wave 2 polarization", "Selects the supported transverse field axis of wave ingredient 2.");
        const double seed_wave_1_sigmaTransverse = seed::real("wave1.sigmaTransverse", sigma, 1.0, std::max(64.0, double(N)), "Wave 2 transverse width", "Sets wave 2 transverse width of wave ingredient 2; increasing it changes the prepared profile before the first tick.", "cells");
        inject_sheet_packet_x(rb, slit_x, mid + slit_sep, sigma, seed_wave_1_sigmaTransverse, sAmp, seed_wave_1_direction, seed_wave_1_polarization, seed_wave_1_carrierK, seed_wave_1_phase);
    }
    else if (name == "quantum-eraser") {
        // Scenario ID: quantum-eraser
        // Physical Purpose: Tests classical transmission of two coherent paths through a state-field grid.
        // Initial Condition Parameters: None.
        // Expected Behaviour: The checkerboard states strongly source the
        // native coupling response. Verification closes the eraser framing;
        // there is no measurement or polarization projection operator.
        configure_free_wave();
        rb.toggles.coupling = true;
        const int sigma = seed::integer("geometry.sigma", 2, 1, std::max(N, 64),
            "Source width", "Gaussian width (lattice sites) of each slit source, used both along x and along the transverse (separation) axis.");
        const double sAmp = seed::real("packet.amplitude", 0.3, 0.0, 0.3 * 40.0,
            "Source amplitude", "Shared peak amplitude of both coherent slit sources.");
        const int slit_sep = seed::integer("geometry.slitSeparation", N / 6, 0, std::max(N, 64),
            "Slit half-separation", "Half the distance (lattice sites) between the two slit sources, measured from the lattice center along y.");
        const int slit_x = seed::integer("source.x", N / 4, 0, N - 1,
            "Source x position", "Shared lattice x-coordinate of both slit sources.");

        const int seed_wave_0_direction = seed::choice("wave0.direction", +1, {{-1,"-x"},{1,"+x"}}, "Wave 1 direction", "Changes the travel direction of wave ingredient 1 along x.");
        const double seed_wave_0_carrierK = seed::real("wave0.carrierK", 0.0, 0.0, PI, "Wave 1 carrier wavenumber", "Sets wave 1 carrier wavenumber of wave ingredient 1; increasing it changes the prepared profile before the first tick.", "radians per cell");
        const double seed_wave_0_phase = seed::real("wave0.phase", 0.0, -PI, PI, "Wave 1 carrier phase", "Sets wave 1 carrier phase of wave ingredient 1; increasing it changes the prepared profile before the first tick.", "radians");
        const int seed_wave_0_polarization = seed::choice("wave0.polarization", 2, {{1,"y"},{2,"z"}}, "Wave 1 polarization", "Selects the supported transverse field axis of wave ingredient 1.");
        const double seed_wave_0_sigmaTransverse = seed::real("wave0.sigmaTransverse", sigma, 1.0, std::max(64.0, double(N)), "Wave 1 transverse width", "Sets wave 1 transverse width of wave ingredient 1; increasing it changes the prepared profile before the first tick.", "cells");
        inject_sheet_packet_x(rb, slit_x, mid - slit_sep, sigma, seed_wave_0_sigmaTransverse, sAmp, seed_wave_0_direction, seed_wave_0_polarization, seed_wave_0_carrierK, seed_wave_0_phase);
        const int seed_wave_1_direction = seed::choice("wave1.direction", +1, {{-1,"-x"},{1,"+x"}}, "Wave 2 direction", "Changes the travel direction of wave ingredient 2 along x.");
        const double seed_wave_1_carrierK = seed::real("wave1.carrierK", 0.0, 0.0, PI, "Wave 2 carrier wavenumber", "Sets wave 2 carrier wavenumber of wave ingredient 2; increasing it changes the prepared profile before the first tick.", "radians per cell");
        const double seed_wave_1_phase = seed::real("wave1.phase", 0.0, -PI, PI, "Wave 2 carrier phase", "Sets wave 2 carrier phase of wave ingredient 2; increasing it changes the prepared profile before the first tick.", "radians");
        const int seed_wave_1_polarization = seed::choice("wave1.polarization", 2, {{1,"y"},{2,"z"}}, "Wave 2 polarization", "Selects the supported transverse field axis of wave ingredient 2.");
        const double seed_wave_1_sigmaTransverse = seed::real("wave1.sigmaTransverse", sigma, 1.0, std::max(64.0, double(N)), "Wave 2 transverse width", "Sets wave 2 transverse width of wave ingredient 2; increasing it changes the prepared profile before the first tick.", "cells");
        inject_sheet_packet_x(rb, slit_x, mid + slit_sep, sigma, seed_wave_1_sigmaTransverse, sAmp, seed_wave_1_direction, seed_wave_1_polarization, seed_wave_1_carrierK, seed_wave_1_phase);

        // Diagonal eraser (y=z polarizer) at x = N/2
        const int eraserX = seed::integer("source.eraserX", N / 2, 0, N - 1,
            "Eraser plane x position", "Lattice x-coordinate of the diagonal (y=z) checkerboard eraser plane.");
        for (int y = 0; y < N; y++) {
            for (int z = 0; z < N; z++) {
                if ((y + z) % 2 == 0) {
                    IP(rb, eraserX, y, z, 1);
                    LOCK(rb, eraserX, y, z);
                }
            }
        }

        rb.toggles.genesis = false;
    }
    else if (name == "quantum-tunnel") {
        // Scenario ID: quantum-tunnel
        // Physical Purpose: Measures native wave transmission through a locked state wall.
        // Initial Condition Parameters: None.
        // Expected Behaviour: The locked state sheets drive the native
        // coupling term and strongly amplify the field. Verification closes
        // the tunneling-barrier interpretation; this is not a Schrodinger potential.
        configure_free_wave();
        rb.toggles.coupling = true;
        const double sigma = seed::real("geometry.sigma", N / 12.0, 0.5, std::max(N * 0.5, 64.0),
            "Packet width", "Gaussian width (lattice sites) of the incident packet, used both along the direction of travel and transversely.");
        const double packetX = seed::real("source.x", double(N / 4), 0.0, N - 1.0,
            "Packet start x", "Lattice x-coordinate where the incident packet is centered before the first tick.");
        const double pAmp = seed::real("packet.amplitude", K_B * 0.5, 0.0, K_B * 40.0,
            "Packet amplitude", "Peak amplitude of the seeded curl-potential building the incident divergence-free transverse packet.");
        const int seed_wave_0_direction = seed::choice("wave0.direction", +1, {{-1,"-x"},{1,"+x"}}, "Wave 1 direction", "Changes the travel direction of wave ingredient 1 along x.");
        const double seed_wave_0_carrierK = seed::real("wave0.carrierK", 0.0, 0.0, PI, "Wave 1 carrier wavenumber", "Sets wave 1 carrier wavenumber of wave ingredient 1; increasing it changes the prepared profile before the first tick.", "radians per cell");
        const double seed_wave_0_phase = seed::real("wave0.phase", 0.0, -PI, PI, "Wave 1 carrier phase", "Sets wave 1 carrier phase of wave ingredient 1; increasing it changes the prepared profile before the first tick.", "radians");
        const double seed_wave_0_sigmaTransverse = seed::real("wave0.sigmaTransverse", sigma, 1.0, std::max(64.0, double(N)), "Wave 1 transverse width", "Sets wave 1 transverse width of wave ingredient 1; increasing it changes the prepared profile before the first tick.", "cells");
        inject_transverse_packet_x(rb, packetX, mid, mid, sigma, seed_wave_0_sigmaTransverse, pAmp, seed_wave_0_direction, seed_wave_0_carrierK, seed_wave_0_phase);
        const int W = seed::integer("geometry.wallThickness", 3, 1, std::max(N, 64),
            "Wall thickness", "Number of lattice planes (along x) making up the locked state wall.");
        for (int y = 0; y < N; y++) for (int z = 0; z < N; z++) for (int dx = 0; dx < W; dx++) {
            IP(rb, mid + dx, y, z, 1);
            LOCK(rb, mid + dx, y, z);
        }
    }
    else if (name == "quantum-well") {
        // Scenario ID: quantum-well
        // Physical Purpose: Visualizes an imposed basis of standing-wave harmonics between marker planes.
        // Initial Condition Parameters: None.
        // Expected Behaviour: The explicitly seeded n=1..8 sine basis evolves
        // exactly as it does with the marker planes removed and propagates
        // beyond them. Verification closes the confinement interpretation.
        // The planes are inert markers, not Gauss charge sheets or material
        // boundaries. Isolate the unprojected wave map so the scenario tests
        // that distinction directly.
        configure_free_wave_terms(rb, false);
        const int wallA = seed::integer("source.wallA", N / 4, 0, N - 1,
            "Left wall x position", "Lattice x-coordinate of the left inert marker plane.");
        const int wallB = seed::integer("source.wallB", 3 * N / 4, 0, N - 1,
            "Right wall x position", "Lattice x-coordinate of the right inert marker plane.");
        const double baseAmp = seed::real("packet.amplitude", K_B * 0.5, 0.0, K_B * 40.0,
            "Fundamental-mode amplitude", "Amplitude of the n=1 standing-wave harmonic; each higher harmonic n=2..8 is seeded at this value divided by n.");
        const double cutoff = seed::real("geometry.cutoff", 1e-6, 0.0, 1.0,
            "Amplitude cutoff", "Sites where a harmonic's magnitude falls below this value are left unwritten.");
        const int harmonicCount = seed::integer("harmonic.count", 8, 1, std::max(N - 1, 8),
            "Harmonic count", "Number of standing-wave harmonics n=1..count superimposed in the basis.");
        const double weightExponent = seed::real("harmonic.weightExponent", 1.0, 0.0, 8.0,
            "Harmonic weight exponent", "Each harmonic n's amplitude is the fundamental amplitude divided by n^exponent.");
        const int boxLength = wallB - wallA;
        for (int y = 0; y < N; y++) for (int z = 0; z < N; z++) {
            IP(rb, wallA, y, z, 1); LOCK(rb, wallA, y, z);
            IP(rb, wallB, y, z, 1); LOCK(rb, wallB, y, z);
        }
        for (int n = 1; n <= harmonicCount; n++) {
            double amp_n = baseAmp / std::pow(double(n), weightExponent);
            for (int x = wallA + 1; x < wallB; x++) for (int y = 0; y < N; y++) for (int z = 0; z < N; z++) {
                double val = amp_n * std::sin(n * PI * (x - wallA) / double(boxLength));
                if (std::fabs(val) > cutoff) IF(rb, x, y, z, 0, val, 0);
            }
        }
        rb.toggles.genesis = false;
        rb.toggles.damping = false;
        rb.toggles.selective_damping = false;
    }
    else if (name == "quantum-entangle") {
        // Scenario ID: quantum-entangle
        // Physical Purpose: Seeds the engine's native tagged, anti-correlated pair object.
        // Initial Condition Parameters: None.
        // Expected Behaviour: Two complementary states share one pair_id and opposite flux.
        // Verification: classical tagged-pair correlation; not Bell entanglement.
        // This is an initial-data bookkeeping probe.  Previously it inherited
        // the RenderBridge defaults (wave/coupling/Gauss/forces/gravity/
        // Lorentz/dual/weak) and disabled only genesis + movement.  The native
        // application's automatic prime tick therefore evolved the pair under
        // an unrelated full-physics stack before the first frame was drawn.
        configure_static_seed_terms(rb);
        const double pairAmp = seed::real("packet.amplitude", K_B, 0.0, K_B * 40.0,
            "Pair z-flux magnitude", "Magnitude of the opposite-signed z-flux the engine's tagged pair carries.");
        const int px = seed::integer("source.x", mid, 0, N - 1,
            "Pair site x", "Lattice x-coordinate of the tagged, anti-correlated pair.");
        const int py = seed::integer("source.y", mid, 0, N - 1,
            "Pair site y", "Lattice y-coordinate of the tagged, anti-correlated pair.");
        const int pz = seed::integer("source.z", mid, 0, N - 1,
            "Pair site z", "Lattice z-coordinate of the tagged, anti-correlated pair.");
        rb.create_entangled_pair(px, py, pz, Vec3(0.0, 0.0, pairAmp));
    }
    else if (name == "quantum-aharonov-bohm") {
        // Scenario ID: quantum-aharonov-bohm
        // Physical Purpose: Provides a solenoid-plus-two-path topology for testing a future gauge-phase observable.
        // Initial Condition Parameters: None.
        // Expected Behaviour: tube and paths evolve as exact linear
        // superposition. Verification closes the interaction claim; no link
        // holonomy or phase-shift observable exists.
        configure_free_wave();
        // genesis=false (audit-2 2026-04-28): A-B effect is gauge-phase,
        // packets shouldn't pair-produce while traversing the solenoid.
        const int R = seed::integer("geometry.radius", N / 8, 1, std::max(N, 64),
            "Solenoid radius", "Radius (lattice sites) of the imposed solenoid tube in the x-y plane.");
        const double solenoidFlux = seed::real("field.solenoidFlux", K_B * 0.5, -K_B * 40.0, K_B * 40.0,
            "Solenoid z-flux", "Uniform z-flux magnitude imposed inside the solenoid tube.");
        const int solX = seed::integer("source.x", mid, 0, N - 1,
            "Solenoid center x", "Lattice x-coordinate of the solenoid tube's axis in the x-y plane.");
        const int solY = seed::integer("source.y", mid, 0, N - 1,
            "Solenoid center y", "Lattice y-coordinate of the solenoid tube's axis in the x-y plane.");
        for (int z = 0; z < N; z++) for (int dy = -R; dy <= R; dy++) for (int dx = -R; dx <= R; dx++) {
            if (dx * dx + dy * dy > R * R) continue;
            IF(rb, solX + dx, solY + dy, z, 0, 0, solenoidFlux);
        }
        const int pSigma = seed::integer("geometry.packetSigma", 3, 1, std::max(N, 64),
            "Packet width", "Gaussian width (lattice sites) of each of the two paths' incident packet.");
        const int pStartX = seed::integer("source.pathX", N / 4, 0, N - 1,
            "Packet start x", "Shared lattice x-coordinate of both packets' centers.");
        const int margin = seed::integer("geometry.margin", 2, 0, std::max(N, 64),
            "Path clearance margin", "Distance (lattice sites) each path's center is offset outward from the solenoid radius.");
        const double pAmp = seed::real("packet.amplitude", K_B * 0.5, 0.0, K_B * 40.0,
            "Packet amplitude", "Peak amplitude of the seeded curl-potential building each divergence-free transverse packet.");
        const int seed_wave_0_direction = seed::choice("wave0.direction", +1, {{-1,"-x"},{1,"+x"}}, "Wave 1 direction", "Changes the travel direction of wave ingredient 1 along x.");
        const double seed_wave_0_carrierK = seed::real("wave0.carrierK", 0.0, 0.0, PI, "Wave 1 carrier wavenumber", "Sets wave 1 carrier wavenumber of wave ingredient 1; increasing it changes the prepared profile before the first tick.", "radians per cell");
        const double seed_wave_0_phase = seed::real("wave0.phase", 0.0, -PI, PI, "Wave 1 carrier phase", "Sets wave 1 carrier phase of wave ingredient 1; increasing it changes the prepared profile before the first tick.", "radians");
        const double seed_wave_0_sigmaTransverse = seed::real("wave0.sigmaTransverse", pSigma, 1.0, std::max(64.0, double(N)), "Wave 1 transverse width", "Sets wave 1 transverse width of wave ingredient 1; increasing it changes the prepared profile before the first tick.", "cells");
        inject_transverse_packet_x(rb, pStartX, solY + R + margin, mid, pSigma, seed_wave_0_sigmaTransverse, pAmp, seed_wave_0_direction, seed_wave_0_carrierK, seed_wave_0_phase);
        const int seed_wave_1_direction = seed::choice("wave1.direction", +1, {{-1,"-x"},{1,"+x"}}, "Wave 2 direction", "Changes the travel direction of wave ingredient 2 along x.");
        const double seed_wave_1_carrierK = seed::real("wave1.carrierK", 0.0, 0.0, PI, "Wave 2 carrier wavenumber", "Sets wave 2 carrier wavenumber of wave ingredient 2; increasing it changes the prepared profile before the first tick.", "radians per cell");
        const double seed_wave_1_phase = seed::real("wave1.phase", 0.0, -PI, PI, "Wave 2 carrier phase", "Sets wave 2 carrier phase of wave ingredient 2; increasing it changes the prepared profile before the first tick.", "radians");
        const double seed_wave_1_sigmaTransverse = seed::real("wave1.sigmaTransverse", pSigma, 1.0, std::max(64.0, double(N)), "Wave 2 transverse width", "Sets wave 2 transverse width of wave ingredient 2; increasing it changes the prepared profile before the first tick.", "cells");
        inject_transverse_packet_x(rb, pStartX, solY - R - margin, mid, pSigma, seed_wave_1_sigmaTransverse, pAmp, seed_wave_1_direction, seed_wave_1_carrierK, seed_wave_1_phase);
    }
    else if (name == "quantum-casimir") {
        // Scenario ID: quantum-casimir
        // Physical Purpose: Tests whether two locked marker planes affect a
        // reproducible transverse lattice eigenmode.
        // Verification: plate/no-plate null control; no vacuum ensemble,
        // force estimator, or Casimir mechanism is present.
        configure_free_wave_terms(rb, false);
        const int d = seed::integer("geometry.plateSeparation", 6, 0, std::max(N, 64),
            "Plate separation", "Distance (lattice sites) between the two locked marker planes.");
        const int cx = seed::integer("source.x", mid, 0, N - 1,
            "Plate pair center x", "Lattice x-coordinate about which the two locked marker planes are symmetrically offset.");
        const int plateA = cx - d / 2, plateB = cx + d / 2;
        for (int y = 0; y < N; y++) for (int z = 0; z < N; z++) {
            IP(rb, plateA, y, z, 1); LOCK(rb, plateA, y, z);
            IP(rb, plateB, y, z, 1); LOCK(rb, plateB, y, z);
        }
        const int modeN = seed::integer("harmonic.modeNumber", 4, 1, std::max(N - 1, 8),
            "Harmonic mode number", "Integer wavenumber index n (k = 2*pi*n/N) of the seeded transverse eigenmode.");
        const double hAmp = seed::real("packet.amplitude", 0.05, 0.0, 0.05 * 40.0,
            "Harmonic amplitude", "Peak amplitude of the seeded transverse eigenmode.");
        const int seed_wave_0_direction = seed::choice("wave0.direction", +1, {{-1,"-x"},{1,"+x"}}, "Wave 1 direction", "Changes the travel direction of wave ingredient 1 along x.");
        inject_plane_harmonic_x(rb, modeN, hAmp, seed_wave_0_direction);
    }
    else if (name == "quantum-zeno") {
        // Scenario ID: quantum-zeno
        // Physical Purpose: Measures a supercritical one-tick genesis cohort.
        // Initial Condition Parameters: None.
        // Expected Behaviour: Natural genesis/decay proceeds without a measurement intervention.
        // Verification: genesis-response control only; no measurement operator exists in the frozen engine.
        configure_genesis_gate_terms(rb);
        const double sigma = seed::real("geometry.sigma", N / 10.0, 0.5, std::max(N * 0.5, 64.0),
            "Envelope width", "Gaussian width (lattice sites) of the seeded supercritical envelope.");
        const double amp = seed::real("packet.amplitude", K_GENESIS * 1.2, 0.0, K_GENESIS * 40.0,
            "Envelope peak amplitude", "Peak flux/wave-velocity magnitude at the center of the seeded envelope, equal on all three components.");
        const double cutoff = seed::real("geometry.cutoff", 0.001, 0.0, 1.0,
            "Amplitude cutoff", "Sites where the Gaussian envelope falls below this magnitude are left unwritten.");
        const int cx = seed::integer("source.x", mid, 0, N - 1,
            "Envelope center x", "Lattice x-coordinate of the seeded supercritical envelope's center.");
        const int cy = seed::integer("source.y", mid, 0, N - 1,
            "Envelope center y", "Lattice y-coordinate of the seeded supercritical envelope's center.");
        const int cz = seed::integer("source.z", mid, 0, N - 1,
            "Envelope center z", "Lattice z-coordinate of the seeded supercritical envelope's center.");
        const int pulseR = std::min(CEL(sigma * 3), mid - 1);
        for (int dz = -pulseR; dz <= pulseR; dz++) for (int dy = -pulseR; dy <= pulseR; dy++) for (int dx = -pulseR; dx <= pulseR; dx++) {
            double val = amp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2 * sigma * sigma));
            if (val > cutoff) { IF(rb, cx + dx, cy + dy, cz + dz, val, val, val); IW(rb, cx + dx, cy + dy, cz + dz, val, val, val); }
        }
        // B2 (2026-07-27): SCOPED OUT of the remove_wave_mean fix, deliberately.
        // One-tick genesis-threshold probe; see quantum-born-rule above for the
        // full rationale. Measured cost of applying it anyway: exact one-tick
        // manifested count shifts from its deterministic baseline (491 -> 461)
        // with no corresponding benefit at this usage pattern.
    }
    else {
        return false;
    }
    return true;
}

}  // namespace ftd
