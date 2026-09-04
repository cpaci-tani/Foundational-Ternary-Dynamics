from __future__ import annotations
from dataclasses import dataclass
import numpy as np
from . import state as S, tick as T
from ._proofs import readout, rotate


def _orbit(idx):
    z = S.z_of(idx); zs = set(); w = z
    for _ in range(4):
        zs.add(w); w = rotate(w)
    return frozenset(zs)


def _occ(idx): return int(readout(S.z_of(idx))[0])
def _pol(idx): return int(readout(S.z_of(idx))[1]) if _occ(idx) else 0


@dataclass
class RelationCensus:
    period: int | None; duty: int | None; orbit_constant: bool; polarity_constant: bool
    in_place_flips: int; nulls: int; bounces: int; reversals: int


def relation_census(journal, kind, owner, idx, horizon) -> RelationCensus:
    series = journal.relation_series(kind, owner, idx, horizon)
    prim = [_occ(l) for l, _ in series]
    tokens = [(l if _occ(l) else r) for l, r in series]
    orbits = {_orbit(t) for t in tokens if _occ(t)}
    pols = {_pol(t) for t in tokens if _occ(t)}
    # period of the (lambda, rho) pair
    period = next((p for p in range(1, horizon) if series[p:] == series[:-p] and series[0] == series[p]), None)
    duty = sum(prim[:period]) if period else None
    # in-place polarity flip of the token
    flips = sum(1 for a, b in zip(tokens, tokens[1:]) if _occ(a) and _occ(b) and _pol(a) == -_pol(b))
    # site-like readout of the primary slot: nulls entered from sign s, exited to sign s'
    sig = [_pol(l) for l, _ in series]; nulls = bounces = reversals = 0; n = len(sig)
    for i in range(1, n):
        if sig[i] == 0 and sig[i - 1] != 0:
            j = i
            while j < n and sig[j] == 0: j += 1
            if j < n:
                nulls += 1
                if sig[j] == sig[i - 1]: bounces += 1
                else: reversals += 1
    return RelationCensus(period, duty, len(orbits) <= 1, len(pols) <= 1, flips, nulls, bounces, reversals)


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
        for t in range(1, len(seq)):
            if seq[t] == 0 and seq[t - 1] != 0:
                j = t
                while j < len(seq) and seq[j] == 0: j += 1
                if j < len(seq):
                    if seq[j] == seq[t - 1]: out["null_bounce"] += 1
                    else: out["null_reversal"] += 1
    return out
