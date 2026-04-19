#!/usr/bin/env python3
"""
sLoop Bell Experiment: Can Embedded Detectors Produce S > 2?
============================================================

EPISTEMIC STATUS: [INVESTIGATION]

This script tests the central open question in FTD: whether the sLoop
mechanism -- physical detectors coupled to the flux substrate via
STATE_FLUX_COUPLING -- can produce CHSH Bell inequality violations.

Key innovation over existing Bell tests:
  1. Enables STATE_FLUX_COUPLING (g_c * grad(s) in wave equation)
  2. Builds oriented detector structures (not single voxels)
  3. Equilibrates flux field with detectors BEFORE pair creation
  4. Tests dynamical measurement (outcome from physics, not external projection)

This creates the sLoop: detector modifies flux, flux determines outcomes,
detector is part of the system it measures.

Seven configurations (A-G) with increasing sLoop fidelity, plus
parameter sweeps over coupling strength and equilibration time.

Author: Claude Code
Date: February 17, 2026
"""

import numpy as np
import sys
import os
import time

project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
if project_root not in sys.path:
    sys.path.insert(0, project_root)

# PY-2 refactor (April 2026): CHSH helpers consolidated into
# scripts/common/bell_chsh.py. Behavior preserved; angle_to_axis still
# returns float32 at the call sites that need it.
_PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
if _PROJECT_ROOT not in sys.path:
    sys.path.insert(0, _PROJECT_ROOT)
from scripts.common.bell_chsh import (
    CHSH_ANGLES,
    compute_chsh,
    compute_correlation,
    random_unit_vectors,
)
from scripts.common.bell_chsh import angle_to_axis as _angle_to_axis


def angle_to_axis(theta):
    """Convert measurement angle to unit vector in x-z plane (float32)."""
    return _angle_to_axis(theta, dtype=np.float32)


import ternary_matrix.config as cfg
from ternary_matrix.model.grid import Universe
from ternary_matrix.physics import waves, forces


# ============================================================================
# SECTION 1: CHSH INFRASTRUCTURE
# ============================================================================
# (helpers moved to scripts/common/bell_chsh; re-exported above for back-compat)


# ============================================================================
# SECTION 2: DETECTOR CONSTRUCTION
# ============================================================================

def build_detector(universe, center, angle, n_voxels=5):
    """
    Build an oriented detector structure from manifested voxels.

    Places a line of n_voxels locked +1 voxels centered at `center`,
    oriented at `angle` in the x-z plane. The detector's sensitivity
    axis is perpendicular to the line direction.

    With STATE_FLUX_COUPLING enabled, these manifested voxels source
    flux gradients via g_c * grad(s), coupling the detector to the
    flux field bidirectionally.

    Args:
        universe: Universe instance
        center: (x, y, z) tuple for detector center
        angle: measurement angle in radians (orientation in x-z plane)
        n_voxels: number of voxels in the detector line

    Returns:
        list of (x, y, z) positions of detector voxels
    """
    cx, cy, cz = center
    # Direction along which voxels are placed
    dx = np.cos(angle)
    dz = np.sin(angle)

    positions = []
    half = n_voxels // 2

    for i in range(-half, half + 1):
        if n_voxels % 2 == 0 and i == 0:
            continue
        x = int(round(cx + i * dx))
        z = int(round(cz + i * dz))
        y = cy

        # Bounds check
        if 0 <= x < universe.size and 0 <= y < universe.size and 0 <= z < universe.size:
            universe.states[x, y, z] = 1
            universe.charge[x, y, z] = 1.0
            universe.is_locked[x, y, z] = True
            # Give detector voxels a small flux along their sensitivity axis
            # (perpendicular to the line direction)
            sens_x = -np.sin(angle)
            sens_z = np.cos(angle)
            universe.flux[x, y, z] = np.array(
                [sens_x * 0.5, 0.0, sens_z * 0.5], dtype=np.float32
            )
            positions.append((x, y, z))

    return positions


def equilibrate(universe, n_ticks, with_forces=False):
    """
    Run wave propagation to let flux field equilibrate with detectors.

    With STATE_FLUX_COUPLING > 0, the manifested detector voxels
    source flux gradients, creating a flux background that is
    correlated with the detector configuration.

    Args:
        universe: Universe instance (with detectors already placed)
        n_ticks: number of propagation ticks
        with_forces: if True, also compute forces (slower but more realistic)
    """
    for _ in range(n_ticks):
        waves.propagate_flux(universe)
        forces.calculate_density(universe)
        if with_forces:
            forces.accumulate_forces(universe)


# ============================================================================
# SECTION 3: PAIR CREATION AND MEASUREMENT
# ============================================================================

def create_sloop_pair(universe, amplitude=5.0):
    """
    Create an entangled pair at the universe center.

    The pair is created IN the flux background that was shaped by
    the equilibrated detectors. This is the key sLoop feature:
    the hidden variable (flux state) is correlated with detector
    configurations through the shared substrate.

    Args:
        universe: Universe instance (possibly with equilibrated flux)
        amplitude: flux magnitude for the pair

    Returns:
        (pos_A, pos_B) positions of the created particles
    """
    c = universe.size // 2

    # Random initial flux direction
    J0_dir = random_unit_vectors(1)[0]
    J0 = J0_dir.astype(np.float32) * amplitude

    # Particle A at center
    pos_A = (c, c, c)
    universe.states[pos_A] = 1
    universe.flux[pos_A] = J0
    universe.charge[pos_A] = 1.0

    # Particle B at center+1 (anti-correlated flux)
    pos_B = (c + 1, c, c)
    universe.states[pos_B] = -1
    universe.flux[pos_B] = -J0
    universe.charge[pos_B] = -1.0

    # Separate the pair
    universe.velocity[pos_A] = np.array([-0.4, 0, 0], dtype=np.float32)
    universe.velocity[pos_B] = np.array([0.4, 0, 0], dtype=np.float32)

    return pos_A, pos_B


def measure_external(universe, center, radius, axis):
    """
    External measurement: project flux onto measurement axis.
    This is the standard (non-sLoop) measurement used in existing tests.

    Returns +1, 0, or -1.
    """
    cx, cy, cz = center
    s = universe.size
    region = universe.flux[
        max(0, cx - radius):min(s, cx + radius + 1),
        max(0, cy - radius):min(s, cy + radius + 1),
        max(0, cz - radius):min(s, cz + radius + 1),
    ]
    total_flux = np.sum(region, axis=(0, 1, 2))
    projection = np.dot(total_flux, axis)
    if abs(projection) < 1e-10:
        return 0
    return int(np.sign(projection))


def measure_dynamical(universe, det_center, det_angle, radius=3):
    """
    Dynamical sLoop measurement: outcome determined by physics.

    Instead of externally projecting flux, we measure the NET FLUX
    CHANGE at the detector region during propagation. The detector's
    coupling to the flux field (via STATE_FLUX_COUPLING) creates a
    response that depends on both the incoming flux and the detector's
    orientation.

    The outcome is the sign of the flux change projected onto the
    detector's sensitivity axis.

    Args:
        universe: Universe after propagation
        det_center: detector center position
        det_angle: detector orientation angle
        radius: measurement region radius

    Returns:
        +1, 0, or -1
    """
    cx, cy, cz = det_center
    s = universe.size

    # Sensitivity axis (perpendicular to detector line)
    sens = np.array([-np.sin(det_angle), 0.0, np.cos(det_angle)],
                    dtype=np.float32)

    # Measure flux in detector region
    region = universe.flux[
        max(0, cx - radius):min(s, cx + radius + 1),
        max(0, cy - radius):min(s, cy + radius + 1),
        max(0, cz - radius):min(s, cz + radius + 1),
    ]
    total_flux = np.sum(region, axis=(0, 1, 2))

    # Project onto sensitivity axis
    projection = np.dot(total_flux, sens)
    if abs(projection) < 1e-10:
        return 0
    return int(np.sign(projection))


# ============================================================================
# SECTION 4: sLOOP CHSH TEST
# ============================================================================

def run_sloop_chsh(config, n_trials=500, seed=None):
    """
    Run a complete CHSH test with sLoop configuration.

    Args:
        config: dict with keys:
            'g_c': STATE_FLUX_COUPLING value
            'n_det_voxels': number of voxels per detector (0 = no detector)
            'n_equil': equilibration ticks before pair creation
            'n_prop': propagation ticks after pair creation
            'measurement': 'external' or 'dynamical'
            'grid_size': lattice size
            'label': human-readable label
        n_trials: number of trials per angle pair
        seed: random seed (None = random)

    Returns:
        dict with S, correlations, efficiency, diagnostics
    """
    if seed is not None:
        np.random.seed(seed)

    g_c = config.get('g_c', 0.0)
    n_det = config.get('n_det_voxels', 0)
    n_equil = config.get('n_equil', 0)
    n_prop = config.get('n_prop', 20)
    meas_type = config.get('measurement', 'external')
    grid_size = config.get('grid_size', 32)
    label = config.get('label', 'unnamed')

    angles = CHSH_ANGLES
    c = grid_size // 2
    det_offset = grid_size // 4

    det_A_center = (c - det_offset, c, c)
    det_B_center = (c + det_offset, c, c)

    all_results = {}
    t_start = time.time()

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
            # Configure physics
            cfg.CONSTANTS = cfg.PhysicsConfig(
                GRID_SIZE=grid_size,
                DAMPING=0.0,
                DECAY_RATE=0.0,
                C_WAVE=0.4,
                STATE_FLUX_COUPLING=g_c,
            )
            u = Universe(grid_size)

            # Phase 1: Build detectors and equilibrate
            if n_det > 0:
                build_detector(u, det_A_center, aa, n_voxels=n_det)
                build_detector(u, det_B_center, ab, n_voxels=n_det)

            if n_equil > 0:
                equilibrate(u, n_equil)

            # Phase 2: Create entangled pair
            create_sloop_pair(u, amplitude=5.0)

            # Phase 3: Propagate
            for _ in range(n_prop):
                waves.propagate_flux(u)
                forces.calculate_density(u)

            # Phase 4: Measure
            if meas_type == 'external':
                outcomes_A[trial] = measure_external(u, det_A_center, 3, a_hat)
                outcomes_B[trial] = -measure_external(u, det_B_center, 3, b_hat)
            elif meas_type == 'dynamical':
                outcomes_A[trial] = measure_dynamical(u, det_A_center, aa, 3)
                outcomes_B[trial] = -measure_dynamical(u, det_B_center, ab, 3)

        corr, eff = compute_correlation(outcomes_A, outcomes_B)
        all_results[name] = {'correlation': corr, 'efficiency': eff}

    S = compute_chsh(
        all_results['E11']['correlation'],
        all_results['E12']['correlation'],
        all_results['E21']['correlation'],
        all_results['E22']['correlation'],
    )

    avg_eff = np.mean([r['efficiency'] for r in all_results.values()])
    elapsed = time.time() - t_start

    return {
        'S': S,
        'correlations': all_results,
        'avg_efficiency': avg_eff,
        'n_trials': n_trials,
        'elapsed': elapsed,
        'config': config,
        'label': label,
    }


def print_result(result):
    """Pretty-print a CHSH test result."""
    print(f"\n  {result['label']}")
    print(f"  {'=' * 55}")
    for name in ['E11', 'E12', 'E21', 'E22']:
        r = result['correlations'][name]
        print(f"    {name} = {r['correlation']:+.4f}  "
              f"(eff: {r['efficiency']:.0%})")
    print(f"  {'=' * 55}")
    S = result['S']
    print(f"    S = {S:.4f}  (classical: 2.000, quantum: {2*np.sqrt(2):.4f})")
    if S > 2.0:
        print(f"    *** BELL VIOLATION: S > 2 by {S - 2:.4f} ***")
    else:
        print(f"    No violation (S <= 2)")
    print(f"    Efficiency: {result['avg_efficiency']:.0%}, "
          f"Time: {result['elapsed']:.1f}s")


# ============================================================================
# SECTION 5: CONFIGURATIONS A-F
# ============================================================================

def run_configurations(n_trials=500):
    """Run all planned configurations."""
    configs = [
        {
            'label': 'Config A: Baseline (no coupling, no detectors)',
            'g_c': 0.0,
            'n_det_voxels': 0,
            'n_equil': 0,
            'n_prop': 20,
            'measurement': 'external',
            'grid_size': 32,
        },
        {
            'label': 'Config B: Passive detectors (no coupling)',
            'g_c': 0.0,
            'n_det_voxels': 5,
            'n_equil': 0,
            'n_prop': 20,
            'measurement': 'external',
            'grid_size': 32,
        },
        {
            'label': 'Config C: Coupling ON, no equilibration',
            'g_c': 0.085,
            'n_det_voxels': 5,
            'n_equil': 0,
            'n_prop': 20,
            'measurement': 'external',
            'grid_size': 32,
        },
        {
            'label': 'Config D: Equilibrated sLoop (KEY TEST)',
            'g_c': 0.085,
            'n_det_voxels': 5,
            'n_equil': 50,
            'n_prop': 20,
            'measurement': 'external',
            'grid_size': 32,
        },
        {
            'label': 'Config E: Dynamical measurement + sLoop',
            'g_c': 0.085,
            'n_det_voxels': 5,
            'n_equil': 50,
            'n_prop': 20,
            'measurement': 'dynamical',
            'grid_size': 32,
        },
        {
            'label': 'Config F: Longer equilibration + dynamical',
            'g_c': 0.085,
            'n_det_voxels': 5,
            'n_equil': 100,
            'n_prop': 20,
            'measurement': 'dynamical',
            'grid_size': 32,
        },
    ]

    results = []
    for config in configs:
        print(f"\n  Running: {config['label']}...")
        r = run_sloop_chsh(config, n_trials=n_trials, seed=42)
        print_result(r)
        results.append(r)

    return results


# ============================================================================
# SECTION 6: PARAMETER SWEEPS
# ============================================================================

def sweep_coupling(n_trials=300):
    """Sweep STATE_FLUX_COUPLING and plot S vs g_c."""
    print("\n" + "=" * 60)
    print("PARAMETER SWEEP: S vs STATE_FLUX_COUPLING (g_c)")
    print("=" * 60)

    g_c_values = [0.0, 0.01, 0.05, 0.085, 0.1, 0.2, 0.5, 1.0]
    results = []

    for g_c in g_c_values:
        config = {
            'label': f'g_c = {g_c}',
            'g_c': g_c,
            'n_det_voxels': 5,
            'n_equil': 50,
            'n_prop': 20,
            'measurement': 'dynamical',
            'grid_size': 32,
        }
        print(f"  g_c = {g_c:.3f}...", end=" ", flush=True)
        r = run_sloop_chsh(config, n_trials=n_trials, seed=42)
        print(f"S = {r['S']:.4f}  (eff: {r['avg_efficiency']:.0%}, "
              f"{r['elapsed']:.1f}s)")
        results.append((g_c, r['S'], r['avg_efficiency']))

    print(f"\n  {'g_c':>8}  {'S':>8}  {'Eff':>6}")
    print(f"  {'-' * 26}")
    for g_c, S, eff in results:
        marker = " ***" if S > 2.0 else ""
        print(f"  {g_c:8.3f}  {S:8.4f}  {eff:5.0%}{marker}")

    return results


def sweep_equilibration(n_trials=300):
    """Sweep equilibration ticks and plot S vs N_equil."""
    print("\n" + "=" * 60)
    print("PARAMETER SWEEP: S vs Equilibration Ticks")
    print("=" * 60)

    equil_values = [0, 10, 25, 50, 100, 200]
    results = []

    for n_equil in equil_values:
        config = {
            'label': f'N_equil = {n_equil}',
            'g_c': 0.085,
            'n_det_voxels': 5,
            'n_equil': n_equil,
            'n_prop': 20,
            'measurement': 'dynamical',
            'grid_size': 32,
        }
        print(f"  N_equil = {n_equil:3d}...", end=" ", flush=True)
        r = run_sloop_chsh(config, n_trials=n_trials, seed=42)
        print(f"S = {r['S']:.4f}  (eff: {r['avg_efficiency']:.0%}, "
              f"{r['elapsed']:.1f}s)")
        results.append((n_equil, r['S'], r['avg_efficiency']))

    print(f"\n  {'N_equil':>8}  {'S':>8}  {'Eff':>6}")
    print(f"  {'-' * 26}")
    for n_eq, S, eff in results:
        marker = " ***" if S > 2.0 else ""
        print(f"  {n_eq:8d}  {S:8.4f}  {eff:5.0%}{marker}")

    return results


# ============================================================================
# SECTION 7: DIAGNOSTICS
# ============================================================================

def diagnose_correlation_shape(n_angles=18, n_trials=300):
    """
    Sweep E(theta) for key configurations.
    Compare correlation shape to triangle (classical) and cosine (quantum).
    """
    print("\n" + "=" * 60)
    print("DIAGNOSTIC: Correlation Shape E(theta)")
    print("=" * 60)

    theta_values = np.linspace(0, np.pi, n_angles)

    configs = [
        ('Baseline (g_c=0)', 0.0, 0, 'external'),
        ('sLoop (g_c=0.085, eq=50)', 0.085, 50, 'dynamical'),
    ]

    shapes = {}

    for name, g_c, n_equil, meas_type in configs:
        corrs = []
        print(f"  {name}: ", end="", flush=True)

        for theta in theta_values:
            # Run single-angle correlation: Alice at 0, Bob at theta
            cfg.CONSTANTS = cfg.PhysicsConfig(
                GRID_SIZE=32, DAMPING=0.0, DECAY_RATE=0.0,
                C_WAVE=0.4, STATE_FLUX_COUPLING=g_c,
            )

            c = 16
            det_offset = 8
            det_A = (c - det_offset, c, c)
            det_B = (c + det_offset, c, c)

            oA = np.zeros(n_trials, dtype=int)
            oB = np.zeros(n_trials, dtype=int)

            a_hat = angle_to_axis(0.0)
            b_hat = angle_to_axis(theta)

            for trial in range(n_trials):
                u = Universe(32)

                if g_c > 0:
                    build_detector(u, det_A, 0.0, n_voxels=5)
                    build_detector(u, det_B, theta, n_voxels=5)
                    equilibrate(u, n_equil)

                create_sloop_pair(u, amplitude=5.0)

                for _ in range(20):
                    waves.propagate_flux(u)
                    forces.calculate_density(u)

                if meas_type == 'external':
                    oA[trial] = measure_external(u, det_A, 3, a_hat)
                    oB[trial] = -measure_external(u, det_B, 3, b_hat)
                else:
                    oA[trial] = measure_dynamical(u, det_A, 0.0, 3)
                    oB[trial] = -measure_dynamical(u, det_B, theta, 3)

            c_val, _ = compute_correlation(oA, oB)
            corrs.append(c_val)
            print(".", end="", flush=True)

        shapes[name] = np.array(corrs)
        print(" done")

    # Print comparison table
    print(f"\n  {'theta':>6} ", end="")
    for name in shapes:
        short = name[:12]
        print(f" {short:>12}", end="")
    print(f" {'QM -cos':>12} {'Triangle':>12}")
    print(f"  {'-' * (6 + 14 * (len(shapes) + 2))}")

    for i, theta in enumerate(theta_values):
        qm = -np.cos(theta)
        tri = -(1 - 2 * abs(theta) / np.pi)
        print(f"  {np.degrees(theta):6.1f} ", end="")
        for name in shapes:
            print(f" {shapes[name][i]:+12.4f}", end="")
        print(f" {qm:+12.4f} {tri:+12.4f}")

    return theta_values, shapes


def diagnose_backaction(n_trials=100):
    """
    Measure how much the detectors modify the flux field at the
    pair creation site. This quantifies the measurement-independence
    violation: if detectors change the background flux, then the
    'hidden variable' (flux at creation) depends on detector settings.
    """
    print("\n" + "=" * 60)
    print("DIAGNOSTIC: Detector Back-Action on Source Region")
    print("=" * 60)

    grid_size = 32
    c = grid_size // 2
    det_offset = grid_size // 4

    # Compare flux at center with different detector angles
    angle_pairs = [
        (0.0, np.pi / 4),
        (0.0, 3 * np.pi / 4),
        (np.pi / 2, np.pi / 4),
        (np.pi / 2, 3 * np.pi / 4),
    ]

    print(f"  Measuring flux at center after equilibration with different detector angles")
    print(f"  g_c = 0.085, N_equil = 50, grid = {grid_size}")

    flux_at_center = []
    for aa, ab in angle_pairs:
        cfg.CONSTANTS = cfg.PhysicsConfig(
            GRID_SIZE=grid_size, DAMPING=0.0, DECAY_RATE=0.0,
            C_WAVE=0.4, STATE_FLUX_COUPLING=0.085,
        )
        u = Universe(grid_size)
        build_detector(u, (c - det_offset, c, c), aa, n_voxels=5)
        build_detector(u, (c + det_offset, c, c), ab, n_voxels=5)
        equilibrate(u, 50)

        # Measure flux at pair creation site
        center_flux = u.flux[c, c, c].copy()
        center_density = np.linalg.norm(center_flux)

        flux_at_center.append({
            'angles': (aa, ab),
            'flux': center_flux,
            'density': center_density,
        })

        print(f"    a={np.degrees(aa):5.1f}, b={np.degrees(ab):5.1f}: "
              f"J = [{center_flux[0]:+.6f}, {center_flux[1]:+.6f}, "
              f"{center_flux[2]:+.6f}], |J| = {center_density:.6f}")

    # Check if flux differs between angle configurations
    densities = [f['density'] for f in flux_at_center]
    max_diff = max(densities) - min(densities)
    print(f"\n  Max density difference between configs: {max_diff:.6f}")
    if max_diff > 1e-6:
        print("  --> DETECTORS MODIFY FLUX AT SOURCE (measurement-independence violated)")
    else:
        print("  --> No measurable back-action (measurement independence preserved)")

    # Also check without coupling
    print(f"\n  Control: g_c = 0 (no coupling)")
    for aa, ab in angle_pairs[:1]:
        cfg.CONSTANTS = cfg.PhysicsConfig(
            GRID_SIZE=grid_size, DAMPING=0.0, DECAY_RATE=0.0,
            C_WAVE=0.4, STATE_FLUX_COUPLING=0.0,
        )
        u = Universe(grid_size)
        build_detector(u, (c - det_offset, c, c), aa, n_voxels=5)
        build_detector(u, (c + det_offset, c, c), ab, n_voxels=5)
        equilibrate(u, 50)

        center_flux = u.flux[c, c, c].copy()
        center_density = np.linalg.norm(center_flux)
        print(f"    a={np.degrees(aa):5.1f}, b={np.degrees(ab):5.1f}: "
              f"|J| = {center_density:.6f}")

    return flux_at_center


# ============================================================================
# SECTION 8: SYNTHESIS AND VERDICT
# ============================================================================

def print_summary(all_results, sweep_gc, sweep_eq):
    """Print comprehensive summary and verdict."""
    print("\n" + "=" * 70)
    print("COMPREHENSIVE RESULTS SUMMARY")
    print("=" * 70)

    print(f"\n  {'Configuration':<50} {'S':>8} {'Eff':>6}")
    print(f"  {'-' * 66}")

    max_S = 0
    for r in all_results:
        S = r['S']
        eff = r['avg_efficiency']
        label = r['label'][:49]
        marker = " ***" if S > 2.0 else ""
        print(f"  {label:<50} {S:8.4f} {eff:5.0%}{marker}")
        max_S = max(max_S, S)

    print(f"\n  Classical bound: S <= 2.0000")
    print(f"  Quantum bound:   S <= {2 * np.sqrt(2):.4f}")
    print(f"  Maximum observed: S = {max_S:.4f}")

    # Check sweep results
    if sweep_gc:
        max_sweep_S = max(s for _, s, _ in sweep_gc)
        max_S = max(max_S, max_sweep_S)

    violations = [r for r in all_results if r['S'] > 2.0]

    print("\n" + "=" * 70)
    print("VERDICT")
    print("=" * 70)

    if violations:
        print(f"""
  *** BELL VIOLATION OBSERVED ***

  {len(violations)} configuration(s) produced S > 2:""")
        for v in violations:
            print(f"    {v['label']}: S = {v['S']:.4f}")
        print(f"""
  Before accepting:
  1. Check for detection loophole (post-selection bias)
  2. Check for bugs in CHSH calculation
  3. Run with more trials for statistical significance
  4. Verify with different random seeds
""")
    else:
        print(f"""
  NO BELL VIOLATION OBSERVED (S <= 2 for all configurations).

  This is consistent with Bell's theorem: FTD's lattice dynamics are
  local deterministic, guaranteeing S <= 2.

  What the sLoop coupling DID (or didn't) do:""")

        # Compare baseline S with best sLoop S
        baseline_S = all_results[0]['S'] if all_results else 0
        sloop_S_values = [r['S'] for r in all_results[1:]] if len(all_results) > 1 else [0]
        best_sloop_S = max(sloop_S_values) if sloop_S_values else 0

        if best_sloop_S > baseline_S + 0.05:
            print(f"    - Baseline S = {baseline_S:.4f}")
            print(f"    - Best sLoop S = {best_sloop_S:.4f}")
            print(f"    - Improvement: +{best_sloop_S - baseline_S:.4f}")
            print(f"    - The sLoop coupling MODIFIES correlations but")
            print(f"      not enough to exceed the classical bound.")
        else:
            print(f"    - Baseline S = {baseline_S:.4f}")
            print(f"    - Best sLoop S = {best_sloop_S:.4f}")
            print(f"    - No significant change in S with sLoop coupling.")

        print(f"""
  Interpretation:
    The STATE_FLUX_COUPLING creates bidirectional interaction between
    detectors and flux field, establishing the sLoop. However, this
    coupling operates WITHIN the local deterministic framework --
    Bell's theorem still applies.

    The sLoop mechanism as currently implemented in the simulation
    engine does not bridge the Bell gap. The transition from S=2
    (substrate) to S=2*sqrt(2) (quantum) may require:
    1. The Hilbert space construction (complexified flux)
    2. The Born rule (|psi|^2 probability, not sign function)
    3. Tensor product structure (H_A x H_B, not classical product)
    These are aggregate/statistical features, not substrate features.
""")


# ============================================================================
# MAIN
# ============================================================================

def main():
    np.random.seed(42)

    print("=" * 70)
    print("sLOOP BELL EXPERIMENT")
    print("Can Embedded Detectors with STATE_FLUX_COUPLING Produce S > 2?")
    print("=" * 70)
    print(f"Date: February 17, 2026")
    print(f"Framework: FTD v5.27")
    print()
    print("Innovation: First test to enable STATE_FLUX_COUPLING (g_c * grad(s))")
    print("in the wave equation, creating genuine bidirectional detector-flux")
    print("coupling (the sLoop mechanism).")
    print()

    # --- Section 1: Core configurations ---
    print("=" * 70)
    print("SECTION 1: CORE CONFIGURATIONS (A-F)")
    print("=" * 70)

    all_results = run_configurations(n_trials=500)

    # --- Section 2: Coupling strength sweep ---
    sweep_gc = sweep_coupling(n_trials=300)

    # --- Section 3: Equilibration sweep ---
    sweep_eq = sweep_equilibration(n_trials=300)

    # --- Section 4: Diagnostics ---
    print("\n" + "=" * 70)
    print("SECTION 4: DIAGNOSTICS")
    print("=" * 70)

    diagnose_backaction()
    diagnose_correlation_shape(n_angles=12, n_trials=200)

    # --- Summary ---
    print_summary(all_results, sweep_gc, sweep_eq)


if __name__ == "__main__":
    main()
