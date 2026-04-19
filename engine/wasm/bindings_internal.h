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

// Lattice info
int get_lattice_size(ftd::RenderBridge& rb);

} // namespace ftd_wasm_internal
