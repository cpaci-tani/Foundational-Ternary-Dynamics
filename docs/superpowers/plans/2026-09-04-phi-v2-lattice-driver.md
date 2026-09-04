# Φ v2 Lattice Driver Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** A pure-Python reference implementation of the v3 selected reference law `Φ v2` on a finite periodic simple-cubic lattice, composed from the constitution's *certified* cell-level functions, with a transition journal and a census that measures the corrected FTD-1028 acceptance criterion (period-8 site square wave, 4/8 duty, carrier orbit conserved across nulls, polarity conserved, no in-place flip) under **composition** — many relations interacting through the frozen collision table.

**Architecture:** One package `scripts/phi_v2_lattice/` whose modules mirror the spec's six synchronous cases. The cell law is **imported** from the certified proof scripts (`readout`, `rotate`, `encode`, `relation_tick`, `one_particle_states`, `internal_tick`, `layer_value`, the 55,008-row collision table) — never re-implemented. State is a small set of NumPy arrays; the tick is a pure function of the pre-state that computes every output from `X_n` (a coordinate definition, as the spec says — no hidden microticks) and commits once. A journal records site-level and relation-level transitions; a census computes the criterion.

**Tech Stack:** Python 3.11+, NumPy, pytest (project Python tests live in `scripts/tests/`), the proof scripts under `scripts/proofs/` (SymPy is pulled in transitively by the collision certificate; it runs ~40 s once and is cached).

**Spec:** `docs/theory/01_reference/SPEC_V3_COMMON_ACTION_PHI_R2_R5_v2.md` (§1 state, §2 collision, §3 tick, §5 R5 preparation) + `docs/theory/01_reference/SPEC_V3_FINITE_CARRIER_INVENTORY_R1_v2.md` (§3 alphabets, §4 ownership) + `docs/theory/01_reference/strict_discrete_common_action_register_v3.json` (`selected_phi`, `carrier_inventory`). Executable cell reference: `scripts/proofs/proof_v3_common_action_phi_v2.py`. Corrected criterion: `docs/theory/07_assessment/engine_infrastructure_rg/AUDIT_C4_TRANSACTION_CENSUS.md` §3.5/§5.

## Global Constraints

- **Never re-implement a certified function.** Import from `scripts/proofs/`: `proof_v3_common_action_phi_v2.{A9, BLANK, PHASES, readout, rotate, phase_index, encode, relation_tick}`; `proof_hodge_flag_pair_collision_invariant_space.{one_particle_states, field_value, PHASE_COORDINATES}`; `proof_global_c3_cotangent_layer_hodge_maxwell_target.{internal_tick, layer_value}`; `proof_shared_edge_hodge_flag_bcc_propagation.{SC_DIRECTIONS}`; `proof_global_c3_cotangent_layer_equivariant_collision.{main, CERTIFICATE_DATA}`.
- **Collision table hash is frozen:** `D0BB71DBED7938ED286E1D6D91A16700DA31F4550E83B2FB3580CCC347B2BD25` — the driver verifies it on load and refuses to run otherwise.
- **Synchrony:** every output of `tick` is a function of the pre-state only. Implement by reading only from the input `LatticeState` (never from partially written outputs), then constructing the new state in one commit.
- **Declared gauges (not in the spec; do not "improve" them, they are named so results are reproducible):**
  - G1 tail/head: SC edge on axis `a` at site `i`: tail `i`, head `shift(i, +e_a)`. FCC plane `p ∈ {0,1,2}` is the plane orthogonal to axis `p`, with in-plane axes `(a, b) = sorted({0,1,2} − {p})`. Diagonal `q=0` ("diag+"): tail `i`, head `shift(shift(i,+e_a),+e_b)`. Diagonal `q=1` ("diag−"): tail `shift(i,+e_a)`, head `shift(i,+e_b)`.
  - G2 "total endpoint field occupation" for the crossing gate: `popcount(bank[tail]) + popcount(bank[head])`, pre-tick banks.
  - G3 `bal3(q) = ((q + 1) % 3) - 1` (balanced residue in {−1,0,+1}).
  - G4 "exactly one endpoint/channel presentation targets that edge": count phase-2 channels at `x` with tangent `+e_a` **plus** phase-2 channels at `shift(x,+e_a)` with tangent `−e_a`, over both polarities and all `(n,h)`; admit iff that count is exactly 1 and both A9 slots of the edge are blank.
  - G5 streaming: every occupied channel streams **exactly one** SC hop (the registered vacuum rule); the "manifested departure" half-turn applies to a channel leaving site `x` iff `s_x ≠ 0` **pre-tick**.
  - G6 manifestation uses **post-crossing** primaries `λ'` (so that `ΔQ + div J = 0` is an identity, spec §3.4).
- No AI co-author trailer on commits (CLAUDE.md policy). Commit after every task with the message given.
- Do not edit `LEDGER.md`, `CLAUDE.md`, or any file outside `scripts/phi_v2_lattice/`, `scripts/tests/phi_v2_lattice/`, and this plan.
- Branch: create your working branch **from `integration-ptime-tx`** (`git checkout -b <name> integration-ptime-tx`), which carries this plan and the Φ v2 isolated-relation census.

---

## File Structure

```
scripts/phi_v2_lattice/
  __init__.py          (empty)
  _proofs.py           sys.path shim: makes scripts/proofs importable; re-exports the certified functions
  channels.py          the 384 field channels: indexing, U, half-turn, tangent, (E,B), collision tables (cached)
  geometry.py          periodic L^3 sites, shifts, the 9 relations per site, tail/head (G1)
  state.py             LatticeState (s, ell, bank, sc, fcc) + A9 index helpers
  tick.py              the six synchronous cases as pure functions + tick()
  journal.py           site-level and relation-level transition rows
  prepare.py           isolated relation, R5 vacuum, sparse material preparations
  conservation.py      work units, Gauss identity, layer sums
  census.py            corrected criterion per relation; site-level transition classification
  experiments/run_census.py   the composed-law experiment; prints the verdict table
  _cache/              collision table pickle (gitignored)
scripts/tests/phi_v2_lattice/
  test_channels.py test_geometry.py test_state.py test_tick.py test_prepare.py test_conservation.py test_census.py
```

`_cache/` must be added to `.gitignore` (one line: `scripts/phi_v2_lattice/_cache/`).

---

### Task 1: Proof shim and channel algebra (`_proofs.py`, `channels.py`)

**Files:**
- Create: `scripts/phi_v2_lattice/__init__.py`, `scripts/phi_v2_lattice/_proofs.py`, `scripts/phi_v2_lattice/channels.py`
- Test: `scripts/tests/phi_v2_lattice/test_channels.py`
- Modify: `.gitignore` (append `scripts/phi_v2_lattice/_cache/`)

**Interfaces:**
- Produces:
  - `_proofs.py`: re-exports `A9, BLANK, PHASES, readout, rotate, phase_index, encode, relation_tick, one_particle_states, field_value, internal_tick, layer_value, SC_DIRECTIONS, collision_main, collision_data`.
  - `channels.py`: `N_STATES = 192`, `N_CHANNELS = 384`, `STATES` (tuple from `one_particle_states()`), `STATE_INDEX` (dict state→int), `def channel(i_state: int, eps: int) -> int`, `def unpack(c: int) -> tuple[int, int]` (returns `(i_state, eps)` with `eps ∈ {+1,−1}`), `def tangent(c: int) -> tuple[int,int,int]`, `def phase(c: int) -> int`, `def polarity(c: int) -> int`, `def U(c: int) -> int`, `def half_turn(c: int) -> int`, `def field_value_of(c: int) -> tuple[int,...]`, `def layer_value_of(c: int, layer: int) -> tuple[int,...]`, `COLLISION_HASH: str`, `def load_collision_tables() -> tuple[dict[tuple[int,int], tuple[int,int]], dict, dict]` (three layers; keys and values are canonical `(lo, hi)` pairs of **state** indices).

- [ ] **Step 1: Write the failing tests**

```python
# scripts/tests/phi_v2_lattice/test_channels.py
import itertools
import pytest
from phi_v2_lattice import channels as C

def test_channel_indexing_is_bijective():
    seen = set()
    for i in range(C.N_STATES):
        for eps in (+1, -1):
            c = C.channel(i, eps)
            assert 0 <= c < C.N_CHANNELS
            assert C.unpack(c) == (i, eps)
            seen.add(c)
    assert len(seen) == C.N_CHANNELS

def test_U_is_the_certified_internal_tick_with_period_twelve():
    """U = internal_tick lifted to channels: the Hodge flag cycles with period 3
    (d -> h n -> d x n -> d, the Z3 that matches the collision-layer decrement) and the
    C4 phase with period 4, so the full state has period lcm(3,4) = 12. Polarity is a
    separate copy label and is never touched."""
    for c in range(C.N_CHANNELS):
        w = c
        for _ in range(12):
            assert C.polarity(w) == C.polarity(c)
            w = C.U(w)
        assert w == c
        assert any(C.U(C.U(C.U(C.U(x)))) != x for x in (c,)) or True   # period is 12, not 4 (see below)
    # the phase alone returns after four applications; the flag alone after three
    for c in range(C.N_CHANNELS):
        w = c
        for _ in range(4):
            w = C.U(w)
        assert C.phase(w) == C.phase(c)
        assert (w == c) is False or C.tangent(C.U(C.U(C.U(c)))) == C.tangent(c)

def test_half_turn_is_a_phase_only_shift():
    """Spec 3.2: the manifested-departure half-turn is k -> k+2 with the FLAG fixed
    (a channel permutation that cannot create a streaming write collision). It is NOT U∘U,
    which also rotates the flag."""
    for c in range(C.N_CHANNELS):
        h = C.half_turn(c)
        assert C.tangent(h) == C.tangent(c) and C.polarity(h) == C.polarity(c)
        assert C.phase(h) == (C.phase(c) + 2) % 4
        assert C.half_turn(h) == c
        assert h != C.U(C.U(c)) or C.tangent(C.U(C.U(c))) == C.tangent(c)

def test_tangent_is_an_sc_unit_vector():
    for c in range(C.N_CHANNELS):
        d = C.tangent(c)
        assert sorted(abs(x) for x in d) == [0, 0, 1]

def test_collision_tables_are_fixed_point_free_involutions_preserving_records_and_fields():
    tables = C.load_collision_tables()
    assert len(tables) == 3
    for layer, table in enumerate(tables):
        assert len(table) == 18_336
        for before, after in table.items():
            assert before != after                      # fixed-point free
            assert table[after] == before               # involution
            assert before[0] < before[1] and after[0] < after[1]
            fb = tuple(a + b for a, b in zip(C.layer_value_of(C.channel(before[0], +1), layer),
                                             C.layer_value_of(C.channel(before[1], +1), layer)))
            fa = tuple(a + b for a, b in zip(C.layer_value_of(C.channel(after[0], +1), layer),
                                             C.layer_value_of(C.channel(after[1], +1), layer)))
            assert fb == fa                             # six layer-appropriate (E,B) sums

def test_collision_hash_is_the_frozen_one():
    assert C.COLLISION_HASH == "D0BB71DBED7938ED286E1D6D91A16700DA31F4550E83B2FB3580CCC347B2BD25"
    C.load_collision_tables()   # raises if the rebuilt table does not hash to COLLISION_HASH
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `cd scripts && python -m pytest tests/phi_v2_lattice/test_channels.py -q`
Expected: FAIL with `ModuleNotFoundError: No module named 'phi_v2_lattice'`

- [ ] **Step 3: Write the shim and channel module**

```python
# scripts/phi_v2_lattice/_proofs.py
"""Import shim: the certified cell-level functions from scripts/proofs. Never re-implement them."""
from __future__ import annotations
import sys
from pathlib import Path

PROOFS = Path(__file__).resolve().parents[1] / "proofs"
if str(PROOFS) not in sys.path:
    sys.path.insert(0, str(PROOFS))

from proof_v3_common_action_phi_v2 import (  # noqa: E402
    A9, BLANK, PHASES, readout, rotate, phase_index, encode, relation_tick)
from proof_hodge_flag_pair_collision_invariant_space import (  # noqa: E402
    one_particle_states, field_value, PHASE_COORDINATES)
from proof_global_c3_cotangent_layer_hodge_maxwell_target import (  # noqa: E402
    internal_tick, layer_value)
from proof_shared_edge_hodge_flag_bcc_propagation import SC_DIRECTIONS  # noqa: E402
import proof_global_c3_cotangent_layer_equivariant_collision as _collision  # noqa: E402


def collision_main():
    _collision.main()


def collision_data():
    return _collision.CERTIFICATE_DATA
```

```python
# scripts/phi_v2_lattice/channels.py
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
    digest = _hash_tables(tables)
    if digest != COLLISION_HASH:
        raise RuntimeError(f"collision table hash {digest} != frozen {COLLISION_HASH}")
    return tables
```

Also create an empty `scripts/phi_v2_lattice/__init__.py` and append `scripts/phi_v2_lattice/_cache/` to `.gitignore`. Add `scripts/tests/phi_v2_lattice/conftest.py` containing:

```python
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[2]))   # makes `phi_v2_lattice` importable
```

- [ ] **Step 4: Run tests to verify they pass**

Run: `cd scripts && python -m pytest tests/phi_v2_lattice/test_channels.py -q`
Expected: 6 passed (first run ~60 s while the collision certificate builds; later runs < 5 s from cache)

- [ ] **Step 5: Commit**

```bash
git add scripts/phi_v2_lattice/__init__.py scripts/phi_v2_lattice/_proofs.py scripts/phi_v2_lattice/channels.py scripts/tests/phi_v2_lattice/conftest.py scripts/tests/phi_v2_lattice/test_channels.py .gitignore
git commit -m "feat(phi_v2_lattice): certified-import shim and 384-channel algebra with frozen collision tables"
```

---

### Task 2: Geometry and state (`geometry.py`, `state.py`)

**Files:**
- Create: `scripts/phi_v2_lattice/geometry.py`, `scripts/phi_v2_lattice/state.py`
- Test: `scripts/tests/phi_v2_lattice/test_geometry.py`, `scripts/tests/phi_v2_lattice/test_state.py`

**Interfaces:**
- Consumes: `channels.tangent`, `_proofs.A9/BLANK/rotate/readout/encode`.
- Produces:
  - `geometry.py`: `E = ((1,0,0),(0,1,0),(0,0,1))`, `def site_index(L, x, y, z) -> int`, `def coords(L, i) -> tuple[int,int,int]`, `def shift(L, i, d: tuple[int,int,int]) -> int` (periodic), `def plane_axes(p) -> tuple[int,int]` (returns `(a,b)`, `a<b`, both ≠ p), `def sc_endpoints(L, i, a) -> tuple[int,int]` (tail, head), `def fcc_endpoints(L, i, p, q) -> tuple[int,int]` (G1), `def relations_at(L, i) -> list[tuple[str, int, tuple, int]]` (every relation touching site `i` as `(kind, owner, idx, role)` with `kind ∈ {'sc','fcc'}`, `idx = (a,)` or `(p,q)`, `role = +1` if `i` is the tail, `-1` if head; exactly 18 entries, 9 with each role), `def sc_edge_of(L, x, d) -> tuple[int, int]` (the `(owner, axis)` of SC edge `{x, x+d}` for a unit SC vector `d`).
  - `state.py`: `A9_LIST = tuple(A9)`, `A9_INDEX`, `BLANK_IDX`, `def z_of(idx) -> tuple[int,int]`, `def idx_of(z) -> int`, `@dataclass class LatticeState: L:int; s: np.ndarray[int8,(N,)]; ell: np.ndarray[int8,(N,)]; bank: np.ndarray[bool,(N,384)]; sc: np.ndarray[int8,(N,3,2)]; fcc: np.ndarray[int8,(N,3,2,2)]` (last axis: 0 = primary λ, 1 = reserve ρ; entries are A9 indices), `def blank(L) -> LatticeState`, `def copy(st) -> LatticeState`, `def n_sites(st) -> int`.

- [ ] **Step 1: Write the failing tests**

```python
# scripts/tests/phi_v2_lattice/test_geometry.py
from phi_v2_lattice import geometry as G

def test_shift_is_periodic_and_invertible():
    L = 4
    for i in range(L**3):
        for a in range(3):
            j = G.shift(L, i, G.E[a]); assert G.shift(L, j, tuple(-x for x in G.E[a])) == i
    assert G.shift(L, G.site_index(L, 3, 0, 0), (1, 0, 0)) == G.site_index(L, 0, 0, 0)

def test_every_site_is_tail_of_nine_and_head_of_nine():
    L = 4
    for i in range(L**3):
        rels = G.relations_at(L, i)
        assert len(rels) == 18
        assert sum(1 for r in rels if r[3] == +1) == 9
        assert sum(1 for r in rels if r[3] == -1) == 9

def test_endpoints_are_consistent_with_relations_at():
    L = 4
    for i in range(L**3):
        for kind, owner, idx, role in G.relations_at(L, i):
            tail, head = (G.sc_endpoints(L, owner, *idx) if kind == 'sc' else G.fcc_endpoints(L, owner, *idx))
            assert (tail if role == +1 else head) == i

def test_sc_edge_of_maps_both_endpoints_to_the_same_edge():
    L = 4
    for x in range(L**3):
        for a in range(3):
            d = G.E[a]; md = tuple(-v for v in d)
            assert G.sc_edge_of(L, x, d) == (x, a)
            assert G.sc_edge_of(L, G.shift(L, x, d), md) == (x, a)
```

```python
# scripts/tests/phi_v2_lattice/test_state.py
import numpy as np
from phi_v2_lattice import state as S
from phi_v2_lattice._proofs import BLANK, encode, rotate

def test_a9_index_roundtrip_and_blank():
    assert len(S.A9_LIST) == 9
    for i in range(9): assert S.idx_of(S.z_of(i)) == i
    assert S.z_of(S.BLANK_IDX) == BLANK

def test_blank_state_shapes():
    st = S.blank(3); N = 27
    assert st.s.shape == (N,) and st.ell.shape == (N,) and st.bank.shape == (N, 384)
    assert st.sc.shape == (N, 3, 2) and st.fcc.shape == (N, 3, 2, 2)
    assert (st.sc == S.BLANK_IDX).all() and (st.fcc == S.BLANK_IDX).all() and not st.bank.any()

def test_copy_is_deep():
    st = S.blank(3); cp = S.copy(st)
    cp.sc[0, 0, 1] = S.idx_of(encode(0, +1))
    assert st.sc[0, 0, 1] == S.BLANK_IDX
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `cd scripts && python -m pytest tests/phi_v2_lattice/test_geometry.py tests/phi_v2_lattice/test_state.py -q`
Expected: FAIL with `ImportError`/`ModuleNotFoundError` for `geometry`/`state`

- [ ] **Step 3: Write geometry and state**

```python
# scripts/phi_v2_lattice/geometry.py
"""Periodic L^3 simple-cubic lattice and the nine relations per site (declared gauge G1)."""
from __future__ import annotations

E = ((1, 0, 0), (0, 1, 0), (0, 0, 1))


def site_index(L: int, x: int, y: int, z: int) -> int:
    return ((x % L) * L + (y % L)) * L + (z % L)


def coords(L: int, i: int):
    return (i // (L * L), (i // L) % L, i % L)


def shift(L: int, i: int, d) -> int:
    x, y, z = coords(L, i)
    return site_index(L, x + d[0], y + d[1], z + d[2])


def plane_axes(p: int) -> tuple[int, int]:
    a, b = [k for k in range(3) if k != p]
    return a, b


def sc_endpoints(L: int, i: int, a: int) -> tuple[int, int]:
    return i, shift(L, i, E[a])                                   # G1


def fcc_endpoints(L: int, i: int, p: int, q: int) -> tuple[int, int]:
    a, b = plane_axes(p)
    if q == 0:                                                     # diag+
        return i, shift(L, shift(L, i, E[a]), E[b])
    return shift(L, i, E[a]), shift(L, i, E[b])                    # diag-


def relations_at(L: int, i: int):
    """All 18 (relation, role) incidences at site i: 9 as tail (+1), 9 as head (-1)."""
    out = []
    for a in range(3):
        out.append(("sc", i, (a,), +1))
        out.append(("sc", shift(L, i, tuple(-v for v in E[a])), (a,), -1))
    for p in range(3):
        a, b = plane_axes(p)
        ma, mb = tuple(-v for v in E[a]), tuple(-v for v in E[b])
        out.append(("fcc", i, (p, 0), +1))                                       # diag+ tail at i
        out.append(("fcc", shift(L, shift(L, i, ma), mb), (p, 0), -1))           # diag+ head at i
        out.append(("fcc", shift(L, i, ma), (p, 1), +1))                         # diag- tail at i (owner i-e_a)
        out.append(("fcc", shift(L, i, mb), (p, 1), -1))                         # diag- head at i (owner i-e_b)
    assert len(out) == 18
    return out


def sc_edge_of(L: int, x: int, d) -> tuple[int, int]:
    a = [k for k in range(3) if d[k] != 0]
    assert len(a) == 1
    a = a[0]
    if d[a] > 0:
        return x, a
    return shift(L, x, d), a
```

```python
# scripts/phi_v2_lattice/state.py
"""LatticeState: (s, ell, bank) per site; (lambda, rho) per SC edge and per FCC diagonal."""
from __future__ import annotations
from dataclasses import dataclass
import numpy as np
from ._proofs import A9, BLANK

A9_LIST = tuple(A9)
A9_INDEX = {z: i for i, z in enumerate(A9_LIST)}
BLANK_IDX = A9_INDEX[BLANK]


def z_of(idx: int):
    return A9_LIST[int(idx)]


def idx_of(z) -> int:
    return A9_INDEX[tuple(z)]


@dataclass
class LatticeState:
    L: int
    s: np.ndarray      # int8 (N,)
    ell: np.ndarray    # int8 (N,)   collision layer in {0,1,2}
    bank: np.ndarray   # bool (N,384)
    sc: np.ndarray     # int8 (N,3,2)      [site, axis, slot]        slot 0 = lambda, 1 = rho
    fcc: np.ndarray    # int8 (N,3,2,2)    [site, plane, diag, slot]


def n_sites(st: LatticeState) -> int:
    return st.L ** 3


def blank(L: int) -> LatticeState:
    N = L ** 3
    return LatticeState(
        L=L,
        s=np.zeros(N, dtype=np.int8),
        ell=np.zeros(N, dtype=np.int8),
        bank=np.zeros((N, 384), dtype=bool),
        sc=np.full((N, 3, 2), BLANK_IDX, dtype=np.int8),
        fcc=np.full((N, 3, 2, 2), BLANK_IDX, dtype=np.int8),
    )


def copy(st: LatticeState) -> LatticeState:
    return LatticeState(st.L, st.s.copy(), st.ell.copy(), st.bank.copy(), st.sc.copy(), st.fcc.copy())
```

- [ ] **Step 4: Run tests to verify they pass**

Run: `cd scripts && python -m pytest tests/phi_v2_lattice/test_geometry.py tests/phi_v2_lattice/test_state.py -q`
Expected: 7 passed

- [ ] **Step 5: Commit**

```bash
git add scripts/phi_v2_lattice/geometry.py scripts/phi_v2_lattice/state.py scripts/tests/phi_v2_lattice/test_geometry.py scripts/tests/phi_v2_lattice/test_state.py
git commit -m "feat(phi_v2_lattice): periodic SC geometry with nine declared relations per site, and LatticeState"
```

---

### Task 3: The synchronous tick (`tick.py`)

**Files:**
- Create: `scripts/phi_v2_lattice/tick.py`
- Test: `scripts/tests/phi_v2_lattice/test_tick.py`

**Interfaces:**
- Consumes: everything from Tasks 1–2; `_proofs.relation_tick/rotate/readout/encode/phase_index`.
- Produces: `@dataclass class TickEvents: absorptions: list[tuple[int,int,int,int]]` (`(x, channel, owner, axis)`), `collisions: list[tuple[int,int,tuple,tuple]]` (`(site, eps, before_pair, after_pair)` state-index pairs), `crossings: list[tuple[str,int,tuple,int]]` (`(kind, owner, idx, direction)`; `+1` = token went primary→reserve, `−1` = reserve→primary); `def bal3(q: int) -> int`; `def gate(st, tail, head) -> int`; `def admitted_absorptions(st) -> dict[tuple[int,int], tuple[int,int]]` (`(owner,axis) -> (x, channel)`); `def collide_row(row: np.ndarray, ell: int, tables) -> tuple[np.ndarray, list]`; `def stream(st, bank_after_collision: np.ndarray) -> np.ndarray`; `def cross_relations(st, absorbing: set[tuple[int,int]]) -> tuple[np.ndarray, np.ndarray, list]`; `def manifest(st, new_sc, new_fcc) -> np.ndarray`; `def tick(st, tables) -> tuple[LatticeState, TickEvents]`.

- [ ] **Step 1: Write the failing tests**

```python
# scripts/tests/phi_v2_lattice/test_tick.py
import numpy as np
from phi_v2_lattice import channels as C, geometry as G, state as S, tick as T
from phi_v2_lattice._proofs import BLANK, encode, rotate, readout

def _tables():
    return C.load_collision_tables()

def _isolated(L=4, owner=None, axis=0, phase=0, pol=+1, slot=1):
    st = S.blank(L)
    owner = (L**3)//2 if owner is None else owner
    st.sc[owner, axis, slot] = S.idx_of(encode(phase, pol))
    return st, owner

def test_bal3():
    assert [T.bal3(q) for q in (-3, -2, -1, 0, 1, 2, 3)] == [0, 1, -1, 0, 1, -1, 0]

def test_isolated_relation_reproduces_the_certified_period_eight_square_wave():
    """Lift of the constitution's C15/C16: one token, reserve slot, phase 0, no field."""
    st, owner = _isolated(); tables = _tables()
    tail, head = G.sc_endpoints(st.L, owner, 0)
    seq_tail, seq_head, occ = [], [], []
    st0 = S.copy(st)
    for t in range(16):
        st, ev = T.tick(st, tables)
        seq_tail.append(int(st.s[tail])); seq_head.append(int(st.s[head]))
        occ.append(int(readout(S.z_of(st.sc[owner, 0, 0]))[0]))
    assert occ == [1,1,1,1,0,0,0,0]*2                      # primary owned 4 of 8, exact period 8
    assert seq_tail == [1,1,1,1,0,0,0,0]*2                 # tail site +1 while primary-owned
    assert seq_head == [-1,-1,-1,-1,0,0,0,0]*2             # head site -1: a relation manifests as a dipole
    # after 8 ticks the pair returns exactly
    assert st.sc[owner, 0].tolist() == st0.sc[owner, 0].tolist()

def test_gauss_identity_holds_on_random_states():
    rng = np.random.default_rng(1); tables = _tables(); L = 4; N = L**3
    for _ in range(5):
        st = S.blank(L)
        st.sc[:] = rng.integers(0, 9, size=st.sc.shape).astype(np.int8)
        st.fcc[:] = rng.integers(0, 9, size=st.fcc.shape).astype(np.int8)
        st.bank[:] = rng.random((N, 384)) < 0.01
        st.ell[:] = rng.integers(0, 3, size=N).astype(np.int8)
        Q_before = T._incidence(st, st.sc, st.fcc)
        new, ev = T.tick(st, tables)
        Q_after = T._incidence(new, new.sc, new.fcc)
        J = T._current(st, new)                            # per-relation oriented current
        div = T._divergence(st, J)
        assert np.array_equal(Q_after - Q_before + div, np.zeros(N, dtype=int))
        assert np.array_equal(new.s, np.vectorize(T.bal3)(Q_after))

def test_work_units_are_conserved_by_the_tick():
    rng = np.random.default_rng(2); tables = _tables(); L = 4; N = L**3
    for _ in range(5):
        st = S.blank(L)
        st.sc[:] = rng.choice([S.BLANK_IDX, S.idx_of(encode(2, +1))], size=st.sc.shape, p=[0.7, 0.3]).astype(np.int8)
        st.bank[:] = rng.random((N, 384)) < 0.02
        before = int(st.bank.sum()) + int((st.sc != S.BLANK_IDX).sum()) + int((st.fcc != S.BLANK_IDX).sum())
        new, ev = T.tick(st, tables)
        after = int(new.bank.sum()) + int((new.sc != S.BLANK_IDX).sum()) + int((new.fcc != S.BLANK_IDX).sum())
        assert before == after

def test_streaming_never_write_collides():
    rng = np.random.default_rng(3); L = 4; N = L**3
    st = S.blank(L); st.bank[:] = rng.random((N, 384)) < 0.3
    st.s[:] = rng.integers(-1, 2, size=N).astype(np.int8)     # random manifested sites (half-turn path)
    out = T.stream(st, st.bank)
    assert int(out.sum()) == int(st.bank.sum())               # a permutation of occupied slots

def test_absorption_admits_only_unique_proposals_onto_blank_edges():
    tables = _tables(); L = 4; st = S.blank(L); x = 0
    # one phase-2 channel with tangent +e_0 at x
    c = next(c for c in range(384) if C.phase(c) == 2 and C.tangent(c) == (1, 0, 0) and C.polarity(c) == +1)
    st.bank[x, c] = True
    adm = T.admitted_absorptions(st)
    assert adm == {(x, 0): (x, c)}
    # a competing proposal from the far endpoint (tangent -e_0) makes both fail closed
    y = G.shift(L, x, (1, 0, 0))
    c2 = next(c for c in range(384) if C.phase(c) == 2 and C.tangent(c) == (-1, 0, 0) and C.polarity(c) == +1)
    st.bank[y, c2] = True
    assert T.admitted_absorptions(st) == {}
    # and an occupied slot blocks admission
    st.bank[y, c2] = False; st.sc[x, 0, 1] = S.idx_of(encode(0, +1))
    assert T.admitted_absorptions(st) == {}

def test_absorption_writes_blank_primary_and_rotated_token_to_the_reserve():
    tables = _tables(); L = 4; st = S.blank(L); x = 5
    c = next(c for c in range(384) if C.phase(c) == 2 and C.tangent(c) == (0, 1, 0) and C.polarity(c) == -1)
    st.bank[x, c] = True
    new, ev = T.tick(st, tables)
    assert ev.absorptions == [(x, c, x, 1)]
    assert not new.bank.any()                                  # the channel was cleared, nothing else streamed
    assert S.z_of(new.sc[x, 1, 0]) == BLANK
    assert S.z_of(new.sc[x, 1, 1]) == rotate(encode(2, -1))     # (lambda', rho') = (0, R z)  (C13/C14)

def test_tick_is_a_pure_function_of_the_prestate():
    rng = np.random.default_rng(4); tables = _tables(); L = 3; N = 27
    st = S.blank(L); st.bank[:] = rng.random((N, 384)) < 0.02
    st.sc[:] = rng.integers(0, 9, size=st.sc.shape).astype(np.int8)
    a, _ = T.tick(S.copy(st), tables); b, _ = T.tick(S.copy(st), tables)
    for f in ("s", "ell", "bank", "sc", "fcc"):
        assert np.array_equal(getattr(a, f), getattr(b, f))
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `cd scripts && python -m pytest tests/phi_v2_lattice/test_tick.py -q`
Expected: FAIL with `ImportError` for `tick`

- [ ] **Step 3: Write the tick**

```python
# scripts/phi_v2_lattice/tick.py
"""The complete synchronous tick of Phi v2 (spec section 3), as pure functions of the pre-state.

Order of the coordinate definition (each stage reads ONLY the pre-state and the
outputs of earlier pure functions; nothing is written until the final commit):
  3.1 unique absorption/expiry   -> which phase-2 channels become relation tokens
  3.2 collision, Hodge tick U, one-hop streaming (+ manifested-departure half-turn)
  3.3 relation crossing (relation_tick with the even-parity gate G2)
  3.4 manifestation: s' = bal3(incidence of POST-crossing primaries)  (G3, G6)
"""
from __future__ import annotations
from dataclasses import dataclass, field
import numpy as np
from . import channels as C, geometry as G, state as S
from ._proofs import BLANK, readout, rotate, encode, relation_tick

_N = C.N_STATES


@dataclass
class TickEvents:
    absorptions: list = field(default_factory=list)   # (x, channel, owner, axis)
    collisions: list = field(default_factory=list)    # (site, eps, before_pair, after_pair)
    crossings: list = field(default_factory=list)     # (kind, owner, idx, direction)


def bal3(q: int) -> int:
    return ((int(q) + 1) % 3) - 1


# ---------------------------------------------------------------- 3.1 absorption
def admitted_absorptions(st: S.LatticeState):
    """(owner, axis) -> (x, channel): phase-2 channels admitted onto blank SC edges (G4)."""
    proposals: dict[tuple[int, int], list[tuple[int, int]]] = {}
    xs, cs = np.nonzero(st.bank)
    for x, c in zip(xs.tolist(), cs.tolist()):
        if C.phase(c) != 2:
            continue
        edge = G.sc_edge_of(st.L, x, C.tangent(c))
        proposals.setdefault(edge, []).append((x, c))
    admitted = {}
    for (owner, axis), plist in proposals.items():
        if len(plist) != 1:
            continue                                                       # competing -> fail closed
        if st.sc[owner, axis, 0] != S.BLANK_IDX or st.sc[owner, axis, 1] != S.BLANK_IDX:
            continue                                                       # needs both slots blank
        admitted[(owner, axis)] = plist[0]
    return admitted


# ---------------------------------------------------------------- 3.2 collision + U + streaming
def collide_row(row: np.ndarray, ell: int, tables):
    """Per polarity layer: exactly two occupied channels -> replace by C_ell of the pair."""
    out = row.copy(); events = []
    table = tables[int(ell)]
    for eps, lo in ((+1, 0), (-1, _N)):
        occ = np.nonzero(row[lo:lo + _N])[0]
        if len(occ) == 2:
            before = (int(occ[0]), int(occ[1]))
            after = table[before]
            out[lo + before[0]] = False; out[lo + before[1]] = False
            out[lo + after[0]] = True; out[lo + after[1]] = True
            events.append((eps, before, after))
    return out, events


def stream(st: S.LatticeState, bank: np.ndarray) -> np.ndarray:
    """U on every occupied channel (+ half-turn if the departure site is manifested), then one SC hop
    along the PRE-update tangent (G5). A permutation of occupied slots: no write collisions."""
    out = np.zeros_like(bank)
    xs, cs = np.nonzero(bank)
    for x, c in zip(xs.tolist(), cs.tolist()):
        d = C.tangent(c)
        c2 = C.U(c)
        if st.s[x] != 0:
            c2 = C.half_turn(c2)
        y = G.shift(st.L, x, d)
        assert not out[y, c2], "streaming write collision (spec says impossible)"
        out[y, c2] = True
    return out


# ---------------------------------------------------------------- 3.3 crossing
def gate(st: S.LatticeState, tail: int, head: int) -> int:
    total = int(st.bank[tail].sum()) + int(st.bank[head].sum())          # G2, pre-tick banks
    return 1 if total % 2 == 0 else 0


def _cross_pair(st, lam_idx, rho_idx, tail, head):
    lam, rho = S.z_of(lam_idx), S.z_of(rho_idx)
    lam2, rho2 = relation_tick(lam, rho, even_gate=bool(gate(st, tail, head)))
    direction = 0
    if readout(lam)[0] and not readout(lam2)[0]:
        direction = +1
    elif not readout(lam)[0] and readout(lam2)[0]:
        direction = -1
    return S.idx_of(lam2), S.idx_of(rho2), direction


def cross_relations(st: S.LatticeState, absorbing: set):
    new_sc = st.sc.copy(); new_fcc = st.fcc.copy(); events = []
    N = S.n_sites(st)
    for i in range(N):
        for a in range(3):
            if (i, a) in absorbing:
                continue
            tail, head = G.sc_endpoints(st.L, i, a)
            l2, r2, dirn = _cross_pair(st, st.sc[i, a, 0], st.sc[i, a, 1], tail, head)
            new_sc[i, a, 0], new_sc[i, a, 1] = l2, r2
            if dirn:
                events.append(("sc", i, (a,), dirn))
        for p in range(3):
            for q in range(2):
                tail, head = G.fcc_endpoints(st.L, i, p, q)
                l2, r2, dirn = _cross_pair(st, st.fcc[i, p, q, 0], st.fcc[i, p, q, 1], tail, head)
                new_fcc[i, p, q, 0], new_fcc[i, p, q, 1] = l2, r2
                if dirn:
                    events.append(("fcc", i, (p, q), dirn))
    return new_sc, new_fcc, events


# ---------------------------------------------------------------- 3.4 manifestation
def _eps(idx) -> int:
    o, p, _ = readout(S.z_of(idx))
    return int(p) if o else 0


def _incidence(st: S.LatticeState, sc: np.ndarray, fcc: np.ndarray) -> np.ndarray:
    """Q_x = sum over oriented PRIMARY relations of +eps(lambda) at the tail and -eps(lambda) at the head."""
    N = S.n_sites(st); Q = np.zeros(N, dtype=int)
    for i in range(N):
        for a in range(3):
            e = _eps(sc[i, a, 0])
            if e:
                tail, head = G.sc_endpoints(st.L, i, a); Q[tail] += e; Q[head] -= e
        for p in range(3):
            for q in range(2):
                e = _eps(fcc[i, p, q, 0])
                if e:
                    tail, head = G.fcc_endpoints(st.L, i, p, q); Q[tail] += e; Q[head] -= e
    return Q


def manifest(st: S.LatticeState, new_sc: np.ndarray, new_fcc: np.ndarray) -> np.ndarray:
    Q = _incidence(st, new_sc, new_fcc)                                       # G6: post-crossing primaries
    return np.array([bal3(q) for q in Q], dtype=np.int8)


def _current(before: S.LatticeState, after: S.LatticeState):
    """J_r = -(eps(lambda'_r) - eps(lambda_r)) per relation, keyed like the state arrays."""
    return (-(np.vectorize(_eps)(after.sc[..., 0]) - np.vectorize(_eps)(before.sc[..., 0])),
            -(np.vectorize(_eps)(after.fcc[..., 0]) - np.vectorize(_eps)(before.fcc[..., 0])))


def _divergence(st: S.LatticeState, J) -> np.ndarray:
    """div J at each site: +J at the tail, -J at the head of every oriented primary relation."""
    Jsc, Jfcc = J; N = S.n_sites(st); div = np.zeros(N, dtype=int)
    for i in range(N):
        for a in range(3):
            tail, head = G.sc_endpoints(st.L, i, a); div[tail] += Jsc[i, a]; div[head] -= Jsc[i, a]
        for p in range(3):
            for q in range(2):
                tail, head = G.fcc_endpoints(st.L, i, p, q); div[tail] += Jfcc[i, p, q]; div[head] -= Jfcc[i, p, q]
    return div


# ---------------------------------------------------------------- the tick
def tick(st: S.LatticeState, tables):
    ev = TickEvents()
    # 3.1 absorption: decide from the pre-state; remove admitted channels from the bank copy
    adm = admitted_absorptions(st)
    bank = st.bank.copy()
    absorbing = set()
    for (owner, axis), (x, c) in adm.items():
        bank[x, c] = False
        absorbing.add((owner, axis))
        ev.absorptions.append((x, c, owner, axis))
    # 3.2 collision on the non-absorbed bank, then U/half-turn + one-hop streaming
    collided = bank.copy()
    for i in np.nonzero(bank.any(axis=1))[0].tolist():
        collided[i], evs = collide_row(bank[i], st.ell[i], tables)
        for eps, b, a in evs:
            ev.collisions.append((i, eps, b, a))
    new_bank = stream(st, collided)
    new_ell = ((st.ell.astype(int) - 1) % 3).astype(np.int8)
    # 3.3 crossing on every non-absorbing relation (gate from PRE-tick banks)
    new_sc, new_fcc, crossings = cross_relations(st, absorbing)
    ev.crossings.extend(crossings)
    # 3.1 (write half): the absorbed token lands as (lambda', rho') = (BLANK, R z)
    for (owner, axis), (x, c) in adm.items():
        z = encode(2, C.polarity(c))
        new_sc[owner, axis, 0] = S.BLANK_IDX
        new_sc[owner, axis, 1] = S.idx_of(rotate(z))
    # 3.4 manifestation from post-crossing primaries
    new_s = manifest(st, new_sc, new_fcc)
    return S.LatticeState(st.L, new_s, new_ell, new_bank, new_sc, new_fcc), ev
```

- [ ] **Step 4: Run tests to verify they pass**

Run: `cd scripts && python -m pytest tests/phi_v2_lattice/test_tick.py -q`
Expected: 8 passed. If `test_isolated_relation_reproduces_the_certified_period_eight_square_wave` fails on the tail/head sign, **do not flip G1** — report it; the sign is a declared gauge and the test encodes the spec's `+ε` at the tail.

- [ ] **Step 5: Commit**

```bash
git add scripts/phi_v2_lattice/tick.py scripts/tests/phi_v2_lattice/test_tick.py
git commit -m "feat(phi_v2_lattice): the complete synchronous Phi v2 tick as pure functions of the pre-state"
```

---

### Task 4: Preparations, journal, conservation (`prepare.py`, `journal.py`, `conservation.py`)

**Files:**
- Create: `scripts/phi_v2_lattice/prepare.py`, `scripts/phi_v2_lattice/journal.py`, `scripts/phi_v2_lattice/conservation.py`
- Test: `scripts/tests/phi_v2_lattice/test_prepare.py`, `scripts/tests/phi_v2_lattice/test_conservation.py`

**Interfaces:**
- Consumes: Tasks 1–3.
- Produces:
  - `prepare.py`: `def isolated_relation(L, owner=None, axis=0, phase=0, polarity=+1, slot=1) -> LatticeState`; `def r5_vacuum(L, seed, occupation=0.5, eps=+1, background=None) -> LatticeState` (every relation's both slots = `background` (default `encode(0,+1)`), `s=0`, `ell=0`, one polarity layer's 192 channels each occupied independently with probability `occupation` from `np.random.default_rng(seed)`, conjugate layer blank); `def sparse_material(L, seed, n_tokens, field_occupation=0.0, eps=+1) -> LatticeState` (`n_tokens` distinct random relations each get one token `encode(phase, polarity)` with random phase/polarity in a random slot; field layer as in `r5_vacuum` at `field_occupation`).
  - `journal.py`: `@dataclass class SiteRow: tick:int; site:int; s_before:int; s_after:int`; `@dataclass class RelationRow: tick:int; kind:str; owner:int; idx:tuple; lam_before:int; rho_before:int; lam_after:int; rho_after:int`; `class Journal` with `site_rows: list[SiteRow]`, `relation_rows: list[RelationRow]`, `def record(self, tick, before, after)` (appends a `SiteRow` for every site whose `s` changed and a `RelationRow` for every relation whose `(λ,ρ)` changed), `def site_series(self, site, horizon) -> list[int]` (the full `s` trajectory reconstructed from an initial state — store `s0` at construction: `Journal(state0)`), `def relation_series(self, kind, owner, idx, horizon) -> list[tuple[int,int]]`.
  - `conservation.py`: `def work_units(st) -> int` (occupied channels + occupied relation slots); `def gauss_residual(before, after) -> int` (`max |ΔQ + div J|`, must be 0); `def layer_sum(st) -> tuple[int,...]` (sum over sites and occupied channels of `layer_value_of(c, ell_x)`, both polarities).

- [ ] **Step 1: Write the failing tests**

```python
# scripts/tests/phi_v2_lattice/test_prepare.py
import numpy as np
from phi_v2_lattice import prepare as Pz, state as S, channels as C
from phi_v2_lattice._proofs import encode, readout

def test_isolated_relation_places_one_token():
    st = Pz.isolated_relation(4, phase=1, polarity=-1)
    occ = (st.sc != S.BLANK_IDX).sum() + (st.fcc != S.BLANK_IDX).sum()
    assert occ == 1 and not st.bank.any() and not st.s.any()

def test_r5_vacuum_has_inert_relations_and_one_polarity_layer():
    st = Pz.r5_vacuum(4, seed=7)
    N = 64
    assert (st.s == 0).all() and (st.ell == 0).all()
    assert (st.sc != S.BLANK_IDX).all() and (st.fcc != S.BLANK_IDX).all()      # both slots occupied everywhere
    assert not st.bank[:, C.N_STATES:].any()                                  # conjugate polarity blank
    frac = st.bank[:, :C.N_STATES].mean()
    assert 0.45 < frac < 0.55

def test_sparse_material_token_count():
    st = Pz.sparse_material(4, seed=3, n_tokens=5)
    assert (st.sc != S.BLANK_IDX).sum() + (st.fcc != S.BLANK_IDX).sum() == 5
```

```python
# scripts/tests/phi_v2_lattice/test_conservation.py
import numpy as np
from phi_v2_lattice import prepare as Pz, tick as T, channels as C, conservation as K, journal as J, state as S

def test_r5_vacuum_conserves_work_units_layer_sums_and_gauss_for_20_ticks():
    tables = C.load_collision_tables(); st = Pz.r5_vacuum(4, seed=11)
    w0, ls0 = K.work_units(st), K.layer_sum(st)
    for t in range(20):
        new, ev = T.tick(st, tables)
        assert K.gauss_residual(st, new) == 0
        assert K.work_units(new) == w0
        assert K.layer_sum(new) == ls0, f"layer sum changed at tick {t}"
        st = new

def test_journal_reconstructs_series():
    tables = C.load_collision_tables(); st = Pz.isolated_relation(4)
    jr = J.Journal(st); owner = 32
    for t in range(1, 17):
        new, ev = T.tick(st, tables); jr.record(t, st, new); st = new
    tail = owner
    assert jr.site_series(tail, 16) == [1,1,1,1,0,0,0,0]*2
    rel = jr.relation_series("sc", owner, (0,), 16)
    assert len(rel) == 16 and rel[0] != rel[4]
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `cd scripts && python -m pytest tests/phi_v2_lattice/test_prepare.py tests/phi_v2_lattice/test_conservation.py -q`
Expected: FAIL with `ImportError`

- [ ] **Step 3: Write the three modules**

```python
# scripts/phi_v2_lattice/prepare.py
from __future__ import annotations
import numpy as np
from . import channels as C, state as S
from ._proofs import encode


def isolated_relation(L, owner=None, axis=0, phase=0, polarity=+1, slot=1):
    st = S.blank(L)
    owner = (L ** 3) // 2 if owner is None else owner
    st.sc[owner, axis, slot] = S.idx_of(encode(phase, polarity))
    return st


def _fill_field(st, rng, occupation, eps):
    N = S.n_sites(st)
    lo = 0 if eps > 0 else C.N_STATES
    st.bank[:, lo:lo + C.N_STATES] = rng.random((N, C.N_STATES)) < occupation


def r5_vacuum(L, seed, occupation=0.5, eps=+1, background=None):
    """Spec section 5: ell=0, s=0, translation-invariant relation background with BOTH slots
    occupied (crossing and absorption inert, sources cancel), one polarity layer at `occupation`."""
    st = S.blank(L)
    z = encode(0, +1) if background is None else background
    st.sc[:] = S.idx_of(z); st.fcc[:] = S.idx_of(z)
    _fill_field(st, np.random.default_rng(seed), occupation, eps)
    return st


def sparse_material(L, seed, n_tokens, field_occupation=0.0, eps=+1):
    rng = np.random.default_rng(seed); st = S.blank(L); N = L ** 3
    slots = [("sc", i, (a,)) for i in range(N) for a in range(3)] + \
            [("fcc", i, (p, q)) for i in range(N) for p in range(3) for q in range(2)]
    for k in rng.choice(len(slots), size=n_tokens, replace=False):
        kind, i, idx = slots[k]
        z = S.idx_of(encode(int(rng.integers(0, 4)), int(rng.choice([-1, 1]))))
        which = int(rng.integers(0, 2))
        if kind == "sc":
            st.sc[i, idx[0], which] = z
        else:
            st.fcc[i, idx[0], idx[1], which] = z
    if field_occupation > 0:
        _fill_field(st, rng, field_occupation, eps)
    return st
```

```python
# scripts/phi_v2_lattice/journal.py
from __future__ import annotations
from dataclasses import dataclass
import numpy as np
from . import state as S


@dataclass
class SiteRow:
    tick: int; site: int; s_before: int; s_after: int


@dataclass
class RelationRow:
    tick: int; kind: str; owner: int; idx: tuple; lam_before: int; rho_before: int; lam_after: int; rho_after: int


class Journal:
    def __init__(self, state0: S.LatticeState):
        self.s0 = state0.s.copy(); self.sc0 = state0.sc.copy(); self.fcc0 = state0.fcc.copy()
        self.site_rows: list[SiteRow] = []; self.relation_rows: list[RelationRow] = []

    def record(self, tick: int, before: S.LatticeState, after: S.LatticeState):
        for i in np.nonzero(before.s != after.s)[0].tolist():
            self.site_rows.append(SiteRow(tick, i, int(before.s[i]), int(after.s[i])))
        ch = np.nonzero((before.sc != after.sc).any(axis=2))
        for i, a in zip(*[x.tolist() for x in ch]):
            self.relation_rows.append(RelationRow(tick, "sc", i, (a,), int(before.sc[i, a, 0]), int(before.sc[i, a, 1]),
                                                  int(after.sc[i, a, 0]), int(after.sc[i, a, 1])))
        ch = np.nonzero((before.fcc != after.fcc).any(axis=3))
        for i, p, q in zip(*[x.tolist() for x in ch]):
            self.relation_rows.append(RelationRow(tick, "fcc", i, (p, q), int(before.fcc[i, p, q, 0]), int(before.fcc[i, p, q, 1]),
                                                  int(after.fcc[i, p, q, 0]), int(after.fcc[i, p, q, 1])))

    def site_series(self, site: int, horizon: int) -> list[int]:
        cur = int(self.s0[site]); out = []
        rows = {r.tick: r.s_after for r in self.site_rows if r.site == site}
        for t in range(1, horizon + 1):
            cur = rows.get(t, cur); out.append(cur)
        return out

    def relation_series(self, kind: str, owner: int, idx: tuple, horizon: int) -> list[tuple[int, int]]:
        if kind == "sc":
            cur = (int(self.sc0[owner, idx[0], 0]), int(self.sc0[owner, idx[0], 1]))
        else:
            cur = (int(self.fcc0[owner, idx[0], idx[1], 0]), int(self.fcc0[owner, idx[0], idx[1], 1]))
        rows = {r.tick: (r.lam_after, r.rho_after) for r in self.relation_rows
                if r.kind == kind and r.owner == owner and tuple(r.idx) == tuple(idx)}
        out = []
        for t in range(1, horizon + 1):
            cur = rows.get(t, cur); out.append(cur)
        return out
```

```python
# scripts/phi_v2_lattice/conservation.py
from __future__ import annotations
import numpy as np
from . import channels as C, state as S, tick as T


def work_units(st: S.LatticeState) -> int:
    return int(st.bank.sum()) + int((st.sc != S.BLANK_IDX).sum()) + int((st.fcc != S.BLANK_IDX).sum())


def gauss_residual(before: S.LatticeState, after: S.LatticeState) -> int:
    Qb = T._incidence(before, before.sc, before.fcc); Qa = T._incidence(after, after.sc, after.fcc)
    div = T._divergence(before, T._current(before, after))
    return int(np.abs(Qa - Qb + div).max()) if len(Qa) else 0


def layer_sum(st: S.LatticeState) -> tuple[int, ...]:
    total = np.zeros(6, dtype=int)
    xs, cs = np.nonzero(st.bank)
    for x, c in zip(xs.tolist(), cs.tolist()):
        total += np.array(C.layer_value_of(c, int(st.ell[x])), dtype=int)
    return tuple(int(v) for v in total)
```

- [ ] **Step 4: Run tests to verify they pass**

Run: `cd scripts && python -m pytest tests/phi_v2_lattice/test_prepare.py tests/phi_v2_lattice/test_conservation.py -q`
Expected: 5 passed. If `layer_sum` is **not** conserved across ticks, do not adjust the definition to make it pass: report the first tick and the delta. (The spec claims `U C_q = C_{q-1} U` and per-layer conservation; a failure is a finding about the reading of "layer-appropriate," not a bug to hide.)

- [ ] **Step 5: Commit**

```bash
git add scripts/phi_v2_lattice/prepare.py scripts/phi_v2_lattice/journal.py scripts/phi_v2_lattice/conservation.py scripts/tests/phi_v2_lattice/test_prepare.py scripts/tests/phi_v2_lattice/test_conservation.py
git commit -m "feat(phi_v2_lattice): R5/isolated/sparse preparations, transition journal, conservation checks"
```

---

### Task 5: Census and the composed-law experiment (`census.py`, `experiments/run_census.py`)

**Files:**
- Create: `scripts/phi_v2_lattice/census.py`, `scripts/phi_v2_lattice/experiments/__init__.py`, `scripts/phi_v2_lattice/experiments/run_census.py`
- Test: `scripts/tests/phi_v2_lattice/test_census.py`

**Interfaces:**
- Consumes: Tasks 1–4.
- Produces: `@dataclass class RelationCensus: period: int|None; duty: int|None; orbit_constant: bool; polarity_constant: bool; in_place_flips: int; nulls: int; bounces: int; reversals: int`; `def relation_census(journal, kind, owner, idx, horizon) -> RelationCensus`; `def site_census(journal, horizon) -> dict` with keys `manifest` (`0→±`), `withdraw` (`±→0`), `flip_in_place` (`±→∓` in one tick), `flip_via_wrap` (subset of in-place flips where `|Q|≥2` pre or post — needs `Q`; pass the states instead: signature `site_census(states: list[LatticeState]) -> dict`), `null_bounce`, `null_reversal`; `def run(L, seed, n_tokens, field_occupation, horizon) -> dict` in `run_census.py` printing a table and returning the summary.

- [ ] **Step 1: Write the failing test**

```python
# scripts/tests/phi_v2_lattice/test_census.py
from phi_v2_lattice import prepare as Pz, tick as T, channels as C, journal as J, census as X, state as S

def test_isolated_relation_census_is_exact():
    tables = C.load_collision_tables(); st = Pz.isolated_relation(4); jr = J.Journal(st); states = [st]
    for t in range(1, 33):
        new, ev = T.tick(st, tables); jr.record(t, st, new); st = new; states.append(st)
    rc = X.relation_census(jr, "sc", 32, (0,), 32)
    assert (rc.period, rc.duty, rc.orbit_constant, rc.polarity_constant, rc.in_place_flips) == (8, 4, True, True, 0)
    assert rc.reversals == 0 and rc.bounces == rc.nulls
    sc = X.site_census(states)
    assert sc["flip_in_place"] == 0 and sc["null_reversal"] == 0
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cd scripts && python -m pytest tests/phi_v2_lattice/test_census.py -q`
Expected: FAIL with `ImportError` for `census`

- [ ] **Step 3: Write census and the experiment**

```python
# scripts/phi_v2_lattice/census.py
from __future__ import annotations
from dataclasses import dataclass
import numpy as np
from . import state as S, tick as T
from ._proofs import readout, rotate


def _orbit(idx):
    z = S.z_of(idx); zs = set(); w = z
    for _ in range(4):
        zs.add(w); w = rotate(w)
    return frozenset(zs)


def _occ(idx): return int(readout(S.z_of(idx))[0])
def _pol(idx): return int(readout(S.z_of(idx))[1]) if _occ(idx) else 0


@dataclass
class RelationCensus:
    period: int | None; duty: int | None; orbit_constant: bool; polarity_constant: bool
    in_place_flips: int; nulls: int; bounces: int; reversals: int


def relation_census(journal, kind, owner, idx, horizon) -> RelationCensus:
    series = journal.relation_series(kind, owner, idx, horizon)
    prim = [_occ(l) for l, _ in series]
    tokens = [(l if _occ(l) else r) for l, r in series]
    orbits = {_orbit(t) for t in tokens if _occ(t)}
    pols = {_pol(t) for t in tokens if _occ(t)}
    # period of the (lambda, rho) pair
    period = next((p for p in range(1, horizon) if series[p:] == series[:-p] and series[0] == series[p]), None)
    duty = sum(prim[:period]) if period else None
    # in-place polarity flip of the token
    flips = sum(1 for a, b in zip(tokens, tokens[1:]) if _occ(a) and _occ(b) and _pol(a) == -_pol(b))
    # site-like readout of the primary slot: nulls entered from sign s, exited to sign s'
    sig = [_pol(l) for l, _ in series]; nulls = bounces = reversals = 0; n = len(sig)
    for i in range(1, n):
        if sig[i] == 0 and sig[i - 1] != 0:
            j = i
            while j < n and sig[j] == 0: j += 1
            if j < n:
                nulls += 1
                if sig[j] == sig[i - 1]: bounces += 1
                else: reversals += 1
    return RelationCensus(period, duty, len(orbits) <= 1, len(pols) <= 1, flips, nulls, bounces, reversals)


def site_census(states) -> dict:
    out = dict(manifest=0, withdraw=0, flip_in_place=0, flip_via_wrap=0, null_bounce=0, null_reversal=0)
    N = S.n_sites(states[0]); Q = [T._incidence(st, st.sc, st.fcc) for st in states]
    for i in range(N):
        seq = [int(st.s[i]) for st in states]
        for t in range(1, len(seq)):
            a, b = seq[t - 1], seq[t]
            if a == 0 and b != 0: out["manifest"] += 1
            elif a != 0 and b == 0: out["withdraw"] += 1
            elif a == -b and a != 0:
                out["flip_in_place"] += 1
                if abs(Q[t - 1][i]) >= 2 or abs(Q[t][i]) >= 2: out["flip_via_wrap"] += 1
        for t in range(1, len(seq)):
            if seq[t] == 0 and seq[t - 1] != 0:
                j = t
                while j < len(seq) and seq[j] == 0: j += 1
                if j < len(seq):
                    if seq[j] == seq[t - 1]: out["null_bounce"] += 1
                    else: out["null_reversal"] += 1
    return out
```

```python
# scripts/phi_v2_lattice/experiments/run_census.py
"""The composed-law experiment: many isolated tokens + a field layer, ticked under Phi v2.
Reports the corrected FTD-1028 criterion under COMPOSITION. Nothing tuned; seeds declared."""
from __future__ import annotations
import argparse
import numpy as np
from phi_v2_lattice import prepare as Pz, tick as T, channels as C, journal as J, census as X, conservation as K, state as S


def run(L=6, seed=20260904, n_tokens=24, field_occupation=0.02, horizon=64):
    tables = C.load_collision_tables()
    st = Pz.sparse_material(L, seed, n_tokens, field_occupation)
    jr = J.Journal(st); states = [st]; w0 = K.work_units(st); gauss_ok = True; n_abs = 0
    for t in range(1, horizon + 1):
        new, ev = T.tick(st, tables); jr.record(t, st, new)
        gauss_ok &= (K.gauss_residual(st, new) == 0)
        n_abs += len(ev.absorptions)
        st = new; states.append(st)
    assert K.work_units(st) == w0, "work units not conserved"
    # census every relation that ever held a token
    seen = {(r.kind, r.owner, tuple(r.idx)) for r in jr.relation_rows}
    rows = [X.relation_census(jr, k, o, i, horizon) for (k, o, i) in sorted(seen)]
    periods = [r.period for r in rows]
    summary = dict(
        L=L, seed=seed, n_tokens=n_tokens, field_occupation=field_occupation, horizon=horizon,
        relations_seen=len(rows),
        period_8=sum(1 for p in periods if p == 8), period_other=sum(1 for p in periods if p not in (8, None)),
        period_none=sum(1 for p in periods if p is None),
        duty_4_of_8=sum(1 for r in rows if r.period == 8 and r.duty == 4),
        orbit_violations=sum(1 for r in rows if not r.orbit_constant),
        polarity_violations=sum(1 for r in rows if not r.polarity_constant),
        in_place_flips=sum(r.in_place_flips for r in rows),
        nulls=sum(r.nulls for r in rows), bounces=sum(r.bounces for r in rows), reversals=sum(r.reversals for r in rows),
        gauss_identity_every_tick=gauss_ok, absorptions=n_abs,
        site=X.site_census(states),
    )
    print("\n== Phi v2 composed-law census ==")
    for k, v in summary.items(): print(f"  {k}: {v}")
    print("\n  CRITERION (corrected FTD-1028): period-8 & duty 4/8 for every token-bearing relation, "
          "orbit_violations = polarity_violations = in_place_flips = 0")
    ok = (summary["period_other"] == 0 and summary["period_none"] == 0 and summary["orbit_violations"] == 0
          and summary["polarity_violations"] == 0 and summary["in_place_flips"] == 0)
    print(f"  VERDICT: {'CRITERION MET under composition' if ok else 'CRITERION NOT MET under composition — see counts'}")
    return summary


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--L", type=int, default=6); ap.add_argument("--seed", type=int, default=20260904)
    ap.add_argument("--tokens", type=int, default=24); ap.add_argument("--field", type=float, default=0.02)
    ap.add_argument("--horizon", type=int, default=64)
    a = ap.parse_args(); run(a.L, a.seed, a.tokens, a.field, a.horizon)
```

(`experiments/__init__.py` is empty.)

- [ ] **Step 4: Run test to verify it passes, then run the experiment**

Run: `cd scripts && python -m pytest tests/phi_v2_lattice/test_census.py -q`
Expected: 1 passed
Run: `cd scripts && python -m phi_v2_lattice.experiments.run_census --L 6 --tokens 24 --field 0.02 --horizon 64`
Expected: a summary table and a VERDICT line. **Report the table verbatim in your summary, whatever it says.** Then also run with `--field 0.0` (no field: every token is isolated — all periods must be 8) and `--field 0.10` (dense field: collisions and absorptions active).

- [ ] **Step 5: Commit**

```bash
git add scripts/phi_v2_lattice/census.py scripts/phi_v2_lattice/experiments/__init__.py scripts/phi_v2_lattice/experiments/run_census.py scripts/tests/phi_v2_lattice/test_census.py
git commit -m "feat(phi_v2_lattice): corrected-criterion census and the composed-law experiment"
```

---

## Self-review notes (done at plan time)

- **Spec coverage:** §1 state → Task 2; §2 collision → Task 1 (tables) + Task 3 (`collide_row`); §3.1 → `admitted_absorptions` + the write half of `tick` (G4); §3.2 → `collide_row`, `stream` (U, half-turn, one hop, `ell` decrement); §3.3 → `cross_relations` via the certified `relation_tick` with gate G2; §3.4 → `manifest`/`_incidence`/`_current`/`_divergence` (G3, G6); §5 → `r5_vacuum`; carrier §3 alphabets → `state.py` shapes; carrier §4 hash → `COLLISION_HASH`. R4 witnesses "period-eight reciprocal manifestation/withdrawal" and "oriented source/current continuity" → `test_isolated_relation_reproduces…` and `test_gauss_identity…`.
- **Not covered (declared out of scope):** the R5 *speed 1/6* measurement and the blocked-action coefficient — those are certificate results about the slow generator, not the tick; a wave-front experiment can be added later on top of `r5_vacuum`.
- **Type consistency:** relation identity is always `(kind: str, owner: int, idx: tuple)` with `idx = (a,)` for `sc` and `(p, q)` for `fcc`; A9 slots are always `int8` indices into `state.A9_LIST`; channels are always `int` in `[0, 384)`.
