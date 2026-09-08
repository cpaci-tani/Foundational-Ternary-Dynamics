"""Exact, passive block observations of the selected Phi-v2 lattice.

No macro integrator or physical identification is supplied. Incidence Q is
kept separately from the many-to-one ternary readout; token counts are not
identified with physical energy or mass. Periodic boxes are a test fixture.
"""
from __future__ import annotations

from dataclasses import dataclass
from numbers import Integral

from . import conservation as K, geometry as G, state as S, tick as T


def _shape(L: int, width: int) -> int:
    if (not isinstance(L, Integral) or isinstance(L, bool) or L <= 0
            or not isinstance(width, Integral) or isinstance(width, bool)
            or width <= 0 or L % width):
        raise ValueError("positive integer block width must divide positive lattice size")
    return int(L // width)


def _block(L: int, width: int, site: int) -> int:
    x, y, z = G.coords(L, site)
    return G.site_index(L // width, x // width, y // width, z // width)


@dataclass(frozen=True)
class BlockReadout:
    """Exact extensive counts; density conversion belongs to the observer."""

    lattice_size: int
    width: int
    incidence: tuple[int, ...]
    manifestation_counts: tuple[tuple[int, int, int], ...]  # -1, 0, +1
    field_tokens: tuple[int, ...]
    relation_tokens: tuple[int, ...]  # stored relation-owner convention


def restrict(st: S.LatticeState, width: int) -> BlockReadout:
    """Aggregate a complete state without mutating it or discarding its owner."""
    side = _shape(st.L, width)
    size = side ** 3
    incidence = [0] * size
    manifestations = [[0, 0, 0] for _ in range(size)]
    field = [0] * size
    relations = [0] * size
    q = T._incidence(st, st.sc, st.fcc)
    for i in range(S.n_sites(st)):
        b = _block(st.L, width, i)
        incidence[b] += int(q[i])
        manifestations[b][int(st.s[i]) + 1] += 1
        field[b] += int(st.bank[i].sum())
        relations[b] += int((st.sc[i] != S.BLANK_IDX).sum())
        relations[b] += int((st.fcc[i] != S.BLANK_IDX).sum())
    return BlockReadout(st.L, int(width), tuple(incidence),
                        tuple(tuple(row) for row in manifestations),
                        tuple(field), tuple(relations))


def merge(readout: BlockReadout, factor: int) -> BlockReadout:
    """Nested restriction of extensive counts; no underlying state required."""
    side = _shape(readout.lattice_size, readout.width)
    target_side = _shape(side, factor)
    size = target_side ** 3
    q, field, relations = ([0] * size for _ in range(3))
    manifests = [[0, 0, 0] for _ in range(size)]
    for i in range(side ** 3):
        b = _block(side, factor, i)
        q[b] += readout.incidence[i]
        field[b] += readout.field_tokens[i]
        relations[b] += readout.relation_tokens[i]
        for k in range(3):
            manifests[b][k] += readout.manifestation_counts[i][k]
    return BlockReadout(readout.lattice_size, readout.width * int(factor),
                        tuple(q), tuple(tuple(row) for row in manifests),
                        tuple(field), tuple(relations))


def boundary_current(before: S.LatticeState, events: T.TickEvents,
                     width: int) -> dict[tuple[int, int], int]:
    """Signed current from crossing events, aggregated onto block pairs.

    Positive values flow from lower to higher block index. Internal relations
    cancel exactly. Diagonal/periodic links retain graph incidence: no invented
    face-area, metric, or physical charge normalization is introduced.
    """
    _shape(before.L, width)
    jsc, jfcc = K.current_from_events(before, events)
    result: dict[tuple[int, int], int] = {}

    def add(endpoints, value):
        tail, head = (_block(before.L, width, i) for i in endpoints)
        if tail == head or value == 0:
            return
        key = (min(tail, head), max(tail, head))
        result[key] = result.get(key, 0) + (int(value) if tail < head else -int(value))

    for i in range(S.n_sites(before)):
        for a in range(3):
            add(G.sc_endpoints(before.L, i, a), jsc[i, a])
        for p in range(3):
            for d in range(2):
                add(G.fcc_endpoints(before.L, i, p, d), jfcc[i, p, d])
    return {key: value for key, value in result.items() if value}


def continuity_residual(before: S.LatticeState, after: S.LatticeState,
                        events: T.TickEvents, width: int) -> tuple[int, ...]:
    """Per-block delta(Q) + outward current; zero is exact integer balance.

    State incidence and crossing-log current are independent bookkeeping routes.
    This certifies the supplied step's balance, not admissibility of the law.
    """
    if before.L != after.L:
        raise ValueError("continuity requires matching lattice sizes")
    old, new = restrict(before, width), restrict(after, width)
    residual = [b - a for a, b in zip(old.incidence, new.incidence)]
    for (tail, head), current in boundary_current(before, events, width).items():
        residual[tail] += current
        residual[head] -= current
    return tuple(residual)
