"""Exact finite counting ensembles of complete candidate states.

Multiplicities are a selected external counting measure, not primitive chance
or simultaneous particles in one universe. No marginal reconstruction is used.
Point observables depend on the actual periodic complete state. Merged states
do not retain unwrapped winding or separate trajectory histories; those require
externally retained trajectory labels, not extra ontic ensemble records.
"""
from __future__ import annotations

from dataclasses import dataclass, field
from fractions import Fraction
from numbers import Integral
from typing import Callable, Iterable

from . import staged as P


def _integer(value, name, minimum=0):
    if isinstance(value, bool) or not isinstance(value, Integral) or value < minimum:
        raise ValueError(f"{name} must be an integer >= {minimum}")
    return int(value)


@dataclass(frozen=True)
class CountedCheckpoint:
    checkpoint: bytes
    multiplicity: int


@dataclass(frozen=True)
class FiniteEnsemble:
    """Lossless multiplicity compression of a finite list of complete states.

    All members share lattice size and physical clock. States that coincide
    after named expiry merge, retaining their complete counting multiplicity.
    Resource limits reject work; they never remove low-weight branches.
    """

    atoms: tuple[CountedCheckpoint, ...]
    max_support: int = 256
    microtick: int = field(init=False)
    lattice_size: int = field(init=False)
    total_count: int = field(init=False)

    def __post_init__(self):
        limit = _integer(self.max_support, "max_support", 1)
        merged = {}
        frame = None
        for atom in self.atoms:
            if not isinstance(atom, CountedCheckpoint) or not isinstance(atom.checkpoint, bytes):
                raise ValueError("ensemble atoms require immutable complete checkpoint bytes")
            count = _integer(atom.multiplicity, "multiplicity", 1)
            state = P.restore(atom.checkpoint)
            current_frame = (state.microtick, state.lattice.L)
            if frame is not None and frame != current_frame:
                raise ValueError("ensemble members must share physical clock and lattice")
            frame = current_frame
            canonical = P.checkpoint(state)
            merged[canonical] = merged.get(canonical, 0) + count
            if len(merged) > limit:
                raise ValueError("exact support exceeds resource limit; no branches discarded")
        if frame is None:
            raise ValueError("empty counting ensemble")
        atoms = tuple(CountedCheckpoint(key, merged[key]) for key in sorted(merged))
        object.__setattr__(self, "atoms", atoms)
        object.__setattr__(self, "max_support", limit)
        object.__setattr__(self, "microtick", frame[0])
        object.__setattr__(self, "lattice_size", frame[1])
        object.__setattr__(self, "total_count", sum(merged.values()))

    @classmethod
    def from_states(cls, members: Iterable[tuple[P.StagedState, int]], *, max_support=256):
        limit = _integer(max_support, "max_support", 1)
        counted = {}
        for state, count in members:
            count = _integer(count, "multiplicity", 1)
            key = P.checkpoint(state)
            counted[key] = counted.get(key, 0) + count
            if len(counted) > limit:
                raise ValueError("exact support exceeds resource limit; no branches discarded")
        return cls(tuple(CountedCheckpoint(key, count) for key, count in counted.items()), limit)

    def advance(self, microticks: int, *, max_updates: int = 4096):
        ticks = _integer(microticks, "microticks")
        budget = _integer(max_updates, "max_updates")
        if ticks * len(self.atoms) > budget:
            raise ValueError("complete-state update budget exceeded; no projected substitute")
        if ticks == 0:
            return self
        results = []
        for atom in self.atoms:
            state = P.restore(atom.checkpoint)
            for _ in range(ticks):
                state, _events = P.step(state)
            results.append(CountedCheckpoint(P.checkpoint(state), atom.multiplicity))
        return FiniteEnsemble(tuple(results), self.max_support)

    def expectation(self, observable: Callable[[P.StagedState], int | Fraction]) -> Fraction:
        """Evaluate an exact observable on a fresh private copy of every member."""
        total = Fraction(0)
        for atom in self.atoms:
            value = observable(P.restore(atom.checkpoint))
            if isinstance(value, Integral):
                value = Fraction(int(value))
            if not isinstance(value, Fraction):
                raise TypeError("an exact observable must return an integer or Fraction")
            total += atom.multiplicity * value
        return total / self.total_count

    def covariance(self, left, right) -> Fraction:
        # Separate private states also prevent one observer mutating the other's
        # input. Their functions must represent the stated observable, not RNG.
        joint = Fraction(0)
        for atom in self.atoms:
            singleton = FiniteEnsemble((CountedCheckpoint(atom.checkpoint, 1),))
            joint += atom.multiplicity * singleton.expectation(left) * singleton.expectation(right)
        return joint / self.total_count - self.expectation(left) * self.expectation(right)

    def provenance(self):
        return {
            "law_id": P.LAW_ID,
            "evolution_kind": "full_finite_counting_ensemble",
            "measure": "selected_external_integer_multiplicities",
            "microtick": str(self.microtick),
            "time_unit": "staged_microtick",
            "lattice_size": str(self.lattice_size),
            "support_size": str(len(self.atoms)),
            "total_count": str(self.total_count),
            "status": "exact_complete_state_pushforward",
            "marginal_reprojection": False,
            "physical_probability_identification": False,
        }
