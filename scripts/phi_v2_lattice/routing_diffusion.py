"""Complete finite routing field with a scoped exact counting-ensemble diffusion law.

The actual router array evolves deterministically; no runtime random draws or
marginal replacement occur. See SPEC_STRICT_ROUTING_DIFFUSION_V1.md.
"""
from dataclasses import dataclass
from itertools import permutations
from numbers import Integral
import hashlib
import json

import numpy as np

LAW_ID = "phi-routing-diffusion-field-sector-1"
MAGIC = b"FTDRD1\0"
VELOCITIES = ((1, 0, 0), (-1, 0, 0), (0, 1, 0), (0, -1, 0), (0, 0, 1), (0, 0, -1))
ROUTER_SHIFT = (1, 1, 1)
PERMUTATIONS = tuple(permutations(range(6)))
INVERSES = tuple(tuple(p.index(i) for i in range(6)) for p in PERMUTATIONS)


def _integer(value, minimum, name):
    if isinstance(value, bool) or not isinstance(value, Integral) or value < minimum:
        raise ValueError(f"invalid {name}")
    return int(value)


@dataclass(frozen=True)
class RoutingState:
    L: int
    microtick: int
    bank: np.ndarray
    routers: np.ndarray
    law_id: str = LAW_ID

    @property
    def phase(self):
        return self.microtick % 2


def validate(state):
    if type(state) is not RoutingState or state.law_id != LAW_ID:
        raise ValueError("foreign routing state/law")
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
    if np.shares_memory(state.bank, state.routers):
        raise ValueError("complete record arrays must have disjoint storage")


def initialize(bank, routers):
    if not isinstance(bank, np.ndarray) or bank.ndim != 5:
        raise ValueError("expected five-dimensional bank")
    state = RoutingState(bank.shape[0], 0, bank, routers)
    validate(state)
    return RoutingState(state.L, 0, bank.copy(), routers.copy())


def step(state):
    validate(state)
    if state.phase == 0:
        inverse = np.asarray(INVERSES, dtype=np.intp)[state.routers]
        bank = np.take_along_axis(state.bank, inverse[..., None, :], axis=-1)
    else:
        bank = np.empty_like(state.bank)
        for channel, velocity in enumerate(VELOCITIES):
            bank[..., channel] = np.roll(state.bank[..., channel], velocity, axis=(0, 1, 2))
    routers = np.roll(state.routers, ROUTER_SHIFT, axis=(0, 1, 2))
    return RoutingState(state.L, int(state.microtick) + 1, bank, routers)


def populations(state):
    validate(state)
    return tuple(int(state.bank[..., p, :].sum()) for p in range(2))


def densities(state):
    """Owned exact integer observation of the same authoritative bank."""
    validate(state)
    return state.bank.sum(axis=-1, dtype=np.int64)


def checkpoint(state):
    validate(state)
    header = json.dumps({"L": int(state.L), "law": LAW_ID,
                         "tick_hex": format(int(state.microtick), "x")},
                        sort_keys=True, separators=(",", ":")).encode("ascii")
    bank = np.packbits(state.bank.reshape(-1), bitorder="little").tobytes()
    routers = state.routers.astype("<u2", copy=False).tobytes()
    body = len(header).to_bytes(4, "little") + header + bank + routers
    return MAGIC + hashlib.sha256(body).digest() + body


def restore(blob):
    if type(blob) is not bytes or not blob.startswith(MAGIC):
        raise ValueError("foreign or mutable routing checkpoint")
    offset = len(MAGIC)
    if len(blob) < offset + 36:
        raise ValueError("truncated checkpoint")
    body = blob[offset + 32:]
    if hashlib.sha256(body).digest() != blob[offset:offset + 32]:
        raise ValueError("checkpoint digest mismatch")
    header_length = int.from_bytes(body[:4], "little")
    if header_length > len(body) - 4:
        raise ValueError("truncated metadata")
    encoded = body[4:4 + header_length]

    def unique(items):
        if len({k for k, _ in items}) != len(items):
            raise ValueError("duplicate metadata")
        return dict(items)

    try:
        header = json.loads(encoded.decode("ascii"), object_pairs_hook=unique)
        if type(header) is not dict or set(header) != {"L", "law", "tick_hex"}:
            raise ValueError("metadata keys")
        if json.dumps(header, sort_keys=True, separators=(",", ":")).encode("ascii") != encoded:
            raise ValueError("noncanonical metadata")
        L = _integer(header["L"], 3, "L")
        if header["law"] != LAW_ID:
            raise ValueError("foreign law")
        tick = header["tick_hex"]
        if (type(tick) is not str or not tick or any(c not in "0123456789abcdef" for c in tick)
                or (len(tick) > 1 and tick[0] == "0")):
            raise ValueError("noncanonical ordinal")
        ordinal = int(tick, 16)
    except (TypeError, UnicodeError, json.JSONDecodeError, OverflowError) as error:
        raise ValueError("malformed metadata") from error
    sites = L ** 3
    bank_count = 12 * sites
    bank_bytes = (bank_count + 7) // 8
    payload = body[4 + header_length:]
    if len(payload) != bank_bytes + 2 * sites:
        raise ValueError("wrong payload length")
    unpacked = np.unpackbits(np.frombuffer(payload[:bank_bytes], dtype=np.uint8), bitorder="little")
    if np.any(unpacked[bank_count:]):
        raise ValueError("nonzero bit padding")
    bank = unpacked[:bank_count].astype(bool).reshape(L, L, L, 2, 6)
    routers = np.frombuffer(payload[bank_bytes:], dtype="<u2").astype(np.uint16, copy=True).reshape(L, L, L)
    state = RoutingState(L, ordinal, bank, routers)
    validate(state)
    return state


def validate_freshness_scope(L, cycles):
    L = _integer(L, 3, "L")
    cycles = _integer(cycles, 0, "cycles")
    if L <= 3 * cycles:
        raise ValueError("exact prepared-ensemble certificate requires L > 3*cycles")
    return L, cycles


def predict_densities(initial, cycles):
    """Full-counting-ensemble mean under the registered independent-router preparation.

    This is an external prediction, never a microscopic state owner or restore
    mechanism. Float output represents evaluation of an exact rational kernel.
    """
    if not isinstance(initial, np.ndarray) or initial.ndim != 4:
        raise ValueError("expected LxLxLx2 mean-population array")
    L, cycles = validate_freshness_scope(initial.shape[0], cycles)
    if (initial.shape != (L, L, L, 2) or initial.dtype.kind not in "iuf"
            or not np.isfinite(initial).all()
            or np.any(initial < 0) or np.any(initial > 6)):
        raise ValueError("invalid mean populations")
    result = initial.astype(np.float64, copy=True)
    for _ in range(cycles):
        result = sum(np.roll(result, v, axis=(0, 1, 2)) for v in VELOCITIES) / 6
    return result
