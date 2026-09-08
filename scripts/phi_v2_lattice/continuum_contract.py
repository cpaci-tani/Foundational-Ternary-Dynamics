"""Bounded audit instruments, not a trajectory-to-continuum certificate.

The projected generator below belongs only to the selected Phi-v2 channel
geometry and its three-original-tick convention. It is not inherited by a
successor law that changes scheduling, clocks, or collision preparation.
"""
from __future__ import annotations

from dataclasses import dataclass
from enum import Enum
from fractions import Fraction
from math import comb
from numbers import Integral

from . import channels as C


def _nonnegative_integer(value: int, name: str) -> int:
    if isinstance(value, bool) or not isinstance(value, Integral) or value < 0:
        raise ValueError(f"{name} must be a nonnegative integer")
    return int(value)


def half_occupation_collision_probability(channels: int = C.N_STATES) -> Fraction:
    """Exactly two occupied channels in one independent half-filled bank."""
    channels = _nonnegative_integer(channels, "channels")
    if channels < 2:
        raise ValueError("at least two channels are required")
    return Fraction(comb(channels, 2), 2 ** channels)


@dataclass(frozen=True)
class EventBudget:
    opportunities: int
    probability_per_opportunity: Fraction
    expected_events: Fraction
    probability_any_upper_bound: Fraction
    assumption: str = "Independent Bernoulli(1/2) channels at each counted opportunity"


def half_reference_event_budget(opportunities: int) -> EventBudget:
    """Conditional expectation and union bound; no time independence assumed.

    One opportunity is one site's one active polarity collision invocation.
    The caller must justify the named marginal reference at every invocation.
    This is not a bound for sparse, conditioned, or successor preparations.
    """
    count = _nonnegative_integer(opportunities, "opportunities")
    probability = half_occupation_collision_probability()
    expected = count * probability
    return EventBudget(count, probability, expected, min(Fraction(1), expected))


class EvolutionKind(str, Enum):
    MICROSCOPIC_TRAJECTORY = "microscopic_trajectory"
    FULL_COUNTING_ENSEMBLE = "full_counting_ensemble"
    MARGINAL_TANGENT = "marginal_tangent"
    PROJECTED_PREDICTION = "projected_prediction"


@dataclass(frozen=True)
class EvolutionIdentity:
    """Minimum provenance for one side of a continuum comparison.

    Norm/observable/preparation identifiers refer to declared definitions,
    not physical identifications. Tick units must identify the actual law's
    global tick. Providing metadata does not establish a certificate.
    """

    kind: EvolutionKind
    law_id: str
    law_hash: str
    preparation_id: str
    norm_id: str
    horizon_ticks: int
    observable_id: str
    tick_unit_id: str
    restriction_id: str

    def __post_init__(self):
        if not isinstance(self.kind, EvolutionKind):
            raise ValueError("kind must explicitly distinguish the evolution object")
        _nonnegative_integer(self.horizon_ticks, "horizon_ticks")
        for name in ("law_id", "law_hash", "preparation_id", "norm_id",
                     "observable_id", "tick_unit_id", "restriction_id"):
            value = getattr(self, name)
            if not isinstance(value, str) or not value.strip():
                raise ValueError(f"{name} must identify a declared definition")


@dataclass(frozen=True)
class RecoveryDisposition:
    trajectory_continuum_certified: bool
    missing_certificates: tuple[str, ...]


def selected_v2_disposition() -> RecoveryDisposition:
    """Audit result, deliberately not inferred from supplied metadata/budgets."""
    return RecoveryDisposition(False, (
        "Full counting-ensemble evolution versus repeated marginal tangent closure",
        "Unresolved-mode leakage and return under multiple projected steps",
        "Finite-amplitude preparation and finite-time trajectory/ensemble error",
        "Usable finite wavelength/time window including kinetic relaxation",
    ))


def projected_first_order_axes():
    """Reconstruct three exact 7x7 k-linear generator matrices from 192 flags.

    Uses actual layer readouts/displacements, not a hardcoded wave polynomial.
    This is the layer-averaged projected streaming moment. Identifying it with
    a multi-step full ensemble derivative requires separate closure evidence.
    """
    from sympy import I, ImmutableMatrix, Matrix

    axes = [Matrix.zeros(7, 7) for _ in range(3)]
    for layer in range(3):
        columns = [(1,) + C.layer_value_of(c, layer) for c in range(C.N_STATES)]
        rows = Matrix(7, C.N_STATES, lambda i, j: columns[j][i])
        gram_inverse = (rows * rows.T).inv()
        for axis in range(3):
            weighted = Matrix(7, C.N_STATES,
                              lambda i, j: columns[j][i] * C.tangent(j)[axis])
            axes[axis] += -I * weighted * rows.T * gram_inverse / 3
    return tuple(ImmutableMatrix(axis) for axis in axes)


def projection_iteration_counterexample():
    """Contraction F: exact projected one-step equality, unequal iterates.

    P F P = 0, but P F^2 P = P. The removed component leaves and returns;
    no one-step residual of P F P alone can account for that memory.
    """
    from sympy import ImmutableMatrix

    return (ImmutableMatrix([[0, 1], [1, 0]]),
            ImmutableMatrix([[1, 0], [0, 0]]))


@dataclass(frozen=True)
class ProductTangentWitness:
    source_pair: tuple[int, int]
    target_pair: tuple[int, int]
    perturbed_channel: int
    score_values: tuple[int, int, int, int]
    mixed_difference: int


def product_tangent_witness(collision_table) -> ProductTangentWitness:
    """Actual frozen collision breaks the independent-product tangent family.

    Perturb input p_a=1/2+epsilon, with all other p=1/2. Since the collision
    is a permutation, output density relative to uniform is exactly
    1+2*epsilon*(2*C^{-1}(y)_a-1). A product tangent has an affine Boolean
    score. On y=empty,{c},{d},{c,d}, the preimage occupancy of a has a
    nonzero mixed difference. This certifies correlation content at FIRST
    order, not merely a finite-amplitude counterexample. It does not prove
    that this content feeds back into every particular projected observable.
    """
    source, target = min(collision_table.items())
    source, target = tuple(source), tuple(target)
    lost = sorted(set(source) - set(target))
    if not lost or collision_table[target] != source:
        raise ValueError("witness requires a nontrivial involutive pair collision")
    perturbed = lost[0]
    c, d = target

    def score(occupied):
        preimage = collision_table[tuple(sorted(occupied))] if len(occupied) == 2 else occupied
        return int(perturbed in preimage)

    values = tuple(score(occupied) for occupied in ((), (c,), (d,), (c, d)))
    mixed = values[3] - values[1] - values[2] + values[0]
    return ProductTangentWitness(source, target, perturbed, values, mixed)
