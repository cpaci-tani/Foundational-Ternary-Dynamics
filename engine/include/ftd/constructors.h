#pragma once
/**
 * ftd/constructors.h — lattice constructor library
 *
 * Named factory functions that stamp FTD theoretical entities onto a
 * RenderBridge's voxel grid. Every entry point returns a StampResult
 * describing exactly which voxels were modified, so tests and composite
 * constructors can validate and combine results uniformly.
 *
 * Catalog (first slice — Levels 0 and 1A):
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

}  // namespace ctor
}  // namespace ftd
