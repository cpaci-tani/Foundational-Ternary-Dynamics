#pragma once
//
// model/commands.h — the scale-generic command vocabulary (SPEC_NATIVE_REBUILD_R0R1 §4.2).
//
// Composition, not a mega-variant: a ScaleCommand is EITHER a scale-common core
// command (the host handles it directly: loop control, reload, scale switch,
// telemetry demand) OR a scale-namespaced payload (the owning adapter handles
// it). Adding a scale adds one payload alternative; it never touches the core.
//
// The Scale-0 payload REUSES the proven command structs from native/ui_command.h
// verbatim, so the Scale-0 adapter can dispatch them through the existing,
// golden-covered apply_mutation_on_bridge()/observe_on_bridge() logic with no
// behavioral change. ui_command.h stays the single definition site for those
// structs; this header only groups them.
//
#include "native/host/run_config.h"
#include "native/ui_command.h"   // Pause/Run/Step/SetToggle/… struct definitions

#include <string>
#include <variant>

namespace ftd::native {

// ── scale-common core (ScaleHost owns these) ─────────────────────────────────
// Pause/Run/Step/LoadScenario/SetTelemetryDemand are reused from ui_command.h.
struct SetRunConfig { RunConfig cfg; };
struct SwitchScale  { int scale_level = 0; std::string scenario; };

using CoreCommand = std::variant<Pause, Run, Step, LoadScenario, SetRunConfig,
                                 SwitchScale, SetTelemetryDemand>;

// ── per-scale payloads (the owning adapter handles these) ────────────────────
// Scale 0 keeps the current SPEC_UI_V2 §3.4 vocabulary verbatim. SetLatticeSize
// and ApplyReboot are intentionally NOT here: lattice resize is a reload knob
// carried by the core SetRunConfig, so a scale never re-implements host reload.
using Scale0Cmd = std::variant<
    SetToggle, SetToggleProfile, SetDouble, SetEnum, SetUInt, SetBoolConfig,
    SetBoundary, SetDt, SetSorIterations, ResetToDefaults,
    InjectWavepacket, InjectFluxAdd, CreateEntangledPair, ClearField, SeedRandomFlux,
    InspectVoxel, InspectForce, RequestField, SetFieldOverlay, RequestContinuity,
    RequestChargeSum>;

// ── Scale 1 (ParticleEngine) payload ─────────────────────────────────────────
// Minimal-but-real vocabulary that proves the seam carries a second,
// structurally different scale's commands (R1 validation): (re)seed a named
// particle scenario, or drop one charged particle at a position. The full
// Scale-1 vocabulary (per-force toggles, decay/scattering observables) lands in
// later steps — this is exactly enough to seed/run and to exercise apply().
struct Seed1        { std::string scenario; };
struct AddParticle1 { int charge = 1; float x = 0.0f; float y = 0.0f; float z = 0.0f; };
// Read-only Scale-1 observation (the click-to-inspect analog for a scale with no
// voxel field): select the particle at `index` in the engine's particle list and
// publish its charge / position / velocity into the Scale1Snapshot inspection
// payload. index < 0 clears the selection. Routed as an observation (never a
// mutation), so it flows through the adapter's observe() path, not apply().
struct InspectParticle1 { int index = -1; };
using Scale1Cmd = std::variant<Seed1, AddParticle1, InspectParticle1>;

// std::monostate is the "this is a core command" sentinel. Scale2Cmd, Scale5Cmd,
// … slot in here as they arrive — one added alternative each, no schema change.
using ScalePayload = std::variant<std::monostate, Scale0Cmd, Scale1Cmd>;

struct ScaleCommand {
    CoreCommand  core{Pause{}};
    ScalePayload scale{std::monostate{}};

    // A command is a core command exactly when it carries no scale payload.
    bool is_core() const { return std::holds_alternative<std::monostate>(scale); }
};

// ── ergonomic constructors (call sites stay readable) ────────────────────────
inline ScaleCommand core_command(CoreCommand c) {
    return ScaleCommand{std::move(c), std::monostate{}};
}
inline ScaleCommand scale0_command(Scale0Cmd c) {
    return ScaleCommand{Pause{}, ScalePayload{std::move(c)}};
}
inline ScaleCommand scale1_command(Scale1Cmd c) {
    return ScaleCommand{Pause{}, ScalePayload{std::move(c)}};
}

}  // namespace ftd::native
