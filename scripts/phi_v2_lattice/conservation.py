from __future__ import annotations
import numpy as np
from . import channels as C, state as S, tick as T


def work_units(st: S.LatticeState) -> int:
    return int(st.bank.sum()) + int((st.sc != S.BLANK_IDX).sum()) + int((st.fcc != S.BLANK_IDX).sum())


def current_from_events(st: S.LatticeState, ev) -> tuple[np.ndarray, np.ndarray]:
    """J_r rebuilt from the tick's crossing LOG alone (direction x polarity of the moved token,
    read from the PRE-state slot: 0 for +1 primary->reserve, 1 for -1). Independent of
    tick._current's eps-difference route, so the Gauss check below is not a tautology."""
    Jsc = np.zeros(st.sc.shape[:2], dtype=int); Jfcc = np.zeros(st.fcc.shape[:3], dtype=int)
    for kind, owner, idx, dirn in ev.crossings:
        slot = 0 if dirn == +1 else 1
        tok = st.sc[owner, idx[0], slot] if kind == "sc" else st.fcc[owner, idx[0], idx[1], slot]
        pol = T._eps(tok)
        if kind == "sc":
            Jsc[owner, idx[0]] = dirn * pol
        else:
            Jfcc[owner, idx[0], idx[1]] = dirn * pol
    return Jsc, Jfcc


def gauss_residual(before: S.LatticeState, after: S.LatticeState, ev) -> int:
    """max |dQ + div J| with J from the crossing LOG, plus the disagreement between the log route
    and the eps-difference route. 0 iff both the identity and the tick's bookkeeping hold."""
    Qb = T._incidence(before, before.sc, before.fcc); Qa = T._incidence(after, after.sc, after.fcc)
    J_ev = current_from_events(before, ev)
    div = T._divergence(before, J_ev)
    Jsc, Jfcc = T._current(before, after)
    mismatch = int(np.abs(Jsc - J_ev[0]).max(initial=0)) + int(np.abs(Jfcc - J_ev[1]).max(initial=0))
    return int(np.abs(Qa - Qb + div).max(initial=0)) + mismatch


def layer_sum(st: S.LatticeState) -> tuple[int, ...]:
    total = np.zeros(6, dtype=int)
    xs, cs = np.nonzero(st.bank)
    for x, c in zip(xs.tolist(), cs.tolist()):
        total += np.array(C.layer_value_of(c, int(st.ell[x])), dtype=int)
    return tuple(int(v) for v in total)
