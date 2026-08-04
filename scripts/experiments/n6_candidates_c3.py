"""Test the five N=6 unit-distance frameworks that satisfy Connelly's
necessary condition (self-stress AND flex) against the FTD compact law.

Provenance: unit_distance_n4_search.py enumerated N=4,5,6 and found 701 N=6
frameworks with BOTH stress>0 and flex>0 -- the first candidates in this whole
arc to clear the necessary condition. Five distinct (B, rank, flex, stress)
signatures. FTD-0800's Tier B could not have found these: it relaxed 38
equilibria from RANDOM starts, and these are measure-zero degenerate
geometries.

Two questions per candidate:
  1. REALIZABLE?  Is there a polarity assignment s in {-1,0,+1}^6 such that
     every bonded pair has mask > 0 and every NON-bonded pair is either
     mask = 0 (both nonzero, same sign) or out of support (q >= 3/2)?
  2. n = 4?  With the FTD compact law at those stiffnesses, is the quartic
     form positive definite on the whole null space -- under the FTD-0787
     relaxation guard?
"""
import itertools
import numpy as np
from scipy.optimize import least_squares
from maxwell_c3_screen import (CUT, mask, energy, grad, hessian,
                               trivial_modes, null_beyond_trivial,
                               relaxed_profile)

RNG = np.random.default_rng(20260804)

CANDIDATES = [
    ("K_2,4        B=8  flex=5 stress=1",
     [(0,1),(0,2),(0,3),(0,4),(1,5),(2,5),(3,5),(4,5)]),
    ("K_2,4 +1edge B=9  flex=4 stress=1",
     [(0,1),(0,2),(0,3),(0,4),(1,2),(1,5),(2,5),(3,5),(4,5)]),
    ("             B=10 flex=3 stress=1",
     [(0,1),(0,2),(0,3),(0,4),(1,2),(1,3),(1,5),(2,5),(3,5),(4,5)]),
    ("             B=11 flex=2 stress=1",
     [(0,1),(0,2),(0,3),(0,4),(0,5),(1,2),(1,3),(2,4),(2,5),(3,4),(3,5)]),
    ("BALANCE PT   B=12 flex=1 stress=1",
     [(0,1),(0,2),(0,3),(0,4),(0,5),(1,2),(1,3),(1,4),(2,3),(2,4),(3,5),(4,5)]),
]
N = 6


DMIN = 0.5          # no two constituents closer than half a bond length


def embed(edges, tries=400, dmin=DMIN):
    """Unit-distance embedding in R^3, NON-DEGENERATE.

    The first version of this returned embeddings with COINCIDENT vertices
    (non-bond q = 0.0 in four of five candidates, and one with pairs at
    q = 0.005). Those are not frameworks, and the realizability check waved
    them through because same-polarity pairs carry mask = 0 at ANY distance,
    including zero. A separation penalty is now part of the residual, and any
    embedding still violating dmin is rejected outright.
    """
    npair = [(i, j) for i in range(N) for j in range(i + 1, N)]
    best = None
    for _ in range(tries):
        p0 = RNG.normal(scale=0.8, size=(N, 3))

        def f(z):
            p = z.reshape(N, 3)
            res = [np.linalg.norm(p[i] - p[j]) - 1.0 for i, j in edges]
            for i, j in npair:                       # push apart, never together
                d = np.linalg.norm(p[i] - p[j])
                res.append(10.0 * max(0.0, dmin - d))
            return np.array(res)

        r = least_squares(f, p0.reshape(-1), xtol=1e-15, ftol=1e-15, gtol=1e-15)
        p = r.x.reshape(N, 3)
        edge_err = max(abs(np.linalg.norm(p[i] - p[j]) - 1.0) for i, j in edges)
        sep = min(np.linalg.norm(p[i] - p[j]) for i, j in npair)
        if edge_err < 1e-10 and sep >= dmin - 1e-9:
            if best is None or edge_err < best[1]:
                best = (p, edge_err)
    return best


def realizable(p, edges):
    """Find every polarity assignment consistent with this bond set."""
    E = set(map(tuple, (sorted(e) for e in edges)))
    ok = []
    for s in itertools.product((-1, 0, 1), repeat=N):
        if s[0] < 0:                                  # global inversion
            continue
        A = mask(np.array(s)); good = True
        for i in range(N):
            for j in range(i + 1, N):
                q = float(np.sum((p[i] - p[j]) ** 2))
                bonded = (i, j) in E
                if bonded:
                    if A[i, j] == 0.0 or q >= CUT:    # must actually bond
                        good = False; break
                else:
                    if A[i, j] != 0.0 and q < CUT:    # must NOT bond
                        good = False; break
            if not good:
                break
        if good:
            ok.append(s)
    return ok


for label, edges in CANDIDATES:
    print("=" * 74); print(label)
    got = embed(edges)
    if got is None:
        print("  no unit-distance embedding found\n"); continue
    p, resid = got
    print(f"  embedding residual {resid:.1e}")
    d = [(i, j, float(np.sum((p[i]-p[j])**2)))
         for i in range(N) for j in range(N) if i < j]
    nb = [(i, j, q) for i, j, q in d if [i, j] not in [sorted(e) for e in edges]]
    print(f"  non-bond squared distances: "
          f"{sorted(round(q,3) for _,_,q in nb)}   (support cut q < {CUT})")
    sols = realizable(p, edges)
    if not sols:
        print("  NOT REALIZABLE under the polarity mask + compact support\n")
        continue
    print(f"  REALIZABLE: {len(sols)} polarity assignment(s), e.g. {sols[0]}")
    for s in sols[:2]:
        A = mask(np.array(s)); x0 = p.reshape(-1)
        g = np.linalg.norm(grad(x0, A))
        H = hessian(x0, A); N0, ev = null_beyond_trivial(H, x0, 1e-7)
        print(f"    s={s}  |grad|={g:.2e}  E0={energy(x0,A):.6f}  "
              f"dim(N0)={N0.shape[1]}")
        if g > 1e-7:
            print("      NOT an equilibrium under this law (bonds not all at "
                  "their minimum) -> pre-stressed, outside the screen scope")
            continue
        flat = 0; exps = []
        for k in range(N0.shape[1]):
            prof = relaxed_profile(x0, A, N0[:, k])
            keep = [q for q in prof if q["valid"]]
            t = np.array([q["t"] for q in keep]); dE = np.array([q["dE"] for q in keep])
            if len(keep) < 2 or np.max(np.abs(dE)) < 1e-12:
                flat += 1; continue
            m = np.abs(dE) > 1e-14
            if m.sum() >= 2:
                exps.append(round(float(np.polyfit(np.log(t[m]),
                            np.log(np.abs(dE[m])), 1)[0]), 3))
        print(f"      relaxed probe: {flat}/{N0.shape[1]} exactly flat, "
              f"exponents {exps}")
        if flat == 0 and exps and all(3.5 < e < 4.5 for e in exps):
            print("      *** n = 4 CANDIDATE -- quartic positive definite ***")
        elif flat == N0.shape[1]:
            print("      n = infinity (all null dirs are finite mechanisms)")
        else:
            print("      positive SEMI-definite at best -> fails the criterion")
    print()
