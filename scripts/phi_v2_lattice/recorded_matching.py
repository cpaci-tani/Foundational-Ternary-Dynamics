"""Finite recorded-matching successor; no physical/canonical identification.

The complete pre-implementation contract is SPEC_STRICT_RECORDED_MATCHING_V1.md.
The frozen flux decisions remain unchanged. The matching is selected from an
explicit local signed triad, never reconstructed from coordinate parity.
"""
from __future__ import annotations

import base64
from dataclasses import dataclass, field
import hashlib
from itertools import permutations
import json
from types import MappingProxyType

import numpy as np

from . import flux_binding as _reference


LAW_ID = "phi-flux-recorded-matching-candidate-1"
BACKEND_ID = "python-flux-recorded-matching-1"
SCHEMA = "ftd-flux-recorded-matching-checkpoint-1"
MAGIC = b"FTD-FLUX-RECORDED-MATCHING-1\n"
ACCOUNT_ID = "flux-hop-credit-count"
DIRECTIONS = _reference.DIRECTIONS
NAMES = ("direction", "credit", "flux", "matching_frame")
AXIS_PERMUTATIONS = tuple(permutations(range(3)))
FRAME_DIRECTIONS = tuple(
    tuple(2 * axes[a] + 1 + ((color >> a) & 1) for a in range(3))
    for axes in AXIS_PERMUTATIONS for color in range(8)
)
TRIADS = tuple(tuple(DIRECTIONS[d - 1] for d in row) for row in FRAME_DIRECTIONS)
FRAME_HASH = hashlib.sha256(bytes(d for row in FRAME_DIRECTIONS for d in row)).hexdigest()
_FLIP_CODES = tuple(tuple(code ^ (1 << AXIS_PERMUTATIONS[code // 8].index(axis))
                         for code in range(48)) for axis in range(3))
_LAYOUT = (("direction", 2, "uint8"), ("credit", 2, "uint8"),
           ("flux", 3, "int8"), ("matching_frame", 1, "uint8"))
RULE_DESCRIPTION = MappingProxyType({
    "law": LAW_ID, "reference_rule": _reference.RULE_HASH,
    "frames": FRAME_HASH, "background": "48 signed triads; every +/- SC neighbor flips only its axis column",
    "schedule": "p=t%12; slot=p//6; a=(p%6)//2; b=p%2; matched direction=(-1)^b B_a(x)",
    "background_output": "copy own code; no pending or cache",
    "edge_rule": "unchanged flux-hop-credit tree on every matched edge at its positive stored owner",
    "account": ACCOUNT_ID, "boundary": "periodic even integer L>=4",
})
RULE_HASH = hashlib.sha256(json.dumps(dict(RULE_DESCRIPTION), sort_keys=True,
                                    separators=(",", ":")).encode()).hexdigest()
ENCODING_HASH = hashlib.sha256(repr((NAMES, _LAYOUT, DIRECTIONS, FRAME_DIRECTIONS,
                                   "row-major-xyz")).encode()).hexdigest()
# Event records are external observations; their unchanged format has no law state.
MatchingEvents = _reference.FluxEvents


@dataclass(frozen=True)
class MatchingState:
    L: int
    microtick: int
    direction: np.ndarray
    credit: np.ndarray
    flux: np.ndarray
    matching_frame: np.ndarray
    law_id: str = field(default=LAW_ID, init=False)
    rule_hash: str = field(default=RULE_HASH, init=False)
    frame_hash: str = field(default=FRAME_HASH, init=False)
    encoding_hash: str = field(default=ENCODING_HASH, init=False)
    boundary: str = field(default="periodic", init=False)

    @property
    def phase(self):
        return self.microtick % 12


def seed_background(L, origin_code=0):
    """Explicit owned preparation; coordinate parity is used only here."""
    L = _reference._size(L)
    code = _reference._integer(origin_code, "origin triad code")
    if code >= 48:
        raise ValueError("origin triad code outside finite alphabet")
    axes = AXIS_PERMUTATIONS[code // 8]
    coordinates = np.indices((L, L, L), dtype=np.intp)
    color = np.zeros((L, L, L), dtype=np.uint8)
    for a, axis in enumerate(axes):
        color ^= ((coordinates[axis] & 1) << a).astype(np.uint8)
    return (np.uint8(code // 8 * 8) + (color ^ np.uint8(code % 8))).reshape(-1).copy()


def _validate_payload(L, direction, credit, flux, matching_frame, *, owned):
    L = _reference._size(L)
    _reference._validate_payload(L, direction, credit, flux)
    if (not isinstance(matching_frame, np.ndarray)
            or matching_frame.shape != (L ** 3,)
            or matching_frame.dtype != np.dtype("uint8")):
        raise ValueError("invalid matching_frame array shape or dtype")
    if np.any(matching_frame > 47):
        raise ValueError("matching_frame outside finite alphabet")
    arrays = (direction, credit, flux, matching_frame)
    for name, array in zip(NAMES, arrays):
        if not array.flags.c_contiguous:
            raise ValueError(f"{name} requires contiguous storage")
        if owned and not array.flags.owndata:
            raise ValueError(f"{name} requires owned state storage")
    if any(np.shares_memory(arrays[i], arrays[j]) for i in range(4) for j in range(i)):
        raise ValueError("complete-state arrays must have disjoint storage")
    cube = matching_frame.reshape(L, L, L)
    for axis in range(3):
        expected = np.asarray(_FLIP_CODES[axis], dtype=np.uint8)[cube]
        for sign in (-1, 1):
            if not np.array_equal(np.roll(cube, sign, axis=axis), expected):
                raise ValueError("inconsistent both-sign matching-frame neighbors")


def validate(state):
    if type(state) is not MatchingState:
        raise ValueError("expected MatchingState; no implicit cross-law conversion")
    for name, expected in (("law_id", LAW_ID), ("rule_hash", RULE_HASH),
                           ("frame_hash", FRAME_HASH), ("encoding_hash", ENCODING_HASH),
                           ("boundary", "periodic")):
        if getattr(state, name) != expected:
            raise ValueError(f"incompatible matching state {name}")
    _reference._integer(state.microtick, "microtick")
    _validate_payload(state.L, state.direction, state.credit, state.flux,
                      state.matching_frame, owned=True)


def initialize(L, direction, credit, flux, matching_frame):
    """Fresh ordinal-zero state; every payload array must be explicitly supplied."""
    L = _reference._size(L)
    _validate_payload(L, direction, credit, flux, matching_frame, owned=False)
    state = MatchingState(L, 0, *(array.copy() for array in
                                 (direction, credit, flux, matching_frame)))
    validate(state)
    return state


def populations(state):
    validate(state)
    return tuple(int(np.count_nonzero(state.direction[:, slot])) for slot in range(2))


def account_units(state):
    """The exact flux-hop-credit count; no physical-energy identification."""
    validate(state)
    return int(np.abs(state.flux).sum()) + int(state.credit.sum())


def manifestation(state):
    validate(state)
    return (state.direction[:, 0] != 0).astype(np.int8) - (state.direction[:, 1] != 0).astype(np.int8)


def step(state):
    """One charged matching tick, pure on the complete input, including B."""
    validate(state)
    L = int(state.L)
    phase = int(state.phase)
    slot, internal, parity = phase // 6, (phase % 6) // 2, phase % 2
    epsilon = 1 if slot == 0 else -1
    direction, credit, flux, background = (getattr(state, name).copy() for name in NAMES)
    events = MatchingEvents()
    # B_a has positive matched direction exactly when its sign bit equals b.
    # Physical axis is decoded locally from B; no coordinate parity is read.
    owners = np.flatnonzero(((state.matching_frame >> internal) & 1) == parity)
    for index in owners:
        owner = int(index)
        axis = AXIS_PERMUTATIONS[int(state.matching_frame[owner]) // 8][internal]
        head = _reference._shift(L, owner, axis, 1)
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
            redirected = _reference._guide(_reference._outward(state, source), epsilon, k, v)
            direction[source, slot] = redirected
            reason = "flux_capacity" if abs(q_next) > 1 else "credit_deficit" if k_next < 0 else "credit_capacity"
            events.redirects.append((epsilon, source, v, redirected, reason))
    result = MatchingState(L, int(state.microtick) + 1, direction, credit, flux, background)
    validate(result)
    return result, events


def checkpoint(state):
    validate(state)
    arrays = {name: base64.b64encode(getattr(state, name).tobytes()).decode("ascii") for name in NAMES}
    payload = dict(schema=SCHEMA, law=LAW_ID, rule=RULE_HASH, frames=FRAME_HASH,
                   encoding=ENCODING_HASH, backend=BACKEND_ID, boundary="periodic",
                   L=int(state.L), microtick_hex=format(int(state.microtick), "x"), arrays=arrays)
    return MAGIC + json.dumps(payload, sort_keys=True, separators=(",", ":")).encode("ascii")


def restore(data):
    try:
        if type(data) is not bytes or not data.startswith(MAGIC):
            raise ValueError("incompatible recorded-matching checkpoint magic")
        payload = json.loads(data[len(MAGIC):], object_pairs_hook=_reference._unique)
        keys = {"schema", "law", "rule", "frames", "encoding", "backend", "boundary",
                "L", "microtick_hex", "arrays"}
        if not isinstance(payload, dict) or set(payload) != keys:
            raise ValueError("invalid recorded-matching checkpoint schema fields")
        for key, value in (("schema", SCHEMA), ("law", LAW_ID), ("rule", RULE_HASH),
                           ("frames", FRAME_HASH), ("encoding", ENCODING_HASH),
                           ("backend", BACKEND_ID), ("boundary", "periodic")):
            if payload[key] != value:
                raise ValueError(f"incompatible recorded-matching checkpoint {key}")
        L = _reference._size(payload["L"])
        ordinal = payload["microtick_hex"]
        if (not isinstance(ordinal, str) or not ordinal
                or (len(ordinal) > 1 and ordinal[0] == "0")
                or any(c not in "0123456789abcdef" for c in ordinal)):
            raise ValueError("invalid canonical hexadecimal ordinal")
        tick = int(ordinal, 16)
        encoded = payload["arrays"]
        if not isinstance(encoded, dict) or set(encoded) != set(NAMES):
            raise ValueError("checkpoint requires all four complete arrays")
        arrays = []
        for name, columns, dtype in _LAYOUT:
            size = L ** 3 * columns
            value = encoded[name]
            if not isinstance(value, str) or len(value) != 4 * ((size + 2) // 3):
                raise ValueError(f"invalid {name} encoded length")
            raw = base64.b64decode(value, validate=True)
            if len(raw) != size or base64.b64encode(raw).decode("ascii") != value:
                raise ValueError(f"invalid {name} canonical encoding")
            shape = (L ** 3,) if name == "matching_frame" else (L ** 3, columns)
            arrays.append(np.frombuffer(raw, dtype=dtype).reshape(shape).copy())
        state = MatchingState(L, tick, *arrays)
        validate(state)
        return state
    except (TypeError, KeyError, UnicodeError, OverflowError) as exc:
        raise ValueError("malformed recorded-matching checkpoint") from exc
