"""Canonical FCHC-projected velocity set on the cubic lattice and its symmetry group.

Faces: six directions, each doubled by a binary w label (the FCHC fourth-coordinate
sign), speed 1. Edges: twelve FCC directions, speed sqrt(2). Channel index within a
site: c = pol*96 + k*24 + v with polarity pol in {0,1}, passive C4 phase k in 0..3,
velocity v in 0..23. Nothing here is fitted; every table is a finite enumeration.
"""
from __future__ import annotations

import hashlib
import itertools
import os
from pathlib import Path

import numpy as np

from .. import geometry as G

N_VEL, N_PHASE, N_POL = 24, 4, 2
N_CHANNELS = N_VEL * N_PHASE * N_POL  # 192


def _faces():
    out = []
    for axis in range(3):
        for sign in (1, -1):
            for w in (0, 1):
                v = [0, 0, 0]
                v[axis] = sign
                out.append((tuple(v), w))
    return out


def _edges():
    out = []
    for a, b in itertools.combinations(range(3), 2):
        for sa, sb in ((1, 1), (1, -1), (-1, 1), (-1, -1)):
            v = [0, 0, 0]
            v[a], v[b] = sa, sb
            out.append((tuple(v), -1))
    return out


_LIST = _faces() + _edges()
VELOCITIES = tuple(v for v, _ in _LIST)
FACE_W = tuple(w for _, w in _LIST)
IS_FACE = tuple(w >= 0 for w in FACE_W)
_INDEX = {key: i for i, key in enumerate(_LIST)}


def channel(pol: int, k: int, v: int) -> int:
    assert 0 <= pol < N_POL and 0 <= k < N_PHASE and 0 <= v < N_VEL
    return pol * (N_PHASE * N_VEL) + k * N_VEL + v


def unpack(c: int) -> tuple[int, int, int]:
    assert 0 <= c < N_CHANNELS
    pol, rest = divmod(c, N_PHASE * N_VEL)
    k, v = divmod(rest, N_VEL)
    return pol, k, v


def _group():
    identity = ((1, 0, 0), (0, 1, 0), (0, 0, 1))
    elements = []
    for perm in itertools.permutations(range(3)):
        for signs in itertools.product((1, -1), repeat=3):
            m = [[0, 0, 0] for _ in range(3)]
            for i in range(3):
                m[i][perm[i]] = signs[i]
            for wflip in (0, 1):
                elements.append((tuple(tuple(row) for row in m), wflip))
    elements.sort(key=lambda e: (e != (identity, 0), e))
    return tuple(elements)


GROUP = _group()


def _perm():
    perm = np.zeros((len(GROUP), N_VEL), dtype=np.int64)
    for g, (m, wflip) in enumerate(GROUP):
        for c, v in enumerate(VELOCITIES):
            image = tuple(int(sum(m[i][j] * v[j] for j in range(3))) for i in range(3))
            w = (FACE_W[c] ^ wflip) if IS_FACE[c] else -1
            perm[g, c] = _INDEX[(image, w)]
    return perm


PERM = _perm()
_PERM_INDEX = {tuple(PERM[g].tolist()): g for g in range(len(GROUP))}


def _mul():
    mul = np.zeros((len(GROUP), len(GROUP)), dtype=np.int64)
    for g in range(len(GROUP)):
        for h in range(len(GROUP)):
            mul[g, h] = _PERM_INDEX[tuple(PERM[g][PERM[h]].tolist())]  # (g o h)(c) = g(h(c))
    return mul


MUL = _mul()
INVERSE = np.array([_PERM_INDEX[tuple(np.argsort(PERM[g]).tolist())] for g in range(len(GROUP))], dtype=np.int64)
CONJ = np.array([[MUL[MUL[g, h], INVERSE[g]] for h in range(len(GROUP))] for g in range(len(GROUP))], dtype=np.int64)


def relation_target(v: int):
    """Relation traversed by a particle of velocity v leaving a site x:
    ('sc', owner_offset, (axis,)) or ('fcc', owner_offset, (plane, diag))."""
    vec = VELOCITIES[v]
    offset = [0, 0, 0]
    if IS_FACE[v]:
        axis = next(i for i in range(3) if vec[i])
        if vec[axis] < 0:
            offset[axis] = -1
        return ("sc", tuple(offset), (axis,))
    plane = next(i for i in range(3) if vec[i] == 0)
    a, b = G.plane_axes(plane)
    sa, sb = vec[a], vec[b]
    if (sa, sb) == (1, 1):
        return ("fcc", (0, 0, 0), (plane, 0))
    if (sa, sb) == (-1, -1):
        offset[a] = offset[b] = -1
        return ("fcc", tuple(offset), (plane, 0))
    if (sa, sb) == (-1, 1):
        offset[a] = -1
        return ("fcc", tuple(offset), (plane, 1))
    offset[b] = -1
    return ("fcc", tuple(offset), (plane, 1))


ENCODING_HASH = hashlib.sha256(repr((VELOCITIES, FACE_W, GROUP)).encode()).hexdigest()
TABLE_HASH = "094d6f42dae6c84383b347305aa9bb570fb93fabc21b1d093dc86b4a26900e52"


def table_path() -> Path:
    root = Path(__file__).resolve().parents[3]
    default = root / "engine" / "build_strict_hydro_tables" / f"hydro_collision_{TABLE_HASH[:16]}.u32"
    return Path(os.environ.get("FTD_HYDRO_TABLE", default))


def load_table() -> np.ndarray:
    path = table_path()
    if not path.is_file():
        raise FileNotFoundError(f"generate the collision table first: {path}")
    data = path.read_bytes()
    if hashlib.sha256(data).hexdigest() != TABLE_HASH:
        raise ValueError("collision table hash mismatch")
    table = np.frombuffer(data, dtype="<u4")
    if table.shape != (1 << N_VEL,):
        raise ValueError("collision table has wrong length")
    return table
