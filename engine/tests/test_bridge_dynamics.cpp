/**
 * Test: RenderBridge tick dynamics
 *
 * Integration tests for vacuum stability, flux injection,
 * propagation, manifestation, and diagnostics.
 *
 * Theory references:
 *   - SPEC_FTD_LAGRANGIAN.md   (G*-tick dynamics, Born-Infeld action)
 *   - SPEC_FTD_REFERENCE.md    (update cycle phases)
 */

#include <iostream>
#include <iomanip>
#include <cmath>
#include "ftd/render_bridge.h"
#include "ftd/constants.h"

int failures = 0;

void check(const char* name, bool condition) {
    if (condition) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << "\n";
        ++failures;
    }
}

void check_close(const char* name, double a, double b, double tol) {
    bool ok = std::abs(a - b) < tol;
    if (ok) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << " (got " << std::setprecision(15)
                  << a << ", expected " << b << ")\n";
        ++failures;
    }
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: Bridge Dynamics\n";
    std::cout << "================================================================\n\n";

    const int N = 8;

    // ---- Vacuum stability ----
    // Empty lattice should remain empty after many ticks
    {
        ftd::RenderBridge bridge(N);
        bridge.run(100);
        auto d = bridge.diagnostics();
        check("Vacuum stability: no manifestation after 100 ticks",
              d.manifested_count == 0);
        check("Vacuum stability: tick counter = 100", d.tick == 100);
    }

    // ---- Flux injection ----
    {
        ftd::RenderBridge bridge(N);
        int cx = 4, cy = 4, cz = 4;
        bridge.inject_flux(cx, cy, cz, {0.5, 0.0, 0.0});

        auto& v = bridge.voxel_at(cx, cy, cz);
        check("Inject flux: state remains 0", v.state == 0);
        check("Inject flux: density > 0", v.density() > 0);
        check_close("Inject flux: flux.x = 0.5", v.flux.x, 0.5, 1e-12);
    }

    // ---- Inject particle ----
    {
        ftd::RenderBridge bridge(N);
        int cx = 4, cy = 4, cz = 4;
        bridge.inject_particle(cx, cy, cz, 1, {0.0, 0.0, ftd::K_B});

        auto& v = bridge.voxel_at(cx, cy, cz);
        check("Inject particle: state = +1", v.state == 1);
        check("Inject particle: density > 0", v.density() > 0);
        check_close("Inject particle: flux.z = K_B", v.flux.z, ftd::K_B, 1e-12);
    }

    // ---- Diagnostics match actual count ----
    {
        ftd::RenderBridge bridge(N);
        bridge.inject_particle(2, 2, 2, 1, {0.0, 0.0, ftd::K_B});
        bridge.inject_particle(5, 5, 5, -1, {0.0, 0.0, -ftd::K_B});

        auto d = bridge.diagnostics();
        check("Diagnostics: manifested = 2", d.manifested_count == 2);
        check("Diagnostics: positive = 1", d.positive_count == 1);
        check("Diagnostics: negative = 1", d.negative_count == 1);
    }

    // ---- Flux propagation ----
    // Inject flux, run ticks, verify it spreads to neighbors
    {
        ftd::RenderBridge bridge(N);
        int cx = 4, cy = 4, cz = 4;
        bridge.inject_flux(cx, cy, cz, {0.5, 0.0, 0.0});

        // Run a few ticks
        bridge.run(5);

        // Check that at least one face neighbor has nonzero flux
        bool spread = false;
        auto n6 = bridge.lattice().neighbors_6(bridge.lattice().index(cx, cy, cz));
        for (int k = 0; k < 6; ++k) {
            if (bridge.voxels()[n6[k]].density() > 1e-10) {
                spread = true;
                break;
            }
        }
        check("Flux propagation: spread to neighbors after 5 ticks", spread);
    }

    // ---- Sub-threshold flux stays void ----
    {
        ftd::RenderBridge bridge(N);
        // Inject flux well below K_B
        bridge.inject_flux(4, 4, 4, {0.01, 0.0, 0.0});
        bridge.run(10);

        auto d = bridge.diagnostics();
        check("Sub-threshold: no manifestation from weak flux", d.manifested_count == 0);
    }

    // ---- Speed limit enforcement ----
    {
        ftd::RenderBridge bridge(N);
        // Create a particle and set high velocity manually
        bridge.inject_particle(4, 4, 4, 1, {0.0, 0.0, ftd::K_B});
        bridge.voxel_at(4, 4, 4).velocity = {2.0, 0.0, 0.0};  // exceeds C=1

        // Run one tick — bandwidth enforcement should clamp
        bridge.tick();

        // After speed limit enforcement, v^2 + L^2 <= 1 (capped at C_SPEED)
        // The particle may have moved, so check all manifested voxels
        bool speed_ok = true;
        for (int i = 0; i < bridge.lattice().total_sites(); ++i) {
            auto& v = bridge.voxels()[i];
            if (v.state != 0) {
                if (v.bandwidth_used() > 1.0 + 1e-10) {
                    speed_ok = false;
                    break;
                }
            }
        }
        check("Speed limit: bandwidth <= 1 after enforcement", speed_ok);
    }

    // ---- Total flux non-negative ----
    {
        ftd::RenderBridge bridge(N);
        bridge.inject_flux(4, 4, 4, {1.0, 0.0, 0.0});
        bridge.run(20);

        auto d = bridge.diagnostics();
        check("Total flux >= 0 after 20 ticks", d.total_flux >= 0.0);
    }

    // ---- Multiple particles coexist ----
    {
        ftd::RenderBridge bridge(N);
        bridge.inject_particle(2, 2, 2, 1, {0.0, 0.0, ftd::K_B});
        bridge.inject_particle(6, 6, 6, -1, {0.0, 0.0, -ftd::K_B});
        bridge.run(5);

        auto d = bridge.diagnostics();
        check("Multiple particles: manifested >= 1 after 5 ticks", d.manifested_count >= 1);
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All bridge dynamics tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";

    return failures;
}
