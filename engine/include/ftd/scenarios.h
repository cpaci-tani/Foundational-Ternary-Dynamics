#pragma once
// ==========================================================================
//  engine/include/ftd/scenarios.h
//
//  C++ port of the Scale-0 scenario library that was previously JS-only on
//  the MockBridge (engine/web/js/bridge/scenarios/*.js).
//
//  BEFORE this port, WasmBridge.setupScenario(name) dispatched to
//  ftd_wasm.cpp::setup_scenario which only knew 35 of the 83 UI-exposed
//  scenarios. The other 48 silently no-op'd on WASM — users with the
//  native/WASM backend (the default fast path) would click `s0-seed-hydrogen`
//  and get an empty lattice with no error.
//
//  AFTER this port, every UI scenario has a C++ implementation; ftd_wasm.cpp
//  becomes a thin dispatcher that delegates to this module.
//
//  Function layout mirrors the JS group files 1-for-1:
//    setup_flux_scenario      ← js/bridge/scenarios/flux-scenarios.js
//    setup_light_scenario     ← js/bridge/scenarios/light-scenarios.js
//    setup_quantum_scenario   ← js/bridge/scenarios/quantum-scenarios.js
//    setup_s0_seed_scenario   ← js/bridge/scenarios/s0-seed-scenarios.js
//    setup_s0_field_scenario  ← js/bridge/scenarios/s0-field-scenarios.js
//    dispatch_scenario        ← js/bridge/scenarios/index.js
//
//  Each group function returns true if it handled `name`, false otherwise,
//  letting dispatch_scenario walk a prefix-matching fall-through list (same
//  pattern as the JS dispatcher).
//
//  PRIMITIVES USED (all live on RenderBridge):
//    rb.inject_flux_add(x,y,z,Vec3)      — JS _injectFlux (additive)
//    rb.inject_wave_vel_add(x,y,z,Vec3)  — JS _injectWaveVel (additive)
//    rb.inject_particle(x,y,z,state,...) — JS injectParticle / injectParticleFull
//    rb.inject_wavepacket(x,y,z,state)   — JS injectWavepacket
//    rb.create_entangled_pair(...)       — JS createEntangledPair
// ==========================================================================

#include <string>

namespace ftd {

class RenderBridge;

// ── Scenario dispatcher ────────────────────────────────────────────────
// Call this from WasmBridge / WS bridge / tests; it fans out to the
// group-specific functions below and returns true iff name was handled.
// The caller is responsible for rb.reset() BEFORE calling this (matches
// the JS runSetupScenario contract).
bool dispatch_scenario(RenderBridge& rb, const std::string& name);

// ── Group functions ────────────────────────────────────────────────────
// Each returns true iff the name prefix matched and the scenario was
// executed. Identical contract to the JS setupXxxScenario group functions.
bool setup_flux_scenario    (RenderBridge& rb, const std::string& name);
bool setup_light_scenario   (RenderBridge& rb, const std::string& name);
bool setup_quantum_scenario (RenderBridge& rb, const std::string& name);
bool setup_s0_seed_scenario (RenderBridge& rb, const std::string& name);
bool setup_s0_field_scenario(RenderBridge& rb, const std::string& name);

}  // namespace ftd
