"""
Proof - Q4a: Z-factor measurement for FTD lattice stencils (FTD-0116 test)
============================================================================

CLAIM under test [HYPOTHESIS]: the FTD lattice Z-factor analog

    Z_FTD := G_L(0) / [continuum 2r * G(r) on light cone]
           = G_L(0) / (1/(2*pi))
           = G_L(0) * 2 * pi

equals G*^2 ≈ 8.754, where G* = Gamma(1/4)/Gamma(3/4) ≈ 2.9587.

TEST METHODOLOGY:
   For each candidate stencil S (SC, G18), and for L ∈ {8, 16, 32, 64}:
     1. Compute the lattice Poisson Green's function value at the origin
        G_L(0; S) = (1/L^3) * Sum_{k != 0} 1 / Delta_S(k)
        where Delta_S(k) is the Fourier eigenvalue of the lattice
        Laplacian for stencil S.
     2. Extrapolate to L = infinity.
     3. Compute Z_FTD(S) = G_inf(0; S) * 2 * pi.
   Compare to:
     (a) G*^2 ≈ 8.754                               (FTD-0116 hypothesis)
     (b) Watson cubic-lattice value (sanity check)
     (c) Any other clean closed-form candidate

VERDICT: PASS / PARTIAL / FAIL based on whether Z_FTD matches any
clean closed-form constant.

Stencil definitions (from SPEC_FTD_NATIVE_ELECTRODYNAMICS.md, §15):
   SC:  6 face neighbors, weights (a=1, b=0, c=0)
        Delta_SC(k) = 6 - 2(cos k_x + cos k_y + cos k_z) = k_hat^2
   G18: 6 face + 12 edge neighbors, weights (a=1/3, b=1/6, c=0)
        Delta_G18(k) = (1/3)(6 - 2 sum cos)
                     + (1/6)(12 - 4 sum cos*cos pairs)

Provenance: docs/theory/09_mathematical/archive/EXPLR_TWO_PI_GSTAR_CONNECTION.md
LEDGER: FTD-0116 Q4a test.

Usage:
    python scripts/proofs/proof_z_factor_q4a.py
"""

import math
import sys
from itertools import product

# ---------------------------------------------------------------------------
# Canonical constants
# ---------------------------------------------------------------------------

# Project canonical G_STAR = Gamma(1/4) / Gamma(3/4) ~ 2.9587 (see FTD-0117)
# Computed exactly via Gamma function for precision:
import math as _m
GAMMA_QUARTER_SQ_OVER_PI_SQRT2 = (
    # Gamma(1/4) ~ 3.625609908..., Gamma(3/4) ~ 1.225416703...
    3.6256099082219083 / 1.2254167024651776
)
G_STAR = GAMMA_QUARTER_SQ_OVER_PI_SQRT2  # ~ 2.9587

WATSON_CUBIC_STANDARD = 0.50546201232  # Watson 1939 W3 (cubic lattice)
                                        # = (1/pi^3) * triple integral over [0,pi]^3
                                        # of 1/(3 - cos x - cos y - cos z)


# ---------------------------------------------------------------------------
# Stencil eigenvalues
# ---------------------------------------------------------------------------

def k_hat_squared(k_vec):
    """Standard SC lattice Laplacian eigenvalue: 4 sum_i sin^2(k_i / 2)."""
    return sum(4.0 * math.sin(0.5 * ki) ** 2 for ki in k_vec)


def delta_SC(k_vec):
    """Simple cubic stencil Laplacian eigenvalue."""
    return k_hat_squared(k_vec)


def delta_G18(k_vec):
    """G18 stencil (face + edge): a=1/3, b=1/6, c=0.

    Delta_G18(k) = (1/3) (6 - 2 sum_i cos k_i)
                 + (1/6) (12 - 4 sum_{i<j} cos k_i cos k_j)
                 = 4 - (2/3)(sum_i cos k_i + sum_{i<j} cos k_i cos k_j)

    Or equivalently (one can verify):
      Delta_G18(k) = (1/3) k_hat^2 + (1/6) * (face-diagonal kernel)
    where the face-diagonal kernel comes from the 12 edge neighbors.
    """
    cos_x, cos_y, cos_z = (math.cos(ki) for ki in k_vec)
    sum_cos = cos_x + cos_y + cos_z
    sum_cos_pairs = cos_x * cos_y + cos_x * cos_z + cos_y * cos_z
    return 4.0 - (2.0 / 3.0) * (sum_cos + sum_cos_pairs)


# ---------------------------------------------------------------------------
# Lattice Green's function at origin
# ---------------------------------------------------------------------------

def green_at_origin(L, delta_func):
    """G_L(0) = (1/L^3) * Sum_{k != 0} 1 / Delta(k).

    Uses crystal momenta k_i = 2*pi*n_i/L for n_i in 0, ..., L-1.
    """
    total = 0.0
    for ints in product(range(L), repeat=3):
        if all(n == 0 for n in ints):
            continue
        k_vec = tuple(2.0 * math.pi * n / L for n in ints)
        d = delta_func(k_vec)
        if abs(d) < 1e-14:
            continue  # skip near-zero (zero mode already excluded)
        total += 1.0 / d
    return total / (L ** 3)


def extrapolate_to_infinity(L_values, G_values):
    """Richardson-style extrapolation: assume G_L(0) ≈ G_inf + a/L + b/L^2.

    Use last three points to fit. Returns (G_inf_estimate, a, b).
    """
    if len(L_values) < 3:
        return G_values[-1], 0.0, 0.0
    # Solve linear system using last three points
    L1, L2, L3 = L_values[-3:]
    G1, G2, G3 = G_values[-3:]
    # G_inf + a/L + b/L^2 = G_L
    # 3 equations, 3 unknowns
    import numpy as np  # acceptable for this script
    A = np.array([[1.0, 1.0/L1, 1.0/L1**2],
                  [1.0, 1.0/L2, 1.0/L2**2],
                  [1.0, 1.0/L3, 1.0/L3**2]])
    b = np.array([G1, G2, G3])
    x = np.linalg.solve(A, b)
    return float(x[0]), float(x[1]), float(x[2])


# ---------------------------------------------------------------------------
# Test
# ---------------------------------------------------------------------------

def run_test(stencil_name, delta_func, L_values=(8, 16, 32, 64, 96, 128)):
    print(f"\n--- Stencil: {stencil_name} ---")
    print(f"  {'L':>6} | {'G_L(0)':>22} | {'Z_FTD = G_L(0) * 2pi':>22}")
    print(f"  {'-'*6} | {'-'*22} | {'-'*22}")
    G_values = []
    for L in L_values:
        G = green_at_origin(L, delta_func)
        Z = G * 2 * math.pi
        G_values.append(G)
        print(f"  {L:>6} | {G:>22.10f} | {Z:>22.10f}")

    # Extrapolate
    G_inf, a, b = extrapolate_to_infinity(list(L_values), G_values)
    Z_inf = G_inf * 2 * math.pi
    print(f"  {'∞':>6} | {G_inf:>22.10f} | {Z_inf:>22.10f}  (Richardson extrapolated)")
    print(f"  Fit: G_L(0) ≈ {G_inf:.6f} + {a:.4f}/L + {b:.4f}/L^2")

    # Compare to candidates
    print()
    print(f"  Candidate matches for Z_FTD = {Z_inf:.6f}:")
    candidates = [
        ("G*^2",                G_STAR ** 2,                    "FTD-0116 hypothesis"),
        ("G*",                  G_STAR,                         "single power of G*"),
        ("G*^2 / 2",            G_STAR ** 2 / 2,                "half of G*^2"),
        ("Watson_cubic * 2π",   WATSON_CUBIC_STANDARD * 2 * math.pi, "standard Watson scaled"),
        ("G_inf(0)*2π reading", Z_inf,                          "the measurement itself"),
        ("π/2",                 math.pi / 2,                    "geometric"),
        ("3π/2",                3 * math.pi / 2,                "geometric"),
        ("1.5",                 1.5,                            "rational"),
        ("2",                   2.0,                            "rational"),
        ("3",                   3.0,                            "rational"),
    ]
    for label, val, comment in candidates:
        rel_err = abs(Z_inf - val) / val if val != 0 else float("inf")
        flag = "  <-- MATCH" if rel_err < 0.01 else ("  (close)" if rel_err < 0.05 else "")
        print(f"    {label:25s} = {val:>14.6f},  rel_err = {rel_err:>9.2%}{flag}")

    return Z_inf, G_inf


def main():
    print("=" * 72)
    print("PROOF Q4a: FTD lattice Z-factor measurement")
    print("Tests FTD-0116 hypothesis: Z_FTD = G_L(0) * 2pi = G*^2 ≈ 8.754")
    print("=" * 72)
    print(f"G* = Gamma(1/4)/Gamma(3/4) = {G_STAR:.10f}")
    print(f"G*^2                       = {G_STAR**2:.10f}")
    print(f"Continuum amplitude 1/(2π) = {1/(2*math.pi):.10f}")
    print()
    print("Computing G_L(0) for each stencil at increasing L:")

    # Test SC stencil
    Z_SC, G_SC = run_test("SC (simple cubic, 6 face nbrs only)", delta_SC)

    # Test G18 stencil (the engine canonical)
    Z_G18, G_G18 = run_test("G18 (engine canonical: face + edge, weights 1/3, 1/6)", delta_G18)

    # Verdict
    print()
    print("=" * 72)
    print("VERDICT")
    print("=" * 72)
    G_star_sq = G_STAR ** 2

    sc_match  = abs(Z_SC - G_star_sq) / G_star_sq < 0.01
    g18_match = abs(Z_G18 - G_star_sq) / G_star_sq < 0.01

    if sc_match and g18_match:
        print("PASS: Z_FTD = G*^2 holds for both SC and G18 stencils.")
        print("      FTD-0116 hypothesis confirmed numerically.")
    elif g18_match and not sc_match:
        print("PARTIAL: Z_FTD = G*^2 holds for G18 but NOT for SC.")
        print("         The Z-factor reading is stencil-specific, not universal.")
    elif sc_match and not g18_match:
        print("PARTIAL: Z_FTD = G*^2 holds for SC but NOT for G18.")
        print("         FTD-0116 needs revision (engine uses G18, not SC).")
    else:
        print("FAIL: Z_FTD = G*^2 does NOT hold for either stencil.")
        print("      The naive Z-factor reading is FALSIFIED for FTD's lattice.")
        print()
        print(f"      Measured Z_FTD(SC)  = {Z_SC:.6f}")
        print(f"      Measured Z_FTD(G18) = {Z_G18:.6f}")
        print(f"      Predicted G*^2       = {G_star_sq:.6f}")
        print(f"      Ratio (G18 / G*^2)   = {Z_G18 / G_star_sq:.6f}")
        print(f"      Ratio (G*^2 / G18)   = {G_star_sq / Z_G18:.6f}")
        print()
        print("      RECOMMENDATION: revise FTD-0116 hypothesis to identify")
        print("      what Z_FTD actually equals for the engine's G18 stencil,")
        print("      OR demote FTD-0116 to [CLOSED NEGATIVE].")

    # Always exit 0 — this is a measurement, not a unit test
    return 0


if __name__ == "__main__":
    sys.exit(main())
