// ==========================================================================
//  engine/src/scenarios/quantum.cpp
//
//  Group: quantum-* (8 scenarios)
//  JS source: engine/web/js/bridge/scenarios/quantum-scenarios.js
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
        const double sigma = N / 8.0;
        const double amp = K_B * 2.0;
        // Fixed orientation makes the native C++ and browser fallback profiles
        // exactly reproducible. Genesis depends on magnitude, so random phase
        // added no information to this single-envelope response probe.
        const double theta = PI / 7.0;
        const int pulseR = std::min(CEL(sigma * 3), mid - 1);
        for (int dz = -pulseR; dz <= pulseR; dz++) for (int dy = -pulseR; dy <= pulseR; dy++) for (int dx = -pulseR; dx <= pulseR; dx++) {
            double val = amp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2 * sigma * sigma));
            if (val > 0.001) { IF(rb, mid + dx, mid + dy, mid + dz, val * std::cos(theta), val * std::sin(theta), 0); IW(rb, mid + dx, mid + dy, mid + dz, val * std::cos(theta), val * std::sin(theta), 0); }
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
        const int sigma = 2;
        const double sAmp = 0.3;
        const int slit_sep = N / 6;
        const int slit_x = N / 4;
        inject_sheet_packet_x(rb, slit_x, mid - slit_sep, sigma, sigma, sAmp, +1);
        inject_sheet_packet_x(rb, slit_x, mid + slit_sep, sigma, sigma, sAmp, +1);
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
        const int sigma = 2;
        const double sAmp = 0.3;
        const int slit_sep = N / 6;
        const int slit_x = N / 4;

        inject_sheet_packet_x(rb, slit_x, mid - slit_sep, sigma, sigma, sAmp, +1);
        inject_sheet_packet_x(rb, slit_x, mid + slit_sep, sigma, sigma, sAmp, +1);

        // Diagonal eraser (y=z polarizer) at x = N/2
        const int eraserX = N / 2;
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
        const double sigma = N / 12.0;
        const int packetX = N / 4;
        inject_transverse_packet_x(rb, packetX, mid, mid, sigma, sigma,
                                   K_B * 0.5, +1);
        const int W = 3;
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
        const int wallA = N / 4;
        const int wallB = 3 * N / 4;
        const int boxLength = wallB - wallA;
        for (int y = 0; y < N; y++) for (int z = 0; z < N; z++) {
            IP(rb, wallA, y, z, 1); LOCK(rb, wallA, y, z);
            IP(rb, wallB, y, z, 1); LOCK(rb, wallB, y, z);
        }
        for (int n = 1; n <= 8; n++) {
            double amp_n = K_B * 0.5 / n;
            for (int x = wallA + 1; x < wallB; x++) for (int y = 0; y < N; y++) for (int z = 0; z < N; z++) {
                double val = amp_n * std::sin(n * PI * (x - wallA) / double(boxLength));
                if (std::fabs(val) > 1e-6) IF(rb, x, y, z, 0, val, 0);
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
        rb.toggles.genesis = false;
        rb.toggles.evaporation = false;
        rb.toggles.movement = false;
        rb.create_entangled_pair(mid, mid, mid, Vec3(0.0, 0.0, K_B));
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
        const int R = N / 8;
        for (int z = 0; z < N; z++) for (int dy = -R; dy <= R; dy++) for (int dx = -R; dx <= R; dx++) {
            if (dx * dx + dy * dy > R * R) continue;
            IF(rb, mid + dx, mid + dy, z, 0, 0, K_B * 0.5);
        }
        const int pSigma = 3;
        const int pStartX = N / 4;
        inject_transverse_packet_x(rb, pStartX, mid + R + 2, mid,
                                   pSigma, pSigma, K_B * 0.5, +1);
        inject_transverse_packet_x(rb, pStartX, mid - R - 2, mid,
                                   pSigma, pSigma, K_B * 0.5, +1);
    }
    else if (name == "quantum-casimir") {
        // Scenario ID: quantum-casimir
        // Physical Purpose: Tests whether two locked marker planes affect a
        // reproducible transverse lattice eigenmode.
        // Verification: plate/no-plate null control; no vacuum ensemble,
        // force estimator, or Casimir mechanism is present.
        configure_free_wave_terms(rb, false);
        const int d = 6;
        const int plateA = mid - d / 2, plateB = mid + d / 2;
        for (int y = 0; y < N; y++) for (int z = 0; z < N; z++) {
            IP(rb, plateA, y, z, 1); LOCK(rb, plateA, y, z);
            IP(rb, plateB, y, z, 1); LOCK(rb, plateB, y, z);
        }
        inject_plane_harmonic_x(rb, 4, 0.05, +1);
    }
    else if (name == "quantum-zeno") {
        // Scenario ID: quantum-zeno
        // Physical Purpose: Measures a supercritical one-tick genesis cohort.
        // Initial Condition Parameters: None.
        // Expected Behaviour: Natural genesis/decay proceeds without a measurement intervention.
        // Verification: genesis-response control only; no measurement operator exists in the frozen engine.
        configure_genesis_gate_terms(rb);
        const double sigma = N / 10.0;
        const double amp = K_GENESIS * 1.2;
        const int pulseR = std::min(CEL(sigma * 3), mid - 1);
        for (int dz = -pulseR; dz <= pulseR; dz++) for (int dy = -pulseR; dy <= pulseR; dy++) for (int dx = -pulseR; dx <= pulseR; dx++) {
            double val = amp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2 * sigma * sigma));
            if (val > 0.001) { IF(rb, mid + dx, mid + dy, mid + dz, val, val, val); IW(rb, mid + dx, mid + dy, mid + dz, val, val, val); }
        }
        // B2 (2026-07-27): SCOPED OUT of the remove_wave_mean fix, deliberately.
        // One-tick genesis-threshold probe; see quantum-born-rule above for the
        // full rationale. Measured cost of applying it anyway: exact one-tick
        // manifested count shifts from its deterministic baseline (491 -> 461)
        // with no corresponding benefit at this usage pattern.
    }
    return true;
}

}  // namespace ftd
