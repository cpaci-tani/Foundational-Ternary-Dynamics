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

namespace ftd {

// One red-black SOR sweep of ∇²φ = source on the 18-point isotropic stencil.
// Interior voxels use precomputed offsets (zero modulo); boundary voxels use
// lattice's modular neighbor tables.
void sor_sweep_18pt(std::vector<double>& phi,
                    const std::vector<double>& source,
                    const Lattice& lattice,
                    double omega);

// Gauss projection: solve ∇²φ = ∇·J − charge_coupling · s, then J -= ∇φ at
// void sites only. Manifested sites (state != 0) are skipped (transverse
// flux preserved). When dual_substrate is true the correction is split
// half-and-half between flux_L and flux_R. `charge_coupling` is the
// Phase-H coupling constant in Gauss's law; default 1.0 preserves
// geometric Coulomb (Phase G theorem). See
// docs/theory/10_eft_program/DERIV_EMERGENT_COULOMB_GEOMETRIC.md Section 7.
void gauss_project_cpu(std::vector<Voxel>& voxels,
                       const TernaryField& state,
                       std::vector<double>& phi,
                       std::vector<double>& sor_source,
                       const Lattice& lattice,
                       bool dual_substrate,
                       bool exact_dual_gauss,
                       double charge_coupling = 1.0,
                       int sor_iters = SOR_ITERATIONS);

// Coulomb Poisson: ∇²φ_C = -s. Warm-started, mean-subtracted for periodic BC.
void solve_coulomb_poisson_cpu(const TernaryField& state,
                               std::vector<double>& phi_coulomb,
                               std::vector<double>& sor_source,
                               const Lattice& lattice,
                               int sor_iters = SOR_ITERATIONS);

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
                               bool include_field_energy = false);

}  // namespace ftd
