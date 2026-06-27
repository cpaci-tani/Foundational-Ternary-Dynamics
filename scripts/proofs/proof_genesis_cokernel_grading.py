#!/usr/bin/env python3
"""
proof_genesis_cokernel_grading.py  (v3 -- post red-team round 2)
===============================================================
FROZEN INSTRUMENT for PREREG_GENESIS_COKERNEL_GRADING_v1 (MC-T4.3 cokernel crack).

Design doc: docs/theory/02_foundations/EXPLR_GENESIS_COKERNEL_GRADED_SQRT.md
Pre-reg:    docs/theory/02_foundations/PREREG_GENESIS_COKERNEL_GRADING_v1.md

Classifies a candidate grading `g` (the section-INVARIANT arithmetic content of the genesis
disintegration fiber, an algebraic number to high precision -- fed WITHOUT modulus-normalization)
by its Galois position over the FIELD Q(G*):
    OUTCOME A  -- Q(G*)(g) carries delta = sqrt(G*(4G*-1)): g lies in Q(G*,i,delta) with delta PRESENT.
                  Re-sources delta. Does NOT derive alpha.
    OUTCOME B  -- g in Q(G*,i) (delta absent: real Q(G*), Gaussian, or cyclotomic). A 6th FORCED Z/2;
                  HARDENS the wall (extends K-BIND/FTD-0326 to the cokernel).
    UNDERDETERMINED -- no single well-defined g (G2), g section-dependent (G3), construction not
                  well-posed (G5), non-unanimous over the dps band (G4), or g in another extension
                  (e.g. zeta_8*delta, sqrt(2)*delta -- genuinely outside Q(G*,i,delta)).

------------------------------------------------------------------------------------------------
CHANGELOG (red-team fixes that MUST be in place before the hash-lock):
  v2 [BLOCKER-1a complex blind spot]  v1 saw only a REAL Q(G*)-multiple of delta; v2 added the
      {1,i,delta,i*delta} CM-field decomposition so i*delta, (1+i)*delta, (G*+i)*delta -> A.
  v3 [BLOCKER-1b field-vs-polynomial]  v2 tested the bounded-degree POLYNOMIAL ring Q[G*]_{<=d}, not
      the FIELD Q(G*) (= rational functions) the verdict map claims. Because G* is transcendental,
      Q[G*] is a PROPER subring of Q(G*), so any grading with a G* DENOMINATOR (1/G*, delta/G*,
      delta/(4G*-1) -- the GENERIC output of the sec.2 Moore-Penrose/Hodge complement) was wrongly
      UNDERDETERMINED. v3 tests genuine FIELD membership by CLEARING DENOMINATORS: PSLQ over
      {x*G*^j} jointly with {G*^k} (and {delta*G*^k}), so x = (poly in G*)/(poly in G*) is detected.
      delta/G*, delta/(4G*-1), 1/delta=(1/C)delta -> A; 1/G*, i/G* -> B (verified in self-tests).
      MAXDEG raised 8->12 for rational-function headroom; the post-lock attempt must verify
      degree-stability (raise MAXDEG and confirm the verdict is unchanged).
  v3 [disclosure correction]  UNDERDETERMINED now means genuinely OUTSIDE Q(G*,i,delta) (sqrt(2),
      zeta_8) -- NOT an in-field rational-coefficient miss (those are now correctly A/B).
  [G3 section-invariance / import-separation / quarantine]  These are CONSTRUCTION OBLIGATIONS the
      instrument cannot mechanically check from a single number; the post-lock attempt must EXHIBIT
      the section-invariance proof and the import-separation, and that proof is itself red-teamed.
      The pre-reg MANDATES them (reviewer-verified); the frozen artifact does NOT self-enforce them.

CEILING (verbatim): even OUTCOME A does NOT derive alpha. Re-sourcing delta supplies only the
root-DISTINGUISHING surd; it does NOT assemble the master-quadratic operator (MC-T4.3, untouched) nor
identify x+ with 1/alpha (FTD-0013, [SMC]). FC-W-conditional is the CEILING of Outcome A.

Discriminator protocol = the FTD-0244/0326/0327 PSLQ-over-Q(G*) protocol. The CONSTRUCTION of g is
the POST-LOCK attempt; construct_genesis_cokernel_grading() is a NotImplementedError stub.
"""

from mpmath import mp, mpf, mpc, gamma, sqrt, pslq, fabs, re, im

DPS = 140
mp.dps = DPS

def G_star():
    """G* = Gamma(1/4)/Gamma(3/4) ~ 2.95868... (NOT the lemniscate constant varpi ~ 2.6221)."""
    return gamma(mpf(1) / 4) / gamma(mpf(3) / 4)

def C_disc():
    """C = G*(4G*-1) ~ 32.057... -- the master-quadratic discriminant carrier (delta = sqrt(C))."""
    g = G_star()
    return g * (4 * g - 1)

def delta_surd():
    """delta = sqrt(C) ~ 5.66183... -- the DISCRIMINATOR target ONLY (never injected into a construction)."""
    return sqrt(C_disc())

# --- frozen discriminator thresholds -----------------------------------------------------------
PSLQ_MAXCOEFF = 10**12
PSLQ_MAXDEG_QGSTAR = 12        # degree budget for numerator/denominator polynomials in G*
ROU_MAX_ORDER = 24

def _tol():
    return mp.mpf(10) ** (-(mp.dps - 25))

def _field_relation(target_terms, plain_terms):
    """PSLQ over (target_terms ++ plain_terms). Return the integer relation iff it (i) verifies to
    tolerance and (ii) has a NONZERO coefficient on some target term (so the unknown participates --
    a relation among plain terms alone is trivial). Else None. By transcendence of G*, a verified
    relation with the target participating PROVES field membership (denominators cleared)."""
    basis = list(target_terms) + list(plain_terms)
    rel = pslq(basis, maxcoeff=PSLQ_MAXCOEFF, maxsteps=20000)
    if rel is None:
        return None
    nt = len(target_terms)
    if all(rel[j] == 0 for j in range(nt)):
        return None
    if fabs(sum(mp.mpf(c) * b for c, b in zip(rel, basis))) >= _tol():
        return None
    return rel

def is_in_QGstar(x, max_deg=PSLQ_MAXDEG_QGSTAR):
    """True iff x is in the FIELD Q(G*) (a rational function of G*), tested by clearing denominators:
    PSLQ over {x*G*^j} (the unknown) jointly with {G*^k}. x must be real (Q(G*) subset R); 0 is in Q(G*)."""
    if fabs(im(x)) > _tol():
        return False
    xr = mp.mpf(re(x))
    if fabs(xr) < _tol():
        return True
    g = G_star()
    gp = [g**k for k in range(0, max_deg + 1)]
    return _field_relation([xr * p for p in gp], gp) is not None

def is_root_of_unity(x, max_order=ROU_MAX_ORDER):
    """True iff x^n == 1 for some 1 <= n <= max_order (cyclotomic; modulus-1 by design). Returns (bool, n)."""
    if fabs(fabs(x) - 1) > _tol():
        return (False, None)
    xn = mpc(1)
    for n in range(1, max_order + 1):
        xn = xn * x
        if fabs(xn - 1) < _tol():
            return (True, n)
    return (False, None)

def QGstar_delta_decomp(x_real, max_deg=PSLQ_MAXDEG_QGSTAR):
    """If real x = a + c*delta with a,c in the FIELD Q(G*) (rational functions), return (a, c); else None.
    Clears denominators: PSLQ over {x*G*^j} jointly with {G*^k} and {delta*G*^k}. c is the delta-coefficient
    (delta present iff c != 0). Q(G*) is purely transcendental over Q, so sqrt(2)/zeta_8 etc. (genuinely
    outside Q(G*,i,delta)) admit no finite relation -> None (UNDERDETERMINED), as intended."""
    if fabs(im(x_real)) > _tol():
        return None
    xr = mp.mpf(re(x_real))
    if fabs(xr) < _tol():
        return (mp.mpf(0), mp.mpf(0))
    g = G_star(); d = delta_surd()
    gp = [g**k for k in range(0, max_deg + 1)]
    n = len(gp)
    rel = _field_relation([xr * p for p in gp], gp + [d * p for p in gp])
    if rel is None:
        return None
    Ppoly = sum(mp.mpf(rel[j]) * gp[j] for j in range(n))            # x-side denominator P(G*)
    Anum = sum(mp.mpf(rel[n + k]) * gp[k] for k in range(n))         # numerator of a
    Cnum = sum(mp.mpf(rel[2 * n + k]) * gp[k] for k in range(n))     # numerator of c (delta-coeff)
    if fabs(Ppoly) < _tol():
        return None
    return (-Anum / Ppoly, -Cnum / Ppoly)

def classify_grading(g_value):
    """FROZEN VERDICT MAP. Returns ('A'|'B'|'UNDERDETERMINED', reason).
    g_value is the FULL algebraic grading -- do NOT modulus-normalize it (the magnitude carries the
    delta-content; normalizing i*delta/(4G*-1) to i would falsely collapse A->B). Criterion: g in the
    degree-4 CM field Q(G*,i,delta) iff Re(g),Im(g) each in Q(G*)+Q(G*)*delta; OUTCOME A iff delta is
    PRESENT (nonzero delta-coefficient); OUTCOME B iff g in Q(G*,i); else UNDERDETERMINED.
    G3 (section-invariance) and the import-separation/quarantine are CONSTRUCTION obligations the
    instrument cannot check from a single number; a section-dependent g must be reported UNDERDETERMINED."""
    g = mpc(g_value)
    rou, n = is_root_of_unity(g)
    if rou:
        return ('B', f'root of unity, order {n} (cyclotomic; Q(G*)-blind to delta)')
    if is_in_QGstar(g):
        return ('B', 'in Q(G*) (degree 1, field)')
    decR = QGstar_delta_decomp(re(g))
    decI = QGstar_delta_decomp(im(g))
    if decR is None or decI is None:
        return ('UNDERDETERMINED', 'g outside Q(G*,i,delta) field (e.g. sqrt(2)*delta, zeta_8*delta; manual check)')
    if fabs(decR[1]) > _tol() or fabs(decI[1]) > _tol():
        return ('A', 'g in Q(G*,i,delta), delta PRESENT -- Q(G*)(g) carries delta')
    return ('B', 'g in Q(G*,i) (Gaussian/real, delta ABSENT) -- a forced Z/2')

def reclassify_over_dps_band(g_of_dps, dps_list=(100, 120, 140)):
    """G4 INSTRUMENTED: re-run classify at each dps (recomputing G*, delta, g via g_of_dps(dps));
    return the verdict only if UNANIMOUS across the band, else UNDERDETERMINED."""
    saved = mp.dps
    verdicts = []
    try:
        for dd in dps_list:
            mp.dps = dd
            verdicts.append(classify_grading(g_of_dps(dd))[0])
    finally:
        mp.dps = saved
    if len(set(verdicts)) == 1:
        return (verdicts[0], f'unanimous across dps {list(dps_list)}')
    return ('UNDERDETERMINED', f'non-unanimous across dps {list(dps_list)}: {verdicts}')

# --- the deferred construction (POST-LOCK; see PREREG sec.2) ------------------------------------
def construct_genesis_cokernel_grading():
    """POST-LOCK ATTEMPT (NOT run by the self-tests). Per PREREG sec.2 (v3):
      (1) the LINEAR Dirac-Kahler (d - delta) complex of FTD-0089 (REUSE; K^2 = -Hodge-Laplacian);
      (2) the genesis disintegration FIBER over a CANONICAL minimal-norm/Hodge-orthogonal complement,
          its Q(G*)-valued structure constants carrying the genesis nonlinearity;
      (3) g = the SECTION-INVARIANT chirality grading of the fiber content -- the attempt MUST EXHIBIT
          invariance (compute g under >=2 admissible sections; machine-zero agreement), else return None
          (G3 -> UNDERDETERMINED). G5: if the linear-harmonic vs nonlinear-fiber content is not
          separably well-defined, return None (UNDERDETERMINED), do NOT default to B.
    HARD CONSTRAINT (import-separation, reviewer-verified): this function MUST NOT reference delta_surd()/
    C_disc()/G*(4G*-1) while producing g; feed g UN-normalized to classify_grading. Return g or None."""
    raise NotImplementedError(
        "Deferred to the post-lock attempt; the discriminator above is frozen and self-tested. "
        "See PREREG_GENESIS_COKERNEL_GRADING_v1.md sec.2 (v3).")

# --- instrument self-tests (validate the DISCRIMINATOR; never run the deferred construction) -----
def _selftests():
    g = G_star(); C = C_disc(); d = delta_surd()
    zeta8 = (1 + 1j) / sqrt(mp.mpf(2))
    cases = [
        # OUTCOME B (cyclotomic / Q(G*) field, incl. denominators)
        ("zeta_8=(1+i)/sqrt2 [native arrow grading]", mpc(zeta8),            'B'),
        ("i                  [imaginary unit, ROU 4]", mpc(0, 1),            'B'),
        ("G*                 [degree 1 in Q(G*)]",     mpc(g),              'B'),
        ("1/G*               [Q(G*) DENOMINATOR]",     mpc(1) / g,          'B'),
        ("i/G*               [Gaussian-Q(G*) denom]",  mpc(0, 1) / g,       'B'),
        # OUTCOME A -- real coset
        ("delta = sqrt(C)            [real surd]",     mpc(d),              'A'),
        ("G* * delta                 [Q(G*)-multiple]",mpc(g * d),          'A'),
        ("delta/G*           [Q(G*) DENOMINATOR coset]", mpc(d) / g,        'A'),
        ("delta/(4G*-1)      [rational-fn coeff coset]", mpc(d) / (4*g-1),  'A'),
        ("delta/(G*^2+1)     [rational-fn coeff coset]", mpc(d) / (g*g+1),  'A'),
        ("1/delta = (1/C)*delta      [inverse surd]",  mpc(1) / d,          'A'),
        # OUTCOME A -- complex coset (the v1 blind spot)
        ("i*delta = sqrt(-C)         [Z/2 sign on integral]", mpc(0,1)*d,   'A'),
        ("(1+i)*delta                [Gaussian multiple]",    mpc(1,1)*d,   'A'),
        ("(G*+i)*delta               [Q(G*,i) multiple]",     mpc(g,1)*d,   'A'),
        # UNDERDETERMINED -- genuinely outside Q(G*,i,delta), and controls
        ("zeta_8*delta   [outside field: sqrt2]",      mpc(zeta8)*d,        'UNDERDETERMINED'),
        ("sqrt(2)*delta  [outside field: sqrt2]",      sqrt(mp.mpf(2))*d,   'UNDERDETERMINED'),
        ("e/pi           [generic transcendental]",    mp.e/mp.pi,          'UNDERDETERMINED'),
    ]
    print(f"# proof_genesis_cokernel_grading.py v3  -- discriminator self-tests (dps={DPS}, MAXDEG={PSLQ_MAXDEG_QGSTAR})")
    print(f"# G* = {mp.nstr(g, 26)}   C = {mp.nstr(C, 26)}   delta = {mp.nstr(d, 26)}")
    print("# columns: EXPECT  GOT  PASS/FAIL  case")
    npass = 0
    for label, value, expect in cases:
        got, why = classify_grading(value)
        ok = (got == expect); npass += ok
        print(f"  {expect:>15}  {got:>15}  {'PASS' if ok else 'FAIL'}  {label}   [{why}]")
    band_A = reclassify_over_dps_band(lambda _d: mpc(0, 1) * sqrt(G_star() * (4 * G_star() - 1)) / (4 * G_star() - 1))
    band_B = reclassify_over_dps_band(lambda _d: mpc(1) / G_star())
    print(f"# G4 dps-band: i*delta/(4G*-1) -> {band_A[0]} ({band_A[1]}); 1/G* -> {band_B[0]} ({band_B[1]})")
    band_ok = (band_A[0] == 'A' and band_B[0] == 'B')
    print(f"# {npass}/{len(cases)} discriminator self-tests passed; G4-band {'PASS' if band_ok else 'FAIL'}.")
    print("# Construction of g is DEFERRED to the post-lock attempt (NotImplementedError by design).")
    return npass == len(cases) and band_ok

if __name__ == "__main__":
    import sys
    sys.exit(0 if _selftests() else 1)
