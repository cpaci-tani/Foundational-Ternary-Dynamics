#pragma once

#include "native/ui_demand.h"

#include "ftd/term_toggles.h"
#include "ftd/visual_field_sample.h"

#include <string>
#include <variant>

namespace ftd::native {

enum class DoubleKey {
    langevin_T,
    langevin_gamma,
    coulomb_charge_coupling,
    coulomb_source_scale,
    omega0,
    kinetic_drain,
    genesis_threshold_override,
    manifest_scale_override
};

enum class EnumKey { bcc_stencil, langevin_site_filter };
enum class UIntKey { langevin_seed };
enum class BoolCfgKey { manifest_use_temperature };

struct SetToggle {
    std::string name;
    bool value = false;
};
struct SetToggleProfile {
    ftd::TermToggles profile;
};
struct SetDouble {
    DoubleKey key = DoubleKey::langevin_T;
    double value = 0.0;
};
struct SetEnum {
    EnumKey key = EnumKey::bcc_stencil;
    int value = 0;
};
struct SetUInt {
    UIntKey key = UIntKey::langevin_seed;
    unsigned value = 0;
};
struct SetBoolConfig {
    BoolCfgKey key = BoolCfgKey::manifest_use_temperature;
    bool value = false;
};
struct SetBoundary {
    ftd::FluxBoundaryMode mode = ftd::FluxBoundaryMode::Periodic;
};
struct SetDt {
    double dt = 1.0;
};
struct SetSorIterations {
    int n = 1;
};
struct LoadScenario {
    std::string id;
};
struct SetLatticeSize {
    int n = 0;
};
struct ApplyReboot {};
struct ResetToDefaults {};
struct InspectVoxel {
    int x = 0;
    int y = 0;
    int z = 0;
};
struct InspectForce {
    int x = 0;
    int y = 0;
    int z = 0;
};
struct RequestField {
    ftd::VisualFieldKind kind = ftd::VisualFieldKind::FluxVector;
    int stride = 1;
};
// Toggles ONE overlay (by its stable OverlayId, see scale0_overlays.h) in the
// Scale-0 adapter's active-overlay SET. `on` adds it, false removes it. The
// adapter composites every active overlay into the scene each capture(); when
// the set is empty the ambient flux cloud shows. This replaces the old
// single-select SetFieldOverlay — the native side now mirrors the web, which
// composites all active overlays at once. Adapter view-state only; it never
// touches the RenderBridge, so apply() handles it without a bridge mutation.
struct SetOverlay {
    std::uint32_t overlay_id = 0;  // OverlayId
    bool on = false;
};
struct RequestContinuity {};
struct RequestChargeSum {};
struct SetTelemetryDemand {
    DataNeeds needs;
};
struct Pause {};
struct Step {
    int ticks = 1;
};
struct Run {};
struct InjectWavepacket {
    int x = 0;
    int y = 0;
    int z = 0;
    int state = 1;
};
struct InjectFluxAdd {
    int x = 0;
    int y = 0;
    int z = 0;
    double fx = 0.0;
    double fy = 0.0;
    double fz = 0.0;
};
struct CreateEntangledPair {
    int x = 0;
    int y = 0;
    int z = 0;
    double fx = 0.0;
    double fy = 0.0;
    double fz = 0.0;
};
struct ClearField {};
struct SeedRandomFlux {};

using UiCommand = std::variant<
    SetToggle, SetToggleProfile, SetDouble, SetEnum, SetUInt, SetBoolConfig,
    SetBoundary, SetDt, SetSorIterations, LoadScenario, SetLatticeSize,
    ApplyReboot, ResetToDefaults, InspectVoxel, InspectForce, RequestField,
    SetOverlay, RequestContinuity, RequestChargeSum, SetTelemetryDemand,
    Pause, Step, Run, InjectWavepacket, InjectFluxAdd, CreateEntangledPair,
    ClearField, SeedRandomFlux>;

inline bool is_observation_command(const UiCommand& command) {
    return std::holds_alternative<InspectVoxel>(command)
        || std::holds_alternative<InspectForce>(command)
        || std::holds_alternative<RequestField>(command)
        || std::holds_alternative<RequestContinuity>(command)
        || std::holds_alternative<RequestChargeSum>(command)
        || std::holds_alternative<SetTelemetryDemand>(command);
}

inline bool is_loop_control_command(const UiCommand& command) {
    return std::holds_alternative<Pause>(command)
        || std::holds_alternative<Step>(command)
        || std::holds_alternative<Run>(command);
}

inline bool is_harness_command(const UiCommand& command) {
    return std::holds_alternative<InjectWavepacket>(command)
        || std::holds_alternative<InjectFluxAdd>(command)
        || std::holds_alternative<CreateEntangledPair>(command)
        || std::holds_alternative<ClearField>(command)
        || std::holds_alternative<SeedRandomFlux>(command);
}

inline bool is_coalescible_command(const UiCommand& command) {
    return std::holds_alternative<RequestField>(command)
        || std::holds_alternative<SetTelemetryDemand>(command)
        || std::holds_alternative<InspectVoxel>(command)
        || std::holds_alternative<InspectForce>(command);
}

}  // namespace ftd::native
