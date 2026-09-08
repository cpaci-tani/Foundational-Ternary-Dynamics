"""Deterministic, complete, non-pickle snapshots of the staged candidate."""
from __future__ import annotations

import base64
import hashlib
import json
import numpy as np
from . import channels as C, state as S
from .staged import LAW_ID, StagedState, validate

SCHEMA = "ftd-staged-checkpoint-1"
ENCODING_HASH = hashlib.sha256(repr((S.A9_LIST, C.STATES)).encode("utf-8")).hexdigest()
_NAMES = ("s", "ell", "bank", "sc", "fcc", "admitted_sc", "gate_sc", "gate_fcc")


def checkpoint(state: StagedState) -> bytes:
    validate(state)
    arrays = {}
    for name in _NAMES:
        value = getattr(state.lattice if name in _NAMES[:5] else state, name)
        arrays[name] = base64.b64encode(value.tobytes(order="C")).decode("ascii")
    payload = dict(schema=SCHEMA, law=LAW_ID, collision=C.COLLISION_HASH, encoding=ENCODING_HASH,
                   boundary="periodic", L=int(state.lattice.L), microtick=int(state.microtick), arrays=arrays)
    return json.dumps(payload, sort_keys=True, separators=(",", ":")).encode("utf-8")


def _unique_object(pairs):
    out = {}
    for key, value in pairs:
        if key in out:
            raise ValueError(f"duplicate checkpoint key: {key}")
        out[key] = value
    return out


def restore(data: bytes) -> StagedState:
    try:
        if not isinstance(data, bytes):
            raise ValueError("checkpoint must be bytes")
        payload = json.loads(data, object_pairs_hook=_unique_object)
        expected = {"schema", "law", "collision", "encoding", "boundary", "L", "microtick", "arrays"}
        if not isinstance(payload, dict) or set(payload) != expected:
            raise ValueError("checkpoint fields do not match schema")
        for key, value in (("schema", SCHEMA), ("law", LAW_ID), ("collision", C.COLLISION_HASH),
                           ("encoding", ENCODING_HASH), ("boundary", "periodic")):
            if payload[key] != value:
                raise ValueError(f"incompatible checkpoint {key}")
        L = payload["L"]
        if type(L) is not int or L < 3 or type(payload["microtick"]) is not int or payload["microtick"] < 0:
            raise ValueError("invalid checkpoint dimensions or clock")
        arrays = payload["arrays"]
        if not isinstance(arrays, dict) or set(arrays) != set(_NAMES):
            raise ValueError("checkpoint must contain every complete-state array")
        n = L ** 3
        shapes = ((n,), (n,), (n, 384), (n, 3, 2), (n, 3, 2, 2), (n, 3), (n, 3), (n, 3, 2))
        decoded = {}
        for name, shape in zip(_NAMES, shapes):
            raw = base64.b64decode(arrays[name], validate=True)
            size = 1
            for dim in shape:
                size *= dim
            if len(raw) != size:
                raise ValueError(f"invalid checkpoint {name} byte length")
            boolean = name in ("bank", "admitted_sc", "gate_sc", "gate_fcc")
            if boolean and any(byte not in (0, 1) for byte in raw):
                raise ValueError(f"noncanonical boolean encoding in {name}")
            decoded[name] = np.frombuffer(raw, dtype=bool if boolean else np.int8).reshape(shape).copy()
        lattice = S.LatticeState(L, *(decoded[name] for name in _NAMES[:5]))
        state = StagedState(payload["microtick"], lattice, *(decoded[name] for name in _NAMES[5:]))
        validate(state)
        return state
    except (TypeError, KeyError, UnicodeError, OverflowError) as exc:
        raise ValueError("malformed staged checkpoint") from exc
