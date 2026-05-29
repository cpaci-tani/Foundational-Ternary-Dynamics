#!/usr/bin/env python3
"""
proof_odd_period_jtwisted.py

Closure attempt for the MC-T4.3 odd-period sub-problem via the J-twisted
zeta-regularized determinant (FQCR Model I).

Pre-registration : FTD-0218 (provisional)
                   docs/theory/10_eft_program/PREREG_ALPHA_READOUT_ODD_PERIOD_v1.md
                   SHA256 a5c97b7363a1e389ea5e2eff0f139a00f0bd04f8b0d21166845fefd38c53faa1
Verdict supported: UNDERDETERMINED (pre-reg §6: the J-twisted det_zeta ratio is
                   a real, FTD-native, CLEAN odd-degree G* source, but its
                   identification with the readout operator's DETERMINANT is
                   natural-yet-unforced -- a structural lead, not a closure).

WHAT THIS ESTABLISHES (the genuine progress)
--------------------------------------------
The determinant-grading no-go (FTD-0217) showed the frozen ingredients are all
EVEN G*-degree and the only even->odd route is sqrt(Watson) (a forbidden
prefactor, F4).  This attempt adds ONE candidate odd source -- the J-twisted
zeta-regularized determinant ratio (FQCR Model I, a [THEOREM]) -- and finds it
is a CLEAN degree-1 G* (no sqrt-pi prefactor; the sqrt(2pi) cancels in the
ratio).  So a forward, FTD-native, clean ODD power of G* DOES exist on the
J^2=-I structure.  This defeats the "no clean odd source" part of the no-go.

WHAT IT DOES NOT ESTABLISH (the unforced hinge, OP3)
---------------------------------------------------
The readout operator T on V_complex is FINITE (2 eigenvalues x_+, x_-); the
det_zeta ratio is a ratio of regularized determinants of two INFINITE S^1
spectra.  Numerically the det_zeta ratio is G* (~2.96), NOT the operator
determinant 16 G*^3 (~414).  Relating them requires asserting
  Det(T) = Tr(T) * (det_zeta ratio) = 16 G*^2 * G* = 16 G*^3,
i.e. the same "det = trace * G*" factorization the resolution docs assert --
now in det_zeta language.  No structural compulsion forces it (OP1/OP3).

All numerics computed (mpmath), cross-checked vs constants.py.  No CODATA value.

Run:  python scripts/proofs/proof_odd_period_jtwisted.py
"""

import os
import sys

import mpmath as mp

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
from constants import G_STAR as G_STAR_NUMPY  # noqa: E402

mp.mp.dps = 40
results = []


def check(name, passed, detail=""):
    results.append((name, bool(passed)))
    print(f"  [{'PASS' if passed else 'FAIL'}] {name}" + (f"  --  {detail}" if detail else ""))


print("=" * 78)
print("Odd-period via J-twisted det_zeta (FQCR Model I) -- verdict: UNDERDETERMINED")
print("=" * 78)

GAMMA_QUARTER = mp.gamma(mp.mpf(1) / 4)
GAMMA_3Q = mp.gamma(mp.mpf(3) / 4)
GAMMA_HALF = mp.gamma(mp.mpf(1) / 2)
G_STAR = GAMMA_QUARTER**2 / (mp.sqrt(2) * GAMMA_HALF**2)
TOL = mp.mpf(10) ** (-25)

# --- 1. FQCR Model I: det_zeta{n+a} = sqrt(2pi)/Gamma(a) (Lerch), ratio = G* --
print("\n[1] J-twisted det_zeta ratio is a CLEAN degree-1 G* (sqrt(2pi) cancels)")
det_z_14 = mp.sqrt(2 * mp.pi) / GAMMA_QUARTER     # det_zeta D_{1/4}
det_z_34 = mp.sqrt(2 * mp.pi) / GAMMA_3Q          # det_zeta D_{3/4}
ratio = det_z_34 / det_z_14                        # = Gamma(1/4)/Gamma(3/4) = G*
check("det_zeta(D_3/4)/det_zeta(D_1/4) = G* exactly", abs(ratio - G_STAR) < TOL,
      f"ratio = {mp.nstr(ratio, 12)}")
check("ratio is CLEAN (no sqrt-pi prefactor; sqrt(2pi) cancels)",
      abs(ratio - GAMMA_QUARTER / GAMMA_3Q) < TOL, "= Gamma(1/4)/Gamma(3/4), degree-1 ODD")
check("G_STAR matches constants.py", abs(G_STAR - mp.mpf(str(G_STAR_NUMPY))) < mp.mpf(10)**(-12),
      f"G* = {mp.nstr(G_STAR, 10)}")

# --- 2. The det_zeta ratio supplies the missing odd power CLEANLY ------------
print("\n[2] The odd source EXISTS: 16 G*^3 = (16 G*^2) * (det_zeta ratio)")
tr = 16 * G_STAR**2
det = 16 * G_STAR**3
check("16 G*^3 = (16 G*^2) * (det_zeta ratio)", abs(det - tr * ratio) < mp.mpf(10)**(-20),
      f"Tr={mp.nstr(tr,8)} * G*={mp.nstr(ratio,8)} = {mp.nstr(tr*ratio,10)}")
print("        => a forward, FTD-native, clean ODD power of G* exists on J^2=-I")
print("        (defeats the 'no clean odd source' part of the FTD-0217 no-go).")

# --- 3. The unforced hinge (OP3): det_zeta ratio is NOT the operator det -----
print("\n[3] BUT the hinge fails: det_zeta ratio is NOT the readout determinant")
check("det_zeta ratio (G* ~ 2.96) != operator determinant (16 G*^3 ~ 414)",
      abs(ratio - det) > 1, f"{mp.nstr(ratio,6)} vs {mp.nstr(det,6)} -- categorically different")
# T is finite (2 eigenvalues x_+, x_-); det_zeta is a ratio of INFINITE spectra.
a1 = -16 * G_STAR**2
a0 = 16 * G_STAR**3
disc = a1**2 - 4 * a0
x_plus = (-a1 + mp.sqrt(disc)) / 2
x_minus = (-a1 - mp.sqrt(disc)) / 2
# T's eigenvalues are x_+, x_- -- NOT the J-twisted spectra {n+1/4},{n+3/4}.
spec_14_first = [mp.mpf(n) + mp.mpf(1) / 4 for n in range(3)]   # 0.25, 1.25, 2.25
check("T's spectrum {x_+, x_-} is NOT the J-twisted spectrum {n+1/4}",
      min(abs(x_plus - s) for s in spec_14_first) > 1,
      f"x_+={mp.nstr(x_plus,7)}, x_-={mp.nstr(x_minus,7)} vs D_1/4={[float(s) for s in spec_14_first]}...")
print("        Relating them REQUIRES asserting Det(T) = Tr(T) * (det_zeta ratio)")
print("        = the same 'det = trace * G*' factorization (OP1/OP3) -- unforced.")

# ---------------------------------------------------------------------------
print("\n" + "=" * 78)
n_pass = sum(1 for _, p in results if p)
print(f"FACTS: {n_pass}/{len(results)} verified.")
print("PROGRESS: a clean forward odd-degree G* source exists (J-twisted det_zeta,")
print("FQCR Model I [THEOREM]) -- the parity no-go's 'no clean odd source' is lifted.")
print("REMAINING GAP: no structural compulsion makes the FINITE readout operator's")
print("determinant equal Tr * (det_zeta ratio); the factorization is asserted (OP3).")
print("VERDICT: UNDERDETERMINED. The forced det <-> det_zeta identity is the v2 target.")
print("=" * 78)
sys.exit(0 if n_pass == len(results) else 1)
