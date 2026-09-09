"""Python-only, separately identified alignment research runtime/checkpoint.

No canonical adoption, backend parity, binding or continuum claim is made.
Only collision is changed; the existing pure stages and relation primitive
are reused without replacing any original-law globals or cached tables.
"""
from __future__ import annotations

import base64
from dataclasses import dataclass, field
import hashlib
import json

import numpy as np

from . import alignment as A, channels as C, staged as P, state as S, tick as T

LAW_ID = A.LAW_ID
TABLE_ID = A.TABLE_ID
COLLISION_HASH = A.COLLISION_HASH
BACKEND_ID = "python-phi-alignment-staged-1"
SCHEMA = "ftd-alignment-checkpoint-2"
MAGIC = b"FTD-PHI-ALIGNMENT-2\n"
ENCODING_HASH = hashlib.sha256(repr((S.A9_LIST, C.STATES)).encode("utf-8")).hexdigest()
NAMES = ("s", "ell", "bank", "sc", "fcc", "admitted_sc", "gate_sc", "gate_fcc")


@dataclass(frozen=True)
class AlignmentState:
    microtick: int
    lattice: S.LatticeState
    admitted_sc: np.ndarray
    gate_sc: np.ndarray
    gate_fcc: np.ndarray
    law_id: str = field(default=LAW_ID, init=False)
    table_id: str = field(default=TABLE_ID, init=False)
    collision_hash: str = field(default=COLLISION_HASH, init=False)
    encoding_hash: str = field(default=ENCODING_HASH, init=False)
    boundary: str = field(default="periodic", init=False)

    @property
    def phase(self):
        return self.microtick % 4


def _view(state):
    """Private adapter for unchanged pure primitives, never a public conversion."""
    return P.StagedState(state.microtick, state.lattice, state.admitted_sc,
                         state.gate_sc, state.gate_fcc)


def _arrays(state):
    return {name: getattr(state.lattice if name in NAMES[:5] else state, name) for name in NAMES}


def validate(state):
    if type(state) is not AlignmentState:
        raise ValueError("expected AlignmentState; cross-law state use is forbidden")
    for name, expected in (("law_id", LAW_ID), ("table_id", TABLE_ID),
                           ("collision_hash", COLLISION_HASH), ("encoding_hash", ENCODING_HASH),
                           ("boundary", "periodic")):
        if getattr(state, name) != expected:
            raise ValueError(f"incompatible alignment state {name}")
    P.validate(_view(state))
    values = list(_arrays(state).values())
    for i, value in enumerate(values):
        if any(np.shares_memory(value, other) for other in values[:i]):
            raise ValueError("complete-state arrays must have disjoint storage")


def initialize(lattice):
    """Explicit fresh preparation from a bare payload; never converts a checkpoint.

    All arrays are copied, the ordinal starts at zero, pending controls clear,
    and initial s is allowed to be lagged. No continuation provenance is inferred.
    """
    prepared = P.initialize(lattice)
    out = AlignmentState(prepared.microtick, prepared.lattice, prepared.admitted_sc,
                         prepared.gate_sc, prepared.gate_fcc)
    validate(out)
    return out


def work_units(state):
    validate(state)
    return P.work_units(_view(state))


def step(state):
    """Advance one charged physical microtick, validating before any computation.

    Returned arrays own independent storage. An exception never writes the
    supplied complete state; events are external, nonfeedback observations.
    """
    validate(state)
    if state.phase != 1:
        result, events = P.step(_view(state))
        out = AlignmentState(result.microtick, result.lattice, result.admitted_sc,
                             result.gate_sc, result.gate_fcc)
    else:
        A.load_collision_tables()  # verify selected table before building output
        st = state.lattice
        dst = S.copy(st)
        events = T.TickEvents()
        for i in np.flatnonzero(st.bank.any(axis=1)):
            dst.bank[i], collisions = A.collide_row(st.bank[i], st.ell[i])
            events.collisions.extend((int(i), eps, before, after) for eps, before, after in collisions)
        dst.ell[:] = (st.ell.astype(int) - 1) % 3
        for i in range(S.n_sites(st)):
            for kind, gates, indices in (
                ("sc", state.gate_sc, ((a,) for a in range(3))),
                ("fcc", state.gate_fcc, ((p, q) for p in range(3) for q in range(2)))):
                for idx in indices:
                    key = (i,) + idx
                    if kind == "sc" and state.admitted_sc[key]:
                        continue
                    getattr(dst, kind)[key], direction, held = P._cross(getattr(st, kind)[key], gates[key])
                    if direction:
                        events.crossings.append((kind, i, idx, direction))
                    if held:
                        events.gate_holds.append((kind, i, idx))
        out = AlignmentState(int(state.microtick) + 1, dst, state.admitted_sc.copy(),
                             state.gate_sc.copy(), state.gate_fcc.copy())
    validate(out)
    return out, events


def checkpoint(state):
    """Complete serialization; hexadecimal ordinal avoids decimal digit limits."""
    validate(state)
    arrays = {name: base64.b64encode(value.tobytes(order="C")).decode("ascii")
              for name, value in _arrays(state).items()}
    payload = dict(schema=SCHEMA, law=LAW_ID, table=TABLE_ID, collision=COLLISION_HASH,
                   encoding=ENCODING_HASH, backend=BACKEND_ID, boundary="periodic",
                   L=int(state.lattice.L), microtick_hex=format(int(state.microtick), "x"), arrays=arrays)
    return MAGIC + json.dumps(payload, sort_keys=True, separators=(",", ":")).encode("utf-8")


def _unique_object(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError(f"duplicate checkpoint key: {key}")
        result[key] = value
    return result


def _parse_ordinal(value):
    """Canonical ASCII lowercase hexadecimal; no artificial ordinal bound."""
    if (not isinstance(value, str) or not value
            or (len(value) > 1 and value[0] == "0")
            or any(c not in "0123456789abcdef" for c in value)):
        raise ValueError("invalid canonical hexadecimal checkpoint clock")
    # Power-of-two integer conversion is independent of CPython's process-wide
    # decimal digit limit. Do not disable that limit or alter global settings.
    return int(value, 16)


def restore(data):
    """Reject foreign/incomplete/corrupt records before returning any state."""
    try:
        if not isinstance(data, bytes) or not data.startswith(MAGIC):
            raise ValueError("incompatible alignment checkpoint magic")
        payload = json.loads(data[len(MAGIC):], object_pairs_hook=_unique_object)
        expected = {"schema", "law", "table", "collision", "encoding", "backend", "boundary",
                    "L", "microtick_hex", "arrays"}
        if not isinstance(payload, dict) or set(payload) != expected:
            raise ValueError("checkpoint fields do not match alignment schema")
        for key, value in (("schema", SCHEMA), ("law", LAW_ID), ("table", TABLE_ID),
                           ("collision", COLLISION_HASH), ("encoding", ENCODING_HASH),
                           ("backend", BACKEND_ID), ("boundary", "periodic")):
            if payload[key] != value:
                raise ValueError(f"incompatible alignment checkpoint {key}")
        L = payload["L"]
        if type(L) is not int or L < 3:
            raise ValueError("invalid checkpoint dimensions")
        microtick = _parse_ordinal(payload["microtick_hex"])
        arrays = payload["arrays"]
        if not isinstance(arrays, dict) or set(arrays) != set(NAMES):
            raise ValueError("checkpoint must contain every complete-state array")
        n = L ** 3
        shapes = ((n,), (n,), (n, 384), (n, 3, 2), (n, 3, 2, 2), (n, 3), (n, 3), (n, 3, 2))
        decoded = {}
        for name, shape in zip(NAMES, shapes):
            if not isinstance(arrays[name], str):
                raise ValueError(f"invalid checkpoint {name} encoding")
            size = 1
            for dim in shape:
                size *= dim
            if len(arrays[name]) != 4 * ((size + 2) // 3):
                raise ValueError(f"invalid checkpoint {name} encoded length")
            raw = base64.b64decode(arrays[name], validate=True)
            if len(raw) != size:
                raise ValueError(f"invalid checkpoint {name} byte length")
            if base64.b64encode(raw).decode("ascii") != arrays[name]:
                raise ValueError(f"noncanonical checkpoint {name} base64 encoding")
            boolean = name in ("bank", "admitted_sc", "gate_sc", "gate_fcc")
            if boolean and any(byte not in (0, 1) for byte in raw):
                raise ValueError(f"noncanonical Boolean encoding in {name}")
            decoded[name] = np.frombuffer(raw, dtype=bool if boolean else np.int8).reshape(shape).copy()
        lattice = S.LatticeState(L, *(decoded[name] for name in NAMES[:5]))
        result = AlignmentState(microtick, lattice, *(decoded[name] for name in NAMES[5:]))
        validate(result)
        return result
    except (TypeError, KeyError, UnicodeError, OverflowError) as exc:
        raise ValueError("malformed alignment checkpoint") from exc
