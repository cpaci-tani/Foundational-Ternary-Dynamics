"""Experimental four-physical-tick causal candidate; NOT canonical Phi-v2.

All token payload lives in ``lattice``. Pending arrays are finite control bits
owned by the same SC/FCC record anchors as the corresponding relation arrays.
The schedule is part of the declared law, indexed by the global ordinal clock.
"""
from __future__ import annotations

from dataclasses import dataclass
from functools import lru_cache
from types import MappingProxyType
import numpy as np

from . import channels as C, geometry as G, state as S, tick as T
from ._proofs import encode, rotate, relation_tick, phase_index, readout

LAW_ID = "phi-v2-staged-candidate-1"

# Concrete numeric storage is allowed; user-defined arithmetic is not state.
_INTEGER_TYPES = (int, np.int8, np.int16, np.int32, np.int64,
                           np.uint8, np.uint16, np.uint32, np.uint64,
                           np.longlong, np.ulonglong)


def _plain_integer(value, name, minimum=0):
    if not any(type(value) is allowed for allowed in _INTEGER_TYPES) or int(value) < minimum:
        raise ValueError(f"{name} must be a concrete integer >= {minimum}")
    return int(value)


@dataclass
class StagedState:
    microtick: int
    lattice: S.LatticeState
    admitted_sc: np.ndarray
    gate_sc: np.ndarray
    gate_fcc: np.ndarray

    @property
    def phase(self) -> int:
        return self.microtick % 4


def _array(value, dtype, shape, name, bounds=None):
    if type(value) is not np.ndarray or value.dtype != np.dtype(dtype) or value.shape != shape:
        raise ValueError(f"{name}: expected {dtype} array of shape {shape}")
    if value.dtype == np.dtype(bool) and np.any(value.view(np.uint8) > 1):
        raise ValueError(f"{name}: noncanonical boolean bytes")
    if bounds is not None and (np.any(value < bounds[0]) or np.any(value > bounds[1])):
        raise ValueError(f"{name}: outside finite alphabet {bounds}")


def _validate_lattice(st):
    if type(st) is not S.LatticeState:
        raise ValueError("expected LatticeState payload")
    n = _plain_integer(st.L, "periodic lattice side", 3) ** 3
    for name, shape, bounds in (("s", (n,), (-1, 1)), ("ell", (n,), (0, 2)),
                                ("sc", (n, 3, 2), (0, 8)), ("fcc", (n, 3, 2, 2), (0, 8))):
        _array(getattr(st, name), "int8", shape, name, bounds)
    _array(st.bank, "bool", (n, C.N_CHANNELS), "bank")
    return n


def validate(state: StagedState) -> None:
    if type(state) is not StagedState:
        raise ValueError("expected StagedState")
    _plain_integer(state.microtick, "microtick")
    st = state.lattice
    n = _validate_lattice(st)
    for name, shape in (("admitted_sc", (n, 3)), ("gate_sc", (n, 3)), ("gate_fcc", (n, 3, 2))):
        _array(getattr(state, name), "bool", shape, name)
    if state.phase == 0:
        if state.admitted_sc.any() or state.gate_sc.any() or state.gate_fcc.any():
            raise ValueError("cycle boundary must have cleared pending controls")
    else:
        pairs = st.sc[state.admitted_sc]
        reserves = (S.idx_of(rotate(encode(2, +1))), S.idx_of(rotate(encode(2, -1))))
        if len(pairs) and (np.any(pairs[:, 0] != S.BLANK_IDX) or not np.isin(pairs[:, 1], reserves).all()):
            raise ValueError("admitted relation must retain its blank primary and absorbed reserve")


def initialize(lattice: S.LatticeState) -> StagedState:
    """Copy a valid preparation; initial s is an explicitly allowed lagged readout."""
    n = _validate_lattice(lattice)
    state = StagedState(0, S.copy(lattice), np.zeros((n, 3), dtype=bool),
                        np.zeros((n, 3), dtype=bool), np.zeros((n, 3, 2), dtype=bool))
    validate(state)
    return state


def work_units(state: StagedState) -> int:
    validate(state)
    st = state.lattice
    return int(st.bank.sum()) + int((st.sc != S.BLANK_IDX).sum()) + int((st.fcc != S.BLANK_IDX).sum())


@lru_cache(maxsize=1)
def _default_tables():
    return _checked_tables(C.load_collision_tables())


def _checked_tables(tables):
    """Freeze the very lookups whose contents pass the law identity check.

    Hashing ``items()`` on a mapping subclass does not constrain its lookup
    behavior. Copy only concrete dictionaries with concrete finite pair leaves.
    The owned read-only snapshot also prevents later caller mutation.
    """
    if type(tables) is not tuple or len(tables) != 3 or any(type(t) is not dict for t in tables):
        raise ValueError("collision tables must be a tuple of three concrete dictionaries")
    snapshots = tuple(dict(table) for table in tables)
    for table in snapshots:
        for before, after in table.items():
            for pair in (before, after):
                if (type(pair) is not tuple or len(pair) != 2
                        or any(type(value) is not int for value in pair)):
                    raise ValueError("collision table pairs must contain concrete integer values")
    if C._hash_tables(snapshots) != C.COLLISION_HASH:
        raise ValueError("collision table does not match candidate law")
    return tuple(MappingProxyType(table) for table in snapshots)


def _cross(pair, even):
    lam, rho = map(S.z_of, pair)
    lo, ro = readout(lam)[0], readout(rho)[0]
    held = bool(lo) != bool(ro)
    held = held and phase_index(lam if lo else rho) == 0 and not even
    left, right = relation_tick(lam, rho, even_gate=bool(even))
    direction = 1 if lo and not readout(left)[0] else (-1 if not lo and readout(left)[0] else 0)
    return (S.idx_of(left), S.idx_of(right)), direction, held


def step(state: StagedState, tables=None) -> tuple[StagedState, T.TickEvents]:
    """Advance exactly ONE physical tick; event rows are external observations."""
    validate(state)
    st = state.lattice
    out = StagedState(int(state.microtick) + 1, S.copy(st), state.admitted_sc.copy(),
                      state.gate_sc.copy(), state.gate_fcc.copy())
    dst = out.lattice
    events = T.TickEvents()
    if state.phase == 0:
        for i in range(S.n_sites(st)):
            for a in range(3):
                out.gate_sc[i, a] = bool(T.gate(st, *G.sc_endpoints(st.L, i, a)))
            for p in range(3):
                for q in range(2):
                    out.gate_fcc[i, p, q] = bool(T.gate(st, *G.fcc_endpoints(st.L, i, p, q)))
        for (owner, axis), (x, c) in T.admitted_absorptions(st).items():
            dst.bank[x, c] = False
            dst.sc[owner, axis] = (S.BLANK_IDX, S.idx_of(rotate(encode(2, C.polarity(c)))))
            out.admitted_sc[owner, axis] = True
            events.absorptions.append((x, c, owner, axis))
    elif state.phase == 1:
        tables = _default_tables() if tables is None else _checked_tables(tables)
        for i in np.flatnonzero(st.bank.any(axis=1)):
            dst.bank[i], evs = T.collide_row(st.bank[i], st.ell[i], tables)
            events.collisions.extend((int(i), eps, before, after) for eps, before, after in evs)
        dst.ell[:] = (st.ell.astype(int) - 1) % 3
        for i in range(S.n_sites(st)):
            for kind, pairs, gates, indices in (
                ("sc", dst.sc, state.gate_sc, ((a,) for a in range(3))),
                ("fcc", dst.fcc, state.gate_fcc, ((p, q) for p in range(3) for q in range(2)))):
                for idx in indices:
                    key = (i,) + idx
                    if kind == "sc" and state.admitted_sc[key]:
                        continue
                    pairs[key], direction, held = _cross(getattr(st, kind)[key], gates[key])
                    if direction:
                        events.crossings.append((kind, i, idx, direction))
                    if held:
                        events.gate_holds.append((kind, i, idx))
    elif state.phase == 2:
        dst.bank[:] = T.stream(st, st.bank)
    else:
        dst.s[:] = T.manifest(st, st.sc, st.fcc)
        out.admitted_sc.fill(False)
        out.gate_sc.fill(False)
        out.gate_fcc.fill(False)
    validate(out)
    return out, events


def checkpoint(state: StagedState) -> bytes:
    from .checkpoint import checkpoint as save
    return save(state)


def restore(data: bytes) -> StagedState:
    from .checkpoint import restore as read
    return read(data)
