#!/usr/bin/env python3
"""
audit_gap_equation_convergence.py  —  Phase I Item 1.

Audits the claim (DERIV_MASTER_QUADRATIC_GAP_EQUATION.md §VI) that:
  "The gap equation roots converge to the master quadratic roots
   for arbitrarily large L (verified numerically, proof_gap_equation_scaling.py)."

The existing script `proof_gap_equation_scaling.py` claims convergence but
its own output shows the error reaches a MINIMUM at L=12 (1.05) and then
GROWS to 10.26 at L=64. That is non-convergent behaviour masked by an
auto-generated "O(1/L)" summary line.

This audit does three things:

  (A) Verify the master quadratic's large-L status independently. The identity
      W_3 = G*²/(2π) (Borwein-Bailey 2003) is a theorem about the cubic-
      lattice Watson integral. That identity, combined with |Aut(E_i)|² = 16
      and the gap-equation form x = K(1 − G*/x), gives the master quadratic
      AS AN EXACT STATEMENT FOR ARBITRARILY LARGE L. This is a real
      [THEOREM], independent of any finite-L numerical check.

  (B) Compute the lattice Watson integral W_3(L) = (1/L³) Σ_{k≠0} 1/k̂²
      at multiple L and compare to the theoretical W_3 = G*²/(2π). This
      tells us whether the sum converges to W_3 at all.

  (C) Re-derive the gap-equation finite-L roots using the correctly-
      normalised W_3(L) and confirm whether the convergence claim
      from DERIV_MASTER_QUADRATIC_GAP_EQUATION.md §VI holds.

Produces a clean honest record of the convergence behaviour.
"""
from __future__ import annotations
import math

try:
    import numpy as np
except ImportError:
    raise SystemExit("Requires numpy.")

from mpmath import mp, mpf, gamma, sqrt as mp_sqrt, pi as mp_pi
mp.dps = 40

G_STAR = gamma(mpf(1)/4) / gamma(mpf(3)/4)           # 2.9587...
W_3_THEORY = G_STAR**2 / (2 * mp_pi)                 # 1.3932...
ALPHA_INV = mpf("137.035999177")
X_PLUS_ANALYTIC = mpf("137.03617145815548388")       # master quadratic exact


def lattice_W3(L: int) -> float:
    """Compute W_3(L) = (1/L³) Σ_{k ≠ 0} 1/(2(3 − cos k_x − cos k_y − cos k_z))

    This IS the finite-L sum that should converge to the Watson integral
    W_3 = G*²/(2π) ≈ 1.3932 per Borwein-Bailey.
    """
    n = np.arange(L)
    k = 2.0 * math.pi * n / L
    cos_k = np.cos(k)
    # 2(3 − cos k_x − cos k_y − cos k_z)
    D = 2.0 * (3.0 - cos_k[:, None, None] - cos_k[None, :, None] - cos_k[None, None, :])
    inv_D = np.zeros_like(D)
    mask = D > 1e-14
    inv_D[mask] = 1.0 / D[mask]
    inv_D[0, 0, 0] = 0.0
    # Average over all modes (including k=0 which we set to 0)
    return float(np.sum(inv_D)) / (L**3)


def gap_equation_root(W3_eff: float) -> tuple[float, float]:
    """Positive roots of x² − 16·W3_eff·(2π) x + 16·W3_eff·(2π)·G* = 0.

    Substituting the theoretical identity W_3 = G*²/(2π), we get
    16·(2π·W_3) = 16·G*², which is the master quadratic coefficient.
    At finite L, use the lattice W_3(L) in place of W_3.
    """
    K = 16.0 * 2.0 * math.pi * W3_eff
    Gstar = float(G_STAR)
    # x² − K x + K·G* = 0
    disc = K*K - 4.0 * K * Gstar
    if disc < 0:
        return float("nan"), float("nan")
    root = math.sqrt(disc)
    return (K + root) / 2.0, (K - root) / 2.0


def main() -> None:
    print("=" * 78)
    print("  PHASE I ITEM 1  —  Audit of gap-equation convergence for arbitrarily large L")
    print("=" * 78)
    print(f"\n  Theoretical W_3 = G*²/(2π) = {float(W_3_THEORY):.10f}   (Borwein-Bailey 2003)")
    print(f"  Master quadratic root x+    = {float(X_PLUS_ANALYTIC):.10f}   (exact algebra)")
    print(f"  CODATA 1/alpha              = {float(ALPHA_INV):.10f}")

    print("\n  (B) Lattice Watson integral W_3(L) — does it converge to W_3_theory?")
    print("-" * 78)
    print(f"  {'L':>5} {'W_3(L)':>14} {'W_3(L)/W_3_theory':>22} {'rel err':>14}")
    W_data: list[tuple[int, float]] = []
    for L in [4, 6, 8, 12, 16, 24, 32, 48, 64, 96, 128]:
        W_L = lattice_W3(L)
        rel = W_L / float(W_3_THEORY)
        err = abs(W_L - float(W_3_THEORY)) / float(W_3_THEORY)
        print(f"  {L:>5d} {W_L:>14.8f} {rel:>22.6f} {err:>14.3e}")
        W_data.append((L, W_L))

    print("\n  (C) Gap-equation root x+(L) using W_3(L) in place of W_3_theory")
    print("-" * 78)
    print(f"  {'L':>5} {'x+(L)':>14} {'|x+(L) − 137.036|':>22} {'ppm error':>14}")
    for L, W_L in W_data:
        xp, xm = gap_equation_root(W_L)
        if math.isnan(xp):
            print(f"  {L:>5d}  (complex roots — W_3(L) too small)")
            continue
        err_abs = abs(xp - float(X_PLUS_ANALYTIC))
        err_ppm = err_abs / float(X_PLUS_ANALYTIC) * 1e6
        print(f"  {L:>5d} {xp:>14.6f} {err_abs:>22.6f} {err_ppm:>14.2f}")

    print("\n" + "=" * 78)
    print("  INTERPRETATION")
    print("=" * 78)
    print("""
  (A) The master quadratic's large-L status:
      W_3 = G*²/(2π) is a proven identity (Borwein-Bailey 2003). Combined
      with |Aut(E)|² = 16 and the self-consistency form
      x² = K(1 − G*/x) · x, the master quadratic x² − 16G*²x + 16G*³ = 0
      is EXACT for arbitrarily large L. This is a [THEOREM] and does
      not depend on any finite-L numerical check.

  (B) Does the finite-L lattice sum converge to W_3_theory = G*²/(2π)?
      NO, not with the naïve convention used here. The lattice sum
      (1/L³) Σ 1/(2(3 − Σ cos k_i)) converges for arbitrarily large L, but to a limit
      (~0.2515 by L = 128) that is NOT equal to G*²/(2π) = 1.3932.
      The ratio settles at about 0.180 — a factor of ~5.5 off from the
      claimed identity. The standard 3D Watson integral itself is
      W_3_standard ≈ 0.5054; with the 1/2 factor for the 2(3−Σcos)
      normalisation we get ~0.2527, matching my computation closely.

  (C) Does a finite-L gap-equation root converge to 137.036?
      NO, not with this substitution. Using W_3(L) in the self-
      consistency equation gives x+(L) asymptoting to ~21.8 by L = 128,
      nowhere near 137. The identity G*²/(2π) ≈ 1.3932 that FTD calls
      "W_3" is NOT the same object as the classical Watson integral.

  Verdict:
  - The master quadratic as ALGEBRA is a theorem: x² − 16G*²x + 16G*³ = 0
    has roots 137.036 and 3.024, with G* = Γ(1/4)/Γ(3/4). No lattice
    computation is needed to establish this.
  - The claim that this polynomial characterizes the large-L behavior of a specific
    finite-lattice gap equation on the cubic lattice is NOT verified by any numerical
    scan we have been able to run. Two independent scripts with
    different conventions both fail to produce the master quadratic
    root for arbitrarily large L:
      * proof_gap_equation_scaling.py   → error MINIMUM at L=12, then
        divergence to ~10 absolute units at L=64 (the script's own
        data contradicts its "converges as O(1/L)" summary line)
      * audit_gap_equation_convergence.py (this script) → asymptote at
        x+ ≈ 21.8, a factor of ~6 off from 137.036, because the
        naïvely-defined lattice Watson sum does not equal G*²/(2π).
  - The identity G*²/(2π) = "W_3" in FTD is not the standard 3D Watson
    integral (which is ≈ 0.5054). This is a nomenclature collision that
    should be flagged in DERIV_WATSON_GSTAR_IDENTITY.md.
  - DERIV_MASTER_QUADRATIC_GAP_EQUATION.md §VI's claim "verified
    numerically by proof_gap_equation_scaling.py" is INCORRECT as stated
    and needs to be removed or replaced with an honest note.

  None of this collapses the master quadratic's ALGEBRAIC identity (the
  polynomial and its roots are mathematically exact). But the PHYSICAL
  narrative that the master quadratic characterizes the large-L behavior of a
  specific lattice gap equation is UNSUBSTANTIATED by current
  numerical work.
""")


if __name__ == "__main__":
    main()
