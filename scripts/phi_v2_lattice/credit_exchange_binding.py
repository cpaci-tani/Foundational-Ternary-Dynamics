"""Selected local credit exchange and one-attempt finite binding candidate.

SPEC_STRICT_CREDIT_EXCHANGE_BINDING_V1.md fixes this complete nineteen-tick
law before implementation. Account units are not mechanical energy, and a
transported selected composite is not a physical matter identification.
"""
from __future__ import annotations

import base64
from dataclasses import dataclass, field, replace
import hashlib
import json
from types import MappingProxyType

import numpy as np

from . import balanced_matching as _balanced
from . import flux_binding as _flux
from . import recorded_matching as _recorded


LAW_ID = "phi-flux-credit-exchange-binding-candidate-1"
BACKEND_ID = "python-flux-credit-exchange-binding-1"
SCHEMA = "ftd-flux-credit-exchange-binding-checkpoint-1"
MAGIC = b"FTD-FLUX-CREDIT-EXCHANGE-BINDING-1\n"
ACCOUNT_ID = "flux-hop-credit-count"
DIRECTIONS = _recorded.DIRECTIONS
FRAME_DIRECTIONS = _recorded.FRAME_DIRECTIONS
TRIADS = _recorded.TRIADS
FRAME_HASH = _recorded.FRAME_HASH
NAMES = (*_balanced.NAMES, "attempted")
_LAYOUT = (*_balanced._LAYOUT, ("attempted", 2, "uint8"))
RULE_DESCRIPTION = MappingProxyType({
    "law": LAW_ID, "frames": FRAME_HASH, "account": ACCOUNT_ID,
    "domain": "even periodic L>=4; Gauss; six owned arrays; all attempt bits at all phases",
    "background": "48 coherent signed triads and locally constant eta; copied every tick",
    "phase": "p=t%19; every stage advances exactly one physical tick",
    "reset": "p0: clear attempts; co-located headings copy own site's input slot eta",
    "exchange_schedule": "p1..6: a=(p-1)//2,b=(p-1)%2; matched displacement=(-1)^b B_a(x)",
    "exchange_pair": "q+ selects plus at owner/minus at head; q- selects minus at owner/plus at head",
    "exchange": "occupied pair with unequal credits: transfer1 to0; both headings donor to recipient",
    "hop_schedule": "p7..18: r=p-7; slot=(r//6) xor local eta; a=(r%6)//2,b=r%2",
    "attempt": "unattempted carrier pointing across match consumes attempt on acceptance or denial",
    "capacity": "both same-polarity slots occupied: hold; mark each addressed unattempted carrier",
    "hop": "q_new=q-epsilon*sigma; k_new=k-(q_new^2-q^2); accept ternary q and bit k",
    "move_attempt": "accepted carrier carries attempt1; vacated slot becomes direction/credit/attempt zero",
    "denial": "source-only unique epsilon outward port if k0; otherwise reverse; attempt1",
    "C": "same ordinal; swap direction/credit/attempt columns, negate q, toggle eta, preserve B",
    "pending": "only attempted array; no other deferred state or mutable cache",
})
RULE_HASH = hashlib.sha256(json.dumps(dict(RULE_DESCRIPTION), sort_keys=True,
                                    separators=(",", ":")).encode()).hexdigest()
ENCODING_HASH = hashlib.sha256(repr((NAMES, _LAYOUT, DIRECTIONS, FRAME_DIRECTIONS,
                                   "row-major-xyz; attempt bit per polarity carrier")).encode()).hexdigest()
seed_background = _recorded.seed_background
seed_charge_frame = _balanced.seed_charge_frame
_INTEGER_TYPES = frozenset((int, np.int8, np.int16, np.int32, np.int64,
                            np.uint8, np.uint16, np.uint32, np.uint64,
                            np.intp, np.uintp, np.longlong, np.ulonglong))


@dataclass(frozen=True)
class ExchangeState:
    L: int
    microtick: int
    direction: np.ndarray
    credit: np.ndarray
    flux: np.ndarray
    matching_frame: np.ndarray
    charge_frame: np.ndarray
    attempted: np.ndarray
    law_id: str = field(default=LAW_ID, init=False)
    rule_hash: str = field(default=RULE_HASH, init=False)
    frame_hash: str = field(default=FRAME_HASH, init=False)
    encoding_hash: str = field(default=ENCODING_HASH, init=False)
    boundary: str = field(default="periodic", init=False)

    @property
    def phase(self):
        return self.microtick % 19


@dataclass
class ExchangeEvents(_flux.FluxEvents):
    # (polarity, original source, reason), including accepted/denied/capacity.
    attempt_marks: list = field(default_factory=list)
    # (polarity, site): a pre-state one bit expires on phase zero.
    attempt_expiries: list = field(default_factory=list)
    # (site, selected physical polarity, old_plus_heading, old_minus_heading, new_heading).
    onsite_alignments: list = field(default_factory=list)
    # (owner, axis, donor_polarity, donor_site, recipient_polarity, recipient_site,
    #  donor_k0, recipient_k0, donor_k1, recipient_k1, donor_v0, recipient_v0, common_v1).
    credit_exchanges: list = field(default_factory=list)


def _integer(value, name):
    """Reject behavior-bearing scalar subclasses before shared validation."""
    if not any(type(value) is allowed for allowed in _INTEGER_TYPES):
        raise ValueError(f"invalid {name}: concrete integer required")
    return _flux._integer(value, name)


def _validate_payload(L, direction, credit, flux, matching_frame, charge_frame, attempted, *, owned):
    L = _flux._size(_integer(L, "periodic size"))
    if any(type(array) is not np.ndarray for array in
           (direction, credit, flux, matching_frame, charge_frame, attempted)):
        raise ValueError("complete-state arrays must be exact NumPy arrays")
    _balanced._validate_payload(L, direction, credit, flux, matching_frame, charge_frame, owned=owned)
    if (not isinstance(attempted, np.ndarray) or attempted.shape != (L ** 3, 2)
            or attempted.dtype != np.dtype("uint8")):
        raise ValueError("invalid attempted array shape or dtype")
    if np.any(attempted > 1):
        raise ValueError("attempted outside finite bit alphabet")
    if np.any(attempted[direction == 0] != 0):
        raise ValueError("empty slot must have zero attempted bit")
    if not attempted.flags.c_contiguous or (owned and not attempted.flags.owndata):
        raise ValueError("attempted requires contiguous owned state storage")
    if any(np.shares_memory(attempted, a) for a in (direction, credit, flux, matching_frame, charge_frame)):
        raise ValueError("complete-state arrays must have disjoint storage")


def validate(state):
    if type(state) is not ExchangeState:
        raise ValueError("expected ExchangeState; no implicit cross-law conversion")
    for name, expected in (("law_id", LAW_ID), ("rule_hash", RULE_HASH),
                           ("frame_hash", FRAME_HASH), ("encoding_hash", ENCODING_HASH),
                           ("boundary", "periodic")):
        if type(getattr(state, name)) is not str or getattr(state, name) != expected:
            raise ValueError(f"incompatible credit-exchange state {name}")
    _integer(state.microtick, "microtick")
    _validate_payload(state.L, *(getattr(state, name) for name in NAMES), owned=True)


def initialize(L, direction, credit, flux, matching_frame, charge_frame, attempted):
    """Validate six explicit arrays, then make an owned ordinal-zero preparation."""
    L = _flux._size(_integer(L, "periodic size"))
    arrays = (direction, credit, flux, matching_frame, charge_frame, attempted)
    _validate_payload(L, *arrays, owned=False)
    result = ExchangeState(L, 0, *(a.copy() for a in arrays))
    validate(result)
    return result


def populations(state):
    validate(state)
    return tuple(int(np.count_nonzero(state.direction[:, p])) for p in range(2))


def account_units(state):
    """Exact integer flux-hop-credit count, not physical energy or mass."""
    validate(state)
    return int(np.abs(state.flux).sum()) + int(state.credit.sum())


def manifestation(state):
    validate(state)
    return (state.direction[:, 0] != 0).astype(np.int8) - (state.direction[:, 1] != 0).astype(np.int8)


def _matching_edges(state, column, parity):
    # Each emitted edge reads its owner's supplied frame, never coordinate parity.
    owners = np.flatnonzero(((state.matching_frame >> column) & 1) == parity)
    for raw in owners:
        owner = int(raw)
        axis = _recorded.AXIS_PERMUTATIONS[int(state.matching_frame[owner]) // 8][column]
        yield owner, _flux._shift(state.L, owner, axis, 1), axis


def step(state):
    """One real tick of the frozen nineteen-stage rule, pure on the complete input."""
    validate(state)
    # The admitted domain includes NumPy integral scalars. Normalize the
    # complete wrapper before frozen neighbor helpers perform signed seam
    # arithmetic; changing only a local L would leave their state.L unsigned.
    state = replace(state, L=int(state.L), microtick=int(state.microtick))
    L, phase = int(state.L), int(state.phase)
    direction, credit, flux, background, charge, attempted = (getattr(state, name).copy() for name in NAMES)
    events = ExchangeEvents()
    if phase == 0:
        events.attempt_expiries = [(1 if p == 0 else -1, int(x))
                                  for x, p in np.argwhere(state.attempted != 0)]
        attempted.fill(0)
        for raw in np.flatnonzero(np.all(state.direction != 0, axis=1)):
            x = int(raw)
            slot = int(state.charge_frame[x])
            heading = int(state.direction[x, slot])
            direction[x] = heading
            events.onsite_alignments.append((x, 1 if slot == 0 else -1,
                                            int(state.direction[x, 0]), int(state.direction[x, 1]), heading))
    elif phase <= 6:
        r = phase - 1
        for owner, head, axis in _matching_edges(state, r // 2, r % 2):
            q = int(state.flux[owner, axis])
            if q == 0:
                continue
            left_slot, right_slot = (0, 1) if q == 1 else (1, 0)
            if not state.direction[owner, left_slot] or not state.direction[head, right_slot]:
                continue
            left_k, right_k = int(state.credit[owner, left_slot]), int(state.credit[head, right_slot])
            if left_k == right_k:
                continue
            if left_k == 1:
                donor, recipient, dp, rp, heading = owner, head, left_slot, right_slot, 2 * axis + 1
            else:
                donor, recipient, dp, rp, heading = head, owner, right_slot, left_slot, 2 * axis + 2
            credit[donor, dp], credit[recipient, rp] = 0, 1
            direction[donor, dp] = direction[recipient, rp] = heading
            events.credit_exchanges.append((owner, axis, 1 if dp == 0 else -1, donor,
                                            1 if rp == 0 else -1, recipient, 1, 0, 0, 1,
                                            int(state.direction[donor, dp]), int(state.direction[recipient, rp]), heading))
    else:
        r = phase - 7
        for owner, head, axis in _matching_edges(state, (r % 6) // 2, r % 2):
            slot = (r // 6) ^ int(state.charge_frame[owner])
            epsilon = 1 if slot == 0 else -1
            left, right = int(state.direction[owner, slot]), int(state.direction[head, slot])
            if left and right:
                events.capacity_holds.append((epsilon, owner, axis))
                for source, v, pointed in ((owner, left, 2 * axis + 1), (head, right, 2 * axis + 2)):
                    if v == pointed and not state.attempted[source, slot]:
                        attempted[source, slot] = 1
                        events.attempt_marks.append((epsilon, source, "capacity"))
                continue
            if not left and not right:
                continue
            source, destination, sigma, v = (owner, head, 1, left) if left else (head, owner, -1, right)
            if state.attempted[source, slot] or v != 2 * axis + (1 if sigma == 1 else 2):
                continue
            q, k = int(state.flux[owner, axis]), int(state.credit[source, slot])
            q_new = q - epsilon * sigma
            k_new = k - (q_new * q_new - q * q)
            attempted[source, slot] = 1
            if -1 <= q_new <= 1 and 0 <= k_new <= 1:
                direction[source, slot] = credit[source, slot] = attempted[source, slot] = 0
                direction[destination, slot], credit[destination, slot], attempted[destination, slot] = v, k_new, 1
                flux[owner, axis] = q_new
                events.moves.append((epsilon, source, destination, owner, axis, q, q_new, k, k_new))
                events.attempt_marks.append((epsilon, source, "accepted"))
            else:
                # Only the refused source heading consults the nonshared incident star.
                redirected = _flux._guide(_flux._outward(state, source), epsilon, k, v)
                direction[source, slot] = redirected
                reason = "flux_capacity" if abs(q_new) > 1 else "credit_deficit" if k_new < 0 else "credit_capacity"
                events.redirects.append((epsilon, source, v, redirected, reason))
                events.attempt_marks.append((epsilon, source, reason))
    result = ExchangeState(L, int(state.microtick) + 1, direction, credit, flux, background, charge, attempted)
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
            raise ValueError("incompatible credit-exchange checkpoint magic")
        payload = json.loads(data[len(MAGIC):], object_pairs_hook=_flux._unique)
        keys = {"schema", "law", "rule", "frames", "encoding", "backend", "boundary",
                "L", "microtick_hex", "arrays"}
        if not isinstance(payload, dict) or set(payload) != keys:
            raise ValueError("invalid credit-exchange checkpoint schema fields")
        for key, value in (("schema", SCHEMA), ("law", LAW_ID), ("rule", RULE_HASH),
                           ("frames", FRAME_HASH), ("encoding", ENCODING_HASH),
                           ("backend", BACKEND_ID), ("boundary", "periodic")):
            if payload[key] != value:
                raise ValueError(f"incompatible credit-exchange checkpoint {key}")
        L = _flux._size(payload["L"])
        ordinal = payload["microtick_hex"]
        if (not isinstance(ordinal, str) or not ordinal
                or (len(ordinal) > 1 and ordinal[0] == "0")
                or any(c not in "0123456789abcdef" for c in ordinal)):
            raise ValueError("invalid canonical hexadecimal ordinal")
        tick = int(ordinal, 16)
        encoded = payload["arrays"]
        if not isinstance(encoded, dict) or set(encoded) != set(NAMES):
            raise ValueError("checkpoint requires all six complete arrays")
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
        result = ExchangeState(L, tick, *arrays)
        validate(result)
        return result
    except (TypeError, KeyError, UnicodeError, OverflowError) as exc:
        raise ValueError("malformed credit-exchange checkpoint") from exc
