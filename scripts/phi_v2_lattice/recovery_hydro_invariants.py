"""Exact additive-invariant census of the field sector (gate H0).

Fixed-weight invariants of the three collision layers plus streaming, the
locked-schedule precessing invariants w_t = w_0 o U^{-t} with collision layer
(l0 - t) mod 3, the layer covariance U C_q = C_{q-1} U, and fourth-rank
velocity isotropy. Every result is an exact integer; no float enters any
acceptance. Nothing here evolves a state or changes the law.
"""
from __future__ import annotations

from dataclasses import dataclass
from itertools import product
from math import gcd

import flint
import numpy as np

from . import channels as C

N = 192  # channels per polarity


def _tables():
    tables = C.load_collision_tables()
    if C._hash_tables(tables) != C.COLLISION_HASH:
        raise ValueError("frozen collision identity changed")
    return tables


def u_powers() -> tuple[list[int], list[list[int]]]:
    """U and the maps U^0..U^11 on one polarity; U^12 must be the identity."""
    U = [C.U(c) for c in range(N)]
    powers = [list(range(N))]
    for _ in range(11):
        powers.append([U[i] for i in powers[-1]])
    if [U[i] for i in powers[11]] != list(range(N)):
        raise ValueError("U does not have period 12 on one polarity")
    return U, powers


def covariance_violations() -> int:
    """Rows of the exactly-two sector where U C_q differs from C_{q-1} U."""
    tables = _tables()
    U, _ = u_powers()
    violations = 0
    for q in range(3):
        for (a, b), (c, d) in tables[q].items():
            lhs = tuple(sorted((U[c], U[d])))
            rhs = tuple(sorted(tables[(q - 1) % 3][tuple(sorted((U[a], U[b])))]))
            violations += lhs != rhs
    return violations


def _collision_rows(table, relabel):
    """Sparse constraint rows sum_before w(relabel c) = sum_after w(relabel c)."""
    for before, after in table.items():
        row = {}
        for c in before:
            row[relabel[c]] = row.get(relabel[c], 0) - 1
        for c in after:
            row[relabel[c]] = row.get(relabel[c], 0) + 1
        if any(row.values()):
            yield row


def _gram_kernel(rows) -> tuple[tuple[int, ...], ...]:
    """Primitive integer basis of the common kernel of sparse integer rows.

    The Gram matrix A^T A has the same rational kernel as A, so the 192x192
    integer Gram matrix replaces a matrix with hundreds of thousands of rows.
    """
    gram = np.zeros((N, N), dtype=np.int64)
    for row in rows:
        items = [(i, v) for i, v in row.items() if v]
        for i, a in items:
            for j, b in items:
                gram[i, j] += a * b
    basis, nullity = flint.fmpz_mat(gram.tolist()).nullspace()
    out = []
    for j in range(nullity):
        column = [int(basis[i, j]) for i in range(N)]
        g = 0
        for value in column:
            g = gcd(g, abs(value))
        column = [value // g for value in column]
        if next(value for value in column if value) < 0:
            column = [-value for value in column]
        out.append(tuple(column))
    return tuple(sorted(out))


def fixed_kernel() -> tuple[tuple[int, ...], ...]:
    """Weights conserved by every collision layer and by streaming."""
    tables = _tables()
    U, _ = u_powers()
    identity = list(range(N))
    rows = [row for table in tables for row in _collision_rows(table, identity)]
    rows.extend({c: -1, U[c]: 1} for c in range(N) if U[c] != c)
    return _gram_kernel(rows)


def locked_kernel(l0: int) -> tuple[tuple[int, ...], ...]:
    """Weights w_0 such that w_0 o U^{-t} is conserved by layer (l0-t) mod 3, t=0..11."""
    if l0 not in (0, 1, 2):
        raise ValueError("starting layer must be 0, 1 or 2")
    tables = _tables()
    _, powers = u_powers()
    rows = []
    for t in range(12):
        inverse = [0] * N
        for i, j in enumerate(powers[t]):
            inverse[j] = i  # inverse[c] = U^{-t}(c)
        rows.extend(_collision_rows(tables[(l0 - t) % 3], inverse))
    return _gram_kernel(rows)


def span_membership(basis, vectors) -> bool:
    rows = [list(map(int, row)) for row in basis]
    base_rank = flint.fmpz_mat(rows).rank()
    return flint.fmpz_mat(rows + [list(map(int, v)) for v in vectors]).rank() == base_rank


def tangent_rows() -> tuple[tuple[int, ...], ...]:
    return tuple(tuple(C.tangent(c)[axis] for c in range(N)) for axis in range(3))


def layer_rows(layer: int) -> tuple[tuple[int, ...], ...]:
    return tuple(tuple(C.layer_value_of(c, layer)[k] for c in range(N)) for k in range(6))


@dataclass(frozen=True)
class Isotropy:
    T_xxxx: int
    T_xxyy: int
    T_xx: int
    T_xy: int

    @property
    def isotropic4(self) -> bool:
        return self.T_xxxx == 3 * self.T_xxyy

    @property
    def isotropic2(self) -> bool:
        return self.T_xy == 0


def fourth_rank_isotropy(velocities) -> Isotropy:
    """velocities: iterable of ((x, y, z), multiplicity) with integer entries."""
    xxxx = xxyy = xx = xy = 0
    for (x, y, z), m in velocities:
        xxxx += m * x ** 4
        xxyy += m * x * x * y * y
        xx += m * x * x
        xy += m * x * y
    return Isotropy(xxxx, xxyy, xx, xy)


_ALL = tuple(product((-1, 0, 1), repeat=3))
FACE = tuple(v for v in _ALL if sum(map(abs, v)) == 1)
EDGE = tuple(v for v in _ALL if sum(map(abs, v)) == 2)
CORNER = tuple(v for v in _ALL if sum(map(abs, v)) == 3)
VELOCITY_SETS = {
    "body_diagonal_8": tuple((v, 1) for v in CORNER),
    "face_edge_18": tuple((v, 1) for v in FACE + EDGE),
    "moore_26": tuple((v, 1) for v in FACE + EDGE + CORNER),
    "fchc_projected_18": tuple((v, 2) for v in FACE) + tuple((v, 1) for v in EDGE),
}


def census() -> dict:
    fixed = fixed_kernel()
    locked = {str(l0): locked_kernel(l0) for l0 in range(3)}
    return {
        "law_id": "phi-v2-staged-candidate-1",
        "collision_sha256": C.COLLISION_HASH,
        "covariance_violations": covariance_violations(),
        "fixed_dimension_per_polarity": len(fixed),
        "fixed_basis": fixed,
        "locked_dimension_per_polarity": {k: len(v) for k, v in locked.items()},
        "locked_basis": locked,
        "tangent_in_locked_span": span_membership(locked["0"], tangent_rows()),
        "tangent_in_fixed_span": span_membership(fixed, tangent_rows()),
        "isotropy": {name: {"T_xxxx": r.T_xxxx, "T_xxyy": r.T_xxyy, "T_xx": r.T_xx,
                            "T_xy": r.T_xy, "isotropic4": r.isotropic4, "isotropic2": r.isotropic2}
                     for name, r in ((n, fourth_rank_isotropy(v)) for n, v in VELOCITY_SETS.items())},
        "scope": "field sector on the homogeneous doubly occupied background; linear additive "
                 "invariants with fixed or U-precessing weights; nonlinear and mixed-record "
                 "observables not classified",
    }
