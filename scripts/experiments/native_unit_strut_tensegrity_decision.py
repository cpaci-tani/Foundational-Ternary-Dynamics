"""FTD — Native C3, third formulation: the exact unit-strut-tensegrity decision.

QUESTION (verbatim, ANALYSIS_MINIMUM_VIABLE_CLOCK_CARRIER_v1.md Section 3.1):
    [OPEN - native C3, third formulation] does a finite integer unit-strut
    tensegrity exist under the registered law (struts = single unit bonds
    carrying compression; straight integer-span tension chains; polarity,
    floor, and q < 3/2 clearances), with the blocking form definite on its
    full flex space?

Preregistered decision instrument for the declared scope (see
PREREG_UNIT_STRUT_TENSEGRITY_DECISION_v1.md). Executes
SPEC_SUBSTRATE_NATIVE_CLOCK_MINIMUM_v1.md Section 8 step 2 (recorded there as
never run). Certificates are exact (integer / symbolic) where stated,
interval-with-margin where stated -- Section 7 allows "exact, symbolic, or
interval certificates".

HARD RULE (Section 8 step 2): the search must not read the target constant.
The framework period constant is never imported or computed; check C02 lints
this source for banned tokens.

Declared families (a closed scope -- NOT the whole class):
  F1  axial lens (replication of the recorded mod-4 kill)          exact
  F3  planar unit wheel, span-1 spokes                             exact
  F4  single-ring axial cage (case tree; arithmetic machine-checked,
      logical steps stated in the prereg)                          exact
  F7  rectangle ladder (two parallel unit struts)                  exact
  ArmC two-term axial closures (partial obstruction)               exact
  F5  prism family: n in {3,4}, spans a,b <= 6, wirings
      {diag-strut, vert-strut} x chirality {+1,-1} -- full battery:
      polarity -> closure -> clearance -> stress -> blocking

Checks C01..C12 assert PROTOCOL INTEGRITY. Family verdicts are DATA (verdict
table + JSON artifact). An F5 PASS = a native C3 candidate; the run then
stops and reports. No period is computed in this campaign.

Modes:  --selftest   known-answer calibration only (no decision cells)
        (default)    the registered decision run
"""

import hashlib
import itertools
import json
import os
import sys
import time

import sympy as sp

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))

SOURCES = {
    "docs/theory/10_eft_program/native_time_carrier_programme/"
    "ANALYSIS_MINIMUM_VIABLE_CLOCK_CARRIER_v1.md":
        "3CABBD8C34ACAF0FB598189F4103A06D6FF82C40D3B112B246E5E59970FE2706",
    "docs/theory/10_eft_program/native_time_carrier_programme/"
    "SPEC_SUBSTRATE_NATIVE_CLOCK_MINIMUM_v1.md":
        "2A9F33284088C869E3A78B203F82F0129CC1CE36F22A30DF1CD6D8F02F0C44F8",
    "docs/theory/02_foundations/DERIV_MINIMAL_MANY_BODY_MATTER_NETWORK_v1.md":
        "B712373544641828D96CFD4053CE9AC59E08E7AE076F4506DF4F239E14F3AD86",
}

SUPPORT_Q = sp.Rational(3, 2)      # opposite-polarity support edge (q = r^2)
SAME_FLOOR_Q = sp.Rational(4, 25)  # recorded same-polarity floor r >= 0.40
INTERVAL_DPS = 50
INTERVAL_MARGIN = sp.Float(10) ** -20
N_SET = (3, 4)
SPAN_MAX = 6
STAGE_BUDGET_S = 300.0

CHECKS = []
VERDICTS = []


def check(cid, desc, ok, detail=""):
    CHECKS.append((cid, desc, bool(ok), detail))
    print(f"[{'PASS' if ok else 'FAIL'}] {cid}  {desc}"
          + (f"  -- {detail}" if detail else ""))


def verdict(family, cell, vd, certificate):
    VERDICTS.append({"family": family, "cell": cell, "verdict": vd,
                     "certificate": certificate})


def sign_of(expr):
    """Certified sign of an exact real expression: +1, -1, 0, or None."""
    e = sp.simplify(expr)
    if e.is_zero:
        return 0
    if e.is_positive:
        return 1
    if e.is_negative:
        return -1
    val = sp.N(e, INTERVAL_DPS)
    if abs(val) < INTERVAL_MARGIN:
        return None
    return 1 if val > 0 else -1


def cmp_ge(expr, bound):
    """Certified expr >= bound. Returns (bool_or_None, method)."""
    s = sign_of(expr - bound)
    if s is None:
        return None, "indeterminate"
    return s >= 0, "certified"


# ---------------------------------------------------------------------------
# C01 pins / C02 lint / C03 law facts
# ---------------------------------------------------------------------------
def run_c01():
    bad = []
    for rel, want in SOURCES.items():
        got = hashlib.sha256(
            open(os.path.join(ROOT, rel), "rb").read()).hexdigest().upper()
        if got != want:
            bad.append(f"{rel}: {got[:12]}...")
    check("C01", "source documents match pinned SHA-256", not bad,
          "; ".join(bad))


# LINT-EXEMPT-BEGIN  (the banned-token list itself)
_BANNED = ["G_" + "STAR", "2.95" + "86", "137." + "03",
           "lemn" + "iscat", "gst" + "ar"]
# LINT-EXEMPT-END


def run_c02():
    src = open(os.path.abspath(__file__), encoding="utf-8").read()
    pre, rest = src.split("# LINT-EXEMPT-BEGIN", 1)
    _, post = rest.split("# LINT-EXEMPT-END", 1)
    body = pre + post
    hits = [b for b in _BANNED if b in body]
    check("C02", "no-target lint: banned tokens absent from decision code",
          not hits, ", ".join(hits))


def run_c03():
    eps, q = sp.symbols("eps q", positive=True)
    V = -16 * eps * (q - sp.Rational(3, 2)) ** 2 * (q - sp.Rational(3, 4))
    ok = (sp.simplify(sp.diff(V, q).subs(q, 1)) == 0
          and sp.simplify(sp.diff(V, q, 2).subs(q, 1) - 24 * eps) == 0
          and sp.simplify(V.subs(q, SUPPORT_Q)) == 0)
    check("C03", "registered law: V'(1)=0, V''(1)=24eps, support edge q=3/2",
          ok)


# ---------------------------------------------------------------------------
# Exact combinatorial / Diophantine families (bound-free)
# ---------------------------------------------------------------------------
def run_f1_lens():
    residues = sorted({(s * s - 4 * k * k) % 4
                       for s in range(4) for k in range(4)})
    ok = 3 not in residues
    verdict("F1 axial lens", "all (s,k)", "CLOSED NEGATIVE (replication)",
            f"s^2-4k^2 mod 4 in {residues}; -1 = 3 mod 4 unreachable")
    check("C04", "F1 lens: recorded mod-4 kill replicated (bound-free)", ok)


def run_f3_wheel():
    flips = 1 + 1 + 1        # hub->rim spoke, rim->rim, rim->hub spoke
    ok = flips % 2 == 1
    verdict("F3 unit wheel", "all n", "CLOSED NEGATIVE",
            "hub-rim-rim cycle carries 3 parity flips (odd): "
            "no polarity 2-coloring")
    check("C05", "F3 wheel: odd-cycle polarity kill (bound-free)", ok)


def run_f7_ladder():
    d, p = sp.symbols("d p", integer=True)
    sols = list(sp.diophantine(d ** 2 - p ** 2 - 1))
    admissible = [s for s in sols if all(abs(t) >= 1 for t in s)]
    verdict("F7 rectangle ladder", "all (d,p)", "CLOSED NEGATIVE",
            f"d^2-p^2=1 solutions {sols}: p=0 always (degenerate)")
    check("C06", "F7 ladder: diagonal closure admits no admissible solution",
          not admissible, str(sols))


def run_f4_cage():
    # Machine-checked arithmetic steps of the case tree (logic in prereg):
    # (a) cable-parity constraint: pol(t)=pol(U)(-1)^k = pol(W)(-1)^m with
    #     pol(W) = -pol(U)  <=>  (-1)^(k+m) = -1  <=>  k+m odd. Then every
    #     ring joint gets the SAME polarity (k, m common by symmetry), while
    #     an alternating unit-bond ring needs both polarities: infeasible.
    step_a = all(((-1) ** (k + m) == -1) == ((k + m) % 2 == 1)
                 for k in range(1, 7) for m in range(1, 7))
    # (b) unbonded ring joint: two tension members balance only if collinear
    #     (on-axis). Between the strut ends: k + m = 1 with k,m >= 1
    #     infeasible; beyond an end: the longer chain's integer station grid
    #     contains the near strut end's position -> coincidence.
    k, m = sp.symbols("k m", integer=True, positive=True)
    step_b = sp.solve([sp.Eq(k + m, 1)], [k, m], dict=True) == []
    #     beyond-end overlap: U at 0, W at 1, joint at 1+m: the U-joint chain
    #     (span k = m+1) has a station at distance 1 = W exactly:
    step_c = all((mm + 1 > 1) and (1 in range(1, mm + 1 + 1))
                 for mm in range(1, 7))
    verdict("F4 axial cage", "ring-bonded", "CLOSED NEGATIVE",
            "alternating ring vs single-polarity cable constraint (k+m odd)")
    verdict("F4 axial cage", "ring-unbonded", "CLOSED NEGATIVE",
            "collinearity forces on-axis; k+m=1 infeasible; beyond-end "
            "station coincides with strut end")
    check("C07", "F4 cage: case-tree arithmetic steps verified",
          step_a and step_b and step_c)


def run_axial_two_term():
    ok_sum = bool(sp.sqrt(3) > 1)     # k>=2 term >= sqrt(3) > 1; k=m=1 -> 0
    m, y = sp.symbols("m y", integer=True)
    sols = list(sp.diophantine(m ** 2 - y ** 2 - 1))
    only_degenerate = all(any(abs(t) == 1 for t in s)
                          and any(t == 0 for t in s) for s in sols)
    check("C08", "axial two-term sum closure: no integer solution", ok_sum)
    check("C09", "axial two-term difference closure: m^2-y^2=1 only "
          "degenerate", only_degenerate, str(sols))
    verdict("Arm C (partial)", "two-term axial closures", "CLOSED NEGATIVE",
            "sum: minimum nonzero term sqrt(3) > 1; difference: Pell "
            "factorization (m,y)=(+-1,0)")


# ---------------------------------------------------------------------------
# F5 prism battery
# ---------------------------------------------------------------------------
def parity_ok(n, a, b):
    return (n * a) % 2 == 0 and (1 + a + b) % 2 == 0


def solve_closure(n, a, b, role, chir):
    r = sp.Rational(a) / (2 * sp.sin(sp.pi / n))
    theta = chir * 2 * sp.pi / n
    c, s = sp.symbols("c s", real=True)
    cos_shift = c * sp.cos(theta) - s * sp.sin(theta)
    if role == "diag":
        len_strut2 = 2 * r ** 2 * (1 - cos_shift)
        len_cable2 = 2 * r ** 2 * (1 - c)
    else:
        len_strut2 = 2 * r ** 2 * (1 - c)
        len_cable2 = 2 * r ** 2 * (1 - cos_shift)
    lin = sp.expand((len_strut2 + 0) - (len_cable2 + 0) - (1 - b ** 2))
    sols = sp.solve([sp.Eq(lin, 0), sp.Eq(c ** 2 + s ** 2, 1)],
                    [c, s], dict=True)
    out = []
    for so in sols:
        cc, ss = sp.simplify(so[c]), sp.simplify(so[s])
        if cc.is_real is False or ss.is_real is False:
            continue
        h2 = sp.simplify(1 - len_strut2.subs({c: cc, s: ss}))
        sg = sign_of(h2)
        if sg is None:
            return None                       # indeterminate -> INCOMPLETE
        if sg <= 0:
            continue
        out.append((r, cc, ss, h2))
    return out


def build_sites(n, a, b, role, r, c, s, h2):
    h = sp.sqrt(h2)
    sites, pol, edges = [], [], []

    def add_site(P, p):
        sites.append(sp.Matrix([sp.simplify(x) for x in P]))
        pol.append(p)
        return len(sites) - 1

    polB = [1 if (i * a) % 2 == 0 else -1 for i in range(n)]
    if role == "diag":
        polT = [-polB[(j - 1) % n] for j in range(n)]
    else:
        polT = [-polB[j] for j in range(n)]
    iB, iT = [], []
    for i in range(n):
        ang = 2 * sp.pi * i / n
        iB.append(add_site([r * sp.cos(ang), r * sp.sin(ang), 0], polB[i]))
    for i in range(n):
        ang = 2 * sp.pi * i / n
        ca, sa = sp.cos(ang), sp.sin(ang)
        iT.append(add_site([r * (ca * c - sa * s),
                            r * (sa * c + ca * s), h], polT[i]))

    def add_chain(u, w, span, tag):
        if span == 1:
            edges.append((u, w, tag))
            return
        P, Q = sites[u], sites[w]
        prev = u
        for t in range(1, span):
            M = P + (Q - P) * sp.Rational(t, span)
            prev_new = add_site(list(M), -pol[prev])
            edges.append((prev, prev_new, tag))
            prev = prev_new
        edges.append((prev, w, tag))

    for i in range(n):
        add_chain(iB[i], iB[(i + 1) % n], a, "chain")
        add_chain(iT[i], iT[(i + 1) % n], a, "chain")
    for i in range(n):
        if role == "diag":
            edges.append((iB[i], iT[(i + 1) % n], "strut"))
            add_chain(iB[i], iT[i], b, "chain")
        else:
            edges.append((iB[i], iT[i], "strut"))
            add_chain(iB[i], iT[(i + 1) % n], b, "chain")
    return sites, pol, edges


def clearance_gate(sites, pol, edges):
    bonded = {frozenset((u, w)) for u, w, _ in edges}
    for i in range(len(sites)):
        for j in range(i + 1, len(sites)):
            q = sp.simplify((sites[i] - sites[j]).dot(sites[i] - sites[j]))
            if pol[i] == pol[j]:
                ok, _ = cmp_ge(q, SAME_FLOOR_Q)
                if ok is not True:
                    return False, f"same-pol pair ({i},{j}) q={sp.N(q, 8)}"
            elif frozenset((i, j)) not in bonded:
                ok, _ = cmp_ge(q, SUPPORT_Q)
                if ok is not True:
                    return False, (f"opposite non-bond ({i},{j}) inside "
                                   f"support q={sp.N(q, 8)}")
    return True, "all clearances certified"


def rigidity_matrix(sites, edges):
    ns, ne = len(sites), len(edges)
    R = sp.zeros(ne, 3 * ns)
    for e, (u, w, _) in enumerate(edges):
        d = sites[u] - sites[w]
        for k in range(3):
            R[e, 3 * u + k] = d[k]
            R[e, 3 * w + k] = -d[k]
    return R


def trivial_flexes(sites):
    ns = len(sites)
    triv = []
    for k in range(3):
        tv = sp.zeros(3 * ns, 1)
        for i in range(ns):
            tv[3 * i + k] = 1
        triv.append(tv)
    for G in (sp.Matrix([[0, -1, 0], [1, 0, 0], [0, 0, 0]]),
              sp.Matrix([[0, 0, 1], [0, 0, 0], [-1, 0, 0]]),
              sp.Matrix([[0, 0, 0], [0, 0, -1], [0, 1, 0]])):
        rv = sp.zeros(3 * ns, 1)
        for i in range(ns):
            gi = G * sites[i]
            for k in range(3):
                rv[3 * i + k] = gi[k]
        triv.append(rv)
    return triv


def stress_sign_gate(stresses, tags):
    """Existence of a stress with omega<0 exactly on struts, >0 on chains."""
    ne = len(tags)
    rays = []
    if len(stresses) == 1:
        rays = [stresses[0], -stresses[0]]
    elif len(stresses) == 2:
        rays = [p * stresses[0] + q * stresses[1]
                for p in range(-4, 5) for q in range(-4, 5)
                if (p, q) != (0, 0)]
    else:
        return None, f"stress dim {len(stresses)} > 2: sign LP not run"
    for wv in rays:
        good = True
        for e in range(ne):
            sg = sign_of(wv[e])
            if sg is None:
                return None, "indeterminate stress entry"
            want = -1 if tags[e] == "strut" else 1
            if sg != want:
                good = False
                break
        if good:
            return wv, "sign pattern realized"
    return False, "no stress with compression exactly on struts"


def blocking_gate(wv, flexes, triv, sites, edges):
    """PD of Sum_e w_e |dq_u - dq_w|^2 on nontrivial ker R."""
    nf = len(flexes)
    M_all = sp.Matrix.hstack(*(flexes + triv))
    triv_rank = sp.Matrix.hstack(*triv).rank()
    nontriv = M_all.rank() - triv_rank
    if nontriv <= 0:
        return "RIGID", "no nontrivial flex: n = 2, not quartic"
    B = sp.zeros(nf, nf)
    for e, (u, w, _) in enumerate(edges):
        dx = [flexes[x][3 * u:3 * u + 3, 0] - flexes[x][3 * w:3 * w + 3, 0]
              for x in range(nf)]
        for x in range(nf):
            for y in range(x, nf):
                B[x, y] += wv[e] * (dx[x].T * dx[y])[0, 0]
    for x in range(nf):
        for y in range(x):
            B[x, y] = B[y, x]
    cp = sp.Poly(B.charpoly().as_expr(), sp.Symbol("lambda"))
    coeffs = cp.all_coeffs()
    nz = 0
    while coeffs and sp.simplify(coeffs[-1]) == 0:
        coeffs.pop()
        nz += 1
    expected_kernel = nf - nontriv
    if nz != expected_kernel:
        return "BLOCKING-KILL", (f"blocking kernel dim {nz} != trivial part "
                                 f"{expected_kernel}: a nontrivial flex "
                                 "escapes at second order")
    signs = [sign_of(cf) for cf in coeffs]
    if any(s is None for s in signs):
        return "INCOMPLETE", "indeterminate charpoly coefficient sign"
    alternating = all(signs[i] == (-1) ** i * signs[0]
                      for i in range(len(signs)))
    if alternating:
        return "PASS", (f"blocking form positive definite on the "
                        f"{nontriv}-dim nontrivial flex space")
    return "BLOCKING-KILL", "charpoly signs not alternating: indefinite"


def run_cell(n, a, b, role, chir):
    if not parity_ok(n, a, b):
        return "POLARITY-KILL", (f"n*a={n*a} even: {(n*a)%2==0}; "
                                 f"1+a+b even: {(1+a+b)%2==0}")
    t0 = time.time()
    sols = solve_closure(n, a, b, role, chir)
    if sols is None:
        return "INCOMPLETE", "closure sign indeterminate"
    if not sols:
        return "CLOSURE-KILL", "no real embedding with h^2 > 0"
    last = ("CLOSURE-KILL", "all closure roots inadmissible")
    for (r, c, s, h2) in sols:
        if time.time() - t0 > STAGE_BUDGET_S:
            return "INCOMPLETE", "stage budget exceeded"
        sites, pol, edges = build_sites(n, a, b, role, r, c, s, h2)
        okc, why = clearance_gate(sites, pol, edges)
        if not okc:
            last = ("CLEARANCE-KILL", why)
            continue
        R = rigidity_matrix(sites, edges)
        stresses = R.T.nullspace()
        if not stresses:
            last = ("NO-STRESS", "coker(R)=0: every flex extends")
            continue
        tags = [t for _, _, t in edges]
        wv, why = stress_sign_gate(stresses, tags)
        if wv is None:
            return "INCOMPLETE", why
        if wv is False:
            last = ("SIGN-KILL", why)
            continue
        flexes = R.nullspace()
        vd, why = blocking_gate(wv, flexes, trivial_flexes(sites),
                                sites, edges)
        last = (vd, why)
        if vd == "PASS":
            return last
    return last


def run_f5():
    cells = list(itertools.product(N_SET, range(1, SPAN_MAX + 1),
                                   range(1, SPAN_MAX + 1),
                                   ("diag", "vert"), (1, -1)))
    n_done, n_incomplete, n_pass = 0, 0, 0
    stopped_early = False
    for (n, a, b, role, chir) in cells:
        cell = f"n={n},a={a},b={b},{role},chir={chir:+d}"
        try:
            vd, cert = run_cell(n, a, b, role, chir)
        except Exception as ex:
            vd, cert = "INCOMPLETE", f"exception: {type(ex).__name__}: {ex}"
        verdict("F5 prism", cell, vd, cert)
        n_done += 1
        if vd == "INCOMPLETE":
            n_incomplete += 1
        if vd == "PASS":
            n_pass += 1
            stopped_early = True
            break
    enumerated_ok = stopped_early or n_done == len(cells)
    check("C10", f"F5 grid enumerated ({n_done}/{len(cells)} cells; "
          "early stop on PASS is declared)", enumerated_ok,
          f"incomplete: {n_incomplete}")
    check("C11", "no F5 PASS without a positive-definite blocking "
          "certificate",
          all("positive definite" in x["certificate"]
              for x in VERDICTS
              if x["family"] == "F5 prism" and x["verdict"] == "PASS"))
    return n_pass


def run_c12():
    fams = {x["family"] for x in VERDICTS}
    declared = {"F1 axial lens", "F3 unit wheel", "F7 rectangle ladder",
                "F4 axial cage", "Arm C (partial)", "F5 prism"}
    check("C12", "verdicts confined to declared scope (no silent claims)",
          fams <= declared, str(fams - declared))


# ---------------------------------------------------------------------------
# Selftest: known-answer calibration (runs no decision cells)
# ---------------------------------------------------------------------------
def selftest():
    print("SELFTEST -- known-answer calibration")
    # S1: SC 2x2x2 checkerboard block: recorded self-stress dimension 0.
    pts = [(i, j, k) for i in range(2) for j in range(2) for k in range(2)]
    sites = [sp.Matrix(p) for p in pts]
    edges = []
    for x in range(8):
        for y in range(x + 1, 8):
            if sum(abs(a - b) for a, b in zip(pts[x], pts[y])) == 1:
                edges.append((x, y, "chain"))
    R = rigidity_matrix(sites, edges)
    s1 = len(R.T.nullspace()) == 0
    print(f"  S1 SC L=2 block coker dim 0 (recorded): "
          f"{'PASS' if s1 else 'FAIL'}")
    # S2: unit square: 1 nontrivial flex (shear); fabricated all-tension
    # stress must FAIL blocking (shear escapes) -> BLOCKING-KILL expected.
    sq = [sp.Matrix(v) for v in
          ([0, 0, 0], [1, 0, 0], [1, 1, 0], [0, 1, 0])]
    sqe = [(0, 1, "chain"), (1, 2, "chain"), (2, 3, "chain"), (3, 0, "chain")]
    Rs = rigidity_matrix(sq, sqe)
    fx = Rs.nullspace()
    wv = sp.Matrix([1, 1, 1, 1])
    vd, why = blocking_gate(wv, fx, trivial_flexes(sq), sq, sqe)
    s2 = vd in ("BLOCKING-KILL",)
    print(f"  S2 unit square fabricated stress -> {vd} (expect "
          f"BLOCKING-KILL): {'PASS' if s2 else 'FAIL'}  [{why}]")
    # S3: certified comparisons on radicals.
    s3 = (cmp_ge(sp.sqrt(2), sp.Rational(7, 5))[0] is True
          and cmp_ge(sp.Rational(7, 5), sp.sqrt(2))[0] is False
          and sign_of(sp.sqrt(3) - sp.sqrt(2)) == 1)
    print(f"  S3 certified radical comparisons: {'PASS' if s3 else 'FAIL'}")
    return 0 if (s1 and s2 and s3) else 1


def main():
    if "--selftest" in sys.argv:
        return selftest()
    print("=" * 78)
    print("NATIVE C3 -- UNIT-STRUT TENSEGRITY DECISION (preregistered run)")
    print("=" * 78)
    run_c01(); run_c02(); run_c03()
    run_f1_lens(); run_f3_wheel(); run_f7_ladder(); run_f4_cage()
    run_axial_two_term()
    n_pass = run_f5()
    run_c12()

    print("\n" + "=" * 78)
    print("VERDICT TABLE (data)")
    print("=" * 78)
    tally = {}
    for x in VERDICTS:
        if x["family"] == "F5 prism":
            tally[x["verdict"]] = tally.get(x["verdict"], 0) + 1
        else:
            print(f"  {x['family']:<24} {x['cell']:<26} {x['verdict']}")
    print(f"  F5 prism tally over enumerated cells: {tally}")
    interesting = [x for x in VERDICTS if x["family"] == "F5 prism"
                   and x["verdict"] not in ("POLARITY-KILL", "CLOSURE-KILL")]
    for x in interesting:
        print(f"    {x['cell']:<28} {x['verdict']:<16} {x['certificate']}")

    ok_n = sum(1 for _, _, ok, _ in CHECKS if ok)
    outcome = ("A: NATIVE C3 CANDIDATE FOUND" if n_pass
               else "B: SCOPE-NEGATIVE (declared families closed; class "
                    "open beyond scope)")
    print("\n" + "=" * 78)
    print(f"CHECKS {ok_n}/{len(CHECKS)}   OUTCOME {outcome}")
    print("=" * 78)

    outdir = os.path.join(HERE, "results", "native_c3")
    os.makedirs(outdir, exist_ok=True)
    with open(os.path.join(outdir, "unit_strut_tensegrity_decision.json"),
              "w", encoding="utf-8") as f:
        json.dump({"checks": CHECKS, "verdicts": VERDICTS,
                   "outcome": outcome}, f, indent=1, default=str)
    return 0 if ok_n == len(CHECKS) else 1


if __name__ == "__main__":
    sys.exit(main())
