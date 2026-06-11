#!/usr/bin/env python3
"""
genesis_throttle_model.py -- FTD-0110 nonlinear bridge attack (Route 1)

Analytical model of genesis-throttled cluster formation on a full cubic
lattice.  Simulates the wave equation with the 18-point O_h-isotropic
Laplacian on an L^3 lattice with periodic BCs, plus genesis threshold
and flux/kinetic drain -- matching the engine's actual physics.

Key insight from reading engine code (phase_write.cpp):
Within a single tick, genesis is a SIMULTANEOUS threshold count -- all
voxels check |J|^2 > K_GENESIS^2 against the current flux, and the
drain modifies each voxel independently.  Inter-voxel effects only
propagate via the wave equation on the NEXT tick.

FTD-0267 engine telemetry targets (L=64, canonical CPU, 4 seeds):
  A=9:   cum_gen = 3              (single seed)
  A=10:  cum_gen = {3,7,5,4}      (mean 4.8)
  A=14:  cum_gen = {15,18,15,17}  (mean 16.2)
  A=30:  cum_gen = 47             (single seed)

All inputs from ontic chain -- NO free parameters.

[EPISTEMIC TAG: EXPLORATORY CALCULATION -- not a derivation until
verified against engine telemetry]

Author: Antigravity / FTD project
Date: 2026-06-10
LEDGER: FTD-0110 extension
"""

import numpy as np
from itertools import product

# ============================================================================
# Constants from ontic chain (engine/include/ftd/ontic/)
# ============================================================================
K_B = 0.511                          # electron mass, simulation units
K_MANIFEST = K_B                     # genesis/evaporation kinetics scale
N_C = 3                              # color charge number
K_GENESIS = N_C * K_MANIFEST         # = 1.533
K_GENESIS_KINETIC_DRAIN = 0.5        # wave_vel drain fraction at genesis
K_GENESIS_FLUX_EPSILON = 1e-9        # floor on |J| during flux drain
C_SPEED = 1.0 / np.sqrt(3.0)        # CFL speed on cubic lattice
DAMPING = 0.001                      # base damping constant

# ============================================================================
# Full-lattice simulation
# ============================================================================

class LatticeWaveField:
    """
    Full 3D vector wave field on an L^3 periodic cubic lattice.
    Implements the engine's tick cycle:
      1. Leapfrog wave evolution with 18-point Laplacian
      2. Simultaneous genesis threshold check + drain
      3. Damping

    No Langevin noise (deterministic baseline).
    No Gauss projection (we want to isolate the genesis throttling).
    """
    def __init__(self, L, amplitude):
        self.L = L
        self.N = L * L * L
        self.A = amplitude

        # 3-vector flux and wave_vel per voxel
        self.flux = np.zeros((self.N, 3))
        self.wave_vel = np.zeros((self.N, 3))
        self.state = np.zeros(self.N, dtype=np.int8)

        # Precompute neighbor tables
        self._build_neighbor_tables()

        # Initial condition: delta injection at center
        center = self._idx(L // 2, L // 2, L // 2)
        self.flux[center, 0] = amplitude * K_GENESIS  # axial injection

        # Telemetry
        self.genesis_per_tick = []
        self.total_genesis = 0
        self.tick_count = 0

    def _idx(self, x, y, z):
        """Convert 3D coords to flat index with periodic wrapping."""
        return ((x % self.L) * self.L + (y % self.L)) * self.L + (z % self.L)

    def _build_neighbor_tables(self):
        """Precompute SC (face, 6) and FCC (edge, 12) neighbor indices."""
        L = self.L
        self.sc_neighbors = []   # 6 face neighbors per voxel
        self.fcc_neighbors = []  # 12 edge neighbors per voxel

        # SC neighbor offsets (6 faces): +/-x, +/-y, +/-z
        sc_offsets = [(1,0,0),(-1,0,0),(0,1,0),(0,-1,0),(0,0,1),(0,0,-1)]
        # FCC neighbor offsets (12 edges): all (dx,dy,0) etc with |dx|=|dy|=1
        fcc_offsets = [
            (1,1,0),(1,-1,0),(-1,1,0),(-1,-1,0),
            (1,0,1),(1,0,-1),(-1,0,1),(-1,0,-1),
            (0,1,1),(0,1,-1),(0,-1,1),(0,-1,-1),
        ]

        for x in range(L):
            for y in range(L):
                for z in range(L):
                    sc = []
                    for dx, dy, dz in sc_offsets:
                        sc.append(self._idx(x+dx, y+dy, z+dz))
                    self.sc_neighbors.append(sc)

                    fcc = []
                    for dx, dy, dz in fcc_offsets:
                        fcc.append(self._idx(x+dx, y+dy, z+dz))
                    self.fcc_neighbors.append(fcc)

    def _apply_laplacian_component(self, f_comp):
        """
        Apply 18-point O_h Laplacian to a scalar field (one component).
        L*f = (1/3)*sum_SC(f_n - f) + (1/6)*sum_FCC(f_n - f)
            = (1/3)*sum_SC(f_n) + (1/6)*sum_FCC(f_n) - 4*f
        """
        result = np.zeros_like(f_comp)
        for i in range(self.N):
            sc_sum = sum(f_comp[n] for n in self.sc_neighbors[i])
            fcc_sum = sum(f_comp[n] for n in self.fcc_neighbors[i])
            result[i] = (sc_sum / 3.0 + fcc_sum / 6.0) - 4.0 * f_comp[i]
        return result

    def tick(self):
        """Execute one tick of the engine pipeline."""
        self.tick_count += 1

        # Phase 1: Wave equation (leapfrog, each component independent)
        for comp in range(3):
            delta_j = C_SPEED**2 * self._apply_laplacian_component(self.flux[:, comp])
            self.wave_vel[:, comp] += delta_j
            self.flux[:, comp] += self.wave_vel[:, comp]

        # Phase 2: Genesis (simultaneous threshold check)
        genesis_this_tick = 0
        for i in range(self.N):
            if self.state[i] != 0:
                continue
            jmag2 = np.sum(self.flux[i]**2)
            if jmag2 > K_GENESIS**2:
                # Genesis fires
                genesis_this_tick += 1
                self.state[i] = 1 if self.flux[i, 0] >= 0 else -1

                # Kinetic drain: wave_vel *= (1 - K_GENESIS_KINETIC_DRAIN)
                self.wave_vel[i] *= (1.0 - K_GENESIS_KINETIC_DRAIN)

                # Flux drain: flux *= max(0, 1 - K_GENESIS / |J|)
                jmag = np.sqrt(jmag2)
                if jmag > K_GENESIS_FLUX_EPSILON:
                    self.flux[i] *= max(0.0, 1.0 - K_GENESIS / jmag)

        self.genesis_per_tick.append(genesis_this_tick)
        self.total_genesis += genesis_this_tick

        # Phase 3: Damping
        self.flux *= (1.0 - DAMPING)
        self.wave_vel *= (1.0 - DAMPING)

        return genesis_this_tick


class SmallLatticeWaveField:
    """
    Optimized version using numpy vectorization for L <= 32.
    Same physics as LatticeWaveField but uses dense neighbor arrays.
    """
    def __init__(self, L, amplitude):
        self.L = L
        self.N = L * L * L
        self.A = amplitude

        # 3-vector flux and wave_vel per voxel
        self.flux = np.zeros((self.N, 3))
        self.wave_vel = np.zeros((self.N, 3))
        self.state = np.zeros(self.N, dtype=np.int8)

        # Precompute neighbor index arrays (N x 6 for SC, N x 12 for FCC)
        self._build_neighbor_arrays()

        # Initial condition: delta injection at center
        center = self._idx(L // 2, L // 2, L // 2)
        self.flux[center, 0] = amplitude * K_GENESIS  # axial injection

        # Telemetry
        self.genesis_per_tick = []
        self.total_genesis = 0
        self.tick_count = 0

    def _idx(self, x, y, z):
        return ((x % self.L) * self.L + (y % self.L)) * self.L + (z % self.L)

    def _build_neighbor_arrays(self):
        L = self.L
        sc_offsets = [(1,0,0),(-1,0,0),(0,1,0),(0,-1,0),(0,0,1),(0,0,-1)]
        fcc_offsets = [
            (1,1,0),(1,-1,0),(-1,1,0),(-1,-1,0),
            (1,0,1),(1,0,-1),(-1,0,1),(-1,0,-1),
            (0,1,1),(0,1,-1),(0,-1,1),(0,-1,-1),
        ]

        self.sc_nbr = np.zeros((self.N, 6), dtype=np.int32)
        self.fcc_nbr = np.zeros((self.N, 12), dtype=np.int32)

        for x in range(L):
            for y in range(L):
                for z in range(L):
                    i = self._idx(x, y, z)
                    for k, (dx, dy, dz) in enumerate(sc_offsets):
                        self.sc_nbr[i, k] = self._idx(x+dx, y+dy, z+dz)
                    for k, (dx, dy, dz) in enumerate(fcc_offsets):
                        self.fcc_nbr[i, k] = self._idx(x+dx, y+dy, z+dz)

    def _apply_laplacian_component(self, f):
        """Vectorized 18-point Laplacian."""
        sc_sum = np.sum(f[self.sc_nbr], axis=1)  # (N,)
        fcc_sum = np.sum(f[self.fcc_nbr], axis=1)  # (N,)
        return sc_sum / 3.0 + fcc_sum / 6.0 - 4.0 * f

    def tick(self):
        self.tick_count += 1

        # Phase 1: Wave equation
        for comp in range(3):
            delta_j = C_SPEED**2 * self._apply_laplacian_component(self.flux[:, comp])
            self.wave_vel[:, comp] += delta_j
            self.flux[:, comp] += self.wave_vel[:, comp]

        # Phase 2: Genesis (vectorized check, sequential drain)
        jmag2 = np.sum(self.flux**2, axis=1)
        candidates = np.where((self.state == 0) & (jmag2 > K_GENESIS**2))[0]

        genesis_this_tick = len(candidates)
        for i in candidates:
            self.state[i] = 1 if self.flux[i, 0] >= 0 else -1
            self.wave_vel[i] *= (1.0 - K_GENESIS_KINETIC_DRAIN)
            jmag = np.sqrt(jmag2[i])
            if jmag > K_GENESIS_FLUX_EPSILON:
                self.flux[i] *= max(0.0, 1.0 - K_GENESIS / jmag)

        self.genesis_per_tick.append(genesis_this_tick)
        self.total_genesis += genesis_this_tick

        # Phase 3: Damping
        self.flux *= (1.0 - DAMPING)
        self.wave_vel *= (1.0 - DAMPING)

        return genesis_this_tick


# ============================================================================
# Main analysis
# ============================================================================

def run_simulation(amplitude, L=16, n_ticks=50, verbose=True):
    """
    Run the genesis throttle model on an L^3 lattice.

    Returns: (total_genesis, genesis_per_tick, field)
    """
    field = SmallLatticeWaveField(L, amplitude)

    if verbose:
        print(f"\n{'='*60}")
        print(f"Genesis Throttle Model: A = {amplitude:.1f}, L = {L}")
        print(f"K_GENESIS = {K_GENESIS:.3f}, K_MANIFEST = {K_MANIFEST:.3f}")
        print(f"Injection energy |J_center|^2 = {(amplitude * K_GENESIS)**2:.2f}")
        print(f"{'='*60}")

    burst_ended = False
    for t in range(n_ticks):
        gen = field.tick()
        if verbose and (gen > 0 or t < 5 or (burst_ended and t < 20)):
            jmag2 = np.sum(field.flux**2, axis=1)
            max_jmag2 = np.max(jmag2)
            manifested = np.sum(field.state != 0)
            print(f"  tick {t:3d}: genesis={gen:3d}  cum={field.total_genesis:4d}  "
                  f"manifested={manifested:4d}  max|J|^2={max_jmag2:.4f}")
        if gen == 0 and field.total_genesis > 0 and not burst_ended:
            burst_ended = True
            if verbose:
                print(f"  --- burst ended at tick {t} ---")

    if verbose:
        print(f"\nRESULT: A={amplitude:.1f} -> N_gen = {field.total_genesis}")

    return field.total_genesis, field.genesis_per_tick, field


def main():
    print("="*70)
    print("FTD-0110 NONLINEAR BRIDGE -- GENESIS THROTTLE MODEL")
    print("="*70)
    print(f"\nOntic constants:")
    print(f"  K_B (m_e)          = {K_B}")
    print(f"  K_MANIFEST         = {K_MANIFEST}")
    print(f"  N_c                = {N_C}")
    print(f"  K_GENESIS          = {K_GENESIS:.3f}")
    print(f"  K_GENESIS_KIN_DRAIN= {K_GENESIS_KINETIC_DRAIN}")
    print(f"  c (CFL speed)      = {C_SPEED:.6f}")
    print(f"  DAMPING            = {DAMPING}")

    # ========================================================================
    # Run the throttle model at the FTD-0267 test amplitudes
    # ========================================================================
    print("\n" + "="*70)
    print("GENESIS THROTTLE PREDICTIONS vs FTD-0267 TELEMETRY")
    print("="*70)

    # FTD-0267 targets (engine CPU, L=64, canonical stack)
    # NOTE: The engine telemetry measured on L=64.  We run on L=16 (fast)
    # and L=32 for validation.  At small A (A<=14, cluster radius < 4),
    # L=16 should suffice.  At A=30 (cluster ~47 voxels, radius ~5),
    # L=16 may show boundary effects.
    targets = {
        9:  {"engine_gen": 3, "seeds": [3], "desc": "single seed"},
        10: {"engine_gen": 4.8, "seeds": [3,7,5,4], "desc": "4 seeds, mean 4.8"},
        14: {"engine_gen": 16.2, "seeds": [15,18,15,17], "desc": "4 seeds, mean 16.2"},
        30: {"engine_gen": 47, "seeds": [47], "desc": "single seed"},
    }

    # Use L=16 for speed (A <= 14 fits easily; A=30 is borderline)
    L_default = 16

    results = {}
    for A in sorted(targets.keys()):
        L = L_default if A <= 14 else 24  # larger lattice for A=30
        n_gen, per_tick, field = run_simulation(A, L=L, n_ticks=60, verbose=True)
        results[A] = n_gen

    # ========================================================================
    # Summary comparison
    # ========================================================================
    print("\n" + "="*70)
    print("SUMMARY: MODEL vs ENGINE")
    print("="*70)
    print(f"{'A':>4s}  {'Model N_gen':>12s}  {'Engine N_gen':>12s}  {'Ratio':>8s}  {'Match?':>8s}")
    print("-"*52)

    all_match = True
    for A in sorted(targets.keys()):
        model = results[A]
        engine = targets[A]["engine_gen"]
        ratio = model / engine if engine > 0 else float('inf')
        match = abs(model - engine) <= max(2, engine * 0.5)
        if not match:
            all_match = False
        print(f"{A:4d}  {model:12d}  {engine:12.1f}  {ratio:8.2f}  {'PASS' if match else 'FAIL':>8s}")

    # k(A) predictions
    print(f"\nk(A) = N_gen(A) / A^2:")
    print(f"{'A':>4s}  {'k_model':>10s}  {'k_engine':>10s}  {'k_linear':>10s}")
    print("-"*40)
    for A in sorted(targets.keys()):
        k_model = results[A] / (A**2)
        k_engine = targets[A]["engine_gen"] / (A**2)
        print(f"{A:4d}  {k_model:10.4f}  {k_engine:10.4f}  {0.25:10.4f}")

    # Extended amplitude sweep
    print("\n" + "="*70)
    print("EXTENDED AMPLITUDE SWEEP (L=16)")
    print("="*70)
    amplitudes = [2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 16, 18, 20]
    print(f"{'A':>4s}  {'N_gen':>8s}  {'k=N/A^2':>10s}  {'k_linear':>10s}  {'Burst ticks':>12s}")
    print("-"*50)
    for A in amplitudes:
        n_gen, per_tick, _ = run_simulation(A, L=16, n_ticks=60, verbose=False)
        k = n_gen / (A**2) if A > 0 else 0
        burst_end = 0
        for t, g in enumerate(per_tick):
            if g > 0:
                burst_end = t + 1
        print(f"{A:4d}  {n_gen:8d}  {k:10.4f}  {0.25:10.4f}  {burst_end:12d}")

    print("\n" + "="*70)
    print("VERDICT")
    print("="*70)
    if all_match:
        print("PASS: ALL FTD-0267 targets matched within tolerance!")
        print("   The genesis-throttle model reproduces the engine's one-shot burst.")
    else:
        print("PARTIAL: Some targets outside tolerance band.")
        print("   Possible causes:")
        print("   - No Gauss projection in model (engine has it)")
        print("   - No Langevin noise in model (engine has stochastic RNG)")
        print("   - Lattice size effects (model uses L=16/24, engine uses L=64)")
        print("   - No coupling/state-flux back-reaction in model")

    print("\n[EPISTEMIC NOTE: This is an EXPLORATORY calculation.")
    print(" Match != derivation. The model must be formally justified")
    print(" against the full engine pipeline before any tag movement.]")


if __name__ == "__main__":
    main()
