# scripts/phi_v2_lattice/census.py
"""Corrected-criterion census per relation (measured from FIRST OCCUPATION, so tokens created
mid-horizon by absorption are judged on their own history) and per site."""
from __future__ import annotations
from dataclasses import dataclass
from . import state as S, tick as T
from ._proofs import readout, rotate, phase_index


def _orbit(idx):
    z = S.z_of(idx); zs = set(); w = z
    for _ in range(4):
        zs.add(w); w = rotate(w)
    return frozenset(zs)


def _occ(idx): return int(readout(S.z_of(idx))[0])
def _pol(idx): return int(readout(S.z_of(idx))[1]) if _occ(idx) else 0
def _phase(idx): return phase_index(S.z_of(idx))


def _classify_nulls(sig):
    """Nulls of a signed readout sequence: entered from sign s, exited to s' (bounce iff s' == s)."""
    nulls = bounces = reversals = 0; n = len(sig)
    for i in range(1, n):
        if sig[i] == 0 and sig[i - 1] != 0:
            j = i
            while j < n and sig[j] == 0: j += 1
            if j < n:
                nulls += 1
                if sig[j] == sig[i - 1]: bounces += 1
                else: reversals += 1
    return nulls, bounces, reversals


def _runs(bits):
    """(value, length) of the maximal constant runs of a sequence."""
    runs = []; start = 0
    for i in range(1, len(bits) + 1):
        if i == len(bits) or bits[i] != bits[start]:
            runs.append((bits[start], i - start)); start = i
    return runs


@dataclass
class RelationCensus:
    period: int | None; duty: int | None; orbit_constant: bool; polarity_constant: bool
    in_place_flips: int; nulls: int; bounces: int; reversals: int
    first_occupied: int | None      # series index at which the relation first holds a token (None: never)
    phase_steps_ok: bool            # the token's phase advances by exactly +1 mod 4 every tick while present
    crossings_at_phase0: bool       # every primary-occupancy change happens with the token's previous phase == 0
    runs_mod4_ok: bool              # every INTERIOR occupancy run after first occupation has length == 0 mod 4


def relation_census(journal, kind, owner, idx, horizon) -> RelationCensus:
    series = journal.relation_series(kind, owner, idx, horizon)
    tokens = [(l if _occ(l) else r) for l, r in series]
    first = next((i for i, t in enumerate(tokens) if _occ(t)), None)
    if first is None:
        return RelationCensus(None, None, True, True, 0, 0, 0, 0, None, True, True, True)
    tail = series[first:]; toks = tokens[first:]; prim = [_occ(l) for l, _ in tail]; n = len(tail)
    orbits = {_orbit(t) for t in toks if _occ(t)}
    pols = {_pol(t) for t in toks if _occ(t)}
    period = next((p for p in range(1, n) if tail[p:] == tail[:-p]), None)
    duty = sum(prim[:period]) if period else None
    flips = sum(1 for a, b in zip(toks, toks[1:]) if _occ(a) and _occ(b) and _pol(a) == -_pol(b))
    nulls, bounces, reversals = _classify_nulls([_pol(l) for l, _ in tail])
    phase_ok = all(_phase(b) == (_phase(a) + 1) % 4 for a, b in zip(toks, toks[1:]) if _occ(a) and _occ(b))
    cross_ok = all(_phase(toks[t - 1]) == 0 for t in range(1, n) if prim[t] != prim[t - 1])
    runs_ok = all(length % 4 == 0 for _, length in _runs(prim)[1:-1])
    return RelationCensus(period, duty, len(orbits) <= 1, len(pols) <= 1, flips, nulls, bounces, reversals,
                          first, phase_ok, cross_ok, runs_ok)


def site_census(states) -> dict:
    out = dict(manifest=0, withdraw=0, flip_in_place=0, flip_via_wrap=0, null_bounce=0, null_reversal=0)
    N = S.n_sites(states[0]); Q = [T._incidence(st, st.sc, st.fcc) for st in states]
    for i in range(N):
        seq = [int(st.s[i]) for st in states]
        for t in range(1, len(seq)):
            a, b = seq[t - 1], seq[t]
            if a == 0 and b != 0: out["manifest"] += 1
            elif a != 0 and b == 0: out["withdraw"] += 1
            elif a == -b and a != 0:
                out["flip_in_place"] += 1
                if abs(Q[t - 1][i]) >= 2 or abs(Q[t][i]) >= 2: out["flip_via_wrap"] += 1
        _, bounce, reversal = _classify_nulls(seq)
        out["null_bounce"] += bounce; out["null_reversal"] += reversal
    return out
