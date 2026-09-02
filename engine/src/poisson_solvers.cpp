/**
 * Poisson solvers — implementation.
 * Extracted from render_bridge.cpp, 2026-04-18 refactor ticket R1.
 */

#include "ftd/poisson_solvers.h"
#include "ftd/constants.h"
#include "ftd/parallel.h"
#include "ftd/volumetric_measure.h"
#include <algorithm>
#include <cmath>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace ftd {

// ============================================================================
// SOR sweep helper — isotropic 18-point Poisson stencil with RED-BLACK ordering
// See original commentary in render_bridge.cpp (pre-refactor) for perf notes.
// ============================================================================
void sor_sweep_18pt(std::vector<double>& phi,
                    const std::vector<double>& source,
                    const Lattice& lattice,
                    double omega) {
  constexpr double INV3 = 1.0 / 3.0;
  constexpr double INV6 = 1.0 / 6.0;
  constexpr double INV4 = 1.0 / 4.0;
  const int L = lattice.size();
  const int LL = L * L;
  const int Nm1 = L - 1;

  const int o_xp = 1,  o_xm = -1;
  const int o_yp = L,  o_ym = -L;
  const int o_zp = LL, o_zm = -LL;
  const int o_xpyp = o_xp + o_yp, o_xpym = o_xp + o_ym;
  const int o_xmyp = o_xm + o_yp, o_xmym = o_xm + o_ym;
  const int o_xpzp = o_xp + o_zp, o_xpzm = o_xp + o_zm;
  const int o_xmzp = o_xm + o_zp, o_xmzm = o_xm + o_zm;
  const int o_ypzp = o_yp + o_zp, o_ypzm = o_yp + o_zm;
  const int o_ymzp = o_ym + o_zp, o_ymzm = o_ym + o_zm;

// NOTE: Standard 2-color Red-Black sweeps fail for the 18-point Laplacian because
  // the stencil includes 12 edge-sharing neighbors (radius-1 diagonals), which
  // create read-write races within the same Red/Black partition.
  // Instead, we use an 8-color (2x2x2) coloring scheme.
  //
  // DETERMINISM (golden gate): the 2x2x2 parity coloring is race-free ONLY for
  // INTERIOR cells, whose 18-point neighbours never leave the lattice. The
  // lattice has PERIODIC boundary conditions (lattice.h), and on an ODD lattice
  // the wrap maps coord Nm1 (even) → 0 (even): a face/edge neighbour of a
  // boundary cell can wrap to ANOTHER boundary cell of the SAME colour, so two
  // same-colour boundary cells become stencil-neighbours and racing — a genuine
  // read-write race, not a ULP reduction issue, that floats phi run-to-run.
  // Fix: update interior cells in PARALLEL (8-colour, race-free) and boundary
  // cells SEQUENTIALLY in lexicographic order per colour. This is bit-exact to a
  // fully-sequential lexicographic sweep because (a) within a colour every cell
  // reads only other-colour (frozen) neighbours, so interior update order is
  // irrelevant; (b) same-colour interior/boundary cells are never neighbours
  // (adjacency is an odd offset → different colour; only a wrap, i.e. two
  // boundary cells, yields a same-colour pair), so the interior/boundary split
  // does not change reads; (c) the seam boundary↔boundary same-colour pairs are
  // resolved in the same lexicographic order by the sequential boundary pass.
  // Boundary cells are O(L^2) (a small fraction for large L), so the parallel
  // interior sweep — the valuable hot loop — is preserved.
  for (int color = 0; color < 8; ++color) {
    int start_x = color & 1;
    int start_y = (color >> 1) & 1;
    int start_z = (color >> 2) & 1;

    // --- Interior cells: PARALLEL (fast path; never wraps → race-free) ---
    ftd::parallel_for(start_x, L, [&](int _lo, int _hi) {
    for (int ix = _lo; ix < _hi; ++ix) {
      if (((ix - start_x) & 1) != 0) continue;  // preserve 8-color stride-2 in ix
      if (ix == 0 || ix == Nm1) continue;  // x-face → boundary pass
      for (int iy = start_y; iy < L; iy += 2) {
        if (iy == 0 || iy == Nm1) continue;  // y-face → boundary pass
        for (int iz = start_z; iz < L; iz += 2) {
          if (iz == 0 || iz == Nm1) continue;  // z-face → boundary pass
          int idx = ix * LL + iy * L + iz;

          double face_sum = phi[idx + o_xp] + phi[idx + o_xm]
                          + phi[idx + o_yp] + phi[idx + o_ym]
                          + phi[idx + o_zp] + phi[idx + o_zm];
          double edge_sum = phi[idx + o_xpyp] + phi[idx + o_xpym]
                          + phi[idx + o_xmyp] + phi[idx + o_xmym]
                          + phi[idx + o_xpzp] + phi[idx + o_xpzm]
                          + phi[idx + o_xmzp] + phi[idx + o_xmzm]
                          + phi[idx + o_ypzp] + phi[idx + o_ypzm]
                          + phi[idx + o_ymzp] + phi[idx + o_ymzm];

          const double gs = (INV3 * face_sum + INV6 * edge_sum - source[idx]) * INV4;
          phi[idx] += omega * (gs - phi[idx]);
        }
      }
    }
    });

    // --- Boundary cells: SEQUENTIAL lexicographic (slow path; wraps at seam) ---
    for (int ix = start_x; ix < L; ix += 2) {
      for (int iy = start_y; iy < L; iy += 2) {
        for (int iz = start_z; iz < L; iz += 2) {
          if (ix > 0 && ix < Nm1 && iy > 0 && iy < Nm1 && iz > 0 && iz < Nm1)
            continue;  // interior → already done in the parallel pass
          int idx = ix * LL + iy * L + iz;

          double face_sum = 0.0, edge_sum = 0.0;
          const auto& face = lattice.neighbors_6(ix, iy, iz);
          const auto& edge = lattice.neighbors_12(ix, iy, iz);
          for (int n : face) face_sum += phi[n];
          for (int n : edge) edge_sum += phi[n];

          const double gs = (INV3 * face_sum + INV6 * edge_sum - source[idx]) * INV4;
          phi[idx] += omega * (gs - phi[idx]);
        }
      }
    }
  }
}

// Local helper: central-difference divergence with lattice's periodic wrap.
// Mirrors RenderBridge::divergence_flux without requiring a bridge ref.
static inline double divergence_flux_at(const std::vector<Voxel>& voxels,
                                        const Lattice& lattice, int idx) {
  const auto& nbrs = lattice.neighbors_6(idx);
  double div = 0.0;
  div += (voxels[nbrs[0]].flux.x - voxels[nbrs[1]].flux.x) * 0.5;
  div += (voxels[nbrs[2]].flux.y - voxels[nbrs[3]].flux.y) * 0.5;
  div += (voxels[nbrs[4]].flux.z - voxels[nbrs[5]].flux.z) * 0.5;
  return div;
}

// ============================================================================
// GAUSS PROJECTION — SOLVABILITY (FREDHOLM) CONDITION AND ITS LIMITS
// ============================================================================
//
// The projection enforces a constraint of the form
//   div_centred(J) = charge_coupling * (s - s_bar)
// where div_centred is the 6-point central-difference divergence used both
// to build sor_source (below) and to define the correction J -= grad(phi).
// On a periodic lattice this equation is solvable for phi — equivalently,
// the correction can be found at all — ONLY if the source integrates to
// zero against every function in div_centred's cokernel: its Fredholm
// condition.
//
// div_centred's cokernel is exactly the set of functions constant on each
// coset of 2*Z^3: the central difference only couples same-parity sites two
// steps apart along each axis, so it factors the lattice into parity
// classes that never communicate through this operator.
//
//   * ODD L: gcd(L,2)=1, so the 2*Z^3 quotient has a single coset and the
//     cokernel is 1-DIMENSIONAL (the constants). The mean_charge
//     subtraction immediately below IS exactly this one condition —
//     charge_sum/N removed so the total source integrates to zero. This is
//     FORCED, not an ad-hoc regularization: it is the unique solvability
//     requirement for the 18-point SOR solve, whose own kernel is the
//     constants (see sor_sweep_18pt above). The golden lattice (L=17) and
//     the 33 scenario tests all sit here and are unaffected by what
//     follows.
//
//   * EVEN L: the same coset decomposition has EIGHT classes — one per
//     choice of (x mod 2, y mod 2, z mod 2), the eight parity sublattices.
//     The Fredholm condition is then EIGHT independent equations: charge
//     must be separately neutral on each of the eight sublattices, not
//     merely on the box as a whole. A single point charge sits on ONE
//     sublattice and violates the other SEVEN neutrality conditions
//     outright, so NO flux field whatsoever satisfies the constraint.
//     Subtracting the single scalar mean_charge removes only one of the
//     eight obstructions and leaves the rest standing. This is NOT a
//     solver-quality issue — more SOR sweeps, a different omega, or a
//     better Poisson solver cannot close it, because the unsatisfied part
//     of the source has no phi whose Laplacian reaches it under
//     div_centred. The residual floors at an irreducible sum-of-squares of
//     exactly 7/N (seven unsatisfiable sublattice conditions, N sites).
//     Measured: repeated projection on a static point charge converges to
//     6.4594e-4 at L=16 against the predicted sqrt(7)/N = 6.459e-4, while
//     the same iteration at the neighbouring ODD L=15 drives the residual
//     to 9e-6 and keeps falling. Campaigns run at L=32/64/128 are therefore
//     enforcing an unsatisfiable constraint throughout — this is a hard
//     mathematical limit of the centred-stencil formulation on even boxes,
//     not a bug to be tuned away.
//
// A second, independent limit holds at BOTH parities: even where the
// constraint IS solvable, one application of the correction below is not a
// projection in the idempotent sense. See the comment at the correction
// loop further down in this function for the mechanism and measured
// numbers.
//
// See include/ftd/poisson_solvers.h for the declaration-level summary, and
// the SOR_ITERATIONS comment in include/ftd/constants.h for a related but
// DIFFERENT, already-documented effect — the 18-point-solved-vs-6-point-
// measured stencil-mismatch floor, which is present even on odd lattices
// (unlike the even-lattice-only obstruction above, which is exactly absent
// on odd L in the idealized single-charge case).
// ============================================================================
void gauss_project_cpu(std::vector<Voxel>& voxels,
                       const TernaryField& state,
                       std::vector<double>& phi,
                       std::vector<double>& sor_source,
                       const Lattice& lattice,
                       bool dual_substrate,
                       bool exact_dual_gauss,
                       double charge_coupling,
                       int sor_iters) {
  const int N = static_cast<int>(lattice.total_sites());
  const int L = lattice.size();
  const int LL = L * L;
  const int Nm1 = L - 1;
  constexpr double OMEGA = SOR_OMEGA;

  // Fredholm condition for div_centred (full derivation in the block comment
  // above this function): on ODD L this mean_charge subtraction is the
  // complete, forced solvability condition. On EVEN L it removes only ONE
  // of the EIGHT per-parity-sublattice neutrality conditions the operator's
  // 8-dimensional cokernel actually demands — the other seven are left
  // unsatisfied and floor the residual at 7/N regardless of what follows.
  const double charge_sum = static_cast<double>(state.charge_sum());
  const double mean_charge = charge_sum / N;

  ftd::parallel_for(0, L, [&](int _lo, int _hi) {
  for (int ix = _lo; ix < _hi; ++ix) {
    for (int iy = 0; iy < L; ++iy) {
      for (int iz = 0; iz < L; ++iz) {
        const int i = ix * LL + iy * L + iz;
        double div;
        if (iz > 0 && iz < Nm1 && iy > 0 && iy < Nm1 && ix > 0 && ix < Nm1) {
          div = (voxels[i+LL].flux.x - voxels[i-LL].flux.x) * 0.5
              + (voxels[i+L].flux.y  - voxels[i-L].flux.y)  * 0.5
              + (voxels[i+1].flux.z  - voxels[i-1].flux.z)  * 0.5;
        } else {
          div = divergence_flux_at(voxels, lattice, i);
        }
        sor_source[i] = div - charge_coupling * (static_cast<double>(state.state_at(i)) - mean_charge);
      }
    }
  }
  });

  for (int iter = 0; iter < sor_iters; ++iter) {
    sor_sweep_18pt(phi, sor_source, lattice, OMEGA);
  }

  // Sequential sum — DETERMINISM REQUIREMENT (golden gate). Float `+` under an
  // OpenMP reduction is not order-stable across threads, so a parallel reduction
  // floats phi_mean by ULPs run-to-run. The phi-mean shift is gauge-irrelevant
  // to grad(phi) (physics unchanged), but it leaks into absolute-phi audit
  // scalars (e.g. coulomb_pe) and breaks the bit-reproducible golden hash. This
  // is a single O(N) pass, dwarfed by the iterative SOR sweeps above, so the
  // cost of sequential summation is negligible. The 8-color SOR sweep stays
  // parallel (race-free, deterministic).
  double phi_sum = 0.0;
  for (int i = 0; i < N; ++i) {
    phi_sum += phi[i];
  }
  const double phi_mean = phi_sum / N;
  ftd::parallel_for(0, N, [&](int _lo, int _hi) {
  for (int i = _lo; i < _hi; ++i) {
    phi[i] -= phi_mean;
  }
  });

  // NOT AN IDEMPOTENT PROJECTION. J -= grad(phi) below drives the field
  // toward the constraint but does not land on it in one application, at
  // EITHER parity of L. After an exact solve of the equation above (the SOR
  // equation residual itself converges cleanly to ~1e-18 by 500 sweeps),
  // the per-Fourier-mode constraint residual that SURVIVES this correction
  // carries a factor
  //     1 - sigma_wide(k) / sigma_18(k)
  // (sigma_18 the 18-point stencil symbol actually solved for phi;
  // sigma_wide the different, wider symbol implicitly annihilated by one
  // gradient-subtraction step). That factor reaches its maximum of 1.0
  // exactly when every component of k is 0 or pi — and a point charge has a
  // flat spectrum, driving every mode there. Consequently one application
  // realises only ~40% of a point charge's target correction; applying this
  // whole function a SECOND time moves the field by a FURTHER ~42% of the
  // first application's change (measured at both even and odd L), and the
  // constraint residual saturates near 1e-2 and stops improving after
  // roughly six sweeps — even though the SOR equation residual above keeps
  // converging the whole time. The correction is also not energy-neutral: a
  // single application changed a random field's gradient energy by 18% in
  // one measured case. None of this is a defect in the SOR solve itself; it
  // is a property of representing a hard constraint as one
  // gradient-subtraction step rather than an exact orthogonal projector.
  ftd::parallel_for(0, L, [&](int _lo, int _hi) {
  for (int ix = _lo; ix < _hi; ++ix) {
    for (int iy = 0; iy < L; ++iy) {
      for (int iz = 0; iz < L; ++iz) {
        const int i = ix * LL + iy * L + iz;
        if (!exact_dual_gauss && state.state_at(i) != 0) continue;
        Vec3 grad_phi;
        if (iz > 0 && iz < Nm1 && iy > 0 && iy < Nm1 && ix > 0 && ix < Nm1) {
          grad_phi.x = (phi[i+LL] - phi[i-LL]) * 0.5;
          grad_phi.y = (phi[i+L]  - phi[i-L])  * 0.5;
          grad_phi.z = (phi[i+1]  - phi[i-1])  * 0.5;
        } else {
          const auto& n = lattice.neighbors_6(ix, iy, iz);
          grad_phi.x = (phi[n[0]] - phi[n[1]]) * 0.5;
          grad_phi.y = (phi[n[2]] - phi[n[3]]) * 0.5;
          grad_phi.z = (phi[n[4]] - phi[n[5]]) * 0.5;
        }
        voxels[i].flux -= grad_phi;

        if (dual_substrate) {
          Vec3 half_corr = grad_phi * 0.5;
          voxels[i].flux_L -= half_corr;
          voxels[i].flux_R -= half_corr;
        }
      }
    }
  }
  });
}

void solve_coulomb_poisson_cpu(const TernaryField& state,
                               std::vector<double>& phi_coulomb,
                               std::vector<double>& sor_source,
                               const Lattice& lattice,
                               int sor_iters,
                               double charge_scale) {
  const int N = static_cast<int>(lattice.total_sites());
  constexpr double OMEGA = SOR_OMEGA;

  double charge_sum = static_cast<double>(state.charge_sum());
  const double mean_charge = charge_sum / N;

  // FTD-0281 helium extension: rho = -charge_scale·(s − mean_charge). Z=1.0 is
  // bit-identical to the legacy -(s − mean_charge); Z=2 doubles the Coulomb
  // well that drives the db_clock_coulomb KG term. The mean-charge subtraction
  // keeps the net source zero (periodic Poisson solvability) at any scale.
  ftd::parallel_for(0, N, [&](int _lo, int _hi) {
  for (int i = _lo; i < _hi; ++i) {
    sor_source[i] = -charge_scale * (static_cast<double>(state.state_at(i)) - mean_charge);
  }
  });

  for (int iter = 0; iter < sor_iters; ++iter) {
    sor_sweep_18pt(phi_coulomb, sor_source, lattice, OMEGA);
  }

  // Sequential sum — DETERMINISM REQUIREMENT (golden gate); see note in
  // gauss_project_cpu. coulomb_pe in the energy audit reads absolute phi_coulomb
  // values, so a floated phi_mean here is the primary path that broke the hash.
  double phi_sum = 0.0;
  for (int i = 0; i < N; ++i)
    phi_sum += phi_coulomb[i];
  const double phi_mean = phi_sum / N;
  ftd::parallel_for(0, N, [&](int _lo, int _hi) {
  for (int i = _lo; i < _hi; ++i)
    phi_coulomb[i] -= phi_mean;
  });
}

void solve_latency_poisson_cpu(std::vector<Voxel>& voxels,
                               const TernaryField& state,
                               std::vector<double>& phi_latency,
                               std::vector<double>& sor_source,
                               const Lattice& lattice,
                               int sor_iters,
                               bool include_field_energy,
                               const std::vector<StrongStressCell>* strong_cells) {
  const int N = static_cast<int>(lattice.total_sites());
  constexpr double OMEGA = SOR_OMEGA;
  constexpr double FOUR_PI_G = 4.0 * PI * G_N;

  // [IMPOSED] Gravitating density = M_GRAVITATIONAL·|state| plus, when
  // include_field_energy is set, the local field-energy density
  // ½(|J|²+|wave_vel|²). This is T00 at a site, not its volume-integrated
  // cell energy; the explicit V_cell factor belongs in spatial totals, not
  // in this local Poisson source. Motivated by GR sourcing gravity from the full
  // stress-energy so a flux-only configuration (e.g. a gravity wave) carries a
  // real potential; the coupling is imposed in the engine, not derived.
  double rho_sum = M_GRAVITATIONAL * static_cast<double>(state.manifested_count());
  if (include_field_energy) {
    // Sequential sum — DETERMINISM REQUIREMENT (golden gate); see note in
    // gauss_project_cpu. field_energy_sum sources the latency potential, so a
    // floated value here is not gauge-cancelled and reaches voxel latency.
    double field_energy_sum = 0.0;
    for (int i = 0; i < N; ++i) {
      field_energy_sum += local_field_wave_energy_density(
          voxels[i].flux.mag2(), voxels[i].wave_vel.mag2());
    }
    rho_sum += field_energy_sum;
  }
  // FTD-0406 [OWNER-AUTHORIZED SELECTION]: the selected local strong T00
  // sources gravitational mass through E=M*C_SPEED^2.  Do not silently use
  // c=1 here: the raw lattice normalization has C_SPEED^2=1/3.
  const double inv_c2 = 1.0 / (C_SPEED * C_SPEED);
  if (strong_cells && strong_cells->size() == static_cast<std::size_t>(N)) {
    double strong_mass_sum = 0.0;
    for (int i = 0; i < N; ++i)
      strong_mass_sum += (*strong_cells)[i].energy_density * inv_c2;
    rho_sum += strong_mass_sum;
  }
  const double mean_rho = rho_sum / N;

  ftd::parallel_for(0, N, [&](int _lo, int _hi) {
  for (int i = _lo; i < _hi; ++i) {
    double rho = M_GRAVITATIONAL * std::abs(state.state_at(i));
    if (include_field_energy)
      rho += local_field_wave_energy_density(
          voxels[i].flux.mag2(), voxels[i].wave_vel.mag2());
    if (strong_cells && strong_cells->size() == static_cast<std::size_t>(N))
      rho += (*strong_cells)[i].energy_density * inv_c2;
    sor_source[i] = FOUR_PI_G * (rho - mean_rho);
  }
  });

  for (int iter = 0; iter < sor_iters; ++iter) {
    sor_sweep_18pt(phi_latency, sor_source, lattice, OMEGA);
  }

  // Sequential sum — DETERMINISM REQUIREMENT (golden gate); see note in
  // gauss_project_cpu. voxel latency reads absolute phi_latency values.
  double phi_sum = 0.0;
  for (int i = 0; i < N; ++i)
    phi_sum += phi_latency[i];
  const double phi_mean = phi_sum / N;
  ftd::parallel_for(0, N, [&](int _lo, int _hi) {
  for (int i = _lo; i < _hi; ++i)
    phi_latency[i] -= phi_mean;
  });

  // P6 (2026-07-26): map the WELL, not |phi|.
  //
  // The periodic Poisson solve requires a zero-mean source, so phi necessarily
  // takes BOTH signs and the mean is subtracted just above. Taking |phi| then
  // mapped the under-dense ~73% of the box (measured frac(phi>0) = 0.7275 /
  // 0.7313 / 0.7309 at L = 17/33/65) to a POSITIVE latency identical in form to
  // a gravity well, and made the axial profile non-monotone -- at L=65 latency
  // fell to 0.00099 at r=28 and then ROSE to 0.00274 at the box edge. sqrt()
  // makes even a tiny positive phi first-order, so the artefact is not small.
  //
  // Sign convention read from the solver itself (sor_sweep_18pt): the sweep
  // solves grad^2 phi = +4 pi G (rho - rho_bar), so an overdensity gives phi<0
  // and the physical well is -phi. Clamping at 0 leaves under-dense regions at
  // zero latency, which is what "no well here" should mean.
  for (int i = 0; i < N; ++i) {
    double well = -phi_latency[i];                 // physical well depth (>=0 inside a mass)
    double clamped = std::min(std::max(well, 0.0), LATENCY_HORIZON_CLAMP);
    voxels[i].latency = std::sqrt(clamped);
  }
}

}  // namespace ftd
