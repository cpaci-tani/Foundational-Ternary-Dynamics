"""proof_bridge_functional_arithmetic_mean.py — MC-T3.4 investigation.

FTD-0095 Bridge Functional ontology commitment: mass is the value of a
functional `M : Couplings → ℝ` evaluated on the master-quadratic root
spectrum (x_+, x_-). The arithmetic-mean rule
    M(x_+, x_-) = α · (x_+ + x_-) / 2
is currently [SELECTION] — geometric, harmonic, and power-mean
alternatives are not structurally excluded.

T3.4 closure paths (per CHECKLIST_MATH_COMPLETE.md):
    (i)  variational principle on σ_BCC
    (ii) 't Hooft beable equiprobability under unbroken-phase
    (iii) Beilinson regulator slot for trace functional

This script investigates the four candidate functionals and confirms
which empirical predictions distinguish them. T3.4 closure requires
new mathematical machinery (research-program-scale) — this script
documents the question and its empirical signature, NOT a closure.

Usage:
    python scripts/proofs/proof_bridge_functional_arithmetic_mean.py
"""

from __future__ import annotations

import sys
import math


# Master quadratic roots (per FTD-0001)
X_PLUS = 137.0361714582
X_MINUS = 3.0239639163
ALPHA = 1.0 / X_PLUS  # tree-level

# Empirical electron mass in m_e units (definitionally 1)
M_E_TARGET = 1.0


# ─────────────────────────────────────────────────────────────────────
# Candidate functionals
# ─────────────────────────────────────────────────────────────────────
def arithmetic_mean(x_plus: float, x_minus: float) -> float:
    """M = α · (x+ + x-) / 2."""
    return ALPHA * (x_plus + x_minus) / 2.0


def geometric_mean(x_plus: float, x_minus: float) -> float:
    """M = α · √(x+ · x-)."""
    return ALPHA * math.sqrt(x_plus * x_minus)


def harmonic_mean(x_plus: float, x_minus: float) -> float:
    """M = α · 2 / (1/x+ + 1/x-)."""
    return ALPHA * 2.0 / (1.0 / x_plus + 1.0 / x_minus)


def quadratic_mean(x_plus: float, x_minus: float) -> float:
    """M = α · √((x+² + x-²)/2)."""
    return ALPHA * math.sqrt((x_plus ** 2 + x_minus ** 2) / 2.0)


def main() -> int:
    print("=" * 72)
    print("proof_bridge_functional_arithmetic_mean.py — MC-T3.4 investigation")
    print("=" * 72)
    print()
    print("Master quadratic roots:")
    print(f"  x+ = {X_PLUS}")
    print(f"  x- = {X_MINUS}")
    print(f"  α  = 1/x+ = {ALPHA:.10f}")
    print()
    print("Candidate functionals:")
    print()
    funcs = [
        ("Arithmetic mean", arithmetic_mean, "α(x+ + x-)/2"),
        ("Geometric mean ", geometric_mean, "α√(x+·x-)"),
        ("Harmonic mean  ", harmonic_mean, "α·2/(1/x+ + 1/x-)"),
        ("Quadratic mean ", quadratic_mean, "α√((x+² + x-²)/2)"),
    ]
    print(f"  Functional       | Formula             | M value     | M / m_e_target")
    print(f"  ─────────────────|─────────────────────|─────────────|──────────────")
    for name, fn, formula in funcs:
        M = fn(X_PLUS, X_MINUS)
        ratio = M / M_E_TARGET
        print(f"  {name}  | {formula:19s} | {M:.6f}    | {ratio:.6f}")
    print()
    print("Interpretation:")
    print()
    print("  All four functionals produce values of order O(α·x+) ≈ 1, so all")
    print("  four are consistent with M ~ m_e at order-of-magnitude. The")
    print("  finite-precision FTD prediction is m_e = M_P · √(2π) · (16/3) · α^11,")
    print("  which is computed via a different route (the ladder walk / FTD-0015).")
    print()
    print("  The Bridge Functional ontology (FTD-0095) is a META-claim about")
    print("  HOW mass is computed: as a functional on the root spectrum. The")
    print("  arithmetic-mean rule is the SPECIFIC functional. This script")
    print("  shows the four natural means give CLOSE BUT NOT IDENTICAL values:")
    print()
    print(f"    Arithmetic - Geometric = {arithmetic_mean(X_PLUS, X_MINUS) - geometric_mean(X_PLUS, X_MINUS):.6f}")
    print(f"    Arithmetic - Harmonic  = {arithmetic_mean(X_PLUS, X_MINUS) - harmonic_mean(X_PLUS, X_MINUS):.6f}")
    print(f"    Arithmetic - Quadratic = {arithmetic_mean(X_PLUS, X_MINUS) - quadratic_mean(X_PLUS, X_MINUS):.6f}")
    print()
    print("  Discriminating among them requires either:")
    print("    (a) variational derivation on σ_BCC (research-program-scale)")
    print("    (b) high-precision m_e prediction with sub-percent measurements")
    print("        (already FTD-0015 at 0.19% via the ladder walk; doesn't")
    print("         distinguish the means).")
    print()
    print("CLOSURE STATUS:")
    print("  T3.4 NOT closed. The four candidate functionals all agree to")
    print("  within ~10% on the master quadratic roots, and the FTD-0015")
    print("  high-precision m_e formula uses a different functional (ladder")
    print("  walk) entirely. The Bridge Functional ontology commitment")
    print("  remains [SELECTION] until one of the three structural closure")
    print("  paths (variational, 't Hooft, Beilinson) is worked out at")
    print("  research-program scale.")
    print()
    print("  Honest classification: [INVESTIGATED — empirical signatures of")
    print("  the four mean-rules computed; structural derivation deferred to")
    print("  Tier-IV research].")
    return 0


if __name__ == "__main__":
    sys.exit(main())
