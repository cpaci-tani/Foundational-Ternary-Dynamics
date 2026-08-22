#pragma once

// Environment backgrounds for the Scale-0 viewport (native parity with the web
// dashboard's background selector). Each theme is a large procedural point cloud
// centred on the lattice ((N/2)^3) at a radius that scales with the lattice, so
// it surrounds the scene. Emitted into NativeFrame::background_points and drawn
// BEHIND everything through the additive depth-off heat sprite PSO. Port of
// engine/web/js/backgrounds/*.js. Deterministic per index (seeded hash PRNG) so
// captures are reproducible; animated by a wall-clock time (twinkle / swirl).

#include <vector>

#include "native/native_frame.h"

namespace ftd::native {

// Theme ids match the web registry order (used by SetBackground + the selector).
// Beyond (the fading grid) is line geometry — added in a later phase.
enum class BackgroundTheme : int {
    None   = 0,
    Stars  = 1,
    Nebula = 2,
    Foam   = 3,
    Beyond = 4,
    Storm  = 5,
    Count  = 6,
};

// Fill `points` + `lines` (both cleared first) with the procedural background for
// `theme`, sized to a lattice of `lattice_size` voxels and animated by `time_sec`
// (twinkle for stars/foam, swirl for storm, drift for nebula). Most themes emit
// only points; Beyond emits a fading grid into `lines` plus flickering void
// points. None / out-of-range yield nothing.
void build_background(int theme, double time_sec, int lattice_size,
                      std::vector<NativeParticle>& points, std::vector<NativeLine>& lines);

const char* background_theme_name(int theme);
int background_theme_from_name(const char* name);

}  // namespace ftd::native
