"""External finite counting references for the unchanged staged candidate.

Bank stationarity is exact; gates require a correlated complete-state lift.
This module samples no probabilities and supplies no hydrodynamic closure.
"""
from __future__ import annotations

from dataclasses import dataclass
from copy import deepcopy
from fractions import Fraction
from itertools import combinations
from math import comb
from numbers import Integral

import numpy as np

from . import channels as C, staged as P, state as S
from ._proofs import encode, rotate, relation_tick

PREPARATION_ID = "strict-kinetic-counting-reference-1"


def _integer(value, name, minimum=0):
    if isinstance(value, bool) or not isinstance(value, Integral) or value < minimum:
        raise ValueError(f"invalid {name}")
    return int(value)


def _probability(value):
    if isinstance(value, bool) or not isinstance(value, (Fraction, Integral)):
        raise ValueError("external weight must be an exact integer or Fraction")
    value = Fraction(value)
    if not 0 <= value <= 1:
        raise ValueError("external weight outside [0,1]")
    return value


@dataclass(frozen=True)
class CountingReference:
    L: int
    p: Fraction

    def __post_init__(self):
        object.__setattr__(self, "L", _integer(self.L, "periodic size", 3))
        object.__setattr__(self, "p", _probability(self.p))

    @property
    def slots(self):
        return 384 * self.L**3

    @property
    def total_multiplicity(self):
        """Binomial-theorem cardinality; does not enumerate configurations."""
        return self.p.denominator**self.slots

    def multiplicity(self, occupied):
        occupied = _integer(occupied, "occupied count")
        if occupied > self.slots:
            raise ValueError("occupied count exceeds slot count")
        a, b = self.p.numerator, self.p.denominator
        return a**occupied * (b-a)**(self.slots-occupied)

    def configuration_weight(self, occupied):
        return Fraction(self.multiplicity(occupied), self.total_multiplicity)


def collision_eligibility(p) -> Fraction:
    p = _probability(p)
    return comb(192, 2) * p**2 * (1-p)**190


def eligibility_derivative_sign(p) -> int:
    """Sign on the open unit interval; endpoint derivative is zero."""
    p = _probability(p)
    if p in (0, 1):
        return 0
    numerator = 2 - 192*p
    return (numerator > 0) - (numerator < 0)


def feasibility(L, microticks, p=Fraction(1, 96), start_tick=0) -> dict:
    reference = CountingReference(L, p)
    microticks = _integer(microticks, "microtick horizon")
    start_tick = _integer(start_tick, "start tick")
    stages = (start_tick + microticks + 2)//4 - (start_tick + 2)//4
    eligible = collision_eligibility(reference.p)
    expected = 2 * reference.L**3 * stages * eligible
    return {"preparation_id": PREPARATION_ID, "law_id": P.LAW_ID,
            "p": reference.p, "start_tick": start_tick, "microticks": microticks,
            "collision_stages": stages, "per_site_polarity_eligibility": eligible,
            "expected_logged_pair_events": expected,
            "at_least_one_event_upper_bound": min(Fraction(1), expected),
            "temporal_independence_assumed": False,
            "relaxation_certified": False, "hydrodynamics_certified": False}


def even_gate_weight(p) -> Fraction:
    """Two distinct endpoints, each with 384 field slots."""
    p = _probability(p)
    return (1 + (1-2*p)**768)/2


def finite_map_certificate() -> dict:
    """Inspect complete finite maps underlying the non-enumerative proof."""
    tables = C.load_collision_tables()
    pairs = set(combinations(range(192), 2))
    if C._hash_tables(tables) != C.COLLISION_HASH:
        raise ValueError("changed collision table")
    for table in tables:
        if set(table) != pairs or set(table.values()) != pairs:
            raise ValueError("collision layer is not a complete pair permutation")
    if {C.U(c) for c in range(384)} != set(range(384)):
        raise ValueError("internal map is not a permutation")
    for c in range(384):
        if C.polarity(C.U(c)) != C.polarity(c):
            raise ValueError("internal map changes polarity")
        if sum(abs(d) for d in C.tangent(c)) != 1:
            raise ValueError("streaming is not one signed axial hop")
    for code in range(9):
        if code == S.BLANK_IDX:
            continue
        z = S.z_of(code)
        for gate in (False, True):
            if relation_tick(z, z, even_gate=gate) != (rotate(z), rotate(z)):
                raise ValueError("full homogeneous pair is gate-dependent")
    return {"law_id": P.LAW_ID, "collision_hash": C.COLLISION_HASH,
            "layers": len(tables), "pair_states_per_layer": len(pairs),
            "streaming_channels": 384, "background_codes": 8,
            "record_family_period_microticks": 48,
            "stationary_field_reference": True,
            "stationary_absolute_clock": False,
            "product_tangent_closure": False}


def prepare_bank(bank, L, layer=0, background=None) -> P.StagedState:
    """Prepare one actual bank in the reference support, without random draws."""
    L = _integer(L, "periodic size", 3)
    layer = _integer(layer, "layer")
    if layer > 2:
        raise ValueError("layer outside finite alphabet")
    if background is None:
        background = S.idx_of(encode(0, +1))
    background = _integer(background, "background")
    if background > 8 or background == S.BLANK_IDX:
        raise ValueError("background must be nonblank A9")
    if not isinstance(bank, np.ndarray) or bank.dtype != np.dtype(bool) or bank.shape != (L**3, 384):
        raise ValueError("bank must have canonical bool shape (L^3,384)")
    if np.any(bank.view(np.uint8) > 1):
        raise ValueError("bank has noncanonical bool bytes")
    lattice = S.blank(L)
    lattice.bank[:] = bank
    lattice.ell[:] = layer
    lattice.sc[:] = background
    lattice.fcc[:] = background
    return P.initialize(lattice)


def phase_lift(boundary: P.StagedState, phase: int) -> P.StagedState:
    """Actual complete-state lift; retained gates are never resampled/reset."""
    phase = _integer(phase, "phase")
    P.validate(boundary)
    st = boundary.lattice
    if (phase > 3 or boundary.phase != 0 or np.any(st.s)
            or np.any(st.ell != st.ell[0]) or st.sc.flat[0] == S.BLANK_IDX
            or np.any(st.sc != st.sc.flat[0]) or np.any(st.fcc != st.sc.flat[0])):
        raise ValueError("not a homogeneous reference boundary/phase")
    # Preserve complete arrays and the Python reference's unbounded clock.
    current = deepcopy(boundary)
    for _ in range(phase):
        current, _ = P.step(current)
    return current
