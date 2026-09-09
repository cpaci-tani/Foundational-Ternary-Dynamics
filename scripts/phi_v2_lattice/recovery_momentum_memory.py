"""Exact registered conserved-momentum return after eight physical microticks.

This contracts the complete finite counting ensemble. It is not a simulator,
product-closure certificate, fluid limit, or new microscopic law.
"""
from __future__ import annotations

from dataclasses import dataclass
from fractions import Fraction as F
from functools import lru_cache
import hashlib
from itertools import product
from numbers import Integral
from pathlib import Path

from . import hydro_parity as H
from . import recovery_micro_correlation as characters

EPSILON = F(1, 8)
FRONTIER = (4, 0, 0)
EXPECTED_DELTA = F(139921773018153334181957509616241,
                   44601490397061246283071436545296723011960832)
EXPECTED_SIGNED_ERROR = -EXPECTED_DELTA / 576
FROZEN_INPUTS = (
    ("scripts/phi_v2_lattice/hydro_parity.py", "593bd2eb314bd4f712e4ac125600147518003fecd8ad98ca51cd8893f77a410b"),
    ("scripts/phi_v2_lattice/recovery_micro_correlation.py", "9645b3872be8f45c9ac63b7d312ac430872cfb6fa20fa88ec988ee4cb6011dee"),
    ("engine/docs/PROPOSAL_STRICT_MICROSCOPIC_FLUID_MEMORY_V1.md", "0e73ffcece95a3cefe1c9c89b0821d80d7cc0ad0ebfb1cf3680d8811b8e055c3"),
)


def _integer(value, name, low=0, high=None):
    if (isinstance(value, bool) or not isinstance(value, Integral) or value < low
            or (high is not None and value > high)):
        raise ValueError(f"invalid {name}")
    return int(value)


def _case_inputs(L, bank, origin):
    L = _integer(L, "registered size", 9, 10)
    bank = _integer(bank, "polarity bank", 0, 1)
    try:
        origin = tuple(origin)
    except TypeError as exc:
        raise ValueError("three integer origin coordinates required") from exc
    if len(origin) != 3 or any(isinstance(x, bool) or not isinstance(x, Integral) for x in origin):
        raise ValueError("three integer origin coordinates required")
    return L, bank, tuple(int(x) % L for x in origin)


def frozen_inputs():
    root = Path(__file__).resolve().parents[2]
    rows = []
    for name, expected in FROZEN_INPUTS:
        actual = hashlib.sha256((root / name).read_bytes()).hexdigest()
        if actual != expected:
            raise ValueError(f"frozen diagnostic input changed: {name}")
        rows.append({"path": name, "sha256": actual})
    return tuple(rows)


@lru_cache(maxsize=1)
def _operator():
    return characters.CharacterOperator(H)


def _spin(occupied, channel):
    return 2 * ((occupied >> channel) & 1) - 1


def _character(occupied, mask):
    return -1 if (mask.bit_count() - (mask & occupied).bit_count()) % 2 else 1


def _subsets(mask):
    labels = tuple(i for i in range(24) if mask >> i & 1)
    return tuple(sum(1 << c for k, c in enumerate(labels) if n >> k & 1)
                 for n in range(1 << len(labels)))


def _reflection(mask):
    return sum(1 << H.R4_ACTION[c] for c in range(24) if mask >> c & 1)


@lru_cache(maxsize=1)
def _eligible_outputs():
    # Actual scalar outputs, not the expected signed-return constant.
    return tuple((a, H.collide_mask(a)) for a in H.eligible_masks())


def _joint_assignment(mask_a, value_a, mask_b, value_b):
    if (value_a ^ value_b) & mask_a & mask_b:
        return F(0)
    return F(1, 1 << (mask_a | mask_b).bit_count())


@lru_cache(maxsize=32)
def _configuration_joint(incoming, outgoing):
    """Full local joint distribution, independently counted in configuration space."""
    if max(incoming.bit_count(), outgoing.bit_count()) > 2:
        raise ValueError("registered middle block exceeds two slots")
    rows = []
    for x, y in product(_subsets(incoming), _subsets(outgoing)):
        identity = _joint_assignment(incoming, x, outgoing, y)
        reflected = _joint_assignment(incoming, x, _reflection(outgoing), _reflection(y))
        # At most four fixed bits leave free parity exactly balanced.
        value = (identity + reflected) / 2
        correction = sum(int((after & outgoing) == y) - int((before & outgoing) == y)
                         for before, after in _eligible_outputs() if before & incoming == x)
        rows.append(((x, y), value + F(correction, 1 << 24)))
    if sum(value for _, value in rows) != 1 or any(value < 0 for _, value in rows):
        raise AssertionError("invalid complete local joint probability")
    return tuple(rows)


@lru_cache(maxsize=16)
def _configuration_endpoint(channel, mask):
    if H.R4_ACTION[channel] != channel:
        raise ValueError("registered endpoint is not a reflection-fixed edge label")
    rows = []
    for x in _subsets(mask):
        # On these FCC endpoint labels the entire odd branch is the identity.
        baseline = _spin(x, channel) if mask >> channel & 1 else 0
        correction = sum(_spin(after, channel) - _spin(before, channel)
                         for before, after in _eligible_outputs() if before & mask == x)
        value = F(baseline) + F(correction, 1 << (24 - mask.bit_count()))
        if abs(value) > 1:
            raise AssertionError("conditional spin expectation outside alphabet")
        rows.append((x, value))
    return tuple(rows)


def _separator(displacement):
    incoming, outgoing = {}, {}
    for c, velocity in enumerate(H.VELOCITIES):
        incoming[velocity] = incoming.get(velocity, 0) | (1 << c)
        y = tuple(displacement[a] - velocity[a] for a in range(3))
        outgoing[y] = outgoing.get(y, 0) | (1 << c)
    blocks = tuple((y, incoming[y], outgoing[y]) for y in sorted(set(incoming) & set(outgoing)))
    left = 0
    right = 0
    for _, a, b in blocks:
        left |= a
        right |= b
    if max(left.bit_count(), right.bit_count()) > 6:
        raise ValueError("registered endpoint separator exceeds six bits")
    return left, right, blocks


def _walsh_contraction(source, target, left, right, blocks):
    op = _operator()
    total = F(0)
    for a, b in product(_subsets(left), _subsets(right)):
        value = op.moment_masks(1 << source, a) * op.moment_masks(1 << target, b)
        if not value:
            continue
        for _, incoming, outgoing in blocks:
            value *= op.moment_masks(b & outgoing, a & incoming)
            if not value:
                break
        total += value
    return total


def _configuration_contraction(source, target, left, right, blocks):
    tables = tuple(dict(_configuration_joint(a, b)) for _, a, b in blocks)
    total = F(0)
    for (x, f), (y, g) in product(_configuration_endpoint(source, left),
                                   _configuration_endpoint(target, right)):
        probability = F(1)
        for (_, incoming, outgoing), table in zip(blocks, tables):
            probability *= table[x & incoming, y & outgoing]
        total += f * g * probability
    return total


@dataclass(frozen=True)
class EndpointTerm:
    source: int
    target: int
    displacement: tuple[int, int, int]
    momentum_weight: int
    endpoint_bits: tuple[int, int]
    shared_sites: int
    configuration_pairs: int
    actual_walsh: F
    actual_configuration: F
    projected: F

    @property
    def returned(self):
        return self.actual_walsh - self.projected


@lru_cache(maxsize=1)
def endpoint_terms():
    labels = tuple(c for c, v in enumerate(H.VELOCITIES) if v[0] == 1 and v[1] != 0)
    if len(labels) != 2:
        raise AssertionError("frontier does not have the registered two FCC labels")
    jacobian = _operator().jacobian
    terms = []
    for source, target in product(labels, repeat=2):
        displacement = tuple(FRONTIER[a] - H.VELOCITIES[source][a] - H.VELOCITIES[target][a]
                             for a in range(3))
        left, right, blocks = _separator(displacement)
        actual = _walsh_contraction(source, target, left, right, blocks)
        independently_counted = _configuration_contraction(source, target, left, right, blocks)
        if actual != independently_counted:
            raise AssertionError("Walsh and complete configuration contractions disagree")
        # Independent three-collision, two-stream FIRST-DEGREE comparison.
        projected = F(0)
        for _, incoming, outgoing in blocks:
            for c in range(24):
                if not incoming >> c & 1:
                    continue
                for b in range(24):
                    if outgoing >> b & 1:
                        projected += jacobian[target][b] * jacobian[b][c] * jacobian[c][source]
        terms.append(EndpointTerm(source, target, displacement,
                                  H.VELOCITIES[source][1] * H.VELOCITIES[target][1],
                                  (left.bit_count(), right.bit_count()), len(blocks),
                                  1 << (left.bit_count() + right.bit_count()),
                                  actual, independently_counted, projected))
    return tuple(terms)


def projected_score(microticks=8):
    """External comparison on an unwrapped cone; immutable exact coefficients."""
    microticks = _integer(microticks, "registered microticks", 0, 8)
    return _projected_score(microticks)


@lru_cache(maxsize=9)
def _projected_score(microticks):
    if microticks == 0:
        return tuple((((0, 0, 0), c), F(v[1], 12)) for c, v in enumerate(H.VELOCITIES) if v[1])
    previous = dict(projected_score(microticks - 1))
    values = {}
    if microticks % 2 == 1:
        jacobian = _operator().jacobian
        for (site, c), value in previous.items():
            for a in range(24):
                key = (site, a)
                values[key] = values.get(key, F(0)) + value * jacobian[a][c]
    else:
        for (site, c), value in previous.items():
            target = tuple(site[a] + H.VELOCITIES[c][a] for a in range(3))
            key = (target, c)
            if key in values:
                raise AssertionError("labelled streaming is not bijective")
            values[key] = value
    return tuple(sorted((key, value) for key, value in values.items() if value))


def lifted_comparison(L, bank=0, origin=(0, 0, 0), microticks=8):
    L, bank, origin = _case_inputs(L, bank, origin)
    values = {}
    for (site, c), value in projected_score(microticks):
        point = tuple((origin[a] + site[a]) % L for a in range(3))
        key = (point, bank, c)
        if key in values:
            raise AssertionError("periodic identification in the registered causal cone")
        values[key] = value
    return tuple(sorted(values.items()))


@lru_cache(maxsize=1)
def _response():
    terms = endpoint_terms()
    actual = sum((t.momentum_weight * t.actual_walsh for t in terms), F(0)) / 144
    projected = sum((t.momentum_weight * t.projected for t in terms), F(0)) / 144
    score = dict(projected_score())
    direct_projection = sum((F(v[1], 12) * score.get((FRONTIER, a), F(0))
                             for a, v in enumerate(H.VELOCITIES)), F(0))
    if direct_projection != projected:
        raise AssertionError("four endpoint terms omit or misorient an outer stream")
    l1 = sum(abs(value) for value in score.values())
    norm2 = sum(value * value for value in score.values())
    minimum = 1 - EPSILON * l1
    if minimum < 0:
        raise AssertionError("registered comparison density is not nonnegative")
    return actual, projected, l1, norm2, minimum


def momentum_case(L, bank=0, origin=(0, 0, 0)):
    L, bank, origin = _case_inputs(L, bank, origin)
    lifted = lifted_comparison(L, bank, origin)
    actual, projected, l1, norm2, minimum = _response()
    target = tuple((origin[a] + FRONTIER[a]) % L for a in range(3))
    observed = sum((F(H.VELOCITIES[c][1], 12) * value for (site, p, c), value in lifted
                    if site == target and p == bank), F(0))
    if observed != projected:
        raise AssertionError("translated observation changed the exact comparison")
    return {"L": L, "polarity_bank": bank, "origin": origin, "target": target,
            "represented_microticks": (0, 8), "cycles": 4,
            "complete_boolean_slots": 48 * L**3,
            "prepared_score_norm_squared": F(1, 12), "observable_norm_squared": F(1, 12),
            "true_density_integral": F(1), "true_density_minimum": 1 - EPSILON,
            "true_density_maximum": 1 + EPSILON,
            "true_density_norm_squared": 1 + EPSILON**2 / 12,
            "comparison_density_integral": F(1), "comparison_density_minimum": minimum,
            "comparison_score_l1": l1, "comparison_score_norm_squared": norm2,
            "actual_observable_expectation": EPSILON * actual,
            "comparison_observable_expectation": EPSILON * projected,
            "signed_weak_error": EPSILON * (actual - projected),
            "absolute_weak_error": abs(EPSILON * (actual - projected)),
            "full_density_L2_error_squared_lower_bound": 12 * (EPSILON * (actual - projected))**2,
            "total_variation_lower_bound": abs(EPSILON * (actual - projected)) / 2}


def _json_ready(value):
    if isinstance(value, F):
        return str(value)
    if isinstance(value, dict):
        return {key: _json_ready(item) for key, item in value.items()}
    if isinstance(value, (tuple, list)):
        return [_json_ready(item) for item in value]
    return value


def certificate():
    inputs = frozen_inputs()
    terms = endpoint_terms()
    rows = [momentum_case(L, bank, origin) for L in (9, 10) for bank in (0, 1)
            for origin in ((0, 0, 0), (L - 1, L - 1, L - 1))]
    for term in terms:
        expected = F(0) if term.momentum_weight == 1 else EXPECTED_DELTA
        if term.returned != expected:
            raise AssertionError("fixed endpoint return differs from registered evidence")
    if any(row["signed_weak_error"] != EXPECTED_SIGNED_ERROR for row in rows):
        raise AssertionError("registered momentum-memory identity failed")
    return _json_ready({
        "schema": "strict-momentum-memory-certificate-v1", "law_id": H.LAW_ID,
        "frozen_inputs": inputs, "epsilon": EPSILON,
        "preparation": "positive equal mixture of twelve single-slot product preparations",
        "reference": "complete half-occupation counting ensemble, ordinal conditioned",
        "norm": "absolute normalized local y-momentum expectation difference",
        "endpoint_terms": [dict(source=t.source, target=t.target, displacement=t.displacement,
                                 momentum_weight=t.momentum_weight, endpoint_bits=t.endpoint_bits,
                                 shared_sites=t.shared_sites, configuration_pairs=t.configuration_pairs,
                                 actual_walsh=t.actual_walsh, actual_configuration=t.actual_configuration,
                                 projected=t.projected, returned=t.returned) for t in terms],
        "cases": rows,
        "memory_facts": {"K0": "zero by exact two-collision support theorem",
                         "cycle_operator": "U=S*C on ordinal-conditioned counting-measure fibres",
                         "kernel_definition": "K_l=B*D^l*Cm; B=P*U*Q, Cm=Q*U*P, D=Q*U*Q",
                         "exact_recurrence": "x[n+1]=A*x[n]+sum(l=0..n-1,K_l*x[n-1-l])+B*D^n*z0",
                         "initial_unresolved_score": "z0=0 for the registered mixture",
                         "initial_Cm_score": "zero: local conserved momentum is collision-fixed",
                         "first_degree_exact_through_cycles": 3,
                         "conserved_local_means_exact_through_microtick": 7,
                         "cycle4_difference": "K1*A*f0, with K1=B*D*Cm",
                         "full_memory_kernel_computed": False},
        "registered_identity_verified": True,
        "limits": {"sampling_error": "none: exact integer/rational contractions",
                   "single_product_finite_amplitude_preparation": False,
                   "exterior_records_resampled": False,
                   "individual_trajectory_fluid_limit": False,
                   "long_time_error_upper_bound": False, "continuum_recovered": False,
                   "mechanical_momentum_identified": False, "unified_binding_law": False,
                   "canonical_adoption": False}})
