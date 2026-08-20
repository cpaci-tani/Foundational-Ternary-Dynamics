#pragma once
//
// scale0_overlays.h — the data-driven Scale-0 overlay registry.
//
// The web dashboard composites MANY Scale-0 field/phenomena overlays at once,
// grouped into 7 menu columns (Volume · Fields · Forces · Quantum · Topology ·
// Stress-Energy · Phenomena). This registry is the native mirror of that menu:
// one immutable descriptor per overlay, keyed by a stable OverlayId, carrying
// everything both consumers need —
//   • the Scale-0 adapter's capture(): how to turn the overlay into scene
//     geometry (which VisualFieldKind to sample, points-vs-arrows, colour ramp,
//     threshold, per-overlay quirks like ×DUAL_DELTA or a latency-horizon cut);
//   • the app's FIELDS panel + CLI: the stable name, human label, and column.
//
// Adding an overlay in a later tranche is a single row here (plus its ramp/logic
// in the adapter if genuinely new) — the panel groups by `column` automatically
// and the CLI resolves it by `name`, so no panel/CLI wiring changes are needed.
//
// This header names only ftd::VisualFieldKind (a visualization enum, freely
// named across the native UI). It does NOT name RenderBridge/TermToggles/any
// Scale-0 command or snapshot struct, so it stays outside the adapter's
// concrete-type boundary and can be shared by the app layer.
//
#include "ftd/visual_field_sample.h"

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace ftd::native {

// The 7 web overlay-menu columns, in menu order. `Count` bounds iteration.
enum class OverlayColumn : std::uint32_t {
    Volume = 0,
    Fields,
    Forces,
    Quantum,
    Topology,
    StressEnergy,
    Phenomena,
    Count,
};

// How an overlay's samples become scene geometry through the presenter's two
// existing primitives (sprite point cloud + line list).
enum class OverlayRender : std::uint32_t {
    Sprite,      // ambient |J| flux cloud (FluxVector magnitude) → sprite points
    Points,      // scalar field → magnitude/sign-coloured sprite points
    Arrows,      // 3-vector field → line-segment arrows (dim base → bright tip)
    Streamline,  // 3-vector field → RK4-traced field lines (LINE PSO polylines)
};

// Colour ramp applied to the per-sample magnitude/sign.
enum class OverlayRamp : std::uint32_t {
    FluxCloud,   // ambient flux cloud (cool blue, brightening with |J|)
    Diverging,   // signed red(+)/blue(-) — divergence, Gauss residual
    StateSign,   // s=+1 red / s=-1 blue, saturated (void invisible)
    Latency,     // blue(low) → red(high)
    Horizon,     // dim ember red (dark, threshold-selected shell)
    CoolHot,     // magnitude cool→hot (force arrows)
    Poynting,    // yellow → orange (energy-flux arrows)
    Weak,        // violet (parity-odd ∇×J pseudovector arrows)
    // Streamline ramps: applied per-vertex inside the RK4 integrator (streamlines
    // .cpp), keyed off the overlay, not consulted by the point/arrow colour
    // switches. Named here so each streamline descriptor documents its own look.
    FluxLine,    // Flux Lines: flux colormap by LOCAL |J| (dark-blue→cyan→red)
    CyanFade,    // E Field: cyan (0.30,0.82,0.88) faded along the line
    GreenFade,   // B Field: green (0.40,0.73,0.42) faded along the line
};

// Stable overlay identifiers. The numeric value is the wire id carried by the
// SetOverlay command; NEVER renumber an existing entry (later tranches append).
enum class OverlayId : std::uint32_t {
    // ── Tranche 1: the 11 COVERED overlays ──
    FluxVolume = 0,
    Divergence,
    State,
    Poynting,
    EmForce,
    GravityForce,
    StrongForce,
    WeakCurl,
    Latency,
    GaussResidual,
    Horizon,
    // ── Tranche 4: the 3 STREAMLINE overlays (RK4 field-line integration) ──
    FluxLines,
    EField,
    BField,
    // Future tranches append here (EXTEND / NEW overlays) — do not reorder.
};

struct OverlayDescriptor {
    OverlayId            id;
    const char*          name;    // stable id (panel + --overlays CLI)
    const char*          label;   // human label shown in the panel
    OverlayColumn        column;
    OverlayRender        render;
    ftd::VisualFieldKind kind;    // field sampled from the bridge
    OverlayRamp          ramp;
    // Keep a sample when its magnitude fraction of the frame max ≥ threshold,
    // UNLESS select_min ≥ 0, in which case keep it when the RAW value ≥
    // select_min (absolute — used by the Horizon L≥0.95 shell).
    float                threshold;
    float                select_min;
    bool                 scale_by_dual_delta;  // ∇×J "weak": ×DUAL_DELTA (canonical)
    bool                 force_stride1;         // State / Gauss sample every voxel
};

// The registry, in menu/column order. Defined once here (header-inline) so both
// the adapter and the app see the same table without a link dependency.
inline constexpr OverlayDescriptor kOverlayRegistry[] = {
    // ── Volume ──
    {OverlayId::FluxVolume,   "fluxVolume", "Flux Volume", OverlayColumn::Volume,
     OverlayRender::Sprite, ftd::VisualFieldKind::FluxVector,   OverlayRamp::FluxCloud, 0.04f, -1.0f, false, false},
    {OverlayId::FluxLines,    "fluxLines",  "Flux Lines",  OverlayColumn::Volume,
     OverlayRender::Streamline, ftd::VisualFieldKind::FluxVector, OverlayRamp::FluxLine, 0.0f, -1.0f, false, true},
    {OverlayId::Divergence,   "divJ",       "\xE2\x88\x87\xC2\xB7J", OverlayColumn::Volume,
     OverlayRender::Points, ftd::VisualFieldKind::Divergence,   OverlayRamp::Diverging, 0.01f, -1.0f, false, false},
    {OverlayId::State,        "state",      "State s",     OverlayColumn::Volume,
     OverlayRender::Points, ftd::VisualFieldKind::State,        OverlayRamp::StateSign, 0.50f, -1.0f, false, true},
    // ── Fields ──
    {OverlayId::EField,       "eField",     "E Field",     OverlayColumn::Fields,
     OverlayRender::Streamline, ftd::VisualFieldKind::Electric, OverlayRamp::CyanFade,  0.0f, -1.0f, false, true},
    {OverlayId::BField,       "bField",     "B Field",     OverlayColumn::Fields,
     OverlayRender::Streamline, ftd::VisualFieldKind::Magnetic, OverlayRamp::GreenFade, 0.0f, -1.0f, false, true},
    {OverlayId::Poynting,     "poynting",   "Poynting S",  OverlayColumn::Fields,
     OverlayRender::Arrows, ftd::VisualFieldKind::Poynting,     OverlayRamp::Poynting,  0.05f, -1.0f, false, false},
    // ── Forces ──
    {OverlayId::EmForce,      "emForce",    "EM",          OverlayColumn::Forces,
     OverlayRender::Arrows, ftd::VisualFieldKind::EmForce,      OverlayRamp::CoolHot,   0.04f, -1.0f, false, false},
    {OverlayId::GravityForce, "gravityForce","Gravity",    OverlayColumn::Forces,
     OverlayRender::Arrows, ftd::VisualFieldKind::GravityForce, OverlayRamp::CoolHot,   0.04f, -1.0f, false, false},
    {OverlayId::StrongForce,  "strongForce","Strong",      OverlayColumn::Forces,
     OverlayRender::Arrows, ftd::VisualFieldKind::StrongForce,  OverlayRamp::CoolHot,   0.04f, -1.0f, false, false},
    {OverlayId::WeakCurl,     "weakCurl",   "\xE2\x88\x87\xC3\x97J", OverlayColumn::Forces,
     OverlayRender::Arrows, ftd::VisualFieldKind::Curl,         OverlayRamp::Weak,      0.08f, -1.0f, true,  false},
    // ── Topology ──
    {OverlayId::Latency,      "latency",    "Latency L",   OverlayColumn::Topology,
     OverlayRender::Points, ftd::VisualFieldKind::Latency,      OverlayRamp::Latency,   0.02f, -1.0f, false, false},
    {OverlayId::GaussResidual,"gaussResidual","Gauss resid.", OverlayColumn::Topology,
     OverlayRender::Points, ftd::VisualFieldKind::GaussResidual,OverlayRamp::Diverging, 0.05f, -1.0f, false, true},
    // ── Phenomena ──
    {OverlayId::Horizon,      "horizon",    "Horizon",     OverlayColumn::Phenomena,
     OverlayRender::Points, ftd::VisualFieldKind::Latency,      OverlayRamp::Horizon,   0.02f, 0.95f, false, false},
};

inline constexpr std::size_t kOverlayCount =
    sizeof(kOverlayRegistry) / sizeof(kOverlayRegistry[0]);

// Human title for a column (menu order). Empty columns are simply not rendered.
inline const char* overlay_column_title(OverlayColumn c) {
    switch (c) {
        case OverlayColumn::Volume:       return "Volume";
        case OverlayColumn::Fields:       return "Fields";
        case OverlayColumn::Forces:       return "Forces";
        case OverlayColumn::Quantum:      return "Quantum";
        case OverlayColumn::Topology:     return "Topology";
        case OverlayColumn::StressEnergy: return "Stress-Energy";
        case OverlayColumn::Phenomena:    return "Phenomena";
        default:                          return "";
    }
}

inline const OverlayDescriptor* overlay_by_id(std::uint32_t id) {
    for (const OverlayDescriptor& d : kOverlayRegistry)
        if (static_cast<std::uint32_t>(d.id) == id) return &d;
    return nullptr;
}

inline const OverlayDescriptor* overlay_by_name(std::string_view name) {
    for (const OverlayDescriptor& d : kOverlayRegistry)
        if (name == d.name) return &d;
    return nullptr;
}

}  // namespace ftd::native
