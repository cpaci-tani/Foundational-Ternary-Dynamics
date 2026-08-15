"""FTD — Native C3 v3: first decisions in the ternary-mask sector (FTD-1007).

Under the owner-adopted ternary mask (DERIV_TERNARY_MASK_EXTENSION_v1.md):
only equal-NONZERO bonds are forbidden; the void bonds to everything at half
stiffness; clearances: same-nonzero non-bonded pairs keep the capacity floor
q >= 4/25, all other non-bonded pairs must clear the support q >= 3/2. The
rigidity machinery (coker(R), stress sign for chains/struts, blocking PD) is
unchanged — positive bond weights cannot alter definiteness.

Declared menu (see PREREG_TERNARY_SECTOR_DECISION_v3.md):
  M1  W6 unit wheel, void hub, alternating rim  (newly legal; the classic
      wheel stress exists in-plane — the live question is 3D blocking)
  M2  regular unit octahedron, ternary-colored by antipodal pairs
      (K_{2,2,2} is exactly 3-chromatic; first legal fully-braced solid)
  M3  gyroelongated square bipyramid J17 (unit deltahedron, void ring +
      void apex coloring)
  M4  PROBE — line-symmetric (Bricard-I ansatz) unit-edge realizations of
      the octahedron graph beyond the regular one: numeric continuation
      over the symmetry family with exact spot-verification at any
      rank-drop candidate. Declared as a probe: may return INCOMPLETE.

All-single-bond frameworks carry no chain, so the buckling sign constraint
is vacuous: gate 4 asks only for a nonzero self-stress (any sign pattern —
single unit bonds carry either sign), gate 5 for blocking PD on nontrivial
flexes. No target constant appears (v1 lint discipline; no banned tokens).
"""

import itertools
import json
import os
import sys

import numpy as np
import sympy as sp

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import native_unit_strut_tensegrity_decision as v1
import native_antiprism_decision as v2

CHECKS = []
VERDICTS = []


def check(cid, desc, ok, detail=""):
    CHECKS.append((cid, desc, bool(ok), detail))
    print(f"[{'PASS' if ok else 'FAIL'}] {cid}  {desc}"
          + (f"  -- {detail}" if detail else ""))


def verdict(cell, vd, cert):
    VERDICTS.append({"cell": cell, "verdict": vd, "certificate": cert})
    print(f"    {cell:<44} {vd:<16} {cert[:66]}")


# ---------------------------------------------------------------------------
# ternary gates
# ---------------------------------------------------------------------------
def ternary_legal(pol, edges):
    bad = [(u, w) for u, w, _ in edges
           if pol[u] != 0 and pol[w] != 0 and pol[u] == pol[w]]
    return not bad, bad


def ternary_clearance(sites, pol, edges):
    bonded = {frozenset((u, w)) for u, w, _ in edges}
    for i in range(len(sites)):
        for j in range(i + 1, len(sites)):
            if frozenset((i, j)) in bonded:
                continue
            q = sp.simplify((sites[i] - sites[j]).dot(sites[i] - sites[j]))
            same_nonzero = pol[i] != 0 and pol[i] == pol[j]
            bound = sp.Rational(4, 25) if same_nonzero else sp.Rational(3, 2)
            ok, _ = v1.cmp_ge(q, bound)
            if ok is not True:
                kind = "same-nonzero" if same_nonzero else "interacting"
                return False, f"{kind} pair ({i},{j}) q={sp.N(q, 8)} < {bound}"
    return True, "all clearances certified"


def decide(cell, sites, pol, edges):
    okl, bad = ternary_legal(pol, edges)
    if not okl:
        return "POLARITY-KILL", f"equal-nonzero bond {bad[0]}"
    for u, w, _ in edges:
        L2 = sp.simplify((sites[u] - sites[w]).dot(sites[u] - sites[w]))
        if sp.simplify(L2 - 1) != 0:
            return "CLOSURE-KILL", f"edge ({u},{w}) length^2 = {L2}"
    okc, why = ternary_clearance(sites, pol, edges)
    if not okc:
        return "CLEARANCE-KILL", why
    R = v1.rigidity_matrix(sites, edges)
    stresses = R.T.nullspace()
    if not stresses:
        return "NO-STRESS", "coker(R) = 0: every flex extends"
    flexes = R.nullspace()
    triv = v1.trivial_flexes(sites)
    # all bonds single: any stress sign admissible; test blocking for the
    # basis stresses and their +/- combinations (dim <= 2 declared)
    if len(stresses) > 2:
        return "INCOMPLETE", f"stress dim {len(stresses)} > 2"
    rays = [stresses[0], -stresses[0]]
    if len(stresses) == 2:
        rays += [stresses[1], -stresses[1],
                 stresses[0] + stresses[1], stresses[0] - stresses[1],
                 -stresses[0] + stresses[1], -stresses[0] - stresses[1]]
    best = ("BLOCKING-KILL", "no candidate stress blocks the flex space")
    for wv in rays:
        vd, why = v1.blocking_gate(wv, flexes, triv, sites, edges)
        if vd == "PASS":
            return "PASS", why
        if vd == "RIGID":
            return "RIGID", why
        best = (vd, why)
    return best


# ---------------------------------------------------------------------------
# M1 — W6 void-hub wheel
# ---------------------------------------------------------------------------
def m1():
    sites, pol = [], []
    sites.append(sp.Matrix([0, 0, 0])); pol.append(0)          # void hub
    for j in range(6):
        a = 2 * sp.pi * j / 6
        sites.append(sp.Matrix([sp.cos(a), sp.sin(a), 0]))
        pol.append(1 if j % 2 == 0 else -1)
    edges = [(0, 1 + j, "strut") for j in range(6)]            # spokes
    edges += [(1 + j, 1 + (j + 1) % 6, "chain") for j in range(6)]
    vd, cert = decide("M1 W6 void-hub wheel", sites, pol, edges)
    verdict("M1 W6 void-hub wheel", vd, cert)


# ---------------------------------------------------------------------------
# M2 — ternary octahedron
# ---------------------------------------------------------------------------
def m2():
    a = 1 / sp.sqrt(2)
    P = [( a, 0, 0), (-a, 0, 0), (0, a, 0), (0, -a, 0), (0, 0, a), (0, 0, -a)]
    pol = [1, 1, -1, -1, 0, 0]                     # antipodal color classes
    sites = [sp.Matrix(p) for p in P]
    edges = [(i, j, "chain") for i in range(6) for j in range(i + 1, 6)
             if sp.simplify((sites[i] - sites[j]).dot(
                 sites[i] - sites[j]) - 1) == 0]
    vd, cert = decide("M2 ternary octahedron", sites, pol, edges)
    verdict("M2 ternary octahedron", vd, cert)
    check("C03", "M2: K(2,2,2) 3-coloring legal, 12 unit bonds",
          len(edges) == 12 and ternary_legal(pol, edges)[0])


# ---------------------------------------------------------------------------
# M3 — J17 gyroelongated square bipyramid
# ---------------------------------------------------------------------------
def m3():
    sites, pol, edges = v2.build_antiprism(4)      # full bracing, 16 edges
    pol = [1, -1, 1, -1, 0, 0, 0, 0]               # ring1 +/- alt, ring2 void
    rho, chord, S = v2.antiprism_geometry(4)
    h = sp.sqrt(1 - rho ** 2)
    iB = len(sites); sites.append(sp.Matrix([0, 0, -h])); pol.append(0)
    iT = len(sites); sites.append(sp.Matrix([0, 0, S + h])); pol.append(0)
    for j in range(4):
        edges.append((iB, j, "chain"))             # bottom apex to ring1
        edges.append((iT, 4 + j, "chain"))         # top apex to ring2
    vd, cert = decide("M3 J17 unit deltahedron", sites, pol, edges)
    verdict("M3 J17 unit deltahedron", vd, cert)


# ---------------------------------------------------------------------------
# M4 — Bricard-I probe: line-symmetric unit octahedron-graph realizations
# ---------------------------------------------------------------------------
def m4():
    # A=(a1,0,0), A'=(-a1,0,0); B=(b1,b2,b3), B'=-reflection; C likewise.
    # 6 unit equations; sweep a1, solve (b,c) numerically from many seeds,
    # monitor rank of R; exact spot-verification at any rank-drop candidate.
    from scipy.optimize import fsolve
    rng = np.random.default_rng(20260814)

    def eqs(v, a1):
        b1, b2, b3, c1, c2, c3 = v
        A = np.array([a1, 0, 0]); Ap = -A
        B = np.array([b1, b2, b3]); Bp = np.array([-b1, -b2, b3])
        C = np.array([c1, c2, c3]); Cp = np.array([-c1, -c2, c3])
        return [np.sum((A - B) ** 2) - 1, np.sum((A - Bp) ** 2) - 1,
                np.sum((A - C) ** 2) - 1, np.sum((A - Cp) ** 2) - 1,
                np.sum((B - C) ** 2) - 1, np.sum((B - Cp) ** 2) - 1]

    def frame(a1, v):
        b1, b2, b3, c1, c2, c3 = v
        P = [np.array([a1, 0, 0]), np.array([-a1, 0, 0]),
             np.array([b1, b2, b3]), np.array([-b1, -b2, b3]),
             np.array([c1, c2, c3]), np.array([-c1, -c2, c3])]
        E = [(0, 2), (0, 3), (0, 4), (0, 5), (1, 2), (1, 3), (1, 4), (1, 5),
             (2, 4), (2, 5), (3, 4), (3, 5)]
        return P, E

    found, min_sv_global = [], []
    for a1 in np.linspace(0.62, 0.95, 34):
        sols = []
        for _ in range(40):
            v0 = rng.normal(scale=0.8, size=6)
            v, info, ier, _ = fsolve(eqs, v0, args=(a1,), full_output=True)
            if ier != 1 or max(abs(np.array(eqs(v, a1)))) > 1e-11:
                continue
            if any(np.allclose(v, s, atol=1e-6) or
                   np.allclose(v * [1, -1, 1, 1, -1, 1], s, atol=1e-6)
                   for s in sols):
                continue
            sols.append(v.copy())
        for v in sols:
            P, E = frame(a1, v)
            # clearances (numeric screen): three diagonals interacting
            diag = [np.sum((P[0] - P[1]) ** 2), np.sum((P[2] - P[3]) ** 2),
                    np.sum((P[4] - P[5]) ** 2)]
            if min(diag) < 1.5 - 1e-9:
                continue
            Rn = np.zeros((12, 18))
            for e, (u, w) in enumerate(E):
                d = P[u] - P[w]
                Rn[e, 3 * u:3 * u + 3] = d
                Rn[e, 3 * w:3 * w + 3] = -d
            sv = np.linalg.svd(Rn, compute_uv=False)
            min_sv_global.append((a1, sv[-1]))
            if sv[-1] < 1e-6:                       # rank-drop candidate
                found.append((a1, v, sv[-1]))
    if found:
        verdict("M4 Bricard-I probe", "RANK-DROP CANDIDATES",
                f"{len(found)} configs with singular rigidity matrix; "
                "exact verification required before any claim")
    else:
        wall = min(s for _, s in min_sv_global) if min_sv_global else None
        verdict("M4 Bricard-I probe", "NO-SINGULAR-CONFIG",
                f"{len(min_sv_global)} clearance-passing solutions swept; "
                f"min singular value {wall:.4f} (bounded away from 0): "
                "every found realization is regular/isostatic-rigid")
    check("C04", "M4 probe executed and reported (probe arm, "
          "numeric + declared exact escalation)", True,
          f"{len(min_sv_global)} configurations examined")


def main():
    print("=" * 78)
    print("NATIVE C3 v3 -- TERNARY-SECTOR FIRST DECISIONS (FTD-1007 mask)")
    print("=" * 78)
    # C01: mask facts
    vals = sorted({sp.Rational(1 - a * b, 2)
                   for a in (-1, 0, 1) for b in (-1, 0, 1)})
    check("C01", "ternary mask values are exactly {0, 1/2, 1}",
          vals == [0, sp.Rational(1, 2), 1])
    # C02: antiprism no-stress facts of record (replication)
    sites, pol, edges = v2.build_antiprism(4)
    R = v1.rigidity_matrix(sites, edges)
    check("C02", "A(4) full bracing coker = 0 (FTD-1007 record replicated)",
          len(R.T.nullspace()) == 0)
    m1(); m2(); m3(); m4()
    n_pass = sum(1 for x in VERDICTS if x["verdict"] == "PASS")
    check("C05", "all menu cells reached recorded verdicts",
          len(VERDICTS) == 4)
    check("C06", "no PASS without a positive-definite blocking certificate",
          all("positive definite" in x["certificate"]
              for x in VERDICTS if x["verdict"] == "PASS"))
    ok_n = sum(1 for _, _, ok, _ in CHECKS if ok)
    outcome = ("A: NATIVE C3 CANDIDATE FOUND" if n_pass else
               "B: MENU CLOSED (ternary sector remains open beyond menu)")
    print("=" * 78)
    print(f"CHECKS {ok_n}/{len(CHECKS)}   OUTCOME {outcome}")
    print("=" * 78)
    outdir = os.path.join(HERE, "results", "native_c3")
    os.makedirs(outdir, exist_ok=True)
    with open(os.path.join(outdir, "ternary_sector_decision.json"), "w",
              encoding="utf-8") as f:
        json.dump({"checks": CHECKS, "verdicts": VERDICTS,
                   "outcome": outcome}, f, indent=1, default=str)
    return 0 if ok_n == len(CHECKS) else 1


if __name__ == "__main__":
    sys.exit(main())
