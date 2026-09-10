"""Exact finite-horizon observations with unknown exterior, not a boundary law.

The periodic completion is scratch computation. Only the eroding Moore interior
is certified. No exterior history is manufactured and no halo can be refilled.
See engine/docs/CONTRACT_STRICT_FINITE_WINDOW_V1.md for the support theorem.
"""
from dataclasses import dataclass, replace
import base64
import hashlib
from . import exact_json as J

import numpy as np

from . import channels as C, checkpoint as K, checkpoint_integrity as I
from . import staged as P

SCHEMA = "ftd-staged-causal-window-1"
BOUNDARY = "unknown_exterior_finite_causal_window"
NAMES = ("s", "ell", "bank", "sc", "fcc", "admitted_sc", "gate_sc", "gate_fcc")


def _coordinate(value, name):
    if (type(value) is not tuple or len(value) != 3
            or any(type(component) is not int for component in value)):
        raise ValueError(f"{name} must contain three concrete Python integers")
    return value


def _slices(origin, extent, lower, upper):
    _coordinate(lower, "lower corner")
    _coordinate(upper, "upper corner")
    if any(not o <= lo < hi <= o + n for o, n, lo, hi in zip(origin, extent, lower, upper)):
        raise ValueError("observation must lie inside the nonempty certified bounds")
    return tuple(slice(lo - o, hi - o) for o, lo, hi in zip(origin, lower, upper))


@dataclass(frozen=True)
class RecordArray:
    name: str
    dtype: str
    shape: tuple
    data: bytes

    def array(self):
        """An ndarray backed by immutable bytes; callers may explicitly copy it."""
        return np.frombuffer(self.data, dtype=self.dtype).reshape(self.shape)


@dataclass(frozen=True)
class Observation:
    origin: tuple
    extent: tuple
    microtick: int
    records: tuple
    state_id: str
    generation: int
    law_id: str = P.LAW_ID
    collision_hash: str = C.COLLISION_HASH
    encoding_hash: str = K.ENCODING_HASH
    boundary: str = BOUNDARY
    status: str = "exact_observation"
    spatial_support: str = "oriented_record_anchors"
    length_unit: str = "microscopic_node"
    time_unit: str = "staged_microtick"
    physical_calibration: str = "unidentified"

    @property
    def phase(self):
        return self.microtick % 4

    @property
    def tick_interval(self):
        return self.microtick, self.microtick

    def record(self, name):
        for record in self.records:
            if record.name == name:
                return record.array()
        raise ValueError(f"unknown record: {name}")

    def restrict(self, lower, upper):
        selection = _slices(self.origin, self.extent, lower, upper)
        records = tuple(_record(record.name, record.array()[selection]) for record in self.records)
        return replace(self, origin=lower, extent=tuple(hi - lo for lo, hi in zip(lower, upper)),
                       records=records)


def _record(name, array):
    return RecordArray(name, array.dtype.str, array.shape, array.tobytes(order="C"))


class CausalWindow:
    """Immutable computation and its remaining finite domain of dependence."""
    __slots__ = ("_snapshot", "_origin", "_initial_tick", "_tick", "_side")

    def __init_subclass__(cls, **kwargs):
        raise TypeError("causal window subclasses are not a certified state domain")

    def __init__(self, state, origin=(0, 0, 0)):
        _coordinate(origin, "origin")
        snapshot = P.checkpoint(state)
        self._set(snapshot, origin, int(state.microtick), int(state.microtick), int(state.lattice.L))

    def __setattr__(self, name, value):
        raise AttributeError("causal windows are immutable; advance returns a new window")

    def __delattr__(self, name):
        raise AttributeError("causal window records cannot be deleted")

    def _set(self, snapshot, origin, initial_tick, tick, side):
        if hasattr(self, "_snapshot"):
            raise AttributeError("causal window initialization is one-shot")
        for name, value in zip(self.__slots__, (snapshot, origin, initial_tick, tick, side)):
            object.__setattr__(self, name, value)

    @classmethod
    def _from_snapshot(cls, snapshot, origin, initial_tick):
        if cls is not CausalWindow:
            raise ValueError("expected concrete causal window class")
        _coordinate(origin, "origin")
        if type(initial_tick) is not int or initial_tick < 0:
            raise ValueError("invalid initial microtick")
        state = P.restore(snapshot)
        elapsed = state.microtick - initial_tick
        if elapsed < 0 or 2 * elapsed >= state.lattice.L:
            raise ValueError("checkpoint has incoherent or exhausted causal bounds")
        result = object.__new__(cls)
        result._set(snapshot, origin, initial_tick, state.microtick, state.lattice.L)
        return result

    @property
    def microtick(self):
        return self._tick

    @property
    def remaining_microticks(self):
        return (self._side - 1) // 2 - (self._tick - self._initial_tick)

    @property
    def bounds(self):
        consumed = self._tick - self._initial_tick
        return (tuple(o + consumed for o in self._origin),
                tuple(o + self._side - consumed for o in self._origin))

    def advance(self, microticks=1):
        microticks = P._plain_integer(microticks, "microticks")
        if microticks > self.remaining_microticks:
            raise ValueError("insufficient retained causal halo; exterior is unknown")
        if microticks == 0:
            return self
        state = P.restore(self._snapshot)
        for _ in range(microticks):
            state, _ = P.step(state)
        return self._from_snapshot(P.checkpoint(state), self._origin, self._initial_tick)

    def observe(self, lower=None, upper=None):
        bounds = self.bounds
        lower = bounds[0] if lower is None else lower
        upper = bounds[1] if upper is None else upper
        extent = tuple(hi - lo for lo, hi in zip(*bounds))
        _slices(bounds[0], extent, lower, upper)  # reject uncertified anchors first
        selection = _slices(self._origin, (self._side,) * 3, lower, upper)
        state = P.restore(self._snapshot)
        records = []
        for i, name in enumerate(NAMES):
            array = getattr(state.lattice if i < 5 else state, name)
            spatial = array.reshape((self._side,) * 3 + array.shape[1:])
            records.append(_record(name, spatial[selection]))
        return Observation(lower, tuple(hi - lo for lo, hi in zip(lower, upper)),
                           self.microtick, tuple(records),
                           hashlib.sha256(self.checkpoint()).hexdigest(),
                           self._tick - self._initial_tick)

    def checkpoint(self):
        payload = {"schema": SCHEMA, "boundary": BOUNDARY, "origin": self._origin,
                   "initial_tick": self._initial_tick,
                   "snapshot": base64.b64encode(self._snapshot).decode("ascii")}
        return I.pack(J.dumps(payload).encode())

    @classmethod
    def restore(cls, envelope):
        try:
            payload = J.loads(I.unpack(envelope), object_pairs_hook=K._unique_object)
            if type(payload) is not dict or set(payload) != {"schema", "boundary", "origin", "initial_tick", "snapshot"}:
                raise ValueError("causal window checkpoint fields do not match schema")
            if payload["schema"] != SCHEMA or payload["boundary"] != BOUNDARY:
                raise ValueError("incompatible causal window identity")
            if type(payload["origin"]) is not list or type(payload["snapshot"]) is not str:
                raise ValueError("invalid causal window origin or snapshot")
            snapshot = base64.b64decode(payload["snapshot"], validate=True)
            return cls._from_snapshot(snapshot, tuple(payload["origin"]), payload["initial_tick"])
        except (TypeError, KeyError, UnicodeError, OverflowError) as exc:
            raise ValueError("malformed causal window checkpoint") from exc
