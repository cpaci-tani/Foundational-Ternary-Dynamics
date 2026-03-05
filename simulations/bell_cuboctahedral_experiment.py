#!/usr/bin/env python3
"""
Cuboctahedral Bell Experiments
==============================

EPISTEMIC STATUS: [INVESTIGATION]

Three experiments testing whether the cuboctahedral (FCC) lattice geometry,
combined with active measurement and embedded observation, can produce
CHSH S > 2 from pure lattice dynamics.

Mathematical basis:
  - 12 FCC nearest-neighbor vectors form the D_3 root system
  - D_3 ≅ A_3 ≅ su(4) ≅ so(6) (exceptional isomorphism)
  - su(4) → su(2)_A × su(2)_B × u(1) branching gives entangled spin-1/2 structure
  - CHSH optimal direction (1,0,1)/sqrt(2) IS a cuboctahedral lattice vector
  - Tsirelson bound 2*sqrt(2) = 2 × (FCC neighbor distance sqrt(2))

Experiments:
  1. Cubic vs Cuboctahedral S-parameter comparison (passive flux reading)
  2. Passive vs Active measurement protocol on both geometries
  3. Embedded observer correlation computation on cuboctahedral lattice

Honest expectation: S <= 2 from all passive measurements.
Open question: Does active measurement or embedded observation change anything?

Author: Claude Code
Date: February 16, 2026
"""

import numpy as np
import sys
import os
import time

# Add project root to path
project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
if project_root not in sys.path:
    sys.path.insert(0, project_root)

from ternary_matrix.model.grid import Universe
from ternary_matrix.model.cubic_geometry import CubicGeometry
from ternary_matrix.model.cuboctahedral_geometry import CuboctahedralGeometry
from ternary_matrix.physics import waves, forces
from ternary_matrix.config import CONSTANTS


# ============================================================================
# CHSH INFRASTRUCTURE
# ============================================================================

# Standard CHSH optimal angles (in x-z plane)
CHSH_ANGLES = {
    'a1': 0.0,
    'a2': np.pi / 2,
    'b1': np.pi / 4,
    'b2': 3 * np.pi / 4,
}

# Cuboctahedral measurement directions: CHSH-optimal vectors that ARE FCC lattice vectors
CUBOCT_DIRS = {
    'a1': np.array([0.0, 0.0, 1.0]),                    # z-axis (Cartesian)
    'a2': np.array([1.0, 0.0, 0.0]),                    # x-axis (Cartesian)
    'b1': np.array([1.0, 0.0, 1.0]) / np.sqrt(2),      # cuboctahedral direction
    'b2': np.array([-1.0, 0.0, 1.0]) / np.sqrt(2),     # cuboctahedral direction
}


def angle_to_axis(theta):
    """Convert measurement angle to unit vector in x-z plane."""
    return np.array([np.sin(theta), 0.0, np.cos(theta)])


def compute_chsh(E11, E12, E21, E22):
    """CHSH S-parameter from four correlations."""
    return abs(E11 - E12) + abs(E21 + E22)


def compute_correlation(outcomes_A, outcomes_B):
    """Correlation E(a,b) = <A*B> with null-outcome filtering."""
    valid = (outcomes_A != 0) & (outcomes_B != 0)
    n_valid = np.sum(valid)
    if n_valid == 0:
        return 0.0, 0.0
    efficiency = n_valid / len(outcomes_A)
    correlation = np.mean(outcomes_A[valid] * outcomes_B[valid])
    return float(correlation), float(efficiency)


def random_unit_vectors(n):
    """Generate n random unit vectors uniformly on the sphere."""
    z = np.random.uniform(-1, 1, n)
    phi = np.random.uniform(0, 2 * np.pi, n)
    r = np.sqrt(1 - z**2)
    x = r * np.cos(phi)
    y = r * np.sin(phi)
    return np.column_stack([x, y, z])


def is_valid_fcc(x, y, z):
    """Check if (x,y,z) is a valid FCC lattice site."""
    return (x + y + z) % 2 == 0


# ============================================================================
# PAIR CREATION AND MEASUREMENT
# ============================================================================

def create_entangled_pair(universe, J0_direction, amplitude=5.0):
    """
    Create entangled pair at lattice center, handling both cubic and FCC geometry.

    On FCC: particle B is placed at the nearest valid FCC neighbor (1,1,0)
    rather than (1,0,0) which would be an invalid FCC site.

    Returns (pos_A, pos_B).
    """
    c = universe.size // 2
    J0 = np.array(J0_direction, dtype=np.float32) * amplitude
    is_fcc = universe.geometry.name == 'cuboctahedral'

    # Particle A at center
    pos_A = (c, c, c)
    universe.states[c, c, c] = 1
    universe.flux[c, c, c] = J0
    universe.charge[c, c, c] = 1.0

    if is_fcc:
        # FCC nearest neighbor: offset (1,1,0) at distance sqrt(2)
        bx, by, bz = c + 1, c + 1, c
        assert is_valid_fcc(bx, by, bz), f"FCC neighbor ({bx},{by},{bz}) invalid"
        pos_B = (bx, by, bz)
        universe.states[bx, by, bz] = -1
        universe.flux[bx, by, bz] = -J0
        universe.charge[bx, by, bz] = -1.0
        # Separate along (1,1,0) direction
        sep = np.array([1.0, 1.0, 0.0], dtype=np.float32) / np.sqrt(2)
        universe.velocity[c, c, c] = -0.4 * sep
        universe.velocity[bx, by, bz] = 0.4 * sep
    else:
        # Cubic: adjacent along x-axis
        pos_B = (c + 1, c, c)
        universe.states[c + 1, c, c] = -1
        universe.flux[c + 1, c, c] = -J0
        universe.charge[c + 1, c, c] = -1.0
        universe.velocity[c, c, c] = np.array([-0.4, 0, 0], dtype=np.float32)
        universe.velocity[c + 1, c, c] = np.array([0.4, 0, 0], dtype=np.float32)

    return pos_A, pos_B


def get_detector_positions(universe):
    """Get detector positions at 1/4 grid offset from center."""
    c = universe.size // 2
    det_offset = universe.size // 4
    det_A = (c - det_offset, c, c)
    det_B = (c + det_offset, c, c)

    # Verify FCC validity
    if universe.geometry.name == 'cuboctahedral':
        for label, pos in [('A', det_A), ('B', det_B)]:
            if not is_valid_fcc(*pos):
                # Shift x by 1 to fix parity
                pos = (pos[0] + 1, pos[1], pos[2])
                if label == 'A':
                    det_A = pos
                else:
                    det_B = pos
    return det_A, det_B


def measure_passive(universe, region_center, radius, axis):
    """
    PASSIVE measurement: read sign of total projected flux.
    Does NOT modify the universe state. Pure observation.
    """
    cx, cy, cz = region_center
    s = universe.size
    x_lo, x_hi = max(0, cx - radius), min(s, cx + radius + 1)
    y_lo, y_hi = max(0, cy - radius), min(s, cy + radius + 1)
    z_lo, z_hi = max(0, cz - radius), min(s, cz + radius + 1)

    total_flux = np.sum(
        universe.flux[x_lo:x_hi, y_lo:y_hi, z_lo:z_hi], axis=(0, 1, 2)
    )
    projection = np.dot(total_flux, axis)

    if abs(projection) < 1e-10:
        return 0
    return int(np.sign(projection))


def measure_active(universe, region_center, radius, axis,
                   coupling_strength=2.0, n_interact_ticks=3):
    """
    ACTIVE measurement: place detector particle, let it couple to flux,
    then read the modified flux.

    The detector is a manifested particle (s=+1) with flux aligned along
    the measurement axis. Its coupling to the local flux via the Lagrangian
    term g_c * s * div(J) modifies the flux configuration.

    On cuboctahedral lattice, the 12-neighbor Laplacian mixes flux components
    along non-orthogonal directions, potentially introducing noncommutativity
    between measurements along different axes.
    """
    cx, cy, cz = region_center

    # Place detector particle with flux along measurement axis
    det_flux = np.array(axis, dtype=np.float32) * coupling_strength
    universe.states[cx, cy, cz] = 1
    universe.flux[cx, cy, cz] += det_flux
    universe.charge[cx, cy, cz] = coupling_strength
    universe.is_locked[cx, cy, cz] = True

    # Let detector interact with local flux through lattice dynamics
    for _ in range(n_interact_ticks):
        waves.propagate_flux(universe)
        forces.calculate_density(universe)
        forces.accumulate_forces(universe)

    # Read modified flux
    result = measure_passive(universe, region_center, radius, axis)
    return result


# ============================================================================
# EXPERIMENT 1: CUBIC vs CUBOCTAHEDRAL S-PARAMETER
# ============================================================================

def experiment_1(n_trials=400, grid_size=16, n_ticks=12):
    """
    Compare CHSH S-parameter between cubic and cuboctahedral lattices.
    Both use passive flux reading — same measurement protocol, different geometry.
    Tests whether the 12-neighbor Laplacian changes the correlation structure.
    """
    print("\n" + "=" * 70)
    print("EXPERIMENT 1: CUBIC vs CUBOCTAHEDRAL S-PARAMETER")
    print("=" * 70)
    print(f"  Grid size: {grid_size} (FCC doubles to {2*grid_size} internally)")
    print(f"  Trials per angle pair: {n_trials}")
    print(f"  Propagation ticks: {n_ticks}")
    print(f"  Measurement: PASSIVE flux reading")
    print()

    original_damping = CONSTANTS.DAMPING
    CONSTANTS.DAMPING = 0.0

    results = {}

    # Test three configurations:
    # 1. Cubic geometry + standard CHSH angles
    # 2. Cuboctahedral geometry + standard CHSH angles
    # 3. Cuboctahedral geometry + D_3 root directions
    configs = [
        ("cubic_standard", CubicGeometry, "standard"),
        ("cuboct_standard", CuboctahedralGeometry, "standard"),
        ("cuboct_d3_dirs", CuboctahedralGeometry, "d3"),
    ]

    for config_name, geom_class, angle_type in configs:
        label = config_name.replace("_", " / ").upper()
        print(f"  --- {label} ---")

        all_corr = {}

        if angle_type == "standard":
            angle_pairs = [
                ('E11', angle_to_axis(CHSH_ANGLES['a1']), angle_to_axis(CHSH_ANGLES['b1'])),
                ('E12', angle_to_axis(CHSH_ANGLES['a1']), angle_to_axis(CHSH_ANGLES['b2'])),
                ('E21', angle_to_axis(CHSH_ANGLES['a2']), angle_to_axis(CHSH_ANGLES['b1'])),
                ('E22', angle_to_axis(CHSH_ANGLES['a2']), angle_to_axis(CHSH_ANGLES['b2'])),
            ]
        else:
            angle_pairs = [
                ('E11', CUBOCT_DIRS['a1'], CUBOCT_DIRS['b1']),
                ('E12', CUBOCT_DIRS['a1'], CUBOCT_DIRS['b2']),
                ('E21', CUBOCT_DIRS['a2'], CUBOCT_DIRS['b1']),
                ('E22', CUBOCT_DIRS['a2'], CUBOCT_DIRS['b2']),
            ]

        for name, a_hat, b_hat in angle_pairs:
            outcomes_A = np.zeros(n_trials, dtype=int)
            outcomes_B = np.zeros(n_trials, dtype=int)

            t0 = time.time()
            for trial in range(n_trials):
                u = Universe(size=grid_size, geometry=geom_class(grid_size))
                det_A, det_B = get_detector_positions(u)

                J0_dir = random_unit_vectors(1)[0]
                create_entangled_pair(u, J0_dir, amplitude=5.0)

                for _ in range(n_ticks):
                    waves.propagate_flux(u)
                    forces.calculate_density(u)

                outcomes_A[trial] = measure_passive(u, det_A, 3, a_hat)
                outcomes_B[trial] = -measure_passive(u, det_B, 3, b_hat)

            corr, eff = compute_correlation(outcomes_A, outcomes_B)
            elapsed = time.time() - t0
            all_corr[name] = {'correlation': corr, 'efficiency': eff}
            print(f"    {name} = {corr:+.4f}  (eff: {eff:.2%})  [{elapsed:.1f}s]")

        S = compute_chsh(
            all_corr['E11']['correlation'],
            all_corr['E12']['correlation'],
            all_corr['E21']['correlation'],
            all_corr['E22']['correlation'],
        )
        status = "*** VIOLATION ***" if S > 2.0 else "(no violation)"
        print(f"    S = {S:.4f}  {status}")
        print()

        results[config_name] = {'S': S, 'correlations': all_corr}

    CONSTANTS.DAMPING = original_damping
    return results


# ============================================================================
# EXPERIMENT 2: PASSIVE vs ACTIVE MEASUREMENT
# ============================================================================

def experiment_2(n_trials=250, grid_size=16, n_ticks=12, n_interact=3):
    """
    Compare passive and active measurement protocols on both geometries.

    Key hypothesis: Active measurement on cuboctahedral lattice introduces
    noncommutativity because non-orthogonal neighbor directions share flux
    components. The 12-neighbor Laplacian mixes measurement axes in a way
    that the 6-neighbor cubic Laplacian does not.

    For each geometry, we test:
      (a) Passive: read sign(J·a_hat) — no modification to flux
      (b) Active: place detector, let it couple, then read — modifies flux
    """
    print("\n" + "=" * 70)
    print("EXPERIMENT 2: PASSIVE vs ACTIVE MEASUREMENT")
    print("=" * 70)
    print(f"  Grid: {grid_size}, Trials: {n_trials}")
    print(f"  Propagation: {n_ticks} ticks, Interaction: {n_interact} ticks")
    print(f"  Active coupling strength: 2.0")
    print()

    original_damping = CONSTANTS.DAMPING
    CONSTANTS.DAMPING = 0.0

    results = {}

    configs = [
        ("cubic_passive", CubicGeometry, False),
        ("cubic_active", CubicGeometry, True),
        ("cuboct_passive", CuboctahedralGeometry, False),
        ("cuboct_active", CuboctahedralGeometry, True),
    ]

    for config_name, geom_class, is_active in configs:
        geom_label = "CUBOCTAHEDRAL" if geom_class == CuboctahedralGeometry else "CUBIC"
        meas_label = "ACTIVE" if is_active else "PASSIVE"
        print(f"  --- {geom_label} / {meas_label} ---")

        all_corr = {}
        angle_pairs = [
            ('E11', angle_to_axis(CHSH_ANGLES['a1']), angle_to_axis(CHSH_ANGLES['b1'])),
            ('E12', angle_to_axis(CHSH_ANGLES['a1']), angle_to_axis(CHSH_ANGLES['b2'])),
            ('E21', angle_to_axis(CHSH_ANGLES['a2']), angle_to_axis(CHSH_ANGLES['b1'])),
            ('E22', angle_to_axis(CHSH_ANGLES['a2']), angle_to_axis(CHSH_ANGLES['b2'])),
        ]

        for name, a_hat, b_hat in angle_pairs:
            outcomes_A = np.zeros(n_trials, dtype=int)
            outcomes_B = np.zeros(n_trials, dtype=int)

            t0 = time.time()
            for trial in range(n_trials):
                u = Universe(size=grid_size, geometry=geom_class(grid_size))
                det_A, det_B = get_detector_positions(u)

                J0_dir = random_unit_vectors(1)[0]
                create_entangled_pair(u, J0_dir, amplitude=5.0)

                # Propagate flux to detectors
                for _ in range(n_ticks):
                    waves.propagate_flux(u)
                    forces.calculate_density(u)

                if not is_active:
                    # Passive: read flux without modification
                    outcomes_A[trial] = measure_passive(u, det_A, 3, a_hat)
                    outcomes_B[trial] = -measure_passive(u, det_B, 3, b_hat)
                else:
                    # Active: each measurement gets its own copy of the universe
                    # so Alice's measurement doesn't contaminate Bob's
                    import copy

                    u_for_A = Universe(size=grid_size, geometry=geom_class(grid_size))
                    u_for_A.flux[:] = u.flux
                    u_for_A.wave_velocity[:] = u.wave_velocity
                    u_for_A.density[:] = u.density
                    u_for_A.states[:] = u.states
                    u_for_A.charge[:] = u.charge

                    u_for_B = Universe(size=grid_size, geometry=geom_class(grid_size))
                    u_for_B.flux[:] = u.flux
                    u_for_B.wave_velocity[:] = u.wave_velocity
                    u_for_B.density[:] = u.density
                    u_for_B.states[:] = u.states
                    u_for_B.charge[:] = u.charge

                    outcomes_A[trial] = measure_active(
                        u_for_A, det_A, 3, a_hat,
                        coupling_strength=2.0, n_interact_ticks=n_interact
                    )
                    outcomes_B[trial] = -measure_active(
                        u_for_B, det_B, 3, b_hat,
                        coupling_strength=2.0, n_interact_ticks=n_interact
                    )

            corr, eff = compute_correlation(outcomes_A, outcomes_B)
            elapsed = time.time() - t0
            all_corr[name] = {'correlation': corr, 'efficiency': eff}
            print(f"    {name} = {corr:+.4f}  (eff: {eff:.2%})  [{elapsed:.1f}s]")

        S = compute_chsh(
            all_corr['E11']['correlation'],
            all_corr['E12']['correlation'],
            all_corr['E21']['correlation'],
            all_corr['E22']['correlation'],
        )
        status = "*** VIOLATION ***" if S > 2.0 else "(no violation)"
        print(f"    S = {S:.4f}  {status}")
        print()

        results[config_name] = {'S': S, 'correlations': all_corr}

    CONSTANTS.DAMPING = original_damping
    return results


# ============================================================================
# EXPERIMENT 3: EMBEDDED OBSERVER MODEL
# ============================================================================

def create_observer_cluster(universe, center, radius=2, measurement_axis=None):
    """
    Create an embedded observer: a cluster of locked manifested voxels.
    The observer is part of the same flux field as the entangled particles.

    On FCC, only valid lattice sites within the sphere are activated.
    The cluster has a net flux orientation set by measurement_axis.

    Returns the number of observer voxels placed.
    """
    cx, cy, cz = center
    s = universe.size
    is_fcc = universe.geometry.name == 'cuboctahedral'
    count = 0

    for dx in range(-radius, radius + 1):
        for dy in range(-radius, radius + 1):
            for dz in range(-radius, radius + 1):
                if dx * dx + dy * dy + dz * dz > radius * radius:
                    continue
                x = (cx + dx) % s
                y = (cy + dy) % s
                z = (cz + dz) % s

                if is_fcc and not is_valid_fcc(x, y, z):
                    continue

                universe.states[x, y, z] = 1
                universe.is_locked[x, y, z] = True
                universe.charge[x, y, z] = 0.5

                if measurement_axis is not None:
                    universe.flux[x, y, z] = (
                        np.array(measurement_axis, dtype=np.float32) * 0.5
                    )
                count += 1

    return count


def measure_observer_response(universe, center, radius, axis):
    """
    Read the observer cluster's response to incoming entangled flux.

    The measurement outcome is the net flux of the locked observer voxels
    projected onto the measurement axis. This represents the observer
    computing the correlation THROUGH ITS OWN LATTICE DYNAMICS — it
    doesn't "read" external data, it responds to flux that has interacted
    with its own manifested structure.

    This is the sLoop measurement: the observer is ontologically part of
    the system being observed.
    """
    cx, cy, cz = center
    s = universe.size
    is_fcc = universe.geometry.name == 'cuboctahedral'

    total_flux = np.zeros(3, dtype=np.float64)
    for dx in range(-radius, radius + 1):
        for dy in range(-radius, radius + 1):
            for dz in range(-radius, radius + 1):
                if dx * dx + dy * dy + dz * dz > radius * radius:
                    continue
                x = (cx + dx) % s
                y = (cy + dy) % s
                z = (cz + dz) % s
                if is_fcc and not is_valid_fcc(x, y, z):
                    continue
                if universe.is_locked[x, y, z]:
                    total_flux += universe.flux[x, y, z].astype(np.float64)

    projection = np.dot(total_flux, axis)
    if abs(projection) < 1e-10:
        return 0
    return int(np.sign(projection))


def experiment_3(n_trials=200, grid_size=16, n_ticks=15, observer_radius=2):
    """
    Embedded observer experiment on cuboctahedral lattice.

    Two observer clusters are embedded in the FCC lattice. An entangled pair
    is created between them. The observers interact with the incoming flux
    through the SAME lattice dynamics that propagate the entanglement.

    Key distinction from Experiments 1-2: the observer's measurement outcome
    is determined by its OWN flux response (sLoop), not an external readout.
    """
    print("\n" + "=" * 70)
    print("EXPERIMENT 3: EMBEDDED OBSERVER (sLOOP) ON CUBOCTAHEDRAL LATTICE")
    print("=" * 70)
    print(f"  Grid: {grid_size} (FCC internal: {2*grid_size})")
    print(f"  Trials: {n_trials}, Ticks: {n_ticks}")
    print(f"  Observer radius: {observer_radius}")
    print(f"  Physics: waves + forces (full sLoop coupling)")
    print()

    original_damping = CONSTANTS.DAMPING
    CONSTANTS.DAMPING = 0.0

    results = {}

    # Test with both standard angles and D_3 root directions
    angle_configs = [
        ("standard_angles", [
            ('E11', angle_to_axis(CHSH_ANGLES['a1']), angle_to_axis(CHSH_ANGLES['b1'])),
            ('E12', angle_to_axis(CHSH_ANGLES['a1']), angle_to_axis(CHSH_ANGLES['b2'])),
            ('E21', angle_to_axis(CHSH_ANGLES['a2']), angle_to_axis(CHSH_ANGLES['b1'])),
            ('E22', angle_to_axis(CHSH_ANGLES['a2']), angle_to_axis(CHSH_ANGLES['b2'])),
        ]),
        ("d3_root_dirs", [
            ('E11', CUBOCT_DIRS['a1'], CUBOCT_DIRS['b1']),
            ('E12', CUBOCT_DIRS['a1'], CUBOCT_DIRS['b2']),
            ('E21', CUBOCT_DIRS['a2'], CUBOCT_DIRS['b1']),
            ('E22', CUBOCT_DIRS['a2'], CUBOCT_DIRS['b2']),
        ]),
    ]

    for angle_name, angle_pairs in angle_configs:
        label = angle_name.replace("_", " ").upper()
        print(f"  --- {label} ---")

        all_corr = {}
        n_obs_A = n_obs_B = 0

        for name, a_hat, b_hat in angle_pairs:
            outcomes_A = np.zeros(n_trials, dtype=int)
            outcomes_B = np.zeros(n_trials, dtype=int)

            t0 = time.time()
            for trial in range(n_trials):
                u = Universe(
                    size=grid_size,
                    geometry=CuboctahedralGeometry(grid_size)
                )
                det_A, det_B = get_detector_positions(u)

                # Create observer clusters oriented along measurement axes
                n_obs_A = create_observer_cluster(
                    u, det_A, observer_radius, a_hat
                )
                n_obs_B = create_observer_cluster(
                    u, det_B, observer_radius, b_hat
                )

                # Create entangled pair at center (between observers)
                J0_dir = random_unit_vectors(1)[0]
                create_entangled_pair(u, J0_dir, amplitude=5.0)

                # Propagate with FULL dynamics (waves + forces = sLoop)
                for _ in range(n_ticks):
                    waves.propagate_flux(u)
                    forces.calculate_density(u)
                    forces.accumulate_forces(u)

                # Read observer responses through their own dynamics
                outcomes_A[trial] = measure_observer_response(
                    u, det_A, observer_radius, a_hat
                )
                outcomes_B[trial] = -measure_observer_response(
                    u, det_B, observer_radius, b_hat
                )

            corr, eff = compute_correlation(outcomes_A, outcomes_B)
            elapsed = time.time() - t0
            all_corr[name] = {'correlation': corr, 'efficiency': eff}
            print(f"    {name} = {corr:+.4f}  (eff: {eff:.2%}, "
                  f"obs: {n_obs_A}+{n_obs_B})  [{elapsed:.1f}s]")

        S = compute_chsh(
            all_corr['E11']['correlation'],
            all_corr['E12']['correlation'],
            all_corr['E21']['correlation'],
            all_corr['E22']['correlation'],
        )
        status = "*** VIOLATION ***" if S > 2.0 else "(no violation)"
        print(f"    S = {S:.4f}  {status}")
        print()

        results[angle_name] = {'S': S, 'correlations': all_corr}

    CONSTANTS.DAMPING = original_damping
    return results


# ============================================================================
# CORRELATION FUNCTION SHAPE ANALYSIS
# ============================================================================

def correlation_shape_analysis(n_angles=18, n_trials=200, grid_size=16, n_ticks=12):
    """
    Compare E(theta) shape between cubic and cuboctahedral lattices.

    Classical local HV: E(theta) = -(1 - 2|theta|/pi)  (triangle)
    Quantum singlet:    E(theta) = -cos(theta)

    The cuboctahedral lattice might produce a different shape due to
    the non-orthogonal neighbor structure in the 12-point Laplacian.
    """
    print("\n" + "=" * 70)
    print("CORRELATION FUNCTION SHAPE ANALYSIS")
    print("=" * 70)
    print(f"  Angles: {n_angles}, Trials/angle: {n_trials}, Grid: {grid_size}")
    print()

    original_damping = CONSTANTS.DAMPING
    CONSTANTS.DAMPING = 0.0

    angles = np.linspace(0, np.pi, n_angles)
    results = {}

    for geom_name, geom_class in [
        ("cubic", CubicGeometry),
        ("cuboctahedral", CuboctahedralGeometry),
    ]:
        correlations = []
        print(f"  {geom_name}: ", end="", flush=True)

        for theta in angles:
            a_hat = np.array([0.0, 0.0, 1.0])  # fixed Alice axis = z
            b_hat = angle_to_axis(theta)

            outcomes_A = np.zeros(n_trials, dtype=int)
            outcomes_B = np.zeros(n_trials, dtype=int)

            for trial in range(n_trials):
                u = Universe(size=grid_size, geometry=geom_class(grid_size))
                det_A, det_B = get_detector_positions(u)

                J0_dir = random_unit_vectors(1)[0]
                create_entangled_pair(u, J0_dir, amplitude=5.0)

                for _ in range(n_ticks):
                    waves.propagate_flux(u)
                    forces.calculate_density(u)

                outcomes_A[trial] = measure_passive(u, det_A, 3, a_hat)
                outcomes_B[trial] = -measure_passive(u, det_B, 3, b_hat)

            corr, _ = compute_correlation(outcomes_A, outcomes_B)
            correlations.append(corr)
            print(".", end="", flush=True)

        results[geom_name] = np.array(correlations)
        print(" done")

    # Print comparison table
    print(f"\n  {'Angle':>8} {'Cubic':>8} {'Cuboct':>8} "
          f"{'QM -cos':>9} {'Triangle':>9} {'C-Q diff':>9}")
    print(f"  {'-' * 55}")
    for i, theta in enumerate(angles):
        qm = -np.cos(theta)
        tri = -(1 - 2 * abs(theta) / np.pi)
        c_val = results['cubic'][i]
        q_val = results['cuboctahedral'][i]
        diff = q_val - c_val
        print(f"  {np.degrees(theta):7.1f}  {c_val:+.4f}  {q_val:+.4f}  "
              f"{qm:+.5f}  {tri:+.5f}  {diff:+.5f}")

    CONSTANTS.DAMPING = original_damping
    return angles, results


# ============================================================================
# MAIN
# ============================================================================

def main():
    np.random.seed(42)  # Reproducibility

    print("=" * 70)
    print("CUBOCTAHEDRAL BELL EXPERIMENTS")
    print("Can FCC Geometry + Active Measurement + Embedded Observers")
    print("Produce S > 2 From Pure Lattice Dynamics?")
    print("=" * 70)
    print(f"Date: February 16, 2026")
    print(f"Framework: FTD v5.26")
    print()
    print("Mathematical basis:")
    print(f"  12 FCC vectors = D_3 root system = su(4) = so(6)")
    print(f"  su(4) -> su(2)_A x su(2)_B x u(1) branching")
    print(f"  Tsirelson bound 2*sqrt(2) = {2 * np.sqrt(2):.6f}")
    print(f"                            = 2 x sqrt(2) = 2 x (FCC neighbor distance)")
    print(f"  CHSH optimal b1 = (1,0,1)/sqrt(2) IS a cuboctahedral direction")
    print()
    print("Honest expectation: S <= 2 everywhere (local deterministic substrate).")
    print("Open question: Does the cuboctahedral algebra change anything?")
    print()

    all_S_values = []

    # --- Experiment 1 ---
    t_start = time.time()
    r1 = experiment_1(n_trials=400, grid_size=16, n_ticks=12)
    for v in r1.values():
        all_S_values.append((v['S'], 'Exp1'))

    # --- Experiment 2 ---
    r2 = experiment_2(n_trials=250, grid_size=16, n_ticks=12, n_interact=3)
    for k, v in r2.items():
        all_S_values.append((v['S'], f'Exp2:{k}'))

    # --- Experiment 3 ---
    r3 = experiment_3(n_trials=200, grid_size=16, n_ticks=15, observer_radius=2)
    for k, v in r3.items():
        all_S_values.append((v['S'], f'Exp3:{k}'))

    # --- Correlation Shape ---
    angles, shapes = correlation_shape_analysis(
        n_angles=18, n_trials=200, grid_size=16, n_ticks=12
    )

    total_time = time.time() - t_start

    # ================================================================
    # COMPREHENSIVE SUMMARY
    # ================================================================
    print("\n" + "=" * 70)
    print("COMPREHENSIVE RESULTS SUMMARY")
    print("=" * 70)

    print(f"\n  {'Experiment':<52} {'S':>8} {'Result':>12}")
    print(f"  {'-' * 72}")

    # Exp 1
    for key in ['cubic_standard', 'cuboct_standard', 'cuboct_d3_dirs']:
        if key in r1:
            S = r1[key]['S']
            st = "VIOLATION!" if S > 2.0 else "S <= 2"
            label = f"Exp1: {key.replace('_', ' ')}"
            print(f"  {label:<52} {S:8.4f} {st:>12}")

    # Exp 2
    for key in ['cubic_passive', 'cubic_active', 'cuboct_passive', 'cuboct_active']:
        if key in r2:
            S = r2[key]['S']
            st = "VIOLATION!" if S > 2.0 else "S <= 2"
            label = f"Exp2: {key.replace('_', ' ')}"
            print(f"  {label:<52} {S:8.4f} {st:>12}")

    # Exp 3
    for key, label_suffix in [
        ('standard_angles', 'standard angles'),
        ('d3_root_dirs', 'D3 root directions'),
    ]:
        if key in r3:
            S = r3[key]['S']
            st = "VIOLATION!" if S > 2.0 else "S <= 2"
            label = f"Exp3: embedded observer ({label_suffix})"
            print(f"  {label:<52} {S:8.4f} {st:>12}")

    print(f"\n  Classical bound:  S <= 2.0000")
    print(f"  Tsirelson bound:  S <= {2 * np.sqrt(2):.4f}")

    max_S = max(s for s, _ in all_S_values) if all_S_values else 0
    max_label = [label for s, label in all_S_values if s == max_S][0]

    print("\n" + "=" * 70)
    print("VERDICT")
    print("=" * 70)

    if max_S > 2.0:
        print(f"""
  *** S > 2 OBSERVED (max S = {max_S:.4f} in {max_label}) ***

  Before accepting this result, verify:
  1. Statistical significance — run with 10x more trials
  2. Detection loophole — check null-outcome rates
  3. Code correctness — verify CHSH formula and sign conventions
  4. Is this a genuine lattice effect or a measurement artifact?

  If robust, this would mean the cuboctahedral lattice dynamics
  produce noncommutative measurement structure from purely local rules.
""")
    else:
        print(f"""
  NO BELL VIOLATION OBSERVED (max S = {max_S:.4f} in {max_label}).

  The cuboctahedral lattice geometry does NOT produce S > 2, even with:
  - 12-neighbor FCC Laplacian (vs 6-neighbor cubic)
  - Active measurement (detector-flux coupling)
  - Embedded observer clusters (sLoop)
  - Cuboctahedral D_3 measurement directions

  INTERPRETATION:
  The D_3 = su(4) algebraic structure is PRESENT in the geometry but
  does NOT automatically generate measurement noncommutativity. The
  geometric algebra provides the CONTAINER for quantum correlations,
  but the actual measurement operators remain commutative (sign functions
  on a real-valued flux field).

  The bridge from geometric algebra to measurement algebra requires
  something beyond the classical wave equation — likely a genuine
  complex structure (i.e., the complexified flux psi = Jx + i*Jy)
  with non-commuting measurement operators.

  The substrate correctly gives S <= 2. Bell violations remain an
  aggregate/epistemic property, not a substrate behavior that can be
  produced by changing the lattice geometry alone.
""")

    print(f"  Total runtime: {total_time:.1f}s")
    print("=" * 70)
    print("END OF CUBOCTAHEDRAL BELL EXPERIMENTS")
    print("=" * 70)


if __name__ == "__main__":
    main()
