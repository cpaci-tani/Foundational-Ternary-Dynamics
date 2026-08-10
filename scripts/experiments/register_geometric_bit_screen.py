"""register_geometric_bit_screen.py — T2 first screen: the geometric bit.

Candidate (class 2 of SPEC_REGISTER_CRITERIA_v1 pool): one + body C held by
three - anchors on an equilateral triangle (side s, plane z = 0) under the
registered compact law V(q) = -16 eps (q - 3/2)^2 (q - 3/4), q = r^2,
support q < 3/2. For s < sqrt(3) the bonds sit at exactly r = 1 in TWO
mirror configurations C_+- = (0, 0, +-h), h = sqrt(1 - s^2/3): a
barrier-separated two-state system at zero tension — the R2 candidate.

Statics-level finite-grid screen:
  - both states verified as zero-tension minima; Hessian = 96 eps * sum
    u_i u_i^T (positive definite iff the three bond directions span R^3);
  - a finite-grid upper estimate of the minimax barrier by watershed
    flood-fill over a bounded 3D box (union-find over energy-sorted grid
    cells). It is not a continuum certificate; the exact Delta E=eps result
    is proved by derive_register_barrier_lower_bound.py;
  - the hinge-path profile (break one bond, swing on the two-bond circle)
    reported for interpretation;
  - scan over anchor side s.

Anchors are PINNED SCAFFOLD in this screen ([SELECTED]): the self-holding
composite (anchors bonded into a frame + a companion MVC clock) is the
registered next construction, not this artifact. Statics only: R3
retention requires a declared noise ensemble (Arrhenius in Delta E / T,
[IMPOSED]) and is deferred to the composite preregistration.
[STATICS-LEVEL SCREEN — registers nothing]
"""
from __future__ import annotations

import numpy as np

EPS = 1.0
GRID_N = 121          # per axis
BOX = 1.6


def V(q):
    return np.where(q < 1.5, -16 * EPS * (q - 1.5) ** 2 * (q - 0.75), 0.0)


def screen(s):
    Ra = s / np.sqrt(3.0)
    if s >= np.sqrt(3.0):
        return None
    h = np.sqrt(1.0 - s * s / 3.0)
    anchors = np.array([[Ra * np.cos(a), Ra * np.sin(a), 0.0]
                        for a in (0, 2 * np.pi / 3, 4 * np.pi / 3)])

    def E(pts):
        e = np.zeros(pts.shape[:-1])
        for a in anchors:
            d = pts - a
            e += V(np.sum(d * d, axis=-1))
        return e

    # states + Hessian (zero tension: H = 96 eps sum u u^T)
    Cp = np.array([0.0, 0.0, h])
    H = np.zeros((3, 3))
    for a in anchors:
        u = (Cp - a)
        u /= np.linalg.norm(u)
        H += 96 * EPS * np.outer(u, u)
    hess_eigs = np.linalg.eigvalsh(H)
    E0 = float(E(Cp[None, :])[0])

    # finite-grid watershed estimate over the declared bounded 3D box
    ax = np.linspace(-BOX, BOX, GRID_N)
    X, Y, Z = np.meshgrid(ax, ax, ax, indexing="ij")
    pts = np.stack([X, Y, Z], axis=-1)
    Egrid = E(pts).ravel()
    order = np.argsort(Egrid)
    parent = np.full(Egrid.size, -1, dtype=np.int64)

    def find(i):
        root = i
        while parent[root] != root:
            root = parent[root]
        while parent[i] != root:
            parent[i], i = root, parent[i]
        return root

    def idx_of(p):
        ii = np.argmin(np.abs(ax - p[0]))
        jj = np.argmin(np.abs(ax - p[1]))
        kk = np.argmin(np.abs(ax - p[2]))
        return (ii * GRID_N + jj) * GRID_N + kk

    top_id, bot_id = idx_of(Cp), idx_of(-Cp)
    n2, n1 = GRID_N * GRID_N, GRID_N
    barrier = None
    for cell in order:
        parent[cell] = cell
        i, r0 = divmod(int(cell), n2)
        j, k = divmod(r0, n1)
        for di, dj, dk in ((1, 0, 0), (-1, 0, 0), (0, 1, 0), (0, -1, 0),
                           (0, 0, 1), (0, 0, -1)):
            ni, nj, nk = i + di, j + dj, k + dk
            if not (0 <= ni < GRID_N and 0 <= nj < GRID_N and 0 <= nk < GRID_N):
                continue
            nb = (ni * n2 // GRID_N + nj) * n1 + nk  # careful composite
            nb = (ni * GRID_N + nj) * GRID_N + nk
            if parent[nb] == -1:
                continue
            ra, rb = find(cell), find(nb)
            if ra != rb:
                parent[ra] = rb
        if parent[top_id] != -1 and parent[bot_id] != -1:
            if find(top_id) == find(bot_id):
                barrier = Egrid[cell] - E0
                break

    # hinge-path profile: bonds to anchors 0,1 exact at r=1; angle scan
    a0, a1 = anchors[0], anchors[1]
    mid = (a0 + a1) / 2
    axis = (a1 - a0)
    axis /= np.linalg.norm(axis)
    rho = np.sqrt(max(1 - (s / 2) ** 2, 0.0))
    e1 = np.array([0.0, 0.0, 1.0])
    e1 = e1 - (e1 @ axis) * axis
    e1 /= np.linalg.norm(e1)
    e2 = np.cross(axis, e1)
    th = np.linspace(0, np.pi, 721)
    ring = mid[None, :] + rho * (np.cos(th)[:, None] * e1[None, :]
                                 + np.sin(th)[:, None] * e2[None, :])
    # orient so th=0 is the upper state side
    if ring[0, 2] < 0:
        ring = ring[::-1]
    Ehinge = E(ring)
    hinge_barrier = float(Ehinge.max() - E0)
    return dict(s=s, h=h, sep=2 * h, E0=E0, eigs=hess_eigs,
                barrier=barrier, hinge=hinge_barrier)


def main():
    print("=" * 74)
    print("T2 FIRST SCREEN — the geometric bit (pinned anchors, exact statics)")
    print(f"  law: V(q) = -16 eps (q-3/2)^2 (q-3/4), eps = {EPS}; "
          f"grid {GRID_N}^3 watershed")
    print("=" * 74)
    print(f"{'s':>5} {'sep=2h':>7} {'E0':>8} {'Hessian eigs (x96eps)':>26} "
          f"{'watershed dE':>13} {'hinge dE':>9}")
    for s in (0.9, 1.0, 1.1, 1.2, 1.3):
        r = screen(s)
        eigs = "/".join(f"{e/96:.3f}" for e in r["eigs"])
        print(f"{r['s']:>5.2f} {r['sep']:>7.3f} {r['E0']:>8.3f} "
              f"{eigs:>26} {r['barrier']:>13.4f} {r['hinge']:>9.4f}")
    print()
    print("reading: E0 = -3 eps at every s (zero tension); positive-definite")
    print("Hessian (R2 stability); watershed = finite-grid upper estimate")
    print("to the minimax barrier, not a continuum certificate; hinge =")
    print("the break-one-bond path for")
    print("interpretation. Retention (R3) = Arrhenius in dE/T under a")
    print("declared [IMPOSED] ensemble — deferred to the composite prereg.")


if __name__ == "__main__":
    main()
