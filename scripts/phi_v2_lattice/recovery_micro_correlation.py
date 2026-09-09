"""Exact half-reference microscopic correlation contractions, not a fluid solver.

See DERIV_STRICT_MICRO_CORRELATION_V1.md for the pre-validation contract.
The character oracle integrates every exterior Boolean variable. It does not
reinitialize those variables between collisions or evolve a product closure.
"""
from __future__ import annotations

from dataclasses import dataclass
from fractions import Fraction
from functools import lru_cache
from numbers import Integral

FULL = (1 << 24) - 1
SOURCE_LABEL = (1, 1, 0, 0)
TARGET_LABEL = (1, -1, 0, 0)
RETURN_DISPLACEMENT = (2, 0, 0)


def _integer(value, name, minimum=0, maximum=None):
    if (isinstance(value, bool) or not isinstance(value, Integral)
            or value < minimum or (maximum is not None and value > maximum)):
        raise ValueError(f"invalid {name}")
    return int(value)


def _rational(value, name):
    if isinstance(value, bool) or not isinstance(value, (Integral, Fraction)):
        raise ValueError(f"{name} must be exact rational")
    return Fraction(value)


def _coordinate(value, L):
    try:
        values = tuple(value)
    except TypeError as exc:
        raise ValueError("three integer coordinates required") from exc
    if len(values) != 3 or any(isinstance(v, bool) or not isinstance(v, Integral)
                               for v in values):
        raise ValueError("three integer coordinates required")
    return tuple(int(v) % L for v in values)


def _mask(channels):
    values = tuple(_integer(c, "channel", maximum=23) for c in channels)
    if len(set(values)) != len(values):
        raise ValueError("character channels must be distinct")
    return sum(1 << c for c in values)


def character(mask, occupied):
    """Product of 2*N-1 over mask, with the empty product equal to one."""
    mask = _integer(mask, "character mask", maximum=FULL)
    occupied = _integer(occupied, "occupied mask", maximum=FULL)
    return -1 if (mask.bit_count() - (mask & occupied).bit_count()) % 2 else 1


class CharacterOperator:
    """Exact character matrix of one of the two specified local involutions.

    Candidate enumeration completeness belongs to its independent finite
    certificate. This adapter checks shape and the defining eligible predicate;
    it does not claim to reprove completeness from a supplied list alone.
    """

    def __init__(self, candidate):
        if candidate.LAW_ID not in ("phi-hydro-complement-field-sector-1",
                                    "phi-hydro-parity-field-sector-1"):
            raise ValueError("unsupported collision law")
        self.law_id = candidate.LAW_ID
        self.labels = tuple(tuple(v) for v in candidate.LIFTED_VELOCITIES)
        if (len(self.labels) != 24 or len(set(self.labels)) != 24
                or any(len(v) != 4 or sum(x*x for x in v) != 2
                       or any(x not in (-1, 0, 1) for x in v) for v in self.labels)):
            raise ValueError("invalid 24-cell labels")
        self.velocities = tuple(v[:3] for v in self.labels)
        if tuple(candidate.VELOCITIES) != self.velocities:
            raise ValueError("foreign streaming convention")
        self.parity = self.law_id == "phi-hydro-parity-field-sector-1"
        index = {v: c for c, v in enumerate(self.labels)}
        self.reflection = tuple(index[v[:3]+(-v[3],)] for v in self.labels)
        if self.parity and tuple(candidate.R4_ACTION) != self.reflection:
            raise ValueError("foreign reflection convention")
        self.eligible = tuple(candidate.eligible_masks())
        if self.eligible != tuple(sorted(set(self.eligible))):
            raise ValueError("eligible list must be unique and sorted")
        dimensions = 4 if self.parity else 3
        members = set(self.eligible)
        for occupied in self.eligible:
            _integer(occupied, "eligible mask", maximum=FULL)
            if (occupied.bit_count() != 12 or FULL ^ occupied not in members
                    or any(sum(v[a] for c, v in enumerate(self.labels)
                               if occupied & (1 << c)) != 0
                           for a in range(dimensions))):
                raise ValueError("invalid complement-eligible state")

    def reflected(self, mask):
        return sum(1 << self.reflection[c] for c in range(24) if mask & (1 << c))

    @lru_cache(maxsize=4096)
    def eligible_character_sum(self, mask):
        mask = _integer(mask, "character mask", maximum=FULL)
        parity = mask.bit_count() % 2
        return sum(-1 if ((occupied & mask).bit_count() % 2) != parity else 1
                   for occupied in self.eligible)

    @lru_cache(maxsize=8192)
    def moment_masks(self, output_mask, input_mask):
        """E_mu[chi_output(F(X))*chi_input(X)], exactly."""
        output_mask = _integer(output_mask, "output mask", maximum=FULL)
        input_mask = _integer(input_mask, "input mask", maximum=FULL)
        if self.parity:
            reflected = self.reflected(output_mask)
            value = Fraction(int(input_mask == output_mask)
                             + int(input_mask == reflected)
                             + int(input_mask == (FULL ^ output_mask))
                             - int(input_mask == (FULL ^ reflected)), 2)
        else:
            value = Fraction(input_mask == output_mask)
        if output_mask.bit_count() % 2:
            value -= Fraction(self.eligible_character_sum(output_mask ^ input_mask), 1 << 23)
        return value

    def moment(self, output_channels, input_channels):
        return self.moment_masks(_mask(output_channels), _mask(input_channels))

    @property
    def jacobian(self):
        return tuple(tuple(self.moment((a,), (i,)) for i in range(24)) for a in range(24))


def _subsets(channels):
    return tuple(sum(1 << c for k, c in enumerate(channels) if subset & (1 << k))
                 for subset in range(1 << len(channels)))


def projected_coefficients(operator, L, microticks, source_channel, origin=(0, 0, 0)):
    """External comparison only: local J and exact labelled streaming."""
    L = _integer(L, "periodic size", 3)
    microticks = _integer(microticks, "diagnostic microticks", maximum=6)
    source_channel = _integer(source_channel, "source channel", maximum=23)
    origin = _coordinate(origin, L)
    coefficients = {(origin, source_channel): Fraction(1)}
    jacobian = operator.jacobian
    for tick in range(microticks):
        updated = {}
        if tick % 2 == 0:
            for (site, c), value in coefficients.items():
                for a in range(24):
                    key = (site, a)
                    updated[key] = updated.get(key, Fraction(0)) + jacobian[a][c]*value
        else:
            for (site, c), value in coefficients.items():
                target = tuple((site[j]+operator.velocities[c][j]) % L for j in range(3))
                # Streaming is a bijection of labelled slots, including seams.
                key = (target, c)
                if key in updated:
                    raise AssertionError("non-bijective streaming")
                updated[key] = value
        coefficients = {key: value for key, value in updated.items() if value}
    return tuple(sorted(coefficients.items()))


def two_cycle_density(operator, L, source_channel, epsilon=Fraction(1, 8), origin=(0, 0, 0)):
    """Sharp full-ensemble errors by the proved two-collision orthogonality."""
    epsilon = _rational(epsilon, "epsilon")
    if abs(epsilon) > 1:
        raise ValueError("invalid single-bit preparation")
    rows = []
    for tick in range(5):
        values = projected_coefficients(operator, L, tick, source_channel, origin)
        norm = sum((value*value for _, value in values), Fraction(0))
        if norm > 1:
            raise AssertionError("projected coefficient norm is not contractive")
        rows.append({"microtick": tick, "represented_slots": len(values),
                     "projected_norm_squared": norm,
                     "full_density_error_squared": epsilon*epsilon*(1-norm),
                     "linear_comparator_nonnegative": abs(epsilon)*sum(abs(v) for _, v in values) <= 1})
    return tuple(rows)


@dataclass(frozen=True)
class EndpointContraction:
    left_channels: tuple[int, ...]
    right_channels: tuple[int, ...]
    # Each block is (shared middle site, incoming mask, outgoing mask).
    blocks: tuple[tuple[tuple[int, int, int], int, int], ...]
    left_score: tuple[tuple[int, Fraction], ...]
    right_score: tuple[tuple[int, Fraction], ...]


def endpoint_contraction(operator, L, displacement, source_channel, target_channel, *, max_bits=8):
    """Construct the exact third-collision separator, refusing large endpoints."""
    L = _integer(L, "periodic size", 3)
    source_channel = _integer(source_channel, "source channel", maximum=23)
    target_channel = _integer(target_channel, "target channel", maximum=23)
    max_bits = _integer(max_bits, "endpoint bit budget", 1, 12)
    displacement = _coordinate(displacement, L)
    incoming, outgoing = {}, {}
    for c, v in enumerate(operator.velocities):
        y = tuple(coordinate % L for coordinate in v)
        incoming[y] = incoming.get(y, 0) | (1 << c)
        y = tuple((displacement[j]-v[j]) % L for j in range(3))
        outgoing[y] = outgoing.get(y, 0) | (1 << c)
    shared = sorted(set(incoming) & set(outgoing))
    blocks = tuple((y, incoming[y], outgoing[y]) for y in shared)
    left = tuple(c for c in range(24) if any(a & (1 << c) for _, a, _ in blocks))
    right = tuple(c for c in range(24) if any(b & (1 << c) for _, _, b in blocks))
    if max(len(left), len(right)) > max_bits:
        raise ValueError("exact endpoint contraction exceeds declared bit budget")
    if any(a.bit_count() > 2 or b.bit_count() > 2 for _, a, b in blocks):
        raise AssertionError("streaming velocity multiplicity exceeded two")
    left_score = tuple((mask, operator.moment_masks(1 << source_channel, mask)) for mask in _subsets(left))
    right_score = tuple((mask, operator.moment_masks(1 << target_channel, mask)) for mask in _subsets(right))
    return EndpointContraction(left, right, blocks, left_score, right_score)


def contract_walsh(operator, contraction):
    """Exact microscopic response; all unretained bits were integrated out."""
    total = Fraction(0)
    for a, left in contraction.left_score:
        if not left:
            continue
        for b, right in contraction.right_score:
            if not right:
                continue
            value = left*right
            for _, incoming, outgoing in contraction.blocks:
                value *= operator.moment_masks(b & outgoing, a & incoming)
                if not value:
                    break
            total += value
    return total


def third_collision_response(operator, L, displacement, source_channel, target_channel):
    contraction = endpoint_contraction(operator, L, displacement, source_channel, target_channel)
    actual = contract_walsh(operator, contraction)
    comparison = dict(projected_coefficients(operator, L, 5, source_channel)).get(
        (_coordinate(displacement, L), int(target_channel)), Fraction(0))
    return {"L": int(L), "microtick": 5, "next_stream_microtick": 6,
            "displacement": _coordinate(displacement, L),
            "source_channel": int(source_channel), "target_channel": int(target_channel),
            "shared_middle_sites": len(contraction.blocks),
            "left_endpoint_bits": len(contraction.left_channels),
            "right_endpoint_bits": len(contraction.right_channels),
            "subset_pairs": len(contraction.left_score)*len(contraction.right_score),
            "microscopic_response": actual, "projected_response": comparison,
            "returned_correlation": actual-comparison}


def _json_ready(value):
    if isinstance(value, Fraction):
        return str(value)
    if isinstance(value, dict):
        return {key: _json_ready(item) for key, item in value.items()}
    if isinstance(value, (tuple, list)):
        return [_json_ready(item) for item in value]
    return value


def certificate(candidate):
    operator = CharacterOperator(candidate)
    source = operator.labels.index(SOURCE_LABEL)
    target = operator.labels.index(TARGET_LABEL)
    j = operator.labels.index((1, 0, 0, -1))
    k = operator.labels.index((1, 0, 0, 1))
    epsilon = Fraction(1, 8)
    third = operator.moment((source,), (source, j, k))
    rows = [third_collision_response(operator, L, RETURN_DISPLACEMENT, source, target) for L in (5, 6)]
    for row in rows:
        row["finite_amplitude_expectation_difference"] = epsilon*row["returned_correlation"]
        row["full_density_L2_error_lower_bound"] = abs(row["finite_amplitude_expectation_difference"])
        coefficients = projected_coefficients(operator, row["L"], 5, source)
        row["linear_comparator_nonnegative"] = epsilon*sum(abs(v) for _, v in coefficients) <= 1
    return _json_ready({
        "schema": "strict-micro-correlation-certificate-v1", "law_id": operator.law_id,
        "reference": "complete independent half-occupation counting ensemble, clock conditioned",
        "preparation": "one biased slot; exact density 1+epsilon*(2N-1)",
        "epsilon": epsilon, "polarity": "either separately; other bank integrates out",
        "source_label": SOURCE_LABEL, "target_label": TARGET_LABEL,
        "complete_density_comparison_L": 5,
        "first_leakage": {"microtick": 1, "transported_microtick": 2,
                          "triple_channels": (source, j, k), "score_coefficient": third,
                          "finite_amplitude_expectation": epsilon*third},
        "two_cycle_full_density": two_cycle_density(operator, 5, source, epsilon),
        "third_collision_return": rows,
        "limits": {"outside_bits_conditioned_or_resampled": False,
                   "sampling_error": "none: exact rational counting contraction",
                   "density_comparator": "external first-degree density, not regenerated product",
                   "all_response_entries_at_third_collision": False,
                   "long_horizon_memory_tail_certified": False,
                   "hydrodynamic_recovery": False, "material_recovery": False,
                   "canonical_adoption": False},
    })
