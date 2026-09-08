"""H0-prime census: weights over the 24 velocity channels conserved by every collision.

Streaming permutes sites, not velocities, and the passive phase is invisible to velocity
weights, so the fixed kernel is {w : w . (1_F(A) - 1_A) = 0 for every occupancy set A}.
The 24x24 integer Gram matrix of the 2^24 constraint rows has the same rational kernel.
"""
from __future__ import annotations
from dataclasses import asdict
from math import gcd
import flint
import numpy as np
from ..recovery_hydro_invariants import fourth_rank_isotropy, span_membership  # noqa: F401
from . import channels as H

_BITS = np.arange(24, dtype=np.uint32)


def gram() -> np.ndarray:
    table = H.load_table()
    G = np.zeros((24, 24), dtype=np.int64)
    chunk = 1 << 18
    for start in range(0, 1 << 24, chunk):
        states = np.arange(start, start + chunk, dtype=np.uint32)
        A = ((states[:, None] >> _BITS[None, :]) & 1).astype(np.int64)
        F = ((table[start:start + chunk][:, None] >> _BITS[None, :]) & 1).astype(np.int64)
        delta = F - A
        G += delta.T @ delta
    return G


def _primitive(column):
    g = 0
    for value in column:
        g = gcd(g, abs(value))
    column = [value // g for value in column]
    if next(value for value in column if value) < 0:
        column = [-value for value in column]
    return tuple(column)


def fixed_kernel() -> tuple[tuple[int, ...], ...]:
    basis, nullity = flint.fmpz_mat(gram().tolist()).nullspace()
    return tuple(sorted(_primitive([int(basis[i, j]) for i in range(24)]) for j in range(nullity)))


def census() -> dict:
    basis = fixed_kernel()
    rows = [(1,) * 24] + [tuple(v[a] for v in H.VELOCITIES) for a in range(3)]
    iso = fourth_rank_isotropy([(v, 1) for v in H.VELOCITIES])
    return {"law": "phi-hydro-staged-candidate-1", "table_hash": H.TABLE_HASH,
            "fixed_dimension": len(basis), "basis": [list(b) for b in basis],
            "spans_number_and_momentum": bool(span_membership(basis, rows) and span_membership(rows, basis)),
            "isotropy": {**asdict(iso), "isotropic4": iso.isotropic4}}
