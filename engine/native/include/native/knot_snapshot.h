#pragma once

#include <vector>

namespace ftd::native {

// One alive knot, projected from ftd::KnotRow for the UI (Knots panel). A "knot"
// here is the engine C++ KnotTracker's definition: a connected component of
// same-sign manifested voxels (s=±1), tracked across ticks with a persistent id
// — NOT the web panel's field-line tangles.
struct KnotRowUi {
    int id = 0;
    int sign = 0;       // ±1 charge sign
    int age = 0;        // ticks since birth
    int size = 0;       // voxels this tick
    float flux = 0.0f;  // |Σ J| over the knot
    float org = 0.0f;   // organization proxy N·coherence
};

// Per-boundary knot telemetry snapshot: the aggregate lifecycle counts + the
// top knots (by size). Filled by the adapter from bridge.knot_tracker() when
// DataNeeds::knots is set.
struct KnotSnapshot {
    int alive = 0;
    int net_charge = 0;
    int births = 0;
    int deaths = 0;
    int fissions = 0;
    int fusions = 0;
    std::vector<KnotRowUi> knots;   // sorted by size desc, capped
    bool ok = false;
    bool blocked = false;   // GPU + L>64: knot_tracking would throw (W6), so not run
};

}  // namespace ftd::native
