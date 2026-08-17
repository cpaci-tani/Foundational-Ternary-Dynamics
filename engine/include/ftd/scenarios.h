#pragma once
// ==========================================================================
//  engine/include/ftd/scenarios.h
//
//  C++ port of the Scale-0 scenario library that was previously JS-only on
//  the MockBridge (engine/web/js/bridge/scenarios/*.js).
//
//  BEFORE this port, WasmBridge.setupScenario(name) dispatched to
//  ftd_wasm.cpp::setup_scenario which only knew 35 of the then-83 UI-exposed
//  scenarios. The other 48 silently no-op'd on WASM — users with the
//  native/WASM backend (the default fast path) would click `s0-seed-hydrogen`
//  and get an empty lattice with no error.
//
//  AFTER this port, every UI scenario has a C++ implementation.  The catalog
//  has since grown to 130 public Scale-0 scenarios; scale0_scenario_ids() is
//  the native registry audited by the scenario wiring test.
//
//  Function layout mirrors the JS group files 1-for-1:
//    setup_flux_scenario      ← js/bridge/scenarios/flux-scenarios.js
//    setup_light_scenario     ← js/bridge/scenarios/light-scenarios.js
//    setup_quantum_scenario   ← js/bridge/scenarios/quantum-scenarios.js
//    setup_s0_seed_scenario   ← js/bridge/scenarios/s0-seed-scenarios.js
//    setup_s0_field_scenario  ← js/bridge/scenarios/s0-field-scenarios.js
//    dispatch_scenario        ← js/bridge/scenarios/index.js
//
//  Each group function returns true only when it executed an exact scenario
//  body, false otherwise. Prefixes select a group but never count as success.
//
//  PRIMITIVES USED (all live on RenderBridge):
//    rb.inject_flux_add(x,y,z,Vec3)      — JS _injectFlux (additive)
//    rb.inject_wave_vel_add(x,y,z,Vec3)  — JS _injectWaveVel (additive)
//    detail::IP/IPF                      — staged particle marker primitives
//    rb.inject_wavepacket(x,y,z,state)   — JS injectWavepacket
//    rb.create_entangled_pair(...)       — JS createEntangledPair
// ==========================================================================

#include <string>
#include <string_view>
#include <vector>

namespace ftd {

class RenderBridge;

// ── Scenario dispatcher ────────────────────────────────────────────────
// Call this from WasmBridge / WS bridge / tests; it fans out to the
// group-specific functions below and returns true iff name was handled.
// The caller is responsible for rb.reset() BEFORE calling this (matches
// the JS runSetupScenario contract).
//
// Toggle contract:
//   • Isolation profiles (configure_*_terms / FREE_WAVE_DISABLED_TERMS) MAY
//     zero the full TermToggles / TOGGLE_SPECS registry, including research
//     keys (pair_production, langevin, latency_field, emergent_forces, …).
//     That is intentional: an IC that promises an isolated map must not inherit
//     a prior research run.
//   • The dashboard whitelist SCALE0_TOGGLES is only the UI-visible subset.
//     The JS loader resets those keys between loads; research keys outside the
//     whitelist persist across ordinary loads unless a configure_* helper
//     clears them. Scenario-specific research pins live in
//     SCALE0_SCENARIO_RESEARCH_TERMS (toggles.js).
//   • See engine/web/js/scales/scale0/runtime/scenario-loader.js.
bool dispatch_scenario(RenderBridge& rb, const std::string& name);

// Canonical native application registry. dispatch_scenario rejects names not
// present here even when they share a recognized prefix, preventing the old
// "accepted but blank" failure mode from returning.
const std::vector<std::string_view>& scale0_scenario_ids();

// ── Group functions ────────────────────────────────────────────────────
// Each returns true iff a concrete scenario body was executed. A recognized
// prefix with an unknown suffix returns false.
bool setup_flux_scenario    (RenderBridge& rb, const std::string& name);
bool setup_light_scenario   (RenderBridge& rb, const std::string& name);
bool setup_quantum_scenario (RenderBridge& rb, const std::string& name);
bool setup_s0_seed_scenario (RenderBridge& rb, const std::string& name);
bool setup_s0_field_scenario(RenderBridge& rb, const std::string& name);
bool setup_vacuum_scenario  (RenderBridge& rb, const std::string& name);

}  // namespace ftd
