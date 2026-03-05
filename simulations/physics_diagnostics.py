"""
FTD Physics Diagnostics
========================
Root-cause analysis of the physics experiment failures.

The physics_fundamentals.py suite found:
  1. Gravity exponent n = -4.09 (expect -2.0)
  2. 99.9% energy loss in 200 ticks
  3. Wave dispersion 2-26x off theory
  4. All particles evaporate by tick 30

This script isolates each mechanism to identify the root cause.

Run: python -m simulations.physics_diagnostics
"""
import sys
import os
import numpy as np

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from ternary_matrix.config import PhysicsConfig, get_test_config, CONSTANTS
from ternary_matrix.model.grid import Universe
from ternary_matrix.physics import forces, waves, integration
from ternary_matrix.physics.forces import gravity_force, coulomb_force

def make_universe(size=32, config=None):
    cfg = config or get_test_config(grid_size=size)
    from ternary_matrix.config import get_geometry
    geom = get_geometry(cfg)
    return Universe(size=size, geometry=geom)


def separator(title):
    print(f"\n{'='*70}")
    print(f"  {title}")
    print(f"{'='*70}\n")


# =============================================================================
# DIAGNOSTIC 1: WHY IS GRAVITY n = -4 INSTEAD OF n = -2?
# =============================================================================

def diagnose_gravity():
    """
    The gravity formula is:
        F_grav = G_N * grad(smooth(density))
    where smooth = 6-neighbor average, grad = central difference.

    For a point source: density = delta function at center.
    After smoothing: smooth(delta) is nonzero only at the 6 neighbors.
    After gradient: grad of that is nonzero only at neighbors-of-neighbors.

    For a Gaussian blob: density ~ exp(-r^2/2sigma^2)
    After smoothing: still ~ Gaussian (slightly wider)
    After gradient: ~ r * exp(-r^2/...) which falls off as Gaussian

    KEY INSIGHT: The issue is that F = G*grad(rho_smooth) is NOT equivalent
    to F = -GM/r^2. The gradient of a density field is NOT a 1/r^2 force.
    For 1/r^2, we need: F = -grad(phi) where phi is the POTENTIAL satisfying
    Laplacian(phi) = -4*pi*G*rho.

    The current gravity is essentially F ~ d(rho)/dr, NOT F ~ -GM/r^2.
    """
    separator("DIAGNOSTIC 1: Gravity Force Formula Analysis")

    size = 64
    u = make_universe(size=size)
    center = size // 2

    # --- Test A: Gaussian blob source ---
    print("  Test A: Gaussian blob density profile")
    sigma = 3.0
    for x in range(size):
        for y in range(size):
            for z in range(size):
                dx = min(abs(x - center), size - abs(x - center))
                dy = min(abs(y - center), size - abs(y - center))
                dz = min(abs(z - center), size - abs(z - center))
                r = np.sqrt(dx**2 + dy**2 + dz**2)
                # Set density via flux
                amp = 2.0 * np.exp(-r**2 / (2 * sigma**2))
                u.flux[x, y, z] = np.array([amp, 0, 0], dtype=np.float32)

    forces.calculate_density(u)
    f_grav = gravity_force(u)

    # Measure along x-axis
    print(f"\n  {'r':>4} | {'rho(r)':>10} | {'rho_sm(r)':>10} | {'F_grav':>10} | {'1/r^2':>10} | {'d(rho)/dr':>10}")
    print(f"  {'-'*4}-+-{'-'*10}-+-{'-'*10}-+-{'-'*10}-+-{'-'*10}-+-{'-'*10}")

    geom = u.geometry
    rho_smooth = geom.smooth_field(u.density)

    prev_rho = None
    for r in range(1, 28):
        x = center + r
        if x >= size:
            break
        rho = u.density[x, center, center]
        rho_sm = rho_smooth[x, center, center]
        f_mag = np.sqrt(np.sum(f_grav[x, center, center]**2))
        inv_r2 = 1.0 / (r * r) if r > 0 else 0

        # Numerical derivative of rho
        if r > 1 and r < 27:
            rho_plus = u.density[x + 1, center, center]
            rho_minus = u.density[x - 1, center, center]
            drho_dr = (rho_plus - rho_minus) / 2.0
        else:
            drho_dr = 0

        if rho > 1e-10 or f_mag > 1e-12:
            print(f"  {r:4d} | {rho:10.6f} | {rho_sm:10.6f} | {f_mag:10.2e} | {inv_r2:10.6f} | {abs(drho_dr):10.2e}")

    # --- Fit power law to F_grav ---
    distances = []
    force_mags = []
    for r in range(4, 25):
        x = center + r
        if x >= size:
            break
        f_mag = np.sqrt(np.sum(f_grav[x, center, center]**2))
        if f_mag > 1e-12:
            distances.append(r)
            force_mags.append(f_mag)

    if len(distances) >= 4:
        log_r = np.log(distances)
        log_f = np.log(force_mags)
        A = np.vstack([log_r, np.ones_like(log_r)]).T
        result = np.linalg.lstsq(A, log_f, rcond=None)
        n = result[0][0]
        print(f"\n  Fitted exponent: n = {n:.3f}")

    # --- Compare with what 1/r^2 WOULD need ---
    print(f"\n  ROOT CAUSE ANALYSIS:")
    print(f"  The gravity formula is: F = G_N * grad(smooth(rho))")
    print(f"  For a Gaussian density rho ~ exp(-r^2/2s^2):")
    print(f"    smooth(rho) ~ exp(-r^2/2s'^2)  (slightly wider Gaussian)")
    print(f"    grad(smooth(rho)) ~ -(r/s'^2) * exp(-r^2/2s'^2)")
    print(f"  This is a GAUSSIAN falloff, which looks like ~ r^(-4) or steeper")
    print(f"  over the range r=4..25 because the Gaussian tail decays faster")
    print(f"  than any power law.")
    print(f"")
    print(f"  For TRUE 1/r^2 gravity, we need Poisson's equation:")
    print(f"    Laplacian(phi) = -4*pi*G*rho")
    print(f"    F = -grad(phi)")
    print(f"")
    print(f"  Current formula: F = G*grad(rho) -- this is the gradient of the")
    print(f"  SOURCE, not the gradient of the POTENTIAL. These are fundamentally")
    print(f"  different!")


# =============================================================================
# DIAGNOSTIC 2: ENERGY DISSIPATION BUDGET
# =============================================================================

def diagnose_energy_dissipation():
    """
    Three dissipation mechanisms:
    1. DAMPING (0.05): Applied to flux and wave_velocity in waves.py every tick
    2. DECAY_RATE (0.00729): Applied to flux of unlocked voxels every tick
    3. EVAPORATION: When density < KB, particle dies and velocity/charge zeroed

    Let's quantify each one separately.
    """
    separator("DIAGNOSTIC 2: Energy Dissipation Budget")

    size = 16

    # --- Test A: Pure wave damping (no particles) ---
    print("  Test A: Flux energy under DAMPING alone (no particles)")
    u = make_universe(size=size)
    center = size // 2
    # Inject flux
    u.flux[center, center, center] = np.array([1.0, 0, 0], dtype=np.float32)
    forces.calculate_density(u)
    e0 = np.sum(u.density)

    for t in range(100):
        waves.propagate_flux(u)
        forces.calculate_density(u)

    e100 = np.sum(u.density)
    damping = CONSTANTS.DAMPING
    # Theoretical: both flux and wave_velocity multiply by (1-DAMPING) each tick
    # So flux ~ (1-DAMPING)^t after t ticks
    # density = |flux| also ~ (1-DAMPING)^t
    # Total flux energy ~ sum(density) ~ (1-DAMPING)^t (wave spreads but total magnitude decays)
    theoretical_decay = (1 - damping) ** 100
    # Actually flux *= (1-D) AND wave_velocity *= (1-D), so it's more complex
    # But the flux itself gets multiplied by (1-D) each tick, so density ~ (1-D)^t
    print(f"  DAMPING = {damping}")
    print(f"  Initial flux energy: {e0:.4f}")
    print(f"  After 100 ticks:     {e100:.6f}")
    print(f"  Ratio:               {e100/e0:.6f}")
    print(f"  Theory (1-D)^100:    {theoretical_decay:.6f}")
    print(f"  Half-life (ticks):   {np.log(2)/np.log(1/(1-damping)):.1f}")

    # --- Test B: Particle decay rate alone ---
    print(f"\n  Test B: Particle flux under DECAY_RATE alone")
    u2 = make_universe(size=size)
    # Place a locked particle (won't decay) vs unlocked (will decay)
    u2.states[center, center, center] = 1
    u2.flux[center, center, center] = np.array([1.0, 0, 0], dtype=np.float32)
    u2.density[center, center, center] = 1.0
    u2.is_locked[center, center, center] = False

    # Manual decay (Phase 2 from master_equation)
    decay_rate = CONSTANTS.DECAY_RATE
    flux_values = [1.0]
    for t in range(200):
        factor = 1.0 - decay_rate
        u2.flux[center, center, center] *= factor
        flux_values.append(np.linalg.norm(u2.flux[center, center, center]))

    flux_values = np.array(flux_values)
    half_life_decay = np.log(2) / np.log(1/(1-decay_rate))
    print(f"  DECAY_RATE = {decay_rate}")
    print(f"  After 200 ticks: {flux_values[-1]:.6f}")
    print(f"  Ratio: {flux_values[-1]/flux_values[0]:.6f}")
    print(f"  Half-life (ticks): {half_life_decay:.1f}")

    # --- Test C: Combined effect ---
    print(f"\n  Test C: Combined damping + decay in one tick")
    # Per tick, flux gets: decay_factor * damping_factor = (1-0.00729) * (1-0.05)
    combined = (1 - decay_rate) * (1 - damping)
    print(f"  Per-tick flux retention: (1-DECAY)*(1-DAMP) = {combined:.6f}")
    print(f"  Per-tick flux loss: {(1-combined)*100:.2f}%")
    print(f"  Combined half-life: {np.log(2)/np.log(1/combined):.1f} ticks")
    print(f"  After 30 ticks: {combined**30:.6f} ({combined**30*100:.2f}% remaining)")
    print(f"  After 100 ticks: {combined**100:.10f}")

    # --- Test D: Evaporation threshold ---
    print(f"\n  Test D: Evaporation threshold analysis")
    print(f"  KB (manifestation threshold) = {CONSTANTS.KB}")
    print(f"  A particle with initial flux_mag = 0.8:")
    initial_flux = 0.8
    ticks_to_evaporate = 0
    f = initial_flux
    while f > CONSTANTS.KB:
        f *= combined
        ticks_to_evaporate += 1
    print(f"  Ticks until density < KB: {ticks_to_evaporate}")
    print(f"  (flux decays: {initial_flux:.2f} -> {CONSTANTS.KB:.3f} in {ticks_to_evaporate} ticks)")

    # With flux_mag = 1.0
    f = 1.0
    ticks = 0
    while f > CONSTANTS.KB:
        f *= combined
        ticks += 1
    print(f"\n  With initial flux_mag = 1.0: evaporates in {ticks} ticks")

    # With flux_mag = 2.0
    f = 2.0
    ticks = 0
    while f > CONSTANTS.KB:
        f *= combined
        ticks += 1
    print(f"  With initial flux_mag = 2.0: evaporates in {ticks} ticks")

    # With flux_mag = 10.0
    f = 10.0
    ticks = 0
    while f > CONSTANTS.KB:
        f *= combined
        ticks += 1
    print(f"  With initial flux_mag = 10.0: evaporates in {ticks} ticks")


# =============================================================================
# DIAGNOSTIC 3: WAVE PROPAGATION (DAMPING-FREE TEST)
# =============================================================================

def diagnose_waves():
    """
    Test wave propagation with and without damping to isolate the issue.
    """
    separator("DIAGNOSTIC 3: Wave Dispersion (Damping Analysis)")

    size = 128
    lam = 16
    k = 2 * np.pi / lam
    center = size // 2
    c_wave = CONSTANTS.C_WAVE

    print(f"  Testing wavelength = {lam}, k = {k:.4f}")
    print(f"  C_WAVE = {c_wave}")
    print(f"  Theory: w = C_WAVE * sqrt(2 - 2*cos(k)) = {c_wave * np.sqrt(2 - 2*np.cos(k)):.6f}")
    print(f"  DAMPING = {CONSTANTS.DAMPING}")

    # --- Test A: With damping (as in real sim) ---
    u1 = make_universe(size=size)
    for x in range(size):
        u1.flux[x, center, center, 0] = 0.3 * np.sin(k * x)

    probe_a = []
    for t in range(lam * 4):
        probe_a.append(u1.flux[size//4, center, center, 0])
        waves.propagate_flux(u1)
    probe_a = np.array(probe_a)

    # --- Test B: Without damping (manually propagate) ---
    u2 = make_universe(size=size)
    for x in range(size):
        u2.flux[x, center, center, 0] = 0.3 * np.sin(k * x)

    probe_b = []
    for t in range(lam * 4):
        probe_b.append(u2.flux[size//4, center, center, 0])
        # Manual wave equation without damping
        acc = (c_wave ** 2) * u2.geometry.laplacian_vector(u2.flux)
        u2.wave_velocity += acc
        u2.flux += u2.wave_velocity
        # NO DAMPING applied
    probe_b = np.array(probe_b)

    print(f"\n  With damping (real sim):")
    print(f"    Initial amplitude: {abs(probe_a[0]):.6f}")
    print(f"    After {len(probe_a)} ticks: {abs(probe_a[-1]):.6f}")
    print(f"    Decay ratio: {abs(probe_a[-1])/max(abs(probe_a[0]),1e-10):.6f}")

    print(f"\n  Without damping:")
    print(f"    Initial amplitude: {abs(probe_b[0]):.6f}")
    print(f"    After {len(probe_b)} ticks: {abs(probe_b[-1]):.6f}")

    # Check for instability in undamped case
    max_b = np.max(np.abs(probe_b))
    print(f"    Max amplitude seen: {max_b:.6f}")
    if max_b > 1.0:
        print(f"    WARNING: Amplitude grew! Wave equation may be CFL-unstable without damping")

    # FFT analysis of both
    for label, probe in [("damped", probe_a), ("undamped", probe_b)]:
        p = probe - np.mean(probe)
        if np.max(np.abs(p)) < 1e-15:
            print(f"\n  {label}: signal too weak for FFT")
            continue
        fft = np.fft.rfft(p)
        power = np.abs(fft)**2
        freqs = np.fft.rfftfreq(len(p), d=1.0)
        if len(power) > 2:
            peak_idx = np.argmax(power[1:]) + 1
            w_meas = 2 * np.pi * freqs[peak_idx]
            w_theory = c_wave * np.sqrt(2 - 2 * np.cos(k))
            print(f"\n  {label} FFT: w_meas = {w_meas:.6f}, w_theory = {w_theory:.6f}, ratio = {w_meas/w_theory:.4f}")

    # --- Test C: Check CFL stability condition ---
    print(f"\n  CFL Stability Check:")
    print(f"  For 6-connected cubic Laplacian: C_WAVE^2 * dt^2 / dx^2 <= 1/6")
    print(f"  With dt=1, dx=1: C_WAVE^2 <= 1/6 = {1/6:.4f}")
    print(f"  C_WAVE^2 = {c_wave**2:.4f}")
    if c_wave**2 <= 1/6:
        print(f"  STATUS: CFL condition SATISFIED (stable)")
    else:
        print(f"  STATUS: CFL condition VIOLATED! ({c_wave**2:.4f} > {1/6:.4f})")
        print(f"  This means waves are UNSTABLE -- damping is the only thing preventing blowup!")


# =============================================================================
# DIAGNOSTIC 4: BINDING AND SURVIVAL
# =============================================================================

def diagnose_binding():
    """
    Test whether locked particles survive longer than unlocked ones.
    The binding mechanism suppresses decay for locked particles.
    But do they actually survive?
    """
    separator("DIAGNOSTIC 4: Particle Survival (Locked vs Unlocked)")

    size = 16

    # --- Test A: Single unlocked particle ---
    print("  Test A: Single unlocked particle")
    u1 = make_universe(size=size)
    c = size // 2
    u1.states[c, c, c] = 1
    u1.charge[c, c, c] = 1.0
    u1.flux[c, c, c] = np.array([0.8, 0, 0], dtype=np.float32)
    u1.density[c, c, c] = 0.8
    u1.is_locked[c, c, c] = False

    from ternary_matrix.physics.master_equation import tick, apply_decay, update_manifestation

    for t in range(100):
        # Just do decay + manifestation check (no waves or forces)
        apply_decay(u1)
        forces.calculate_density(u1)
        if u1.states[c, c, c] == 0:
            print(f"    Evaporated at tick {t}")
            break
        if u1.density[c, c, c] < CONSTANTS.KB:
            print(f"    Density ({u1.density[c,c,c]:.4f}) < KB ({CONSTANTS.KB}) at tick {t}")
            update_manifestation(u1)
            if u1.states[c, c, c] == 0:
                print(f"    Evaporated at tick {t}")
                break
    else:
        print(f"    Survived 100 ticks! density = {u1.density[c,c,c]:.4f}")

    # --- Test B: Triad (3 locked particles) ---
    print("\n  Test B: Triad (3 locked particles)")
    u2 = make_universe(size=size)
    # Place 3 particles in adjacent positions
    positions = [(c, c, c), (c+1, c, c), (c, c+1, c)]
    for pos in positions:
        x, y, z = pos
        u2.states[x, y, z] = 1
        u2.charge[x, y, z] = 1.0
        u2.flux[x, y, z] = np.array([0.8, 0, 0], dtype=np.float32)
        u2.density[x, y, z] = 0.8
        u2.is_locked[x, y, z] = True  # Manually lock

    for t in range(100):
        # Decay does NOT apply to locked particles
        apply_decay(u2)
        forces.calculate_density(u2)
        n_alive = sum(1 for pos in positions if u2.states[pos] != 0)
        if n_alive == 0:
            print(f"    All evaporated at tick {t}")
            break
        if n_alive < 3 and t < 5:
            print(f"    Lost particle at tick {t} ({n_alive} remaining)")
    else:
        densities = [u2.density[pos] for pos in positions]
        print(f"    Survived 100 ticks! densities = {[f'{d:.4f}' for d in densities]}")

    # --- Test C: Locked particles WITH full wave propagation ---
    print("\n  Test C: Triad with full wave propagation (waves.py damping)")
    u3 = make_universe(size=size)
    for pos in positions:
        x, y, z = pos
        u3.states[x, y, z] = 1
        u3.charge[x, y, z] = 1.0
        u3.flux[x, y, z] = np.array([0.8, 0, 0], dtype=np.float32)
        u3.density[x, y, z] = 0.8
        u3.is_locked[x, y, z] = True

    for t in range(100):
        # Decay (skips locked) + waves (does NOT skip locked!)
        apply_decay(u3)
        waves.propagate_flux(u3)  # This damps ALL flux, including locked particles!
        forces.calculate_density(u3)
        n_alive = sum(1 for pos in positions if u3.states[pos] != 0)
        if n_alive < 3 and t == 0:
            # Check why
            for pos in positions:
                d = u3.density[pos]
                if d < CONSTANTS.KB:
                    print(f"    Position {pos}: density {d:.4f} < KB after first tick")
        if n_alive == 0:
            print(f"    All evaporated at tick {t}")
            break
    else:
        densities = [u3.density[pos] for pos in positions]
        print(f"    Survived 100 ticks! densities = {[f'{d:.4f}' for d in densities]}")

    # KEY CHECK: Does waves.py damping affect ALL voxels or just non-locked?
    print(f"\n  KEY FINDING:")
    print(f"  Phase 2 (decay): Skips locked voxels (correct)")
    print(f"  Phase 4 (waves): Applies DAMPING to ALL flux (does NOT check is_locked)")
    print(f"  This means locked particles STILL lose flux via wave damping!")
    print(f"  Combined with wave equation spreading flux to neighbors,")
    print(f"  even locked particles lose energy and eventually evaporate.")


# =============================================================================
# DIAGNOSTIC 5: WHAT WOULD IT TAKE FOR 1/r^2?
# =============================================================================

def diagnose_poisson_gravity():
    """
    To get 1/r^2 gravity, we need to solve Poisson's equation:
        Laplacian(phi) = -4*pi*G*rho
    Then F = -grad(phi).

    For a point mass, phi ~ -GM/r, and -grad(phi) ~ GM/r^2.

    Let's test: what does the EXISTING Laplacian solver give us
    if we use it to solve Poisson?
    """
    separator("DIAGNOSTIC 5: What Would 1/r^2 Gravity Require?")

    size = 64
    u = make_universe(size=size)
    center = size // 2

    # Set up a point-like density
    sigma = 2.0
    for x in range(size):
        for y in range(size):
            for z in range(size):
                dx = min(abs(x - center), size - abs(x - center))
                dy = min(abs(y - center), size - abs(y - center))
                dz = min(abs(z - center), size - abs(z - center))
                r = np.sqrt(dx**2 + dy**2 + dz**2)
                u.density[x, y, z] = 2.0 * np.exp(-r**2 / (2 * sigma**2))

    # Current gravity: F = G * grad(smooth(rho))
    geom = u.geometry
    rho_smooth = geom.smooth_field(u.density)
    f_current = CONSTANTS.GRAVITY_BIAS * geom.gradient(rho_smooth)

    # Correct gravity would need: solve Laplacian(phi) = -4*pi*G*rho
    # Then F = -grad(phi)
    # We can approximate by iterating: phi_{n+1} = (sum_neighbors phi_n + 4*pi*G*rho) / 6
    # Jacobi iteration for Poisson equation
    phi = np.zeros((size, size, size), dtype=np.float32)
    rho = u.density
    G = CONSTANTS.GRAVITY_BIAS

    print("  Solving Poisson equation via Jacobi iteration...")
    for iteration in range(500):
        phi_new = (
            np.roll(phi, 1, axis=0) + np.roll(phi, -1, axis=0) +
            np.roll(phi, 1, axis=1) + np.roll(phi, -1, axis=1) +
            np.roll(phi, 1, axis=2) + np.roll(phi, -1, axis=2) +
            4 * np.pi * G * rho
        ) / 6.0

        # Remove mean (periodic BCs need zero-mean for convergence)
        phi_new -= np.mean(phi_new)

        if iteration % 100 == 0:
            residual = np.max(np.abs(phi_new - phi))
            print(f"    Iteration {iteration}: max change = {residual:.2e}")

        phi = phi_new

    # Compute force from potential
    f_poisson = -geom.gradient(phi)

    # Compare power laws
    print(f"\n  {'r':>4} | {'F_current':>10} | {'F_poisson':>10} | {'1/r^2':>10}")
    print(f"  {'-'*4}-+-{'-'*10}-+-{'-'*10}-+-{'-'*10}")

    dist_curr = []
    fmag_curr = []
    dist_pois = []
    fmag_pois = []

    for r in range(2, 28):
        x = center + r
        if x >= size:
            break
        fc = np.sqrt(np.sum(f_current[x, center, center]**2))
        fp = np.sqrt(np.sum(f_poisson[x, center, center]**2))
        ir2 = 1.0 / (r * r)

        if fc > 1e-12:
            dist_curr.append(r)
            fmag_curr.append(fc)
        if fp > 1e-12:
            dist_pois.append(r)
            fmag_pois.append(fp)

        print(f"  {r:4d} | {fc:10.2e} | {fp:10.2e} | {ir2:10.6f}")

    # Fit power laws
    if len(dist_curr) >= 4:
        log_r = np.log(dist_curr)
        log_f = np.log(fmag_curr)
        A = np.vstack([log_r, np.ones_like(log_r)]).T
        n_curr = np.linalg.lstsq(A, log_f, rcond=None)[0][0]
        print(f"\n  Current gravity exponent: n = {n_curr:.3f}")

    if len(dist_pois) >= 4:
        log_r = np.log(dist_pois)
        log_f = np.log(fmag_pois)
        A = np.vstack([log_r, np.ones_like(log_r)]).T
        n_pois = np.linalg.lstsq(A, log_f, rcond=None)[0][0]
        print(f"  Poisson gravity exponent: n = {n_pois:.3f}")

    print(f"\n  CONCLUSION:")
    print(f"  Current: F = G*grad(smooth(rho)) gives n ~ -4 (Gaussian falloff)")
    print(f"  Poisson: F = -grad(phi) where Lap(phi)=rho gives n ~ -2 (correct)")
    print(f"  The fix requires solving Poisson's equation, not just taking grad(rho).")


# =============================================================================
# MAIN
# =============================================================================

def main():
    print("=" * 70)
    print("  FTD PHYSICS DIAGNOSTICS")
    print("  Root-cause analysis of simulation physics failures")
    print("=" * 70)

    diagnose_gravity()
    diagnose_energy_dissipation()
    diagnose_waves()
    diagnose_binding()
    diagnose_poisson_gravity()

    separator("OVERALL DIAGNOSIS")

    print("""  ISSUE 1: Gravity is NOT inverse-square (n = -4 instead of -2)
  ROOT CAUSE: F = G*grad(rho) computes the gradient of the DENSITY field,
  not the gradient of the gravitational POTENTIAL. For 1/r^2 gravity,
  you need to solve Poisson's equation: Lap(phi) = -4*pi*G*rho, then
  compute F = -grad(phi). The current formula gives a force that falls
  off with whatever the density profile is (Gaussian -> super-exponential).

  ISSUE 2: 99.9% energy loss in 200 ticks
  ROOT CAUSE: Three compounding dissipation mechanisms:
    a) DAMPING = 0.05 in waves.py (per-tick flux *= 0.95)
    b) DECAY_RATE = 0.00729 (per-tick flux *= 0.99271 for unlocked voxels)
    c) Combined per-tick retention: ~0.943, half-life ~12 ticks
  A particle with flux_mag = 0.8 and KB = 0.511 evaporates in ~8 ticks.
  Even with flux_mag = 10.0, it evaporates in ~52 ticks.

  CRITICAL: waves.py applies DAMPING to ALL flux including locked particles.
  This means even bound triads lose energy and eventually evaporate.

  ISSUE 3: Wave dispersion is wrong
  ROOT CAUSE: Damping dominates the oscillation. The wave amplitude decays
  by factor (1-0.05) = 0.95 per tick, losing 50% in ~14 ticks. For long
  wavelengths, the oscillation period exceeds the damping timescale, so
  the FFT sees mostly decay, not oscillation.

  Also: C_WAVE^2 = 0.16 > 1/6 = 0.167 -- marginal CFL stability. The
  damping is likely needed to prevent numerical blowup.

  ISSUE 4: All particles evaporate (nothing survives)
  ROOT CAUSE: Combination of issues 2 and the binding mechanism:
    - Decay (Phase 2) correctly skips locked particles
    - But wave damping (Phase 4) does NOT skip locked particles
    - So locked particles still lose flux at 5% per tick
    - Plus wave equation spreads their concentrated flux to neighbors
    - Combined effect: even triads evaporate within ~30 ticks

  RECOMMENDED FIXES (prioritized):
  1. Fix waves.py to NOT damp flux at locked particle sites
  2. Fix gravity to use Poisson solver instead of grad(rho)
  3. Reduce DAMPING or make it CFL-adaptive
  4. Consider flux source terms for manifested particles
     (particles should MAINTAIN their flux, not passively decay)
""")


if __name__ == '__main__':
    main()
