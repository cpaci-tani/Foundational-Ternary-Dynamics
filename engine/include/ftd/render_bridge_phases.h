#pragma once
/**
 * @file engine/include/ftd/render_bridge_phases.h
 * @purpose Free-function declarations for the decomposed phase methods.
 *          RenderBridge's phase_write / phase_forces / phase_read /
 *          phase_movement methods become thin orchestrators that call
 *          these free functions; the actual physics lives in
 *          engine/src/render_bridge_phases/<phase>.cpp TUs.
 * @consumers engine/src/render_bridge.cpp (orchestrator), all
 *            engine/src/render_bridge_phases/*.cpp TUs.
 * @related engine/include/ftd/render_bridge.h (the class),
 *          engine/include/ftd/render_bridge_diagnostics.h (POD structs)
 *
 * Phase 4 of the refactor sweep extends the R1-R5 precedent
 * (poisson_solvers.cpp, transmutation_phases.cpp, etc) to the four
 * remaining fat phase methods. ADR-0008 documents the pattern.
 */
#include <cstdint>
#include <vector>
#include "voxel.h"

namespace ftd {
class RenderBridge;

// =============================================================================
// Phase 4a — phase_write decomposition (2026-04-27)
//
// phase_write_cpu() is the thin orchestrator entry point: it preserves the
// full original parallel-for body (leapfrog + damping/Langevin + genesis +
// evaporation) but delegates the prologue (mask + larmor accel) and the
// shared manifest body to the helpers below. Splitting the parallel-for
// itself into smaller free functions would require thread-local RNG plumbing
// that breaks bit-exact determinism, so the loop body stays inline inside
// phase_write_cpu().
// =============================================================================

/// Build the per-voxel near-particle mask (and optional Larmor near-accel
/// field). Called once per phase_write before the main parallel-for, only
/// when toggles.selective_damping is on. Operates directly on rb.voxels_,
/// rb.lattice_, rb.near_particle_, rb.near_accel_.
void compute_near_particle_mask(RenderBridge& rb);

/// Pre-write flux snapshot for race-free curl/divergence reads during the
/// genesis branch of the main parallel-for. Only invoked when toggles.genesis
/// is on. Resizes rb.flux_pre_write_ to N and copies voxel.flux into it.
void snapshot_flux_pre_write(RenderBridge& rb);

/// Run the parallel-for body of phase_write: leapfrog (dual or single
/// substrate), damping or Langevin OU, genesis, evaporation. Reads
/// rb.toggles, rb.delta_j_*, rb.near_particle_, rb.near_accel_,
/// rb.flux_pre_write_; writes voxel state. RNG state is owned by
/// rb.rng_state_; Langevin seed plumbing handled by the caller.
void phase_write_main_loop(RenderBridge& rb);

/// Sequential post-pass that turns each "pending" particle_id sentinel
/// (-2) into a deterministic ID via rb.injector_.next_particle_id().
/// Required so that ID assignment order matches voxel-index order
/// regardless of OMP thread scheduling (ARCH-7).
void phase_write_assign_pending_ids(RenderBridge& rb);

/// Absorbing-boundary sponge: a graduated quadratic-ramp damping shell at the
/// lattice faces (width D = min(6, max(2, N/4)), f(d) = (d/D)²) applied to the
/// observable flux/wave_vel AND the dual L/R substrates, so outgoing waves
/// disperse into the void at the edges instead of wrapping/reflecting. Mirrors
/// the MockBridge JS sponge. Invoked from tick() ONLY when
/// toggles.absorbing_boundary is on (default off → golden-tick hash +
/// conservation tests unchanged).
void apply_absorbing_boundary(RenderBridge& rb);

// =============================================================================
// Phase 4b — phase_forces decomposition (2026-04-27)
//
// phase_forces is a single sequential per-voxel loop that computes EM,
// gravity, Lorentz, and color forces and integrates relativistic momentum
// (γ_FTD bandwidth-respecting). Splitting the per-voxel body into separate
// passes is NOT bit-exact — each force contributes to f_total in the same
// iteration and the integration step consumes f_total before the next voxel.
// So Phase 4b mirrors the Phase 4a structure: extract the orchestration
// steps (solve potentials, build color cache, run main loop) and keep the
// loop body intact. Splitting the loop further is rejected on the same
// grounds as the Phase 4a parallel-for body.
// =============================================================================

/// If toggles.poisson_coulomb is on (and emergent_forces is off), invoke
/// the SOR Coulomb solver to populate rb.phi_coulomb_. No-op otherwise.
void phase_forces_solve_potentials(RenderBridge& rb);

/// Repopulate rb.colored_sites_cache_ with every manifested coloured voxel.
/// Cleared+pushed (capacity reused). Skipped when toggles.color_forces is off.
void phase_forces_build_color_cache(RenderBridge& rb);

/// Per-voxel loop: EM force (3 modes), gravity (tier-2 gradient), Lorentz
/// (curl-of-flux as B), color (3-regime SU(3)-flavoured profile), then
/// γ_FTD relativistic momentum integration with bandwidth clamp. Reads
/// rb.toggles, rb.phi_coulomb_, rb.colored_sites_cache_, rb.dt_; writes
/// voxel velocity/accel_mag and rb.force_diag_.
void phase_forces_main_loop(RenderBridge& rb);

/// Unified-mass Phase 2 — rigid-body cluster inertia. Flood-fills each
/// connected (26-Moore) cluster of LOCKED, same-sign manifested voxels,
/// reconstructs F_cluster from rb.force_diag_ (f_coulomb+f_gravity+f_strong+
/// f_magnetic), and integrates the COM at inertial mass N·M_REST
/// (a_COM = F_cluster/(N·M_REST)), writing the resulting V_COM to every
/// member. Reads rb.voxels_.{state,locked,velocity,latency}, rb.force_diag_,
/// rb.lattice_, rb.dt_; writes member velocities only. Gated by
/// toggles.cluster_inertia at the call site (default OFF ⇒ additive / golden-
/// safe). The GPU mirror (GpuBackend::tick) reuses this verbatim on synced
/// host data so the CPU and GPU paths are bit-exact by construction.
/// (Also friend-declared in render_bridge.h for force_diag_ access; this is
/// the namespace-scope declaration that qualified ::ftd:: call sites need.)
void phase_forces_integrate_clusters(RenderBridge& rb);

// =============================================================================
// Phase 4c — phase_read + phase_movement decomposition (2026-04-27)
//
// phase_read is a single OMP parallel-for that branches per-voxel between
// dual-substrate and single-substrate paths, and within the single-substrate
// path between the FULL-stencil interior fast path and the sublattice slow
// path. Splitting the parallel-for into per-branch passes would require
// re-walking the lattice multiple times (rejected on cache-locality grounds)
// or per-voxel scratch storage (rejected on bit-exactness grounds; the golden
// gate forbids structural drift). Mirror Phase 4a/4b: extract orchestration
// (none here), keep the loop body intact in a single main-loop function.
//
// phase_movement is a single SEQUENTIAL per-voxel loop where each iteration
// can mutate two voxels (the moving particle and its target — move, bounce,
// or annihilation). Subsequent iterations then read those mutations via the
// moved_ guard. Splitting the loop into drift / annihilation / compact passes
// would require either (a) recording every (i, target) decision in a scratch
// buffer to be applied later — different observable order — or (b) running
// multiple sequential passes that each re-read state mutated by the previous
// pass — different physics. Both break the golden gate. So Phase 4c mirrors
// Phase 4a/4b: extract one main-loop function with the full body verbatim.
// =============================================================================

/// The single OMP parallel-for of phase_read: 18-pt isotropic Laplacian
/// (interior fast path + boundary slow path) plus state-flux coupling
/// (g_c·∇s + g_c·∇×(s·v)). Branches internally on toggles.dual_substrate
/// and toggles.bcc_stencil. Reads rb.voxels_, rb.lattice_, rb.toggles;
/// writes rb.delta_j_ (single) or rb.delta_j_L_ / rb.delta_j_R_ (dual).
void phase_read_main_loop(RenderBridge& rb);

/// The sequential per-voxel loop of phase_movement: remainder accumulation,
/// integer-jump dispatch, void-target move (with self-field carry), same-sign
/// elastic bounce, and opposite-sign annihilation (with 6-neighbor flux burst
/// distribution). Includes dual-substrate L/R flux carry / burst when
/// toggles.dual_substrate is on. Reads rb.toggles, rb.dt_; writes voxel
/// state/velocity/remainder/flux and rb.moved_ (per-tick double-process guard).
void phase_movement_main_loop(RenderBridge& rb);

}  // namespace ftd
