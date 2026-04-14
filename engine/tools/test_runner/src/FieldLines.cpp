// ============================================================================
// FieldLines.cpp — RK4 vector-field line integrator (stub)
// ============================================================================
//
// Phase 4 ships this as a stub. See FieldLines.h for rationale.
//
// TODO (post-Phase 4): port the RK4 sampler + adaptive step logic from
// engine/web/js/fieldlines.js. Depends on the snapshot protocol growing
// a companion event carrying the flux vector field (Jx, Jy, Jz) alongside
// the existing int8 state array.
// ----------------------------------------------------------------------------

#include "FieldLines.h"

namespace ftd::testrunner {

std::vector<QVector3D> integrateFieldLine(const QVector3D& /*seed*/,
                                          const FieldLineParams& /*params*/) {
    // Deliberately empty. Tests only emit voxel state snapshots in the
    // current Phase 2a NDJSON protocol — no vector field to sample yet.
    return {};
}

}  // namespace ftd::testrunner
