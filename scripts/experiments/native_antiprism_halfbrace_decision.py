"""FTD — Native C3 v2.1: the half-braced staggered pair (last axial cell).

The v2 run (PREREG_ANTIPRISM_DECISION_v2.md) closed its declared scope with
every cell at POLARITY-KILL, and localized the obstruction exactly: full
antiprism bracing forms triangles r1[j]-r1[j+1]-r2[j], and the polarity mask
makes the interaction graph bipartite, hence triangle-free. The one
triangle-free bracing of the forced staggered unit-ring geometry — a SINGLE
strut per joint pair, r1[j]-r2[j] — is polarity-consistent and was outside
the v2 scope. This campaign decides it (see PREREG_ANTIPRISM_DECISION_v2_1.md).

Declared expectation (disclosed): single tilted struts carry an unbalanced
tangential component at each joint that uniform ring tension cannot cancel,
so the stress gate is expected to return NO-STRESS — but no computation of
coker(R) for this structure has ever been run; the verdict is live.

Cells: bare H(4), H(6); apex dressings with exact integer closure (spans <= 6);
inter-ring cable classes with exact integer length excluding strut-coincident
offsets (none exist at spans <= 6 per the v2 enumeration; re-derived here).
Gates and machinery identical to v1/v2 (both imported byte-frozen).
"""

import itertools
import json
import os
import sys

import sympy as sp

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import native_unit_strut_tensegrity_decision as v1
import native_antiprism_decision as v2

CHECKS = []
VERDICTS = []
SPAN_MAX = 6


def check(cid, desc, ok, detail=""):
    CHECKS.append((cid, desc, bool(ok), detail))
    print(f"[{'PASS' if ok else 'FAIL'}] {cid}  {desc}"
          + (f"  -- {detail}" if detail else ""))


def build_halfbrace(n, cable=None, apex=None):
    """Staggered unit-ring pair with SINGLE struts r1[j]-r2[j]."""
    rho, chord, S = v2.antiprism_geometry(n)
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
    for j in range(n):
        edges.append((r1[j], r1[(j + 1) % n], "chain"))
        edges.append((r2[j], r2[(j + 1) % n], "chain"))
        edges.append((r1[j], r2[j], "strut"))            # single brace only
    if cable is not None:
        off, b = cable
        for j in range(n):
            v2._add_chain(sites, pol, edges, r1[j], r2[(j + off) % n], b)
    if apex is not None:
        k1, k2, zU = apex
        iU = add([0, 0, zU], None)
        target = [r1[j] for j in range(0, n, 2)]
        pol[iU] = pol[target[0]] * (-1) ** k1
        for u in target:
            v2._add_chain(sites, pol, edges, iU, u, k1)
        if k2:
            for u in [r2[j] for j in range(0, n, 2)]:
                v2._add_chain(sites, pol, edges, iU, u, k2)
    return sites, pol, edges


def enumerate_cables(n):
    """Integer inter-ring chain classes, excluding the strut offset (0)."""
    rho, chord, S = v2.antiprism_geometry(n)
    out = []
    for off in range(1, n):
        ang = sp.pi / n + 2 * sp.pi * off / n
        c2 = sp.simplify(2 * rho ** 2 * (1 - sp.cos(ang)))
        L2 = sp.simplify(c2 + S ** 2)
        for b in range(1, SPAN_MAX + 1):
            if sp.simplify(L2 - b * b) == 0:
                out.append((off, b))
    return out


def main():
    if "--selftest" in sys.argv:
        rc = 0
        for n in (4, 6):
            sites, pol, edges = build_halfbrace(n)
            bad_len = [e for e in edges if sp.simplify(
                (sites[e[0]] - sites[e[1]]).dot(
                    sites[e[0]] - sites[e[1]]) - 1) != 0]
            bad_pol = [e for e in edges if pol[e[0]] != -pol[e[1]]]
            print(f"  S1 H({n}): {len(edges)} edges unit: "
                  f"{'PASS' if not bad_len else 'FAIL'}; bipartite: "
                  f"{'PASS' if not bad_pol else 'FAIL'}")
            rc |= 1 if (bad_len or bad_pol) else 0
        return rc

    print("=" * 78)
    print("NATIVE C3 v2.1 -- HALF-BRACED STAGGERED PAIR (last axial cell)")
    print("=" * 78)
    n_cells = 0
    for n in (4, 6):
        cells = [("bare", None, None)]
        cells += [(f"cable(off={o},b={b})", (o, b), None)
                  for (o, b) in enumerate_cables(n)]
        cells += [(f"apex(k1={k1},k2={k2})", None, (k1, k2, z))
                  for (k1, k2, z) in v2.enumerate_apexes(n)]
        for label, cab, apx in cells:
            n_cells += 1
            cell = f"H({n}) {label}"
            try:
                sites, pol, edges = build_halfbrace(n, cab, apx)
                vd, cert = v2.decide_cell(cell, sites, pol, edges)
            except Exception as ex:
                vd, cert = "INCOMPLETE", f"{type(ex).__name__}: {ex}"
            VERDICTS.append({"cell": cell, "verdict": vd,
                             "certificate": cert})
            print(f"    {cell:<34} {vd:<16} {cert[:72]}")
    check("C01", f"all {n_cells} cells reached a recorded verdict",
          len(VERDICTS) == n_cells)
    check("C02", "no PASS without a positive-definite blocking certificate",
          all("positive definite" in x["certificate"]
              for x in VERDICTS if x["verdict"] == "PASS"))
    n_pass = sum(1 for x in VERDICTS if x["verdict"] == "PASS")
    ok_n = sum(1 for _, _, ok, _ in CHECKS if ok)
    outcome = ("A: NATIVE C3 CANDIDATE FOUND" if n_pass else
               "B: STRUTTED AXIAL CLASS FULLY CLOSED")
    print("=" * 78)
    print(f"CHECKS {ok_n}/{len(CHECKS)}   OUTCOME {outcome}")
    print("=" * 78)
    outdir = os.path.join(HERE, "results", "native_c3")
    os.makedirs(outdir, exist_ok=True)
    with open(os.path.join(outdir, "antiprism_halfbrace_decision.json"),
              "w", encoding="utf-8") as f:
        json.dump({"checks": CHECKS, "verdicts": VERDICTS,
                   "outcome": outcome}, f, indent=1, default=str)
    return 0 if ok_n == len(CHECKS) else 1


if __name__ == "__main__":
    sys.exit(main())
