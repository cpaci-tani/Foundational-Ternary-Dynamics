"""Exact material-recovery obstructions of the current staged candidate.

These passive integer inventories do not identify tokens with mass, energy,
particles or charge. Structural proofs live in AUDIT_STRICT_MATERIAL_MACRO_RECOVERY.
Tests on transitions are witnesses and regression guards, not a substitute for
the per-stage argument. No successor dynamics is supplied here.
"""
from __future__ import annotations

from dataclasses import dataclass

import numpy as np

from . import channels as C, state as S, staged as V
from ._proofs import readout


@dataclass(frozen=True)
class AnchorInventory:
    lattice_size: int
    field_by_polarity: tuple[int, int]  # +1, -1
    sc_by_polarity: tuple[tuple[int, int], ...]   # owner*3 + axis
    fcc_by_polarity: tuple[tuple[int, int], ...]  # (owner*3 + plane)*2 + diagonal

    @property
    def field_count(self) -> int:
        return sum(self.field_by_polarity)

    @property
    def relation_count(self) -> int:
        return sum(sum(pair) for pair in self.sc_by_polarity + self.fcc_by_polarity)


def inventory(state: V.StagedState) -> AnchorInventory:
    """Count both relation slots at their stored anchor; retain polarity."""
    V.validate(state)
    lattice = state.lattice

    def relation_rows(array):
        result = []
        for pair in array.reshape(-1, 2):
            counts = [0, 0]
            for token in pair:
                if token != S.BLANK_IDX:
                    counts[int(readout(S.z_of(token))[1] < 0)] += 1
            result.append(tuple(counts))
        return tuple(result)

    return AnchorInventory(lattice.L,
                           (int(lattice.bank[:, :C.N_STATES].sum()),
                            int(lattice.bank[:, C.N_STATES:].sum())),
                           relation_rows(lattice.sc), relation_rows(lattice.fcc))


@dataclass(frozen=True)
class AnchorBalance:
    """Quantitative one-step residuals, independent of external event logs."""

    field_delta: tuple[int, int]
    relation_delta: tuple[int, int]
    transfer_residual: tuple[int, int]  # delta(field + relations), per polarity
    created_sc_anchors: tuple[int, ...]
    retired_sc_anchors: tuple[int, ...]
    changed_occupied_sc_anchors: tuple[int, ...]
    changed_fcc_anchors: tuple[int, ...]
    invalid_sc_gains: tuple[int, ...]

    @property
    def absorbed_tokens(self) -> int:
        return -sum(self.field_delta)


def transition_balance(before: V.StagedState, after: V.StagedState) -> AnchorBalance:
    """Inspect a supplied microtick pair; does not certify complete evolution.

    A caller can supply different states with the same inventory. This function
    checks the named integer restrictions, not provenance or dynamical closure.
    """
    old, new = inventory(before), inventory(after)
    if old.lattice_size != new.lattice_size or after.microtick != before.microtick + 1:
        raise ValueError("balance requires matching lattices and adjacent microticks")
    field = tuple(b - a for a, b in zip(old.field_by_polarity, new.field_by_polarity))
    old_rel = old.sc_by_polarity + old.fcc_by_polarity
    new_rel = new.sc_by_polarity + new.fcc_by_polarity
    relation = tuple(sum(b[p] - a[p] for a, b in zip(old_rel, new_rel)) for p in range(2))
    created, retired, changed, invalid = [], [], [], []
    for index, (a, b) in enumerate(zip(old.sc_by_polarity, new.sc_by_polarity)):
        if not sum(a) and sum(b):
            created.append(index)
            if sum(b) != 1:
                invalid.append(index)
        if sum(a) and not sum(b):
            retired.append(index)
        if sum(a) and a != b:
            changed.append(index)
        if any(bp < ap for ap, bp in zip(a, b)):
            invalid.append(index)
    fcc_changed = tuple(i for i, (a, b) in enumerate(zip(old.fcc_by_polarity, new.fcc_by_polarity)) if a != b)
    return AnchorBalance(field, relation, tuple(a + b for a, b in zip(field, relation)),
                         tuple(created), tuple(retired), tuple(changed), fcc_changed,
                         tuple(sorted(set(invalid))))


def assert_anchor_obstructions(before: V.StagedState, after: V.StagedState) -> AnchorBalance:
    """Reject supplied transitions violating the current law's exact inventory constraints."""
    balance = transition_balance(before, after)
    if (any(delta > 0 for delta in balance.field_delta)
            or any(balance.transfer_residual)
            or balance.retired_sc_anchors or balance.changed_occupied_sc_anchors
            or balance.changed_fcc_anchors or balance.invalid_sc_gains
            or balance.absorbed_tokens != len(balance.created_sc_anchors)):
        raise ValueError(f"current-law anchor obstruction violated: {balance}")
    if before.phase != 0 and (balance.absorbed_tokens or balance.created_sc_anchors):
        raise ValueError("only admission may transfer field tokens into new SC anchors")
    return balance


def field_free(state: V.StagedState) -> bool:
    """The exactly empty-bank invariant sector, including intermediate phases."""
    V.validate(state)
    return not bool(state.lattice.bank.any())


def streaming_direction_witness(channel: int, manifested: bool) -> tuple[tuple[int, int, int], int]:
    """One isolated streaming leg: manifestation changes phase, not this hop.

    This is not the full candidate tick. Previous collisions can change the
    input channel, and later collisions/admission may depend on its phase.
    """
    if (isinstance(channel, bool) or not isinstance(channel, (int, np.integer))
            or not 0 <= channel < C.N_CHANNELS or not isinstance(manifested, bool)):
        raise ValueError("expected a canonical channel and Boolean manifestation")
    output = C.U(int(channel))
    if manifested:
        output = C.half_turn(output)
    return C.tangent(int(channel)), output
