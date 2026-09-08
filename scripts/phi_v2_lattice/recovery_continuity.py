"""Exact stored-inventory currents; external weak-continuum comparison only.

The counted measure is not physical mass, energy, charge or manifestation.
An evidence record validates balance, not authentication of a supplied history.
Use advance_observed to obtain currents from the actual complete law.
"""
from __future__ import annotations

from collections import Counter
from dataclasses import dataclass
from fractions import Fraction
from numbers import Integral

import numpy as np

from . import channels as C, geometry as G, staged as P, state as S
from ._proofs import readout


def _integer(value, name, minimum=0):
    if isinstance(value, bool) or not isinstance(value, Integral) or value < minimum:
        raise ValueError(f"invalid {name}")
    return int(value)


def _exact(value, name, *, minimum=None):
    if isinstance(value, bool) or not isinstance(value, (Integral, Fraction)):
        raise ValueError(f"{name} requires exact rational input")
    value = Fraction(value)
    if minimum is not None and value < minimum:
        raise ValueError(f"invalid {name}")
    return value


def owner_inventory(state):
    P.validate(state)
    st = state.lattice
    signs = np.array([0 if i == S.BLANK_IDX else readout(S.z_of(i))[1]
                      for i in range(9)])
    columns = []
    for polarity, part in ((1, slice(0, 192)), (-1, slice(192, 384))):
        counts = st.bank[:, part].sum(axis=1, dtype=np.int64)
        counts += (signs[st.sc] == polarity).sum(axis=(1, 2))
        counts += (signs[st.fcc] == polarity).sum(axis=(1, 2, 3))
        columns.append(counts)
    return tuple((int(a), int(b)) for a, b in zip(*columns))


@dataclass(frozen=True)
class Transfer:
    source: int
    direction: tuple[int, int, int]
    polarity: int
    count: int
    kind: str


@dataclass(frozen=True)
class ContinuityStep:
    L: int
    start_tick: int
    before: tuple[tuple[int, int], ...]
    after: tuple[tuple[int, int], ...]
    transfers: tuple[Transfer, ...]

    def __post_init__(self):
        L = _integer(self.L, "L", 3)
        tick = _integer(self.start_tick, "start_tick")
        for rows in (self.before, self.after):
            if not isinstance(rows, tuple) or len(rows) != L**3:
                raise ValueError("inventory must be an immutable complete owner array")
            for row in rows:
                if not isinstance(row, tuple) or len(row) != 2:
                    raise ValueError("inventory requires both polarities")
                for value in row:
                    _integer(value, "inventory count")
        if not isinstance(self.transfers, tuple):
            raise ValueError("transfers must be immutable")
        object.__setattr__(self, "L", L)
        object.__setattr__(self, "start_tick", tick)
        object.__setattr__(self, "before", tuple(tuple(map(int, row)) for row in self.before))
        object.__setattr__(self, "after", tuple(tuple(map(int, row)) for row in self.after))
        balance = [[b-a for a, b in zip(old, new)]
                   for old, new in zip(self.before, self.after)]
        outgoing = Counter()
        keys = set()
        canonical = []
        for edge in self.transfers:
            if not isinstance(edge, Transfer):
                raise ValueError("expected finite Transfer")
            source = _integer(edge.source, "source")
            if source >= L**3 or not isinstance(edge.direction, tuple) or edge.direction not in (
                    (1, 0, 0), (-1, 0, 0), (0, 1, 0), (0, -1, 0), (0, 0, 1), (0, 0, -1)):
                raise ValueError("transfer requires one axial hop")
            for component in edge.direction:
                if isinstance(component, bool) or not isinstance(component, Integral):
                    raise ValueError("direction must have canonical integer components")
            if isinstance(edge.polarity, bool) or not isinstance(edge.polarity, Integral) or edge.polarity not in (1, -1):
                raise ValueError("invalid polarity")
            count = _integer(edge.count, "transfer count", 1)
            if ((edge.kind == "stream" and tick % 4 != 2)
                    or (edge.kind == "admission" and (tick % 4 != 0 or 1 in edge.direction))
                    or edge.kind not in ("stream", "admission")):
                raise ValueError("transfer incompatible with physical stage")
            key = (source, edge.direction, edge.polarity, edge.kind)
            if key in keys:
                raise ValueError("duplicate transfer key")
            keys.add(key)
            canonical.append(Transfer(source, tuple(map(int, edge.direction)), int(edge.polarity), count, edge.kind))
            p = int(edge.polarity < 0)
            outgoing[source, p] += count
            balance[source][p] += count
            balance[G.shift(L, source, edge.direction)][p] -= count
        if any(count > self.before[source][p] for (source, p), count in outgoing.items()):
            raise ValueError("outgoing inventory exceeds source count")
        if any(any(row) for row in balance):
            raise ValueError("nonzero local continuity residual")
        object.__setattr__(self, "transfers", tuple(canonical))

    @property
    def hop_count(self):
        return sum(edge.count for edge in self.transfers)

    def weak_balance(self, test_values, polarity=1):
        if isinstance(polarity, bool) or not isinstance(polarity, Integral) or polarity not in (1, -1):
            raise ValueError("invalid polarity")
        values = tuple(_exact(v, "test value") for v in test_values)
        if len(values) != self.L**3:
            raise ValueError("one test value per owner required")
        p = int(polarity < 0)
        density = sum((b[p]-a[p])*f for a, b, f in zip(self.before, self.after, values))
        current = sum(edge.count*(values[G.shift(self.L, edge.source, edge.direction)]-values[edge.source])
                      for edge in self.transfers if edge.polarity == polarity)
        return density, current


def advance_observed(state):
    """Execute one actual microtick, retaining the full resulting state."""
    before = owner_inventory(state)
    after, events = P.step(state)
    transfers = Counter()
    if state.phase == 0:
        for source, channel, owner, _axis in events.absorptions:
            if owner != source:
                direction = C.tangent(channel)
                if G.shift(state.lattice.L, source, direction) != owner:
                    raise ValueError("admission owner is not the declared adjacent anchor")
                transfers[source, direction, C.polarity(channel), "admission"] += 1
    elif state.phase == 2:
        for source, channel in zip(*np.nonzero(state.lattice.bank)):
            transfers[int(source), C.tangent(int(channel)), C.polarity(int(channel)), "stream"] += 1
    edges = tuple(Transfer(source, d, p, count, kind)
                  for (source, d, p, kind), count in sorted(transfers.items()))
    return after, ContinuityStep(state.lattice.L, state.microtick, before, owner_inventory(after), edges)


def weak_current_error_bound(hops, spacing, curvature_bound, normalizer=1):
    """Conditional Taylor bound for periodic C2 test functions on axial edges.

    The caller must prove the stated curvature bound. This function neither
    checks derivatives nor certifies convergence of any particular sequence.
    """
    hops = _integer(hops, "hop count")
    spacing = _exact(spacing, "spacing", minimum=0)
    curvature = _exact(curvature_bound, "curvature bound", minimum=0)
    normalizer = _exact(normalizer, "normalizer", minimum=0)
    if spacing == 0 or normalizer == 0:
        raise ValueError("spacing and normalizer must be positive")
    return hops * spacing**2 * curvature / (2 * normalizer)
