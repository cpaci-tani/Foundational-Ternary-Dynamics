"""
FTD CA ↔ Analytic Bridge Investigation
========================================

Does the ternary_matrix cellular automata produce emergent observables
that match (or relate to) the analytic FTD predictions?

The analytic layer derives constants from {3, 4, 7, 13} and G*.
The CA layer evolves a discrete lattice with a 12-phase tick() cycle.
This script attempts to bridge the two by measuring emergent quantities
from the simulation dynamics.

Author: AI-assisted research (February 2026)
Framework: FTD v5.24
"""

import sys
import os
import time
import numpy as np
from collections import defaultdict

# Add project root to path
PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
sys.path.insert(0, PROJECT_ROOT)

from ternary_matrix.config import PhysicsConfig, get_test_config
from ternary_matrix.model.grid import Universe
from ternary_matrix.physics.master_equation import tick, get_diagnostics, run_simulation
from ternary_matrix.physics.binding import get_triad_count, get_binding_energy, detect_triads
from ternary_matrix.analysis.structure_metrics import analyze_clusters

# FTD analytic constants for comparison
from simulations.constants import (
    N_c, N_base, b_3, N_eff, ALPHA, ALPHA_INV,
    X_PLUS, X_MINUS, G_STAR, PHI
)


# =============================================================================
# EXPERIMENT 1: Triad Formation Dynamics
# =============================================================================

def experiment_triad_dynamics(grid_size=32, num_ticks=500, seed=42):
    """
    Run the CA and track triad (3-body bound state) formation over time.

    Key question: Does the equilibrium ratio of bound-to-free particles
    relate to any framework constants?

    Analytic prediction: Triads should stabilize when binding energy
    (KB × PHI per triad) balances decay rate (ALPHA).
    The equilibrium condition suggests:
        N_bound / N_free ~ f(PHI, ALPHA, KB)
    """
    print("=" * 70)
    print("EXPERIMENT 1: TRIAD FORMATION DYNAMICS")
    print("=" * 70)

    np.random.seed(seed)
    config = get_test_config(grid_size=grid_size)
    from ternary_matrix.config import get_geometry
    geometry = get_geometry(config)
    universe = Universe(size=grid_size, geometry=geometry)

    # Seed the lattice with random matter (both polarities)
    # Use a moderate seeding density (~5% of lattice)
    n_voxels = grid_size ** 3
    n_seed = int(0.05 * n_voxels)

    # Random positions
    positions = np.random.choice(n_voxels, size=n_seed, replace=False)
    coords = np.unravel_index(positions, (grid_size, grid_size, grid_size))

    # Equal positive and negative
    half = n_seed // 2
    universe.states[coords[0][:half], coords[1][:half], coords[2][:half]] = 1
    universe.states[coords[0][half:], coords[1][half:], coords[2][half:]] = -1
    universe.sync_charge_from_state()

    # Seed flux field with small random perturbations
    universe.flux = np.random.normal(0, 0.3, universe.flux.shape).astype(np.float32)

    # Track observables over time
    history = {
        'tick': [],
        'manifested': [],
        'positive': [],
        'negative': [],
        'triads': [],
        'binding_energy': [],
        'total_flux': [],
        'bound_ratio': [],
        'charge_imbalance': [],
    }

    sample_interval = max(1, num_ticks // 100)

    print(f"\nGrid: {grid_size}³ = {n_voxels} voxels")
    print(f"Seeded: {n_seed} particles ({half} positive, {n_seed - half} negative)")
    print(f"Running {num_ticks} ticks...")
    print()

    t_start = time.time()

    for t in range(num_ticks):
        tick(universe)

        if t % sample_interval == 0 or t == num_ticks - 1:
            diag = get_diagnostics(universe)
            n_triads = get_triad_count(universe)
            be = get_binding_energy(universe)
            n_bound = np.count_nonzero(universe.is_locked)
            n_manifested = diag['manifested_count']
            n_free = max(1, n_manifested - n_bound)

            history['tick'].append(t)
            history['manifested'].append(n_manifested)
            history['positive'].append(diag['positive_count'])
            history['negative'].append(diag['negative_count'])
            history['triads'].append(n_triads)
            history['binding_energy'].append(be)
            history['total_flux'].append(diag['total_flux'])
            history['bound_ratio'].append(n_bound / n_free if n_free > 0 else 0)
            history['charge_imbalance'].append(abs(diag['total_charge']))

    elapsed = time.time() - t_start
    print(f"Simulation complete in {elapsed:.1f}s")
    print()

    # =========================================================================
    # Analysis: What ratios emerge?
    # =========================================================================

    final_manifested = history['manifested'][-1]
    final_triads = history['triads'][-1]
    final_bound = np.count_nonzero(universe.is_locked)
    final_free = max(1, final_manifested - final_bound)
    final_ratio = final_bound / final_free

    print("--- FINAL STATE ---")
    print(f"  Manifested particles:  {final_manifested}")
    print(f"  Bound (locked):        {final_bound}")
    print(f"  Free (unlocked):       {final_free}")
    print(f"  Triads detected:       {final_triads}")
    print(f"  Bound/Free ratio:      {final_ratio:.6f}")
    print(f"  Binding energy:        {history['binding_energy'][-1]:.4f}")
    print(f"  Total flux:            {history['total_flux'][-1]:.4f}")
    print(f"  Charge imbalance:      {history['charge_imbalance'][-1]:.4f}")
    print()

    # Compare emergent ratios to framework constants
    print("--- FRAMEWORK CONSTANT COMPARISONS ---")
    print()

    comparisons = [
        ("α (fine structure)",         ALPHA,          0.00729),
        ("1/α",                        ALPHA_INV,      137.036),
        ("N_c",                        N_c,            3),
        ("N_base",                     N_base,         4),
        ("b_3",                        b_3,            7),
        ("N_eff",                      N_eff,          13),
        ("φ (golden ratio)",           PHI,            1.618),
        ("x₋ (small root)",           X_MINUS,        3.024),
        ("α × φ",                      ALPHA * PHI,    0.01179),
        ("N_c / N_eff",                N_c / N_eff,    0.2308),
    ]

    print(f"  Emergent bound/free ratio: {final_ratio:.6f}")
    print()

    for name, value, approx in comparisons:
        if final_ratio > 0 and value > 0:
            ratio_to_const = final_ratio / value
            print(f"  ratio / {name:25s} = {ratio_to_const:.6f}  ({name} ≈ {approx})")

    print()

    # Check whether triads/manifested approaches a framework ratio
    if final_manifested > 0:
        triad_frac = final_triads * 3 / final_manifested  # fraction in triads
        print(f"  Fraction of particles in triads: {triad_frac:.6f}")
        print(f"  Compare to 1/N_c = {1/N_c:.6f}")
        print(f"  Compare to N_c/N_eff = {N_c/N_eff:.6f}")
        print(f"  Compare to α = {ALPHA:.6f}")

    return history


# =============================================================================
# EXPERIMENT 2: Cluster Size Distribution
# =============================================================================

def experiment_cluster_distribution(grid_size=32, num_ticks=300, seed=42):
    """
    Run the CA and analyze the cluster size distribution at equilibrium.

    Key question: Does the cluster size distribution follow a power law?
    If so, does the exponent relate to framework integers?

    In critical phenomena, cluster size distributions follow P(s) ~ s^(-τ)
    where τ depends on the universality class. If the FTD lattice sits at
    criticality, τ should be characteristic of the framework.
    """
    print("\n" + "=" * 70)
    print("EXPERIMENT 2: CLUSTER SIZE DISTRIBUTION")
    print("=" * 70)

    np.random.seed(seed)
    config = get_test_config(grid_size=grid_size)
    from ternary_matrix.config import get_geometry
    geometry = get_geometry(config)
    universe = Universe(size=grid_size, geometry=geometry)

    # Dense seeding to ensure cluster formation
    n_voxels = grid_size ** 3
    n_seed = int(0.10 * n_voxels)
    positions = np.random.choice(n_voxels, size=n_seed, replace=False)
    coords = np.unravel_index(positions, (grid_size, grid_size, grid_size))

    half = n_seed // 2
    universe.states[coords[0][:half], coords[1][:half], coords[2][:half]] = 1
    universe.states[coords[0][half:], coords[1][half:], coords[2][half:]] = -1
    universe.sync_charge_from_state()
    universe.flux = np.random.normal(0, 0.5, universe.flux.shape).astype(np.float32)

    print(f"\nGrid: {grid_size}³, seeded with {n_seed} particles")
    print(f"Running {num_ticks} ticks to reach equilibrium...")

    # Run to equilibrium
    run_simulation(universe, num_ticks)

    # Analyze cluster distribution
    clusters = analyze_clusters(universe)

    print(f"\nCluster analysis at tick {universe.tick}:")
    print(f"  Total manifested: {universe.get_manifested_count()}")
    print(f"  Distinct cluster sizes found: {len(clusters)}")
    print()

    if clusters:
        print("  Size  | Count | Size × Count")
        print("  ------|-------|-------------")
        total_in_clusters = 0
        for size in sorted(clusters.keys()):
            count = clusters[size]
            total_in_clusters += size * count
            print(f"  {size:5d} | {count:5d} | {size * count:8d}")

        print(f"\n  Total particles in clusters: {total_in_clusters}")

        # Check for power-law behavior
        sizes = np.array(sorted(clusters.keys()))
        counts = np.array([clusters[s] for s in sizes])

        if len(sizes) > 3:
            # Log-log fit for power law exponent
            log_s = np.log(sizes)
            log_c = np.log(counts)
            # Linear regression in log space
            coeffs = np.polyfit(log_s, log_c, 1)
            tau = -coeffs[0]  # Power law exponent

            print(f"\n  Power-law fit: P(s) ~ s^(-{tau:.3f})")
            print(f"  Compare to framework values:")
            print(f"    N_c = {N_c} (percolation: τ ≈ 2.18 in 3D)")
            print(f"    N_base = {N_base}")
            print(f"    b_3/N_c = {b_3/N_c:.3f}")
            print(f"    N_eff/N_base = {N_eff/N_base:.3f}")
    else:
        print("  No clusters found (vacuum state).")

    return clusters


# =============================================================================
# EXPERIMENT 3: Force Ratio Measurement
# =============================================================================

def experiment_force_ratios(grid_size=32, num_ticks=200, seed=42):
    """
    Measure the effective ratio of different force magnitudes in the CA.

    Key question: The CA has 5 force types with coupling constants set
    by PhysicsConfig. But the *effective* force ratios at equilibrium
    depend on the spatial structure of matter — which is emergent.

    We measure <|F_grav|>, <|F_coulomb|>, <|F_lorentz|>, <|F_strong|>
    at each tick and track how their ratios evolve.
    """
    print("\n" + "=" * 70)
    print("EXPERIMENT 3: EMERGENT FORCE RATIOS")
    print("=" * 70)

    from ternary_matrix.physics.forces import (
        gravity_force, coulomb_force, lorentz_force, strong_force, weak_stress,
        calculate_density
    )

    np.random.seed(seed)
    config = get_test_config(grid_size=grid_size)
    from ternary_matrix.config import get_geometry
    geometry = get_geometry(config)
    universe = Universe(size=grid_size, geometry=geometry)

    # Seed
    n_voxels = grid_size ** 3
    n_seed = int(0.08 * n_voxels)
    positions = np.random.choice(n_voxels, size=n_seed, replace=False)
    coords = np.unravel_index(positions, (grid_size, grid_size, grid_size))
    half = n_seed // 2
    universe.states[coords[0][:half], coords[1][:half], coords[2][:half]] = 1
    universe.states[coords[0][half:], coords[1][half:], coords[2][half:]] = -1
    universe.sync_charge_from_state()
    universe.flux = np.random.normal(0, 0.4, universe.flux.shape).astype(np.float32)

    print(f"\nGrid: {grid_size}³, seeded with {n_seed} particles")
    print(f"Measuring force magnitudes over {num_ticks} ticks...")

    force_history = {
        'gravity': [],
        'coulomb': [],
        'lorentz': [],
        'strong': [],
        'weak_stress': [],
    }

    sample_interval = max(1, num_ticks // 50)

    for t in range(num_ticks):
        tick(universe)

        if t % sample_interval == 0 or t == num_ticks - 1:
            # Recompute density for force calculations
            calculate_density(universe)

            manifested = universe.states != 0
            n_active = np.count_nonzero(manifested)

            if n_active > 0:
                f_g = gravity_force(universe)
                f_c = coulomb_force(universe)
                f_l = lorentz_force(universe)
                f_s = strong_force(universe)
                w_s = weak_stress(universe)

                # Mean force magnitude over manifested voxels
                mag = lambda f: np.mean(np.sqrt(np.sum(f[manifested] ** 2, axis=-1))) if n_active > 0 else 0

                force_history['gravity'].append(mag(f_g))
                force_history['coulomb'].append(mag(f_c))
                force_history['lorentz'].append(mag(f_l))
                force_history['strong'].append(mag(f_s))
                force_history['weak_stress'].append(np.mean(w_s[manifested]) if n_active > 0 else 0)

    print()

    # Time-averaged force magnitudes
    avg_forces = {}
    for name, values in force_history.items():
        if values:
            avg_forces[name] = np.mean(values[-10:])  # Average over last 10 samples
        else:
            avg_forces[name] = 0.0

    print("--- TIME-AVERAGED FORCE MAGNITUDES (last 10 samples) ---")
    for name, val in avg_forces.items():
        print(f"  <|F_{name}|> = {val:.6e}")

    # Compute ratios
    print("\n--- EMERGENT FORCE RATIOS ---")

    if avg_forces['coulomb'] > 0 and avg_forces['strong'] > 0:
        em_strong_ratio = avg_forces['coulomb'] / avg_forces['strong']
        print(f"  |F_coulomb| / |F_strong| = {em_strong_ratio:.6f}")
        print(f"    Compare to α = {ALPHA:.6f}")
        print(f"    Compare to α/α_s ≈ {ALPHA / 0.1186:.6f}")

    if avg_forces['gravity'] > 0 and avg_forces['coulomb'] > 0:
        grav_em_ratio = avg_forces['gravity'] / avg_forces['coulomb']
        print(f"  |F_gravity| / |F_coulomb| = {grav_em_ratio:.6f}")

    if avg_forces['lorentz'] > 0 and avg_forces['coulomb'] > 0:
        lor_coul_ratio = avg_forces['lorentz'] / avg_forces['coulomb']
        print(f"  |F_lorentz| / |F_coulomb| = {lor_coul_ratio:.6f}")
        print(f"    Compare to β/1 = {config.BETA:.6f} (input ratio)")

    return force_history, avg_forces


# =============================================================================
# EXPERIMENT 4: Equilibrium Density Scale
# =============================================================================

def experiment_density_scale(grid_size=32, num_ticks=400, seed=42):
    """
    After running to equilibrium, measure the characteristic density scale.

    Key question: Does the equilibrium density profile center around
    KB = 0.511 (the manifestation threshold, set to electron mass)?
    What is the emergent density distribution?
    """
    print("\n" + "=" * 70)
    print("EXPERIMENT 4: EQUILIBRIUM DENSITY SCALE")
    print("=" * 70)

    np.random.seed(seed)
    config = get_test_config(grid_size=grid_size)
    from ternary_matrix.config import get_geometry
    geometry = get_geometry(config)
    universe = Universe(size=grid_size, geometry=geometry)

    # Seed
    n_voxels = grid_size ** 3
    n_seed = int(0.08 * n_voxels)
    positions = np.random.choice(n_voxels, size=n_seed, replace=False)
    coords = np.unravel_index(positions, (grid_size, grid_size, grid_size))
    half = n_seed // 2
    universe.states[coords[0][:half], coords[1][:half], coords[2][:half]] = 1
    universe.states[coords[0][half:], coords[1][half:], coords[2][half:]] = -1
    universe.sync_charge_from_state()
    universe.flux = np.random.normal(0, 0.5, universe.flux.shape).astype(np.float32)

    print(f"\nGrid: {grid_size}³, seeded with {n_seed} particles")
    print(f"Running {num_ticks} ticks to equilibrium...")

    run_simulation(universe, num_ticks)

    # Analyze density distribution
    from ternary_matrix.physics.forces import calculate_density
    calculate_density(universe)

    manifested = universe.states != 0
    n_manifested = np.count_nonzero(manifested)

    print(f"\n--- DENSITY ANALYSIS AT TICK {universe.tick} ---")
    print(f"  Manifested voxels: {n_manifested}")

    if n_manifested > 0:
        rho_manifested = universe.density[manifested]
        rho_mean = np.mean(rho_manifested)
        rho_median = np.median(rho_manifested)
        rho_std = np.std(rho_manifested)
        rho_min = np.min(rho_manifested)
        rho_max = np.max(rho_manifested)

        print(f"  Density (manifested voxels):")
        print(f"    Mean:   {rho_mean:.6f}")
        print(f"    Median: {rho_median:.6f}")
        print(f"    Std:    {rho_std:.6f}")
        print(f"    Min:    {rho_min:.6f}")
        print(f"    Max:    {rho_max:.6f}")
        print()
        print(f"  Manifestation threshold KB = {config.KB:.4f}")
        print(f"  Ratio mean(ρ)/KB = {rho_mean / config.KB:.6f}")
        print(f"  Compare to φ = {PHI:.6f}")
        print(f"  Compare to N_c = {N_c}")
        print(f"  Compare to x₋ = {X_MINUS:.6f}")

        # Density histogram (text-based)
        print("\n  Density histogram (manifested voxels):")
        n_bins = 10
        counts, bin_edges = np.histogram(rho_manifested, bins=n_bins)
        max_count = max(counts) if max(counts) > 0 else 1
        for i in range(n_bins):
            bar_len = int(40 * counts[i] / max_count)
            bar = "█" * bar_len
            print(f"    [{bin_edges[i]:6.3f}, {bin_edges[i+1]:6.3f}) | {bar} ({counts[i]})")

    # Also check the vacuum (state=0) density
    vacuum = universe.states == 0
    n_vacuum = np.count_nonzero(vacuum)
    if n_vacuum > 0:
        rho_vacuum = universe.density[vacuum]
        rho_vac_mean = np.mean(rho_vacuum)
        rho_vac_above_kb = np.count_nonzero(rho_vacuum > config.KB)
        print(f"\n  Vacuum density:")
        print(f"    Mean: {rho_vac_mean:.6f}")
        print(f"    Voxels above KB: {rho_vac_above_kb} ({100*rho_vac_above_kb/n_vacuum:.2f}%)")
        print(f"    These are 'almost-manifesting' sites — the quantum foam")


# =============================================================================
# MAIN
# =============================================================================

def main():
    print("╔══════════════════════════════════════════════════════════════════════╗")
    print("║        FTD CA ↔ ANALYTIC BRIDGE INVESTIGATION                      ║")
    print("║        Does the simulation produce emergent framework constants?    ║")
    print("╚══════════════════════════════════════════════════════════════════════╝")
    print()
    print(f"Framework integers: N_c={N_c}, N_base={N_base}, b_3={b_3}, N_eff={N_eff}")
    print(f"Analytic constants: α={ALPHA:.6f}, 1/α={ALPHA_INV:.6f}, G*={G_STAR:.6f}")
    print(f"Golden ratio: φ={PHI:.6f}")
    print()

    # Run all experiments
    h1 = experiment_triad_dynamics(grid_size=32, num_ticks=500)
    c2 = experiment_cluster_distribution(grid_size=32, num_ticks=300)
    f3, a3 = experiment_force_ratios(grid_size=32, num_ticks=200)
    experiment_density_scale(grid_size=32, num_ticks=400)

    # ==========================================================================
    # SYNTHESIS
    # ==========================================================================
    print("\n" + "=" * 70)
    print("SYNTHESIS: BRIDGE STATUS")
    print("=" * 70)
    print()
    print("The CA simulation has been run and emergent observables measured.")
    print("The key question is whether any emergent ratio matches a framework")
    print("constant WITHOUT that constant being an explicit input parameter.")
    print()
    print("INPUT parameters (directly set in PhysicsConfig):")
    print("  ALPHA = 0.00729  (used in DECAY_RATE)")
    print("  GRAVITY_BIAS = 0.01")
    print("  BETA = 0.01  (magnetic coupling)")
    print("  G_STRONG = 1.0")
    print("  KB = 0.511  (manifestation threshold)")
    print("  PHI = 1.618  (binding energy scale)")
    print()
    print("EMERGENT quantities (NOT directly set):")
    print("  - Bound/free particle ratio")
    print("  - Cluster size distribution exponent")
    print("  - Force magnitude ratios at equilibrium")
    print("  - Equilibrium density scale")
    print()
    print("If an emergent quantity matches a framework constant that is NOT")
    print("a direct input, that would be evidence for the bridge.")
    print()
    print("CRITICAL OBSERVATION:")
    print("  The 12-phase tick() takes α as INPUT (via DECAY_RATE).")
    print("  A true bridge would require the CA to DERIVE α from")
    print("  the lattice structure alone (geometry → dynamics → α).")
    print("  This remains an open challenge for the FTD framework.")


if __name__ == "__main__":
    main()
