"""
FTD Physics Fundamentals Test Suite
====================================
Tests whether the simulation produces correct emergent physics:

1. Inverse-Square Law (Gravity) — Does F ∝ 1/r²?
2. Inverse-Square Law (Coulomb) — Does F ∝ 1/r²?
3. Energy Conservation — Is total energy conserved over time?
4. Wave Dispersion Relation — Does ω(k) follow theory?
5. Thermodynamic Equilibration — Does velocity distribution thermalize?
6. Structure Formation — Do particles cluster and bind?

Each experiment is self-contained and prints results.
Run: python -m simulations.physics_fundamentals
"""
import sys
import os
import numpy as np
import time

# Add parent to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from ternary_matrix.config import PhysicsConfig, get_test_config
from ternary_matrix.model.grid import Universe
from ternary_matrix.physics import master_equation, forces, waves, integration
from ternary_matrix.physics.forces import gravity_force, coulomb_force


# =============================================================================
# HELPERS
# =============================================================================

def make_universe(size=32, config=None):
    """Create a small test universe."""
    cfg = config or get_test_config(grid_size=size)
    from ternary_matrix.config import get_geometry
    geom = get_geometry(cfg)
    return Universe(size=size, geometry=geom)


def place_particle(universe, pos, state=1, charge=1.0, flux_mag=0.6):
    """Place a single manifested particle at a position."""
    x, y, z = pos
    universe.states[x, y, z] = state
    universe.charge[x, y, z] = charge
    # Give it enough flux to stay manifested (above KB)
    universe.flux[x, y, z] = np.array([flux_mag, 0.0, 0.0], dtype=np.float32)
    universe.density[x, y, z] = flux_mag


def inject_flux_blob(universe, center, radius=3, magnitude=1.0):
    """Inject a Gaussian flux blob centered at a position."""
    cx, cy, cz = center
    N = universe.size
    for x in range(N):
        for y in range(N):
            for z in range(N):
                dx = min(abs(x - cx), N - abs(x - cx))
                dy = min(abs(y - cy), N - abs(y - cy))
                dz = min(abs(z - cz), N - abs(z - cz))
                r = np.sqrt(dx**2 + dy**2 + dz**2)
                if r < radius * 3:
                    amp = magnitude * np.exp(-r**2 / (2 * radius**2))
                    # Radial flux direction
                    if r > 0.1:
                        direction = np.array([dx, dy, dz], dtype=np.float32)
                        direction /= max(np.linalg.norm(direction), 1e-10)
                    else:
                        direction = np.array([1.0, 0.0, 0.0], dtype=np.float32)
                    universe.flux[x, y, z] += amp * direction


def separator(title):
    """Print a section separator."""
    print(f"\n{'='*70}")
    print(f"  {title}")
    print(f"{'='*70}\n")


# =============================================================================
# EXPERIMENT 1: GRAVITY INVERSE-SQUARE LAW
# =============================================================================

def experiment_gravity_inverse_square():
    """
    Test: Does gravity force fall off as 1/r²?

    Method: Place a dense flux blob at center. Measure gravity force
    magnitude at various distances. Fit power law F ∝ r^n.
    Expect n ≈ -2 for 3D inverse-square law.
    """
    separator("EXPERIMENT 1: Gravity Inverse-Square Law")

    size = 64
    u = make_universe(size=size)

    # Create a dense central mass (large flux blob)
    center = (size // 2, size // 2, size // 2)
    inject_flux_blob(u, center, radius=3, magnitude=2.0)
    forces.calculate_density(u)

    # Measure gravity force at various distances along x-axis
    f_grav = gravity_force(u)

    distances = []
    force_magnitudes = []

    for r in range(4, 25):
        x = center[0] + r
        if x >= size:
            break
        y, z = center[1], center[2]
        fx, fy, fz = f_grav[x, y, z]
        f_mag = np.sqrt(fx**2 + fy**2 + fz**2)
        if f_mag > 1e-12:
            distances.append(r)
            force_magnitudes.append(f_mag)

    if len(distances) < 4:
        print("  [!] Not enough data points with measurable force")
        print(f"  Only {len(distances)} points above threshold")
        return None

    # Fit power law: log(F) = n * log(r) + const
    log_r = np.log(distances)
    log_f = np.log(force_magnitudes)

    # Linear regression
    A = np.vstack([log_r, np.ones_like(log_r)]).T
    result = np.linalg.lstsq(A, log_f, rcond=None)
    n, const = result[0]

    print(f"  Distances sampled: {distances[0]} to {distances[-1]} lattice units")
    print(f"  Force range: {force_magnitudes[0]:.2e} to {force_magnitudes[-1]:.2e}")
    print(f"  Fitted power law exponent: n = {n:.3f}")
    print(f"  Expected for 3D inverse-square: n = -2.000")
    print(f"  Deviation: {abs(n - (-2.0)):.3f}")

    if abs(n + 2.0) < 0.5:
        print(f"  RESULT: CONSISTENT with inverse-square law (within 0.5)")
    elif abs(n + 1.0) < 0.5:
        print(f"  RESULT: Closer to 1/r (screened potential)")
    else:
        print(f"  RESULT: Exponent = {n:.2f} (neither 1/r nor 1/r²)")

    return n


# =============================================================================
# EXPERIMENT 2: COULOMB INVERSE-SQUARE LAW
# =============================================================================

def experiment_coulomb_inverse_square():
    """
    Test: Does Coulomb force fall off as 1/r²?

    Method: Place a charged particle at center. Set up charge field.
    Measure Coulomb force at various distances.
    """
    separator("EXPERIMENT 2: Coulomb Inverse-Square Law")

    size = 64
    u = make_universe(size=size)

    center = size // 2

    # Place a +1 charge at center with strong flux
    place_particle(u, (center, center, center), state=1, charge=1.0, flux_mag=1.0)

    # Place test charges at various distances along x-axis
    # We need the charge field to exist for Coulomb calculation
    u.sync_charge_from_state()
    forces.calculate_density(u)

    f_coulomb = coulomb_force(u)

    distances = []
    force_magnitudes = []

    for r in range(2, 20):
        x = center + r
        if x >= size:
            break
        y, z = center, center

        # Place a temporary test charge to measure force there
        # Actually, coulomb_force computes -q * grad(q_smoothed)
        # We need charge at the test point too
        fx, fy, fz = f_coulomb[x, y, z]
        f_mag = np.sqrt(fx**2 + fy**2 + fz**2)
        if f_mag > 1e-15:
            distances.append(r)
            force_magnitudes.append(f_mag)

    if len(distances) < 3:
        print("  [!] Not enough measurable force points")
        print("  Note: Coulomb force requires charge at BOTH points")
        print("  The gradient of a single point charge may be too narrow")
        print(f"  Only {len(distances)} points above threshold")

        # Try an alternative: measure the charge gradient directly
        print("\n  Alternative: Measuring charge gradient magnitude...")
        from ternary_matrix.physics.forces import gradient_3d, smooth_field
        q_smooth = smooth_field(u.charge, universe=u)
        grad_q = gradient_3d(q_smooth, universe=u)
        grad_mag = np.sqrt(np.sum(grad_q**2, axis=-1))

        alt_distances = []
        alt_forces = []
        for r in range(1, 20):
            x = center + r
            if x >= size:
                break
            gm = grad_mag[x, center, center]
            if gm > 1e-15:
                alt_distances.append(r)
                alt_forces.append(gm)

        if len(alt_distances) >= 3:
            log_r = np.log(alt_distances)
            log_f = np.log(alt_forces)
            A = np.vstack([log_r, np.ones_like(log_r)]).T
            result = np.linalg.lstsq(A, log_f, rcond=None)
            n, _ = result[0]
            print(f"  Charge gradient power law: n = {n:.3f}")
            print(f"  Expected: n ≈ -2 (single charge in 3D)")
            return n
        else:
            print("  [!] Still not enough data points")
            return None

    # Fit power law
    log_r = np.log(distances)
    log_f = np.log(force_magnitudes)
    A = np.vstack([log_r, np.ones_like(log_r)]).T
    result = np.linalg.lstsq(A, log_f, rcond=None)
    n, _ = result[0]

    print(f"  Fitted Coulomb exponent: n = {n:.3f}")
    print(f"  Expected: n = -2.000")
    return n


# =============================================================================
# EXPERIMENT 3: ENERGY CONSERVATION
# =============================================================================

def experiment_energy_conservation():
    """
    Test: Is total energy (flux + kinetic) conserved?

    Method: Create a closed system with particles and flux.
    Run for many ticks. Track total energy over time.
    Expect: Energy should be approximately conserved (within damping losses).
    """
    separator("EXPERIMENT 3: Energy Conservation")

    size = 32
    u = make_universe(size=size)

    center = size // 2

    # Set up: two particles with flux, no damping check
    place_particle(u, (center - 3, center, center), state=1, charge=1.0, flux_mag=0.8)
    place_particle(u, (center + 3, center, center), state=-1, charge=-1.0, flux_mag=0.8)
    u.sync_charge_from_state()
    forces.calculate_density(u)

    # Track energy over time
    num_ticks = 200
    energies = []
    tick_numbers = []
    particle_counts = []

    for t in range(num_ticks):
        # Calculate total energy = flux energy + kinetic energy
        forces.calculate_density(u)
        flux_energy = np.sum(u.density)
        ke = integration.get_kinetic_energy(u)
        total_e = flux_energy + ke

        energies.append(total_e)
        tick_numbers.append(t)
        particle_counts.append(u.get_manifested_count())

        master_equation.tick(u)

    energies = np.array(energies)

    # Analyze
    e0 = energies[0]
    e_final = energies[-1]
    e_max = np.max(energies)
    e_min = np.min(energies)

    if e0 > 0:
        relative_change = (e_final - e0) / e0
        relative_range = (e_max - e_min) / e0
    else:
        relative_change = 0
        relative_range = 0

    print(f"  Initial energy: {e0:.4f}")
    print(f"  Final energy:   {e_final:.4f}")
    print(f"  Max energy:     {e_max:.4f}")
    print(f"  Min energy:     {e_min:.4f}")
    print(f"  Relative change: {relative_change:+.4f} ({relative_change*100:+.1f}%)")
    print(f"  Relative range:  {relative_range:.4f} ({relative_range*100:.1f}%)")
    print(f"  Particles at start: {particle_counts[0]}")
    print(f"  Particles at end:   {particle_counts[-1]}")

    # Expected: energy decreases due to DAMPING (0.05) and DECAY_RATE (0.00729)
    # But should not increase or oscillate wildly
    if relative_change < 0 and abs(relative_change) < 1.0:
        print(f"  RESULT: Energy decays monotonically (damping-dominated)")
    elif abs(relative_change) < 0.01:
        print(f"  RESULT: Energy approximately conserved (< 1%)")
    else:
        print(f"  RESULT: Energy change = {relative_change*100:.1f}% (non-trivial)")

    # Check for monotonic decay (expected with damping)
    diffs = np.diff(energies)
    frac_decreasing = np.mean(diffs < 0)
    print(f"  Fraction of ticks with energy decrease: {frac_decreasing:.1%}")

    return energies


# =============================================================================
# EXPERIMENT 4: WAVE DISPERSION RELATION
# =============================================================================

def experiment_wave_dispersion():
    """
    Test: What is the dispersion relation ω(k)?

    Method: Initialize a plane wave with known wavelength λ.
    Measure oscillation frequency. Repeat for multiple wavelengths.
    Theory predicts: ω² = C_WAVE² × (2 - 2cos(k)) for 1D discrete Laplacian.
    """
    separator("EXPERIMENT 4: Wave Dispersion Relation")

    size = 128
    from ternary_matrix.config import CONSTANTS

    wavelengths = [4, 6, 8, 12, 16, 24, 32, 48, 64]
    measured_omega = []
    theoretical_omega = []

    for lam in wavelengths:
        u = make_universe(size=size)

        k = 2 * np.pi / lam
        center = size // 2

        # Initialize a 1D plane wave along x-axis in the flux field
        for x in range(size):
            amplitude = 0.3 * np.sin(k * x)
            u.flux[x, center, center, 0] = amplitude
        forces.calculate_density(u)

        # Measure the flux at a probe point over time
        probe_x = size // 4
        probe_values = []
        num_ticks = max(lam * 4, 100)

        for t in range(num_ticks):
            probe_values.append(u.flux[probe_x, center, center, 0])
            waves.propagate_flux(u)

        probe = np.array(probe_values)

        # Extract frequency via FFT
        if len(probe) > 10:
            # Remove mean
            probe -= np.mean(probe)
            fft = np.fft.rfft(probe)
            power = np.abs(fft)**2
            freqs = np.fft.rfftfreq(len(probe), d=1.0)  # d = 1 tick

            # Find dominant frequency (skip DC)
            if len(power) > 2:
                peak_idx = np.argmax(power[1:]) + 1
                omega_measured = 2 * np.pi * freqs[peak_idx]
            else:
                omega_measured = 0.0
        else:
            omega_measured = 0.0

        # Theoretical: ω² = C_WAVE² × (2 - 2cos(k)) for each spatial dimension
        # In 1D: ω_theory = C_WAVE * sqrt(2 - 2*cos(k))
        c_wave = CONSTANTS.C_WAVE
        damping = CONSTANTS.DAMPING
        omega_theory = c_wave * np.sqrt(2 - 2 * np.cos(k))

        measured_omega.append(omega_measured)
        theoretical_omega.append(omega_theory)

    print(f"  {'lam':>5} | {'k':>8} | {'w_meas':>10} | {'w_theory':>10} | {'ratio':>8}")
    print(f"  {'-'*5}-+-{'-'*8}-+-{'-'*10}-+-{'-'*10}-+-{'-'*8}")

    for i, lam in enumerate(wavelengths):
        k = 2 * np.pi / lam
        om = measured_omega[i]
        ot = theoretical_omega[i]
        ratio = om / ot if ot > 1e-10 else float('inf')
        print(f"  {lam:5d} | {k:8.4f} | {om:10.6f} | {ot:10.6f} | {ratio:8.4f}")

    # Note about damping affecting measurements
    print(f"\n  Note: DAMPING = {CONSTANTS.DAMPING} causes exponential decay")
    print(f"  Note: C_WAVE = {CONSTANTS.C_WAVE}, not C = {CONSTANTS.C}")
    print(f"  Theory: w = C_WAVE * sqrt(2 - 2cos(k)) for 1D discrete Laplacian")

    return measured_omega, theoretical_omega


# =============================================================================
# EXPERIMENT 5: THERMODYNAMIC EQUILIBRATION
# =============================================================================

def experiment_thermalization():
    """
    Test: Do particle velocities thermalize?

    Method: Create many particles with random initial velocities.
    Run for many ticks. Check if velocity distribution approaches
    Maxwell-Boltzmann.
    """
    separator("EXPERIMENT 5: Thermodynamic Equilibration")

    size = 32
    u = make_universe(size=size)

    # Place many particles with random velocities
    n_particles = 0
    np.random.seed(42)
    center = size // 2

    for _ in range(100):
        x = np.random.randint(4, size - 4)
        y = np.random.randint(4, size - 4)
        z = np.random.randint(4, size - 4)

        if u.states[x, y, z] == 0:
            state = np.random.choice([-1, 1])
            place_particle(u, (x, y, z), state=state, charge=float(state), flux_mag=0.8)
            # Random initial velocity
            v = np.random.randn(3).astype(np.float32) * 0.3
            u.velocity[x, y, z] = v
            n_particles += 1

    u.sync_charge_from_state()
    forces.calculate_density(u)

    print(f"  Placed {n_particles} particles")

    # Run simulation and collect velocity statistics
    speed_history = []
    particle_count_history = []

    num_ticks = 500
    sample_interval = 50

    for t in range(num_ticks):
        master_equation.tick(u)

        if t % sample_interval == 0:
            manifested = u.states != 0
            if np.any(manifested):
                speeds = np.sqrt(np.sum(u.velocity[manifested]**2, axis=-1))
                speed_history.append(speeds.copy())
                particle_count_history.append(np.sum(manifested))

    if len(speed_history) < 2:
        print("  [!] Not enough data collected")
        return None

    # Analyze velocity distribution at different times
    print(f"\n  Time evolution of velocity statistics:")
    print(f"  {'Tick':>6} | {'N_part':>7} | {'<v>':>8} | {'σ(v)':>8} | {'v_max':>8}")
    print(f"  {'-'*6}-+-{'-'*7}-+-{'-'*8}-+-{'-'*8}-+-{'-'*8}")

    for i, speeds in enumerate(speed_history):
        tick_num = i * sample_interval
        n_part = particle_count_history[i]
        if len(speeds) > 0:
            mean_v = np.mean(speeds)
            std_v = np.std(speeds)
            max_v = np.max(speeds)
            print(f"  {tick_num:6d} | {n_part:7d} | {mean_v:8.4f} | {std_v:8.4f} | {max_v:8.4f}")

    # Check for Maxwell-Boltzmann signature:
    # In 3D, speed distribution should be P(v) ∝ v² exp(-v²/2σ²)
    # Mean speed should stabilize, std should stabilize
    if len(speed_history) >= 3:
        early_speeds = speed_history[0]
        late_speeds = speed_history[-1]

        if len(early_speeds) > 5 and len(late_speeds) > 5:
            early_mean = np.mean(early_speeds)
            late_mean = np.mean(late_speeds)
            early_std = np.std(early_speeds)
            late_std = np.std(late_speeds)

            print(f"\n  Early mean speed: {early_mean:.4f} ± {early_std:.4f}")
            print(f"  Late mean speed:  {late_mean:.4f} ± {late_std:.4f}")

            # Test: has the distribution changed?
            if late_mean < early_mean * 0.1:
                print(f"  RESULT: Velocities damped to near zero (dissipation-dominated)")
            elif abs(late_mean - early_mean) / max(early_mean, 1e-10) < 0.3:
                print(f"  RESULT: Mean speed roughly stable (quasi-equilibrium)")
            else:
                print(f"  RESULT: Mean speed changed by {(late_mean-early_mean)/early_mean*100:.0f}%")

    return speed_history


# =============================================================================
# EXPERIMENT 6: STRUCTURE FORMATION
# =============================================================================

def experiment_structure_formation():
    """
    Test: Do particles spontaneously form bound structures?

    Method: Create a moderately dense region of particles.
    Run for many ticks. Count triads and clusters over time.
    """
    separator("EXPERIMENT 6: Structure Formation")

    size = 32
    u = make_universe(size=size)

    # Place a cluster of same-sign particles (should try to bind)
    center = size // 2
    placed = 0

    for dx in range(-3, 4):
        for dy in range(-3, 4):
            for dz in range(-3, 4):
                r = np.sqrt(dx**2 + dy**2 + dz**2)
                if r < 3.5 and np.random.random() < 0.3:
                    x, y, z = center + dx, center + dy, center + dz
                    if 0 <= x < size and 0 <= y < size and 0 <= z < size:
                        if u.states[x, y, z] == 0:
                            place_particle(u, (x, y, z), state=1, charge=1.0, flux_mag=0.8)
                            placed += 1

    u.sync_charge_from_state()
    forces.calculate_density(u)

    print(f"  Placed {placed} same-sign particles in a cluster")

    # Track structure metrics
    num_ticks = 300
    sample_interval = 30

    print(f"\n  {'Tick':>6} | {'Particles':>9} | {'Locked':>7} | {'Triads':>7} | {'Bind_E':>8}")
    print(f"  {'-'*6}-+-{'-'*9}-+-{'-'*7}-+-{'-'*7}-+-{'-'*8}")

    for t in range(num_ticks):
        master_equation.tick(u)

        if t % sample_interval == 0:
            from ternary_matrix.physics.binding import get_triad_count, get_binding_energy
            n_part = u.get_manifested_count()
            n_locked = np.count_nonzero(u.is_locked)
            n_triads = get_triad_count(u)
            bind_e = get_binding_energy(u)
            print(f"  {t:6d} | {n_part:9d} | {n_locked:7d} | {n_triads:7d} | {bind_e:8.3f}")

    # Final analysis
    final_particles = u.get_manifested_count()
    final_locked = np.count_nonzero(u.is_locked)
    final_triads = get_triad_count(u)

    print(f"\n  Final state:")
    print(f"  Surviving particles: {final_particles}/{placed}")
    print(f"  Locked (in triads):  {final_locked}")
    print(f"  Triads detected:     {final_triads}")

    if final_triads > 0:
        print(f"  RESULT: Stable structures formed ({final_triads} triads)")
    elif final_locked > 0:
        print(f"  RESULT: Partial binding ({final_locked} locked, no complete triads)")
    elif final_particles > 0:
        print(f"  RESULT: Particles survive but no binding ({final_particles} free)")
    else:
        print(f"  RESULT: All particles decayed/evaporated")

    return final_triads, final_particles


# =============================================================================
# MAIN
# =============================================================================

def main():
    print("=" * 70)
    print("  FTD PHYSICS FUNDAMENTALS TEST SUITE")
    print("  Testing emergent physics from the discrete substrate")
    print("=" * 70)

    start = time.time()

    results = {}

    # Run all experiments
    results['gravity_exponent'] = experiment_gravity_inverse_square()
    results['coulomb_exponent'] = experiment_coulomb_inverse_square()
    results['energies'] = experiment_energy_conservation()
    results['dispersion'] = experiment_wave_dispersion()
    results['thermalization'] = experiment_thermalization()
    results['structure'] = experiment_structure_formation()

    elapsed = time.time() - start

    # Summary
    separator("SUMMARY")
    print(f"  Total runtime: {elapsed:.1f} seconds\n")

    grav_n = results.get('gravity_exponent')
    if grav_n is not None:
        print(f"  Gravity power law:  n = {grav_n:.3f}  (expect -2.0)")
    else:
        print(f"  Gravity power law:  INSUFFICIENT DATA")

    coul_n = results.get('coulomb_exponent')
    if coul_n is not None:
        print(f"  Coulomb power law:  n = {coul_n:.3f}  (expect -2.0)")
    else:
        print(f"  Coulomb power law:  INSUFFICIENT DATA")

    energies = results.get('energies')
    if energies is not None and len(energies) > 1:
        e0 = energies[0]
        ef = energies[-1]
        if e0 > 0:
            print(f"  Energy conservation: {(ef-e0)/e0*100:+.1f}% change over {len(energies)} ticks")
        else:
            print(f"  Energy conservation: zero initial energy")

    disp = results.get('dispersion')
    if disp is not None:
        om, ot = disp
        if len(om) > 0 and any(o > 0 for o in ot):
            ratios = [m/t for m, t in zip(om, ot) if t > 1e-10 and m > 1e-10]
            if ratios:
                avg_ratio = np.mean(ratios)
                print(f"  Dispersion w_meas/w_theory: {avg_ratio:.3f}  (expect ~1.0)")

    struct = results.get('structure')
    if struct is not None:
        triads, particles = struct
        print(f"  Structure formation: {triads} triads, {particles} surviving particles")

    print(f"\n  Note: Damping = 0.05, Decay = 0.00729 cause energy dissipation.")
    print(f"  This is phenomenological, not derived. Energy is NOT strictly conserved.")
    print(f"  The power law tests measure the FORCE FORMULA, not emergent behavior.")


if __name__ == '__main__':
    main()
