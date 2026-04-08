#pragma once
/**
 * ScaleEngine: Abstract base class for all FTD per-scale simulation engines.
 *
 * WHY VIRTUAL (not CRTP):
 *   The web dashboard switches between Scale 0/1/2/5 at runtime via a single
 *   bridge pointer. Runtime polymorphism (vtable dispatch) is the natural fit.
 *   CRTP would require compile-time type knowledge at the call site, which is
 *   incompatible with dynamic scale switching. The vtable overhead is one
 *   indirect call per tick — negligible compared to the millions of FLOPS
 *   inside each tick's force computation.
 *
 * WHY run() HAS A DEFAULT IMPLEMENTATION:
 *   Every scale's run(N) is identical: call tick() N times. Providing a default
 *   avoids duplicating this trivial loop in every derived class. A scale that
 *   needs batching or progress callbacks can override it.
 *
 * WHY base_diagnostics() RETURNS A SUBSET:
 *   Each scale has specific diagnostics (e.g., ParticleDiagnostics has angular
 *   momentum, CosmicDiagnostics has Hubble parameter). The bridge layer needs
 *   a uniform way to query {tick, entity_count, energy, momentum} without
 *   knowing which scale is active. ScaleBaseDiagnostics provides that common
 *   subset. Scales still expose their full diagnostics through their own
 *   typed accessor (e.g., ParticleEngine::diagnostics()).
 */

#include <string>
#include "voxel.h"  // Vec3

namespace ftd {

// ============================================================================
// Common diagnostics subset shared across all scales.
//
// Each scale adds specific fields via its own Diagnostics struct, but this
// gives the bridge a uniform way to query basic stats without downcasting.
// ============================================================================

struct ScaleBaseDiagnostics {
    int tick = 0;
    int entity_count = 0;     // particles, atoms, bodies — whatever the scale has
    double total_energy = 0.0;
    double total_ke = 0.0;
    double total_pe = 0.0;
    Vec3 total_momentum;
};

// ============================================================================
// Abstract base for all per-scale simulation engines.
//
// Enables:
//   - Polymorphic WASM dispatch (bridge holds ScaleEngine* for current scale)
//   - Unified toggle registry (get/set by string name across all scales)
//   - Common diagnostics query without knowing which scale is active
// ============================================================================

class ScaleEngine {
public:
    virtual ~ScaleEngine() = default;

    // --- Simulation lifecycle ---

    // Advance simulation by one discrete time step.
    virtual void tick() = 0;

    // Advance simulation by num_ticks steps.
    // Default: simple loop over tick(). Override for batching/progress hooks.
    virtual void run(int num_ticks) {
        for (int i = 0; i < num_ticks; i++) tick();
    }

    // Current discrete time step index.
    virtual int current_tick() const = 0;

    // --- Time step control ---

    virtual double dt() const = 0;
    virtual void set_dt(double d) = 0;

    // --- Toggle access by name (for unified registry) ---
    // Enables the dashboard to enumerate and flip toggles without knowing
    // the concrete engine type. Returns false / does nothing for unknown names.

    virtual bool get_toggle(const std::string& name) const = 0;
    virtual void set_toggle(const std::string& name, bool value) = 0;

    // --- Entity count ---
    // Returns the number of simulation entities: particles (Scale 1),
    // atoms (Scale 2), cosmic bodies (Scale 5), etc.

    virtual int entity_count() const = 0;

    // --- Base diagnostics ---
    // Common across all scales. Each scale implements this by extracting
    // the relevant fields from its own typed diagnostics struct.

    virtual ScaleBaseDiagnostics base_diagnostics() const = 0;

    // --- Reset ---
    // Clear all entities and reset tick counter to zero.

    virtual void clear() = 0;

    // --- Scale identification ---
    // Used by the bridge to know which scale is active without RTTI.

    virtual int scale_level() const = 0;
    virtual const char* scale_name() const = 0;
};

}  // namespace ftd
