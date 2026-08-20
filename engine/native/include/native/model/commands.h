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
    InspectVoxel, InspectForce, RequestField, RequestContinuity, RequestChargeSum>;

// std::monostate is the "this is a core command" sentinel. Scale1Cmd, Scale2Cmd,
// … slot in here as they arrive — one added alternative each, no schema change.
using ScalePayload = std::variant<std::monostate, Scale0Cmd>;

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

}  // namespace ftd::native
