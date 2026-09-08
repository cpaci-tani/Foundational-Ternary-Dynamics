"""Exact preimage obstruction for the already identified co-located pair sector.

This is a passive certificate of the unchanged staged law, not successor
dynamics. The predicate has no physical particle, binding or energy identity.
Its registered structural argument is AUDIT_STRICT_PAIR_FORMATION_OBSTRUCTION.
"""
from __future__ import annotations

from dataclasses import dataclass
from itertools import combinations
from numbers import Integral

import numpy as np

from . import channels as C, staged as P, state as S
from . import recovery_kinetic_reference as K


def _flag(channel):
    return C.STATES[int(channel) % C.N_STATES][0]


@dataclass(frozen=True)
class PreimageCertificate:
    law_id: str
    collision_hash: str
    collision_layers: int
    collision_rows_checked: int
    field_channels_checked: int
    flags_checked: int
    physical_recovery_certified: bool = False


def certify_local_preimages() -> PreimageCertificate:
    """Exhaust local proof ingredients; do not enumerate global configurations."""
    tables = C.load_collision_tables()
    if len(tables) != 3 or C._hash_tables(tables) != C.COLLISION_HASH:
        raise ValueError("collision tables differ from the frozen candidate")
    domain = set(combinations(range(C.N_STATES), 2))
    rows = 0
    for table in tables:
        if set(table) != domain or set(table.values()) != domain:
            raise ValueError("collision layer is not the complete pair permutation")
        for before, after in table.items():
            if (_flag(before[0]) == _flag(before[1])) != (_flag(after[0]) == _flag(after[1])):
                raise ValueError("collision violates same-flag preimage equivalence")
            rows += 1

    flags = {_flag(c) for c in range(C.N_CHANNELS)}
    image = {}
    for c in range(C.N_CHANNELS):
        flag, output = _flag(c), C.U(c)
        next_flag = _flag(output)
        if flag in image and image[flag] != next_flag:
            raise ValueError("flag transport depends on phase or polarity")
        image[flag] = next_flag
        if C.polarity(output) != C.polarity(c):
            raise ValueError("streaming changes field polarity")
        if _flag(C.half_turn(c)) != flag:
            raise ValueError("manifestation half-turn changes the flag")
        if C.tangent(c) != tuple(flag[0]) or sum(abs(v) for v in C.tangent(c)) != 1:
            raise ValueError("streaming does not use the pre-update signed axial tangent")
    if set(image) != flags or set(image.values()) != flags:
        raise ValueError("flag map is not a bijection")
    if {C.U(c) for c in range(C.N_CHANNELS)} != set(range(C.N_CHANNELS)):
        raise ValueError("channel streaming map is not a permutation")
    return PreimageCertificate(P.LAW_ID, C.COLLISION_HASH, len(tables), rows,
                               C.N_CHANNELS, len(flags))


@dataclass(frozen=True)
class PairMembership:
    microtick: int
    slots: tuple[tuple[int, int], ...]  # unordered observation, no constituent IDs
    colocated_same_flag: bool


def _observe_sector(state: P.StagedState) -> PairMembership:
    P.validate(state)
    st = state.lattice
    background = int(st.sc.flat[0])
    if (np.any(st.s) or np.any(st.ell != st.ell[0])
            or background == S.BLANK_IDX or np.any(st.sc != background)
            or np.any(st.fcc != background) or np.any(state.admitted_sc)):
        raise ValueError("outside homogeneous occupied two-token sector")
    slots = tuple(tuple(map(int, row)) for row in np.argwhere(st.bank))
    if len(slots) != 2 or C.polarity(slots[0][1]) != C.polarity(slots[1][1]):
        raise ValueError("exactly two same-polarity field slots required")
    (first, c1), (second, c2) = slots
    return PairMembership(int(state.microtick), slots,
                          first == second and _flag(c1) == _flag(c2))


@dataclass(frozen=True)
class FormationAudit:
    observations: tuple[PairMembership, ...]
    collision_events: int
    work_units: int
    physical_recovery_certified: bool = False


def audit_pair_membership(initial: P.StagedState, microticks: int) -> FormationAudit:
    """Check a bounded actual trajectory against the registered all-time predicate.

    This makes a private complete-state copy. Gates are evolved by the actual
    law, never synthesized from a marginal. The structural proof, not a finite
    history or this observer, establishes absence of entry at every later time.
    """
    if isinstance(microticks, bool) or not isinstance(microticks, Integral) or microticks < 0:
        raise ValueError("microticks must be a nonnegative integer")
    state = K.phase_lift(initial, 0)
    observations = [_observe_sector(state)]
    member = observations[0].colocated_same_flag
    work = P.work_units(state)
    collisions = 0
    for _ in range(int(microticks)):
        state, events = P.step(state)
        observation = _observe_sector(state)
        if observation.microtick != observations[-1].microtick + 1:
            raise ValueError("trajectory skipped a physical microtick")
        if observation.colocated_same_flag != member:
            raise ValueError("same-flag co-location changed: formation preimage obstruction violated")
        if P.work_units(state) != work or events.absorptions or events.crossings or events.gate_holds:
            raise ValueError("prepared background or token accounting changed")
        observations.append(observation)
        collisions += len(events.collisions)
    return FormationAudit(tuple(observations), collisions, work)
