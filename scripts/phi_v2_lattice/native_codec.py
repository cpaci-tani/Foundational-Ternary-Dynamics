"""Exact FTDSC01 native transport. This is not the JSON checkpoint schema.

116-byte header: magic[8], little-endian uint32 L, uint64 microtick, then
32-byte collision, encoding, and law-identifier SHA256 digests. Body is the
eight complete arrays in C order, 416 bytes per site. No native-size integers.
"""
from __future__ import annotations
import hashlib
import struct
import numpy as np
from . import channels as C, state as S, staged as P
from .checkpoint import ENCODING_HASH

MAGIC = b"FTDSC01\0"
HEADER = struct.Struct("<8sIQ32s32s32s")
LAW_HASH = hashlib.sha256(P.LAW_ID.encode("utf-8")).digest()
HASHES = (bytes.fromhex(C.COLLISION_HASH), bytes.fromhex(ENCODING_HASH), LAW_HASH)
NAMES = ("s", "ell", "bank", "sc", "fcc", "admitted_sc", "gate_sc", "gate_fcc")
BYTES_PER_SITE = 416


def encode(state: P.StagedState) -> bytes:
    P.validate(state)
    if state.lattice.L >= 2 ** 32 or state.microtick >= 2 ** 64:
        raise ValueError("state dimensions/clock exceed native unsigned transport")
    result = HEADER.pack(MAGIC, int(state.lattice.L), int(state.microtick), *HASHES)
    return result + b"".join(getattr(state.lattice if i < 5 else state, name).tobytes(order="C")
                             for i, name in enumerate(NAMES))


def decode(data: bytes) -> P.StagedState:
    if type(data) is not bytes or len(data) < HEADER.size:
        raise ValueError("native state must be complete bytes")
    magic, L, microtick, *hashes = HEADER.unpack_from(data)
    if magic != MAGIC or tuple(hashes) != HASHES:
        raise ValueError("native state law/schema/table/encoding mismatch")
    n = L ** 3
    if L < 3 or len(data) != HEADER.size + BYTES_PER_SITE * n:
        raise ValueError("native state dimensions or exact byte length invalid")
    shapes = ((n,), (n,), (n,384), (n,3,2), (n,3,2,2), (n,3), (n,3), (n,3,2))
    arrays = []
    offset = HEADER.size
    for i, (name, shape) in enumerate(zip(NAMES, shapes)):
        size = int(np.prod(shape))
        raw = data[offset:offset+size]
        boolean = i in (2,5,6,7)
        if boolean and any(value > 1 for value in raw):
            raise ValueError(f"noncanonical boolean storage in {name}")
        arrays.append(np.frombuffer(raw, dtype=bool if boolean else np.int8).reshape(shape).copy())
        offset += size
    result = P.StagedState(microtick, S.LatticeState(L, *arrays[:5]), *arrays[5:])
    P.validate(result)
    return result
