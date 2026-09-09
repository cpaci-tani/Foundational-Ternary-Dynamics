"""Selected odd-reflection/full-four-momentum complement field law.

Specified before validation in SPEC_STRICT_HYDRO_PARITY_CANDIDATE_V1.md.
This is an isolated finite research sector, not the production Phi law.
Exact product stationarity and a marginal derivative do not establish
autonomous marginal dynamics or microscopic hydrodynamic closure.
"""
from __future__ import annotations

from dataclasses import dataclass
from fractions import Fraction
from functools import lru_cache
import hashlib
from itertools import combinations, permutations, product
import json

import flint
import numpy as np


LAW_ID = "phi-hydro-parity-field-sector-1"
MAGIC = b"FTDHP1\x00"
LIFTED_VELOCITIES = tuple(v for v in product((-1, 0, 1), repeat=4)
                          if sum(x * x for x in v) == 2)
VELOCITIES = tuple(v[:3] for v in LIFTED_VELOCITIES)
FULL_MASK = (1 << 24) - 1
_LABEL_INDEX = {v: i for i, v in enumerate(LIFTED_VELOCITIES)}
R4_ACTION = tuple(_LABEL_INDEX[v[:3] + (-v[3],)] for v in LIFTED_VELOCITIES)
_INTEGER_TYPES = frozenset((int, np.int8, np.int16, np.int32, np.int64,
                            np.uint8, np.uint16, np.uint32, np.uint64,
                            np.intp, np.uintp, np.longlong, np.ulonglong))


def _integer(value, minimum, name):
    if not any(type(value) is allowed for allowed in _INTEGER_TYPES) or value < minimum:
        raise ValueError(f"{name} must be an integer >= {minimum}")
    return int(value)


def _mask(value):
    value = _integer(value, 0, "mask")
    if value > FULL_MASK:
        raise ValueError("mask exceeds the 24-channel alphabet")
    return value


def transform_mask(mask: int, action: tuple[int, ...]) -> int:
    """Induced occupancy permutation, for internal declared channel actions."""
    return sum(((mask >> i) & 1) << j for i, j in enumerate(action))


def label_momentum(mask: int) -> tuple[int, int, int, int]:
    mask = _mask(mask)
    return tuple(sum(v[j] for i, v in enumerate(LIFTED_VELOCITIES) if mask >> i & 1)
                 for j in range(4))


def momentum(mask: int) -> tuple[int, int, int]:
    return label_momentum(mask)[:3]


def collide_mask(mask: int) -> int:
    mask = _mask(mask)
    count = mask.bit_count()
    if count % 2:
        return transform_mask(mask, R4_ACTION)
    if count == 12 and label_momentum(mask) == (0, 0, 0, 0):
        return FULL_MASK ^ mask
    return mask


def _half_classes(offset: int) -> dict:
    classes = {}
    for mask in range(1 << 12):
        p = tuple(sum(LIFTED_VELOCITIES[offset + i][j] for i in range(12) if mask >> i & 1)
                  for j in range(4))
        classes.setdefault((mask.bit_count(), *p), []).append(mask)
    return classes


@lru_cache(maxsize=1)
def eligible_masks() -> tuple[int, ...]:
    """Exactly the N=12, full-four-momentum-zero complement sector."""
    left, right = _half_classes(0), _half_classes(12)
    result = []
    for (count, *p), masks in left.items():
        partners = right.get((12 - count, *(-x for x in p)), ())
        result.extend(a | (b << 12) for a in masks for b in partners)
    return tuple(sorted(result))


@lru_cache(maxsize=1)
def collision_gram() -> tuple[tuple[int, ...], ...]:
    """Q4 sums every eligible complement state, not just one of each pair."""
    masks = np.asarray(eligible_masks(), dtype=np.uint32)
    occupied = ((masks[:, None] >> np.arange(24, dtype=np.uint32)) & 1).astype(np.int64)
    delta = 1 - 2 * occupied
    gram = delta.T @ delta
    return tuple(tuple(int(x) for x in row) for row in gram)


def reference_coefficients(p: Fraction) -> tuple[Fraction, Fraction]:
    if not isinstance(p, Fraction) or not 0 < p < 1:
        raise ValueError("p must be a Fraction strictly between zero and one")
    return p ** 11 * (1 - p) ** 11 / 2, (1 + (1 - 2 * p) ** 22) / 2


def marginal_jacobian(p: Fraction) -> tuple[tuple[Fraction, ...], ...]:
    """Exact one-collision product-prepared derivative; not a closure claim."""
    c, b = reference_coefficients(p)
    return tuple(tuple(Fraction(i == j) - c * q
                       + b * (int(i == R4_ACTION[j]) - int(i == j))
                       for j, q in enumerate(row))
                 for i, row in enumerate(collision_gram()))


def _symmetry_actions() -> tuple:
    """Physical cube group, full signed-four-axis group, and one triality action."""
    physical = tuple(tuple(_LABEL_INDEX[tuple(signs[j] * v[axes[j]] for j in range(3))
                                       + (v[3],)] for v in LIFTED_VELOCITIES)
                     for axes in permutations(range(3)) for signs in product((-1, 1), repeat=3))
    signed4 = tuple(tuple(_LABEL_INDEX[tuple(signs[j] * v[axes[j]] for j in range(4))]
                         for v in LIFTED_VELOCITIES)
                    for axes in permutations(range(4)) for signs in product((-1, 1), repeat=4))
    hadamard = ((1, 1, 1, 1), (1, 1, -1, -1), (1, -1, 1, -1), (1, -1, -1, 1))
    images = [tuple(sum(row[j] * v[j] for j in range(4)) for row in hadamard)
              for v in LIFTED_VELOCITIES]
    if any(any(x % 2 for x in image) for image in images):
        raise AssertionError("triality action left the integer-label space")
    triality = tuple(_LABEL_INDEX[tuple(x // 2 for x in image)] for image in images)
    return physical, signed4, triality


def stress_weights() -> tuple[tuple[int, ...], ...]:
    return (tuple(tuple(v[i] * v[j] for v in LIFTED_VELOCITIES)
                  for i, j in combinations(range(4), 2))
            + tuple(tuple(v[i] ** 2 - v[3] ** 2 for v in LIFTED_VELOCITIES)
                    for i in range(3)))


def certificate() -> dict:
    masks = eligible_masks()
    members = set(masks)
    gram = collision_gram()
    Q = flint.fmpz_mat(gram)
    S = flint.fmpz_mat([[int(i == j) - int(i == R4_ACTION[j]) for j in range(24)]
                        for i in range(24)])
    label_weights = flint.fmpz_mat([(1, *v) for v in LIFTED_VELOCITIES])
    physical_weights = flint.fmpz_mat([(1, *v) for v in VELOCITIES])
    rank4, rank_total = int(Q.rank()), int((Q + S).rank())
    zero5, zero4 = flint.fmpz_mat(24, 5), flint.fmpz_mat(24, 4)
    full_kernel = Q * label_weights == zero5 and rank4 == 19
    physical_kernel = (Q + S) * physical_weights == zero4 and rank_total == 20
    physical, signed4, triality = _symmetry_actions()
    source = np.asarray(masks, dtype=np.uint32)
    covariance_failures = 0
    for action in signed4 + (triality,):
        mapped = np.zeros(len(source), dtype=np.uint32)
        for i, j in enumerate(action):
            mapped |= ((source >> i) & 1) << j
        covariance_failures += int(np.count_nonzero(~np.isin(mapped, source, assume_unique=True)))
    reflection_commutators = sum(
        action[R4_ACTION[i]] != R4_ACTION[action[i]] for action in physical for i in range(24))
    stresses = stress_weights()
    images = [[sum(q * x for q, x in zip(row, weight)) for row in gram] for weight in stresses]
    first = next(i for i, x in enumerate(stresses[0]) if x)
    eigenvalue = Fraction(images[0][first], stresses[0][first])
    stress_failures = sum(out[i] != eigenvalue * weight[i]
                          for weight, out in zip(stresses, images) for i in range(24))
    spatial_stresses = tuple(tuple(v[i] * v[j] - Fraction(i == j, 2) for v in VELOCITIES)
                             for i in range(3) for j in range(i, 3))
    spatial_reflection_failures = sum(w[i] != w[R4_ACTION[i]]
                                      for w in spatial_stresses for i in range(24))
    # The odd branch is not assumed fixed-point-free; singleton swaps and
    # the entirely fixed mass-two sector supply separate exact witnesses.
    one_and_two = (tuple(1 << i for i in range(24))
                   + tuple((1 << i) | (1 << j) for i, j in combinations(range(24), 2)))
    small_involution_failures = sum(collide_mask(collide_mask(a)) != a for a in one_and_two)
    pair_identity_failures = sum(collide_mask(a) != a for a in one_and_two if a.bit_count() == 2)
    c, b = reference_coefficients(Fraction(1, 2))
    packed = b"".join(mask.to_bytes(3, "little") for mask in masks)
    return {
        "schema": "strict-hydro-parity-finite-certificate-v1", "law_id": LAW_ID,
        "velocity_labels": [list(v) for v in LIFTED_VELOCITIES],
        "reflection_action": list(R4_ACTION), "eligible_subsets": len(masks),
        "eligible_sha256": hashlib.sha256(packed).hexdigest(),
        "complement_closure": all(FULL_MASK ^ a in members for a in masks),
        "all_eligible_mass_momentum4": all(a.bit_count() == 12 and label_momentum(a) == (0, 0, 0, 0)
                                          for a in masks),
        "signed_four_axis_actions": len(signed4), "triality_actions_checked": 1,
        "complement_symmetry_failures": covariance_failures,
        "physical_cube_actions": len(physical), "physical_reflection_commutator_failures": reflection_commutators,
        "full_law_full_four_dimensional_covariance": False,
        "complement_gram_rank": rank4, "complement_five_weight_gate": full_kernel,
        "combined_constraint_rank": rank_total, "four_fixed_channel_weight_gate": physical_kernel,
        "traceless_stress_weights": len(stresses), "stress_eigenvalue": str(eigenvalue),
        "stress_scalar_failures": stress_failures,
        "spatial_stress_reflection_failures": spatial_reflection_failures,
        "small_sector_involution_failures": small_involution_failures,
        "all_mass_two_states_fixed": pair_identity_failures == 0,
        "eligibility_at_half": str(Fraction(len(masks), 1 << 24)),
        "parity_coefficient_at_half": str(b), "fourth_momentum_multiplier_at_half": str(1 - 2 * b),
        "stress_relaxation_per_cycle_at_half": str(c * eigenvalue),
        "inverse_stress_relaxation_cycles_at_half": str(1 / (c * eigenvalue)),
        "unit_modulus_scope": "0<p<1; exact product marginal Jacobian only; "
                              "four +1 weights, no -1 weight because vacuum and all pairs are fixed",
        "limits": {"microscopic_closure": False, "continuum_recovered": False,
                   "canonical_adoption": False, "phase_absorption_relation_embedding": False},
    }


@dataclass(frozen=True)
class FieldState:
    L: int
    microtick: int
    bank: np.ndarray
    law_id: str = LAW_ID

    @property
    def phase(self):
        return self.microtick % 2


def validate(state: FieldState) -> None:
    if type(state) is not FieldState or type(state.law_id) is not str or state.law_id != LAW_ID:
        raise ValueError("foreign parity field-sector state/law")
    L = _integer(state.L, 3, "L")
    _integer(state.microtick, 0, "microtick")
    bank = state.bank
    if (type(bank) is not np.ndarray or bank.dtype != np.dtype(bool)
            or bank.shape != (L, L, L, 2, 24) or not bank.flags.c_contiguous):
        raise ValueError("bank must be a contiguous Boolean LxLxLx2x24 array")
    if np.any(bank.view(np.uint8) > 1):
        raise ValueError("noncanonical Boolean bytes")


def initialize(bank: np.ndarray) -> FieldState:
    if type(bank) is not np.ndarray or bank.ndim != 5:
        raise ValueError("expected LxLxLx2x24 bank")
    state = FieldState(bank.shape[0], 0, bank)
    validate(state)
    return FieldState(state.L, 0, bank.copy())


def inventories(state: FieldState) -> tuple[tuple[int, ...], ...]:
    validate(state)
    counts = state.bank.sum(axis=(0, 1, 2), dtype=np.int64)
    return tuple((int(row.sum()), *(int(v) for v in row @ np.asarray(VELOCITIES, dtype=np.int64)))
                 for row in counts)


def step(state: FieldState) -> FieldState:
    validate(state)
    bank = state.bank
    if state.phase == 0:
        count = bank.sum(axis=-1, dtype=np.int16)
        p4 = bank.astype(np.int16) @ np.asarray(LIFTED_VELOCITIES, dtype=np.int16)
        complement = (count == 12) & np.all(p4 == 0, axis=-1)
        out = np.where((count % 2 == 1)[..., None], np.take(bank, R4_ACTION, axis=-1), bank)
        np.logical_xor(out, complement[..., None], out=out)
    else:
        out = np.empty_like(bank)
        for channel, velocity in enumerate(VELOCITIES):
            out[..., channel] = np.roll(bank[..., channel], shift=velocity, axis=(0, 1, 2))
    result = FieldState(int(state.L), int(state.microtick) + 1, out)
    validate(result)
    return result


def checkpoint(state: FieldState) -> bytes:
    validate(state)
    header = json.dumps({"L": int(state.L), "law": LAW_ID,
                         "tick_hex": format(int(state.microtick), "x")},
                        sort_keys=True, separators=(",", ":")).encode("ascii")
    body = len(header).to_bytes(4, "little") + header + np.packbits(state.bank, bitorder="little").tobytes()
    return MAGIC + hashlib.sha256(body).digest() + body


def restore(data: bytes) -> FieldState:
    if type(data) is not bytes or not data.startswith(MAGIC) or len(data) < len(MAGIC) + 36:
        raise ValueError("foreign or malformed parity field-sector checkpoint")
    digest = data[len(MAGIC):len(MAGIC) + 32]
    body = data[len(MAGIC) + 32:]
    if hashlib.sha256(body).digest() != digest:
        raise ValueError("checkpoint checksum mismatch")
    size = int.from_bytes(body[:4], "little")
    if size > len(body) - 4:
        raise ValueError("checkpoint header length")
    try:
        header = json.loads(body[4:4 + size].decode("ascii"))
        if type(header) is not dict or set(header) != {"L", "tick_hex", "law"} or header["law"] != LAW_ID:
            raise ValueError("foreign checkpoint header")
        L = _integer(header["L"], 3, "L")
        encoded_tick = header["tick_hex"]
        if (type(encoded_tick) is not str or not encoded_tick
                or any(c not in "0123456789abcdef" for c in encoded_tick)):
            raise ValueError("checkpoint tick must be lowercase hexadecimal")
        tick = int(encoded_tick, 16)
        if format(tick, "x") != encoded_tick:
            raise ValueError("noncanonical checkpoint tick")
        canonical = json.dumps(header, sort_keys=True, separators=(",", ":")).encode("ascii")
        if canonical != body[4:4 + size]:
            raise ValueError("noncanonical checkpoint header")
    except (UnicodeError, TypeError, KeyError, json.JSONDecodeError) as error:
        raise ValueError("malformed checkpoint header") from error
    payload = body[4 + size:]
    if len(payload) != 6 * L ** 3:
        raise ValueError("checkpoint payload size")
    bank = np.unpackbits(np.frombuffer(payload, dtype=np.uint8), bitorder="little").astype(bool)
    result = FieldState(L, tick, bank.reshape(L, L, L, 2, 24))
    validate(result)
    return result
