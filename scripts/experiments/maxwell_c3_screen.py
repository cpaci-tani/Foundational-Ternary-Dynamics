"""Maxwell-criterion screen for a native n = 4 mechanism (C3).

Protocol: PREREG_MAXWELL_C3_SCREEN_v1.md, locked at commit 38292bf1 BEFORE
this file existed. Criterion from FTD-0789.

  n = 2        iff null(H) is trivial (rigid)
  n = infinity iff a null direction extends to a finite mechanism
  n = 4        iff null(H) nontrivial AND the quartic form is POSITIVE
               DEFINITE on all of it

Registered model, verbatim (verify_flexural_refutation.py):
  V(q) = 0                                if q >= 3/2      (q = squared dist)
  V(q) = -16 eps (q - 3/2)^2 (q - 3/4)    otherwise
  A(s_i, s_j) = (1 - s_i s_j) / 2         polarity mask
  eps = 0.01;  minimum at q = 1 (r = 1), depth -eps, radial stiffness 96 eps

THE FTD-0787 GUARD (prereg 3.1, mandatory): energy along a null direction is
never evaluated on a straight line. At each amplitude the remaining
coordinates are relaxed first. FTD-0787's quartic was the curvature of a
rectilinear chord across an exactly flat valley.
"""
import itertools, json, sys
from pathlib import Path
import numpy as np
from scipy.optimize import minimize
from scipy.linalg import null_space, orth

EPS = 0.01
CUT = 1.5                      # compact support in q
NULL_TOL = 1e-7                # prereg 7.5; re-run at 1e-6 and 1e-8
RNG = np.random.default_rng(20260804)
OUTPUT_PATH = (
    Path(__file__).resolve().parent
    / "recorded_results" / "ftd_0800" / "maxwell_c3_results.json"
)


# ----------------------------------------------------------------- model ---
def V(q):
    q = np.asarray(q, float)
    out = np.zeros_like(q)
    m = q < CUT
    out[m] = -16 * EPS * (q[m] - 1.5) ** 2 * (q[m] - 0.75)
    return out


def dV(q):      # V'(q) = -48 eps (q - 3/2)(q - 1)
    q = np.asarray(q, float)
    out = np.zeros_like(q)
    m = q < CUT
    out[m] = -48 * EPS * (q[m] - 1.5) * (q[m] - 1.0)
    return out


def ddV(q):     # V''(q) = -96 eps (q - 5/4)
    q = np.asarray(q, float)
    out = np.zeros_like(q)
    m = q < CUT
    out[m] = -96 * EPS * (q[m] - 1.25)
    return out


def mask(s):
    s = np.asarray(s, float)
    return (1.0 - np.outer(s, s)) / 2.0


def energy(x, A):
    p = x.reshape(-1, 3)
    d = p[:, None, :] - p[None, :, :]
    q = (d ** 2).sum(-1)
    iu = np.triu_indices(len(p), 1)
    return float((A[iu] * V(q[iu])).sum())


def grad(x, A):
    p = x.reshape(-1, 3); n = len(p)
    d = p[:, None, :] - p[None, :, :]
    q = (d ** 2).sum(-1)
    np.fill_diagonal(q, 10.0)                       # keep self-pair inert
    w = A * dV(q) * 2.0                             # dE/dr_i = sum_j w_ij d_ij
    g = (w[:, :, None] * d).sum(1)
    return g.reshape(-1)


def hessian(x, A):
    """Exact analytic Hessian. For a pair: M = 4 V''(q) d(x)d + 2 V'(q) I."""
    p = x.reshape(-1, 3); n = len(p)
    H = np.zeros((3 * n, 3 * n))
    for i in range(n):
        for j in range(i + 1, n):
            if A[i, j] == 0.0:
                continue
            d = p[i] - p[j]; q = float(d @ d)
            if q >= CUT:
                continue
            M = A[i, j] * (4 * float(ddV(np.array([q]))[0]) * np.outer(d, d)
                           + 2 * float(dV(np.array([q]))[0]) * np.eye(3))
            si, sj = slice(3 * i, 3 * i + 3), slice(3 * j, 3 * j + 3)
            H[si, si] += M; H[sj, sj] += M
            H[si, sj] -= M; H[sj, si] -= M
    return (H + H.T) / 2


def n_bonds(x, A):
    p = x.reshape(-1, 3); n = len(p); b = 0
    for i in range(n):
        for j in range(i + 1, n):
            d = p[i] - p[j]
            if A[i, j] != 0.0 and d @ d < CUT:
                b += 1
    return b


# ------------------------------------------------------- rigid-body modes ---
def trivial_modes(x):
    """3 translations + rotations. Collinear configurations yield only 2
    independent rotations; detected via the inertia tensor, not assumed."""
    p = x.reshape(-1, 3); n = len(p)
    c = p.mean(0); r = p - c
    cols = []
    for a in np.eye(3):
        cols.append(np.tile(a, n))
    for a in np.eye(3):
        cols.append(np.cross(a, r).reshape(-1))
    T = np.array(cols).T
    return orth(T, rcond=1e-10)          # drops the degenerate rotation


def null_beyond_trivial(H, x, tol):
    n = H.shape[0]
    ev, evec = np.linalg.eigh(H)
    N = evec[:, np.abs(ev) < tol]
    if N.shape[1] == 0:
        return np.zeros((n, 0)), ev
    T = trivial_modes(x)
    P = N - T @ (T.T @ N)                # project out rigid-body content
    if P.shape[1] == 0:
        return np.zeros((n, 0)), ev
    U, S, _ = np.linalg.svd(P, full_matrices=False)
    return U[:, S > 1e-8], ev


# --------------------------------------------- the FTD-0787 relaxed probe ---
def bond_set(x, A):
    """Which pairs are inside compact support. The null-direction probe is only
    meaningful while this is CONSTANT: once a bond breaks or forms, the walk is
    measuring dissociation or rebonding, not the mode. Found by the Tier-A
    controls, which scored the trimer's dissociation at t=1 as curvature."""
    p = x.reshape(-1, 3); n = len(p); s = set()
    for i in range(n):
        for j in range(i + 1, n):
            d = p[i] - p[j]
            if A[i, j] != 0.0 and d @ d < CUT:
                s.add((i, j))
    return frozenset(s)


def relaxed_profile(x0, A, u, amps=(0.005, 0.01, 0.02, 0.05, 0.1, 0.2)):
    """Walk along null direction u, RELAXING every other coordinate at each
    amplitude. Straight-line evaluation is what produced FTD-0787's phantom
    quartic; this is the guard required by prereg 3.1. Amplitudes that change
    the bond set are marked invalid and excluded from the fit."""
    T = trivial_modes(x0)
    block = np.hstack([T, u.reshape(-1, 1)])
    C = null_space(block.T)              # complement: neither drift nor slide
    B0 = bond_set(x0, A); E0 = energy(x0, A)
    out = []
    for t in amps:
        start = x0 + t * u

        def f(z):
            return energy(start + C @ z, A)

        def fp(z):
            return C.T @ grad(start + C @ z, A)

        r = minimize(f, np.zeros(C.shape[1]), jac=fp, method="L-BFGS-B",
                     options=dict(maxiter=6000, ftol=1e-16, gtol=1e-14))
        xr = start + C @ r.x
        out.append(dict(t=t, dE=float(r.fun) - E0,
                        dE_straight=float(energy(start, A)) - E0,
                        valid=bool(bond_set(xr, A) == B0)))
    return out


def classify(x0, A, tol=NULL_TOL, label=""):
    H = hessian(x0, A)
    N0, ev = null_beyond_trivial(H, x0, tol)
    nb = n_bonds(x0, A); n = len(x0) // 3
    rec = dict(label=label, N=n, B=nb, maxwell=3 * n - nb,
               dim_null=int(N0.shape[1]),
               n_trivial=int(trivial_modes(x0).shape[1]),
               eig_min=float(ev.min()), eig_max=float(ev.max()),
               E0=energy(x0, A), grad_norm=float(np.linalg.norm(grad(x0, A))))
    if N0.shape[1] == 0:
        rec["verdict"] = "n=2 (rigid)"
        return rec
    # classify each null direction by its RELAXED profile
    exps, flats, dirs = [], 0, []
    for k in range(N0.shape[1]):
        prof = relaxed_profile(x0, A, N0[:, k])
        keep = [p for p in prof if p["valid"]]
        t = np.array([p["t"] for p in keep]); dE = np.array([p["dE"] for p in keep])
        dirs.append(dict(t=[p["t"] for p in prof],
                         dE_relaxed=[p["dE"] for p in prof],
                         dE_straight=[p["dE_straight"] for p in prof],
                         valid=[p["valid"] for p in prof]))
        if len(keep) < 2:
            exps.append(None); continue          # no bound region to fit
        if np.max(np.abs(dE)) < 1e-12:
            flats += 1; exps.append(None); continue
        m = np.abs(dE) > 1e-14
        exps.append(float(np.polyfit(np.log(t[m]), np.log(np.abs(dE[m])), 1)[0])
                    if m.sum() >= 2 else None)
    rec["directions"] = dirs
    rec["exponents"] = exps
    rec["n_exactly_flat"] = flats
    if flats == N0.shape[1]:
        rec["verdict"] = "n=infinity (all null dirs are finite mechanisms)"
    elif flats > 0:
        rec["verdict"] = "SEMIDEFINITE (some flat, some quartic) -> n=infinity wins"
    else:
        pos = [e for e in exps if e is not None]
        near4 = all(3.5 < e < 4.5 for e in pos)
        rec["verdict"] = ("n=4 CANDIDATE" if near4
                          else f"first-order flexible, exponents {np.round(pos,2)}")
    return rec


# ---------------------------------------------------------- Tier A: ctrl ---
def tier_a():
    print("=" * 72); print("TIER A - CONTROLS (prereg 7: these can INVALIDATE the screen)")
    print("=" * 72)
    out = []
    # collinear trimer (+1,-1,+1); must be n=infinity with 7 zero modes
    s = np.array([1, -1, 1]); A = mask(s)
    x = np.array([-1., 0, 0, 0., 0, 0, 1., 0, 0])
    r = classify(x, A, label="A1 collinear trimer (+1,-1,+1)")
    r["expected"] = "n=infinity; 3N-B = 9-2 = 7 zero modes"
    r["control_ok"] = ("infinity" in r["verdict"]) and r["maxwell"] == 7
    out.append(r); show(r)
    # 2x2x2 checkerboard block, opposite-polarity SC bonds
    pts, sgn = [], []
    for c in itertools.product((0, 1), repeat=3):
        pts.append(c); sgn.append((-1) ** sum(c))
    x = np.array(pts, float).reshape(-1); A = mask(np.array(sgn))
    r = classify(x, A, label="A2 2x2x2 checkerboard block (N=8)")
    r["expected"] = "reference; SC nearest-neighbour bonds only"
    out.append(r); show(r)
    return out


def show(r):
    print(f"\n  {r['label']}")
    print(f"    N={r['N']} B={r['B']}  Maxwell 3N-B={r['maxwell']}  "
          f"trivial={r['n_trivial']}  dim(N0)={r['dim_null']}")
    print(f"    E0={r['E0']:.10f}  |grad|={r['grad_norm']:.3e}  "
          f"eig in [{r['eig_min']:.4e}, {r['eig_max']:.4e}]")
    if r.get("exponents") is not None:
        print(f"    exactly flat dirs: {r['n_exactly_flat']}/{r['dim_null']}   "
              f"exponents: {r['exponents']}")
        d = r["directions"][0]
        print(f"    dir0 relaxed dE:  {[f'{v:.3e}' for v in d['dE_relaxed']]}")
        print(f"    dir0 STRAIGHT dE: {[f'{v:.3e}' for v in d['dE_straight']]}"
              "   <- the FTD-0787 path")
    print(f"    VERDICT: {r['verdict']}")
    if "control_ok" in r:
        print(f"    CONTROL {'PASS' if r['control_ok'] else 'FAIL -> SCREEN_INVALID'}")


def equilibrate(x, A, tries=1):
    """Relax to a stationary point; return None if it dissociates or unbinds."""
    best = None
    for _ in range(tries):
        r = minimize(lambda z: energy(z, A), x, jac=lambda z: grad(z, A),
                     method="L-BFGS-B", options=dict(maxiter=20000, ftol=1e-18,
                                                     gtol=1e-15))
        if np.linalg.norm(grad(r.x, A)) < 1e-9:
            if best is None or r.fun < best[1]:
                best = (r.x, r.fun)
        x = r.x
    return None if best is None else best[0]


def tier_b(nmax=6, seeds=24):
    print("\n" + "=" * 72)
    print("TIER B - EXHAUSTIVE SMALL CLUSTERS (N=3..%d, polarities {-1,0,+1})" % nmax)
    print("=" * 72)
    found, tally, checked = [], {}, 0
    for N in range(3, nmax + 1):
        for s in itertools.product((-1, 0, 1), repeat=N):
            if s[0] < 0:                       # global polarity inversion
                continue
            A = mask(np.array(s))
            if A.sum() == 0:
                continue
            for k in range(seeds):
                x0 = (RNG.normal(scale=0.7, size=3 * N)
                      + np.repeat(np.arange(N), 3) * np.tile([1., 0, 0], N))
                xe = equilibrate(x0, A)
                if xe is None:
                    continue
                if n_bonds(xe, A) < N - 1:     # require connected-ish
                    continue
                checked += 1
                r = classify(xe, A, label=f"B N={N} s={s} seed={k}")
                v = r["verdict"].split("(")[0].strip()
                tally[v] = tally.get(v, 0) + 1
                if "n=4" in r["verdict"] or "flexible" in r["verdict"]:
                    found.append(r); show(r)
    print(f"\n  Tier B: {checked} bound equilibria classified")
    for v, c in sorted(tally.items(), key=lambda kv: -kv[1]):
        print(f"    {c:>5}  {v}")
    return found, tally, checked


def tier_c():
    print("\n" + "=" * 72)
    print("TIER C - SC BINDING NETWORK (the decisive question of prereg 4)")
    print("=" * 72)
    out = []
    for L in (2, 3, 4):
        pts, sgn = [], []
        for c in itertools.product(range(L), repeat=3):
            pts.append(c); sgn.append((-1) ** sum(c))
        x = np.array(pts, float).reshape(-1); A = mask(np.array(sgn))
        r = classify(x, A, label=f"C L={L} checkerboard block (N={L**3})")
        out.append(r); show(r)
    return out


if __name__ == "__main__":
    OUTPUT_PATH.parent.mkdir(parents=True, exist_ok=True)
    res = tier_a()
    ok = all(r.get("control_ok", True) for r in res)
    print("\n" + "=" * 72)
    print(f"TIER A CONTROLS: {'PASS - screen is valid, proceed' if ok else 'FAIL - SCREEN_INVALID'}")
    if not ok:
        with OUTPUT_PATH.open("w", encoding="utf-8") as output:
            json.dump(dict(verdict="SCREEN_INVALID", tierA=res),
                      output, indent=1, default=str)
        sys.exit(1)
    C = tier_c()
    B, tally, checked = tier_b()
    with OUTPUT_PATH.open("w", encoding="utf-8") as output:
        json.dump(dict(tierA=res, tierC=C, tierB_hits=B, tierB_tally=tally,
                       tierB_checked=checked),
                  output, indent=1, default=str)
    hits = [r for r in (res + C + B) if "n=4 CANDIDATE" in r["verdict"]]
    print("\n" + "=" * 72)
    print(f"OUTCOME: {'N4_FOUND - ' + str(len(hits)) + ' candidate(s)' if hits else 'NO_NATIVE_N4'}")
