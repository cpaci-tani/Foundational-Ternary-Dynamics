#!/usr/bin/env python3
"""
proof_det_identity.py

Decisive attempt for the MC-T4.3 det<->det_zeta structural-identity hinge.

Pre-registration : FTD-0219 (provisional)
                   docs/theory/10_eft_program/PREREG_ALPHA_READOUT_DET_IDENTITY_v1.md
                   SHA256 03b967c760fa38fffa8c7d08d5a75c34392dcd2c4c546f24a9c58b4d97a78122
Verdict supported: CLOSED-NEGATIVE for the det<->det_zeta identity (pre-reg §6:
                   sub-test A is provably not realized).

THE HINGE (pre-reg §2): is the readout operator's determinant FORCED to be the
J-twisted zeta-regularized determinant (-> 16 G*^3), or is "Det = Tr * G*"
merely asserted?  FOUND needs A (det = det_zeta, derived) AND B (consistent
symmetry-breaking) AND C (Tr, Det jointly forced).

FINDINGS
--------
A (det<->det_zeta) -- FAILS (V1):
  * The master-quadratic determinant 16 G*^3 = x_+ * x_-  is an ORDINARY finite
    product of the TWO roots (constant term of a degree-2 polynomial). It is not
    a zeta-regularized determinant of anything.
  * An INFINITE operator carrying the J-twisted spectrum {n+1/4}/{n+3/4} has
    det_zeta ratio = G* (degree 1), NOT 16 G*^3 (degree 3).
  * So no single operator's zeta-regularized determinant equals 16 G*^3; the
    "det <-> det_zeta" identity is not realized.

C (joint forcing) -- FAILS (V7):
  * For a 2x2 operator, trace and determinant are INDEPENDENT invariants: fixing
    Tr = 16 G*^2 leaves Det entirely free. So "Det = 16 G*^3" is NOT forced by
    the 2x2 structure -- it is the target relation (Vieta of the master
    quadratic, FTD-0001), i.e. inserted, not compelled.

The 3-plane assembly 16 G*^3 = |mu_4|^2 * (det_zeta ratio)^3 holds NUMERICALLY,
but it is a product of THREE SEPARATE det_zeta ratios times the unit count --
not one operator's det_zeta -- and carries a trace/det G*-degree asymmetry (2 vs
3) a symmetric 3-plane tensor product would not produce.

All numerics computed (mpmath/sympy), cross-checked vs constants.py. No CODATA.

Run:  python scripts/proofs/proof_det_identity.py
"""

import os
import sys

import mpmath as mp
import sympy as sp

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
from constants import G_STAR as G_STAR_NUMPY  # noqa: E402

mp.mp.dps = 40
results = []


def check(name, passed, detail=""):
    results.append((name, bool(passed)))
    print(f"  [{'PASS' if passed else 'FAIL'}] {name}" + (f"  --  {detail}" if detail else ""))


print("=" * 78)
print("det<->det_zeta identity hinge -- verdict: CLOSED-NEGATIVE (A and C fail)")
print("=" * 78)

GAMMA_Q = mp.gamma(mp.mpf(1) / 4)
GAMMA_3Q = mp.gamma(mp.mpf(3) / 4)
GAMMA_H = mp.gamma(mp.mpf(1) / 2)
G_STAR = GAMMA_Q**2 / (mp.sqrt(2) * GAMMA_H**2)
TOL = mp.mpf(10) ** (-22)

# roots of the master quadratic
a1, a0 = -16 * G_STAR**2, 16 * G_STAR**3
disc = a1**2 - 4 * a0
x_plus = (-a1 + mp.sqrt(disc)) / 2
x_minus = (-a1 - mp.sqrt(disc)) / 2

# --- A: the determinant is an ORDINARY product, not a det_zeta --------------
print("\n[A] det<->det_zeta identity: is Det a zeta-regularized determinant?")
check("Det = 16 G*^3 = x_+ * x_-  (ordinary finite product of 2 roots)",
      abs(x_plus * x_minus - 16 * G_STAR**3) < TOL,
      f"x_+*x_- = {mp.nstr(x_plus*x_minus, 10)}")
det_zeta_ratio = (mp.sqrt(2 * mp.pi) / GAMMA_3Q) / (mp.sqrt(2 * mp.pi) / GAMMA_Q)  # = G*
check("J-twisted det_zeta ratio = G* (degree 1)  !=  16 G*^3 (degree 3)",
      abs(det_zeta_ratio - G_STAR) < TOL and abs(det_zeta_ratio - 16 * G_STAR**3) > 1,
      f"det_zeta ratio = {mp.nstr(det_zeta_ratio,8)} ; 16 G*^3 = {mp.nstr(16*G_STAR**3,8)}")
print("        => no single operator's zeta-reg determinant is 16 G*^3.")
print("        A FAILS (V1): the determinant is an ordinary product, not a det_zeta.")

# --- C: for a 2x2, Tr and Det are INDEPENDENT (Det = 16 G*^3 not forced) ----
print("\n[C] 2x2 invariants: is Det = 16 G*^3 forced once Tr = 16 G*^2 is fixed?")
G, p, q, r = sp.symbols("G p q r", real=True)
# General 2x2 with trace fixed to 16 G^2: [[p, q],[r, 16 G^2 - p]].
M = sp.Matrix([[p, q], [r, 16 * G**2 - p]])
trM = sp.expand(M.trace())
detM = sp.expand(M.det())                       # = p(16G^2 - p) - q r : a FREE function of p,q,r
check("trace fixed = 16 G^2 by construction", sp.simplify(trM - 16 * G**2) == 0)
# Determinant depends on free entries p,q,r -> not pinned to 16 G^3.
det_depends_free = any(detM.diff(v) != 0 for v in (p, q, r))
check("det(M) varies with free entries (Tr, Det independent)", det_depends_free,
      f"det(M) = {detM}  (free in p,q,r)")
# Concretely: two different 2x2's, same trace 16 G^2, different determinants.
M1 = M.subs({p: 8 * G**2, q: 0, r: 0})          # det = 64 G^4
M2 = M.subs({p: 8 * G**2, q: 1, r: 1})          # det = 64 G^4 - 1
check("same trace, different determinants exist", sp.simplify(M1.det() - M2.det()) != 0,
      f"det1={sp.expand(M1.det())}, det2={sp.expand(M2.det())}")
print("        C FAILS (V7): Det = 16 G*^3 is the master-quadratic TARGET (Vieta),")
print("        not forced by the operator being 2x2 -- it is inserted, not compelled.")

# --- the 3-plane assembly: holds numerically but is not one operator's det_zeta
print("\n[note] 3-plane assembly 16 G*^3 = |mu_4|^2 * (det_zeta ratio)^3")
check("16 * (det_zeta ratio)^3 == 16 G*^3 (numerically assembles)",
      abs(16 * det_zeta_ratio**3 - 16 * G_STAR**3) < TOL,
      "but = product of THREE separate ratios + unit count, not one operator's det_zeta")
check("G_STAR matches constants.py", abs(G_STAR - mp.mpf(str(G_STAR_NUMPY))) < mp.mpf(10)**(-12))

print("\n" + "=" * 78)
n_pass = sum(1 for _, p_ in results if p_)
print(f"FACTS: {n_pass}/{len(results)} verified.")
print("A fails: the master-quadratic determinant 16 G*^3 = x_+ x_- is an ordinary")
print("product, NOT a zeta-regularized determinant; the J-twisted det_zeta ratio is")
print("G* (degree 1). C fails: Tr and Det of a 2x2 are independent, so Det=16 G*^3 is")
print("the inserted target, not forced. The det<->det_zeta identity is not realized.")
print("VERDICT: CLOSED-NEGATIVE -- the BCC/quantization observable readout route is")
print("exhausted; MC-T4.3's surviving space is ARC-D / a new postulate.")
print("=" * 78)
sys.exit(0 if n_pass == len(results) else 1)
