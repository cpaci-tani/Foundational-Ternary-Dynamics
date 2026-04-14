// ============================================================================
// FieldLines.h — RK4 vector-field line integrator (stub)
// ============================================================================
//
// Phase 4 placeholder. The full port of engine/web/js/fieldlines.js
// (which integrates streamlines through the flux vector field with RK4)
// is deferred until the Phase 2a telemetry protocol starts carrying
// vector-field snapshots. As of the Phase 4 implementation the engine
// only emits `snapshot` events carrying int8 voxel states — no J_x/J_y/J_z
// components — so there is nothing for the integrator to consume yet.
//
// This header/cpp pair exists so that:
//   * the CMake build registers the source file and the class name,
//   * future phases can fill in `integrate()` without touching CMake or
//     the Phase 4 commit boundary.
//
// TODO (post-Phase 4): port RK4 + seeding logic from
// engine/web/js/fieldlines.js. Expected input: flat float32 array of length
// 3 * Ls^3 with layout [Jx, Jy, Jz, Jx, ...]. Expected output: list of
// poly-line vertex lists suitable for the LatticeViewer `lineProgram`.
// ----------------------------------------------------------------------------

#pragma once

#include <vector>

#include <QVector3D>

namespace ftd::testrunner {

// Parameters for a single RK4 streamline integration. Kept public so the
// future callers (tests, LatticeViewer controls) can populate it.
struct FieldLineParams {
    int    maxSteps  = 200;     // total RK4 steps per streamline
    float  stepSize  = 0.05f;   // world-space step length
    float  minSpeed  = 1e-6f;   // terminate when ||J|| drops below this
};

// Opaque field sampler contract: given a position in normalized [-1,1]^3
// coordinates, write the three components (Jx, Jy, Jz) into `out`. The
// future implementation will take a std::function or a templated callable;
// for now it's just here as a forward-contract doc comment.

// Integrate a single streamline starting at `seed` using RK4 with the
// supplied parameters. Returns the list of sample points along the line.
//
// Phase 4 stub: unconditionally returns an empty vector. The engine does
// not emit vector-field telemetry yet, so there is nothing to integrate.
std::vector<QVector3D> integrateFieldLine(const QVector3D& seed,
                                          const FieldLineParams& params);

}  // namespace ftd::testrunner
