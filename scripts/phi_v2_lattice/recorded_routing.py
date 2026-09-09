"""Finite routing with an explicit transforming local corner-direction record.

See SPEC_STRICT_RECORDED_ROUTING_V1.md. This is an isolated research sector;
covariance repair is not complete v3-law or physical continuum approval.
"""
from dataclasses import dataclass
from functools import lru_cache
from itertools import permutations, product
from numbers import Integral
import hashlib
import json

import numpy as np

LAW_ID = "phi-recorded-routing-field-sector-1"
SCHEMA = "ftd-recorded-routing-checkpoint-1"
MAGIC = b"FTDRR1\0"
VELOCITIES = ((1, 0, 0), (-1, 0, 0), (0, 1, 0), (0, -1, 0), (0, 0, 1), (0, 0, -1))
PERMUTATIONS = tuple(permutations(range(6)))
INVERSES = tuple(tuple(p.index(i) for i in range(6)) for p in PERMUTATIONS)
CORNERS = tuple(product((-1, 1), repeat=3))
CUBE_ACTIONS = tuple(product(tuple(permutations(range(3))), CORNERS))
RULE_DESCRIPTION = {
    "law": LAW_ID, "domain": "periodic L>=3; locally equal neighboring corner records",
    "collision": "even ordinal: bank'(x,p,c)=bank(x,p,P_x^-1(c))",
    "stream": "odd ordinal: bank'(x,p,c)=bank(x-v_c,p,c)",
    "router": "every ordinal: routers'(x)=routers(x-D(x)); D decoded from supplied local record",
    "background": "corner_direction'(x)=corner_direction(x)",
    "clock": "advance exactly one physical tick; no random source or pending record",
}
RULE_HASH = hashlib.sha256(json.dumps(RULE_DESCRIPTION, sort_keys=True, separators=(",", ":")).encode()).hexdigest()
ENCODING_HASH = hashlib.sha256(repr((VELOCITIES, PERMUTATIONS, CORNERS,
    "bank:bool,L3x2x6;routers:uint16,L3;corner_direction:uint8,L3;row-major")).encode()).hexdigest()


def _integer(value, minimum, name):
    if isinstance(value, bool) or not isinstance(value, Integral) or value < minimum:
        raise ValueError(f"invalid {name}")
    return int(value)


def _corner_code(value):
    code = _integer(value, 0, "corner code")
    if code >= len(CORNERS):
        raise ValueError("corner code outside finite alphabet")
    return code


def seed_background(L, corner_code):
    """Explicit selected preparation; no default direction or runtime call."""
    return np.full((_integer(L, 3, "L"),) * 3, _corner_code(corner_code), dtype=np.uint8)


def _validate_background(background, L):
    if (not isinstance(background, np.ndarray) or background.dtype != np.dtype(np.uint8)
            or background.shape != (L, L, L) or not background.flags.c_contiguous
            or np.any(background >= len(CORNERS))):
        raise ValueError("expected contiguous uint8 LxLxL corner alphabet 0..7")
    for axis in range(3):
        if not np.array_equal(background, np.roll(background, 1, axis=axis)):
            raise ValueError("neighboring corner records must agree")


@dataclass(frozen=True)
class RoutingState:
    L: int
    microtick: int
    bank: np.ndarray
    routers: np.ndarray
    corner_direction: np.ndarray
    law_id: str = LAW_ID
    rule_hash: str = RULE_HASH
    encoding_hash: str = ENCODING_HASH

    @property
    def phase(self):
        return self.microtick % 2


def validate(state):
    if (type(state) is not RoutingState or state.law_id != LAW_ID
            or state.rule_hash != RULE_HASH or state.encoding_hash != ENCODING_HASH):
        raise ValueError("foreign recorded-routing state or identity")
    L = _integer(state.L, 3, "L")
    _integer(state.microtick, 0, "ordinal")
    if (not isinstance(state.bank, np.ndarray) or state.bank.dtype != np.dtype(bool)
            or state.bank.shape != (L, L, L, 2, 6) or not state.bank.flags.c_contiguous):
        raise ValueError("expected contiguous Boolean LxLxLx2x6 bank")
    if np.any(state.bank.view(np.uint8) > 1):
        raise ValueError("noncanonical Boolean bytes")
    if (not isinstance(state.routers, np.ndarray) or state.routers.dtype != np.dtype(np.uint16)
            or state.routers.shape != (L, L, L) or not state.routers.flags.c_contiguous
            or np.any(state.routers >= len(PERMUTATIONS))):
        raise ValueError("expected contiguous uint16 LxLxL router alphabet 0..719")
    _validate_background(state.corner_direction, L)
    arrays = (state.bank, state.routers, state.corner_direction)
    if any(np.shares_memory(arrays[i], arrays[j]) for i in range(3) for j in range(i)):
        raise ValueError("complete arrays must have disjoint storage")


def initialize(bank, routers, corner_direction):
    """Jointly validate three explicitly supplied arrays and own a clock-zero copy."""
    if not isinstance(bank, np.ndarray) or bank.ndim != 5:
        raise ValueError("expected five-dimensional bank")
    state = RoutingState(bank.shape[0], 0, bank, routers, corner_direction)
    validate(state)
    return RoutingState(state.L, 0, bank.copy(), routers.copy(), corner_direction.copy())


def step(state):
    validate(state)
    if state.phase == 0:
        inverse = np.asarray(INVERSES, dtype=np.intp)[state.routers]
        bank = np.take_along_axis(state.bank, inverse[..., None, :], axis=-1)
    else:
        bank = np.empty_like(state.bank)
        for channel, velocity in enumerate(VELOCITIES):
            bank[..., channel] = np.roll(state.bank[..., channel], velocity, axis=(0, 1, 2))
    # Each destination reads its own complete D record. No representative-site read.
    local_directions = np.asarray(CORNERS, dtype=np.intp)[state.corner_direction]
    coordinates = np.indices((state.L,) * 3, dtype=np.intp)
    source = tuple((coordinates[a] - local_directions[..., a]) % state.L for a in range(3))
    routers = state.routers[source].copy()
    return RoutingState(state.L, int(state.microtick) + 1, bank, routers, state.corner_direction.copy())


def populations(state):
    validate(state)
    return tuple(int(state.bank[..., p, :].sum()) for p in range(2))


def densities(state):
    validate(state)
    return state.bank.sum(axis=-1, dtype=np.int64)


def checkpoint(state):
    validate(state)
    header = json.dumps({"L": int(state.L), "law": LAW_ID, "schema": SCHEMA,
                         "rule_hash": RULE_HASH, "encoding_hash": ENCODING_HASH,
                         "tick_hex": format(int(state.microtick), "x")},
                        sort_keys=True, separators=(",", ":")).encode("ascii")
    bank = np.packbits(state.bank.reshape(-1), bitorder="little").tobytes()
    payload = bank + state.routers.astype("<u2", copy=False).tobytes() + state.corner_direction.tobytes()
    body = len(header).to_bytes(4, "little") + header + payload
    return MAGIC + hashlib.sha256(body).digest() + body


def restore(blob):
    if type(blob) is not bytes or not blob.startswith(MAGIC):
        raise ValueError("foreign or mutable recorded-routing checkpoint")
    offset = len(MAGIC)
    if len(blob) < offset + 36:
        raise ValueError("truncated checkpoint")
    body = blob[offset + 32:]
    if hashlib.sha256(body).digest() != blob[offset:offset + 32]:
        raise ValueError("checkpoint digest mismatch")
    length = int.from_bytes(body[:4], "little")
    if length > len(body) - 4:
        raise ValueError("truncated metadata")
    encoded = body[4:4 + length]

    def unique(items):
        if len({k for k, _ in items}) != len(items):
            raise ValueError("duplicate metadata")
        return dict(items)

    try:
        header = json.loads(encoded.decode("ascii"), object_pairs_hook=unique)
        if type(header) is not dict or set(header) != {"L", "law", "schema", "rule_hash", "encoding_hash", "tick_hex"}:
            raise ValueError("metadata keys")
        if json.dumps(header, sort_keys=True, separators=(",", ":")).encode("ascii") != encoded:
            raise ValueError("noncanonical metadata")
        for key, expected in (("law", LAW_ID), ("schema", SCHEMA),
                              ("rule_hash", RULE_HASH), ("encoding_hash", ENCODING_HASH)):
            if header[key] != expected:
                raise ValueError(f"foreign {key}")
        L = _integer(header["L"], 3, "L")
        tick = header["tick_hex"]
        if (type(tick) is not str or not tick or any(c not in "0123456789abcdef" for c in tick)
                or (len(tick) > 1 and tick[0] == "0")):
            raise ValueError("noncanonical ordinal")
        ordinal = int(tick, 16)
    except (TypeError, UnicodeError, json.JSONDecodeError, OverflowError) as error:
        raise ValueError("malformed metadata") from error
    sites = L ** 3
    bit_count = 12 * sites
    bank_bytes = (bit_count + 7) // 8
    payload = body[4 + length:]
    if len(payload) != bank_bytes + 3 * sites:
        raise ValueError("wrong payload length or missing background")
    bits = np.unpackbits(np.frombuffer(payload[:bank_bytes], dtype=np.uint8), bitorder="little")
    if np.any(bits[bit_count:]):
        raise ValueError("nonzero bit padding")
    bank = bits[:bit_count].astype(bool).reshape(L, L, L, 2, 6)
    router_end = bank_bytes + 2 * sites
    routers = np.frombuffer(payload[bank_bytes:router_end], dtype="<u2").astype(np.uint16, copy=True).reshape(L, L, L)
    background = np.frombuffer(payload[router_end:], dtype=np.uint8).copy().reshape(L, L, L)
    state = RoutingState(L, ordinal, bank, routers, background)
    validate(state)
    return state


@lru_cache(maxsize=48)
def _action_tables(axes, signs):
    def act(vector):
        return tuple(signs[a] * vector[axes[a]] for a in range(3))
    channels = tuple(VELOCITIES.index(act(v)) for v in VELOCITIES)
    inverse = tuple(channels.index(c) for c in range(6))
    router_index = {p: i for i, p in enumerate(PERMUTATIONS)}
    routers = tuple(router_index[tuple(channels[p[inverse[c]]] for c in range(6))] for p in PERMUTATIONS)
    corners = tuple(CORNERS.index(act(d)) for d in CORNERS)
    return channels, routers, corners


def transform(state, axes, signs, translation=(0, 0, 0)):
    """External complete-state symmetry action, not a physical time update."""
    validate(state)
    axes, signs, translation = tuple(axes), tuple(signs), tuple(translation)
    if (len(axes) != 3 or any(type(a) is not int for a in axes) or sorted(axes) != [0, 1, 2]
            or len(signs) != 3 or any(type(s) is not int or s not in (-1, 1) for s in signs)
            or len(translation) != 3
            or any(isinstance(t, bool) or not isinstance(t, Integral) for t in translation)):
        raise ValueError("invalid signed cube action or translation")
    channels, router_action, corner_action = _action_tables(axes, signs)
    coordinates = np.indices((state.L,) * 3, dtype=np.intp)
    targets = tuple((signs[a] * coordinates[axes[a]] + int(translation[a]) % state.L) % state.L for a in range(3))
    bank = np.empty_like(state.bank)
    for source, target in enumerate(channels):
        bank[targets + (slice(None), target)] = state.bank[..., source]
    routers = np.empty_like(state.routers)
    routers[targets] = np.asarray(router_action, dtype=np.uint16)[state.routers]
    background = np.empty_like(state.corner_direction)
    background[targets] = np.asarray(corner_action, dtype=np.uint8)[state.corner_direction]
    result = RoutingState(state.L, int(state.microtick), bank, routers, background)
    validate(result)
    return result


def validate_freshness_scope(L, cycles, corner_code):
    L = _integer(L, 3, "L")
    cycles = _integer(cycles, 0, "cycles")
    corner_code = _corner_code(corner_code)
    if L <= 3 * cycles:
        raise ValueError("prepared-ensemble certificate requires L > 3*cycles")
    return L, cycles, corner_code


def certificate():
    """Bounded exact runtime covariance fixture; not exhaustive state enumeration."""
    L = 3
    bank = (np.arange(L ** 3 * 12).reshape(L, L, L, 2, 6) % 7 < 3)
    routers = ((np.arange(L ** 3).reshape(L, L, L) * 23) % 720).astype(np.uint16)
    cube_checks = translation_checks = 0
    for code in range(8):
        for phase in range(2):
            state = RoutingState(L, phase, bank.copy(), routers.copy(), seed_background(L, code))
            after = step(state)
            for axes, signs in CUBE_ACTIONS:
                if checkpoint(step(transform(state, axes, signs))) != checkpoint(transform(after, axes, signs)):
                    raise ValueError("complete-state cube covariance failed")
                cube_checks += 1
            for shift in ((1, 0, 0), (0, 1, 0), (0, 0, 1)):
                if checkpoint(step(transform(state, (0, 1, 2), (1, 1, 1), shift))) != checkpoint(transform(after, (0, 1, 2), (1, 1, 1), shift)):
                    raise ValueError("complete-state unit translation covariance failed")
                translation_checks += 1
    return {"schema": "strict-recorded-routing-finite-1", "law_id": LAW_ID,
            "rule_hash": RULE_HASH, "encoding_hash": ENCODING_HASH,
            "corner_vectors": [list(d) for d in CORNERS], "corner_alphabet": 8,
            "signed_cube_actions": 48, "physical_phases": 2,
            "complete_runtime_cube_comparisons": cube_checks,
            "complete_runtime_translation_comparisons": translation_checks,
            "covariance_checks_passed": True, "fixture_L": L,
            "domain": "constant local corner records, validated by neighboring equalities",
            "scope": "finite covariance fixture and analytic local rule; not exhaustive microscopic states",
            "runtime_randomness": False, "background_formation_recovered": False,
            "named_expiry_supplied": False, "complete_v3_law_approved": False,
            "interacting_fluid_recovered": False, "canonical_adoption": False}
