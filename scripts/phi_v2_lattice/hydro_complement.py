"""Selected 24-channel complement field sector; not the production Phi law.

The specification is SPEC_STRICT_HYDRO_COMPLEMENT_CANDIDATE_V1.md. Exact
collision/accounting certificates do not certify a hydrodynamic closure.
"""
from __future__ import annotations

from dataclasses import dataclass
from fractions import Fraction
from functools import lru_cache
from itertools import combinations, permutations, product
import hashlib
import json
from numbers import Integral

import flint
import numpy as np

LAW_ID = "phi-hydro-complement-field-sector-1"
MAGIC = b"FTDHC1\x00"
LIFTED_VELOCITIES = tuple(sorted(
    tuple(signs[axes.index(i)] if i in axes else 0 for i in range(4))
    for axes in combinations(range(4), 2) for signs in product((-1, 1), repeat=2)
))
VELOCITIES = tuple(v[:3] for v in LIFTED_VELOCITIES)
FULL_MASK = (1 << 24) - 1


def _integer(value, minimum, name):
    if isinstance(value, bool) or not isinstance(value, Integral) or value < minimum:
        raise ValueError(f"{name} must be an integer >= {minimum}")
    return int(value)


def momentum(mask: int) -> tuple[int, int, int]:
    mask = _integer(mask, 0, "mask")
    if mask > FULL_MASK:
        raise ValueError("mask exceeds the 24-channel alphabet")
    return tuple(sum(v[j] for i, v in enumerate(VELOCITIES) if mask & (1 << i))
                 for j in range(3))


def collide_mask(mask: int) -> int:
    p = momentum(mask)
    mask = int(mask)
    return FULL_MASK ^ mask if mask.bit_count() == 12 and p == (0, 0, 0) else mask


def _half_classes(offset):
    classes = {}
    for mask in range(1 << 12):
        count = mask.bit_count()
        p = tuple(sum(VELOCITIES[offset + i][j] for i in range(12)
                      if mask & (1 << i)) for j in range(3))
        classes.setdefault((count, *p), []).append(mask)
    return classes


@lru_cache(maxsize=1)
def eligible_masks() -> tuple[int, ...]:
    """All and only N=12, P=0 subsets, via exhaustive 4096+4096 halves."""
    left, right = _half_classes(0), _half_classes(12)
    result = []
    for (count, x, y, z), masks in left.items():
        partners = right.get((12 - count, -x, -y, -z), ())
        result.extend(a | (b << 12) for a in masks for b in partners)
    return tuple(sorted(result))


@lru_cache(maxsize=1)
def collision_gram() -> tuple[tuple[int, ...], ...]:
    masks = np.asarray(eligible_masks(), dtype=np.uint32)
    a = ((masks[:, None] >> np.arange(24, dtype=np.uint32)) & 1).astype(np.int64)
    delta = 1 - 2 * a
    # Every summand has entries +/-1; fewer than 2^24 rows prevent int64 overflow.
    gram = delta.T @ delta
    return tuple(tuple(int(v) for v in row) for row in gram)


def marginal_jacobian(p: Fraction) -> tuple[tuple[Fraction, ...], ...]:
    """Exact one-collision marginal derivative; not an autonomous closure."""
    if not isinstance(p, Fraction) or not 0 < p < 1:
        raise ValueError("p must be an exact Fraction strictly between zero and one")
    factor = p ** 11 * (1 - p) ** 11 / 2
    return tuple(tuple(Fraction(i == j) - factor * q for j, q in enumerate(row))
                 for i, row in enumerate(collision_gram()))


def certificate() -> dict:
    masks = eligible_masks()
    members = set(masks)
    gram = collision_gram()
    matrix = flint.fmpz_mat(gram)
    rank = int(matrix.rank())
    weights = [(1, *v) for v in VELOCITIES]
    residual = matrix * flint.fmpz_mat(weights)
    index = {v: i for i, v in enumerate(LIFTED_VELOCITIES)}
    # Check the multiset action, including the retained fourth-coordinate sign.
    actions = []
    for axes in permutations(range(3)):
        for signs in product((-1, 1), repeat=3):
            actions.append(tuple(index[tuple(signs[j] * v[axes[j]] for j in range(3))
                                       + (v[3],)] for v in LIFTED_VELOCITIES))
    source = np.asarray(masks, dtype=np.uint32)
    symmetry_failures = 0
    for action in actions:
        mapped = np.zeros(len(source), dtype=np.uint32)
        for i, j in enumerate(action):
            mapped |= ((source >> i) & 1) << j
        symmetry_failures += int(np.count_nonzero(~np.isin(mapped, source, assume_unique=True)))
    t2 = [[sum(v[i] * v[j] for v in VELOCITIES) for j in range(3)] for i in range(3)]
    t4 = [[[[sum(v[i] * v[j] * v[k] * v[l] for v in VELOCITIES)
              for l in range(3)] for k in range(3)] for j in range(3)] for i in range(3)]
    isotropy_failures = sum(t4[i][j][k][l] != 4 * (
        (i == j) * (k == l) + (i == k) * (j == l) + (i == l) * (j == k))
        for i, j, k, l in product(range(3), repeat=4))
    packed = b"".join(mask.to_bytes(3, "little") for mask in masks)
    zero = all(residual[i, j] == 0 for i in range(24) for j in range(4))
    return {
        "schema": "strict-hydro-complement-finite-certificate-v1", "law_id": LAW_ID,
        "velocity_labels": [list(v) for v in LIFTED_VELOCITIES],
        "eligible_subsets": len(masks), "eligible_sha256": hashlib.sha256(packed).hexdigest(),
        "complement_closure": all(FULL_MASK ^ mask in members for mask in masks),
        "all_eligible_mass_momentum": all(mask.bit_count() == 12 and momentum(mask) == (0, 0, 0)
                                          for mask in masks),
        "signed_cubic_actions": len(actions), "symmetry_failures": symmetry_failures,
        "gram_rank": rank, "fixed_invariant_dimension": 24 - rank,
        "population_momentum_kernel": zero,
        "four_invariant_gate": zero and rank == 20,
        "second_moment": t2, "fourth_rank_isotropy_failures": isotropy_failures,
        "eligibility_at_half": str(Fraction(len(masks), 1 << 24)),
        "marginal_jacobian_factor_at_half": str(Fraction(1, 1 << 23)),
        "limits": {"continuum_closure": False, "material_recovery": False,
                   "canonical_adoption": False, "backend": "Python finite field-sector reference",
                   "phase_absorption_relation_embedding": False},
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
    if type(state) is not FieldState or state.law_id != LAW_ID:
        raise ValueError("foreign field-sector state/law")
    L = _integer(state.L, 3, "L")
    _integer(state.microtick, 0, "microtick")
    bank = state.bank
    if (not isinstance(bank, np.ndarray) or bank.dtype != np.dtype(bool)
            or bank.shape != (L, L, L, 2, 24) or not bank.flags.c_contiguous):
        raise ValueError("bank must be a contiguous Boolean LxLxLx2x24 array")
    if np.any(bank.view(np.uint8) > 1):
        raise ValueError("noncanonical Boolean bytes")


def initialize(bank: np.ndarray) -> FieldState:
    if not isinstance(bank, np.ndarray) or bank.ndim != 5:
        raise ValueError("expected LxLxLx2x24 bank")
    candidate = FieldState(bank.shape[0], 0, bank)
    validate(candidate)
    return FieldState(candidate.L, 0, bank.copy())


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
        p = bank.astype(np.int16) @ np.asarray(VELOCITIES, dtype=np.int16)
        eligible = (count == 12) & np.all(p == 0, axis=-1)
        out = np.logical_xor(bank, eligible[..., None])
    else:
        out = np.empty_like(bank)
        for c, velocity in enumerate(VELOCITIES):
            out[..., c] = np.roll(bank[..., c], shift=velocity, axis=(0, 1, 2))
    result = FieldState(int(state.L), int(state.microtick) + 1, out)
    validate(result)
    return result


def checkpoint(state: FieldState) -> bytes:
    validate(state)
    header = json.dumps({"L": int(state.L), "tick_hex": format(int(state.microtick), "x"), "law": LAW_ID},
                        sort_keys=True, separators=(",", ":")).encode("ascii")
    payload = np.packbits(state.bank, bitorder="little").tobytes()
    body = len(header).to_bytes(4, "little") + header + payload
    return MAGIC + hashlib.sha256(body).digest() + body


def restore(data: bytes) -> FieldState:
    if type(data) is not bytes or not data.startswith(MAGIC) or len(data) < len(MAGIC) + 36:
        raise ValueError("foreign or malformed field-sector checkpoint")
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
        # Roundtrip rejects duplicate keys, extra whitespace and alternate JSON spellings.
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
