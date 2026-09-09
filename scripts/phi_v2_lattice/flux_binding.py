"""Selected ternary-flux / hop-credit research candidate, not physical matter.

The complete local decision rule is frozen in
engine/docs/SPEC_STRICT_FLUX_BINDING_CANDIDATE_V1.md. This module changes none
of the original staged, alignment or mixed-sector sources or identities.
"""
from __future__ import annotations

import base64
from dataclasses import dataclass, field
import hashlib
from itertools import product
import json
from numbers import Integral

import numpy as np

LAW_ID = "phi-flux-hop-credit-candidate-1"
BACKEND_ID = "python-flux-hop-credit-1"
SCHEMA = "ftd-flux-hop-credit-checkpoint-1"
MAGIC = b"FTD-FLUX-HOP-CREDIT-1\n"
ACCOUNT_ID = "flux-hop-credit-count"
NAMES = ("direction", "credit", "flux")
DIRECTIONS = ((1, 0, 0), (-1, 0, 0), (0, 1, 0), (0, -1, 0), (0, 0, 1), (0, 0, -1))
RULE_DESCRIPTION = {
    "law": LAW_ID, "alphabet": "two slots: empty or six directions times credit bit; SC flux ternary",
    "directions": DIRECTIONS, "domain": "periodic even integer L>=4; div q=n_plus-n_minus",
    "schedule": "p=t%12; polarity=+ if p<6 else -; axis=(p%6)//2; tail coordinate parity=p%2",
    "capacity": "both occupied or both empty hold; lone slot must point across active edge",
    "hop": "q_new=q-epsilon*eta; Delta=q_new^2-q^2; k_new=k-Delta; accept iff both alphabets valid",
    "denied": "positions/credit/flux hold; if k=0 and unique outward-flux=epsilon port use it, else reverse v",
    "account": ACCOUNT_ID, "empty_credit": 0, "pending": "none; disjoint edges, complete pre-state reads",
}
RULE_HASH = hashlib.sha256(json.dumps(RULE_DESCRIPTION, sort_keys=True, separators=(",", ":")).encode()).hexdigest()
ENCODING_HASH = hashlib.sha256(repr((NAMES, DIRECTIONS, "uint8,uint8,int8", "row-major-xyz")).encode()).hexdigest()


@dataclass(frozen=True)
class FluxState:
    L: int
    microtick: int
    direction: np.ndarray
    credit: np.ndarray
    flux: np.ndarray
    law_id: str = field(default=LAW_ID, init=False)
    rule_hash: str = field(default=RULE_HASH, init=False)
    encoding_hash: str = field(default=ENCODING_HASH, init=False)
    boundary: str = field(default="periodic", init=False)

    @property
    def phase(self):
        return self.microtick % 12


@dataclass
class FluxEvents:
    # (polarity, source, destination, edge_owner, axis, q_before, q_after, k_before, k_after)
    moves: list = field(default_factory=list)
    # (polarity, site, direction_before, direction_after, rejection_reason)
    redirects: list = field(default_factory=list)
    # (polarity, edge_owner, axis): both active-polarity endpoint slots occupied
    capacity_holds: list = field(default_factory=list)


def _integer(value, name, minimum=0):
    if isinstance(value, bool) or not isinstance(value, Integral) or value < minimum:
        raise ValueError(f"invalid {name}")
    return int(value)


def _size(L):
    L = _integer(L, "periodic size", 4)
    if L % 2:
        raise ValueError("matching schedule requires an even periodic size")
    return L


def _shift(L, site, axis, sign):
    stride = L ** (2 - axis)
    coordinate = (int(site) // stride) % L
    return int(site) + (((coordinate + sign) % L) - coordinate) * stride


def _divergence(flux, L):
    cube = flux.reshape(L, L, L, 3).astype(np.int16)
    result = np.zeros((L, L, L), dtype=np.int16)
    for axis in range(3):
        result += cube[..., axis] - np.roll(cube[..., axis], 1, axis=axis)
    return result.reshape(-1)


def _validate_payload(L, direction, credit, flux):
    L = _size(L)
    n = L ** 3
    values = (direction, credit, flux)
    for name, value, shape, dtype, low, high in (
        ("direction", direction, (n, 2), "uint8", 0, 6),
        ("credit", credit, (n, 2), "uint8", 0, 1),
        ("flux", flux, (n, 3), "int8", -1, 1),
    ):
        if not isinstance(value, np.ndarray) or value.shape != shape or value.dtype != np.dtype(dtype):
            raise ValueError(f"invalid {name} array shape or dtype")
        if np.any(value < low) or np.any(value > high):
            raise ValueError(f"{name} outside finite alphabet")
    if any(np.shares_memory(values[i], values[j]) for i in range(3) for j in range(i)):
        raise ValueError("complete-state arrays must have disjoint storage")
    if np.any(credit[direction == 0] != 0):
        raise ValueError("empty slot must have zero hop credit")
    charge = (direction[:, 0] != 0).astype(np.int16) - (direction[:, 1] != 0).astype(np.int16)
    if not np.array_equal(_divergence(flux, L), charge):
        raise ValueError("flux divergence does not match polarity occupancy")


def validate(state):
    if type(state) is not FluxState:
        raise ValueError("expected FluxState; no implicit cross-law conversion")
    for name, expected in (("law_id", LAW_ID), ("rule_hash", RULE_HASH),
                           ("encoding_hash", ENCODING_HASH), ("boundary", "periodic")):
        if getattr(state, name) != expected:
            raise ValueError(f"incompatible flux state {name}")
    _integer(state.microtick, "microtick")
    _validate_payload(state.L, state.direction, state.credit, state.flux)


def initialize(L, direction=None, credit=None, flux=None):
    """Fresh owned preparation at ordinal zero; inputs must be jointly legal."""
    L = _size(L)
    if direction is None and credit is None and flux is None:
        direction = np.zeros((L ** 3, 2), dtype=np.uint8)
        credit = np.zeros((L ** 3, 2), dtype=np.uint8)
        flux = np.zeros((L ** 3, 3), dtype=np.int8)
    _validate_payload(L, direction, credit, flux)
    result = FluxState(L, 0, direction.copy(), credit.copy(), flux.copy())
    validate(result)
    return result


def populations(state):
    validate(state)
    return tuple(int(np.count_nonzero(state.direction[:, slot])) for slot in range(2))


def account_units(state):
    """Conserved integer flux-hop-credit count; not physical energy or mass."""
    validate(state)
    return int(np.abs(state.flux).sum()) + int(state.credit.sum())


def manifestation(state):
    validate(state)
    return (state.direction[:, 0] != 0).astype(np.int8) - (state.direction[:, 1] != 0).astype(np.int8)


def _reverse(direction):
    return direction + 1 if direction % 2 else direction - 1


def _guide(outward, epsilon, credit, direction):
    ports = [i + 1 for i, value in enumerate(outward) if value == epsilon]
    return ports[0] if credit == 0 and len(ports) == 1 else _reverse(direction)


def _outward(state, site):
    values = []
    for axis in range(3):
        values.extend((int(state.flux[site, axis]),
                       -int(state.flux[_shift(state.L, site, axis, -1), axis])))
    return tuple(values)


def step(state):
    """One charged disjoint-matching tick, pure on its complete input arrays."""
    validate(state)
    L = int(state.L)
    phase = int(state.phase)
    slot = phase // 6
    epsilon = 1 if slot == 0 else -1
    axis, parity = (phase % 6) // 2, phase % 2
    direction, credit, flux = (getattr(state, name).copy() for name in NAMES)
    events = FluxEvents()
    stride = L ** (2 - axis)
    for owner in range(L ** 3):
        if (owner // stride) % L % 2 != parity:
            continue
        head = _shift(L, owner, axis, 1)
        left, right = int(state.direction[owner, slot]), int(state.direction[head, slot])
        if left and right:
            events.capacity_holds.append((epsilon, owner, axis))
            continue
        if not left and not right:
            continue
        source, destination, eta, v = (owner, head, 1, left) if left else (head, owner, -1, right)
        if v != 2 * axis + (1 if eta == 1 else 2):
            continue
        q, k = int(state.flux[owner, axis]), int(state.credit[source, slot])
        q_next = q - epsilon * eta
        delta = q_next * q_next - q * q
        k_next = k - delta
        if -1 <= q_next <= 1 and 0 <= k_next <= 1:
            direction[source, slot] = 0
            credit[source, slot] = 0
            direction[destination, slot] = v
            credit[destination, slot] = k_next
            flux[owner, axis] = q_next
            events.moves.append((epsilon, source, destination, owner, axis, q, q_next, k, k_next))
        else:
            # Only this source direction depends on its other incident flux.
            # Destination and edge outputs above never consult this star.
            redirected = _guide(_outward(state, source), epsilon, k, v)
            direction[source, slot] = redirected
            reason = "flux_capacity" if abs(q_next) > 1 else "credit_deficit" if k_next < 0 else "credit_capacity"
            events.redirects.append((epsilon, source, v, redirected, reason))
    result = FluxState(L, int(state.microtick) + 1, direction, credit, flux)
    validate(result)
    return result, events


def checkpoint(state):
    validate(state)
    arrays = {name: base64.b64encode(getattr(state, name).tobytes(order="C")).decode("ascii") for name in NAMES}
    payload = dict(schema=SCHEMA, law=LAW_ID, rule=RULE_HASH, encoding=ENCODING_HASH,
                   backend=BACKEND_ID, boundary="periodic", L=int(state.L),
                   microtick_hex=format(int(state.microtick), "x"), arrays=arrays)
    return MAGIC + json.dumps(payload, sort_keys=True, separators=(",", ":")).encode()


def _unique(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError("duplicate checkpoint key")
        result[key] = value
    return result


def restore(data):
    try:
        if not isinstance(data, bytes) or not data.startswith(MAGIC):
            raise ValueError("incompatible flux checkpoint magic")
        payload = json.loads(data[len(MAGIC):], object_pairs_hook=_unique)
        expected = {"schema", "law", "rule", "encoding", "backend", "boundary", "L", "microtick_hex", "arrays"}
        if not isinstance(payload, dict) or set(payload) != expected:
            raise ValueError("invalid flux checkpoint schema fields")
        for key, expected_value in (("schema", SCHEMA), ("law", LAW_ID), ("rule", RULE_HASH),
                                     ("encoding", ENCODING_HASH), ("backend", BACKEND_ID), ("boundary", "periodic")):
            if payload[key] != expected_value:
                raise ValueError(f"incompatible flux checkpoint {key}")
        L = _size(payload["L"])
        ordinal = payload["microtick_hex"]
        if (not isinstance(ordinal, str) or not ordinal or (len(ordinal) > 1 and ordinal[0] == "0")
                or any(c not in "0123456789abcdef" for c in ordinal)):
            raise ValueError("invalid canonical hexadecimal ordinal")
        tick = int(ordinal, 16)
        encoded = payload["arrays"]
        if not isinstance(encoded, dict) or set(encoded) != set(NAMES):
            raise ValueError("checkpoint requires all complete arrays")
        arrays = []
        for name, columns, dtype in (("direction", 2, "uint8"), ("credit", 2, "uint8"), ("flux", 3, "int8")):
            size = L ** 3 * columns
            value = encoded[name]
            if not isinstance(value, str) or len(value) != 4 * ((size + 2) // 3):
                raise ValueError(f"invalid {name} encoded length")
            raw = base64.b64decode(value, validate=True)
            if len(raw) != size or base64.b64encode(raw).decode("ascii") != value:
                raise ValueError(f"invalid {name} canonical encoding")
            arrays.append(np.frombuffer(raw, dtype=dtype).reshape(L ** 3, columns).copy())
        result = FluxState(L, tick, *arrays)
        validate(result)
        return result
    except (TypeError, KeyError, UnicodeError, OverflowError) as exc:
        raise ValueError("malformed flux checkpoint") from exc


def local_account_certificate():
    """Exhaust local hop alphabets and guide stars; not a trajectory certificate."""
    accepted = refused = guide_cases = 0
    for epsilon, eta, q, k in product((-1, 1), (-1, 1), (-1, 0, 1), (0, 1)):
        after = q - epsilon * eta
        delta = after * after - q * q
        credit = k - delta
        if abs(after) <= 1 and 0 <= credit <= 1:
            if after * after + credit != q * q + k or eta * (after - q) != -epsilon:
                raise ValueError("local count/divergence identity failed")
            accepted += 1
        else:
            refused += 1
    for star, epsilon, credit, direction in product(product((-1, 0, 1), repeat=6), (-1, 1), (0, 1), range(1, 7)):
        result = _guide(star, epsilon, credit, direction)
        ports = [i + 1 for i, value in enumerate(star) if value == epsilon]
        expected = ports[0] if not credit and len(ports) == 1 else (direction + 1 if direction % 2 else direction - 1)
        if result != expected or not 1 <= result <= 6:
            raise ValueError("guide alphabet/selection mismatch")
        guide_cases += 1
    return dict(law=LAW_ID, rule_hash=RULE_HASH, account=ACCOUNT_ID,
                hop_cases=accepted + refused, accepted=accepted, refused=refused,
                guide_cases=guide_cases, physical_energy_identified=False,
                full_restoring_quotient_enumerated=False)
