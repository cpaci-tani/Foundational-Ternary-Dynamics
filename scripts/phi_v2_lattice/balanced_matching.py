"""Recorded spatial and charge-order contexts for the finite flux reference.

SPEC_STRICT_BALANCED_MATCHING_V1.md fixes the complete preimplementation law.
The two outer conversions are per-owner recodings around one charged tick;
they do not add a force, a propagation stage, or a physical identification.
"""
from __future__ import annotations

import base64
from dataclasses import dataclass, field
import hashlib
import json
from types import MappingProxyType

import numpy as np

from . import flux_binding as _flux
from . import recorded_matching as _recorded


LAW_ID = "phi-flux-balanced-matching-candidate-1"
BACKEND_ID = "python-flux-balanced-matching-1"
SCHEMA = "ftd-flux-balanced-matching-checkpoint-1"
MAGIC = b"FTD-FLUX-BALANCED-MATCHING-1\n"
ACCOUNT_ID = "flux-hop-credit-count"
DIRECTIONS = _recorded.DIRECTIONS
FRAME_DIRECTIONS = _recorded.FRAME_DIRECTIONS
TRIADS = _recorded.TRIADS
FRAME_HASH = _recorded.FRAME_HASH
NAMES = (*_recorded.NAMES, "charge_frame")
_LAYOUT = (*_recorded._LAYOUT, ("charge_frame", 1, "uint8"))
RULE_DESCRIPTION = MappingProxyType({
    "law": LAW_ID, "recorded_rule": _recorded.RULE_HASH,
    "frames": FRAME_HASH, "charge_frame": "one owned bit at each site; equal at every +/- SC neighbor",
    "rule": "local C^eta input conversion; one recorded tick; local C^eta output conversion; eta copies itself",
    "C": "swap local direction/credit columns, negate each owner's flux; full action toggles eta at same ordinal",
    "locality": "each conversion reads that record owner's eta; no distinguished array element",
    "account": ACCOUNT_ID, "boundary": "periodic even integer L>=4",
})
RULE_HASH = hashlib.sha256(json.dumps(dict(RULE_DESCRIPTION), sort_keys=True,
                                    separators=(",", ":")).encode()).hexdigest()
ENCODING_HASH = hashlib.sha256(repr((_recorded.ENCODING_HASH, NAMES, _LAYOUT,
                                   "charge_frame=uint8 bit per owner")).encode()).hexdigest()
MatchingEvents = _recorded.MatchingEvents
seed_background = _recorded.seed_background


@dataclass(frozen=True)
class MatchingState:
    L: int
    microtick: int
    direction: np.ndarray
    credit: np.ndarray
    flux: np.ndarray
    matching_frame: np.ndarray
    charge_frame: np.ndarray
    law_id: str = field(default=LAW_ID, init=False)
    rule_hash: str = field(default=RULE_HASH, init=False)
    frame_hash: str = field(default=FRAME_HASH, init=False)
    encoding_hash: str = field(default=ENCODING_HASH, init=False)
    boundary: str = field(default="periodic", init=False)

    @property
    def phase(self):
        return self.microtick % 12


def seed_charge_frame(L, value=0):
    """Explicit owned choice of polarity-order background, never an implicit default."""
    L = _flux._size(L)
    value = _flux._integer(value, "charge-frame seed")
    if value > 1:
        raise ValueError("charge-frame seed outside bit alphabet")
    return np.full(L ** 3, value, dtype=np.uint8)


def _validate_payload(L, direction, credit, flux, matching_frame, charge_frame, *, owned):
    L = _flux._size(L)
    _recorded._validate_payload(L, direction, credit, flux, matching_frame, owned=owned)
    if (not isinstance(charge_frame, np.ndarray) or charge_frame.shape != (L ** 3,)
            or charge_frame.dtype != np.dtype("uint8")):
        raise ValueError("invalid charge_frame array shape or dtype")
    if np.any(charge_frame > 1):
        raise ValueError("charge_frame outside finite bit alphabet")
    if not charge_frame.flags.c_contiguous:
        raise ValueError("charge_frame requires contiguous storage")
    if owned and not charge_frame.flags.owndata:
        raise ValueError("charge_frame requires owned state storage")
    if any(np.shares_memory(charge_frame, array) for array in (direction, credit, flux, matching_frame)):
        raise ValueError("complete-state arrays must have disjoint storage")
    cube = charge_frame.reshape(L, L, L)
    for axis in range(3):
        for sign in (-1, 1):
            if not np.array_equal(cube, np.roll(cube, sign, axis=axis)):
                raise ValueError("charge_frame must agree at every SC neighbor")


def validate(state):
    if type(state) is not MatchingState:
        raise ValueError("expected balanced MatchingState; no implicit cross-law conversion")
    for name, expected in (("law_id", LAW_ID), ("rule_hash", RULE_HASH),
                           ("frame_hash", FRAME_HASH), ("encoding_hash", ENCODING_HASH),
                           ("boundary", "periodic")):
        if getattr(state, name) != expected:
            raise ValueError(f"incompatible balanced state {name}")
    _flux._integer(state.microtick, "microtick")
    _validate_payload(state.L, state.direction, state.credit, state.flux,
                      state.matching_frame, state.charge_frame, owned=True)


def initialize(L, direction, credit, flux, matching_frame, charge_frame):
    """Explicit complete ordinal-zero preparation with five independent arrays."""
    L = _flux._size(L)
    _validate_payload(L, direction, credit, flux, matching_frame, charge_frame, owned=False)
    state = MatchingState(L, 0, *(array.copy() for array in
                                 (direction, credit, flux, matching_frame, charge_frame)))
    validate(state)
    return state


def populations(state):
    validate(state)
    return tuple(int(np.count_nonzero(state.direction[:, slot])) for slot in range(2))


def account_units(state):
    """Exact flux-hop-credit count, not mechanical energy."""
    validate(state)
    return int(np.abs(state.flux).sum()) + int(state.credit.sum())


def manifestation(state):
    validate(state)
    return (state.direction[:, 0] != 0).astype(np.int8) - (state.direction[:, 1] != 0).astype(np.int8)


def _conjugated_payload(state, charge_frame):
    """Radius-zero recoding at each owner, also defined as a formal array map."""
    mask = charge_frame.astype(bool)[:, None]
    direction = np.where(mask, state.direction[:, ::-1], state.direction)
    credit = np.where(mask, state.credit[:, ::-1], state.credit)
    signs = (1 - 2 * charge_frame.astype(np.int8))[:, None]
    flux = state.flux * signs
    return direction, credit, flux, state.matching_frame.copy()


def _physical_events(events, charge_frame):
    result = MatchingEvents()
    for epsilon, source, destination, owner, axis, q0, q1, k0, k1 in events.moves:
        source_sign = 1 - 2 * int(charge_frame[source])
        edge_sign = 1 - 2 * int(charge_frame[owner])
        result.moves.append((epsilon * source_sign, source, destination, owner, axis,
                             q0 * edge_sign, q1 * edge_sign, k0, k1))
    for epsilon, source, old, new, reason in events.redirects:
        result.redirects.append((epsilon * (1 - 2 * int(charge_frame[source])),
                                 source, old, new, reason))
    for epsilon, owner, axis in events.capacity_holds:
        result.capacity_holds.append((epsilon * (1 - 2 * int(charge_frame[owner])), owner, axis))
    return result


def step(state):
    """One charged tick; all conjugation reads use the corresponding local owner."""
    validate(state)
    converted = _recorded.MatchingState(int(state.L), int(state.microtick),
                                        *_conjugated_payload(state, state.charge_frame))
    evolved, events = _recorded.step(converted)
    result = MatchingState(int(state.L), evolved.microtick,
                           *_conjugated_payload(evolved, state.charge_frame),
                           state.charge_frame.copy())
    validate(result)
    return result, _physical_events(events, state.charge_frame)


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
            raise ValueError("incompatible balanced-matching checkpoint magic")
        payload = json.loads(data[len(MAGIC):], object_pairs_hook=_flux._unique)
        keys = {"schema", "law", "rule", "frames", "encoding", "backend", "boundary",
                "L", "microtick_hex", "arrays"}
        if not isinstance(payload, dict) or set(payload) != keys:
            raise ValueError("invalid balanced-matching checkpoint schema fields")
        for key, value in (("schema", SCHEMA), ("law", LAW_ID), ("rule", RULE_HASH),
                           ("frames", FRAME_HASH), ("encoding", ENCODING_HASH),
                           ("backend", BACKEND_ID), ("boundary", "periodic")):
            if payload[key] != value:
                raise ValueError(f"incompatible balanced-matching checkpoint {key}")
        L = _flux._size(payload["L"])
        ordinal = payload["microtick_hex"]
        if (not isinstance(ordinal, str) or not ordinal
                or (len(ordinal) > 1 and ordinal[0] == "0")
                or any(c not in "0123456789abcdef" for c in ordinal)):
            raise ValueError("invalid canonical hexadecimal ordinal")
        tick = int(ordinal, 16)
        encoded = payload["arrays"]
        if not isinstance(encoded, dict) or set(encoded) != set(NAMES):
            raise ValueError("checkpoint requires all five complete arrays")
        arrays = []
        for name, columns, dtype in _LAYOUT:
            size = L ** 3 * columns
            value = encoded[name]
            if not isinstance(value, str) or len(value) != 4 * ((size + 2) // 3):
                raise ValueError(f"invalid {name} encoded length")
            raw = base64.b64decode(value, validate=True)
            if len(raw) != size or base64.b64encode(raw).decode("ascii") != value:
                raise ValueError(f"invalid {name} canonical encoding")
            shape = (L ** 3,) if columns == 1 else (L ** 3, columns)
            arrays.append(np.frombuffer(raw, dtype=dtype).reshape(shape).copy())
        state = MatchingState(L, tick, *arrays)
        validate(state)
        return state
    except (TypeError, KeyError, UnicodeError, OverflowError) as exc:
        raise ValueError("malformed balanced-matching checkpoint") from exc
