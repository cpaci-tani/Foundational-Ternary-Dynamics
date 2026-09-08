"""Four-microtick staged schedule of phi-hydro-staged-candidate-1."""
from __future__ import annotations
from dataclasses import dataclass
from numbers import Integral
import numpy as np
from .. import geometry as G
from .._proofs import encode, rotate, relation_tick, phase_index, readout
from ..tick import gate
from . import channels as H, state as S, tick as T

LAW_ID = "phi-hydro-staged-candidate-1"


@dataclass
class StagedState:
    microtick: int
    lattice: S.LatticeState
    admitted_sc: np.ndarray    # bool (N,3)
    admitted_fcc: np.ndarray   # bool (N,3,2)
    gate_sc: np.ndarray        # bool (N,3)
    gate_fcc: np.ndarray       # bool (N,3,2)

    @property
    def phase(self) -> int:
        return self.microtick % 4


def _array(value, dtype, shape, name, bounds=None):
    if not isinstance(value, np.ndarray) or value.dtype != np.dtype(dtype) or value.shape != shape:
        raise ValueError(f"{name}: expected {dtype} array of shape {shape}")
    if value.dtype == np.dtype(bool) and np.any(value.view(np.uint8) > 1):
        raise ValueError(f"{name}: noncanonical boolean bytes")
    if bounds is not None and (np.any(value < bounds[0]) or np.any(value > bounds[1])):
        raise ValueError(f"{name}: outside finite alphabet {bounds}")


def _validate_lattice(st):
    if not isinstance(st, S.LatticeState):
        raise ValueError("expected hydro LatticeState")
    if isinstance(st.L, bool) or not isinstance(st.L, Integral) or st.L < 3:
        raise ValueError("periodic candidate requires integer L >= 3")
    n = int(st.L) ** 3
    _array(st.s, "int8", (n,), "s", (-1, 1))
    _array(st.bank, "bool", (n, H.N_CHANNELS), "bank")
    _array(st.sc, "int8", (n, 3, 2), "sc", (0, 8))
    _array(st.fcc, "int8", (n, 3, 2, 2), "fcc", (0, 8))
    if n and st.bank.reshape(n, 2, 4, 24).sum(axis=2).max() > 1:
        raise ValueError("velocity exclusion violated")
    return n


def validate(state: StagedState) -> None:
    if not isinstance(state, StagedState):
        raise ValueError("expected StagedState")
    if isinstance(state.microtick, bool) or not isinstance(state.microtick, Integral) or state.microtick < 0:
        raise ValueError("microtick must be a nonnegative integer")
    n = _validate_lattice(state.lattice)
    for name, shape in (("admitted_sc", (n, 3)), ("admitted_fcc", (n, 3, 2)), ("gate_sc", (n, 3)), ("gate_fcc", (n, 3, 2))):
        _array(getattr(state, name), "bool", shape, name)
    if state.phase == 0 and (state.admitted_sc.any() or state.admitted_fcc.any()
                             or state.gate_sc.any() or state.gate_fcc.any()):
        raise ValueError("cycle boundary must have cleared pending controls")


def initialize(lattice: S.LatticeState) -> StagedState:
    n = _validate_lattice(lattice)
    state = StagedState(0, S.copy(lattice), np.zeros((n, 3), bool), np.zeros((n, 3, 2), bool),
                        np.zeros((n, 3), bool), np.zeros((n, 3, 2), bool))
    validate(state)
    return state


def work_units(state: StagedState) -> int:
    validate(state)
    st = state.lattice
    return int(st.bank.sum()) + int((st.sc != S.BLANK_IDX).sum()) + int((st.fcc != S.BLANK_IDX).sum())


def _cross(pair, even):
    lam, rho = map(S.z_of, pair)
    lo, ro = readout(lam)[0], readout(rho)[0]
    held = bool(lo) != bool(ro)
    held = held and phase_index(lam if lo else rho) == 0 and not even
    left, right = relation_tick(lam, rho, even_gate=bool(even))
    direction = 1 if lo and not readout(left)[0] else (-1 if not lo and readout(left)[0] else 0)
    return (S.idx_of(left), S.idx_of(right)), direction, held


def step(state: StagedState, table=None):
    validate(state)
    table = H.load_table() if table is None else table
    st = state.lattice
    out = StagedState(int(state.microtick) + 1, S.copy(st), state.admitted_sc.copy(), state.admitted_fcc.copy(),
                      state.gate_sc.copy(), state.gate_fcc.copy())
    dst = out.lattice
    events = T.TickEvents()
    n = S.n_sites(st)
    if state.phase == 0:
        for i in range(n):
            for a in range(3):
                out.gate_sc[i, a] = bool(gate(st, *G.sc_endpoints(st.L, i, a)))
            for p in range(3):
                for q in range(2):
                    out.gate_fcc[i, p, q] = bool(gate(st, *G.fcc_endpoints(st.L, i, p, q)))
        for (kind, owner, idx), (x, c) in T.admitted_absorptions(st).items():
            pol = H.unpack(c)[0]
            dst.bank[x, c] = False
            reserve = S.idx_of(rotate(encode(2, +1 if pol == 0 else -1)))
            if kind == "sc":
                dst.sc[owner, idx[0]] = (S.BLANK_IDX, reserve)
                out.admitted_sc[owner, idx[0]] = True
            else:
                dst.fcc[owner, idx[0], idx[1]] = (S.BLANK_IDX, reserve)
                out.admitted_fcc[owner, idx[0], idx[1]] = True
            events.absorptions.append((x, c, kind, owner, idx))
    elif state.phase == 1:
        for i in np.flatnonzero(st.bank.any(axis=1)).tolist():
            dst.bank[i], evs = T.collide_site(st.bank[i], table)
            events.collisions.extend((i, pol, before, after) for pol, before, after in evs)
        for i in range(n):
            for a in range(3):
                if state.admitted_sc[i, a]:
                    continue
                dst.sc[i, a], direction, held = _cross(st.sc[i, a], state.gate_sc[i, a])
                if direction:
                    events.crossings.append(("sc", i, (a,), direction))
                if held:
                    events.gate_holds.append(("sc", i, (a,)))
            for p in range(3):
                for q in range(2):
                    if state.admitted_fcc[i, p, q]:
                        continue
                    dst.fcc[i, p, q], direction, held = _cross(st.fcc[i, p, q], state.gate_fcc[i, p, q])
                    if direction:
                        events.crossings.append(("fcc", i, (p, q), direction))
                    if held:
                        events.gate_holds.append(("fcc", i, (p, q)))
    elif state.phase == 2:
        dst.bank[:] = T.stream(st, st.bank)
    else:
        dst.s[:] = T.manifest(st, st.sc, st.fcc)
        out.admitted_sc.fill(False); out.admitted_fcc.fill(False)
        out.gate_sc.fill(False); out.gate_fcc.fill(False)
    validate(out)
    return out, events
