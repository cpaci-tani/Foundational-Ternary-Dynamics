"""Exact resolved observations, not an autonomous coarse evolution law.

Block indices use geometry.site_index with origin (0, 0, 0). Relations are
counted at their stored owner (including the shifted-tail FCC diagonal).
Polarity order is (+1, -1); relation slots are (primary, reserve); phases
are 0..3. Field channels retain the canonical 192-state order. Moment values
use channels.layer_value_of exactly, without an added polarity multiplier.
All published storage is immutable Python tuples, independent of live arrays.
"""
from __future__ import annotations

from dataclasses import dataclass
from numbers import Integral

import numpy as np

from . import channels as C, coarse as B, staged as P, state as S, tick as T
from ._proofs import phase_index, readout, rotate


RELATION_ORIENTATIONS = (
    ("sc", 0), ("sc", 1), ("sc", 2),
    ("fcc", 0, 0), ("fcc", 0, 1),
    ("fcc", 1, 0), ("fcc", 1, 1),
    ("fcc", 2, 0), ("fcc", 2, 1),
)


@dataclass(frozen=True)
class IntTensor:
    """Row-major exact integers; tuple backing cannot be made writable."""

    shape: tuple[int, ...]
    values: tuple[int, ...]

    @classmethod
    def freeze(cls, array) -> IntTensor:
        allowed = (bool, np.bool_, *P._INTEGER_TYPES)

        def validate_sequence(value):
            if type(value) is list or type(value) is tuple:
                for item in value:
                    validate_sequence(item)
            elif not any(type(value) is kind for kind in allowed):
                raise ValueError("exact tensor leaves must be concrete integers or booleans")

        if type(array) is np.ndarray:
            arr = array
        elif type(array) is list or type(array) is tuple:
            validate_sequence(array)  # Reject custom conversions before NumPy sees a leaf.
            arr = np.asarray(array, dtype=object)  # Preserve mixed signed/large integers.
        else:
            raise ValueError("exact tensor requires a concrete array, list or tuple")
        if any(not any(type(value) is kind for kind in allowed) for value in arr.flat):
            raise ValueError("exact tensor leaves must be concrete integers or booleans")
        return cls(tuple(int(n) for n in arr.shape),
                   tuple(int(v) for v in arr.flat))

    def array(self) -> np.ndarray:
        """Return a detached working copy, never the observation's storage."""
        return np.array(self.values, dtype=object).reshape(self.shape)


@dataclass(frozen=True)
class PendingRecord:
    name: str
    ownership: str
    data: IntTensor


@dataclass(frozen=True)
class ResolvedReadout:
    counts: B.BlockReadout
    site_layers: IntTensor             # (block, layer)
    field_channels: IntTensor          # (block, layer, polarity, state192)
    relation_channels: IntTensor       # (block, orientation9, slot, polarity, phase)
    blank_relation_slots: IntTensor    # (block, orientation9, slot)
    field_moments: IntTensor           # (block, six canonical moments)
    microtick: int | None = None
    phase: int | None = None
    pending: tuple[PendingRecord, ...] = ()
    status: str = "exact observation; autonomous closure unestablished"


def _active(st):
    if type(st) is S.LatticeState:
        return st  # Reference diagnostics also support the legacy L2 fixtures.
    if type(st) is P.StagedState:
        P.validate(st)
        return st.lattice
    raise ValueError("observations require a concrete reference or staged state")


def restrict(st, width: int) -> ResolvedReadout:
    """Resolve a reference lattice or a staged state's current active records.

    Pending control bits are retained at original microscopic owner coordinates,
    even in a merged view. They are not additional carriers. At intermediate
    microticks the manifestation is the stored readout, not a recomputed value.
    """
    lattice = _active(st)
    side = B._shape(lattice.L, width)
    size = side ** 3
    layers = np.zeros((size, 3), dtype=object)
    field = np.zeros((size, 3, 2, C.N_STATES), dtype=object)
    relation = np.zeros((size, 9, 2, 2, 4), dtype=object)
    blanks = np.zeros((size, 9, 2), dtype=object)
    moments = np.zeros((size, 6), dtype=object)
    for site in range(S.n_sites(lattice)):
        block = B._block(lattice.L, width, site)
        layer = int(lattice.ell[site])
        layers[block, layer] += 1
        for channel in np.flatnonzero(lattice.bank[site]).tolist():
            state, polarity = C.unpack(channel)
            field[block, layer, int(polarity < 0), state] += 1
            moments[block] += np.asarray(C.layer_value_of(channel, layer), dtype=object)
        for orientation, key in enumerate(RELATION_ORIENTATIONS):
            slots = getattr(lattice, key[0])[(site,) + key[1:]]
            for slot, token in enumerate(slots):
                if token == S.BLANK_IDX:
                    blanks[block, orientation, slot] += 1
                else:
                    value = S.z_of(token)
                    polarity = int(readout(value)[1])
                    relation[block, orientation, slot, int(polarity < 0),
                             phase_index(value)] += 1
    pending = ()
    microtick = phase = None
    if type(st) is P.StagedState:
        microtick, phase = int(st.microtick), int(st.phase)
        pending = tuple(PendingRecord(name, owner, IntTensor.freeze(getattr(st, name)))
                        for name, owner in (
                            ("admitted_sc", "microscopic SC owner, axis"),
                            ("gate_sc", "microscopic SC owner, axis"),
                            ("gate_fcc", "microscopic FCC owner, plane, diagonal")))
    return ResolvedReadout(B.restrict(lattice, width),
                           *(IntTensor.freeze(a) for a in
                             (layers, field, relation, blanks, moments)),
                           microtick, phase, pending)


def _merge_tensor(tensor: IntTensor, side: int, factor: int) -> IntTensor:
    target_side = B._shape(side, factor)
    output = np.zeros((target_side ** 3,) + tensor.shape[1:], dtype=object)
    source = tensor.array()
    for site in range(side ** 3):
        output[B._block(side, factor, site)] += source[site]
    return IntTensor.freeze(output)


def merge(observation: ResolvedReadout, factor: int) -> ResolvedReadout:
    """Exact extensive restriction; pending bits retain original owner indices."""
    side = B._shape(observation.counts.lattice_size, observation.counts.width)
    B._shape(side, factor)
    return ResolvedReadout(B.merge(observation.counts, factor),
                           *(_merge_tensor(t, side, factor) for t in (
                               observation.site_layers, observation.field_channels,
                               observation.relation_channels, observation.blank_relation_slots,
                               observation.field_moments)),
                           observation.microtick, observation.phase, observation.pending)


@dataclass(frozen=True)
class BoundaryTransfers:
    """Exact oriented graph currents over [start_tick, stop_tick).

    Edges are sorted (lower block, higher block, signed transfer). Endpoint
    incidence is kept to check telescoping independently of current totals.
    Ticks refer to one caller-declared reference law or staged microtick clock;
    mixing reference cycles with staged microticks is rejected.
    """

    lattice_size: int
    width: int
    start_tick: int
    stop_tick: int
    clock: str
    edges: tuple[tuple[int, int, int], ...]
    incidence_before: tuple[int, ...]
    incidence_after: tuple[int, ...]

    def residual(self) -> tuple[int, ...]:
        result = [b - a for a, b in zip(self.incidence_before, self.incidence_after)]
        for tail, head, transfer in self.edges:
            result[tail] += transfer
            result[head] -= transfer
        return tuple(result)


def boundary_transfers(before, after, events: T.TickEvents, width: int,
                       *, start_tick: int | None = None) -> BoundaryTransfers:
    """Read one reference cycle or one staged microtick's crossing log.

    Malformed, duplicate or slot-incompatible events are rejected. Missing
    events remain visible when they cause a net block-incidence discrepancy;
    a zero residual does not prove log completeness (internal/cancelling
    omissions can be invisible). Logs are never repaired from the after-state.
    """
    initial, final = _active(before), _active(after)
    staged = type(before) is P.StagedState
    if staged != (type(after) is P.StagedState):
        raise ValueError("cannot mix reference cycles and staged microticks")
    if staged:
        if start_tick is not None and start_tick != before.microtick:
            raise ValueError("start_tick must match staged state")
        start_tick = before.microtick
        if after.microtick != start_tick + 1:
            raise ValueError("crossing log must describe exactly one microtick")
    if not isinstance(start_tick, Integral) or isinstance(start_tick, bool) or start_tick < 0:
        raise ValueError("a nonnegative integer start_tick is required")
    if initial.L != final.L:
        raise ValueError("transfer endpoints require matching lattice sizes")
    _validate_crossings(initial, final, events)
    current = B.boundary_current(initial, events, width)
    return BoundaryTransfers(initial.L, int(width), int(start_tick), int(start_tick) + 1,
                             "staged_microtick" if staged else "reference_cycle",
                             tuple((a, b, v) for (a, b), v in sorted(current.items())),
                             B.restrict(initial, width).incidence,
                             B.restrict(final, width).incidence)


def _validate_crossings(before, after, events):
    """Validate crossing records before the legacy overwrite-based reducer."""
    seen = set()

    def finite(value, limit):
        return (isinstance(value, Integral) and not isinstance(value, bool)
                and 0 <= value < limit)

    if not hasattr(events, "crossings") or not isinstance(events.crossings, (list, tuple)):
        raise ValueError("crossings must be an explicit event sequence")
    for row in events.crossings:
        if not isinstance(row, (list, tuple)) or len(row) != 4:
            raise ValueError("malformed crossing event")
        kind, owner, indices, direction = row
        if (kind not in ("sc", "fcc") or not finite(owner, S.n_sites(before))
                or not isinstance(indices, (list, tuple))
                or len(indices) != (1 if kind == "sc" else 2)
                or not finite(indices[0], 3)
                or (kind == "fcc" and not finite(indices[1], 2))
                or isinstance(direction, bool) or not isinstance(direction, Integral)
                or direction not in (-1, 1)):
            raise ValueError("crossing fields outside finite relation alphabet")
        key = kind, int(owner), tuple(int(i) for i in indices)
        if key in seen:
            raise ValueError("duplicate crossing relation")
        seen.add(key)
        index = (owner,) + tuple(indices)
        old, new = getattr(before, kind)[index], getattr(after, kind)[index]
        source = 0 if direction == 1 else 1
        target = 1 - source
        if old[source] == S.BLANK_IDX or old[target] != S.BLANK_IDX:
            raise ValueError("crossing direction requires a unique occupied source slot")
        value = S.z_of(old[source])
        if (phase_index(value) != 0 or new[source] != S.BLANK_IDX
                or new[target] != S.idx_of(rotate(value))):
            raise ValueError("crossing event incompatible with source phase or final slots")


def compose_transfers(*windows: BoundaryTransfers) -> BoundaryTransfers:
    """Add contiguous time windows, checking partition, clock and endpoint Q."""
    if not windows:
        raise ValueError("at least one transfer window is required")
    first = previous = windows[0]
    edges = {}
    for number, window in enumerate(windows):
        if (window.lattice_size, window.width, window.clock) != (
                first.lattice_size, first.width, first.clock):
            raise ValueError("transfer windows require identical partitions and clocks")
        if number and (previous.stop_tick != window.start_tick
                       or previous.incidence_after != window.incidence_before):
            raise ValueError("transfer windows must be contiguous with matching endpoint incidence")
        for tail, head, value in window.edges:
            edges[tail, head] = edges.get((tail, head), 0) + value
        previous = window
    return BoundaryTransfers(first.lattice_size, first.width, first.start_tick,
                             previous.stop_tick, first.clock,
                             tuple((a, b, v) for (a, b), v in sorted(edges.items()) if v),
                             first.incidence_before, previous.incidence_after)


def merge_transfers(window: BoundaryTransfers, factor: int) -> BoundaryTransfers:
    """Spatially restrict the boundary graph, cancelling new internal edges."""
    side = B._shape(window.lattice_size, window.width)
    target = B._shape(side, factor)
    initial, final = [0] * target ** 3, [0] * target ** 3
    for site in range(side ** 3):
        block = B._block(side, factor, site)
        initial[block] += window.incidence_before[site]
        final[block] += window.incidence_after[site]
    edges = {}
    for tail, head, value in window.edges:
        a, b = B._block(side, factor, tail), B._block(side, factor, head)
        if a != b:
            key = min(a, b), max(a, b)
            edges[key] = edges.get(key, 0) + (value if a < b else -value)
    return BoundaryTransfers(window.lattice_size, window.width * int(factor),
                             window.start_tick, window.stop_tick, window.clock,
                             tuple((a, b, v) for (a, b), v in sorted(edges.items()) if v),
                             tuple(initial), tuple(final))
