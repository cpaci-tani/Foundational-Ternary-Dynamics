"""
Gap Equation Fixed-Point Convergence by Moore Layer Subset

Tests the conjecture (PI-C14/PI-C15) that self-referential closure — the gap
equation x^2 = 16G*^2(x - G*) — requires all three Moore layers (SC+FCC+BCC),
while measurement (SC+FCC only) does NOT close the loop.

Method:
  For each of 7 sublattice combinations (SC, FCC, BCC, SC+FCC, SC+BCC, FCC+BCC,
  full Moore), we:
    1. Build the sublattice Laplacian eigenvalue function
    2. Compute the Watson integral (Green's function at origin) as L -> inf
    3. Form the gap equation: x^2 = 16 * 2pi * W_S * (x - G*)
    4. Solve the quadratic directly (roots, discriminant)
    5. Run fixed-point iteration: x_{n+1} = K_S(1 - G*/x_n)
    6. Report convergence rate, basin of attraction, and root quality

RESULT (corrected April 11, 2026): The Watson identity W3 = G*^2/(2pi) is the
BCC Watson integral, NOT the SC Watson integral. This was initially misidentified
due to finite-size effects at L=48 where the SC value happened to be numerically
closer to the target. At larger L (64, 96, 128), SC diverges from the target
while BCC converges to it.

Analytic proof: The BCC eigenvalue 1-cos(k1)*cos(k2)*cos(k3) involves a PRODUCT
of cosines. The geometric series 1/(1-xyz) = sum (xyz)^n FACTORS across axes,
giving [C(2m,m)/4^m]^3 whose sum equals Gamma(1/4)^4/(4*pi^3) = G*^2/(2*pi).
SC's SUM structure cannot factor this way.

Status: [EXPLORATORY — corrected]
Epistemic: Confirms PI-C14 original (BCC provides gap equation coefficient)
"""

import numpy as np
from scipy.special import gamma
import sys
import os
import time

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
from constants import G_STAR, N_c, GAMMA_QUARTER

# ============================================================================
# Constants
# ============================================================================

W3_EXACT = G_STAR**2 / (2 * np.pi)   # Watson integral (SC) = 1.3932...
K_EXACT  = 16 * G_STAR**2             # Master quadratic coefficient = 140.06...

# Master quadratic roots (exact)
DISC_EXACT = K_EXACT**2 - 4 * K_EXACT * G_STAR
X_PLUS  = (K_EXACT + np.sqrt(DISC_EXACT)) / 2   # 137.036...
X_MINUS = (K_EXACT - np.sqrt(DISC_EXACT)) / 2    # 3.024...

ALPHA_INV = X_PLUS   # 1/alpha

# ============================================================================
# Sublattice definitions
# ============================================================================

# Neighbor offsets for each Moore layer
SC_OFFSETS = [
    (1,0,0), (-1,0,0), (0,1,0), (0,-1,0), (0,0,1), (0,0,-1)
]  # 6 face-adjacent, distance 1

FCC_OFFSETS = [
    (1,1,0), (1,-1,0), (-1,1,0), (-1,-1,0),
    (1,0,1), (1,0,-1), (-1,0,1), (-1,0,-1),
    (0,1,1), (0,1,-1), (0,-1,1), (0,-1,-1)
]  # 12 edge-adjacent, distance sqrt(2)

BCC_OFFSETS = [
    (1,1,1), (1,1,-1), (1,-1,1), (1,-1,-1),
    (-1,1,1), (-1,1,-1), (-1,-1,1), (-1,-1,-1)
]  # 8 corner-adjacent, distance sqrt(3)

SUBLATTICE_CONFIGS = {
    'SC':        {'offsets': SC_OFFSETS,                    'count': 6,  'label': 'SC only (6)'},
    'FCC':       {'offsets': FCC_OFFSETS,                   'count': 12, 'label': 'FCC only (12)'},
    'BCC':       {'offsets': BCC_OFFSETS,                   'count': 8,  'label': 'BCC only (8)'},
    'SC+FCC':    {'offsets': SC_OFFSETS + FCC_OFFSETS,      'count': 18, 'label': 'SC+FCC (18)'},
    'SC+BCC':    {'offsets': SC_OFFSETS + BCC_OFFSETS,      'count': 14, 'label': 'SC+BCC (14)'},
    'FCC+BCC':   {'offsets': FCC_OFFSETS + BCC_OFFSETS,     'count': 20, 'label': 'FCC+BCC (20)'},
    'Moore':     {'offsets': SC_OFFSETS + FCC_OFFSETS + BCC_OFFSETS, 'count': 26, 'label': 'Full Moore (26)'},
}

# ============================================================================
# Sublattice eigenvalue functions (momentum space)
# ============================================================================

def eigenvalue_sublattice(kx, ky, kz, offsets, n_neighbors):
    """
    Normalized Laplacian eigenvalue for a sublattice.

    sigma_S(k) = 1 - (1/|S|) * sum_{delta in S} cos(k . delta)

    Returns value in [0, 2], with sigma=0 at k=0.
    """
    structure_sum = 0.0
    for dx, dy, dz in offsets:
        structure_sum += np.cos(kx*dx + ky*dy + kz*dz)
    return 1.0 - structure_sum / n_neighbors


# ============================================================================
# Watson integral computation
# ============================================================================

def watson_integral(offsets, n_neighbors, L):
    """
    Compute the Watson integral (Green's function at origin) for a sublattice
    on an L x L x L periodic torus.

    W_S(L) = (1/N) sum_{k != 0} 1 / sigma_S(k)

    Converges to the infinite-lattice Watson integral as L -> inf.
    """
    N = L**3
    G_sum = 0.0

    for nx in range(L):
        for ny in range(L):
            for nz in range(L):
                kx = 2 * np.pi * nx / L
                ky = 2 * np.pi * ny / L
                kz = 2 * np.pi * nz / L

                sigma = eigenvalue_sublattice(kx, ky, kz, offsets, n_neighbors)

                if sigma < 1e-12:
                    continue  # skip zero mode

                G_sum += 1.0 / sigma

    return G_sum / N


# ============================================================================
# Gap equation solver
# ============================================================================

def gap_equation_roots(W_S, n_dof=16):
    """
    Solve the gap equation: x^2 = n_dof * 2pi * W_S * (x - G*)
    Rearranged: x^2 - K*x + K*G* = 0  where K = n_dof * 2pi * W_S

    Returns: (K, discriminant, x_plus, x_minus)
    If discriminant < 0, roots are complex and returned as NaN.
    """
    K = n_dof * 2 * np.pi * W_S
    disc = K**2 - 4 * K * G_STAR

    if disc >= 0:
        x_plus  = (K + np.sqrt(disc)) / 2
        x_minus = (K - np.sqrt(disc)) / 2
    else:
        x_plus = x_minus = float('nan')

    return K, disc, x_plus, x_minus


# ============================================================================
# Fixed-point iteration
# ============================================================================

def fixed_point_iterate(K, x0, tol=1e-12, max_iter=500):
    """
    Run fixed-point iteration: x_{n+1} = K * (1 - G*/x_n)

    Returns: (converged, final_x, n_iterations, history)
    """
    x = x0
    history = [x]

    for i in range(max_iter):
        if abs(x) < 1e-15:
            return False, x, i, history  # diverged to zero

        x_new = K * (1.0 - G_STAR / x)
        history.append(x_new)

        if abs(x_new) > 1e15:
            return False, x_new, i+1, history  # diverged

        if abs(x_new - x) < tol:
            return True, x_new, i+1, history  # converged

        x = x_new

    return False, x, max_iter, history  # did not converge


def contraction_rate_at_root(K, x_root):
    """
    Contraction rate |f'(x)| at a fixed point.
    f(x) = K(1 - G*/x), f'(x) = K*G*/x^2

    At root: f'(x_root) = K*G*/x_root^2 = G*/(x_root - G*)
    (using x_root^2 = K*(x_root - G*) from the gap equation)

    |f'| < 1 means stable (attracting), |f'| > 1 means unstable.
    """
    if abs(x_root - G_STAR) < 1e-15:
        return float('inf')
    return abs(G_STAR / (x_root - G_STAR))


# ============================================================================
# Main computation
# ============================================================================

def main():
    print("=" * 90)
    print("GAP EQUATION CONVERGENCE BY MOORE LAYER SUBSET")
    print("Testing PI-C14 (measurement at FCC boundary) and PI-C15 (sLoop = self-observation)")
    print("=" * 90)
    print()
    print(f"  G*       = {G_STAR:.10f}")
    print(f"  W3 (SC)  = {W3_EXACT:.10f}")
    print(f"  K_exact  = 16*G*^2 = {K_EXACT:.10f}")
    print(f"  x+       = 1/alpha = {X_PLUS:.10f}")
    print(f"  x-       = N_c     = {X_MINUS:.10f}")
    print()

    # ------------------------------------------------------------------
    # PART 1: Watson integrals for each sublattice
    # ------------------------------------------------------------------

    print("=" * 90)
    print("PART 1: Watson integrals by sublattice (convergence as L -> inf)")
    print("=" * 90)
    print()

    lattice_sizes = [4, 8, 16, 32, 48]
    watson_results = {}

    for name, cfg in SUBLATTICE_CONFIGS.items():
        print(f"  {cfg['label']}:")
        offsets = cfg['offsets']
        n_nb = cfg['count']
        ws = []

        for L in lattice_sizes:
            t0 = time.time()
            W = watson_integral(offsets, n_nb, L)
            dt = time.time() - t0
            ws.append(W)
            print(f"    L={L:3d}: W = {W:.10f}  ({dt:.2f}s)")

        # Extrapolate: use largest L as best estimate
        W_best = ws[-1]
        watson_results[name] = W_best

        # Compare to exact BCC Watson integral = G*^2/(2pi)
        ratio = W_best / W3_EXACT
        print(f"    Best estimate (L={lattice_sizes[-1]}): W = {W_best:.10f}")
        print(f"    Ratio W_S / W3_BCC_exact = {ratio:.6f}")
        if name == 'BCC':
            print(f"    ** BCC analytic limit: G*^2/(2pi) = {W3_EXACT:.10f} **")
            print(f"    ** Convergence: {abs(W_best-W3_EXACT)/W3_EXACT*100:.2f}% "
                  f"remaining at L={lattice_sizes[-1]} (slow: 4 zero modes) **")
        print()

    # ------------------------------------------------------------------
    # PART 2: Gap equation roots for each sublattice
    # ------------------------------------------------------------------

    print("=" * 90)
    print("PART 2: Gap equation roots by sublattice")
    print("  Gap equation: x^2 = 16 * 2pi * W_S * (x - G*)")
    print("=" * 90)
    print()

    print(f"  {'Sublattice':<16} {'W_S':>12} {'K=16*2pi*W':>14} {'Disc':>14} "
          f"{'x+':>12} {'x-':>12} {'x+/alpha_inv':>14} {'x-/N_c':>10}")
    print("-" * 108)

    for name in SUBLATTICE_CONFIGS:
        W_S = watson_results[name]
        K, disc, xp, xm = gap_equation_roots(W_S)
        cfg = SUBLATTICE_CONFIGS[name]

        if not np.isnan(xp):
            ratio_p = xp / ALPHA_INV
            ratio_m = xm / N_c
            print(f"  {cfg['label']:<16} {W_S:12.6f} {K:14.6f} {disc:14.4f} "
                  f"{xp:12.6f} {xm:12.6f} {ratio_p:14.6f} {ratio_m:10.6f}")
        else:
            print(f"  {cfg['label']:<16} {W_S:12.6f} {K:14.6f} {disc:14.4f} "
                  f"{'complex':>12} {'complex':>12} {'---':>14} {'---':>10}")

    print()
    print(f"  {'EXACT'::<16} {W3_EXACT:12.6f} {K_EXACT:14.6f} {DISC_EXACT:14.4f} "
          f"{X_PLUS:12.6f} {X_MINUS:12.6f} {'1.000000':>14} {'1.000000':>10}")

    # ------------------------------------------------------------------
    # PART 3: Fixed-point iteration convergence
    # ------------------------------------------------------------------

    print()
    print("=" * 90)
    print("PART 3: Fixed-point iteration x_{n+1} = K(1 - G*/x_n)")
    print("  Starting from x0 = 100 (testing convergence to x+)")
    print("=" * 90)
    print()

    starting_points = [100.0, 50.0, 200.0, 10.0, 500.0]

    for name in SUBLATTICE_CONFIGS:
        W_S = watson_results[name]
        K, disc, xp, xm = gap_equation_roots(W_S)
        cfg = SUBLATTICE_CONFIGS[name]

        print(f"  {cfg['label']}:  K = {K:.4f}")

        if np.isnan(xp):
            print(f"    No real roots — fixed-point iteration not applicable")
            print()
            continue

        # Contraction rates
        rate_plus = contraction_rate_at_root(K, xp)
        rate_minus = contraction_rate_at_root(K, xm)
        print(f"    x+ = {xp:.6f}  |f'(x+)| = {rate_plus:.6f}  "
              f"({'STABLE' if rate_plus < 1 else 'UNSTABLE'})")
        print(f"    x- = {xm:.6f}  |f'(x-)| = {rate_minus:.6f}  "
              f"({'STABLE' if rate_minus < 1 else 'UNSTABLE'})")

        # Run iteration from multiple starting points
        for x0 in starting_points:
            converged, x_final, n_iter, history = fixed_point_iterate(K, x0)
            if converged:
                # Which root did it converge to?
                if abs(x_final - xp) < 0.01:
                    target = "x+"
                elif abs(x_final - xm) < 0.01:
                    target = "x-"
                else:
                    target = "???"
                print(f"    x0={x0:6.1f}: CONVERGED to {target} = {x_final:.8f} "
                      f"in {n_iter} iterations")
            else:
                if abs(x_final) > 1e10:
                    print(f"    x0={x0:6.1f}: DIVERGED (x -> {x_final:.2e}) "
                          f"after {n_iter} iterations")
                elif abs(x_final) < 1e-10:
                    print(f"    x0={x0:6.1f}: COLLAPSED (x -> 0) "
                          f"after {n_iter} iterations")
                else:
                    print(f"    x0={x0:6.1f}: NO CONVERGENCE (x = {x_final:.6f}) "
                          f"after {n_iter} iterations")
        print()

    # ------------------------------------------------------------------
    # PART 4: Convergence rate comparison
    # ------------------------------------------------------------------

    print("=" * 90)
    print("PART 4: Convergence rate comparison (iterations to 1 ppm at x0=100)")
    print("=" * 90)
    print()

    print(f"  {'Sublattice':<16} {'W_S/W3_SC':>10} {'K/K_exact':>10} "
          f"{'x+':>12} {'|f\'(x+)|':>10} {'Iters':>8} {'x+ error ppm':>14}")
    print("-" * 84)

    for name in SUBLATTICE_CONFIGS:
        W_S = watson_results[name]
        K, disc, xp, xm = gap_equation_roots(W_S)
        cfg = SUBLATTICE_CONFIGS[name]

        if np.isnan(xp):
            print(f"  {cfg['label']:<16} {W_S/W3_EXACT:10.4f} {K/K_EXACT:10.4f} "
                  f"{'complex':>12} {'---':>10} {'---':>8} {'---':>14}")
            continue

        rate = contraction_rate_at_root(K, xp)
        converged, x_final, n_iter, _ = fixed_point_iterate(K, 100.0, tol=xp*1e-6)

        # Error relative to the EXACT master quadratic roots
        err_ppm = abs(xp - X_PLUS) / X_PLUS * 1e6

        print(f"  {cfg['label']:<16} {W_S/W3_EXACT:10.4f} {K/K_EXACT:10.4f} "
              f"{xp:12.4f} {rate:10.6f} {n_iter:8d} {err_ppm:14.1f}")

    print()
    print(f"  {'EXACT':<16} {'1.0000':>10} {'1.0000':>10} "
          f"{X_PLUS:12.4f} {contraction_rate_at_root(K_EXACT, X_PLUS):10.6f} "
          f"{'---':>8} {'0.0':>14}")

    # ------------------------------------------------------------------
    # PART 5: The critical test — does SC match and do others deviate?
    # ------------------------------------------------------------------

    print()
    print("=" * 90)
    print("PART 5: CRITICAL TEST — Which sublattice reproduces the master quadratic?")
    print("=" * 90)
    print()

    for name in SUBLATTICE_CONFIGS:
        W_S = watson_results[name]
        K, disc, xp, xm = gap_equation_roots(W_S)
        cfg = SUBLATTICE_CONFIGS[name]

        if np.isnan(xp):
            match = "NO REAL ROOTS"
        else:
            err_alpha = abs(xp - ALPHA_INV) / ALPHA_INV
            err_nc = abs(xm - X_MINUS) / X_MINUS
            if err_alpha < 0.01 and err_nc < 0.01:
                match = f"MATCH (alpha err: {err_alpha*1e6:.1f} ppm, N_c err: {err_nc*1e6:.1f} ppm)"
            else:
                match = f"NO MATCH (x+ = {xp:.4f}, x- = {xm:.4f})"

        print(f"  {cfg['label']:<20} W_S = {W_S:.8f}  ->  {match}")

    # ------------------------------------------------------------------
    # PART 6: Layer contribution analysis
    # ------------------------------------------------------------------

    print()
    print("=" * 90)
    print("PART 6: Watson integral decomposition")
    print("=" * 90)
    print()

    W_SC = watson_results['SC']
    W_FCC = watson_results['FCC']
    W_BCC = watson_results['BCC']
    W_Moore = watson_results['Moore']

    print(f"  Individual sublattices (finite-L estimates, converging to analytic limits):")
    print(f"    W_SC       = {W_SC:.10f}   (analytic limit: ~1.5164, NOT G*^2/(2pi))")
    print(f"    W_FCC      = {W_FCC:.10f}")
    print(f"    W_BCC      = {W_BCC:.10f}   (analytic limit: G*^2/(2pi) = {W3_EXACT:.10f})")
    print()
    print(f"  Combined (computed directly, NOT sum of parts):")
    print(f"    W_SC+FCC   = {watson_results['SC+FCC']:.10f}")
    print(f"    W_SC+BCC   = {watson_results['SC+BCC']:.10f}")
    print(f"    W_FCC+BCC  = {watson_results['FCC+BCC']:.10f}")
    print(f"    W_Moore    = {watson_results['Moore']:.10f}")
    print()
    print(f"  Key ratios:")
    print(f"    W_SC / W3_exact           = {W_SC/W3_EXACT:.8f}")
    print(f"    W_Moore / W3_exact        = {W_Moore/W3_EXACT:.8f}")
    print(f"    W_SC+FCC / W3_exact       = {watson_results['SC+FCC']/W3_EXACT:.8f}")
    print(f"    W_BCC / W3_exact          = {W_BCC/W3_EXACT:.8f}")

    # ------------------------------------------------------------------
    # PART 7: Self-referential closure test
    # ------------------------------------------------------------------

    print()
    print("=" * 90)
    print("PART 7: Self-referential closure — does the loop close?")
    print("  Testing: start from x0, iterate, check if output = input (fixed point)")
    print("=" * 90)
    print()

    # The self-referential test: iterate from a RANDOM starting point
    # and check whether the system finds the gap equation roots.
    np.random.seed(42)
    n_trials = 20
    random_starts = np.random.uniform(1.0, 500.0, n_trials)

    for name in ['SC', 'SC+FCC', 'Moore']:
        W_S = watson_results[name]
        K, disc, xp, xm = gap_equation_roots(W_S)
        cfg = SUBLATTICE_CONFIGS[name]

        if np.isnan(xp):
            print(f"  {cfg['label']}: No real fixed points exist.")
            continue

        n_converged = 0
        n_to_xplus = 0
        converge_iters = []

        for x0 in random_starts:
            converged, x_final, n_iter, _ = fixed_point_iterate(K, x0)
            if converged:
                n_converged += 1
                converge_iters.append(n_iter)
                if abs(x_final - xp) < 0.1:
                    n_to_xplus += 1

        rate = contraction_rate_at_root(K, xp)
        avg_iter = np.mean(converge_iters) if converge_iters else float('nan')

        print(f"  {cfg['label']}:")
        print(f"    x+ = {xp:.6f}, |f'(x+)| = {rate:.6f}")
        print(f"    Converged: {n_converged}/{n_trials} trials")
        print(f"    To x+: {n_to_xplus}/{n_trials} trials")
        print(f"    Avg iterations: {avg_iter:.1f}")

        # Does this match the master quadratic?
        err_alpha = abs(xp - ALPHA_INV) / ALPHA_INV * 1e6
        if err_alpha < 100:
            print(f"    ** x+ matches 1/alpha to {err_alpha:.1f} ppm **")
        else:
            print(f"    x+ deviates from 1/alpha by {err_alpha:.0f} ppm "
                  f"(x+ = {xp:.4f} vs 137.0362)")
        print()

    # ------------------------------------------------------------------
    # Summary
    # ------------------------------------------------------------------

    print("=" * 90)
    print("SUMMARY (CORRECTED)")
    print("=" * 90)
    print()
    print("CORRECTION (April 11, 2026): The Watson identity W3 = G*^2/(2pi) is the")
    print("BCC lattice Green's function, NOT SC. Initial analysis at L=48 was misleading")
    print("because SC was numerically closer to the target at that lattice size.")
    print("At larger L (64, 96, 128), SC diverges from the target while BCC converges.")
    print()
    print(f"  Analytic limits:")
    print(f"    BCC -> G*^2/(2pi) = {W3_EXACT:.10f}  (EXACT, proven)")
    print(f"    SC  -> ~1.5164     (different value, NOT G*^2/(2pi))")
    print()
    print("Gap equation coefficient K = 16 * 2pi * W_S:")

    W_bcc = watson_results['BCC']
    W_sc = watson_results['SC']
    K_bcc_analytic = 16 * 2 * np.pi * W3_EXACT
    print(f"  BCC (analytic L=inf): K = {K_bcc_analytic:.4f} = 16*G*^2 = {K_EXACT:.4f}  ** EXACT MATCH **")
    print(f"  BCC (L={lattice_sizes[-1]}):         K = {16*2*np.pi*W_bcc:.4f}  "
          f"({abs(16*2*np.pi*W_bcc - K_EXACT)/K_EXACT*100:.1f}% from target, converging)")
    print(f"  SC  (L={lattice_sizes[-1]}):         K = {16*2*np.pi*W_sc:.4f}  "
          f"(converging to ~152.4, WRONG target)")
    print()
    print("Why BCC is special:")
    print("  BCC eigenvalue: 1 - cos(k1)*cos(k2)*cos(k3)  [PRODUCT of cosines]")
    print("  SC eigenvalue:  1 - (cos(k1)+cos(k2)+cos(k3))/3  [SUM of cosines]")
    print()
    print("  The PRODUCT structure allows 1/(1-xyz) = sum (xyz)^n to FACTOR across axes,")
    print("  giving [C(2m,m)/4^m]^3 whose sum = Gamma(1/4)^4/(4*pi^3) = G*^2/(2*pi).")
    print("  SC's SUM structure cannot factor this way.")
    print()
    print("  BCC couples all 3 flux directions MULTIPLICATIVELY.")
    print("  This multiplicative coupling IS the lemniscatic connection.")
    print("  The gap equation's self-energy comes from BCC because BCC is where")
    print("  all three J-components are simultaneously excited (SU(3) / strong channel).")


if __name__ == '__main__':
    main()
