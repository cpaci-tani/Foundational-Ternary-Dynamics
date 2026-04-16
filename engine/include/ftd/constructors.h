#pragma once
/**
 * ftd/constructors.h — lattice constructor library
 *
 * Named factory functions that stamp FTD theoretical entities onto a
 * RenderBridge's voxel grid. Every entry point returns a StampResult
 * describing exactly which voxels were modified, so tests and composite
 * constructors can validate and combine results uniformly.
 *
 * Catalog (Levels 0, 1A, and 2):
 *
 *   Constructor           Level  Sites  Theory reference
 *   --------------------  -----  -----  -----------------------------------------
 *   flux                  0      1      ontic.h (flux primitive)
 *   particle              0      1      DERIV_SPIN_STATISTICS_BRIDGE.md
 *   wavepacket            0      N      render_bridge::inject_wavepacket (Phase 6)
 *   entangled_pair        0      2      render_bridge::create_entangled_pair
 *   octahedron            1A     6      THEOREM_MOORE_LAYER_DECOMPOSITION §shell 1
 *   cuboctahedron         1A     12     THEOREM_MOORE_LAYER_DECOMPOSITION §shell 2
 *   stella_octangula      1A     8      THEOREM_MOORE_LAYER_DECOMPOSITION §shell 3
 *                                       + DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md
 *   moore_cell            1A     26     THEOREM_MOORE_LAYER_DECOMPOSITION
 *   plane_wave            2      N³     EM wave (flux + wave_vel propagating)
 *   standing_wave         2      N³     Counter-propagating superposition
 *   uniform_e             2      N³     Constant electric field (wave_vel = -E)
 *   uniform_b             2      N³     Constant magnetic field (∇×J = B)
 *   photon_pulse          2      ~σ³    Gaussian-enveloped plane wave
 *   electric_dipole       2      N³     ±1 charges + Coulomb dressing
 *   magnetic_dipole       2      ~R     Current-loop analog
 *   vortex_line           2      N³     Azimuthal flux vortex
 *
 * Design spec: docs/superpowers/specs/2026-04-15-ftd-constructors-design.md
 */

#include "lattice.h"    // Coord
#include "voxel.h"      // Vec3
#include "constants.h"  // K_B, GAUSSIAN_CUTOFF_SIGMA

#include <cstdint>
#include <vector>

namespace ftd {

class RenderBridge;  // forward declaration

namespace ctor {

struct StampResult {
    const char*      name;
    int              level;
    Coord            center;
    std::vector<int> sites;

    int site_count() const { return static_cast<int>(sites.size()); }
};

// Level 0 — primitive wrappers
StampResult flux(RenderBridge& rb, Coord at, Vec3 J);

StampResult particle(RenderBridge& rb,
                     Coord  at,
                     int8_t state,
                     Vec3   J,
                     int8_t spin  = 0,
                     int8_t color = 0);

StampResult wavepacket(RenderBridge& rb,
                       Coord  at,
                       int8_t state,
                       double sigma = 3.0,
                       double amp   = K_B);

StampResult entangled_pair(RenderBridge& rb, Coord at, Vec3 J);

// Level 1A — Moore polyhedral seeds (state-only; flux left zero)
StampResult octahedron(RenderBridge& rb, Coord center, int8_t state);
StampResult cuboctahedron(RenderBridge& rb, Coord center, int8_t state);
StampResult stella_octangula(RenderBridge& rb, Coord center, int8_t state);
StampResult moore_cell(RenderBridge& rb, Coord center, int8_t state);

// Level 2 — field configurations (stamp flux and/or wave_vel)
StampResult plane_wave(RenderBridge& rb,
                       Vec3 direction,
                       Vec3 polarization,
                       double wavelength,
                       double amplitude);

StampResult standing_wave(RenderBridge& rb,
                          Vec3 direction,
                          Vec3 polarization,
                          double wavelength,
                          double amplitude);

StampResult uniform_e(RenderBridge& rb, Vec3 E);

StampResult uniform_b(RenderBridge& rb, Vec3 B);

StampResult photon_pulse(RenderBridge& rb,
                         Coord center,
                         Vec3 direction,
                         Vec3 polarization,
                         double sigma,
                         double amplitude);

StampResult electric_dipole(RenderBridge& rb,
                            Coord center,
                            Vec3 axis,
                            int separation);

StampResult magnetic_dipole(RenderBridge& rb,
                            Coord center,
                            Vec3 moment,
                            int radius,
                            double amplitude);

StampResult vortex_line(RenderBridge& rb,
                        Coord center,
                        Vec3 axis,
                        double circulation);

}  // namespace ctor
}  // namespace ftd
