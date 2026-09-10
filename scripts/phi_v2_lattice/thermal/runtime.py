"""Independent integer reference for phi-thermal-pair-candidate-1.

One local collision tick and three physical Moore streaming ticks. This is a
research law, not a replacement selected by a dashboard scenario. Lexicographic
collision order is imposed. Kinetic accounting does not establish fluid closure.
"""
from __future__ import annotations

from collections import defaultdict
from dataclasses import dataclass
from functools import lru_cache
from hashlib import sha256
from itertools import product
import struct

LAW_ID = "phi-thermal-pair-candidate-1"
ENCODING_ID = "ftd.thermal.checkpoint-le64-bank6-v1"
VELOCITIES = tuple(product(range(-3, 4), repeat=3))
CHANNELS = len(VELOCITIES)
ENERGY2 = tuple(sum(x*x for x in v) for v in VELOCITIES)
MASK = (1 << CHANNELS) - 1
MAX_TICK = (1 << 64) - 1
HEADER = struct.Struct("<8sIIQ32s32sQ")


@dataclass
class Records:
    L: int
    microtick: int
    bank: list[int]

    @classmethod
    def empty(cls, L: int) -> "Records":
        if type(L) is not int or not 3 <= L <= 128:
            raise ValueError("L must be an integer in [3,128]")
        return cls(L, 0, [0] * L**3)

    @property
    def phase(self) -> int:
        return self.microtick % 4


def validate(state: Records) -> None:
    if type(state) is not Records or type(state.L) is not int or not 3 <= state.L <= 128:
        raise ValueError("invalid lattice shape")
    if type(state.microtick) is not int or not 0 <= state.microtick <= MAX_TICK:
        raise ValueError("invalid microtick")
    if type(state.bank) is not list or len(state.bank) != state.L**3:
        raise ValueError("invalid bank shape")
    if any(type(value) is not int or value < 0 or value & ~MASK for value in state.bank):
        raise ValueError("invalid Boolean channel alphabet or reserved bits")


def channel(v: tuple[int, int, int]) -> int:
    if len(v) != 3 or any(type(x) is not int or not -3 <= x <= 3 for x in v):
        raise ValueError("invalid velocity")
    return (v[0]+3)*49 + (v[1]+3)*7 + v[2]+3


def occupied(bits: int):
    while bits:
        low = bits & -bits
        yield low.bit_length()-1
        bits ^= low


@lru_cache(maxsize=1)
def collision_classes() -> tuple[tuple[tuple[int, int], ...], ...]:
    classes = defaultdict(list)
    for a, va in enumerate(VELOCITIES):
        for b in range(a+1, CHANNELS):
            vb = VELOCITIES[b]
            key = tuple(va[i]+vb[i] for i in range(3)) + (ENERGY2[a]+ENERGY2[b],)
            classes[key].append((a, b))
    return tuple(tuple(classes[key]) for key in sorted(classes))


@lru_cache(maxsize=1)
def collision_map():
    from types import MappingProxyType
    mapping = {}
    for members in collision_classes():
        for i, pair in enumerate(members):
            mapping[pair] = members[(i+1) % len(members)]
    return MappingProxyType(mapping)


def collision_identity() -> str:
    """Digest of all distinct pairs and successors, in channel-index order."""
    digest = sha256()
    for (a, b), (c, d) in sorted(collision_map().items()):
        digest.update(struct.pack("<4H", a, b, c, d))
    return digest.hexdigest()


def _step(state: Records, mapping=None) -> None:
    validate(state)
    if state.microtick == MAX_TICK:
        raise OverflowError("microtick exhausted")
    phase, L = state.phase, state.L
    if phase == 0:
        if mapping is None:
            mapping = collision_map()
        output = state.bank.copy()
        for site, bits in enumerate(state.bank):
            if bits.bit_count() == 2:
                c, d = mapping[tuple(occupied(bits))]
                output[site] = (1 << c) | (1 << d)
    else:
        output = [0] * len(state.bank)
        displacements = tuple(tuple((1 if x > 0 else -1) if abs(x) >= phase else 0 for x in v)
                              for v in VELOCITIES)
        for site, bits in enumerate(state.bank):
            x, y, z = site % L, (site // L) % L, site // (L*L)
            for c in occupied(bits):
                dx, dy, dz = displacements[c]
                dest = (x+dx) % L + L * ((y+dy) % L + L * ((z+dz) % L))
                output[dest] |= 1 << c
    state.bank = output
    state.microtick += 1


def step(state: Records) -> None:
    _step(state)


def advance(state: Records, microticks: int) -> None:
    validate(state)
    if type(microticks) is not int or not 0 <= microticks <= MAX_TICK-state.microtick:
        raise ValueError("invalid advance interval")
    if not microticks:
        return
    candidate = Records(state.L, state.microtick, state.bank.copy())
    for _ in range(microticks):
        step(candidate)
    state.bank, state.microtick = candidate.bank, candidate.microtick


def totals(state: Records) -> tuple[int, tuple[int, int, int], int]:
    validate(state)
    number, momentum, energy2 = 0, [0, 0, 0], 0
    for bits in state.bank:
        for c in occupied(bits):
            number += 1
            energy2 += ENERGY2[c]
            for i, v in enumerate(VELOCITIES[c]):
                momentum[i] += v
    return number, tuple(momentum), energy2


def manifestation(bits: int) -> int:
    if type(bits) is not int or bits < 0 or bits & ~MASK:
        raise ValueError("invalid bank alphabet")
    return 0 if bits == 0 else (1 if bits.bit_count() % 2 else -1)


def _encode(state: Records, law_id: str) -> bytes:
    validate(state)
    payload = b"".join(value.to_bytes(48, "little") for value in state.bank)
    header = HEADER.pack(b"FTDTH01\0", 1, state.L, state.microtick,
                         sha256(law_id.encode()).digest(), sha256(ENCODING_ID.encode()).digest(), len(payload))
    body = header + payload
    return body + sha256(body).digest()


def _decode(data: bytes, law_id: str) -> Records:
    if type(data) is not bytes or len(data) < 128:
        raise ValueError("invalid checkpoint framing")
    magic, schema, L, tick, law, encoding, size = HEADER.unpack_from(data)
    if (magic != b"FTDTH01\0" or schema != 1 or not 3 <= L <= 128
            or law != sha256(law_id.encode()).digest()
            or encoding != sha256(ENCODING_ID.encode()).digest()
            or size != 48*L**3 or len(data) != 128+size):
        raise ValueError("foreign or malformed checkpoint")
    if sha256(data[:-32]).digest() != data[-32:]:
        raise ValueError("checkpoint checksum mismatch")
    state = Records(L, tick, [int.from_bytes(data[i:i+48], "little") for i in range(96, 96+size, 48)])
    validate(state)
    return state


def encode(state: Records) -> bytes:
    return _encode(state, LAW_ID)


def decode(data: bytes) -> Records:
    return _decode(data, LAW_ID)
