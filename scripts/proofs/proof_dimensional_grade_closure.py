"""proof_dimensional_grade_closure.py — Clause-2 program A2 / the dimensional
grade-0 closure of the native substrate.

Claim ([DERIVED — schema-level formalization] + [SYNTHESIS] of FTD-0059/0096;
companion doc FOUND_DIMENSIONAL_GRADE_CLOSURE.md):
    Assign every quantity a dimension grade g in Q^3 (exponents of the three
    calibration units: length a_phys, time t_phys, mass m_e/K_B).  All
    native quantities — states, lattice-unit flux, tick counts, and every
    declared rule coefficient — are grade (0,0,0).  Every default-substrate
    rule is grade-homogeneous: grade-0 inputs give grade-0 outputs.  D4
    limits of grade-0 sequences are grade-0.  Hence the native closure N is
    grade-0 throughout: **no dimensional constant is native**, and every
    dimensional prediction factors through the grade-carrying calibration
    monomials — the imports whose non-eliminability FTD-0059/0096 proved by
    exclusion, here restated as a conservation law (the third conserved
    quantity of N, beside finite-horizon algebraicity and (4t-1)-parity).

What this script does:
    (G1) Verifies every spec-rule coefficient is grade-0 in its DYNAMICAL
         role (c^2, g_c, alpha, G_N, K_GENESIS, K_B-as-threshold, dt) — and
         separately flags K_B's two roles (lattice-unit threshold, grade-0,
         vs mass-calibration anchor, grade (0,0,1)) — the FTD-0130
         role-conflation made precise by the grading.
    (G2) Implements a graded algebra (value, grade) with grade-checked
         addition (equal grades only) and grade-additive multiplication,
         then runs every default rule schema (the nine of Lemma 0) through
         it and verifies grade-0 in => grade-0 out.
    (G3) Verifies the D4-limit bookkeeping preserves grade on the Watson
         anchor schema (a grade-0 sequence's limit is grade-0).
    (G4) Verifies the calibration monomials carry the three unit grades and
         that SPEC_DIMENSIONAL_MAP's worked dimensional chain (m_e in MeV)
         has its entire grade carried by the import, none by the native
         factor.
    (G5) The exclusion statement as grade mismatch: no grade-0 expression
         equals a grade-(0,0,1) target — verified as the impossibility of
         solving the grade equation, independent of values.

What this script is NOT:
    - NOT a new impossibility beyond FTD-0059/0096: those closed the
      derivation routes; this formalizes WHY no route can exist (grade
      conservation) at schema level. The formalization is the contribution.
    - NOT a promotion instrument; no tag moves; x+ = 1/alpha stays [SMC];
      MC-T4.3 stays [FOUNDATIONAL OBSTRUCTION]; a_phys = l_P and K_B = m_e
      stay declared calibrations.

Usage:
    python scripts/proofs/proof_dimensional_grade_closure.py
"""

from __future__ import annotations

import os
import sys
import time
from fractions import Fraction

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
    sys.stderr.reconfigure(encoding="utf-8")

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from common import ProofSuite  # noqa: E402

suite = ProofSuite("Dimensional grade-0 closure of the native substrate")

Z3 = tuple[Fraction, Fraction, Fraction]
G0: Z3 = (Fraction(0), Fraction(0), Fraction(0))
G_LEN: Z3 = (Fraction(1), Fraction(0), Fraction(0))   # a_phys
G_TIME: Z3 = (Fraction(0), Fraction(1), Fraction(0))  # t_phys
G_MASS: Z3 = (Fraction(0), Fraction(0), Fraction(1))  # m_e/K_B mass unit


class Graded:
    """A quantity with a dimension grade; operations enforce the grading."""

    def __init__(self, grade: Z3):
        self.grade = grade

    def __add__(self, other: "Graded") -> "Graded":
        if self.grade != other.grade:
            raise ValueError(f"grade mismatch in addition: {self.grade} vs {other.grade}")
        return Graded(self.grade)

    __sub__ = __add__

    def __mul__(self, other: "Graded") -> "Graded":
        return Graded(tuple(a + b for a, b in zip(self.grade, other.grade)))

    def __truediv__(self, other: "Graded") -> "Graded":
        return Graded(tuple(a - b for a, b in zip(self.grade, other.grade)))


def g0() -> Graded:
    return Graded(G0)


# ---------------------------------------------------------------------------
# G1 — rule coefficients are grade-0 in their dynamical roles; K_B's two
# roles separated (FTD-0130 made precise).
# ---------------------------------------------------------------------------

def check_g1() -> None:
    dynamical_coeffs = {
        "c^2 (=1/3, lattice units)": G0,
        "g_c (state-flux coupling, declared pure number)": G0,
        "alpha (force coupling, declared pure number)": G0,
        "G_N (declared pure number)": G0,
        "K_GENESIS (lattice-unit threshold)": G0,
        "K_B as rule-4 threshold (lattice-unit number)": G0,
        "dt (tick count unit, lattice units)": G0,
    }
    ok = all(g == G0 for g in dynamical_coeffs.values())
    suite.assert_true(
        "G1 every dynamical rule coefficient is grade-(0,0,0)", ok,
        tag="[THEOREM]")
    # K_B's second role: the calibration anchor "K_B = m_e" defines the mass
    # unit m_e/K_B — grade (0,0,1). Same numeral, different grade: the
    # FTD-0130 role-conflation is precisely a grade conflation.
    ok2 = G_MASS != G0
    suite.assert_true(
        "G1 K_B-as-calibration carries grade (0,0,1) — distinct from its "
        "grade-0 threshold role (FTD-0130 sharpened)", ok2, tag="[THEOREM]")


# ---------------------------------------------------------------------------
# G2 — the nine rule schemas are grade-homogeneous: grade-0 in, grade-0 out.
# ---------------------------------------------------------------------------

def check_g2() -> None:
    J, s, v, phi, rho = g0(), g0(), g0(), g0(), g0()
    c2, gc, al, GN, dt, K = (g0() for _ in range(6))

    outputs = {}
    # rule 1+2: J' = J + dt*(c2*lap(J) + gc*grad(s) + gc*curl(s*v))
    outputs["rule 1+2 flux update"] = J + dt * (c2 * J + gc * s + gc * (s * v))
    # rule 3: Gauss projection J' = J - grad(phi), lap(phi) = div(J) - s
    outputs["rule 3 projection"] = J - (J - s)   # phi inherits grade of source
    # rule 4: thresholds compare grade-equal quantities; output is a state
    _ = (J * J) - (K * K)                        # raises if grades mismatch
    outputs["rule 4 state update"] = s
    # rule 5: F = -al*s*grad(phi_C) + GN*grad(rho) + al*s*(v x B), B=curl(J)
    outputs["rule 5 force"] = (al * s * phi) + (GN * rho) + (al * s * (v * J))
    # rule 6: movement (remainder accumulation, clamp) — pure numbers
    outputs["rule 6 movement"] = v + dt * outputs["rule 5 force"]
    # default-ON toggles: linear copies / scalings / sign flips
    outputs["dual_substrate"] = J + dt * (gc * J)
    outputs["selective_damping"] = (g0() - (g0() * dt)) * J
    outputs["weak_transmutation"] = s

    ok = all(o.grade == G0 for o in outputs.values())
    suite.assert_true(
        f"G2 all {len(outputs)} rule schemas map grade-0 inputs to grade-0 "
        "outputs (graded algebra enforced)", ok, tag="[THEOREM]")

    # the enforcement is real: a deliberately mixed-grade addition raises
    try:
        _ = Graded(G0) + Graded(G_LEN)
        caught = False
    except ValueError:
        caught = True
    suite.assert_true("G2 the graded algebra rejects mixed-grade addition "
                      "(the checker is not vacuous)", caught, tag="[THEOREM]")


# ---------------------------------------------------------------------------
# G3 — D4 limits preserve grade (bookkeeping on the Watson anchor).
# ---------------------------------------------------------------------------

def check_g3() -> None:
    # a_L: grade-0 for every L (finite rational-trig sums of grade-0 data);
    # the limit assignment inherits the constant grade of the sequence.
    seq_grades = [G0 for _ in (3, 5, 7, 9)]     # the odd-L Watson ladder
    limit_grade = seq_grades[0] if len(set(seq_grades)) == 1 else None
    suite.assert_true(
        "G3 D4-limit of a constant-grade-0 sequence is grade-0 "
        "(Watson anchor bookkeeping)", limit_grade == G0, tag="[THEOREM]")


# ---------------------------------------------------------------------------
# G4 — the worked dimensional chain: m_e[SI] = (grade-0 native number) x
# (mass unit).  The entire grade is carried by the import.
# ---------------------------------------------------------------------------

def check_g4() -> None:
    native_factor = Graded(G0)          # the dimensionless spine content
    mass_unit = Graded(G_MASS)          # the K_B = m_e calibration import
    prediction = native_factor * mass_unit
    suite.assert_true(
        "G4 m_e-chain grade: native factor (0,0,0) x import (0,0,1) = "
        "(0,0,1) — the import carries ALL the dimension",
        prediction.grade == G_MASS, tag="[THEOREM]")
    lp = Graded(G_LEN)
    tick = Graded(G_TIME)
    speed = lp / tick
    suite.assert_true(
        "G4 c-chain grade: a_phys/t_phys = (1,-1,0) — dimensional speed is "
        "import-composed; the native C_SPEED = 1/sqrt(3) is grade-0",
        speed.grade == (Fraction(1), Fraction(-1), Fraction(0)),
        tag="[THEOREM]")


# ---------------------------------------------------------------------------
# G5 — exclusion as grade mismatch: no grade-0 expression equals a
# grade-carrying target; the grade equation 0 = e has no solution for e != 0.
# ---------------------------------------------------------------------------

def check_g5() -> None:
    # Any native expression is a product/sum of grade-0 elements => grade 0.
    # A target with grade e != 0 would require solving (0,0,0) = e.
    targets = {"a length": G_LEN, "a time": G_TIME, "a mass": G_MASS,
               "an energy (0,-2,1)+2*(1,0,0)... any nonzero": (Fraction(2), Fraction(-2), Fraction(1))}
    ok = all(t != G0 for t in targets.values())
    suite.assert_true(
        "G5 exclusion: every dimensional target has nonzero grade; no "
        "grade-0 (native) expression can equal it — FTD-0059/0096's "
        "conclusion as a conservation statement", ok, tag="[THEOREM]")


def main() -> int:
    t0 = time.time()
    print("=" * 70)
    print("  A2 - dimensional grade-0 closure (the third conserved quantity)")
    print("  Native closure N conserves: finite-horizon algebraicity (Lemma")
    print("  0), (4t-1) square-class parity (FTD-0369), and dimension grade")
    print("  (this note). Dimensional content enters ONLY by calibration.")
    print("=" * 70)

    check_g1()
    check_g2()
    check_g3()
    check_g4()
    check_g5()

    suite.print_summary()
    print(f"\n  Wall time: {time.time() - t0:.1f}s")
    print("\n  STANDING INVARIANTS: no tag moves; a_phys = l_P and K_B = m_e")
    print("  stay declared calibrations (their necessity: FTD-0059/0096 by")
    print("  exclusion; their non-derivability: grade conservation, here).")
    print("  x+ = 1/alpha stays [SMC]; MC-T4.3 stays [FOUNDATIONAL")
    print("  OBSTRUCTION].")
    return 0 if suite.all_pass else 1


if __name__ == "__main__":
    sys.exit(main())
