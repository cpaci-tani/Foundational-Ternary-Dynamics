"""
FTD Part B: Lattice Sum Derivation of Alpha
=============================================

Can 1/α = 137.035999... be expressed as a lattice sum over ℤ[i]?

The Epstein zeta function for the Gaussian integers:
  Z(s) = Σ'_{(a,b)∈ℤ²} 1/(a²+b²)^s = 4·ζ(s)·L(s, χ₋₄)

where χ₋₄ is the non-principal character mod 4 (Dirichlet beta).

This script systematically searches for expressions relating 1/α to:
1. Z(s) at rational or algebraic values of s
2. Twisted lattice sums with framework-integer weights
3. Products/ratios of Z at multiple s values
4. Combinations with framework constants {3, 4, 7, 13}

Author: AI-assisted research (February 2026)
Framework: FTD v5.24
"""

import sys
import os
import numpy as np

try:
    from mpmath import (mp, mpf, pi as mpi, exp as mexp, log as mlog,
                        zeta as mzeta, gamma as mgamma, sqrt as msqrt,
                        power as mpower, altzeta, euler as meuler,
                        catalan as mcatalan, inf as minf, nsum, fac)
    HAS_MPMATH = True
except ImportError:
    HAS_MPMATH = False
    print("ERROR: mpmath required. pip install mpmath")
    sys.exit(1)

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
sys.path.insert(0, PROJECT_ROOT)
from simulations.constants import N_c, N_base, b_3, N_eff

# Target
ALPHA_INV = mpf('137.035999084')  # 1/α (CODATA 2018)


# =============================================================================
# SECTION 1: The Epstein Zeta Function for ℤ[i]
# =============================================================================

def dirichlet_beta(s):
    """
    Compute Dirichlet beta function β(s) = L(s, χ₋₄)
    β(s) = Σ_{n=0}^∞ (-1)^n / (2n+1)^s
    """
    return nsum(lambda n: (-1)**n / (2*n + 1)**s, [0, minf])


def epstein_zeta(s, N_terms=None):
    """
    Compute the Epstein zeta function for ℤ[i]:
      Z(s) = Σ'_{(a,b)∈ℤ²} 1/(a²+b²)^s = 4·ζ(s)·L(s, χ₋₄)

    Uses the factorization for efficiency.
    """
    return 4 * mzeta(s) * dirichlet_beta(s)


def section_1_epstein_zeta():
    """Compute and display Z(s) at various special values."""
    mp.dps = 30

    print("=" * 70)
    print("SECTION 1: EPSTEIN ZETA FUNCTION Z(s) FOR ℤ[i]")
    print("=" * 70)

    print("""
  Z(s) = 4·ζ(s)·β(s)  where β(s) = Σ (-1)^n/(2n+1)^s

  Known exact values:
    β(1) = π/4          (Leibniz formula)
    β(2) = G ≈ 0.916    (Catalan's constant)
    β(3) = π³/32
    β(4) = π⁴/768 + β(2)·(something)
""")

    # Compute Z(s) at integer and half-integer values
    print(f"  {'s':>8} | {'ζ(s)':>20} | {'β(s)':>20} | {'Z(s) = 4ζβ':>20}")
    print(f"  {'-'*8}-+-{'-'*20}-+-{'-'*20}-+-{'-'*20}")

    s_values = [mpf(2), mpf(3), mpf(4), mpf(5), mpf(6),
                mpf(3)/2, mpf(5)/2, mpf(7)/2,
                mpf(4)/3, mpf(5)/3, mpf(7)/3]

    results = {}
    for s in s_values:
        z_val = mzeta(s)
        b_val = dirichlet_beta(s)
        Z_val = 4 * z_val * b_val
        results[float(s)] = Z_val
        print(f"  {float(s):8.4f} | {float(z_val):20.15f} | {float(b_val):20.15f} | {float(Z_val):20.15f}")

    print()

    # Check if any Z(s) is near 1/α
    print(f"  Target: 1/α = {float(ALPHA_INV):.15f}")
    print()
    print("  Direct matches Z(s) ≈ 1/α:")
    for s_float, Z_val in results.items():
        ratio = float(Z_val / ALPHA_INV)
        if 0.01 < abs(ratio) < 100:
            print(f"    Z({s_float:.4f}) = {float(Z_val):.10f}, ratio to 1/α = {ratio:.10f}")

    return results


# =============================================================================
# SECTION 2: Searching for s where Z(s) = 1/α
# =============================================================================

def section_2_root_finding():
    """Find if Z(s) = 1/α has a solution for real s."""
    mp.dps = 30

    print("\n" + "=" * 70)
    print("SECTION 2: FINDING s WHERE Z(s) = 1/α")
    print("=" * 70)

    print("""
  Z(s) is monotonically decreasing for s > 1 (in the region of convergence).
  Z(s) → ∞ as s → 1⁺ and Z(s) → 0 as s → ∞.
  So there exists exactly one s* where Z(s*) = 1/α = 137.036...
  if 1/α is in the range of Z.
""")

    # Binary search for Z(s) = 1/α
    # First check Z at s near 1
    target = ALPHA_INV
    print("  Checking bounds:")
    for s in [mpf('1.01'), mpf('1.05'), mpf('1.1'), mpf('1.2'), mpf('1.5'), mpf(2)]:
        Z_val = epstein_zeta(s)
        print(f"    Z({float(s):.2f}) = {float(Z_val):.6f}")

    # Binary search
    s_lo, s_hi = mpf('1.001'), mpf(2)
    for _ in range(100):
        s_mid = (s_lo + s_hi) / 2
        Z_mid = epstein_zeta(s_mid)
        if Z_mid > target:
            s_lo = s_mid
        else:
            s_hi = s_mid

    s_star = (s_lo + s_hi) / 2
    Z_star = epstein_zeta(s_star)

    print(f"\n  ┌─────────────────────────────────────────────────────┐")
    print(f"  │  Z(s*) = 1/α  at  s* = {float(s_star):.15f}         │")
    print(f"  │  Verification: Z(s*) = {float(Z_star):.10f}         │")
    print(f"  │  Target:       1/α  = {float(ALPHA_INV):.10f}         │")
    print(f"  │  Error: {float(abs(Z_star - ALPHA_INV)):.3e}                          │")
    print(f"  └─────────────────────────────────────────────────────┘")

    # Can s* be expressed in terms of framework integers?
    print("\n  Is s* a recognizable number?")
    s_f = float(s_star)
    print(f"    s* = {s_f:.15f}")
    print(f"    s* - 1 = {s_f - 1:.15f}")
    print(f"    1/(s*-1) = {1/(s_f-1):.15f}")
    print(f"    π·(s*-1) = {np.pi*(s_f-1):.15f}")
    print(f"    (s*-1)·N_c = {(s_f-1)*N_c:.15f}")
    print(f"    (s*-1)·N_eff = {(s_f-1)*N_eff:.15f}")
    print(f"    (s*-1)·N_c·N_base = {(s_f-1)*N_c*N_base:.15f}")
    print(f"    1/(s*-1)/π = {1/((s_f-1)*np.pi):.15f}")

    # Check rationals with small denominators
    from fractions import Fraction
    print(f"\n  Best rational approximations to s*:")
    for max_den in [10, 50, 100, 1000, 10000]:
        frac = Fraction(s_f).limit_denominator(max_den)
        err = abs(float(frac) - s_f)
        print(f"    s* ≈ {frac} (error {err:.3e})")

    return s_star


# =============================================================================
# SECTION 3: Weighted Lattice Sums
# =============================================================================

def section_3_weighted_sums():
    """
    Test weighted lattice sums of the form:
    Z_f(s) = Σ'_{(a,b)∈ℤ²} f(a,b) / (a²+b²)^s
    with ternary-inspired weights.
    """
    mp.dps = 30

    print("\n" + "=" * 70)
    print("SECTION 3: WEIGHTED LATTICE SUMS")
    print("=" * 70)

    target = ALPHA_INV

    # Direct lattice sum computation (truncated)
    N = 100  # sum over |a|,|b| ≤ N

    def lattice_sum(s, weight_fn=None):
        """Compute Σ'_{|a|,|b|≤N} w(a,b)/(a²+b²)^s"""
        result = mpf(0)
        for a in range(-N, N+1):
            for b in range(-N, N+1):
                if a == 0 and b == 0:
                    continue
                r2 = a*a + b*b
                w = weight_fn(a, b) if weight_fn else 1
                result += mpf(w) / mpower(mpf(r2), s)
        return result

    # Weight functions to test
    weights = {
        'uniform': lambda a,b: 1,
        'ternary charge': lambda a,b: (a % 3) - 1,  # -1, 0, 1
        'parity': lambda a,b: (-1)**(a+b),
        'norm mod 3': lambda a,b: (a*a + b*b) % N_c,
        'Gaussian norm character': lambda a,b: 1 if (a*a + b*b) % 4 == 1 else -1 if (a*a + b*b) % 4 == 3 else 0,
    }

    print(f"\n  Weighted lattice sums (|a|,|b| ≤ {N}):")
    print(f"  {'Weight':>25} | {'Z_f(2)':>18} | {'Z_f(3)':>18} | {'ratio to 1/α':>15}")
    print(f"  {'-'*25}-+-{'-'*18}-+-{'-'*18}-+-{'-'*15}")

    for name, w_fn in weights.items():
        Z2 = lattice_sum(2, w_fn)
        Z3 = lattice_sum(3, w_fn)
        r2 = float(Z2/target) if float(Z2) != 0 else 0
        r3 = float(Z3/target) if float(Z3) != 0 else 0
        print(f"  {name:>25} | {float(Z2):18.10f} | {float(Z3):18.10f} | {r2:15.10f}")


# =============================================================================
# SECTION 4: Algebraic Combinations
# =============================================================================

def section_4_algebraic_combinations():
    """
    Test algebraic combinations of Z, ζ, β, π, and framework integers.
    """
    mp.dps = 30

    print("\n" + "=" * 70)
    print("SECTION 4: ALGEBRAIC COMBINATIONS")
    print("=" * 70)

    target = ALPHA_INV

    # Cache special values
    z2 = mzeta(2)    # π²/6
    z3 = mzeta(3)    # Apéry's constant
    z4 = mzeta(4)    # π⁴/90
    b1 = dirichlet_beta(1)  # π/4
    b2 = dirichlet_beta(2)  # Catalan's constant G
    b3 = dirichlet_beta(3)  # π³/32
    G_cat = b2  # Catalan's constant
    Z2 = 4 * z2 * b2  # Epstein at s=2
    Z3 = 4 * z3 * b3  # Epstein at s=3

    # Framework integers
    nc, nb, b3_fw, neff = mpf(N_c), mpf(N_base), mpf(b_3), mpf(N_eff)

    combos = {
        # Simple combinations
        "4·ζ(2)·π": 4 * z2 * mpi,
        "4·ζ(3)·π²": 4 * z3 * mpi**2,
        "ζ(2)²·π²": z2**2 * mpi**2,
        "π⁴/6": mpi**4 / 6,
        "π⁴/7": mpi**4 / b3_fw,

        # With Catalan
        "Z(2)·π": Z2 * mpi,
        "Z(2)·N_eff": Z2 * neff,
        "Z(2)²": Z2**2,
        "Z(2)·Z(3)": Z2 * Z3,

        # Framework-weighted
        "N_c·Z(2)·π": nc * Z2 * mpi,
        "N_base·Z(3)·π²": nb * Z3 * mpi**2,
        "(N_c+N_base)·ζ(3)·π²": (nc + nb) * z3 * mpi**2,
        "N_eff·ζ(2)·π": neff * z2 * mpi,

        # Deeper combinations
        "π²·(N_eff+b_3)/N_c": mpi**2 * (neff + b3_fw) / nc,
        "π⁴·N_c/(N_eff·N_base)": mpi**4 * nc / (neff * nb),
        "G·π²·N_c·N_base": G_cat * mpi**2 * nc * nb,
        "G·π³/N_c": G_cat * mpi**3 / nc,

        # Gamma-based (lemniscate connection)
        "Γ(1/4)⁴/(8π)·π·N_c": mgamma(mpf(1)/4)**4 / (8*mpi) * mpi * nc,
        "Γ(1/4)⁴/(8·N_base)": mgamma(mpf(1)/4)**4 / (8 * nb),
        "Ω²·N_eff·π": (mgamma(mpf(1)/4)**2/(2*msqrt(2*mpi)))**2 * neff * mpi,

        # With e^π (nome connection)
        "e^π·N_eff": mexp(mpi) * neff,
        "e^π·π": mexp(mpi) * mpi,
        "e^π·(N_eff-N_c)": mexp(mpi) * (neff - nc),
        "e^π·N_c·N_base": mexp(mpi) * nc * nb,
        "(e^π)²/(N_c·N_base)": mexp(mpi)**2 / (nc * nb),
        "e^(π/N_c)·N_eff²": mexp(mpi/nc) * neff**2,

        # Cross-terms
        "ζ(3)·e^π": z3 * mexp(mpi),
        "G·e^π": G_cat * mexp(mpi),
        "ζ(3)·π²·N_c": z3 * mpi**2 * nc,
        "ζ(2)·ζ(3)·N_base·N_c²": z2 * z3 * nb * nc**2,
    }

    print(f"\n  Testing {len(combos)} algebraic combinations:")
    print(f"  Target: 1/α = {float(target):.15f}")
    print()

    # Sort by closeness to target
    scored = []
    for name, val in combos.items():
        if float(val) != 0 and float(val) > 0:
            ratio = float(val / target)
            log_ratio = abs(np.log(ratio))
            scored.append((log_ratio, name, float(val), ratio))

    scored.sort()

    print(f"  {'Expression':>35} | {'Value':>20} | {'Ratio to 1/α':>15} | {'log|ratio|':>10}")
    print(f"  {'-'*35}-+-{'-'*20}-+-{'-'*15}-+-{'-'*10}")
    for log_r, name, val, ratio in scored[:25]:
        marker = " ★" if abs(ratio - 1) < 0.1 else ""
        print(f"  {name:>35} | {val:20.10f} | {ratio:15.10f} | {log_r:10.6f}{marker}")


# =============================================================================
# SECTION 5: Systematic r₁Z(s₁) + r₂Z(s₂) search
# =============================================================================

def section_5_two_term_search():
    """
    Search for r₁·Z(s₁) + r₂·Z(s₂) = 1/α with small rational coefficients.
    """
    mp.dps = 30

    print("\n" + "=" * 70)
    print("SECTION 5: TWO-TERM LATTICE SUM SEARCH")
    print("=" * 70)

    target = ALPHA_INV

    # Precompute Z at various s values
    s_values = [mpf(s)/4 for s in range(5, 25)]  # s = 1.25, 1.5, ..., 6.0
    Z_cache = {}
    for s in s_values:
        Z_cache[float(s)] = epstein_zeta(s)
        
    print(f"\n  Precomputed Z(s) at {len(s_values)} values")
    
    # Check: a·Z(s₁) + b·Z(s₂) = 1/α for small integer a, b
    # and for Z values where both terms contribute meaningfully
    
    best_hits = []
    for i, s1 in enumerate(s_values):
        Z1 = Z_cache[float(s1)]
        for s2 in s_values[i+1:]:
            Z2 = Z_cache[float(s2)]
            # For each pair, solve: a·Z1 + b·Z2 = target
            # Try b = 1..20, solve for a
            for b_coeff in range(-20, 21):
                if b_coeff == 0:
                    continue
                a_needed = float((target - b_coeff * Z2) / Z1)
                # Check if a_needed is close to a simple rational
                for denom in [1, 2, 3, 4, 6, 7, 12, 13, 47]:
                    a_int = round(a_needed * denom)
                    if abs(a_int) > 100:
                        continue
                    a_rational = mpf(a_int) / mpf(denom)
                    result = a_rational * Z1 + mpf(b_coeff) * Z2
                    err = abs(float(result - target))
                    if err < 0.01:  # within 0.01 of target
                        best_hits.append((err, a_int, denom, b_coeff, 1, float(s1), float(s2), float(result)))

    best_hits.sort()
    
    if best_hits:
        print(f"\n  Best two-term expressions (error < 0.01):")
        print(f"  {'Error':>10} | {'Expression':>40} | {'Value':>15}")
        print(f"  {'-'*10}-+-{'-'*40}-+-{'-'*15}")
        for err, a_num, a_den, b_num, b_den, s1, s2, val in best_hits[:15]:
            a_str = f"{a_num}/{a_den}" if a_den != 1 else str(a_num)
            expr = f"({a_str})·Z({s1:.2f}) + ({b_num})·Z({s2:.2f})"
            print(f"  {err:10.6f} | {expr:>40} | {val:15.10f}")
    else:
        print("\n  No two-term expressions found within error < 0.01")


# =============================================================================
# SECTION 6: Synthesis
# =============================================================================

def section_6_synthesis(s_star):
    """Synthesize all findings."""
    mp.dps = 30

    print("\n" + "=" * 70)
    print("SYNTHESIS: LATTICE SUM CONNECTION TO α")
    print("=" * 70)

    print(f"""
  ╔═══════════════════════════════════════════════════════════════════╗
  ║  RESULTS                                                         ║
  ╠═══════════════════════════════════════════════════════════════════╣
  ║                                                                   ║
  ║  1. Z(s*) = 1/α ALWAYS has a solution since Z is continuous      ║
  ║     and monotone decreasing on (1, ∞) with range (0, ∞).        ║
  ║                                                                   ║
  ║  s* = {float(s_star):.15f}                                ║
  ║                                                                   ║
  ║  2. The question is: is s* a RECOGNIZABLE number?                ║
  ║     If s* = p/q for small p,q, or s* = rational expression      ║
  ║     of framework integers, then 1/α IS a lattice sum.            ║
  ║     If s* is transcendental/unrecognizable, no luck.             ║
  ║                                                                   ║
  ║  3. Alternative: 1/α might be a LINEAR COMBINATION of Z(s)       ║
  ║     at multiple s values, or involve framework-integer weights.   ║
  ║                                                                   ║
  ╚═══════════════════════════════════════════════════════════════════╝
""")


# =============================================================================
# MAIN
# =============================================================================

def main():
    print("╔══════════════════════════════════════════════════════════════════════╗")
    print("║       PART B: LATTICE SUM DERIVATION OF ALPHA                      ║")
    print("║       Can 1/α be expressed as a sum over ℤ[i]?                     ║")
    print("╚══════════════════════════════════════════════════════════════════════╝")
    print()

    section_1_epstein_zeta()
    s_star = section_2_root_finding()
    section_3_weighted_sums()
    section_4_algebraic_combinations()
    section_5_two_term_search()
    section_6_synthesis(s_star)


if __name__ == "__main__":
    main()
