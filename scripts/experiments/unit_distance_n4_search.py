"""The minimum geometry question: can a UNIT-DISTANCE framework in R^3 be
first-order flexible AND second-order rigid (n = 4)?

Why unit distance. Under the registered compact law every bond has the same
minimum at r = 1, so an unstressed equilibrium forces EVERY bond to length
exactly 1. The n=4 positive control (verify_n4_positive_control.py) used a
bond of length 2 -- a length this law cannot produce.

Why this is the whole question. Support is sqrt(3/2) = 1.2247, strictly
between 1 and sqrt(2) = 1.4142. Every second-neighbour distance in a
unit-distance framework (sqrt2, sqrt3, 2) is therefore OUT of support, so the
law bonds nearest neighbours only and can never bond a second neighbour. The
only way to close a flex is with a closing bond that is itself unit length.

Connelly: second-order rigidity requires a SELF-STRESS (bonds dependent,
B > rank R) together with a flex (rank R < 3N-6), and the stress-energy form
positive on every flex.

Method: for each N and each candidate bond graph, look for a unit-distance
embedding in R^3 (residual ~ 0), then read off rank R, self-stress dimension
and flex dimension. Report every framework that is simultaneously dependent
and flexible -- those are the only n=4 candidates that can exist.
"""
import itertools
import numpy as np
from scipy.optimize import least_squares

RNG = np.random.default_rng(20260804)
TOL = 1e-10


def rigidity_matrix(p, edges):
    N = len(p); R = np.zeros((len(edges), 3 * N))
    for e, (i, j) in enumerate(edges):
        d = p[i] - p[j]
        R[e, 3 * i:3 * i + 3] = d
        R[e, 3 * j:3 * j + 3] = -d
    return R


def trivial_dim(p):
    """3 translations + rotations; collinear configs have only 2 rotations."""
    c = p - p.mean(0)
    return 5 if np.linalg.matrix_rank(c, tol=1e-8) <= 1 else 6


def embed(N, edges, tries=60):
    """Find a unit-distance embedding: |p_i - p_j| = 1 for every edge."""
    best = None
    for _ in range(tries):
        p0 = RNG.normal(scale=0.6, size=(N, 3))

        def res(z):
            p = z.reshape(N, 3)
            return np.array([np.linalg.norm(p[i] - p[j]) - 1.0 for i, j in edges])

        r = least_squares(res, p0.reshape(-1), xtol=1e-15, ftol=1e-15, gtol=1e-15)
        m = float(np.max(np.abs(r.fun)))
        if m < 1e-9 and (best is None or m < best[1]):
            best = (r.x.reshape(N, 3), m)
    return best


def analyse(N, edges):
    got = embed(N, edges)
    if got is None:
        return None
    p, resid = got
    R = rigidity_matrix(p, edges)
    rank = np.linalg.matrix_rank(R, tol=1e-8)
    triv = trivial_dim(p)
    flex = 3 * N - triv - rank        # nontrivial infinitesimal flexes
    stress = len(edges) - rank        # self-stress dimension
    return dict(N=N, B=len(edges), rank=int(rank), trivial=triv,
                flex=int(flex), stress=int(stress), resid=resid, p=p)


def graphs(N, bmin, bmax):
    pairs = list(itertools.combinations(range(N), 2))
    for k in range(bmin, bmax + 1):
        for e in itertools.combinations(pairs, k):
            deg = np.zeros(N, int)
            for i, j in e:
                deg[i] += 1; deg[j] += 1
            if deg.min() < 2:                 # a dangling vertex is a free hinge
                continue
            yield list(e)


print("Searching unit-distance frameworks in R^3 for  flex > 0 AND stress > 0")
print("(both are required for n = 4; either alone gives n = infinity or n = 2)\n")
print(f"{'N':>3} {'graphs':>8} {'embeddable':>11} {'dependent':>10} "
      f"{'flexible':>9} {'BOTH':>6}")

hits = []
for N in (4, 5, 6):
    pairs = list(itertools.combinations(range(N), 2))
    tot = emb = dep = flx = both = 0
    seen = set()
    for edges in graphs(N, N, min(len(pairs), 3 * N - 6 + 3)):
        tot += 1
        a = analyse(N, edges)
        if a is None:
            continue
        emb += 1
        if a["stress"] > 0:
            dep += 1
        if a["flex"] > 0:
            flx += 1
        if a["stress"] > 0 and a["flex"] > 0:
            both += 1
            key = (N, a["B"], a["rank"], a["flex"], a["stress"])
            if key not in seen:
                seen.add(key); a["edges"] = edges; hits.append(a)
    print(f"{N:>3} {tot:>8} {emb:>11} {dep:>10} {flx:>9} {both:>6}")

print(f"\n{len(hits)} distinct (N, B, rank, flex, stress) signatures with BOTH:")
for h in hits:
    print(f"  N={h['N']} B={h['B']} rank={h['rank']} trivial={h['trivial']} "
          f"flex={h['flex']} stress={h['stress']} resid={h['resid']:.1e}")
    print(f"    edges={h['edges']}")
np.save("unit_distance_hits.npy", np.array(
    [dict(N=h["N"], B=h["B"], rank=h["rank"], flex=h["flex"],
          stress=h["stress"], edges=h["edges"], p=h["p"]) for h in hits],
    dtype=object), allow_pickle=True)
