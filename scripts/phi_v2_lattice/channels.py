"""The 384 field channels: (state i in 0..191) x (polarity eps in {+1,-1})."""
from __future__ import annotations
import hashlib
import pickle
from pathlib import Path
from . import _proofs as P

N_STATES = 192
N_CHANNELS = 384
COLLISION_HASH = "D0BB71DBED7938ED286E1D6D91A16700DA31F4550E83B2FB3580CCC347B2BD25"
STATES = P.one_particle_states()
assert len(STATES) == N_STATES
STATE_INDEX = {s: i for i, s in enumerate(STATES)}
_U_STATE = tuple(STATE_INDEX[P.internal_tick(s)] for s in STATES)
_CACHE = Path(__file__).resolve().parent / "_cache" / f"collision_{COLLISION_HASH[:16]}.pkl"


def channel(i_state: int, eps: int) -> int:
    assert 0 <= i_state < N_STATES and eps in (+1, -1)
    return i_state + (N_STATES if eps < 0 else 0)


def unpack(c: int) -> tuple[int, int]:
    return (c % N_STATES, -1 if c >= N_STATES else +1)


def tangent(c: int) -> tuple[int, int, int]:
    (flag, _phase) = STATES[c % N_STATES]
    return tuple(flag[0])


def phase(c: int) -> int:
    return STATES[c % N_STATES][1]


def polarity(c: int) -> int:
    return unpack(c)[1]


def U(c: int) -> int:
    i, eps = unpack(c)
    return channel(_U_STATE[i], eps)


def half_turn(c: int) -> int:
    """Phase-only shift k -> k+2 with the flag fixed (spec 3.2). NOT U∘U."""
    i, eps = unpack(c)
    flag, k = STATES[i]
    return channel(STATE_INDEX[(flag, (k + 2) % 4)], eps)


def field_value_of(c: int) -> tuple[int, ...]:
    return P.field_value(STATES[c % N_STATES])


def layer_value_of(c: int, layer: int) -> tuple[int, ...]:
    return P.layer_value(STATES[c % N_STATES], layer)


# Certified rendering, line-for-line: scripts/proofs/proof_v3_common_action_phi_v2.py, lines 104-110.
def _hash_tables(tables) -> str:
    rows = []
    for layer, table in enumerate(tables):
        for before, after in sorted(table.items()):
            rows.append(f"{layer}:{before[0]},{before[1]}->{after[0]},{after[1]}")
    return hashlib.sha256("\n".join(rows).encode("utf-8")).hexdigest().upper()


def load_collision_tables():
    if _CACHE.exists():
        tables = pickle.loads(_CACHE.read_bytes())
    else:
        P.collision_main()                       # ~40 s, once
        data = P.collision_data()
        tables = tuple(dict(layer) for layer in data["collisions"])
        _CACHE.parent.mkdir(parents=True, exist_ok=True)
        _CACHE.write_bytes(pickle.dumps(tables))
    if _CACHE.parent.exists():
        (_CACHE.parent / ".gitignore").write_text("*\n", encoding="utf-8")
    digest = _hash_tables(tables)
    if digest != COLLISION_HASH:
        raise RuntimeError(f"collision table hash {digest} != frozen {COLLISION_HASH}")
    return tables
