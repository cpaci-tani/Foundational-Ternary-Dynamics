/**
 * @file engine/web/js/viewport/constants.js
 * @purpose Shared pre-allocated-buffer sizes for the viewport renderer
 *          cluster. Centralized (D-6) so the orchestrator and the Phase-3
 *          sub-renderers (field / particle / flux) read a single source of
 *          truth instead of each redeclaring the literals (drift risk).
 * @consumers viewport.js, viewport/field-renderer.js,
 *            viewport/particle-renderer.js, viewport/flux-renderer.js
 *
 * Values are unchanged from the former per-module copies; all copies held
 * identical values (MAX_PARTICLES = 100000, MAX_FIELD_GRID = 16384),
 * verified before centralizing.
 *
 * NOTE on VOXEL_CENTER_OFFSET: it is intentionally NOT centralized here.
 * It is declared in exactly one module (field-renderer.js) — so there is
 * no drift to eliminate — AND it is a mutable `let` that field-renderer's
 * _syncCenterAndRadius() reassigns at runtime. An imported binding is
 * read-only, so moving it here would break that assignment. It stays
 * module-local in field-renderer.js.
 */

// Particle buffer is fixed at init to avoid dynamic GPU reallocation;
// draw range controls the visible count each frame.
export const MAX_PARTICLES = 100000;

// Field-grid buffer: up to 128×128 grid points (must cover lattice²).
export const MAX_FIELD_GRID = 16384;
