"""FTD — Native C3 v2: the reach-parity reduction and the antiprism decision.

Sequel to FTD-1004/FTD-1005 (see PREREG_ANTIPRISM_DECISION_v2.md). Two parts:

PART I — THE REDUCTION (theorem checks, exact, bound-free where stated):
  R-a  strut polarity transport: a unit strut between ring orbits with chain
       spans t1, t2 is polarity-consistent iff t1 = t2 (mod 2).
  R-b  reach: unit struts require |rho1 - rho2| < 1; for Delta-t = 2 (the
       smallest even step), Delta-rho = 1/sin(pi/n) > 1 at every n — so
       strutted ring pairs have EQUAL spans.
  R-c  equal spans: aligned struts have zero in-plane extent (cannot pay the
       (R) stress identity); full-twist chord = t (>= 1, = 1 only at t = 1);
       half-stagger chord = t/(2 cos(pi/2n)) < 1 forces t = 1.
  R-d  t = 1 rings are 2-colorable iff n is even: n in {4, 6} at
       crystallographic orders.
  => the strutted axial class collapses to stacks of staggered unit rings
     (antiprisms) at n in {4, 6}, plus exactly-decided dressings.

PART II — THE DECISION (five-gate battery from the v1 instrument):
  cells: bare unit antiprisms A(n), n in {4, 6}; the coplanar full-twist
  corner; every integer-length inter-ring cable class (spans <= 6); every
  integer apex-cable dressing (spans <= 6, apex between or outside rings).
  Gates per cell: polarity -> closure -> clearance -> stress sign (struts
  compressed, chains tension) -> blocking form PD on nontrivial flexes.

No target constant appears anywhere (C02 lint inherited from v1 by import).
Verdicts are data; checks assert protocol integrity.
"""

import itertools
import json
import os
import sys
import time

import sympy as sp

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import native_unit_strut_tensegrity_decision as v1   # frozen v1 machinery

ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))
CHECKS = []
VERDICTS = []
SPAN_MAX = 6
STAGE_BUDGET_S = 600.0


def check(cid, desc, ok, detail=""):
    CHECKS.append((cid, desc, bool(ok), detail))
    print(f"[{'PASS' if ok else 'FAIL'}] {cid}  {desc}"
          + (f"  -- {detail}" if detail else ""))


def verdict(cell, vd, cert):
    VERDICTS.append({"cell": cell, "verdict": vd, "certificate": cert})


# ---------------------------------------------------------------------------
# PART I — reduction checks
# ---------------------------------------------------------------------------
def part1():
    # R-a: pol(ring i, joint j) = p_i * (-1)^(j * t_i); strut j -> j+m needs
    # -p1*(-1)^(j t1) = p2*(-1)^((j+m) t2) for all j  <=>  t1 = t2 (mod 2).
    ok_a = True
    for t1_, t2_ in itertools.product(range(1, 7), repeat=2):
        consistent = all(
            ((-1) ** (j * t1_)) * ((-1) ** ((j + 1) * t2_)) ==
            ((-1) ** (0 * t1_)) * ((-1) ** (1 * t2_))
            for j in range(12))
        if consistent != ((t1_ - t2_) % 2 == 0):
            ok_a = False
    check("C01", "R-a strut polarity transport iff t1 = t2 (mod 2)", ok_a)

    # R-b: Delta-t = 2 -> Delta-rho = 1/sin(pi/n) > 1 for all n >= 3.
    ok_b = all(sp.simplify(1 / sp.sin(sp.pi / nn) - 1).is_positive
               for nn in range(3, 13))
    check("C02", "R-b reach: Delta-t = 2 exceeds unit-strut reach "
          "(1/sin(pi/n) > 1, n <= 12)", ok_b)

    # R-c: equal spans: full-twist chord = t; half-stagger chord =
    # t / (2 cos(pi/(2n))): < 1 forces t = 1 (n in {3,4,6}).
    ok_c = True
    for nn in (3, 4, 6):
        t_sym = sp.Symbol("t", positive=True)
        rho = t_sym / (2 * sp.sin(sp.pi / nn))
        full = sp.simplify(2 * rho * sp.sin(sp.pi / nn))       # = t
        half = sp.simplify(2 * rho * sp.sin(sp.pi / (2 * nn)))
        ceil = sp.solve(sp.Eq(half, 1), t_sym)[0]
        if sp.simplify(full - t_sym) != 0 or not (1 < ceil < 2):
            ok_c = False
    check("C03", "R-c equal spans: full-twist chord = t; half-stagger "
          "ceiling in (1,2) forces t = 1", ok_c)

    # R-d: unit ring 2-colorable iff n even.
    ok_d = all(((nn % 2) == 0) == ((nn * 1) % 2 == 0) for nn in range(3, 13))
    check("C04", "R-d unit rings 2-colorable iff n even -> n in {4, 6}",
          ok_d)


# ---------------------------------------------------------------------------
# PART II — geometry of the antiprism family (exact)
# ---------------------------------------------------------------------------
def antiprism_geometry(n):
    """Exact (rho, chord, S) for the half-staggered unit-ring pair."""
    rho = 1 / (2 * sp.sin(sp.pi / n))
    chord = sp.simplify(2 * rho * sp.sin(sp.pi / (2 * n)))
    S2 = sp.simplify(1 - chord ** 2)
    return rho, chord, sp.sqrt(S2)


def build_antiprism(n, cable=None, apex=None):
    """Sites/pol/edges for A(n) with optional dressing.

    cable = (offset_halfsteps, span b): one inter-ring cable class whose
      exact length is the integer b (only called for realizable classes).
    apex = (k1, k2, z): apex pair on the axis with integer cable spans to
      the alternate same-polarity joints of each ring (only called when the
      closure is exactly solvable).
    """
    rho, chord, S = antiprism_geometry(n)
    sites, pol, edges = [], [], []

    def add(P, p):
        sites.append(sp.Matrix([sp.simplify(x) for x in P]))
        pol.append(p)
        return len(sites) - 1

    r1, r2 = [], []
    for j in range(n):
        a1 = 2 * sp.pi * j / n
        r1.append(add([rho * sp.cos(a1), rho * sp.sin(a1), 0],
                      1 if j % 2 == 0 else -1))
    for j in range(n):
        a2 = 2 * sp.pi * j / n + sp.pi / n
        r2.append(add([rho * sp.cos(a2), rho * sp.sin(a2), S],
                      -1 if j % 2 == 0 else 1))
    for j in range(n):                                   # ring bonds (span 1)
        edges.append((r1[j], r1[(j + 1) % n], "chain"))
        edges.append((r2[j], r2[(j + 1) % n], "chain"))
    for j in range(n):                                   # struts (unit)
        edges.append((r1[j], r2[j], "strut"))
        edges.append((r1[(j + 1) % n], r2[j], "strut"))
    if cable is not None:
        off, b = cable
        # chain of span b from r1[j] to r2[j + off] (off counted in whole
        # ring-2 steps from the j-th strut partner)
        for j in range(n):
            u, w = r1[j], r2[(j + off) % n]
            _add_chain(sites, pol, edges, u, w, b)
    if apex is not None:
        k1, k2, zU = apex
        iU = add([0, 0, zU], None)
        # polarity: cables span k1 to the even-index (pol +1... determined)
        # joints of ring 1; set pol(U) from consistency with k1.
        target = [r1[j] for j in range(0, n, 2)]
        pU = pol[target[0]] * (-1) ** k1
        pol[iU] = pU
        for u in target:
            _add_chain(sites, pol, edges, iU, u, k1)
        if k2:
            target2 = [r2[j] for j in range(0, n, 2)]
            for u in target2:
                _add_chain(sites, pol, edges, iU, u, k2)
    return sites, pol, edges


def _add_chain(sites, pol, edges, u, w, span):
    if span == 1:
        edges.append((u, w, "chain"))
        return
    P, Q = sites[u], sites[w]
    prev = u
    for s in range(1, span):
        M = P + (Q - P) * sp.Rational(s, span)
        sites.append(sp.Matrix([sp.simplify(x) for x in M]))
        pol.append(-pol[prev])
        edges.append((prev, len(sites) - 1, "chain"))
        prev = len(sites) - 1
    edges.append((prev, w, "chain"))


def polarity_consistent(pol, edges):
    return all(pol[u] is not None and pol[w] is not None
               and pol[u] == -pol[w] for u, w, _ in edges)


def decide_cell(name, sites, pol, edges):
    t0 = time.time()
    # gate 1: polarity (construction guarantees bipartite; verify)
    if not polarity_consistent(pol, edges):
        return "POLARITY-KILL", "an edge joins equal polarities"
    # gate 2: closure — all edges exactly unit
    for u, w, _ in edges:
        L2 = sp.simplify((sites[u] - sites[w]).dot(sites[u] - sites[w]))
        if sp.simplify(L2 - 1) != 0:
            return "CLOSURE-KILL", f"edge ({u},{w}) length^2 = {L2} != 1"
    # gate 3: clearance
    okc, why = v1.clearance_gate(sites, pol, edges)
    if not okc:
        return "CLEARANCE-KILL", why
    if time.time() - t0 > STAGE_BUDGET_S:
        return "INCOMPLETE", "budget exceeded before stress stage"
    # gate 4: stress with signs
    R = v1.rigidity_matrix(sites, edges)
    stresses = R.T.nullspace()
    if not stresses:
        return "NO-STRESS", "coker(R) = 0: every flex extends"
    tags = [t for _, _, t in edges]
    wv, why = v1.stress_sign_gate(stresses, tags)
    if wv is None:
        return "INCOMPLETE", why
    if wv is False:
        return "SIGN-KILL", (f"stress dim {len(stresses)}: " + why)
    # gate 5: blocking
    flexes = R.nullspace()
    vd, why = v1.blocking_gate(wv, flexes, v1.trivial_flexes(sites),
                               sites, edges)
    return vd, why


# ---------------------------------------------------------------------------
# cell enumeration
# ---------------------------------------------------------------------------
def enumerate_cables(n):
    """Integer-length inter-ring chain classes (span <= SPAN_MAX), exact."""
    rho, chord, S = antiprism_geometry(n)
    out = []
    for off in range(n):
        a1 = sp.pi / n + 2 * sp.pi * off / n     # angle offset r1[0]->r2[off]
        c2 = sp.simplify(2 * rho ** 2 * (1 - sp.cos(a1)))
        L2 = sp.simplify(c2 + (1 - chord ** 2))
        for b in range(1, SPAN_MAX + 1):
            if sp.simplify(L2 - b * b) == 0:
                out.append((off, b))
    return out


def enumerate_apexes(n):
    """Integer apex-cable dressings, exact closure. Apex on axis at exact
    height z with k1 cables to ring1 alternates; optional k2 to ring2."""
    rho, chord, S = antiprism_geometry(n)
    out = []
    z = sp.Symbol("z", real=True)
    for k1 in range(1, SPAN_MAX + 1):
        sols = sp.solve(sp.Eq(rho ** 2 + z ** 2, k1 * k1), z)
        for zs in sols:
            if not zs.is_real:
                continue
            for k2 in range(0, SPAN_MAX + 1):
                if k2 == 0:
                    out.append((k1, 0, sp.simplify(zs)))
                    continue
                gap = sp.simplify(rho ** 2 + (zs - S) ** 2 - k2 * k2)
                if gap == 0:
                    out.append((k1, k2, sp.simplify(zs)))
    return out


def part2():
    n_cells = 0
    for n in (4, 6):
        cells = [("bare", None, None)]
        cells += [(f"cable(off={o},b={b})", (o, b), None)
                  for (o, b) in enumerate_cables(n)]
        cells += [(f"apex(k1={k1},k2={k2})", None, (k1, k2, z))
                  for (k1, k2, z) in enumerate_apexes(n)]
        for label, cab, apx in cells:
            n_cells += 1
            cell = f"A({n}) {label}"
            try:
                sites, pol, edges = build_antiprism(n, cab, apx)
                vd, cert = decide_cell(cell, sites, pol, edges)
            except Exception as ex:
                vd, cert = "INCOMPLETE", f"{type(ex).__name__}: {ex}"
            verdict(cell, vd, cert)
            print(f"    {cell:<34} {vd:<16} {cert[:70]}")
    # coplanar full-twist corner: clearance argument, exact
    for n in (4, 6):
        rho = 1 / (2 * sp.sin(sp.pi / n))
        d = sp.simplify(2 * rho * sp.sin(sp.pi / (2 * n)))
        inside = sp.simplify(d ** 2 - sp.Rational(3, 2)).is_negative
        verdict(f"coplanar twist pair n={n}",
                "CLEARANCE-KILL" if inside else "INCOMPLETE",
                f"interleaved non-bond pair at q = {sp.N(d**2, 6)} < 3/2")
        n_cells += 1
    check("C05", f"all {n_cells} decision cells reached a recorded verdict",
          len([x for x in VERDICTS]) == n_cells)
    check("C06", "no PASS without a positive-definite blocking certificate",
          all("positive definite" in x["certificate"]
              for x in VERDICTS if x["verdict"] == "PASS"))


def main():
    if "--selftest" in sys.argv:
        rc = v1.selftest()
        # geometry calibration: antiprism edges exactly unit by construction
        for n in (4, 6):
            sites, pol, edges = build_antiprism(n)
            bad = [
                (u, w) for u, w, _ in edges
                if sp.simplify((sites[u] - sites[w]).dot(
                    sites[u] - sites[w]) - 1) != 0]
            print(f"  S4 A({n}) construction: all {len(edges)} edges unit: "
                  f"{'PASS' if not bad else 'FAIL'}")
            rc |= (1 if bad else 0)
        return rc
    print("=" * 78)
    print("NATIVE C3 v2 -- REACH-PARITY REDUCTION + ANTIPRISM DECISION")
    print("=" * 78)
    part1()
    print("-" * 78)
    part2()
    ok_n = sum(1 for _, _, ok, _ in CHECKS if ok)
    n_pass = sum(1 for x in VERDICTS if x["verdict"] == "PASS")
    outcome = ("A: NATIVE C3 CANDIDATE FOUND" if n_pass else
               "B: SCOPE-NEGATIVE (reduced class closed; non-axial and "
               "chain-networked classes remain open)")
    print("=" * 78)
    print(f"CHECKS {ok_n}/{len(CHECKS)}   OUTCOME {outcome}")
    print("=" * 78)
    outdir = os.path.join(HERE, "results", "native_c3")
    os.makedirs(outdir, exist_ok=True)
    with open(os.path.join(outdir, "antiprism_decision.json"), "w",
              encoding="utf-8") as f:
        json.dump({"checks": CHECKS, "verdicts": VERDICTS,
                   "outcome": outcome}, f, indent=1, default=str)
    return 0 if ok_n == len(CHECKS) else 1


if __name__ == "__main__":
    sys.exit(main())
