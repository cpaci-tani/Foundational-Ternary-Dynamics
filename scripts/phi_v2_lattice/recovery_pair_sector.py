"""Exact pair-sector observations, with no binding or particle identification.

Collision vertices carry unordered channel pairs. Lifted common positions are
external histories; no persistent individual identity is assigned at collision.
"""
from dataclasses import dataclass
from collections import Counter
from functools import lru_cache
from math import lcm
from numbers import Integral

import numpy as np

from . import channels as C, geometry as G, staged as P, state as S
from . import recovery_kinetic_reference as K
from ._proofs import rotate


def _int(value, name, lo=0, hi=None):
    if isinstance(value, bool) or not isinstance(value, Integral) or value < lo or (hi is not None and value > hi):
        raise ValueError(f"invalid {name}")
    return int(value)


def _pair(pair):
    pair = tuple(_int(c, "channel", hi=383) for c in pair)
    if len(pair) != 2 or pair[0] == pair[1]:
        raise ValueError("two distinct co-located channels required")
    return tuple(sorted(pair))


@lru_cache(maxsize=1)
def _tables():
    return C.load_collision_tables()


def drift_numerator(channel):
    """Twelve times the free velocity in nodes per charged microtick."""
    c = _int(channel, "channel", hi=383)
    directions = (C.tangent(c), C.tangent(C.U(c)), C.tangent(C.U(C.U(c))))
    return tuple(sum(d[a] for d in directions) for a in range(3))


def free_displacement(channel, hops):
    c = _int(channel, "channel", hi=383)
    hops = _int(hops, "hops")
    quotient, remainder = divmod(hops, 3)
    drift = drift_numerator(c)
    answer = [quotient*d for d in drift]
    for _ in range(remainder):
        answer = [a+b for a,b in zip(answer,C.tangent(c))]
        c = C.U(c)
    return tuple(answer)


def free_relative(first, second, hops):
    """Change of second-minus-first separation, conditional on no collision."""
    return tuple(b-a for a,b in zip(free_displacement(first,hops), free_displacement(second,hops)))


def collide(pair, layer):
    pair = _pair(pair)
    layer = _int(layer, "layer", hi=2)
    if C.polarity(pair[0]) != C.polarity(pair[1]):
        return pair
    offset = 192 if pair[0] >= 192 else 0
    return tuple(c+offset for c in _tables()[layer][tuple(c-offset for c in pair)])


def same_flag(pair):
    a,b = _pair(pair)
    return C.STATES[a%192][0] == C.STATES[b%192][0]


@dataclass(frozen=True)
class Scattering:
    layer: int
    incoming: tuple
    outgoing: tuple
    free_hops: tuple
    actual_hops: tuple
    incoming_drift: tuple
    outgoing_drift: tuple
    layer_moment: tuple


def scattering(pair, layer):
    pair = _pair(pair)
    layer = _int(layer, "layer", hi=2)
    out = collide(pair,layer)
    total = lambda channels: tuple(sum(C.layer_value_of(c,layer)[a] for c in channels) for a in range(6))
    if total(pair) != total(out):
        raise ValueError("collision violates declared six-moment invariant")
    return Scattering(layer,pair,out,tuple(sorted(C.tangent(c) for c in pair)),
                      tuple(sorted(C.tangent(c) for c in out)),
                      tuple(sorted(drift_numerator(c) for c in pair)),
                      tuple(sorted(drift_numerator(c) for c in out)),total(pair))


@dataclass(frozen=True)
class ColocatedOrbit:
    states: tuple  # (layer, unordered positive-polarity local pair)
    cycle_displacement: tuple
    full_period_microticks: int
    full_period_displacement: tuple


@lru_cache(maxsize=1)
def colocated_orbits():
    """Exhaust exactly the 864 layer/same-flag pair states, not global histories."""
    domain = {(q,pair) for q,table in enumerate(_tables()) for pair in table if same_flag(pair)}
    seen, result = set(), []
    for initial in sorted(domain):
        if initial in seen:
            continue
        current, orbit, displacement = initial, [], [0,0,0]
        while current not in seen:
            if current not in domain:
                raise ValueError("collision leaves same-flag sector")
            seen.add(current); orbit.append(current)
            layer,pair = current
            out = collide(pair,layer)
            if not same_flag(out):
                raise ValueError("collision splits same-flag pair")
            displacement = [a+b for a,b in zip(displacement,C.tangent(out[0]))]
            current = ((layer-1)%3,tuple(sorted(C.U(c) for c in out)))
        if current != initial:
            raise ValueError("prepared finite map has a transient")
        cycles = lcm(len(orbit),4)  # relation phase must also return
        result.append(ColocatedOrbit(tuple(orbit),tuple(displacement),4*cycles,
                      tuple(d*(cycles//len(orbit)) for d in displacement)))
    return tuple(result)


def census():
    """Exact integer map counts. These are not physical momentum measurements."""
    layers=[]
    for q,table in enumerate(_tables()):
        counts=Counter()
        for pair in table:
            obs=scattering(pair,q)
            counts["rows"]+=1
            counts["hop_multiset_changed"]+=obs.free_hops!=obs.actual_hops
            counts["drift_multiset_changed"]+=obs.incoming_drift!=obs.outgoing_drift
            counts["drift_sum_changed"]+=tuple(map(sum,zip(*obs.incoming_drift)))!=tuple(map(sum,zip(*obs.outgoing_drift)))
            counts["same_flag_inputs"]+=same_flag(pair)
            if same_flag(pair)!=same_flag(obs.outgoing):
                raise ValueError("same-flag sector is not invariant in both directions")
        layers.append(dict(counts))
    return tuple(layers)


def prepare_pair(L, first, second, *, site=0, layer=0):
    L=_int(L,"size",3);site=_int(site,"site",hi=L**3-1)
    pair=_pair((first,second))
    bank=np.zeros((L**3,384),dtype=bool)
    bank[site,list(pair)]=True
    return K.prepare_bank(bank,L,layer)


@dataclass(frozen=True)
class ColocatedAudit:
    tick_start: int
    tick_end: int
    positions: tuple
    final_channels: tuple
    collision_vertices: int
    work: int
    binding_identified: bool = False


def audit_colocated(initial, cycles):
    """Bounded actual-law audit of repeated interactions and all pending records."""
    cycles=_int(cycles,"cycles")
    state=K.phase_lift(initial,0)  # validates full prepared boundary, makes private copy
    occupied=np.argwhere(state.lattice.bank)
    if len(occupied)!=2 or occupied[0,0]!=occupied[1,0]:
        raise ValueError("co-located two-token preparation required")
    pair=_pair(occupied[:,1]); site=int(occupied[0,0])
    if not same_flag(pair):
        raise ValueError("audit requires invariant same-flag preparation")
    position=tuple(G.coords(state.lattice.L,site));positions=[position]
    vertices=0;work=P.work_units(state)
    background=int(state.lattice.sc.flat[0]);layer=int(state.lattice.ell[0])
    for elapsed in range(1,4*cycles+1):
        phase=state.phase
        expected_pair=collide(pair,layer) if phase==1 else pair
        if phase==1:
            layer=(layer-1)%3;background=S.idx_of(rotate(S.z_of(background)))
        if phase==2:
            hop=C.tangent(pair[0]);site=G.shift(state.lattice.L,site,hop)
            position=tuple(a+b for a,b in zip(position,hop))
            expected_pair=tuple(sorted(C.U(c) for c in pair))
        state,events=P.step(state)
        st=state.lattice
        slots=tuple(tuple(map(int,row)) for row in np.argwhere(st.bank))
        if slots!=tuple((site,c) for c in expected_pair):
            raise ValueError("actual pair differs from exact interacting map")
        if events.absorptions or events.crossings or events.gate_holds:
            raise ValueError("unexpected background exchange")
        expected_events=int(phase==1 and C.polarity(pair[0])==C.polarity(pair[1]))
        if len(events.collisions)!=expected_events:
            raise ValueError("missing or excess collision event")
        if expected_events:
            offset=192 if pair[0]>=192 else 0
            expected=(site,C.polarity(pair[0]),tuple(c-offset for c in pair),tuple(c-offset for c in expected_pair))
            if events.collisions[0]!=expected:
                raise ValueError("collision event differs from finite map")
        if (np.any(st.s) or np.any(st.ell!=layer) or np.any(st.sc!=background)
                or np.any(st.fcc!=background) or np.any(state.admitted_sc)
                or np.any(state.gate_sc!=bool(state.phase)) or np.any(state.gate_fcc!=bool(state.phase))
                or state.microtick!=initial.microtick+elapsed or P.work_units(state)!=work):
            raise ValueError("complete prepared pair background/accounting mismatch")
        vertices+=expected_events;pair=expected_pair;positions.append(position)
    return ColocatedAudit(initial.microtick,state.microtick,tuple(positions),pair,vertices,work)
