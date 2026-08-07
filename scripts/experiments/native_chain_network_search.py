"""native_chain_network_search.py — exhaustive integer-distance planar K4
search with an interior joint (complete up to the diameter bound).

Why this exists: the unit-edge screen's negative was scope-bounded (N <= 8).
Contracting stress supports shows every degree-2 stressed vertex forces its
two bonds collinear, so stress supports are STRAIGHT UNIT CHAINS terminated
at degree->=3 joints; parallel/overlapping chains between the same joints
force same-parity station coincidences (capacity floor), so the minimal
contracted joint graph is K4 — which carries a self-stress exactly when the
four joints are COPLANAR. A planar K4 of straight unit chains with one
joint strictly inside the triangle of the other three (crossing-free) is a
candidate NATIVE zero-tension stressed framework of the registered
single-scale law: n = 4 with no new species.

Formulation searched (complete up to congruence for max side <= SIDE_MAX):
  A = (0,0), B = (c,0), C above the axis with |AC| = b, |BC| = a integers;
  D at |DA| = p, |DB| = q integers on either Dy branch; the sixth distance
  r = |DC| tested for exact integrality in cleared-denominator integer
  arithmetic:
      X  = p^2 - q^2 + c^2            (= 2c Dx)
      U  = b^2 + c^2 - a^2            (= 2c Cx)
      SD = 4 c^2 p^2 - X^2            (= (2c Dy)^2 > 0)
      SC = 4 c^2 b^2 - U^2            (= (2c Cy)^2 > 0)
      4 c^2 r^2 = (X-U)^2 + SD + SC -+ 2 sqrt(SD*SC)
  which is integral iff SD*SC is a perfect square and the total is
  4 c^2 * (perfect square). Float prefilter + exact integer confirmation.

Filters: D strictly inside ABC; polarity 2-colorability (every chain cycle
even — all-even scaling always available, parity-native preferred);
fatness: all joint pair-angles >= 35 deg (opposite-parity 1-vs-2 stations
at a joint sit at sqrt(5-4cos theta) and must clear the 1.3 cutoff).

[EXPLORATORY SEARCH — registers nothing]
"""
from __future__ import annotations

import numpy as np
from math import isqrt, acos, degrees
import itertools

SIDE_MAX = 60          # max triangle side
LEG_MAX = 60           # max |DA|, |DB|
ANGLE_MIN_DEG = 35.0


def exact_check(a, b, c, p, q, branch):
    """Exact integer re-check of one float-prefiltered hit. Returns the
    integer r or None."""
    X = p * p - q * q + c * c
    U = b * b + c * c - a * a
    SD = 4 * c * c * p * p - X * X
    SC = 4 * c * c * b * b - U * U
    if SD <= 0 or SC <= 0:
        return None
    s = isqrt(SD * SC)
    if s * s != SD * SC:
        return None
    T = (X - U) ** 2 + SD + SC - 2 * branch * s
    if T <= 0 or T % (4 * c * c) != 0:
        return None
    r2 = T // (4 * c * c)
    r = isqrt(r2)
    return r if r * r == r2 else None


def strictly_inside(c, Cx, Cy, Dx, Dy):
    if Dy <= 0 or Dy >= Cy:
        return False
    # left edge A(0,0)->C, right edge B(c,0)->C
    lft = Cx * Dy - Cy * Dx                 # cross(AC, AD) sign
    rgt = (Cx - c) * Dy - Cy * (Dx - c)     # cross(BC, BD) sign
    return lft < 0 and rgt > 0


def joint_angle_min(pts):
    adj = dict(A=["B", "C", "D"], B=["A", "C", "D"],
               C=["A", "B", "D"], D=["A", "B", "C"])
    worst = 180.0
    for v, nb in adj.items():
        for x, y in itertools.combinations(nb, 2):
            u1 = pts[x] - pts[v]
            u2 = pts[y] - pts[v]
            ca = float(u1 @ u2 / (np.linalg.norm(u1) * np.linalg.norm(u2)))
            worst = min(worst, degrees(acos(max(-1.0, min(1.0, ca)))))
    return worst


def main():
    print("=" * 72)
    print("Integer-distance planar K4 with interior joint — exhaustive")
    print(f"  sides <= {SIDE_MAX}, legs <= {LEG_MAX}, "
          f"joint angles >= {ANGLE_MIN_DEG} deg")
    print("=" * 72)
    p_arr = np.arange(1, LEG_MAX + 1, dtype=np.float64)
    q_arr = np.arange(1, LEG_MAX + 1, dtype=np.float64)
    P, Q = np.meshgrid(p_arr, q_arr, indexing="ij")
    cands = []
    n_exact_hits = 0
    for c in range(4, SIDE_MAX + 1):
        for b in range(4, SIDE_MAX + 1):
            for a in range(max(4, abs(b - c) + 1), min(SIDE_MAX, b + c - 1) + 1):
                U = b * b + c * c - a * a
                SCv = 4 * c * c * b * b - U * U
                if SCv <= 0:
                    continue
                Cx, Cy = U / (2 * c), np.sqrt(SCv) / (2 * c)
                X = P * P - Q * Q + c * c
                SD = 4 * c * c * P * P - X * X
                ok = SD > 0
                sqrt_prod = np.sqrt(np.where(ok, SD * SCv, 1.0))
                base = (X - U) ** 2 + SD + SCv
                for branch in (+1, -1):
                    T = base - 2 * branch * sqrt_prod
                    r_f = np.sqrt(np.where(ok & (T > 0), T, 1.0)) / (2 * c)
                    near = ok & (T > 0) & (np.abs(r_f - np.round(r_f)) < 1e-7) \
                        & (np.round(r_f) >= 1)
                    for pi_, qi_ in zip(*np.where(near)):
                        p, q = int(pi_) + 1, int(qi_) + 1
                        r = exact_check(a, b, c, p, q, branch)
                        if r is None:
                            continue
                        n_exact_hits += 1
                        Dx = (p * p - q * q + c * c) / (2 * c)
                        Dy = branch * np.sqrt(4 * c * c * p * p -
                                              (p * p - q * q + c * c) ** 2) / (2 * c)
                        if not strictly_inside(c, Cx, Cy, Dx, abs(Dy)):
                            continue
                        Dy = abs(Dy)
                        L = dict(AB=c, AC=b, BC=a, DA=p, DB=q, DC=r)
                        tri = [L["AB"] + L["DA"] + L["DB"],
                               L["BC"] + L["DB"] + L["DC"],
                               L["AC"] + L["DA"] + L["DC"],
                               L["AB"] + L["BC"] + L["AC"]]
                        parity_ok = all(t % 2 == 0 for t in tri)
                        pts = dict(A=np.array([0.0, 0.0]),
                                   B=np.array([float(c), 0.0]),
                                   C=np.array([Cx, Cy]),
                                   D=np.array([Dx, Dy]))
                        worst = joint_angle_min(pts)
                        if worst < ANGLE_MIN_DEG:
                            continue
                        n_chain = sum(L.values())
                        cands.append((worst, parity_ok, n_chain, L,
                                      dict(a=a, b=b, c=c, p=p, q=q, r=r,
                                           branch=branch)))
    # dedupe by sorted length multiset
    seen, uniq = set(), []
    for t in sorted(cands, key=lambda t: (-int(t[1]), t[2], -t[0])):
        key = tuple(sorted(t[3].values()))
        if key in seen:
            continue
        seen.add(key)
        uniq.append(t)
    print(f"exact integral quadruples (any position): {n_exact_hits}")
    print(f"interior + fat candidates: {len(cands)}  "
          f"(distinct length-sets: {len(uniq)}, "
          f"parity-native: {sum(1 for t in uniq if t[1])})")
    print(f"{'rank':>4} {'minang':>7} {'parity':>7} {'Nchain':>7}   lengths")
    for idx, (worst, pok, nch, L, raw) in enumerate(uniq[:15], 1):
        print(f"{idx:>4} {worst:>7.2f} {str(pok):>7} {nch:>7}   {L}  {raw}")
    if uniq:
        worst, pok, nch, L, raw = uniq[0]
        scale = 1 if pok else 2
        print(f"\nSELECTED for verification: {L} at scale x{scale} "
              f"(N = {nch * scale - 2} bodies), params {raw}")
    else:
        print("\nNO integer-distance interior-joint K4 exists at this "
              "diameter bound.")
    return 0


if __name__ == "__main__":
    main()
