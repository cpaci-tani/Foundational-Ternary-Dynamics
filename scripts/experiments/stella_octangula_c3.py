"""Stella octangula as a C3 candidate, and what it isolates.

The 8 vertices of the stella octangula are the cube corners; its two
interpenetrating tetrahedra are exactly the two parity classes, which is
exactly FTD's +/- polarity split. So the compound IS the 2x2x2 checkerboard
block already screened in FTD-0800 -- but naming it this way exposes WHY it
fails, because the compound's rigidity is carried entirely by the TETRAHEDRAL
edges, and those are the ones FTD cannot bond.

With cube edge 1:
  cube edges   (A<->B)  12 at 1.000     opposite polarity -> BONDED
  tetra edges  (A<->A)  12 at 1.414     SAME polarity (mask 0) AND out of
                                        support (1.2247) -> doubly excluded
  body diags   (A<->B)   4 at 1.732     out of support

Three arms:
  1  cube edges only          = what FTD actually has
  2  cube + ALL tetra edges   = the full compound, braced
  3  cube + k tetra edges     = partial bracing, k = 1..11

The braces are NOT native -- the compact law has a single minimum at r = 1 and
cannot place a bond at 1.414. Arm 2/3 are counterfactual: they measure what
FTD would need to ADD, not what it has.
"""
import itertools
import numpy as np
from scipy.linalg import orth

K1, K2 = 96 * 0.01, 96 * 0.01          # stiffnesses: cube bonds, brace bonds

CUBE = np.array(list(itertools.product((0., 1.), repeat=3)))
PAR = np.array([int(sum(v)) % 2 for v in CUBE])
cube_edges = [(i, j) for i in range(8) for j in range(i + 1, 8)
              if abs(np.linalg.norm(CUBE[i] - CUBE[j]) - 1.0) < 1e-9]
tetra_edges = [(i, j) for i in range(8) for j in range(i + 1, 8)
               if abs(np.linalg.norm(CUBE[i] - CUBE[j]) - np.sqrt(2)) < 1e-9]
print(f"cube edges  (opposite polarity, r=1)      : {len(cube_edges)}")
print(f"tetra edges (same polarity, r=sqrt2)      : {len(tetra_edges)}")
print(f"body diagonals                            : "
      f"{28 - len(cube_edges) - len(tetra_edges)}\n")


def analyse(edges, tag):
    """edges = list of (i, j, natural_length, k). Zero-stress by construction."""
    p = CUBE.copy(); N = 8
    R = np.zeros((len(edges), 3 * N))
    for e, (i, j, L, k) in enumerate(edges):
        d = p[i] - p[j]
        R[e, 3 * i:3 * i + 3] = d; R[e, 3 * j:3 * j + 3] = -d
    rank = np.linalg.matrix_rank(R, tol=1e-8)
    stress = len(edges) - rank
    flex = 3 * N - 6 - rank
    # energy Hessian at zero stress: sum k * (e_hat)(x)(e_hat) per bond
    H = np.zeros((3 * N, 3 * N))
    for i, j, L, k in edges:
        d = p[i] - p[j]; u = d / np.linalg.norm(d)
        M = k * np.outer(u, u)
        H[3*i:3*i+3, 3*i:3*i+3] += M; H[3*j:3*j+3, 3*j:3*j+3] += M
        H[3*i:3*i+3, 3*j:3*j+3] -= M; H[3*j:3*j+3, 3*i:3*i+3] -= M
    ev = np.linalg.eigvalsh((H + H.T) / 2)
    nz = int((np.abs(ev) < 1e-9).sum())
    verdict = ("n=2 (rigid)" if flex == 0 else
               f"first-order flexible ({flex} flexes, {stress} stresses)")
    print(f"  {tag:<34} B={len(edges):>3} rank={rank:>3} "
          f"stress={stress:>2} flex={flex:>2}  zero-eigs={nz:>2}  {verdict}")
    return dict(B=len(edges), rank=int(rank), stress=int(stress), flex=int(flex))


print("=== ARM 1 - what FTD actually has ===")
e1 = [(i, j, 1.0, K1) for i, j in cube_edges]
a1 = analyse(e1, "cube edges only (the FTD case)")

print("\n=== ARM 2 - the full braced compound (counterfactual) ===")
e2 = e1 + [(i, j, np.sqrt(2), K2) for i, j in tetra_edges]
a2 = analyse(e2, "cube + ALL 12 tetra edges")

print("\n=== ARM 3 - partial bracing: is there a k with flex>0 AND stress>0? ===")
best = []
for k in range(1, 12):
    seen = set()
    for sub in itertools.combinations(range(12), k):
        e3 = e1 + [(tetra_edges[t][0], tetra_edges[t][1], np.sqrt(2), K2)
                   for t in sub]
        p = CUBE
        R = np.zeros((len(e3), 24))
        for e, (i, j, L, kk) in enumerate(e3):
            d = p[i] - p[j]
            R[e, 3*i:3*i+3] = d; R[e, 3*j:3*j+3] = -d
        rank = np.linalg.matrix_rank(R, tol=1e-8)
        st, fl = len(e3) - rank, 18 - rank
        key = (rank, st, fl)
        if key not in seen:
            seen.add(key)
            if st > 0 and fl > 0:
                best.append((k, sub, rank, st, fl))
    sig = sorted(seen)
    print(f"  k={k:>2}: signatures (rank, stress, flex) = {sig}")

print(f"\n  configurations with BOTH stress>0 and flex>0: {len(best)}")
for k, sub, rank, st, fl in best[:8]:
    print(f"    k={k} braces={sub} rank={rank} stress={st} flex={fl}")
