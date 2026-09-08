"""FTDHY01 native transport (229 bytes/site) and a JSON checkpoint for the successor."""
from __future__ import annotations
import base64
import hashlib
import json
import struct
import numpy as np
from . import channels as H, staged as P, state as S

MAGIC = b"FTDHY01\0"
HEADER = struct.Struct("<8sIQ32s32s32s")
LAW_HASH = hashlib.sha256(P.LAW_ID.encode("utf-8")).digest()
NAMES = ("s", "bank", "sc", "fcc", "admitted_sc", "admitted_fcc", "gate_sc", "gate_fcc")
BYTES_PER_SITE = 1 + 192 + 6 + 12 + 3 + 6 + 3 + 6  # 229
_INT8 = (0, 2, 3)


def _hashes():
    return (bytes.fromhex(H.TABLE_HASH), bytes.fromhex(H.ENCODING_HASH), LAW_HASH)


def _shapes(n):
    return ((n,), (n, 192), (n, 3, 2), (n, 3, 2, 2), (n, 3), (n, 3, 2), (n, 3), (n, 3, 2))


def _arrays(state):
    return [getattr(state.lattice if i < 4 else state, name) for i, name in enumerate(NAMES)]


def encode(state: P.StagedState) -> bytes:
    P.validate(state)
    head = HEADER.pack(MAGIC, int(state.lattice.L), int(state.microtick), *_hashes())
    return head + b"".join(a.tobytes(order="C") for a in _arrays(state))


def decode(data: bytes) -> P.StagedState:
    if not isinstance(data, bytes) or len(data) < HEADER.size:
        raise ValueError("native state must be complete bytes")
    magic, L, microtick, *hashes = HEADER.unpack_from(data)
    if magic != MAGIC or tuple(hashes) != _hashes():
        raise ValueError("native state law/table/encoding mismatch")
    n = L ** 3
    if L < 3 or len(data) != HEADER.size + BYTES_PER_SITE * n:
        raise ValueError("native state dimensions or exact byte length invalid")
    arrays, offset = [], HEADER.size
    for i, (name, shape) in enumerate(zip(NAMES, _shapes(n))):
        size = int(np.prod(shape)); raw = data[offset:offset + size]
        if i not in _INT8 and any(v > 1 for v in raw):
            raise ValueError(f"noncanonical boolean storage in {name}")
        arrays.append(np.frombuffer(raw, dtype=np.int8 if i in _INT8 else bool).reshape(shape).copy())
        offset += size
    result = P.StagedState(microtick, S.LatticeState(L, *arrays[:4]), *arrays[4:])
    P.validate(result)
    return result


SCHEMA = "ftd-hydro-checkpoint-1"


def checkpoint(state: P.StagedState) -> bytes:
    P.validate(state)
    arrays = {name: base64.b64encode(a.tobytes(order="C")).decode("ascii") for name, a in zip(NAMES, _arrays(state))}
    payload = dict(schema=SCHEMA, law=P.LAW_ID, table=H.TABLE_HASH, encoding=H.ENCODING_HASH, boundary="periodic",
                   L=int(state.lattice.L), microtick=int(state.microtick), arrays=arrays)
    return json.dumps(payload, sort_keys=True, separators=(",", ":")).encode("utf-8")


def restore(data: bytes) -> P.StagedState:
    payload = json.loads(data)
    if payload.get("schema") != SCHEMA or payload.get("law") != P.LAW_ID or payload.get("table") != H.TABLE_HASH:
        raise ValueError("checkpoint schema/law/table mismatch")
    L = int(payload["L"]); n = L ** 3
    arrays = []
    for i, (name, shape) in enumerate(zip(NAMES, _shapes(n))):
        raw = base64.b64decode(payload["arrays"][name])
        arrays.append(np.frombuffer(raw, dtype=np.int8 if i in _INT8 else bool).reshape(shape).copy())
    state = P.StagedState(int(payload["microtick"]), S.LatticeState(L, *arrays[:4]), *arrays[4:])
    P.validate(state)
    return state
