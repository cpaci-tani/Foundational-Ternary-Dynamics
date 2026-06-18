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

    if (name == "quantum-born-rule") {
        // Scenario ID: quantum-born-rule
        // Physical Purpose: Tests the quantum Born rule / wave-function collapse.
        // Initial Condition Parameters: None.
        // Expected Behaviour: Localized wave packet collapses stochastically into particles with Born-rule probability.
        // Discrepancy: None.
        const double sigma = N / 8.0;
        const double amp = K_B * 2.0;
        const double theta = urand() * 2.0 * PI;
        const int pulseR = std::min(CEL(sigma * 3), mid - 1);
        for (int dz = -pulseR; dz <= pulseR; dz++) for (int dy = -pulseR; dy <= pulseR; dy++) for (int dx = -pulseR; dx <= pulseR; dx++) {
            double val = amp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2 * sigma * sigma));
            if (val > 0.001) { IF(rb, mid + dx, mid + dy, mid + dz, val * std::cos(theta), val * std::sin(theta), 0); IW(rb, mid + dx, mid + dy, mid + dz, val * std::cos(theta), val * std::sin(theta), 0); }
        }
        rb.toggles.genesis = true;
    }
    else if (name == "quantum-double-slit") {
        // Scenario ID: quantum-double-slit
        // Physical Purpose: Quantitatively simulates double-slit quantum particle interference.
        // Initial Condition Parameters: None.
        // Expected Behaviour: Emergence of interference pattern from accumulated single particle impacts.
        // Discrepancy: None.
        const int sigma = 2;
        const double sAmp = 0.3;
        const int slit_sep = N / 6;
        const int slit_x = N / 4;
        const int slit_ys[2] = { mid - slit_sep, mid + slit_sep };
        for (int i = 0; i < 2; i++) {
            int sy = slit_ys[i];
            for (int z = 0; z < N; z++) for (int dy = -4; dy <= 4; dy++) for (int dx = -4; dx <= 4; dx++) {
                double g = sAmp * std::exp(-(dx*dx + dy*dy) / (2.0 * sigma * sigma));
                if (g < 1e-6) continue;
                int px = slit_x + dx, py = sy + dy;
                if (px < 0 || px >= N || py < 0 || py >= N) continue;
                IF(rb, px, py, z, 0, 0, g);
                IW(rb, px, py, z, g, 0, 0);
            }
        }
        rb.toggles.genesis = true;
        rb.toggles.coupling = false;
    }
    else if (name == "quantum-eraser") {
        // Scenario ID: quantum-eraser
        // Physical Purpose: Simulates the quantum eraser experiment with path polarization markers.
        // Initial Condition Parameters: None.
        // Expected Behaviour: Restoring interference pattern when path information is erased by polarizer grid.
        // Discrepancy: None.
        const int sigma = 2;
        const double sAmp = 0.3;
        const int slit_sep = N / 6;
        const int slit_x = N / 4;

        // Slit 1: y-polarized
        const int sy1 = mid - slit_sep;
        for (int z = 0; z < N; z++) for (int dy = -4; dy <= 4; dy++) for (int dx = -4; dx <= 4; dx++) {
            double g = sAmp * std::exp(-(dx*dx + dy*dy) / (2.0 * sigma * sigma));
            if (g < 1e-6) continue;
            int px = slit_x + dx, py = sy1 + dy;
            if (px < 0 || px >= N || py < 0 || py >= N) continue;
            IF(rb, px, py, z, 0, g, 0);
            IW(rb, px, py, z, g, 0, 0);
        }

        // Slit 2: z-polarized
        const int sy2 = mid + slit_sep;
        for (int z = 0; z < N; z++) for (int dy = -4; dy <= 4; dy++) for (int dx = -4; dx <= 4; dx++) {
            double g = sAmp * std::exp(-(dx*dx + dy*dy) / (2.0 * sigma * sigma));
            if (g < 1e-6) continue;
            int px = slit_x + dx, py = sy2 + dy;
            if (px < 0 || px >= N || py < 0 || py >= N) continue;
            IF(rb, px, py, z, 0, 0, g);
            IW(rb, px, py, z, g, 0, 0);
        }

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

        rb.toggles.genesis = true;
        rb.toggles.coupling = false;
    }
    else if (name == "quantum-tunnel") {
        // Scenario ID: quantum-tunnel
        // Physical Purpose: Demonstrates quantum tunneling of a wave packet through a potential barrier.
        // Initial Condition Parameters: None.
        // Expected Behaviour: Partial transmission of a wave packet through a thin potential barrier.
        // Discrepancy: None.
        // genesis=false (audit-2 2026-04-28): the wave packet should TUNNEL,
        // not pair-produce. The 3072 initial particles are the locked
        // barrier (32×32×W=3 wall). Mirrors JS quantum-tunnel.
        rb.toggles.genesis = false;
        const double sigma = N / 12.0;
        const double amp = K_B * 2.0;
        const int packetX = N / 4;
        const int pulseR = std::min(CEL(sigma * 3), mid - 1);
        for (int dz = -pulseR; dz <= pulseR; dz++) for (int dy = -pulseR; dy <= pulseR; dy++) for (int dx = -pulseR; dx <= pulseR; dx++) {
            double val = amp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2 * sigma * sigma));
            if (val > 0.001) {
                int x = packetX + dx, y = mid + dy, z = mid + dz;
                if (x >= 0 && x < N && y >= 0 && y < N && z >= 0 && z < N) {
                    IF(rb, x, y, z, val, 0, 0);
                    IW(rb, x, y, z, val, 0, 0);
                }
            }
        }
        const int W = 3;
        for (int y = 0; y < N; y++) for (int z = 0; z < N; z++) for (int dx = 0; dx < W; dx++) {
            IP(rb, mid + dx, y, z, 1);
            LOCK(rb, mid + dx, y, z);
        }
    }
    else if (name == "quantum-well") {
        // Scenario ID: quantum-well
        // Physical Purpose: Simulates a particle in a 1D potential well (infinite square well).
        // Initial Condition Parameters: None.
        // Expected Behaviour: Discrete standing wave harmonics bound by potential walls.
        // Discrepancy: None.
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
    }
    else if (name == "quantum-entangle") {
        // Scenario ID: quantum-entangle
        // Physical Purpose: Tests entanglement correlation between two separated particles.
        // Initial Condition Parameters: None.
        // Expected Behaviour: Correlated behavior of two particles originating from a single high-energy flux burst.
        // Discrepancy: None.
        const double bigAmp = K_GENESIS * 5.0;
        for (int dz = -4; dz <= 4; dz++) for (int dy = -4; dy <= 4; dy++) for (int dx = -4; dx <= 4; dx++) {
            double val = bigAmp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * 6.0));
            if (val > 0.001) { IF(rb, mid + dx, mid + dy, mid + dz, val, val, val); IW(rb, mid + dx, mid + dy, mid + dz, val, val, val); }
        }
        rb.toggles.genesis = true;
    }
    else if (name == "quantum-aharonov-bohm") {
        // Scenario ID: quantum-aharonov-bohm
        // Physical Purpose: Simulates the Aharonov-Bohm effect where a vector potential shifts phase.
        // Initial Condition Parameters: None.
        // Expected Behaviour: Phase shift in the interference pattern of two paths wrapping a solenoid.
        // Discrepancy: None.
        // genesis=false (audit-2 2026-04-28): A-B effect is gauge-phase,
        // packets shouldn't pair-produce while traversing the solenoid.
        rb.toggles.genesis = false;
        const int R = N / 8;
        for (int z = 0; z < N; z++) for (int dy = -R; dy <= R; dy++) for (int dx = -R; dx <= R; dx++) {
            if (dx * dx + dy * dy > R * R) continue;
            IF(rb, mid + dx, mid + dy, z, 0, 0, K_B * 0.5);
        }
        const int pSigma = 3;
        const double pAmp = K_B * 2.0;
        const int pStartX = N / 4;
        for (int dz = -pSigma; dz <= pSigma; dz++) for (int dy = -pSigma; dy <= pSigma; dy++) for (int dx = -pSigma; dx <= pSigma; dx++) {
            double val = pAmp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * pSigma * pSigma));
            if (val > 0.001) {
                int px = pStartX + dx;
                int ayPos = mid + R + 2 + dy;
                int byPos = mid - R - 2 + dy;
                int pz = mid + dz;
                if (px >= 0 && px < N && pz >= 0 && pz < N) {
                    if (ayPos >= 0 && ayPos < N) { IF(rb, px, ayPos, pz, val, 0, 0); IW(rb, px, ayPos, pz, val, 0, 0); }
                    if (byPos >= 0 && byPos < N) { IF(rb, px, byPos, pz, val, 0, 0); IW(rb, px, byPos, pz, val, 0, 0); }
                }
            }
        }
    }
    else if (name == "quantum-casimir") {
        // Scenario ID: quantum-casimir
        // Physical Purpose: Models the Casimir attraction force between parallel plates.
        // Initial Condition Parameters: None.
        // Expected Behaviour: Modification of vacuum fluctuations between plates leading to net attractive force.
        // Discrepancy: None.
        const int d = 6;
        const int plateA = mid - d / 2, plateB = mid + d / 2;
        for (int y = 0; y < N; y++) for (int z = 0; z < N; z++) {
            IP(rb, plateA, y, z, 1); LOCK(rb, plateA, y, z);
            IP(rb, plateB, y, z, 1); LOCK(rb, plateB, y, z);
        }
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double jx = (urand() - 0.5) * K_B * 0.3;
            double jy = (urand() - 0.5) * K_B * 0.3;
            double jz = (urand() - 0.5) * K_B * 0.3;
            IF(rb, x, y, z, jx, jy, jz);
            IW(rb, x, y, z, jx, jy, jz);
        }
        rb.toggles.genesis = false;
    }
    else if (name == "quantum-zeno") {
        // Scenario ID: quantum-zeno
        // Physical Purpose: Simulates the Quantum Zeno effect (frequent observation slows evolution).
        // Initial Condition Parameters: None.
        // Expected Behaviour: Continuous measurement/interaction freezes quantum state transition.
        // Discrepancy: None.
        const double sigma = N / 10.0;
        const double amp = K_GENESIS * 1.2;
        const int pulseR = std::min(CEL(sigma * 3), mid - 1);
        for (int dz = -pulseR; dz <= pulseR; dz++) for (int dy = -pulseR; dy <= pulseR; dy++) for (int dx = -pulseR; dx <= pulseR; dx++) {
            double val = amp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2 * sigma * sigma));
            if (val > 0.001) { IF(rb, mid + dx, mid + dy, mid + dz, val, val, val); IW(rb, mid + dx, mid + dy, mid + dz, val, val, val); }
        }
        rb.toggles.genesis = true;
    }
    return true;
}

}  // namespace ftd
