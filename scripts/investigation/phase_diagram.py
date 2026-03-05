"""
FTD Phase Diagram: Mapping the CA's Parameter Space
=====================================================

Systematically sweep DECAY_RATE × G_STRONG to find the critical boundary
between the "vacuum" phase (all matter decays) and the "condensed" phase
(bound structures persist).

This is the foundational experiment: it tells us WHERE the interesting
physics lives in parameter space, and provides the critical point for
subsequent RG flow analysis.

Author: AI-assisted research (February 2026)
Framework: FTD v5.24
"""

import sys
import os
import time
import numpy as np
from dataclasses import replace

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
sys.path.insert(0, PROJECT_ROOT)

from ternary_matrix.config import PhysicsConfig, get_geometry
from ternary_matrix.model.grid import Universe
from ternary_matrix.physics.master_equation import tick, ensure_initial_bindings, get_diagnostics
from ternary_matrix.physics import forces
from ternary_matrix.physics.binding import get_triad_count
from ternary_matrix.analysis.structure_metrics import analyze_clusters





# =============================================================================
# SINGLE SIMULATION RUN
# =============================================================================

def run_single(decay_rate, g_strong, grid_size=24, num_ticks=300,
               seed_density=0.10, seed=42):
    """
    Run one simulation with specified parameters and return observables.

    Returns dict with:
    - survival_fraction: manifested/initial at final tick
    - triad_density: triads / grid_volume
    - mean_cluster_size: average cluster size (0 if none)
    - max_cluster_size: largest cluster
    - final_manifested: count at end
    - half_life_tick: tick at which manifested drops below 50% of initial
    """
    np.random.seed(seed)

    # Create config with modified parameters
    config = PhysicsConfig(
        GRID_SIZE=grid_size,
        DECAY_RATE=decay_rate,
        ALPHA=decay_rate,  # ALPHA must equal DECAY_RATE per ASSUMP.6
        G_STRONG=g_strong,
    )
    geometry = get_geometry(config)
    universe = Universe(size=grid_size, geometry=geometry)

    # Seed the lattice with PHYSICALLY VALID initial conditions
    # KEY: manifested particles must have density > KB to survive Phase 3
    n_voxels = grid_size ** 3
    n_seed = int(seed_density * n_voxels)
    positions = np.random.choice(n_voxels, size=n_seed, replace=False)
    coords = np.unravel_index(positions, (grid_size, grid_size, grid_size))

    half = n_seed // 2
    universe.states[coords[0][:half], coords[1][:half], coords[2][:half]] = 1
    universe.states[coords[0][half:], coords[1][half:], coords[2][half:]] = -1
    universe.sync_charge_from_state()

    # Set flux at manifested sites to magnitude well above KB
    # Use random directions but controlled magnitude (2*KB)
    flux_magnitude = 2.0 * config.KB  # safely above manifestation threshold
    for idx in range(n_seed):
        x, y, z = coords[0][idx], coords[1][idx], coords[2][idx]
        direction = np.random.randn(3).astype(np.float32)
        direction /= np.linalg.norm(direction) + 1e-8
        universe.flux[x, y, z] = direction * flux_magnitude

    # Add gentle background flux (well below KB so no spontaneous genesis)
    background = np.random.normal(0, 0.05, universe.flux.shape).astype(np.float32)
    empty_mask = universe.states == 0
    universe.flux[empty_mask] = background[empty_mask]

    # CRITICAL: bootstrap binding so is_locked is active from tick 1
    ensure_initial_bindings(universe)

    initial_count = universe.get_manifested_count()
    half_life_tick = num_ticks  # default if never drops below 50%

    # Run simulation, sampling periodically
    sample_ticks = set(range(0, num_ticks, max(1, num_ticks // 20)))
    sample_ticks.add(num_ticks - 1)
    manifested_history = []

    for t in range(num_ticks):
        tick(universe)

        if t in sample_ticks:
            count = universe.get_manifested_count()
            manifested_history.append((t, count))

            if count < initial_count * 0.5 and half_life_tick == num_ticks:
                half_life_tick = t

    # Final measurements
    final_count = universe.get_manifested_count()
    n_triads = get_triad_count(universe)
    clusters = analyze_clusters(universe)

    # Cluster statistics
    if clusters:
        sizes = []
        for size, count in clusters.items():
            sizes.extend([size] * count)
        mean_cluster = np.mean(sizes) if sizes else 0
        max_cluster = max(clusters.keys())
    else:
        mean_cluster = 0
        max_cluster = 0

    return {
        'survival_fraction': final_count / max(1, initial_count),
        'triad_density': n_triads / n_voxels,
        'mean_cluster_size': mean_cluster,
        'max_cluster_size': max_cluster,
        'final_manifested': final_count,
        'initial_manifested': initial_count,
        'half_life_tick': half_life_tick,
        'n_triads': n_triads,
    }


# =============================================================================
# PHASE DIAGRAM SWEEP
# =============================================================================

def run_phase_diagram(n_decay=15, n_strong=15, grid_size=24, num_ticks=500):
    """
    Run a 2D sweep over (DECAY_RATE, G_STRONG) and collect order parameters.
    """
    # Parameter ranges
    # DECAY_RATE: from very slow decay (0.001) to aggressive (0.1)
    # G_STRONG: from weak binding (0.1) to very strong (5.0)
    decay_rates = np.logspace(-3, -1, n_decay)   # 0.001 to 0.1
    g_strongs = np.logspace(-1, np.log10(5), n_strong)  # 0.1 to 5.0

    # Theory value markers
    theory_decay = 0.00729
    theory_strong = 1.0

    total_runs = n_decay * n_strong
    print(f"Phase diagram: {n_decay} × {n_strong} = {total_runs} runs")
    print(f"Grid: {grid_size}³, {num_ticks} ticks each")
    print(f"DECAY_RATE range: [{decay_rates[0]:.4f}, {decay_rates[-1]:.4f}]")
    print(f"G_STRONG range:   [{g_strongs[0]:.4f}, {g_strongs[-1]:.4f}]")
    print(f"Theory point:     DECAY_RATE={theory_decay}, G_STRONG={theory_strong}")
    print()

    # Result arrays
    survival = np.zeros((n_decay, n_strong))
    triad_den = np.zeros((n_decay, n_strong))
    mean_cluster = np.zeros((n_decay, n_strong))
    half_life = np.zeros((n_decay, n_strong))

    t_start = time.time()

    for i, dr in enumerate(decay_rates):
        for j, gs in enumerate(g_strongs):
            run_idx = i * n_strong + j + 1
            result = run_single(dr, gs, grid_size=grid_size, num_ticks=num_ticks)

            survival[i, j] = result['survival_fraction']
            triad_den[i, j] = result['triad_density']
            mean_cluster[i, j] = result['mean_cluster_size']
            half_life[i, j] = result['half_life_tick']

            if run_idx % 10 == 0 or run_idx == total_runs:
                elapsed = time.time() - t_start
                rate = run_idx / elapsed
                eta = (total_runs - run_idx) / rate if rate > 0 else 0
                print(f"  [{run_idx:3d}/{total_runs}] "
                      f"DR={dr:.4f} GS={gs:.2f} → "
                      f"surv={survival[i,j]:.3f} triads={result['n_triads']} "
                      f"clusters={result['mean_cluster_size']:.1f} "
                      f"({elapsed:.0f}s elapsed, ~{eta:.0f}s remaining)")

    total_time = time.time() - t_start
    print(f"\nSweep complete in {total_time:.1f}s ({total_time/total_runs:.2f}s per run)")

    return {
        'decay_rates': decay_rates,
        'g_strongs': g_strongs,
        'survival': survival,
        'triad_density': triad_den,
        'mean_cluster': mean_cluster,
        'half_life': half_life,
    }


# =============================================================================
# TEXT-BASED VISUALIZATION
# =============================================================================

def print_heatmap(data, row_labels, col_labels, title, fmt=".2f"):
    """Print a 2D array as a text heatmap."""
    n_rows, n_cols = data.shape

    # Use symbols for intensity levels
    levels = " ░▒▓█"
    vmin, vmax = np.min(data), np.max(data)

    print(f"\n{'=' * 70}")
    print(f"  {title}")
    print(f"  (rows: DECAY_RATE ↓, cols: G_STRONG →)")
    print(f"  Value range: [{vmin:{fmt}}, {vmax:{fmt}}]")
    print(f"{'=' * 70}")

    # Header
    header = "         "
    for j in range(0, n_cols, max(1, n_cols // 8)):
        header += f"{col_labels[j]:>7.3f}"
    print(header)

    for i in range(n_rows):
        row_str = f"  {row_labels[i]:6.4f} │"
        for j in range(n_cols):
            if vmax > vmin:
                intensity = (data[i, j] - vmin) / (vmax - vmin)
            else:
                intensity = 0
            level_idx = min(len(levels) - 1, int(intensity * (len(levels) - 1)))
            row_str += levels[level_idx]
        # Append the last value
        row_str += f"│ {data[i, -1]:{fmt}}"
        print(row_str)


def analyze_phases(results):
    """Identify phase boundaries from the sweep data."""
    survival = results['survival']
    triad_den = results['triad_density']
    decay_rates = results['decay_rates']
    g_strongs = results['g_strongs']

    print("\n" + "=" * 70)
    print("  PHASE ANALYSIS")
    print("=" * 70)

    # Phase classification thresholds
    VACUUM_THRESHOLD = 0.01     # survival < 1% → vacuum
    CONDENSED_THRESHOLD = 0.001 # triad_density > 0.1% → condensed

    n_vacuum = np.sum(survival < VACUUM_THRESHOLD)
    n_gas = np.sum((survival >= VACUUM_THRESHOLD) & (triad_den < CONDENSED_THRESHOLD))
    n_condensed = np.sum((survival >= VACUUM_THRESHOLD) & (triad_den >= CONDENSED_THRESHOLD))

    total = survival.size
    print(f"\n  Phase counts (out of {total} runs):")
    print(f"    VACUUM    (surv < 1%):           {n_vacuum:4d} ({100*n_vacuum/total:.1f}%)")
    print(f"    GAS       (surv ≥ 1%, no triads):{n_gas:4d} ({100*n_gas/total:.1f}%)")
    print(f"    CONDENSED (surv ≥ 1%, triads):   {n_condensed:4d} ({100*n_condensed/total:.1f}%)")

    # Find the critical boundary
    # For each G_STRONG value, find the DECAY_RATE where survival transitions
    print(f"\n  Critical boundary (survival = 1% contour):")
    print(f"  {'G_STRONG':>10} | {'Critical DECAY_RATE':>20} | {'Phase at theory DR'}")
    print(f"  {'-'*10}-+-{'-'*20}-+-{'-'*20}")

    critical_points = []
    for j, gs in enumerate(g_strongs):
        col = survival[:, j]
        # Find where survival crosses VACUUM_THRESHOLD
        above = np.where(col >= VACUUM_THRESHOLD)[0]
        if len(above) == 0:
            critical_dr = decay_rates[0]  # All vacuum
            status = "always vacuum"
        elif len(above) == len(col):
            critical_dr = decay_rates[-1]  # Never vacuum
            status = "always alive"
        else:
            # Linear interpolation between last alive and first dead
            idx = above[-1]
            if idx < len(decay_rates) - 1:
                # Interpolate
                s0, s1 = col[idx], col[idx + 1]
                d0, d1 = np.log10(decay_rates[idx]), np.log10(decay_rates[idx + 1])
                if s0 != s1:
                    frac = (VACUUM_THRESHOLD - s0) / (s1 - s0)
                    critical_dr = 10 ** (d0 + frac * (d1 - d0))
                else:
                    critical_dr = decay_rates[idx]
            else:
                critical_dr = decay_rates[idx]
            status = f"transition at {critical_dr:.5f}"

        # Check theory point
        theory_survival = survival[np.argmin(np.abs(decay_rates - 0.00729)), j]
        phase_at_theory = "VACUUM" if theory_survival < VACUUM_THRESHOLD else "ALIVE"

        critical_points.append((gs, critical_dr))
        print(f"  {gs:10.4f} | {critical_dr:20.6f} | {phase_at_theory}")

    # Is the theory point in vacuum or alive?
    theory_i = np.argmin(np.abs(decay_rates - 0.00729))
    theory_j = np.argmin(np.abs(g_strongs - 1.0))
    theory_surv = survival[theory_i, theory_j]
    theory_triads = triad_den[theory_i, theory_j]

    print(f"\n  ┌─────────────────────────────────────────────────────┐")
    print(f"  │  THEORY POINT: DR=0.00729, GS=1.0                  │")
    print(f"  │  Survival fraction: {theory_surv:.4f}                       │")
    print(f"  │  Triad density:     {theory_triads:.6f}                   │")
    phase = "VACUUM" if theory_surv < VACUUM_THRESHOLD else "GAS" if theory_triads < CONDENSED_THRESHOLD else "CONDENSED"
    print(f"  │  Phase:             {phase:10s}                       │")
    print(f"  └─────────────────────────────────────────────────────┘")

    return critical_points


# =============================================================================
# DEEPER ANALYSIS: 1D SLICES
# =============================================================================

def run_1d_slice(fixed_param, fixed_value, sweep_param, sweep_range,
                 n_points=20, grid_size=24, num_ticks=500):
    """
    Run a 1D parameter sweep holding one parameter fixed.
    Used for high-resolution analysis near the critical point.
    """
    print(f"\n{'=' * 70}")
    print(f"  1D SLICE: {fixed_param}={fixed_value:.4f}, sweeping {sweep_param}")
    print(f"  {n_points} points in [{sweep_range[0]:.4f}, {sweep_range[-1]:.4f}]")
    print(f"{'=' * 70}")

    results = []
    for val in sweep_range:
        if sweep_param == 'DECAY_RATE':
            r = run_single(val, fixed_value, grid_size=grid_size, num_ticks=num_ticks)
        else:
            r = run_single(fixed_value, val, grid_size=grid_size, num_ticks=num_ticks)
        r['param_value'] = val
        results.append(r)
        print(f"    {sweep_param}={val:.5f}: surv={r['survival_fraction']:.4f} "
              f"triads={r['n_triads']} hl={r['half_life_tick']}")

    return results


# =============================================================================
# MAIN
# =============================================================================

def main():
    print("╔══════════════════════════════════════════════════════════════════════╗")
    print("║        FTD PHASE DIAGRAM: MAPPING THE CA'S PARAMETER SPACE         ║")
    print("║        Where does structure survive? Where does vacuum win?         ║")
    print("╚══════════════════════════════════════════════════════════════════════╝")
    print()

    # =========================================================================
    # PHASE 1: Coarse sweep (15×15)
    # =========================================================================
    print("PHASE 1: COARSE PARAMETER SWEEP")
    print("-" * 40)
    results = run_phase_diagram(n_decay=15, n_strong=15, grid_size=24, num_ticks=500)

    # Print heatmaps
    print_heatmap(results['survival'], results['decay_rates'], results['g_strongs'],
                  "SURVIVAL FRACTION (manifested/initial at t=500)")

    print_heatmap(results['triad_density'] * 1000, results['decay_rates'], results['g_strongs'],
                  "TRIAD DENSITY × 1000")

    print_heatmap(results['half_life'], results['decay_rates'], results['g_strongs'],
                  "HALF-LIFE (tick where 50% decayed)", fmt=".0f")

    # Phase analysis
    critical_points = analyze_phases(results)

    # =========================================================================
    # PHASE 2: High-resolution 1D slices near critical boundary
    # =========================================================================
    print("\n\nPHASE 2: HIGH-RESOLUTION SLICES")
    print("-" * 40)

    # Slice at G_STRONG = 1.0 (theory value), sweep DECAY_RATE finely
    decay_fine = np.logspace(-3, -0.5, 25)
    slice_gs1 = run_1d_slice('G_STRONG', 1.0, 'DECAY_RATE', decay_fine,
                             grid_size=24, num_ticks=500)

    # Slice at DECAY_RATE = 0.00729 (theory value), sweep G_STRONG finely
    gs_fine = np.logspace(-0.5, np.log10(10), 25)
    slice_dr_theory = run_1d_slice('DECAY_RATE', 0.00729, 'G_STRONG', gs_fine,
                                   grid_size=24, num_ticks=500)

    # =========================================================================
    # SYNTHESIS
    # =========================================================================
    print("\n" + "=" * 70)
    print("  SYNTHESIS: PHASE STRUCTURE OF THE FTD CA")
    print("=" * 70)

    # Find maximum survival values
    max_surv_idx = np.unravel_index(np.argmax(results['survival']), results['survival'].shape)
    max_surv = results['survival'][max_surv_idx]
    max_surv_dr = results['decay_rates'][max_surv_idx[0]]
    max_surv_gs = results['g_strongs'][max_surv_idx[1]]

    print(f"""
  RESULTS:
  ─────────
  Maximum survival: {max_surv:.4f} at DR={max_surv_dr:.4f}, GS={max_surv_gs:.2f}

  Phase structure:
  - VACUUM phase dominates when DECAY_RATE >> G_STRONG effects
  - The critical boundary separates regimes where binding
    can overcome decay
  - The theory point (DR=0.00729, GS=1.0) sits in the
    {'VACUUM' if results['survival'][np.argmin(np.abs(results['decay_rates'] - 0.00729)), np.argmin(np.abs(results['g_strongs'] - 1.0))] < 0.01 else 'ALIVE'} phase

  IMPLICATIONS FOR RG FLOW (Part D):
  - Need to run the critical-boundary parameters across scales
  - The critical point is where universal behavior emerges
  - If vacuum phase dominates everywhere → the CA has no
    thermodynamic limit (bad for physics)
  - If condensed phase exists → can extract meaningful
    continuum-limit observables
""")

    # Save raw data for Part D
    output_dir = os.path.join(PROJECT_ROOT, 'scripts', 'investigation', 'data')
    os.makedirs(output_dir, exist_ok=True)

    np.savez(os.path.join(output_dir, 'phase_diagram.npz'),
             decay_rates=results['decay_rates'],
             g_strongs=results['g_strongs'],
             survival=results['survival'],
             triad_density=results['triad_density'],
             mean_cluster=results['mean_cluster'],
             half_life=results['half_life'])

    print(f"  Raw data saved to: {output_dir}/phase_diagram.npz")
    print(f"  (Use this for Part D: RG Flow analysis)")


if __name__ == "__main__":
    main()
