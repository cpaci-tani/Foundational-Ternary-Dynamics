#pragma once

// Boundary-shape wireframes for the Scale-0 viewport (native parity with the web
// dashboard's boundary-select). Each shape is emitted as coloured NativeLine
// edges centred on the lattice ((N/2)^3) with half-extent N/2, so it coincides
// with the legacy wireframe cube. Drawn through the presenter's existing LINE
// pipeline via NativeFrame::boundary_lines — no shader/PSO work. Port of
// engine/web/js/viewport/boundary-geometry.js.

#include <vector>

#include "native/model/draw_list.h"   // ftd::native::BoundaryShape (shared enum)
#include "native/native_frame.h"

namespace ftd::native {

// BoundaryShape (Cube/Sphere/Dodecahedron/Icosahedron/Octahedron/Cylinder/Torus/
// None) is declared in native/model/draw_list.h. Ids match the web selector order.

// Emit the wireframe for `shape` into `out` (cleared first), sized to a lattice
// of `lattice_size` voxels. `shape` is a BoundaryShape int; out-of-range or None
// yields no lines. Deterministic (no RNG), so captures are reproducible.
void build_boundary_lines(int shape, int lattice_size, std::vector<NativeLine>& out);

// Short lowercase id for a shape ("cube", "sphere", …, "none"); "" if invalid.
// Used by the CLI --boundary flag and the UI label.
const char* boundary_shape_name(int shape);

// Parse a lowercase id back to a BoundaryShape int, or -1 if unknown.
int boundary_shape_from_name(const char* name);

}  // namespace ftd::native
