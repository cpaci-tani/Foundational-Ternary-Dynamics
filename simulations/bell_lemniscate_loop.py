#!/usr/bin/env python3
"""
Lemniscate-Circle Bell Experiment
=================================

EPISTEMIC STATUS: [EXPLORATION]

Tests whether the dynamically active i — realized as the circle-lemniscate
loop — produces different correlation structure from pure real flux.

Key insight from the user:
  "i is dynamically active in the sense that the circle turns into
   lemniscate via ReLU and the lemniscate turns into the circle.
   It's a loop of loops."

Mathematical basis:
  - Circle S¹: π₁ = Z (abelian) → commutative → S ≤ 2
  - Lemniscate S¹∨S¹: π₁ = F₂ (free group, NON-abelian) → noncommutative
  - The loop between them IS dynamically active i
  - The lemniscate's period lattice is G* · Z[i] (ratio of periods = i)
  - Anti-correlation under lemniscate map requires phase i (not π)

Experiments:
  A. Real flux + sign projection (baseline)
  B. Complex flux + circle measurement: sign(Re(ψ e^{-iα}))
  C. Complex flux + lemniscate measurement: sign(Re(ψ² e^{-2iα}))
     with lemniscatic entanglement phase (π/2 = i)
  D. Lemniscate-circle loop: squaring + threshold + feedback
  E. Correlation shape analysis for all protocols

Key prediction: The lemniscate measurement with i-phase entanglement
preserves anti-correlations (because (iψ)² = -ψ²), while standard
π-phase entanglement does NOT (because (-ψ)² = +ψ²).
This is i acting as a STRUCTURAL REQUIREMENT, not just a number.

Author: Claude Code
Date: February 16, 2026
"""

import numpy as np
import sys
import os
import time

project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
if project_root not in sys.path:
    sys.path.insert(0, project_root)

from ternary_matrix.model.grid import Universe
from ternary_matrix.model.cuboctahedral_geometry import CuboctahedralGeometry
from ternary_matrix.physics import waves, forces
from ternary_matrix.config import CONSTANTS


# ============================================================================
# INFRASTRUCTURE
# ============================================================================

def compute_chsh(E11, E12, E21, E22):
    return abs(E11 - E12) + abs(E21 + E22)


def correlation(oA, oB):
    valid = (oA != 0) & (oB != 0)
    n = np.sum(valid)
    if n == 0:
        return 0.0, 0.0
    return float(np.mean(oA[valid] * oB[valid])), float(n / len(oA))


def random_phases(n):
    """Random complex phases e^{iφ} uniformly on the circle."""
    phi = np.random.uniform(0, 2 * np.pi, n)
    return np.cos(phi), np.sin(phi)  # (real, imag)


def is_valid_fcc(x, y, z):
    return (x + y + z) % 2 == 0


# ============================================================================
# SECTION 1: ANALYTICAL PREDICTIONS (for comparison)
# ============================================================================

def analytical_predictions():
    """
    Compute analytical E(θ) for circle and lemniscate measurements.

    Circle measurement on uniform hidden variable φ:
      E(Δ) = -(1 - 2|Δ|/π)   [triangle function]

    Lemniscate measurement (doubled angle):
      E(Δ) = -(1 - 4|Δ|/π)   for |Δ| ≤ π/2  [steeper triangle]

    Quantum (singlet):
      E(Δ) = -cos(Δ)          [cosine function]
    """
    print("\n" + "=" * 70)
    print("SECTION 1: ANALYTICAL PREDICTIONS")
    print("=" * 70)

    angles = np.linspace(0, np.pi, 19)
    print(f"\n  {'θ°':>6} {'Triangle':>10} {'Steep Tri':>10} "
          f"{'QM -cos':>10} {'S=2 gap':>10}")
    print(f"  {'-' * 48}")

    for theta in angles:
        tri = -(1 - 2 * abs(theta) / np.pi)
        # Steep triangle (lemniscate) — wraps with period π/2
        eff = abs(theta) % (np.pi / 2)
        if eff > np.pi / 4:
            eff = np.pi / 2 - eff
        steep = -(1 - 4 * eff / np.pi)
        qm = -np.cos(theta)
        gap = qm - tri  # How much "rounder" than triangle
        print(f"  {np.degrees(theta):6.1f} {tri:+10.5f} {steep:+10.5f} "
              f"{qm:+10.5f} {gap:+10.5f}")

    # CHSH S for each
    print("\n  CHSH S-values (optimal angles for each):")

    # Triangle: optimal at standard CHSH angles
    a1, a2, b1, b2 = 0, np.pi / 2, np.pi / 4, 3 * np.pi / 4
    E = [-(1 - 2 * abs(a - b) / np.pi) for a, b in
         [(a1, b1), (a1, b2), (a2, b1), (a2, b2)]]
    S_tri = compute_chsh(*E)
    print(f"    Triangle (circle meas):      S = {S_tri:.4f}")

    # Steep triangle: optimal at halved CHSH angles
    a1, a2, b1, b2 = 0, np.pi / 4, np.pi / 8, 3 * np.pi / 8
    E = []
    for a, b in [(a1, b1), (a1, b2), (a2, b1), (a2, b2)]:
        d = abs(a - b)
        eff = d % (np.pi / 2)
        if eff > np.pi / 4:
            eff = np.pi / 2 - eff
        E.append(-(1 - 4 * eff / np.pi))
    S_steep = compute_chsh(*E)
    print(f"    Steep triangle (lem meas):   S = {S_steep:.4f}")

    # Quantum
    a1, a2, b1, b2 = 0, np.pi / 2, np.pi / 4, 3 * np.pi / 4
    E = [-np.cos(a - b) for a, b in
         [(a1, b1), (a1, b2), (a2, b1), (a2, b2)]]
    S_qm = compute_chsh(*E)
    print(f"    Quantum (-cos):              S = {S_qm:.4f}")

    print(f"\n  Classical bound: 2.0000")
    print(f"  Tsirelson bound: {2 * np.sqrt(2):.4f}")


# ============================================================================
# SECTION 2: MONTE CARLO HIDDEN VARIABLE VERIFICATION
# ============================================================================

def monte_carlo_verification(n_trials=50000):
    """
    Verify analytical predictions with Monte Carlo sampling.

    Tests three measurement protocols with shared hidden variable φ:
    1. Circle: sign(cos(φ - α))
    2. Lemniscate (π phase): sign(cos(2φ - 2α)), ψ_B = -ψ_A
    3. Lemniscate (i phase): sign(cos(2φ - 2α)), ψ_B = iψ_A
    """
    print("\n" + "=" * 70)
    print("SECTION 2: MONTE CARLO HIDDEN VARIABLE VERIFICATION")
    print("=" * 70)
    print(f"  Trials: {n_trials}")

    phi = np.random.uniform(0, 2 * np.pi, n_trials)

    protocols = {}

    # --- Protocol 1: Circle measurement, π-phase entanglement ---
    def circle_measure(phi_arr, alpha):
        return np.sign(np.cos(phi_arr - alpha)).astype(int)

    name = "Circle (π-phase)"
    print(f"\n  --- {name} ---")
    angles_chsh = [(0, np.pi / 4), (0, 3 * np.pi / 4),
                   (np.pi / 2, np.pi / 4), (np.pi / 2, 3 * np.pi / 4)]
    E_vals = []
    for a, b in angles_chsh:
        oA = circle_measure(phi, a)
        oB = -circle_measure(phi, b)  # π-phase anti-correlation
        c, _ = correlation(oA, oB)
        E_vals.append(c)
    S = compute_chsh(*E_vals)
    print(f"    E = [{', '.join(f'{e:+.4f}' for e in E_vals)}]")
    print(f"    S = {S:.4f}")
    protocols[name] = {'S': S, 'E': E_vals}

    # --- Protocol 2: Lemniscate measurement, π-phase (BROKEN anti-corr) ---
    def lem_measure(phi_arr, alpha):
        return np.sign(np.cos(2 * phi_arr - 2 * alpha)).astype(int)

    name = "Lemniscate (π-phase, BROKEN)"
    print(f"\n  --- {name} ---")
    E_vals = []
    for a, b in angles_chsh:
        oA = lem_measure(phi, a)
        # π-phase: ψ_B = -ψ_A → ψ_B² = ψ_A² → SAME outcomes (broken!)
        oB = lem_measure(phi, b)  # No negation — anti-corr destroyed
        c, _ = correlation(oA, oB)
        E_vals.append(c)
    S = compute_chsh(*E_vals)
    print(f"    E = [{', '.join(f'{e:+.4f}' for e in E_vals)}]")
    print(f"    S = {S:.4f}")
    print(f"    NOTE: (-ψ)² = ψ² destroys anti-correlation!")
    protocols[name] = {'S': S, 'E': E_vals}

    # --- Protocol 3: Lemniscate measurement, i-phase (PRESERVED anti-corr) ---
    name = "Lemniscate (i-phase, PRESERVED)"
    print(f"\n  --- {name} ---")
    # With ψ_B = iψ_A: ψ_B² = (iψ_A)² = -ψ_A² → anti-correlation preserved!
    # Use optimal angles for doubled-angle regime: halved from standard
    angles_lem = [(0, np.pi / 8), (0, 3 * np.pi / 8),
                  (np.pi / 4, np.pi / 8), (np.pi / 4, 3 * np.pi / 8)]
    E_vals = []
    for a, b in angles_lem:
        oA = lem_measure(phi, a)
        oB = -lem_measure(phi, b)  # i-phase: (iψ)² = -ψ² → negation works
        c, _ = correlation(oA, oB)
        E_vals.append(c)
    S = compute_chsh(*E_vals)
    print(f"    E = [{', '.join(f'{e:+.4f}' for e in E_vals)}]")
    print(f"    S = {S:.4f}")
    print(f"    KEY: (iψ)² = -ψ² PRESERVES anti-correlation!")
    protocols[name] = {'S': S, 'E': E_vals}

    # --- Protocol 4: Born rule measurement (QM reference) ---
    name = "Born rule (QM, for reference)"
    print(f"\n  --- {name} ---")
    E_vals = []
    for a, b in [(0, np.pi / 4), (0, 3 * np.pi / 4),
                 (np.pi / 2, np.pi / 4), (np.pi / 2, 3 * np.pi / 4)]:
        # Born rule: P(+1|α) = cos²((φ-α)/2)
        # Outcome: +1 with prob cos²((φ-α)/2), -1 with prob sin²((φ-α)/2)
        pA = np.cos((phi - a) / 2) ** 2
        oA = np.where(np.random.random(n_trials) < pA, 1, -1)
        pB = np.cos((phi - b) / 2) ** 2
        oB = np.where(np.random.random(n_trials) < pB, 1, -1)
        oB = -oB  # singlet anti-correlation
        c, _ = correlation(oA, oB)
        E_vals.append(c)
    S = compute_chsh(*E_vals)
    print(f"    E = [{', '.join(f'{e:+.4f}' for e in E_vals)}]")
    print(f"    S = {S:.4f}")
    print(f"    NOTE: This imports QM probability rule (not from lattice)")
    protocols[name] = {'S': S, 'E': E_vals}

    return protocols


# ============================================================================
# SECTION 3: COMPLEX LATTICE PROPAGATION
# ============================================================================

def create_complex_pair(universe, phase_type='pi'):
    """
    Create entangled pair with complex flux structure.

    The complex flux is ψ = Jx + iJy (first two components of flux vector).
    Jz carries no complex information and is set to 0.

    phase_type:
      'pi':  ψ_B = -ψ_A  (standard, anti-correlated)
      'i':   ψ_B = iψ_A  (lemniscatic, 90° phase shift)
             Jx_B = -Jy_A, Jy_B = Jx_A (rotation by π/2)
    """
    c = universe.size // 2
    is_fcc = universe.geometry.name == 'cuboctahedral'

    # Random complex phase for A
    phi = np.random.uniform(0, 2 * np.pi)
    amp = 5.0
    Jx_A = amp * np.cos(phi)
    Jy_A = amp * np.sin(phi)

    # Particle A at center
    universe.states[c, c, c] = 1
    universe.flux[c, c, c] = np.array([Jx_A, Jy_A, 0.0], dtype=np.float32)
    universe.charge[c, c, c] = 1.0

    # Particle B at neighbor
    if is_fcc:
        bx, by, bz = c + 1, c + 1, c
    else:
        bx, by, bz = c + 1, c, c

    universe.states[bx, by, bz] = -1

    if phase_type == 'pi':
        # ψ_B = -ψ_A: standard anti-correlation
        universe.flux[bx, by, bz] = np.array([-Jx_A, -Jy_A, 0.0],
                                              dtype=np.float32)
    elif phase_type == 'i':
        # ψ_B = iψ_A: lemniscatic phase
        # i(Jx + iJy) = -Jy + iJx → Jx_B = -Jy_A, Jy_B = Jx_A
        universe.flux[bx, by, bz] = np.array([-Jy_A, Jx_A, 0.0],
                                              dtype=np.float32)
    else:
        raise ValueError(f"Unknown phase type: {phase_type}")

    universe.charge[bx, by, bz] = -1.0

    # Velocities to separate
    sep = np.array([1, 0, 0], dtype=np.float32)
    if is_fcc:
        sep = np.array([1, 1, 0], dtype=np.float32) / np.sqrt(2)
    universe.velocity[c, c, c] = -0.4 * sep
    universe.velocity[bx, by, bz] = 0.4 * sep


def measure_circle(universe, center, radius, alpha):
    """
    Circle measurement: sign(Re(ψ e^{-iα}))
    = sign(Jx cos α + Jy sin α)
    """
    cx, cy, cz = center
    s = universe.size
    region = universe.flux[
        max(0, cx - radius):min(s, cx + radius + 1),
        max(0, cy - radius):min(s, cy + radius + 1),
        max(0, cz - radius):min(s, cz + radius + 1)
    ]
    total = np.sum(region, axis=(0, 1, 2))
    # Project onto complex measurement axis
    proj = total[0] * np.cos(alpha) + total[1] * np.sin(alpha)
    if abs(proj) < 1e-10:
        return 0
    return int(np.sign(proj))


def measure_lemniscate(universe, center, radius, alpha):
    """
    Lemniscate measurement: sign(Re(ψ² e^{-2iα}))

    ψ² = (Jx + iJy)² = (Jx² - Jy²) + 2iJxJy
    Re(ψ² e^{-2iα}) = (Jx²-Jy²)cos(2α) + 2JxJy sin(2α)

    This is a QUADRATIC function of ψ — the lemniscate map.
    It doubles the angle (circle → lemniscate → double-covered circle).
    """
    cx, cy, cz = center
    s = universe.size
    region = universe.flux[
        max(0, cx - radius):min(s, cx + radius + 1),
        max(0, cy - radius):min(s, cy + radius + 1),
        max(0, cz - radius):min(s, cz + radius + 1)
    ]
    total = np.sum(region, axis=(0, 1, 2))
    Jx, Jy = float(total[0]), float(total[1])

    # ψ² = (Jx² - Jy²) + 2iJxJy
    re_psi2 = Jx ** 2 - Jy ** 2
    im_psi2 = 2 * Jx * Jy

    # Re(ψ² e^{-2iα}) = re_ψ² cos(2α) + im_ψ² sin(2α)
    proj = re_psi2 * np.cos(2 * alpha) + im_psi2 * np.sin(2 * alpha)

    if abs(proj) < 1e-10:
        return 0
    return int(np.sign(proj))


def measure_lemniscate_loop(universe, center, radius, alpha,
                            n_feedback=2):
    """
    Lemniscate-circle loop measurement:
    1. Read ψ at detector
    2. Apply ψ → ψ² (lemniscate map — enter non-abelian phase)
    3. Apply ReLU: manifest based on Re(ψ²)
    4. Feed back into lattice (the measurement changes the state)
    5. Propagate (return to circle phase)
    6. Read final state

    This implements the full loop: Circle → Lemniscate → Circle
    """
    cx, cy, cz = center
    s = universe.size

    # Step 1: Read ψ
    region = universe.flux[
        max(0, cx - radius):min(s, cx + radius + 1),
        max(0, cy - radius):min(s, cy + radius + 1),
        max(0, cz - radius):min(s, cz + radius + 1)
    ]
    total = np.sum(region, axis=(0, 1, 2))
    Jx, Jy = float(total[0]), float(total[1])

    # Step 2: Lemniscate map ψ → ψ²
    re_psi2 = Jx ** 2 - Jy ** 2
    im_psi2 = 2 * Jx * Jy

    # Step 3: ReLU — project onto measurement axis, threshold
    proj = re_psi2 * np.cos(2 * alpha) + im_psi2 * np.sin(2 * alpha)
    outcome = 0 if abs(proj) < 1e-10 else int(np.sign(proj))

    # Step 4: Feedback — place the squared field back at detector
    # This creates a SOURCE of the lemniscate-mapped flux
    feedback_strength = 0.5
    new_Jx = (re_psi2 * np.cos(2 * alpha) +
              im_psi2 * np.sin(2 * alpha)) * feedback_strength
    new_Jy = (-re_psi2 * np.sin(2 * alpha) +
              im_psi2 * np.cos(2 * alpha)) * feedback_strength
    universe.flux[cx, cy, cz, 0] += new_Jx
    universe.flux[cx, cy, cz, 1] += new_Jy
    universe.states[cx, cy, cz] = outcome if outcome != 0 else 1
    universe.is_locked[cx, cy, cz] = True

    # Step 5: Propagate (circle phase with modified source)
    for _ in range(n_feedback):
        waves.propagate_flux(universe)
        forces.calculate_density(universe)

    # Step 6: Re-read (the loop has completed one cycle)
    final_result = measure_circle(universe, center, radius, alpha)

    return final_result if final_result != 0 else outcome


# ============================================================================
# SECTION 4: LATTICE BELL TESTS
# ============================================================================

def run_lattice_bell(meas_fn, phase_type, angles, n_trials=300,
                     grid_size=16, n_ticks=12, label=""):
    """
    Run CHSH test on cuboctahedral lattice with specified measurement
    function and entanglement phase.
    """
    original_damping = CONSTANTS.DAMPING
    CONSTANTS.DAMPING = 0.0

    all_corr = {}

    for name, alpha, beta in [
        ('E11', angles[0], angles[2]),
        ('E12', angles[0], angles[3]),
        ('E21', angles[1], angles[2]),
        ('E22', angles[1], angles[3]),
    ]:
        oA = np.zeros(n_trials, dtype=int)
        oB = np.zeros(n_trials, dtype=int)

        t0 = time.time()
        for trial in range(n_trials):
            u = Universe(size=grid_size,
                         geometry=CuboctahedralGeometry(grid_size))
            c = u.size // 2
            off = u.size // 4
            det_A = (c - off, c, c)
            det_B = (c + off, c, c)

            create_complex_pair(u, phase_type=phase_type)

            for _ in range(n_ticks):
                waves.propagate_flux(u)
                forces.calculate_density(u)

            # Measure A and B on separate copies (no cross-contamination)
            if meas_fn.__name__ == 'measure_lemniscate_loop':
                # Loop measurement modifies state — use copies
                u_A = Universe(size=grid_size,
                               geometry=CuboctahedralGeometry(grid_size))
                u_A.flux[:] = u.flux
                u_A.wave_velocity[:] = u.wave_velocity
                u_A.states[:] = u.states

                u_B = Universe(size=grid_size,
                               geometry=CuboctahedralGeometry(grid_size))
                u_B.flux[:] = u.flux
                u_B.wave_velocity[:] = u.wave_velocity
                u_B.states[:] = u.states

                oA[trial] = meas_fn(u_A, det_A, 3, alpha)
                oB[trial] = meas_fn(u_B, det_B, 3, beta)
            else:
                oA[trial] = meas_fn(u, det_A, 3, alpha)
                oB[trial] = meas_fn(u, det_B, 3, beta)

            # Anti-correlation convention:
            # For π-phase: ψ_B = -ψ_A → circle measurement auto-negates
            # For i-phase: ψ_B = iψ_A → lemniscate measurement auto-negates
            #   because (iψ)² = -ψ², so Re(ψ_B² e^{-2iβ}) = -Re(ψ_A² e^{-2iβ})
            # No explicit negation needed if phase encodes anti-correlation

        c_val, eff = correlation(oA, oB)
        elapsed = time.time() - t0
        all_corr[name] = {'correlation': c_val, 'efficiency': eff}
        print(f"    {name} = {c_val:+.4f}  (eff: {eff:.2%})  [{elapsed:.1f}s]")

    S = compute_chsh(
        all_corr['E11']['correlation'],
        all_corr['E12']['correlation'],
        all_corr['E21']['correlation'],
        all_corr['E22']['correlation'],
    )
    status = "*** VIOLATION ***" if S > 2.0 else "(no violation)"
    print(f"    S = {S:.4f}  {status}")

    CONSTANTS.DAMPING = original_damping
    return {'S': S, 'correlations': all_corr, 'label': label}


def lattice_experiments(n_trials=300, grid_size=16, n_ticks=12):
    """Run all lattice Bell experiments."""
    print("\n" + "=" * 70)
    print("SECTION 3: LATTICE BELL EXPERIMENTS (CUBOCTAHEDRAL)")
    print("=" * 70)
    print(f"  Grid: {grid_size} (FCC: {2*grid_size}), "
          f"Trials: {n_trials}, Ticks: {n_ticks}")

    results = {}

    # Standard CHSH angles
    std = [0, np.pi / 2, np.pi / 4, 3 * np.pi / 4]
    # Halved CHSH angles (optimal for lemniscate doubled-angle)
    lem = [0, np.pi / 4, np.pi / 8, 3 * np.pi / 8]

    configs = [
        # (measurement_fn, phase, angles, label)
        (measure_circle, 'pi', std,
         "A: Circle meas + π-phase (baseline)"),
        (measure_circle, 'i', std,
         "B: Circle meas + i-phase"),
        (measure_lemniscate, 'pi', std,
         "C: Lemniscate meas + π-phase (broken anti-corr)"),
        (measure_lemniscate, 'i', lem,
         "D: Lemniscate meas + i-phase (preserved anti-corr)"),
        (measure_lemniscate, 'i', std,
         "E: Lemniscate meas + i-phase (std angles)"),
        (measure_lemniscate_loop, 'i', lem,
         "F: Lemniscate LOOP + i-phase (full circle-lem cycle)"),
    ]

    for meas_fn, phase, angles, label in configs:
        print(f"\n  --- {label} ---")
        r = run_lattice_bell(meas_fn, phase, angles, n_trials,
                             grid_size, n_ticks, label)
        results[label] = r

    return results


# ============================================================================
# SECTION 5: CORRELATION SHAPE SWEEP
# ============================================================================

def correlation_shape_sweep(n_angles=24, n_trials=500, grid_size=16, n_ticks=12):
    """
    Sweep E(θ) for complex measurement protocols.

    Compare shapes to identify whether any protocol produces
    "rounder" correlations (approaching cosine) vs the triangle.
    """
    print("\n" + "=" * 70)
    print("SECTION 4: CORRELATION SHAPE E(θ)")
    print("=" * 70)
    print(f"  Angles: {n_angles}, Trials/angle: {n_trials}")

    original_damping = CONSTANTS.DAMPING
    CONSTANTS.DAMPING = 0.0

    angles = np.linspace(0, np.pi, n_angles)
    shapes = {}

    configs = [
        ("circle_pi", measure_circle, 'pi'),
        ("circle_i", measure_circle, 'i'),
        ("lem_i", measure_lemniscate, 'i'),
    ]

    for name, meas_fn, phase in configs:
        corrs = []
        print(f"  {name}: ", end="", flush=True)

        for theta in angles:
            oA = np.zeros(n_trials, dtype=int)
            oB = np.zeros(n_trials, dtype=int)

            for trial in range(n_trials):
                u = Universe(size=grid_size,
                             geometry=CuboctahedralGeometry(grid_size))
                c = u.size // 2
                off = u.size // 4
                det_A = (c - off, c, c)
                det_B = (c + off, c, c)

                create_complex_pair(u, phase_type=phase)

                for _ in range(n_ticks):
                    waves.propagate_flux(u)
                    forces.calculate_density(u)

                oA[trial] = meas_fn(u, det_A, 3, 0.0)  # Alice fixed at 0
                oB[trial] = meas_fn(u, det_B, 3, theta)  # Bob sweeps

            c_val, _ = correlation(oA, oB)
            corrs.append(c_val)
            print(".", end="", flush=True)

        shapes[name] = np.array(corrs)
        print(" done")

    # Print comparison
    print(f"\n  {'θ°':>6} {'circ_π':>8} {'circ_i':>8} {'lem_i':>8} "
          f"{'QM':>8} {'tri':>8}")
    print(f"  {'-' * 50}")

    for i, theta in enumerate(angles):
        qm = -np.cos(theta)
        tri = -(1 - 2 * abs(theta) / np.pi)
        c_pi = shapes['circle_pi'][i]
        c_i = shapes['circle_i'][i]
        l_i = shapes['lem_i'][i]
        print(f"  {np.degrees(theta):6.1f} {c_pi:+.4f} {c_i:+.4f} "
              f"{l_i:+.4f} {qm:+.4f} {tri:+.4f}")

    CONSTANTS.DAMPING = original_damping
    return angles, shapes


# ============================================================================
# MAIN
# ============================================================================

def main():
    np.random.seed(42)

    print("=" * 70)
    print("LEMNISCATE-CIRCLE BELL EXPERIMENT")
    print("Is i Dynamically Active in the Circle-Lemniscate Loop?")
    print("=" * 70)
    print()
    print("The hypothesis:")
    print("  Circle (S¹) has π₁ = Z (abelian) → S ≤ 2")
    print("  Lemniscate (S¹∨S¹) has π₁ = F₂ (non-abelian)")
    print("  The LOOP between them is dynamically active i")
    print()
    print("Key mathematical fact:")
    print("  (-ψ)² = +ψ²  → π-phase entanglement BREAKS under squaring")
    print("  (iψ)² = -ψ²  → i-phase entanglement SURVIVES under squaring")
    print("  ∴ The lemniscate map REQUIRES i for anti-correlation!")
    print()

    # Section 1: Analytical predictions
    analytical_predictions()

    # Section 2: Monte Carlo verification
    mc_results = monte_carlo_verification(n_trials=50000)

    # Section 3: Lattice experiments
    lattice_results = lattice_experiments(
        n_trials=300, grid_size=16, n_ticks=12
    )

    # Section 4: Correlation shape
    angles, shapes = correlation_shape_sweep(
        n_angles=18, n_trials=300, grid_size=16, n_ticks=12
    )

    # ================================================================
    # SYNTHESIS
    # ================================================================
    print("\n" + "=" * 70)
    print("SYNTHESIS")
    print("=" * 70)

    print("\n  S-VALUES ACROSS ALL PROTOCOLS:")
    print(f"  {'Protocol':<55} {'S':>8}")
    print(f"  {'-' * 63}")

    for name, data in mc_results.items():
        print(f"  MC: {name:<50} {data['S']:8.4f}")

    for label, data in lattice_results.items():
        short = label[:50]
        print(f"  {short:<55} {data['S']:8.4f}")

    max_S = max(
        max(d['S'] for d in mc_results.values()),
        max(d['S'] for d in lattice_results.values()),
    )

    print(f"\n  Classical bound: S ≤ 2.0000")
    print(f"  Tsirelson bound: S ≤ {2 * np.sqrt(2):.4f}")
    print(f"  Maximum observed: S = {max_S:.4f}")

    print("\n" + "=" * 70)
    print("KEY FINDINGS")
    print("=" * 70)
    print("""
  1. ANTI-CORRELATION STRUCTURE:
     (-ψ)² = +ψ²  →  π-phase breaks under lemniscate map
     (iψ)² = -ψ²  →  i-phase survives under lemniscate map
     ∴ The lemniscate REQUIRES i to maintain entanglement.
     This is not a choice — it's a mathematical necessity.

  2. TOPOLOGICAL INSIGHT:
     Circle: π₁(S¹) = Z (abelian) → commutative → S ≤ 2
     Lemniscate: π₁(S¹∨S¹) = F₂ (non-abelian) → noncommutative
     The self-crossing creates path-dependent operations.
     But path-dependence ≠ Bell violation (Bell is about CORRELATIONS
     between spacelike-separated events, not sequential paths).

  3. MEASUREMENT FUNCTIONS:
     sign(Re(ψ e^{-iα})):     circle measurement, triangle E(θ)
     sign(Re(ψ² e^{-2iα})):   lemniscate measurement, steep triangle E(θ)
     Both give S ≤ 2 because both are deterministic functions of
     the hidden variable φ = arg(ψ). Bell's theorem is absolute.

  4. THE DEEP POINT:
     The lemniscate doesn't give S > 2 as a MEASUREMENT FUNCTION.
     But it reveals WHY i is structurally necessary:
     - i is the entanglement phase that survives the self-referential map
     - Without i, self-reference (ψ → ψ²) destroys correlations
     - With i, self-reference preserves them
     - This is i as STRUCTURAL NECESSITY, not computational convenience

  5. WHAT REMAINS:
     The transition from S = 2 to S = 2√2 requires not just i in the
     phase, but i in the PROBABILITY RULE:
       Classical: P = deterministic sign function → S ≤ 2
       Quantum:   P = |⟨ψ|φ⟩|² (complex inner product) → S ≤ 2√2
     The Born rule itself may be the "ReLU of the lemniscate" —
     the threshold operation that makes the loop of loops physical.
""")

    print("=" * 70)
    print("END OF LEMNISCATE-CIRCLE BELL EXPERIMENT")
    print("=" * 70)


if __name__ == "__main__":
    main()
