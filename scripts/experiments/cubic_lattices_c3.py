"""SC vs BCC vs FCC as C3 candidates.

Connelly: n = 4 needs a SELF-STRESS (B > rank R) together with a flex. The
three cubic lattices differ exactly there. Scale each so its NEAREST
neighbour sits at the compact law's minimum q = 1, then in the bulk:

  SC   6-coordinated   B = 3N   3N - B =   0   ISOSTATIC -> no self-stress
  BCC  8-coordinated   B = 4N   3N - B =  -N   over-constrained -> guaranteed
  FCC 12-coordinated   B = 6N   3N - B = -3N   over-constrained -> guaranteed

Polarity. SC and BCC nearest-neighbour graphs are BIPARTITE so +/- works.
FCC's is not (triangles), so it needs all-neutral s = 0, where the mask gives
1/2 for every pair. That is a real structural prediction, not a convenience.

Second-neighbour check per lattice, at the scale that puts NN at q = 1:
  SC  a=1      2nd at q=2    > 3/2 out of support
  BCC a=2/sqrt3  2nd at q=4/3 < 3/2 IN support, but SAME polarity -> mask 0
  FCC a=sqrt2  2nd at q=2    > 3/2 out of support
"""
import itertools
import numpy as np
from maxwell_c3_screen import (EPS, mask, energy, grad, hessian, n_bonds,
                               trivial_modes, null_beyond_trivial,
                               relaxed_profile, CUT)


def build(kind, n):
    """n = number of conventional cells per axis. Returns points, polarity."""
    pts, sgn = [], []
    if kind == "SC":
        a = 1.0
        for c in itertools.product(range(n + 1), repeat=3):
            pts.append(np.array(c, float) * a); sgn.append((-1) ** sum(c))
    elif kind == "BCC":
        a = 2 / np.sqrt(3)                      # NN = a*sqrt3/2 = 1
        for c in itertools.product(range(n + 1), repeat=3):
            pts.append(np.array(c, float) * a); sgn.append(+1)
        for c in itertools.product(range(n), repeat=3):
            pts.append((np.array(c, float) + 0.5) * a); sgn.append(-1)
    elif kind == "FCC":
        a = np.sqrt(2)                          # NN = a/sqrt2 = 1
        base = [(0, 0, 0), (.5, .5, 0), (.5, 0, .5), (0, .5, .5)]
        for c in itertools.product(range(n), repeat=3):
            for b in base:
                pts.append((np.array(c, float) + np.array(b)) * a)
                sgn.append(0)                   # all neutral: mask 1/2 for all
    return np.array(pts), np.array(sgn)


print(f"{'lattice':>8} {'N':>5} {'B':>6} {'coord':>6} {'3N-B':>7} {'rank':>6} "
      f"{'stress':>7} {'flex':>6} {'verdict':>28}")

for kind, n in (("SC", 3), ("BCC", 2), ("FCC", 2)):
    p, s = build(kind, n)
    A = mask(s); x0 = p.reshape(-1); N = len(p)
    g = np.linalg.norm(grad(x0, A))
    B = n_bonds(x0, A)
    # rigidity matrix over the actual bond set
    edges = [(i, j) for i in range(N) for j in range(i + 1, N)
             if A[i, j] != 0 and np.sum((p[i] - p[j]) ** 2) < CUT]
    R = np.zeros((len(edges), 3 * N))
    for e, (i, j) in enumerate(edges):
        d = p[i] - p[j]
        R[e, 3*i:3*i+3] = d; R[e, 3*j:3*j+3] = -d
    rank = int(np.linalg.matrix_rank(R, tol=1e-8))
    triv = trivial_modes(x0).shape[1]
    stress, flex = B - rank, 3 * N - triv - rank
    H = hessian(x0, A)
    N0, ev = null_beyond_trivial(H, x0, 1e-7)
    # bulk coordination = max degree
    deg = np.zeros(N, int)
    for i, j in edges:
        deg[i] += 1; deg[j] += 1
    v = ("n=2 (rigid)" if N0.shape[1] == 0 else f"{N0.shape[1]} null dirs")
    print(f"{kind:>8} {N:>5} {B:>6} {deg.max():>6} {3*N-B:>7} {rank:>6} "
          f"{stress:>7} {flex:>6} {v:>28}")
    print(f"         |grad|={g:.2e}  E0={energy(x0,A):.6f}  "
          f"eig_min={ev.min():.3e}")
    if N0.shape[1]:
        nflat = 0; exps = []
        for k in range(min(N0.shape[1], 8)):
            prof = relaxed_profile(x0, A, N0[:, k])
            keep = [q for q in prof if q["valid"]]
            t = np.array([q["t"] for q in keep]); dE = np.array([q["dE"] for q in keep])
            if len(keep) < 2 or np.max(np.abs(dE)) < 1e-12:
                nflat += 1
            else:
                m = np.abs(dE) > 1e-14
                if m.sum() >= 2:
                    exps.append(round(float(np.polyfit(np.log(t[m]),
                                np.log(np.abs(dE[m])), 1)[0]), 2))
        print(f"         probed {min(N0.shape[1],8)} dirs: {nflat} exactly flat, "
              f"exponents {exps}")
    print()
