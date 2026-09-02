#pragma once
/**
 * Poisson solvers — SOR sweep + top-level solvers.
 *
 * Extracted from render_bridge.cpp in the 2026-04-18 R1 refactor. These
 * are free functions that operate on whatever potential / source buffers
 * the caller owns; RenderBridge keeps ownership of phi_coulomb_ /
 * phi_latency_ / phi_ and calls into these helpers.
 *
 * Two levels of API:
 *   (1) sor_sweep_18pt: a single red-black SOR sweep of the isotropic
 *       18-point Laplacian stencil. Caller runs the iteration loop.
 *   (2) solve_coulomb_poisson / solve_latency_poisson: full solvers that
 *       build the source term, run SOR_ITERATIONS sweeps, and pin the
 *       gauge. They ALSO write voxel.latency for the latency solver.
 */

#include <vector>
#include "ftd/engine_state.h"
#include "lattice.h"
#include "voxel.h"
#include "ftd/strong_stress_energy.h"

namespace ftd {

// One red-black SOR sweep of ∇²φ = source on the 18-point isotropic stencil.
// Interior voxels use precomputed offsets (zero modulo); boundary voxels use
// lattice's modular neighbor tables.
void sor_sweep_18pt(std::vector<double>& phi,
                    const std::vector<double>& source,
                    const Lattice& lattice,
                    double omega);

// Gauss projection: solve the 18-point-stencil Laplacian ∇²_18 φ = ∇·J_6 −
// charge_coupling · s (∇·J_6 is the 6-point CENTRAL-DIFFERENCE divergence —
// a narrower, DIFFERENT operator from ∇²_18, not the same stencil written
// twice), then J -= ∇_6 φ at void sites only. Manifested sites (state != 0)
// are skipped (transverse flux preserved). When dual_substrate is true the
// correction is split half-and-half between flux_L and flux_R.
// `charge_coupling` is the Phase-H coupling constant in Gauss's law; default
// 1.0 preserves geometric Coulomb (Phase G theorem). See
// docs/theory/10_eft_program/DERIV_EMERGENT_COULOMB_GEOMETRIC.md Section 7.
//
// HONEST STATUS (full derivation + measured numbers in
// src/poisson_solvers.cpp, above gauss_project_cpu): this drives the field
// toward the constraint by a bounded, non-idempotent correction — it neither
// exactly enforces the constraint nor cleanly removes "unphysical" modes.
//   - On EVEN lattice size L, div_c's cokernel is 8-dimensional (one
//     neutrality condition per parity sublattice, vs. the single ODD-L
//     condition that mean-charge subtraction satisfies exactly). A point
//     charge violates 7 of the 8 on even L, so NO flux field satisfies the
//     constraint there and the residual floors at an irreducible 7/N,
//     independent of iteration count or solver quality.
//   - J -= ∇_6 φ is not idempotent: a second application moves the field by
//     a further ~42% of the first application's change, at either parity
//     of L, and the constraint residual saturates (stops improving) after
//     roughly six sweeps even though the underlying SOR solve itself keeps
//     converging.
// The related 18-point-solved-vs-6-point-measured residual floor (present
// even on odd L; a different effect from the even-lattice obstruction above)
// is already documented at SOR_ITERATIONS in constants.h — see that comment
// rather than duplicating it here.
void gauss_project_cpu(std::vector<Voxel>& voxels,
                       const TernaryField& state,
                       std::vector<double>& phi,
                       std::vector<double>& sor_source,
                       const Lattice& lattice,
                       bool dual_substrate,
                       bool exact_dual_gauss,
                       double charge_coupling = 1.0,
                       int sor_iters = SOR_ITERATIONS);

// Coulomb Poisson: ∇²φ_C = -charge_scale·s. Warm-started, mean-subtracted for
// periodic BC. `charge_scale` is the nuclear-charge scale Z (FTD-0281 helium
// extension): rho = -charge_scale·(s − mean_charge), so phi_C scales linearly
// with Z (well depth ×Z, Z=2 = He+). Default 1.0 is bit-identical to the
// legacy rho = -(s − mean_charge) (golden-neutral).
void solve_coulomb_poisson_cpu(const TernaryField& state,
                               std::vector<double>& phi_coulomb,
                               std::vector<double>& sor_source,
                               const Lattice& lattice,
                               int sor_iters = SOR_ITERATIONS,
                               double charge_scale = 1.0);

// Latency Poisson: ∇²φ_L = 4πGρ_mass with ρ_mass = K_B|state|. Writes
// voxel.latency = sqrt(clamp(|phi_latency|, 0, LATENCY_HORIZON_CLAMP)).
// When include_field_energy is true, the source also includes the local
// field-energy density ½(|J|²+|wave_vel|²) (see term_toggles.field_energy_gravity).
void solve_latency_poisson_cpu(std::vector<Voxel>& voxels,
                               const TernaryField& state,
                               std::vector<double>& phi_latency,
                               std::vector<double>& sor_source,
                               const Lattice& lattice,
                               int sor_iters = SOR_ITERATIONS,
                               bool include_field_energy = false,
                               const std::vector<StrongStressCell>* strong_cells = nullptr);

}  // namespace ftd
