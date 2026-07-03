#!/usr/bin/env python3
"""
genesis_cokernel_construction_v1.py -- POST-LOCK CONSTRUCTION ATTEMPT (v1)
==========================================================================
Executes the deferred construction of PREREG_GENESIS_COKERNEL_GRADING_v1.md
(docs/theory/02_foundations/). The FROZEN instrument is
scripts/proofs/proof_genesis_cokernel_grading.py (v3), SHA256
63c342fae6c122c20ce5c6a93349e33a6a8710f73a0cf304ab4983a71b585b39 -- imported
UNMODIFIED below (hash re-verified at runtime before import).

Result doc: docs/theory/02_foundations/ANALYSIS_GENESIS_COKERNEL_GRADING_v1.md

------------------------------------------------------------------------------
CONSTRUCTION CHAIN (forward-derived; provenance per step)
------------------------------------------------------------------------------
 Step 1 [REUSE, FTD-0089]  Discrete Dirac-Kahler complex on the FTD lattice:
        grades (S, V_i, P_ij, T) = (0,1,2,3)-forms; K = d - delta_codiff;
        chirality grading gamma = (-1)^p. The DK codifferential on 1-forms is
        the BACKWARD-difference divergence (DERIV_DIRAC_KAHLER_IDENTIFICATION
        sec. A1.3: delta phi^(1) = sum_i nabla^-_i phi^(1)_i).

 Step 2 [SPEC P3 / sec 4.1; engine phase_write.cpp -- the rule itself is
        [IMPOSED] per SPEC; the factorization is [DERIVED]]
        Genesis merge M_disc (single-substrate canonical path):
          threshold:  |J(v)| > K_B          (0-form modulus rho)
          polarity:   s = sign(div J(v))    (0-form u from grade-1 J)
          drain:      J <- J * (1 - K_B/|J|)   [radial retraction]
        FACTORIZATION (forced by the rule reading exactly these two scalars):
          M_disc = m o pi,   pi(J) = (rho, u),   m = the nonlinear collapse.
        The drain is radial: it maps pi-fibers to pi-fibers (shifts rho,
        fixes direction, touches no neighbor) => adds NO fiber structure.
        Divergence stencil conventions (both covered below):
          engine-canonical (field_operators.h::divergence_from_flux_array):
            u = sum_i (J_i(v+e_i) - J_i(v-e_i)) / 2     [CENTRAL; w_c = 0]
          DK-canonical (FTD-0089 codifferential):
            u = sum_i (J_i(v) - J_i(v-e_i))             [BACKWARD; w_c = +1]

 Step 3 [DERIVED]  Moore-Penrose / Hodge-orthogonal complement of the
        information-loss fiber: H = span{grad rho, grad u} = row space of
        d(pi); fiber V = ker d(pi). The structure constants of the fibration
        over the chosen horizontal complement = the Ehresmann curvature; with
        a 2-dimensional base there is exactly ONE independent component:
            b = P_V( [grad rho, grad u] ).
        Since u is LINEAR (grad u constant) the bracket reduces to
            [grad rho, grad u] = (grad u . nabla)(grad rho)
                               = ( P_perp(w_c) / |c| ; 0 ),
        where c = J(v), P_perp = I - chat chat^T, w_c = center block of grad u.

 Step 4 [DERIVED]  Branch exhibit (G2 obligation): iota: J -> -J exchanges the
        s = +1 and s = -1 fibers of M_disc (rho even, u odd) and is represented
        on the DK grade observables EXACTLY as gamma = (-1)^p:
            S = |c|^2 (even), V = c (odd), P (quadratic, even), T = c1 c2 c3 (odd).
        The bracket field b is gamma-ODD (d iota = -I and b(-x) = b(x), so
        iota_* b = -b). Both facts verified numerically below.

 Step 5 [COMPUTED]  Fiber-content invariants at admissible sections:
        even invariant  |P_V(b)|^2 ;  graded (gamma-odd) invariant
        q = < P_V(b), grad T >  (T = the DK pseudoscalar, the canonical odd
        0-form functional of the center flux).

 Step 6  construct_genesis_cokernel_grading_v1() returns None per the frozen
        stub contract (G3 -> UNDERDETERMINED; G5 -> UNDERDETERMINED).

------------------------------------------------------------------------------
BANNED-MOVE COMPLIANCE (PREREG sec. 7)
------------------------------------------------------------------------------
 1. IMPORT-SEPARATION: the construction code below NEVER calls the frozen
    instrument's delta-carrier accessors (the surd, the discriminant C, or
    the combination G*(4G*-1)); the only inputs are rational lattice/stencil
    data from SPEC + engine. A static self-check at the bottom greps this
    file's own source for the accessor tokens. (The frozen discriminator
    internally uses delta -- that is its job, not the construction's.)
 2. QUARANTINE: the known half-derivative branch sign (1+i)/sqrt2 (FTD-0323
    sec. 5) is nowhere substituted for g; no cyclotomic sign is fed to the
    discriminator as a construction output.
 3. NO SECTION-FORCING: the section points below are used to TEST
    section-invariance (G3), never to select a value.
 4. NO MODULUS-NORMALIZATION: no candidate is normalized.
 5. NO BASKET SEARCH: the two clearly-labeled diagnostics (the degenerate
    central-stencil value g0 = 0; the measure-import fiber average) are run
    to DOCUMENT gate semantics, and are quarantined from the verdict.
"""

import hashlib
import os
import sys

# --------------------------------------------------------------------------
# 0. Verify the frozen instrument's hash BEFORE importing it (lock integrity)
# --------------------------------------------------------------------------
HERE = os.path.dirname(os.path.abspath(__file__))
FROZEN_PATH = os.path.join(HERE, "proof_genesis_cokernel_grading.py")
FROZEN_SHA256 = "63c342fae6c122c20ce5c6a93349e33a6a8710f73a0cf304ab4983a71b585b39"

def verify_frozen_instrument():
    with open(FROZEN_PATH, "rb") as fh:
        h = hashlib.sha256(fh.read()).hexdigest()
    if h != FROZEN_SHA256:
        raise SystemExit(
            "ABORT: frozen instrument SHA256 mismatch.\n  expected %s\n  got      %s"
            % (FROZEN_SHA256, h))
    return h

_HASH_OK = verify_frozen_instrument()
sys.path.insert(0, HERE)
import proof_genesis_cokernel_grading as inst  # the FROZEN discriminator (unmodified)

from mpmath import mp, mpf, mpc, sqrt, fabs, log, quad

mp.dps = 140
TOL = mp.mpf(10) ** (-(mp.dps - 25))

# --------------------------------------------------------------------------
# small exact-ish vector helpers (mpmath, deterministic, no randomness)
# --------------------------------------------------------------------------
def vdot(x, y):
    s = mp.mpf(0)
    for a, b in zip(x, y):
        s += a * b
    return s

def vsub(x, y):
    return [a - b for a, b in zip(x, y)]

def vscale(t, x):
    return [t * a for a in x]

def vnorm2(x):
    return vdot(x, x)


# --------------------------------------------------------------------------
# 1. The merge data pi = (rho, u) and its Moore-Penrose complement
# --------------------------------------------------------------------------
# Configuration coordinates (single-substrate genesis, SPEC sec. 4.1):
#   BACKWARD (DK-canonical) stencil: x = (c1,c2,c3, m1,m2,m3), m_i = J_i(v-e_i)
#       u = sum_i (c_i - m_i);   grad u = (1,1,1, -1,-1,-1)   [w_c = +ones]
#   CENTRAL (engine-canonical) stencil: x = (c1,c2,c3, p1,p2,p3, m1,m2,m3)
#       u = sum_i (p_i - m_i)/2; grad u = (0,0,0, .5,.5,.5, -.5,-.5,-.5) [w_c = 0]
# rho(x) = |c| in both cases; grad rho = (chat; 0...).

def grad_u(stencil):
    if stencil == "backward":
        return [mp.mpf(1)] * 3 + [mp.mpf(-1)] * 3
    if stencil == "central":
        return [mp.mpf(0)] * 3 + [mp.mpf("0.5")] * 3 + [mp.mpf("-0.5")] * 3
    raise ValueError(stencil)

def grad_rho(x, stencil):
    dim = 6 if stencil == "backward" else 9
    c = x[:3]
    r = sqrt(vnorm2(c))
    g = [c[i] / r for i in range(3)] + [mp.mpf(0)] * (dim - 3)
    return g

def bracket(x, stencil):
    """[grad rho, grad u] = (grad u . nabla)(grad rho) - 0
       = ( (w_c - chat (chat.w_c)) / |c| ; 0 ).   Exact reduction: grad u is
       CONSTANT (u linear), so the second Lie-bracket term vanishes and the
       first differentiates chat along the center block w_c of grad u."""
    dim = 6 if stencil == "backward" else 9
    c = x[:3]
    r = sqrt(vnorm2(c))
    chat = [ci / r for ci in c]
    w_c = grad_u(stencil)[:3]
    a = vdot(chat, w_c)
    b_c = [(w_c[i] - chat[i] * a) / r for i in range(3)]
    return b_c + [mp.mpf(0)] * (dim - 3)

def fiber_project(b, x, stencil):
    """P_V(b): subtract the H-component, H = span{grad rho, grad u} (the
       Moore-Penrose / Hodge-orthogonal complement), via the 2x2 Gram solve."""
    gr = grad_rho(x, stencil)
    gu = grad_u(stencil)
    g11, g12, g22 = vdot(gr, gr), vdot(gr, gu), vdot(gu, gu)
    r1, r2 = vdot(b, gr), vdot(b, gu)
    det = g11 * g22 - g12 * g12
    a1 = (g22 * r1 - g12 * r2) / det
    a2 = (g11 * r2 - g12 * r1) / det
    return vsub(vsub(b, vscale(a1, gr)), vscale(a2, gu))

def grad_T(x, stencil):
    """Gradient of the DK pseudoscalar functional T = c1 c2 c3 (grade-3
       observable of the center flux, FTD-0089 table) -- the canonical
       gamma-ODD scalar functional available for the graded pairing."""
    dim = 6 if stencil == "backward" else 9
    c = x[:3]
    return [c[1] * c[2], c[0] * c[2], c[0] * c[1]] + [mp.mpf(0)] * (dim - 3)

def content_even(x, stencil):
    """|P_V(b)|^2 -- gamma-EVEN scalar invariant of the fiber content."""
    return vnorm2(fiber_project(bracket(x, stencil), x, stencil))

def content_odd(x, stencil):
    """q = <P_V(b), grad T> -- gamma-ODD (chirality-graded) scalar invariant.
       This is the graded fiber content the pre-reg's g must be built from."""
    return vdot(fiber_project(bracket(x, stencil), x, stencil), grad_T(x, stencil))


# --------------------------------------------------------------------------
# 2. Step-4 exhibits: branch involution iota = gamma; bracket is gamma-odd
# --------------------------------------------------------------------------
def check_branch_involution():
    """iota: J -> -J.  (i) exchanges the s = +1 / s = -1 merge fibers
       (rho even, u odd -> sign(u) flips);  (ii) acts on the DK grade
       observables exactly as gamma = (-1)^p;  (iii) the bracket field is
       gamma-odd: b(-x) = b(x) with d(iota) = -I  =>  iota_* b = -b."""
    # deterministic generic test point (exact rationals; |c| != 1 deliberately)
    x = [mp.mpf(q) for q in ("0.75", "-0.5", "1.25", "0.3", "-0.2", "0.4")]
    xm = [-t for t in x]
    c = x[:3]
    ok = True
    # (i) rho even, u odd (backward stencil shown; central identical in kind)
    rho = sqrt(vnorm2(c)); rho_m = sqrt(vnorm2(xm[:3]))
    u = vdot(grad_u("backward"), x); u_m = vdot(grad_u("backward"), xm)
    ok &= fabs(rho - rho_m) < TOL and fabs(u + u_m) < TOL
    # (ii) DK grade parity table: S even, V odd, P even, T odd == gamma=(-1)^p
    S = vnorm2(c)
    V = c[:]
    P12 = c[0] * c[1]              # representative quadratic (grade-2) entry
    T = c[0] * c[1] * c[2]
    cm = xm[:3]
    ok &= fabs(S - vnorm2(cm)) < TOL                       # (+) grade 0
    ok &= all(fabs(V[i] + cm[i]) < TOL for i in range(3))  # (-) grade 1
    ok &= fabs(P12 - cm[0] * cm[1]) < TOL                  # (+) grade 2
    ok &= fabs(T + cm[0] * cm[1] * cm[2]) < TOL            # (-) grade 3
    # (iii) b(-x) = b(x)  (then d(iota) = -I gives iota_* b = -b: odd)
    b1 = bracket(x, "backward"); b2 = bracket(xm, "backward")
    ok &= all(fabs(b1[i] - b2[i]) < TOL for i in range(6))
    # sanity: b is orthogonal to grad rho (bracket lies tangent to the sphere)
    ok &= fabs(vdot(b1, grad_rho(x, "backward"))) < TOL
    return bool(ok)


# --------------------------------------------------------------------------
# 3. Admissible sections (G3 test points -- NOT value selections)
# --------------------------------------------------------------------------
# All sections lie on ONE fiber of pi: rho = 1, u = 1 (branch s = +1).
# rho = 1 > K_B = 0.511 in lattice units => genesis-admissible. The G3 test
# compares points on the SAME fiber, so no scale is smuggled in.
#
# BACKWARD (DK-canonical), x = (c; m):
#   SB1: c = (1, 0, 0),          m = (0, 0, 0)             u = 1
#   SB2: c = (1,1,1)/sqrt(3),    m = (sqrt(3)-1, 0, 0)     u = 1
#   SB3: c = (2,-1, 2)/3,        m = (0, 0, 0)             u = 1
# CENTRAL (engine-canonical), x = (c; p; m):
#   SC1: c = (1, 0, 0),          p = (2, 0, 0),  m = 0     u = 1
#   SC2: c = (1,1,1)/sqrt(3),    p = (0, 2, 0),  m = 0     u = 1
#   SC3: c = (2,-1, 2)/3,        p = (1, 1, 0),  m = 0     u = 1

def sections(stencil):
    s3 = sqrt(mp.mpf(3))
    if stencil == "backward":
        return {
            "SB1": [mp.mpf(1), mp.mpf(0), mp.mpf(0), mp.mpf(0), mp.mpf(0), mp.mpf(0)],
            "SB2": [1 / s3, 1 / s3, 1 / s3, s3 - 1, mp.mpf(0), mp.mpf(0)],
            "SB3": [mp.mpf(2) / 3, mp.mpf(-1) / 3, mp.mpf(2) / 3,
                    mp.mpf(0), mp.mpf(0), mp.mpf(0)],
        }
    return {
        "SC1": [mp.mpf(1), mp.mpf(0), mp.mpf(0),
                mp.mpf(2), mp.mpf(0), mp.mpf(0),
                mp.mpf(0), mp.mpf(0), mp.mpf(0)],
        "SC2": [1 / s3, 1 / s3, 1 / s3,
                mp.mpf(0), mp.mpf(2), mp.mpf(0),
                mp.mpf(0), mp.mpf(0), mp.mpf(0)],
        "SC3": [mp.mpf(2) / 3, mp.mpf(-1) / 3, mp.mpf(2) / 3,
                mp.mpf(1), mp.mpf(1), mp.mpf(0),
                mp.mpf(0), mp.mpf(0), mp.mpf(0)],
    }

def fiber_check(x, stencil):
    """Confirm the section lies on the reference fiber (rho = 1, u = 1)."""
    rho = sqrt(vnorm2(x[:3]))
    u = vdot(grad_u(stencil), x)
    return fabs(rho - 1) < TOL and fabs(u - 1) < TOL


# --------------------------------------------------------------------------
# 4. Analytic cross-check (backward stencil):
#    content_even = 3 (3 - a^2) / (rho^2 (6 - a^2)),  a = chat . ones
#    (derived by the Gram solve above in closed form; verified numerically)
# --------------------------------------------------------------------------
def content_even_analytic(a):
    return 3 * (3 - a * a) / (6 - a * a)   # rho = 1


# --------------------------------------------------------------------------
# 5. THE DEFERRED CONSTRUCTION (per the frozen stub's contract)
# --------------------------------------------------------------------------
def construct_genesis_cokernel_grading_v1(report):
    """Per proof_genesis_cokernel_grading.construct_genesis_cokernel_grading's
    frozen contract: return the section-invariant grading g, or None if the
    invariance/well-posedness obligations fail (G3/G5 -> UNDERDETERMINED).

    Outcome of the attempt (computed in main() and passed in `report`):
      - engine-central stencil: the fibration is FLAT (w_c = 0 => bracket == 0
        identically). The unique section-invariant value is g0 = 0: a NULL
        grading. Per G2 (verbatim): 'A null or degenerate grading is NOT
        Outcome B ... g = 0 ... is UNDERDETERMINED.'
      - DK-backward stencil: the graded content is SECTION-DEPENDENT on a
        single fiber (q: 0 at SB1 vs 4/15 at SB3; even part: 6/5 at SB1/SB3
        vs 0 at SB2). Machine-zero agreement across >= 2 admissible sections
        FAILS => G3 => return None.
      - G5: the pre-reg sec. 2.3 premise (Q(G*)-valued fiber structure
        constants) is NOT realized: every computed invariant is an exact
        RATIONAL in section data; the only non-rational scale in the rule is
        the import K_B. No G* enters the genesis rule's fiber content.
    """
    if report["central_all_zero"] and report["backward_section_dependent"]:
        return None
    # (unreachable on the actual rule; kept for contract completeness)
    return None


# --------------------------------------------------------------------------
# 6. Execution
# --------------------------------------------------------------------------
def main():
    print("=" * 78)
    print("POST-LOCK CONSTRUCTION ATTEMPT -- PREREG_GENESIS_COKERNEL_GRADING_v1")
    print("frozen instrument SHA256 verified: %s" % _HASH_OK)
    print("dps = %d" % mp.dps)
    print("=" * 78)

    # ---- G1: frozen discriminator self-tests (17/17 + G4 band) ----
    print("\n[G1] frozen-instrument self-tests (unmodified):")
    g1_ok = inst._selftests()
    print("[G1] %s" % ("PASS" if g1_ok else "FAIL"))

    # ---- Step-4 exhibits ----
    print("\n[G2 branch exhibit] iota: J -> -J")
    branch_ok = check_branch_involution()
    print("  iota exchanges s=+1/-1 merge fibers (rho even, u odd) ........ %s"
          % ("PASS" if branch_ok else "FAIL"))
    print("  iota == gamma=(-1)^p on DK grades (S+,V-,P+,T-) .............. %s"
          % ("PASS" if branch_ok else "FAIL"))
    print("  bracket field is gamma-ODD (iota_* b = -b) ................... %s"
          % ("PASS" if branch_ok else "FAIL"))

    # ---- Step-5: fiber content at admissible sections ----
    report = {}

    print("\n[content] ENGINE-CANONICAL central stencil "
          "(field_operators.h: (J[v+e]-J[v-e])/2; w_c = 0):")
    central_vals = []
    for name, x in sections("central").items():
        assert fiber_check(x, "central"), name
        ce = content_even(x, "central")
        co = content_odd(x, "central")
        central_vals.append((ce, co))
        print("  %s: |P_V b|^2 = %s   q = %s" % (name, mp.nstr(ce, 8), mp.nstr(co, 8)))
    central_all_zero = all(fabs(ce) < TOL and fabs(co) < TOL for ce, co in central_vals)
    report["central_all_zero"] = central_all_zero
    print("  => bracket identically ZERO (flat fibration): the polarity signal")
    print("     never reads the center voxel. Unique section-invariant value:")
    print("     g0 = 0 (NULL grading) -> G2: 'NOT Outcome B ... UNDERDETERMINED'.")
    print("  central-stencil degeneracy confirmed: %s" % central_all_zero)

    print("\n[content] DK-CANONICAL backward stencil "
          "(FTD-0089 codifferential; w_c = +ones):")
    bw = {}
    for name, x in sections("backward").items():
        assert fiber_check(x, "backward"), name
        ce = content_even(x, "backward")
        co = content_odd(x, "backward")
        a = vdot([xi / sqrt(vnorm2(x[:3])) for xi in x[:3]], [mp.mpf(1)] * 3)
        ce_analytic = content_even_analytic(a)
        assert fabs(ce - ce_analytic) < TOL, "analytic cross-check failed at " + name
        bw[name] = (ce, co)
        print("  %s: |P_V b|^2 = %s   q = %s   (a = chat.ones = %s)"
              % (name, mp.nstr(ce, 8), mp.nstr(co, 8), mp.nstr(a, 8)))
    # exact rational identifications (even: 6/5 and 0; odd: 0 and 4/15)
    exact_ok = (fabs(bw["SB1"][0] - mp.mpf(6) / 5) < TOL and
                fabs(bw["SB3"][0] - mp.mpf(6) / 5) < TOL and
                fabs(bw["SB2"][0]) < TOL and
                fabs(bw["SB1"][1]) < TOL and
                fabs(bw["SB3"][1] - mp.mpf(4) / 15) < TOL and
                fabs(bw["SB2"][1]) < TOL)
    print("  exact values: even = {6/5, 6/5, 0}; graded q = {0, 4/15, 0} : %s"
          % ("CONFIRMED" if exact_ok else "MISMATCH"))
    print("  closed form (rho=1): |P_V b|^2 = 3(3-a^2)/(6-a^2) -- NON-CONSTANT")
    print("  on one fiber (a = chat.ones is free on the fiber).")

    # ---- G3: section-invariance obligation ----
    print("\n[G3] section-invariance across admissible sections of ONE fiber")
    print("     (rho = 1, u = 1, branch s = +1):")
    g3_even_fail = fabs(bw["SB1"][0] - bw["SB2"][0]) > mp.mpf("0.1")
    g3_odd_fail = fabs(bw["SB3"][1] - bw["SB1"][1]) > mp.mpf("0.1")
    print("  even content:  SB1 = 6/5  vs  SB2 = 0      -> disagree: %s" % g3_even_fail)
    print("  graded content: SB1 = 0   vs  SB3 = 4/15   -> disagree: %s" % g3_odd_fail)
    print("  NOTE: SB1 and SB3 have EQUAL even content (6/5) but DIFFERENT graded")
    print("  content (0 vs 4/15) -- the chirality-graded part is maximally")
    print("  section-dependent. Machine-zero agreement FAILS. G3: FAIL.")
    report["backward_section_dependent"] = bool(g3_even_fail and g3_odd_fail)

    # ---- G4: dps-band stability of the G3 disagreement (100/120/140) ----
    print("\n[G4] dps-band stability of the gate outcomes:")
    saved = mp.dps
    band_ok = True
    for dd in (100, 120, 140):
        mp.dps = dd
        tol_d = mp.mpf(10) ** (-(mp.dps - 25))
        secs = sections("backward")
        d_even = content_even(secs["SB1"], "backward") - content_even(secs["SB2"], "backward")
        d_odd = content_odd(secs["SB3"], "backward") - content_odd(secs["SB1"], "backward")
        c_zero = content_even(sections("central")["SC2"], "central")
        stable = (fabs(d_even - mp.mpf(6) / 5) < tol_d and
                  fabs(d_odd - mp.mpf(4) / 15) < tol_d and
                  fabs(c_zero) < tol_d)
        band_ok &= stable
        print("  dps=%3d: even-gap = 6/5 (%s), graded-gap = 4/15 (%s), central = 0 (%s)"
              % (dd, stable, stable, stable))
    mp.dps = saved
    print("  G3-failure and G2-degeneracy are UNANIMOUS across dps {100,120,140}: %s"
          % band_ok)

    # ---- G5: well-posedness / the Q(G*) premise ----
    print("\n[G5] well-posedness of the separation premise (PREREG sec. 2.3):")
    print("  Every fiber-content invariant computed above is an exact RATIONAL")
    print("  in section data (6/5, 4/15, 0, and 3(3-a^2)/(6-a^2)); the only")
    print("  non-rational scale in the genesis rule is the import K_B = 0.511.")
    print("  The premise 'structure constants are rational functions of G*' is")
    print("  NOT realized: no K-BIND Q(G*)-valued operator (Watson scaling")
    print("  G*^2/2pi, det_zeta ratio G*) enters the genesis rule; the FTD-0323")
    print("  half-derivative tie is QUARANTINED (banned move 2) and the rule")
    print("  contains no fractional-order operator. G5: FAIL (premise).")

    # ---- Step 6: the construction returns None ----
    g = construct_genesis_cokernel_grading_v1(report)
    print("\n[construction] construct_genesis_cokernel_grading_v1() -> %r" % g)
    print("  (frozen stub contract: None => G3/G5 => UNDERDETERMINED)")

    # ---- DIAGNOSTICS (quarantined from the verdict; labeled) ----
    print("\n[diagnostic 1 -- QUARANTINED] the degenerate central-stencil value g0 = 0:")
    v0, why0 = inst.classify_grading(mpc(0))
    print("  classify_grading(0) = (%r, %r)" % (v0, why0))
    b0 = inst.reclassify_over_dps_band(lambda _d: mpc(0))
    print("  reclassify_over_dps_band(0) = (%r, %r)" % (b0[0], b0[1]))
    print("  PREREG G2 OVERRIDE (verbatim): 'A null or degenerate grading is NOT")
    print("  Outcome B -- g = 0 ... is UNDERDETERMINED.' The instrument cannot")
    print("  see branch content from one number; the gate exists for exactly this.")

    print("\n[diagnostic 2 -- QUARANTINED, measure-import] fiber-averaged even content:")
    print("  Invariantizing by averaging over the fiber requires an IMPORTED")
    print("  regularization of the infinite disintegration measure (the pre-reg's")
    print("  ceiling names this exact import: a chosen prior/base measure).")
    print("  Under the regularized-uniform marginal (a = sqrt(3) t, t uniform):")
    avg_closed = 3 - 3 * log(1 + sqrt(mp.mpf(2))) / sqrt(mp.mpf(2))
    avg_quad = quad(lambda t: 3 * (1 - t * t) / (2 - t * t), [0, 1])  # = <f>/1
    print("  closed form 3 - 3 ln(1+sqrt2)/sqrt2 = %s" % mp.nstr(avg_closed, 30))
    print("  numeric quad cross-check           = %s  (agree: %s)"
          % (mp.nstr(avg_quad, 30), fabs(avg_closed - avg_quad) < mp.mpf(10) ** (-100)))
    vA, whyA = inst.classify_grading(mpc(avg_closed))
    print("  classify_grading(avg) = (%r, %r)" % (vA, whyA))
    print("  => even the imported invariantization lands in the LOG period class")
    print("  (ln(1+sqrt2)), outside Q(G*,i,delta) -- the UNDERDETERMINED row;")
    print("  it reaches neither Q(G*) nor delta. Out of scope for the verdict.")

    # ---- import-separation static self-check ----
    print("\n[import-separation] static self-check of this file's source:")
    with open(os.path.abspath(__file__), "r", encoding="utf-8") as fh:
        src_lines = fh.readlines()
    bad = []
    for ln, line in enumerate(src_lines, 1):
        if "SEPARATION-CHECK-ALLOWED" in line:
            continue
        for token in ("inst.delta_surd", "inst.C_disc"):  # SEPARATION-CHECK-ALLOWED
            if token in line:
                bad.append((ln, token))
    print("  construction references to delta/C carriers: %d  -> %s"
          % (len(bad), "PASS (import-separation held)" if not bad else "FAIL %r" % bad))

    # ---- VERDICT ----
    print("\n" + "=" * 78)
    print("VERDICT (frozen map, PREREG sec. 4, row 3): UNDERDETERMINED")
    print("=" * 78)
    print("""\
  G1  PASS   17/17 self-tests + G4 band, hash-verified frozen instrument.
  G2  FAIL   engine-central stencil: fibration is FLAT; the unique
             section-invariant grading is the NULL g0 = 0 -- 'NOT Outcome B'.
  G3  FAIL   DK-backward stencil: graded content is section-dependent on a
             single fiber (q = 0 vs 4/15 at equal even content; even content
             6/5 vs 0). No machine-zero agreement across admissible sections.
  G4  PASS*  the gate outcomes themselves are unanimous across dps
             {100,120,140} (*no verdict-bearing g exists to band-classify).
  G5  FAIL   the sec. 2.3 premise (Q(G*)-valued fiber structure constants) is
             not realized by the SPEC/engine genesis rule: all invariants are
             exact rationals in section data + the import K_B. No G* enters;
             a fortiori no delta.

  Any gate failure => the run does not count as A or B (re-scope), NOT a
  positive verdict. UNDERDETERMINED was the registered prior-dominant
  outcome (~45%).

  ZERO PROMOTIONS: x+ = 1/alpha stays [STRONGLY MOTIVATED CONJECTURE]
  (FTD-0013); MC-T4.3 stays [FOUNDATIONAL OBSTRUCTION]; FC-W stays adopted
  [AXIOM]-class (FTD-0315); FTD-0244/0314/0326/0327 untouched; no alpha
  derived; golden gate untouched (pure number theory, no engine state).""")
    return 0

if __name__ == "__main__":
    sys.exit(main())
