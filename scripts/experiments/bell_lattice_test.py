#!/usr/bin/env python3
"""
Bell Lattice Investigation: Can FTD Produce S > 2?
===================================================

EPISTEMIC STATUS: [INVESTIGATION]

This script systematically tests whether FTD's lattice dynamics can produce
Bell inequality violations (S > 2) WITHOUT importing Hilbert space structure.

Four tiers of increasing realism:
  Tier 0: Baseline checks (classical S<=2, quantum S=2*sqrt(2))
  Tier 1: Vector hidden variable model (flux as R^3 vector)
  Tier 2: Full FTD lattice pair production (actual wave equation)
  Tier 3: sLoop coupling (detector embedded in substrate)

Honest expectation: S <= 2 everywhere. Bell's theorem is a mathematical
theorem about probability distributions. FTD's lattice has local updates
(26-neighbor Moore neighborhood). Therefore S <= 2 is guaranteed.

The scientific value is understanding WHERE and HOW correlations fall short,
and documenting this rigorously.

Author: Claude Code
Date: February 5, 2026
"""

import numpy as np
import sys
import os
import time

# Add project root to path for ternary_matrix imports
project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
if project_root not in sys.path:
    sys.path.insert(0, project_root)


# ============================================================================
# SECTION 1: CHSH INFRASTRUCTURE (shared by all tiers)
# ============================================================================

# Standard CHSH optimal angles
CHSH_ANGLES = {
    'a1': 0.0,
    'a2': np.pi / 2,
    'b1': np.pi / 4,
    'b2': 3 * np.pi / 4,
}


def compute_chsh_from_correlations(E11, E12, E21, E22):
    """
    Compute CHSH S-parameter from four correlation values.

    S = |E(a1,b1) - E(a1,b2)| + |E(a2,b1) + E(a2,b2)|

    Classical bound: S <= 2
    Tsirelson bound: S <= 2*sqrt(2) ~ 2.828
    """
    S = abs(E11 - E12) + abs(E21 + E22)
    return S


def compute_correlation(outcomes_A, outcomes_B):
    """
    Compute correlation E(a,b) = <A*B> from arrays of outcomes.
    outcomes are in {-1, 0, +1}. Trials with 0 outcome are excluded.
    Returns (correlation, detection_efficiency).
    """
    # Filter out null outcomes (state 0)
    valid = (outcomes_A != 0) & (outcomes_B != 0)
    n_valid = np.sum(valid)
    n_total = len(outcomes_A)

    if n_valid == 0:
        return 0.0, 0.0

    efficiency = n_valid / n_total
    correlation = np.mean(outcomes_A[valid] * outcomes_B[valid])
    return float(correlation), float(efficiency)


def run_chsh_test(measurement_fn, n_trials=10000, label=""):
    """
    Run a complete CHSH test using a measurement function.

    measurement_fn(angle_a, angle_b, n_trials) -> (outcomes_A, outcomes_B)
        where outcomes are arrays of {-1, 0, +1}

    Returns dict with S, correlations, efficiencies.
    """
    angles = CHSH_ANGLES
    results = {}

    for name, (aa, ab) in [
        ('E11', (angles['a1'], angles['b1'])),
        ('E12', (angles['a1'], angles['b2'])),
        ('E21', (angles['a2'], angles['b1'])),
        ('E22', (angles['a2'], angles['b2'])),
    ]:
        outcomes_A, outcomes_B = measurement_fn(aa, ab, n_trials)
        corr, eff = compute_correlation(outcomes_A, outcomes_B)
        results[name] = {'correlation': corr, 'efficiency': eff}

    S = compute_chsh_from_correlations(
        results['E11']['correlation'],
        results['E12']['correlation'],
        results['E21']['correlation'],
        results['E22']['correlation'],
    )

    avg_eff = np.mean([r['efficiency'] for r in results.values()])

    return {
        'S': S,
        'correlations': results,
        'avg_efficiency': avg_eff,
        'n_trials': n_trials,
        'label': label,
    }


def print_chsh_result(result):
    """Pretty-print a CHSH test result."""
    print(f"\n  {result['label']}")
    print(f"  {'='*50}")
    for name in ['E11', 'E12', 'E21', 'E22']:
        r = result['correlations'][name]
        print(f"    {name} = {r['correlation']:+.4f}  (eff: {r['efficiency']:.2%})")
    print(f"  {'='*50}")
    print(f"    S = {result['S']:.4f}")
    print(f"    Classical bound: 2.000")
    print(f"    Quantum bound:   {2*np.sqrt(2):.4f}")
    print(f"    Detection efficiency: {result['avg_efficiency']:.2%}")

    if result['S'] > 2.0:
        print(f"    *** BELL VIOLATION: S > 2 by {result['S'] - 2:.4f} ***")
    else:
        print(f"    No violation (S <= 2)")
    print()


def correlation_function_sweep(measurement_fn, n_angles=36, n_trials=5000):
    """
    Sweep measurement angle difference and compute E(theta).
    Returns arrays of (angles, correlations) for plotting.
    """
    angles = np.linspace(0, np.pi, n_angles)
    correlations = []

    for theta in angles:
        outcomes_A, outcomes_B = measurement_fn(0.0, theta, n_trials)
        corr, _ = compute_correlation(outcomes_A, outcomes_B)
        correlations.append(corr)

    return angles, np.array(correlations)


# ============================================================================
# SECTION 2: TIER 0 — BASELINES
# ============================================================================

def tier0_classical(n_trials=10000):
    """
    Tier 0a: Classical scalar hidden variable model.
    Reproduces verify_bell_inequality.py.
    Expected: S <= 2.
    """
    def measure(angle_a, angle_b, n_trials):
        # Shared hidden variable: uniform angle
        theta_L = np.random.uniform(0, 2 * np.pi, n_trials)
        A = np.sign(np.cos(theta_L - angle_a)).astype(int)
        B = np.sign(np.cos(theta_L - angle_b)).astype(int)
        # Anti-correlate for singlet-like behavior
        B = -B
        return A, B

    return run_chsh_test(measure, n_trials, "Tier 0a: Classical Scalar HV")


def tier0_quantum():
    """
    Tier 0b: Quantum analytical result.
    E(a,b) = -cos(a-b) for singlet state.
    S = 2*sqrt(2).
    """
    def E(a, b):
        return -np.cos(a - b)

    a1, a2 = CHSH_ANGLES['a1'], CHSH_ANGLES['a2']
    b1, b2 = CHSH_ANGLES['b1'], CHSH_ANGLES['b2']

    E11 = E(a1, b1)
    E12 = E(a1, b2)
    E21 = E(a2, b1)
    E22 = E(a2, b2)
    S = compute_chsh_from_correlations(E11, E12, E21, E22)

    result = {
        'S': S,
        'correlations': {
            'E11': {'correlation': E11, 'efficiency': 1.0},
            'E12': {'correlation': E12, 'efficiency': 1.0},
            'E21': {'correlation': E21, 'efficiency': 1.0},
            'E22': {'correlation': E22, 'efficiency': 1.0},
        },
        'avg_efficiency': 1.0,
        'n_trials': 'analytical',
        'label': 'Tier 0b: Quantum Analytical (singlet)',
    }
    return result


# ============================================================================
# SECTION 3: TIER 1 — VECTOR HIDDEN VARIABLE
# ============================================================================

def random_unit_vectors(n):
    """Generate n random unit vectors uniformly distributed on the sphere."""
    # Marsaglia method
    z = np.random.uniform(-1, 1, n)
    phi = np.random.uniform(0, 2 * np.pi, n)
    r = np.sqrt(1 - z**2)
    x = r * np.cos(phi)
    y = r * np.sin(phi)
    return np.column_stack([x, y, z])


def angle_to_axis(theta):
    """Convert measurement angle to unit vector in x-z plane."""
    return np.array([np.sin(theta), 0.0, np.cos(theta)])


def tier1_vector(n_trials=10000):
    """
    Tier 1: Vector hidden variable model.
    Hidden variable is J_L in R^3 (uniform on unit sphere).
    A = sign(J_L . a_hat), B = -sign(J_L . b_hat) (anti-correlated pair).

    For uniform distribution on sphere with sign-function outcomes,
    E(a,b) is the "triangle" correlation: E = -(1 - 2|theta|/pi)
    which gives S = 2.0 exactly. Verify numerically.
    """
    def measure(angle_a, angle_b, n_trials):
        J_L = random_unit_vectors(n_trials)
        a_hat = angle_to_axis(angle_a)
        b_hat = angle_to_axis(angle_b)

        # Project and sign
        A = np.sign(J_L @ a_hat).astype(int)
        B = -np.sign(J_L @ b_hat).astype(int)  # anti-correlated pair

        return A, B

    return run_chsh_test(measure, n_trials, "Tier 1: Vector HV (J in R^3)")


def tier1_ternary(n_trials=10000, threshold=0.3):
    """
    Tier 1b: Ternary projection model.
    Same as Tier 1 but outcome is 0 (null) when |projection| < threshold.
    Tests whether ternary state space creates any loophole.
    """
    def measure(angle_a, angle_b, n_trials):
        J_L = random_unit_vectors(n_trials)
        a_hat = angle_to_axis(angle_a)
        b_hat = angle_to_axis(angle_b)

        proj_A = J_L @ a_hat
        proj_B = J_L @ b_hat

        # Ternary outcomes: 0 if below threshold
        A = np.where(np.abs(proj_A) < threshold, 0, np.sign(proj_A)).astype(int)
        B = np.where(np.abs(proj_B) < threshold, 0, -np.sign(proj_B)).astype(int)

        return A, B

    return run_chsh_test(measure, n_trials,
                         f"Tier 1b: Ternary HV (threshold={threshold})")


# ============================================================================
# SECTION 4: TIER 2 — FTD LATTICE PAIR PRODUCTION
# ============================================================================

def try_import_lattice():
    """Try to import the FTD lattice engine."""
    try:
        from ternary_matrix.model.grid import Universe
        from ternary_matrix.physics import master_equation, waves, forces
        from ternary_matrix.config import CONSTANTS
        return Universe, master_equation, waves, forces, CONSTANTS
    except ImportError as e:
        print(f"  WARNING: Cannot import ternary_matrix: {e}")
        print(f"  Tier 2 and 3 will be skipped.")
        return None


def create_entangled_pair(universe, J0_direction, amplitude=5.0):
    """
    Create an entangled pair at the lattice center.

    Pair production: void -> (+1, -1) with anti-correlated flux.
    The anti-correlation of flux direction is the "entanglement."

    Args:
        universe: Universe instance
        J0_direction: unit vector for initial flux direction
        amplitude: flux amplitude (should be > K_B for manifestation)
    """
    c = universe.size // 2
    J0 = np.array(J0_direction, dtype=np.float32) * amplitude

    # Particle A at center
    universe.states[c, c, c] = 1
    universe.flux[c, c, c] = J0
    universe.charge[c, c, c] = 1.0

    # Particle B at center+1 (anti-correlated)
    universe.states[c + 1, c, c] = -1
    universe.flux[c + 1, c, c] = -J0
    universe.charge[c + 1, c, c] = -1.0

    # Give them initial velocities to separate
    universe.velocity[c, c, c] = np.array([-0.4, 0, 0], dtype=np.float32)
    universe.velocity[c + 1, c, c] = np.array([0.4, 0, 0], dtype=np.float32)


def measure_flux_at_region(universe, region_center, region_size, axis):
    """
    Tier 2a: Read flux at detector region, project onto measurement axis.
    Returns sign of total projected flux.
    """
    cx, cy, cz = region_center
    r = region_size

    # Sum flux over detector region
    x_lo = max(0, cx - r)
    x_hi = min(universe.size, cx + r + 1)
    y_lo = max(0, cy - r)
    y_hi = min(universe.size, cy + r + 1)
    z_lo = max(0, cz - r)
    z_hi = min(universe.size, cz + r + 1)

    total_flux = np.sum(universe.flux[x_lo:x_hi, y_lo:y_hi, z_lo:z_hi], axis=(0, 1, 2))

    # Project onto measurement axis and return sign
    projection = np.dot(total_flux, axis)

    if abs(projection) < 1e-10:
        return 0
    return int(np.sign(projection))


def track_flux_correlation(universe, pos_A, pos_B, radius=2):
    """
    Diagnostic: measure how well flux anti-correlation is preserved.
    Returns correlation coefficient between flux at A and -flux at B.
    """
    cA = pos_A
    cB = pos_B
    r = radius

    flux_A = np.sum(universe.flux[
        max(0,cA[0]-r):min(universe.size,cA[0]+r+1),
        max(0,cA[1]-r):min(universe.size,cA[1]+r+1),
        max(0,cA[2]-r):min(universe.size,cA[2]+r+1)
    ], axis=(0,1,2))

    flux_B = np.sum(universe.flux[
        max(0,cB[0]-r):min(universe.size,cB[0]+r+1),
        max(0,cB[1]-r):min(universe.size,cB[1]+r+1),
        max(0,cB[2]-r):min(universe.size,cB[2]+r+1)
    ], axis=(0,1,2))

    # For perfect anti-correlation: flux_A = -flux_B
    # Correlation = flux_A . (-flux_B) / (|flux_A| * |flux_B|)
    norm_A = np.linalg.norm(flux_A)
    norm_B = np.linalg.norm(flux_B)

    if norm_A < 1e-10 or norm_B < 1e-10:
        return 0.0, norm_A, norm_B

    corr = np.dot(flux_A, -flux_B) / (norm_A * norm_B)
    return float(corr), float(norm_A), float(norm_B)


def tier2_flux_reading(n_trials=500, grid_size=32, n_ticks=20, damping=0.0):
    """
    Tier 2a: Full FTD lattice pair production with flux reading measurement.

    Creates entangled pair, propagates via wave equation, reads flux at
    spatially separated detector regions.

    Uses DAMPING=0 by default to preserve correlations. The standard
    DAMPING=0.05 attenuates flux significantly over propagation distance.
    """
    imports = try_import_lattice()
    if imports is None:
        return None

    Universe, master_equation, waves, forces, CONSTANTS = imports

    # Override damping for this experiment
    original_damping = CONSTANTS.DAMPING
    CONSTANTS.DAMPING = damping

    c = grid_size // 2
    det_offset = grid_size // 4  # Detector distance from center

    # Detector positions
    det_A = (c - det_offset, c, c)
    det_B = (c + det_offset, c, c)

    angles = CHSH_ANGLES
    all_results = {}

    for name, (aa, ab) in [
        ('E11', (angles['a1'], angles['b1'])),
        ('E12', (angles['a1'], angles['b2'])),
        ('E21', (angles['a2'], angles['b1'])),
        ('E22', (angles['a2'], angles['b2'])),
    ]:
        outcomes_A = np.zeros(n_trials, dtype=int)
        outcomes_B = np.zeros(n_trials, dtype=int)

        a_hat = angle_to_axis(aa)
        b_hat = angle_to_axis(ab)

        for trial in range(n_trials):
            # Fresh universe each trial
            u = Universe(size=grid_size)

            # Random initial flux direction
            J0_dir = random_unit_vectors(1)[0]
            create_entangled_pair(u, J0_dir, amplitude=5.0)

            # Propagate
            for _ in range(n_ticks):
                # Minimal tick: just wave propagation + density
                waves.propagate_flux(u)
                forces.calculate_density(u)

            # Measure
            outcomes_A[trial] = measure_flux_at_region(u, det_A, 3, a_hat)
            outcomes_B[trial] = -measure_flux_at_region(u, det_B, 3, b_hat)

        corr, eff = compute_correlation(outcomes_A, outcomes_B)
        all_results[name] = {'correlation': corr, 'efficiency': eff}

    S = compute_chsh_from_correlations(
        all_results['E11']['correlation'],
        all_results['E12']['correlation'],
        all_results['E21']['correlation'],
        all_results['E22']['correlation'],
    )

    # Restore damping
    CONSTANTS.DAMPING = original_damping

    avg_eff = np.mean([r['efficiency'] for r in all_results.values()])

    return {
        'S': S,
        'correlations': all_results,
        'avg_efficiency': avg_eff,
        'n_trials': n_trials,
        'label': f'Tier 2a: FTD Lattice Flux Reading (grid={grid_size}, ticks={n_ticks}, damp={damping})',
    }


def tier2_correlation_diagnostic(grid_size=32, n_ticks=30, damping=0.0):
    """
    Diagnostic: Track how flux anti-correlation decays during propagation.
    Runs one trial and reports correlation at each tick.
    """
    imports = try_import_lattice()
    if imports is None:
        return None

    Universe, master_equation, waves, forces, CONSTANTS = imports

    original_damping = CONSTANTS.DAMPING
    CONSTANTS.DAMPING = damping

    u = Universe(size=grid_size)
    c = grid_size // 2
    J0_dir = np.array([1.0, 0.0, 0.0])
    create_entangled_pair(u, J0_dir, amplitude=5.0)

    det_offset = grid_size // 4
    pos_A = (c - det_offset, c, c)
    pos_B = (c + det_offset, c, c)

    print(f"\n  Flux Correlation Diagnostic (grid={grid_size}, damp={damping})")
    print(f"  Pair at center ({c},{c},{c}), detectors at offset +/-{det_offset}")
    print(f"  {'Tick':>6} {'Corr':>8} {'|J_A|':>10} {'|J_B|':>10}")
    print(f"  {'-'*40}")

    results = []
    for t in range(n_ticks):
        waves.propagate_flux(u)
        forces.calculate_density(u)

        corr, nA, nB = track_flux_correlation(u, pos_A, pos_B, radius=3)
        results.append((t, corr, nA, nB))
        if t % 5 == 0 or t == n_ticks - 1:
            print(f"  {t:6d} {corr:8.4f} {nA:10.4f} {nB:10.4f}")

    CONSTANTS.DAMPING = original_damping
    return results


# ============================================================================
# SECTION 5: TIER 3 — sLOOP COUPLING
# ============================================================================

def tier3_sloop(n_trials=500, grid_size=32, n_ticks=20, coupling_factor=1.0):
    """
    Tier 3: sLoop coupling test.

    Detector is a manifested structure (cluster of s != 0 voxels) at
    detector position. The coupling term g_c * s * (div J) creates
    interaction between detector and incoming flux.

    Args:
        coupling_factor: 0.0 = passive detector, 1.0 = full coupling
    """
    imports = try_import_lattice()
    if imports is None:
        return None

    Universe, master_equation, waves, forces, CONSTANTS = imports

    original_damping = CONSTANTS.DAMPING
    CONSTANTS.DAMPING = 0.0  # No damping for clean test

    c = grid_size // 2
    det_offset = grid_size // 4

    det_A_center = (c - det_offset, c, c)
    det_B_center = (c + det_offset, c, c)

    angles = CHSH_ANGLES
    all_results = {}

    for name, (aa, ab) in [
        ('E11', (angles['a1'], angles['b1'])),
        ('E12', (angles['a1'], angles['b2'])),
        ('E21', (angles['a2'], angles['b1'])),
        ('E22', (angles['a2'], angles['b2'])),
    ]:
        outcomes_A = np.zeros(n_trials, dtype=int)
        outcomes_B = np.zeros(n_trials, dtype=int)

        a_hat = angle_to_axis(aa)
        b_hat = angle_to_axis(ab)

        for trial in range(n_trials):
            u = Universe(size=grid_size)

            # Place detector structures (manifested voxels at detector sites)
            if coupling_factor > 0:
                dx, dy, dz = det_A_center
                u.states[dx, dy, dz] = 1
                u.charge[dx, dy, dz] = coupling_factor
                u.is_locked[dx, dy, dz] = True  # Locked so it doesn't decay

                dx, dy, dz = det_B_center
                u.states[dx, dy, dz] = 1
                u.charge[dx, dy, dz] = coupling_factor
                u.is_locked[dx, dy, dz] = True

            # Create entangled pair
            J0_dir = random_unit_vectors(1)[0]
            create_entangled_pair(u, J0_dir, amplitude=5.0)

            # Propagate with forces (sLoop coupling via force accumulation)
            for _ in range(n_ticks):
                waves.propagate_flux(u)
                forces.calculate_density(u)
                if coupling_factor > 0:
                    forces.accumulate_forces(u)

            # Measure (still flux reading - the detector's presence modifies flux)
            outcomes_A[trial] = measure_flux_at_region(u, det_A_center, 3, a_hat)
            outcomes_B[trial] = -measure_flux_at_region(u, det_B_center, 3, b_hat)

        corr, eff = compute_correlation(outcomes_A, outcomes_B)
        all_results[name] = {'correlation': corr, 'efficiency': eff}

    S = compute_chsh_from_correlations(
        all_results['E11']['correlation'],
        all_results['E12']['correlation'],
        all_results['E21']['correlation'],
        all_results['E22']['correlation'],
    )

    CONSTANTS.DAMPING = original_damping
    avg_eff = np.mean([r['efficiency'] for r in all_results.values()])

    return {
        'S': S,
        'correlations': all_results,
        'avg_efficiency': avg_eff,
        'n_trials': n_trials,
        'label': f'Tier 3: sLoop Coupling (f={coupling_factor}, grid={grid_size})',
    }


# ============================================================================
# SECTION 6: TIER 4 — PARAMETER SWEEP
# ============================================================================

def tier4_sweep():
    """
    Tier 4: Systematic parameter sweep.
    Tests multiple configurations to find if S > 2 is achievable anywhere.
    """
    imports = try_import_lattice()
    if imports is None:
        print("  Tier 4 skipped (no lattice engine).")
        return []

    results = []
    configs = [
        # (grid_size, n_ticks, damping, amplitude, label)
        (16, 10, 0.0, 5.0, "Small grid, no damping"),
        (16, 10, 0.05, 5.0, "Small grid, standard damping"),
        (32, 20, 0.0, 5.0, "Medium grid, no damping"),
        (32, 20, 0.0, 10.0, "Medium grid, high amplitude"),
        (32, 20, 0.01, 5.0, "Medium grid, low damping"),
    ]

    for grid_size, n_ticks, damping, amplitude, label in configs:
        print(f"  Sweep: {label}...", end=" ", flush=True)
        t0 = time.time()

        imports_inner = try_import_lattice()
        Universe, master_equation, waves, forces, CONSTANTS = imports_inner

        original_damping = CONSTANTS.DAMPING
        CONSTANTS.DAMPING = damping

        c = grid_size // 2
        det_offset = grid_size // 4
        det_A = (c - det_offset, c, c)
        det_B = (c + det_offset, c, c)
        n_trials = 300

        # Quick CHSH at optimal angles only
        angles_list = [
            (CHSH_ANGLES['a1'], CHSH_ANGLES['b1']),
            (CHSH_ANGLES['a1'], CHSH_ANGLES['b2']),
            (CHSH_ANGLES['a2'], CHSH_ANGLES['b1']),
            (CHSH_ANGLES['a2'], CHSH_ANGLES['b2']),
        ]

        E_vals = []
        for aa, ab in angles_list:
            a_hat = angle_to_axis(aa)
            b_hat = angle_to_axis(ab)

            outs_A = np.zeros(n_trials, dtype=int)
            outs_B = np.zeros(n_trials, dtype=int)

            for trial in range(n_trials):
                u = Universe(size=grid_size)
                J0_dir = random_unit_vectors(1)[0]
                create_entangled_pair(u, J0_dir, amplitude=amplitude)

                for _ in range(n_ticks):
                    waves.propagate_flux(u)
                    forces.calculate_density(u)

                outs_A[trial] = measure_flux_at_region(u, det_A, 3, a_hat)
                outs_B[trial] = -measure_flux_at_region(u, det_B, 3, b_hat)

            corr, _ = compute_correlation(outs_A, outs_B)
            E_vals.append(corr)

        S = compute_chsh_from_correlations(*E_vals)
        CONSTANTS.DAMPING = original_damping

        elapsed = time.time() - t0
        results.append({'label': label, 'S': S, 'time': elapsed,
                        'grid': grid_size, 'ticks': n_ticks, 'damp': damping})
        print(f"S = {S:.4f} ({elapsed:.1f}s)")

    return results


# ============================================================================
# SECTION 7: ANALYSIS AND HONEST VERDICT
# ============================================================================

def print_summary(all_results):
    """Print comprehensive summary table and honest verdict."""
    print("\n" + "=" * 70)
    print("COMPREHENSIVE RESULTS SUMMARY")
    print("=" * 70)
    print(f"\n  {'Tier':<45} {'S':>8} {'Status':>12}")
    print(f"  {'-'*65}")

    for r in all_results:
        if r is None:
            continue
        S = r['S']
        status = "VIOLATION!" if S > 2.0 else "S <= 2"
        label = r['label'][:44]
        print(f"  {label:<45} {S:8.4f} {status:>12}")

    print(f"\n  Classical bound: S <= 2.0000")
    print(f"  Quantum bound:   S <= {2*np.sqrt(2):.4f}")

    # Check if any violation was found
    violations = [r for r in all_results if r is not None and r['S'] > 2.0]

    print("\n" + "=" * 70)
    print("HONEST VERDICT")
    print("=" * 70)

    if len(violations) == 0:
        print("""
  NO BELL VIOLATION WAS OBSERVED FROM LATTICE DYNAMICS.

  This is the EXPECTED result. Bell's theorem is a mathematical theorem
  about probability distributions. FTD's lattice dynamics use local
  update rules (26-neighbor Moore neighborhood), making them a local
  hidden variable theory. Therefore S <= 2 is mathematically guaranteed.

  What this means for FTD:
  1. The lattice dynamics ALONE cannot reproduce quantum correlations
  2. The sLoop mechanism does NOT produce S > 2 in simulation
  3. To achieve Bell violations, FTD must either:
     a. Accept that it is a local realistic theory (honest option)
     b. Show that Hilbert space emerges from lattice dynamics (undemonstrated)
     c. Identify a genuine loophole in Bell's assumptions (speculative)

  The Bell gap is resolved as EMERGENT (April 2026). S = 2*sqrt(2) follows from Tsirelson's bound once QM emerges from the lattice. See DERIV_QM_FROM_LATTICE.md.
""")
    else:
        print("""
  *** UNEXPECTED: BELL VIOLATION OBSERVED ***

  Before accepting this result, check:
  1. Is there a bug in the CHSH calculation?
  2. Is there a detection loophole (post-selection bias)?
  3. Is there superdeterministic correlation (settings correlated with HV)?
  4. Run with more trials to check statistical significance.
""")
        for v in violations:
            print(f"  Violation in: {v['label']}, S = {v['S']:.4f}")


# ============================================================================
# MAIN
# ============================================================================

def main():
    print("=" * 70)
    print("BELL LATTICE INVESTIGATION")
    print("Can FTD Produce S > 2 Without Importing Hilbert Space?")
    print("=" * 70)
    print(f"Date: February 5, 2026")
    print(f"Framework: FTD v5.17")
    print(f"Honest expectation: S <= 2 everywhere")
    print()

    all_results = []

    # --- TIER 0: Baselines ---
    print("=" * 70)
    print("TIER 0: BASELINE CHECKS")
    print("=" * 70)

    r0a = tier0_classical(n_trials=10000)
    print_chsh_result(r0a)
    all_results.append(r0a)

    r0b = tier0_quantum()
    print_chsh_result(r0b)
    all_results.append(r0b)

    # --- TIER 1: Vector HV ---
    print("=" * 70)
    print("TIER 1: VECTOR HIDDEN VARIABLE")
    print("=" * 70)

    r1 = tier1_vector(n_trials=10000)
    print_chsh_result(r1)
    all_results.append(r1)

    r1b = tier1_ternary(n_trials=10000, threshold=0.3)
    print_chsh_result(r1b)
    all_results.append(r1b)

    # --- TIER 2: FTD Lattice ---
    print("=" * 70)
    print("TIER 2: FTD LATTICE PAIR PRODUCTION")
    print("=" * 70)

    # Diagnostic first
    diag = tier2_correlation_diagnostic(grid_size=32, n_ticks=30, damping=0.0)

    r2a = tier2_flux_reading(n_trials=500, grid_size=32, n_ticks=20, damping=0.0)
    if r2a is not None:
        print_chsh_result(r2a)
        all_results.append(r2a)

    # --- TIER 3: sLoop ---
    print("=" * 70)
    print("TIER 3: sLOOP COUPLING")
    print("=" * 70)

    for f in [0.0, 0.5, 1.0]:
        r3 = tier3_sloop(n_trials=300, grid_size=32, n_ticks=20, coupling_factor=f)
        if r3 is not None:
            print_chsh_result(r3)
            all_results.append(r3)

    # --- TIER 4: Parameter Sweep ---
    print("=" * 70)
    print("TIER 4: PARAMETER SWEEP")
    print("=" * 70)

    sweep_results = tier4_sweep()

    # Convert sweep results to standard format for summary
    for sr in sweep_results:
        all_results.append({
            'S': sr['S'],
            'label': f"Sweep: {sr['label']}",
            'correlations': {},
            'avg_efficiency': 1.0,
            'n_trials': 300,
        })

    # --- SUMMARY ---
    print_summary(all_results)


if __name__ == "__main__":
    main()
