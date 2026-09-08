"""Single-owner research facade for the explicitly staged finite candidate.

Observers are external diagnostics, not instantaneous in-universe measuring
devices. No reference-engine fallback or autonomous continuum step is offered.
"""
from __future__ import annotations

from dataclasses import asdict, dataclass, is_dataclass
from functools import wraps
from numbers import Integral
from threading import RLock
from typing import Any
from uuid import uuid4

from . import channels as C, coarse as B, staged as P


def _serialized(method):
    @wraps(method)
    def call(self, *args, **kwargs):
        with self._lock:
            return method(self, *args, **kwargs)
    return call


@dataclass(frozen=True)
class Observation:
    owner_id: str
    law_id: str
    collision_hash: str
    generation: int
    tick_start: int
    tick_end: int
    phase: int
    lattice_size: int
    block_width: int
    observable: str
    payload: Any
    status: str = "exact_observation"
    spatial_support: str = "aligned_periodic_blocks"
    observer_kind: str = "external_diagnostic"
    length_unit: str = "microscopic_node"
    time_unit: str = "staged_microtick"
    physical_calibration: str = "unidentified"

    def to_wire(self) -> dict:
        """Lossless JSON-ready integers, including counters beyond JS 2^53."""
        def encode(value):
            if isinstance(value, bool) or value is None or isinstance(value, str):
                return value
            if isinstance(value, Integral):
                return str(int(value))
            if is_dataclass(value):
                return encode(asdict(value))
            if isinstance(value, dict):
                return {str(k): encode(v) for k, v in value.items()}
            if isinstance(value, (list, tuple)):
                return [encode(v) for v in value]
            raise TypeError(f"unsupported exact observation value: {type(value).__name__}")
        return {"integer_encoding": "decimal_string", **encode(self)}


class StrictRuntime:
    """Own one state; publish immutable observations of its current generation.

    A batch advance commits only after all requested microticks succeed.
    Checkpoint restoration never rewinds the external generation counter.
    This Python reference facade is not a production/backend certificate.
    """

    def __init__(self, lattice):
        self._lock = RLock()
        self._state = P.initialize(lattice)
        self._generation = 0
        # External publication identity only; never enters Phi or checkpoints.
        self._owner_id = uuid4().hex

    @staticmethod
    def capabilities() -> dict:
        return {
            "backend": "python_reference",
            "law_id": P.LAW_ID,
            "canonical_adoption": False,
            "advance": "exact_staged_microticks",
            "checkpoint": "complete_finite_record",
            "observables": ("counts", "resolved"),
            "autonomous_coarse_evolution": False,
            "continuum_trajectory_certificate": False,
            "physical_units": False,
            "particle_identification": False,
            "atomic_molecular_recovery": False,
            "gravity_recovery": False,
            "production_backend_certified": False,
        }

    @_serialized
    def diagnostics(self) -> dict:
        return {
            "law_id": P.LAW_ID,
            "owner_id": self._owner_id,
            "generation": self._generation,
            "microtick": self._state.microtick,
            "phase": self._state.phase,
            "lattice_size": self._state.lattice.L,
            "token_count": P.work_units(self._state),
            "token_interpretation": "finite_record_count_not_physical_energy",
        }

    @_serialized
    def advance(self, microticks: int) -> dict:
        if (not isinstance(microticks, Integral) or isinstance(microticks, bool)
                or microticks < 0):
            raise ValueError("microticks must be a nonnegative integer")
        candidate = self._state
        for _ in range(int(microticks)):
            candidate, _ = P.step(candidate)
        if microticks:
            self._state = candidate
            self._generation += 1
        return self.diagnostics()

    @_serialized
    def checkpoint(self) -> bytes:
        return P.checkpoint(self._state)

    @_serialized
    def restore(self, checkpoint: bytes) -> dict:
        candidate = P.restore(checkpoint)
        self._state = candidate
        self._generation += 1
        return self.diagnostics()

    @_serialized
    def observe(self, resolution: int = 1, observable: str = "counts") -> Observation:
        if observable == "counts":
            payload = B.restrict(self._state.lattice, resolution)
        elif observable == "resolved":
            from . import resolved
            payload = resolved.restrict(self._state, resolution)
        else:
            raise ValueError(f"unsupported strict observable: {observable}")
        return Observation(
            owner_id=self._owner_id, law_id=P.LAW_ID, collision_hash=C.COLLISION_HASH,
            generation=self._generation, tick_start=self._state.microtick,
            tick_end=self._state.microtick, phase=self._state.phase,
            lattice_size=self._state.lattice.L, block_width=int(resolution),
            observable=observable, payload=payload,
        )
