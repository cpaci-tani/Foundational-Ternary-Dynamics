"""FTDHY01 native transport (229 bytes/site) and a JSON checkpoint for the successor."""
from __future__ import annotations
import base64
import hashlib
import struct
from math import prod
import numpy as np
from .. import exact_json as J
from ..checkpoint import _unique_object
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
    if type(data) is not bytes or len(data) < HEADER.size:
        raise ValueError("native state must be complete bytes")
    magic, L, microtick, *hashes = HEADER.unpack_from(data)
    if magic != MAGIC or tuple(hashes) != _hashes():
        raise ValueError("native state law/table/encoding mismatch")
    n = L ** 3
    if L < 3 or len(data) != HEADER.size + BYTES_PER_SITE * n:
        raise ValueError("native state dimensions or exact byte length invalid")
    arrays, offset = [], HEADER.size
    for i, (name, shape) in enumerate(zip(NAMES, _shapes(n))):
        size = prod(shape); raw = data[offset:offset + size]
        if i not in _INT8 and any(v > 1 for v in raw):
            raise ValueError(f"noncanonical boolean storage in {name}")
        arrays.append(np.frombuffer(raw, dtype=np.int8 if i in _INT8 else bool).reshape(shape).copy())
        offset += size
    result = P.StagedState(microtick, S.LatticeState(L, *arrays[:4]), *arrays[4:])
    P.validate(result)
    return result


LEGACY_SCHEMA = "ftd-hydro-checkpoint-1"
SCHEMA = "ftd-hydro-checkpoint-2"


def checkpoint(state: P.StagedState) -> bytes:
    P.validate(state)
    arrays = {name: base64.b64encode(a.tobytes(order="C")).decode("ascii") for name, a in zip(NAMES, _arrays(state))}
    payload = dict(schema=SCHEMA, law=P.LAW_ID, table=H.TABLE_HASH, encoding=H.ENCODING_HASH, boundary="periodic",
                   L=int(state.lattice.L), microtick=J.integer_text(int(state.microtick)), arrays=arrays)
    return J.dumps(payload).encode("utf-8")


def restore(data: bytes) -> P.StagedState:
    try:
        if type(data) is not bytes:
            raise ValueError("checkpoint must be complete bytes")
        payload = J.loads(data, object_pairs_hook=_unique_object)
        expected = {"schema", "law", "table", "encoding", "boundary", "L", "microtick", "arrays"}
        if type(payload) is not dict or set(payload) != expected:
            raise ValueError("checkpoint fields do not match schema")
        if payload["schema"] not in (LEGACY_SCHEMA, SCHEMA):
            raise ValueError("checkpoint schema mismatch")
        for key, value in (("law", P.LAW_ID), ("table", H.TABLE_HASH),
                           ("encoding", H.ENCODING_HASH), ("boundary", "periodic")):
            if payload[key] != value:
                raise ValueError(f"checkpoint {key} mismatch")
        L = payload["L"]
        if type(L) is not int or L < 3:
            raise ValueError("checkpoint L must be an integer >= 3")
        microtick = payload["microtick"]
        if payload["schema"] == SCHEMA:
            microtick = J.integer_from_text(microtick)
        if type(microtick) is not int or microtick < 0:
            raise ValueError("checkpoint microtick must be a nonnegative exact integer")
        encoded = payload["arrays"]
        if type(encoded) is not dict or set(encoded) != set(NAMES):
            raise ValueError("checkpoint requires all complete-state arrays")
        arrays = []
        for i, (name, shape) in enumerate(zip(NAMES, _shapes(L ** 3))):
            text = encoded[name]
            size = prod(shape)
            if type(text) is not str or len(text) != 4 * ((size + 2) // 3):
                raise ValueError(f"checkpoint encoded array length mismatch: {name}")
            raw = base64.b64decode(text, validate=True)
            if len(raw) != size or base64.b64encode(raw).decode("ascii") != text:
                raise ValueError(f"checkpoint noncanonical base64 or length: {name}")
            if i not in _INT8 and any(v > 1 for v in raw):
                raise ValueError(f"noncanonical boolean storage in {name}")
            arrays.append(np.frombuffer(raw, dtype=np.int8 if i in _INT8 else bool).reshape(shape).copy())
        state = P.StagedState(microtick, S.LatticeState(L, *arrays[:4]), *arrays[4:])
        P.validate(state)
        return state
    except (TypeError, KeyError, UnicodeError, OverflowError) as exc:
        raise ValueError("malformed hydro checkpoint") from exc
