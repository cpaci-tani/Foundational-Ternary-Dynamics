"""native_unitedge_stress_screen.py — can the REGISTERED single-scale law
carry an n = 4 mechanism at all?

The zero-tension equilibria of the registered compact pair law are exactly
the frameworks whose active bonds all sit at the single minimum r0 = 1 with
opposite-polarity endpoints — i.e. UNIT-EDGE BIPARTITE frameworks. By the
FTD-0789 criterion (C3), n = 4 requires a first-order flex blocked at second
order, and by the Connelly stress test a flex is blocked ONLY by a nonzero
self-stress. So the native question reduces to:

    Does any unit-edge bipartite framework (same-polarity separation floor,
    opposite-polarity non-edges outside the cutoff) admit a nonzero
    self-stress?

This screen searches embeddings of every connected min-degree-2 bipartite
graph up to N = 7 (plus the cube graph at N = 8, and exact SC lattice blocks)
for row-dependence of the rigidity matrix under the physical constraints.

  - A HIT (sigma_min ~ 0 with constraints met) = a native n = 4 candidate:
    it would then be tested for flex existence and blocking.
  - A clean MISS across the enumeration = registered-scope evidence for the
    single-scale no-go, complementing FTD-0800's graph-class screen (which
    sampled equilibria of the law; this searches the abstract unit-edge
    class directly, so misses here bound ALL zero-tension equilibria with
    these interaction graphs, not just sampled ones).

Epistemic status: [EXPLORATORY NUMERICAL SCREEN — SCOPED]. Enumerated graph
classes and floors are declared below; no theorem is claimed. Deterministic
under the fixed seed.
"""
from __future__ import annotations

import itertools
import numpy as np
from scipy.optimize import minimize

SEED = 20260807
FLOOR_SAME = 0.40       # same-polarity separation floor (declared)
CUTOFF = 1.30           # opposite-polarity non-edges must sit beyond this
W_PEN = 200.0           # constraint penalty weight
RESTARTS = 40
HIT_TOL = 1e-9


def enumerate_graphs():
    """Connected, min-degree-2 bipartite graphs, parts (a, b), a+b <= 7,
    crude iso-dedup. Returns list of (a, b, edges)."""
    out, seen = [], set()
    for a_n in range(2, 4):
        for b_n in range(a_n, 8 - a_n):
            all_edges = [(i, a_n + j) for i in range(a_n) for j in range(b_n)]
            for r in range(max(a_n, b_n) * 2, len(all_edges) + 1):
                for sub in itertools.combinations(all_edges, r):
                    deg = {}
                    for i, j in sub:
                        deg[i] = deg.get(i, 0) + 1
                        deg[j] = deg.get(j, 0) + 1
                    if len(deg) != a_n + b_n or min(deg.values()) < 2:
                        continue
                    # connectivity
                    adj = {}
                    for i, j in sub:
                        adj.setdefault(i, set()).add(j)
                        adj.setdefault(j, set()).add(i)
                    comp, stack = {0}, [0]
                    while stack:
                        v = stack.pop()
                        for w in adj.get(v, ()):
                            if w not in comp:
                                comp.add(w)
                                stack.append(w)
                    if len(comp) != a_n + b_n:
                        continue
                    # crude canonical signature
                    siga = tuple(sorted(deg[i] for i in range(a_n)))
                    sigb = tuple(sorted(deg[a_n + j] for j in range(b_n)))
                    nbr = tuple(sorted(tuple(sorted(deg[w] for w in adj[v]))
                                       for v in range(a_n + b_n)))
                    key = (a_n, b_n, r, siga, sigb, nbr)
                    if key in seen:
                        continue
                    seen.add(key)
                    out.append((a_n, b_n, list(sub)))
    # bonus: the cube graph Q3 (4+4, all degree 3)
    cube_edges = [(0, 4), (0, 5), (1, 4), (1, 6), (2, 5), (2, 7), (3, 6), (3, 7)]
    # Q3 proper: vertices 0..7 = binary strings; edges between weight-parity
    # classes differing in one bit:
    evens = [0b000, 0b011, 0b101, 0b110]
    odds = [0b001, 0b010, 0b100, 0b111]
    idx = {v: i for i, v in enumerate(evens)}
    idx.update({v: 4 + i for i, v in enumerate(odds)})
    q3 = []
    for v in evens:
        for bit in (1, 2, 4):
            q3.append((idx[v], idx[v ^ bit]))
    out.append((4, 4, sorted(set(tuple(sorted(e)) for e in q3))))
    return out


def rigidity(p, edges):
    n = p.shape[0]
    R = np.zeros((len(edges), 3 * n))
    for e, (i, j) in enumerate(edges):
        d = p[j] - p[i]
        ell = np.linalg.norm(d)
        u = d / max(ell, 1e-12)
        R[e, 3 * i:3 * i + 3] = -u
        R[e, 3 * j:3 * j + 3] = +u
    return R


def objective(x, a_n, n, edges, same_pairs, opp_nonedges):
    p = x.reshape(n, 3)
    R = rigidity(p, edges)
    lam = np.linalg.eigvalsh(R @ R.T)[0]          # smallest: 0 iff stress
    pen = 0.0
    for i, j in edges:
        pen += (np.sum((p[i] - p[j]) ** 2) - 1.0) ** 2
    for i, j in same_pairs:
        d2 = np.sum((p[i] - p[j]) ** 2)
        pen += max(0.0, FLOOR_SAME ** 2 - d2) ** 2
    for i, j in opp_nonedges:
        d2 = np.sum((p[i] - p[j]) ** 2)
        pen += max(0.0, CUTOFF ** 2 - d2) ** 2
    return lam + W_PEN * pen


def screen_graph(a_n, b_n, edges, rng):
    n = a_n + b_n
    eset = set(tuple(sorted(e)) for e in edges)
    same_pairs = ([(i, j) for i in range(a_n) for j in range(i + 1, a_n)] +
                  [(i, j) for i in range(a_n, n) for j in range(i + 1, n)])
    opp_nonedges = [(i, j) for i in range(a_n) for j in range(a_n, n)
                    if tuple(sorted((i, j))) not in eset]
    best = np.inf
    best_x = None
    for _ in range(RESTARTS):
        x0 = rng.normal(scale=1.2, size=3 * n)
        res = minimize(objective, x0, args=(a_n, n, edges, same_pairs,
                                            opp_nonedges),
                       method="BFGS",
                       options={"maxiter": 400, "gtol": 1e-12})
        if res.fun < best:
            best, best_x = res.fun, res.x
    # split best into stress eigenvalue vs constraint violation
    p = best_x.reshape(n, 3)
    R = rigidity(p, edges)
    lam = np.linalg.eigvalsh(R @ R.T)[0]
    viol = (best - lam) / W_PEN
    return lam, viol, best_x


def sc_block_check(L):
    """Exact rank check for the axis-aligned SC checkerboard block LxLxL:
    unit axial bonds between opposite parities (ALL of them: the physical
    law bonds every opposite pair at distance 1). Stress space dim = e - rank."""
    pts = [(x, y, z) for x in range(L) for y in range(L) for z in range(L)]
    idx = {q: i for i, q in enumerate(pts)}
    edges = []
    for (x, y, z) in pts:
        for dx, dy, dz in ((1, 0, 0), (0, 1, 0), (0, 0, 1)):
            q = (x + dx, y + dy, z + dz)
            if q in idx:
                edges.append((idx[(x, y, z)], idx[q]))
    p = np.array(pts, dtype=float)
    R = rigidity(p, edges)
    rank = np.linalg.matrix_rank(R, tol=1e-9)
    n = len(pts)
    return len(edges), rank, len(edges) - rank, 3 * n - rank


def main():
    rng = np.random.default_rng(SEED)
    graphs = enumerate_graphs()
    print("=" * 74)
    print(f"UNIT-EDGE BIPARTITE SELF-STRESS SCREEN "
          f"(seed {SEED}, floors: same >= {FLOOR_SAME}, "
          f"opp non-edge >= {CUTOFF})")
    print(f"graph classes enumerated (min-deg 2, connected, N <= 7 + Q3): "
          f"{len(graphs)}")
    print("=" * 74)
    hits = []
    lam_global = np.inf
    for gi, (a_n, b_n, edges) in enumerate(graphs):
        lam, viol, x = screen_graph(a_n, b_n, edges, rng)
        lam_global = min(lam_global, lam) if viol < 1e-8 else lam_global
        status = ""
        if lam < HIT_TOL and viol < 1e-8:
            status = "  <== HIT: verify stress + blocking"
            hits.append((a_n, b_n, edges, x))
        print(f"  ({a_n},{b_n}) e={len(edges):>2}  "
              f"min lam(RR^T) = {lam:.3e}  constraint viol = {viol:.2e}"
              f"{status}")
    print("-" * 74)
    print(f"HITS: {len(hits)}")
    if not hits:
        print(f"  no unit-edge bipartite self-stress found in the enumerated"
              f" classes;")
        print(f"  smallest constraint-clean stress eigenvalue across screen: "
              f"{lam_global:.3e}")
        print("  => within this scope the zero-tension single-scale law has no")
        print("     stressed framework, hence no second-order-blocked flex,")
        print("     hence no n = 4 — the two-scale/pre-tension doors remain")
        print("     the only ones open. [EXPLORATORY NUMERICAL SCREEN]")
    for a_n, b_n, edges, x in hits:
        p = x.reshape(-1, 3)
        R = rigidity(p, edges)
        U, S, Vt = np.linalg.svd(R)
        print(f"  HIT detail ({a_n},{b_n}): singular values {S}")
        print(f"    embedding:\n{p}")
    print("-" * 74)
    print("Exact SC checkerboard blocks (axis-aligned unit bonds, ALL")
    print("opposite-parity unit pairs bonded — the lattice-geometry class):")
    for L in (2, 3, 4):
        e, rank, coker, flexdim = sc_block_check(L)
        print(f"  L={L}: edges {e:>3}, rank {rank:>3}, "
              f"self-stress dim = {coker}, null dim = {flexdim} "
              f"({'NO stress => flexes extend => n = 2 or inf only' if coker == 0 else 'STRESS PRESENT'})")


if __name__ == "__main__":
    main()
