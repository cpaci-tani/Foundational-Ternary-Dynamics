/**
 * @file bindings_internal.h
 * @brief Shared helpers exposed across the split Embind binding TUs.
 *
 * The WASM binding layer is split into four translation units that all
 * register into the single Emscripten module `ftd_module`:
 *   - ftd_wasm.cpp              — typed-array helpers + constants + core RB tick/run
 *   - bindings_render_bridge.cpp — RenderBridge toggle map, injection, scenarios
 *   - bindings_particle.cpp      — ParticleEngine (Scale 1)
 *   - bindings_atom.cpp          — AtomEngine (Scale 2)
 *
 * Each TU gets its own `EMSCRIPTEN_BINDINGS(ftd_module) { ... }` block.
 * Emscripten allows multiple bindings blocks sharing a module name as
 * long as they live in different TUs, so the final .wasm exports the
 * union of all four.
 *
 * Helpers that are referenced from more than one TU (primarily the
 * typed-array extractors used by the RenderBridge block) are declared
 * here and defined in ftd_wasm.cpp.
 */

#pragma once

#include <emscripten/val.h>
#include "ftd/render_bridge.h"

namespace ftd_wasm_internal {

// RenderBridge data extraction (defined in ftd_wasm.cpp)
emscripten::val get_particle_data(ftd::RenderBridge& rb);
emscripten::val get_diagnostics(ftd::RenderBridge& rb);
emscripten::val get_energy_audit(ftd::RenderBridge& rb);
emscripten::val get_lagrangian(ftd::RenderBridge& rb);
emscripten::val inspect_voxel(ftd::RenderBridge& rb, int x, int y, int z);
emscripten::val get_force_at(ftd::RenderBridge& rb, int x, int y, int z);
emscripten::val get_constants();

// Bulk flux extraction
emscripten::val get_flux_slice(ftd::RenderBridge& rb, int axis, int index);
emscripten::val get_flux_volume(ftd::RenderBridge& rb);

// Bulk sampled vector field exports
emscripten::val get_e_field_sampled(ftd::RenderBridge& rb, int stride);
emscripten::val get_b_field_sampled(ftd::RenderBridge& rb, int stride);
emscripten::val get_poynting_sampled(ftd::RenderBridge& rb, int stride);
emscripten::val get_divj_sampled(ftd::RenderBridge& rb, int stride);
emscripten::val get_flux_vector_sampled(ftd::RenderBridge& rb, int stride);
emscripten::val get_force_field_sampled(ftd::RenderBridge& rb, int stride);
// Force-field decomposition samplers (2026-04-19) — each returns per-voxel
// force vectors for a specific physical interaction. Used by the viewport
// to render force-arrow overlays. All three produce voxel-center positions
// (x + 0.5f) to match the particle-render convention.
emscripten::val get_gravity_field_sampled(ftd::RenderBridge& rb, int stride);
emscripten::val get_em_force_field(ftd::RenderBridge& rb, int stride);
emscripten::val get_strong_force_field(ftd::RenderBridge& rb, int stride);

// Sample the engine's Coulomb potential field along a ray from p1 to p2.
// Returns { positions:Float32Array(3N), V:Float32Array(N), count:N } via
// trilinear interpolation of phi_coulomb_. count=0 when phi_coulomb_ is
// empty (e.g. poisson_coulomb toggle off). Replaces JS-side trilinear
// interp with engine-direct sampling. (Phase 2 tech debt #4 — 2026-04-27)
emscripten::val sample_v_at_ray(
    ftd::RenderBridge& rb,
    double x1, double y1, double z1,
    double x2, double y2, double z2,
    int n);

// Lattice info
int get_lattice_size(ftd::RenderBridge& rb);

} // namespace ftd_wasm_internal
