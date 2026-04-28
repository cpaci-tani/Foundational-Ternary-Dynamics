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

}  // namespace ftd
