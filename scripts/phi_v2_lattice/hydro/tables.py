"""Deterministic G-equivariant collision involution on the 24-velocity occupancy sets.

Class key (n, P): particle number and 3-momentum. Spec A.2's rule pairs an unpaired
orbit A with the first later unpaired orbit *of the same class* containing a member B
with Stab(B) == Stab(A) exactly -- "of the same class" means the *orbit* contains a
member whose momentum equals A's, not that the orbit's canonical representative
(the numerically smallest 24-bit encoding chosen by canonical_orbits()) happens to
carry that momentum itself; a representative's own momentum is not a rotation
invariant, so two orbits genuinely pairable under the rule can have representatives
with differently oriented momentum vectors.

build_table() therefore buckets canonical orbit representatives by particle number,
by the *rotation-orbit* of the representative's momentum under the 48 signed
permutations (momentum_orbit_key -- an O_h orbit invariant: sorted absolute
component values; the w-flip does not move momentum), and by the conjugacy class of
the representative's stabilizer. These three keys are each necessary conditions for
admissibility and cannot themselves discard a genuinely admissible pair. Within a
bucket, every unpaired orbit representative is tested in canonical order against
every later unpaired representative's full 96-element orbit membership for a member
whose momentum matches exactly and whose stabilizer, conjugated back by the group
element that reaches it, equals the first orbit's stabilizer exactly; the first
admissible partner found is paired (F(a) = g.b, F(b) = g^-1.a) and both orbits are
consumed. An orbit with no admissible partner anywhere in its bucket is fixed. No
parameter is fitted; the collision efficiency is a measured property of the rule.

unpaired_admissible() is an independent completeness check: it re-derives, from the
orbits build_table() actually left fixed, whether any pair of them still admits a
partner under the rule's literal condition. It must return 0.
"""
from __future__ import annotations

import hashlib
from pathlib import Path

import numpy as np

from . import channels as H

N = 1 << H.N_VEL
_G = len(H.GROUP)


def _byte_tables():
    """T[g, byte_index, byte_value] = permuted bits contributed by that byte."""
    tables = np.zeros((_G, 3, 256), dtype=np.uint32)
    for g in range(_G):
        for byte_index in range(3):
            for value in range(256):
                image = 0
                for bit in range(8):
                    if value >> bit & 1:
                        image |= 1 << int(H.PERM[g, byte_index * 8 + bit])
                tables[g, byte_index, value] = image
    return tables


_BYTES = _byte_tables()


def apply_perm(g, states: np.ndarray) -> np.ndarray:
    """g may be an int or an int array aligned with `states`."""
    states = np.asarray(states, dtype=np.uint32)
    b0, b1, b2 = states & 0xFF, (states >> 8) & 0xFF, (states >> 16) & 0xFF
    return _BYTES[g, 0, b0] | _BYTES[g, 1, b1] | _BYTES[g, 2, b2]


def state_arrays():
    states = np.arange(N, dtype=np.uint32)
    pop = np.zeros(N, dtype=np.uint8)
    P = np.zeros((N, 3), dtype=np.int8)
    for c, v in enumerate(H.VELOCITIES):
        bit = ((states >> c) & 1).astype(np.int8)
        pop += bit.astype(np.uint8)
        for axis in range(3):
            if v[axis]:
                P[:, axis] += bit * np.int8(v[axis])
    return pop, P


def momentum_of(state: int) -> tuple[int, int, int]:
    total = [0, 0, 0]
    for c, v in enumerate(H.VELOCITIES):
        if state >> c & 1:
            for axis in range(3):
                total[axis] += v[axis]
    return tuple(total)


def canonical_orbits():
    """rep[s] = min over g of g.s; g_of[s] = an element with g_of[s].rep[s] == s."""
    states = np.arange(N, dtype=np.uint32)
    rep = states.copy()
    for g in range(1, _G):
        rep = np.minimum(rep, apply_perm(g, states))
    g_of = np.full(N, 255, dtype=np.uint8)
    for g in range(_G):
        hit = (apply_perm(g, rep) == states) & (g_of == 255)
        g_of[hit] = g
    assert (g_of < _G).all()
    return rep, g_of


def stabilizer_rows(reps: np.ndarray) -> np.ndarray:
    rows = np.zeros((len(reps), _G), dtype=bool)
    for g in range(_G):
        rows[:, g] = apply_perm(g, reps) == reps
    return rows


def _pack(rows: np.ndarray):
    """96 bools -> (hi uint64 of bits 0..63, lo uint64 of bits 64..95) for lexicographic order."""
    weights_hi = (np.uint64(1) << np.arange(64, dtype=np.uint64))
    weights_lo = (np.uint64(1) << np.arange(32, dtype=np.uint64))
    hi = (rows[:, :64].astype(np.uint64) * weights_hi).sum(axis=1, dtype=np.uint64)
    lo = (rows[:, 64:].astype(np.uint64) * weights_lo).sum(axis=1, dtype=np.uint64)
    return hi, lo


def conjugacy_keys(stab: np.ndarray):
    """Lexicographically minimal conjugate of each stabilizer, packed as (hi, lo)."""
    best_hi, best_lo = _pack(stab)
    for g in range(1, _G):
        conj = stab[:, H.CONJ[g]]           # conj[:, h] = stab[:, g^-1 h g] ... membership of h in g Stab g^-1
        hi, lo = _pack(conj)
        better = (hi < best_hi) | ((hi == best_hi) & (lo < best_lo))
        best_hi = np.where(better, hi, best_hi)
        best_lo = np.where(better, lo, best_lo)
    return best_hi, best_lo


def momentum_orbit_key(P_rows: np.ndarray) -> np.ndarray:
    """Canonical key of the O_h orbit of each momentum vector: components' absolute values sorted descending."""
    a = np.sort(np.abs(P_rows.astype(np.int64)), axis=1)[:, ::-1]
    return (a[:, 0] << 16) | (a[:, 1] << 8) | a[:, 2]


def build_table() -> np.ndarray:
    pop, P = state_arrays()
    rep, g_of = canonical_orbits()
    reps = np.unique(rep)
    stab = stabilizer_rows(reps)
    conj_hi, conj_lo = conjugacy_keys(stab)
    bucket = (pop[reps].astype(np.int64) << 32) | momentum_orbit_key(P[reps])
    order = np.lexsort((reps, conj_lo, conj_hi, bucket))
    partner_member = np.array(reps, dtype=np.uint32)   # default: fixed orbit
    paired = np.zeros(len(reps), dtype=bool)
    all_g = np.arange(_G)
    conj_index = H.CONJ[H.INVERSE[all_g]]              # row g: membership index for g Stab g^-1 (the convention verified in 8c9ce823)
    start = 0
    while start < len(order):
        end = start
        while (end < len(order) and bucket[order[end]] == bucket[order[start]]
               and conj_hi[order[end]] == conj_hi[order[start]] and conj_lo[order[end]] == conj_lo[order[start]]):
            end += 1
        members = order[start:end]                       # same n, same momentum orbit type, same stabilizer conjugacy class; rep order
        for i, p in enumerate(members):
            if paired[p]:
                continue
            a = reps[p]
            for q in members[i + 1:]:
                if paired[q]:
                    continue
                b = reps[q]
                images = apply_perm(all_g, np.full(_G, b, dtype=np.uint32))          # g.b for every g
                momentum_ok = (P[images] == P[a]).all(axis=1)
                if not momentum_ok.any():
                    continue
                conj_ok = (stab[q][conj_index] == stab[p]).all(axis=1)              # g Stab(b) g^-1 == Stab(a)
                admissible = np.flatnonzero(momentum_ok & conj_ok)
                if len(admissible) == 0:
                    continue
                g = int(admissible[0])
                partner_member[p] = images[g]                                        # F(a) = g.b, same momentum, equal stabilizer
                partner_member[q] = apply_perm(H.INVERSE[g], np.array([a], dtype=np.uint32))[0]   # F(b) = g^-1.a
                paired[p] = paired[q] = True
                break
        start = end
    lookup = np.zeros(N, dtype=np.uint32)
    lookup[reps] = partner_member
    base = lookup[rep]
    return apply_perm(g_of.astype(np.int64), base)


def unpaired_admissible(table: np.ndarray) -> int:
    """Number of pairs of FIXED orbit representatives that the rule would still pair (must be 0)."""
    pop, P = state_arrays()
    rep, _ = canonical_orbits()
    reps = np.unique(rep)
    fixed = reps[table[reps] == reps]
    stab = stabilizer_rows(fixed)
    conj_hi, conj_lo = conjugacy_keys(stab)
    bucket = (pop[fixed].astype(np.int64) << 32) | momentum_orbit_key(P[fixed])
    keys = {}
    for i in range(len(fixed)):
        keys.setdefault((int(bucket[i]), int(conj_hi[i]), int(conj_lo[i])), []).append(i)
    all_g = np.arange(_G)
    conj_index = H.CONJ[H.INVERSE[all_g]]
    count = 0
    for members in keys.values():
        for x in range(len(members)):
            p = members[x]
            for q in members[x + 1:]:
                images = apply_perm(all_g, np.full(_G, fixed[q], dtype=np.uint32))
                momentum_ok = (P[images] == P[fixed[p]]).all(axis=1)
                conj_ok = (stab[q][conj_index] == stab[p]).all(axis=1)
                if (momentum_ok & conj_ok).any():
                    count += 1
    return count


def verify_table(table: np.ndarray) -> dict:
    states = np.arange(N, dtype=np.uint32)
    pop, P = state_arrays()
    report = {"involution": bool((table[table] == states).all()),
              "mass_conserved": bool((pop[table] == pop).all()),
              "momentum_conserved": bool((P[table] == P).all()),
              "fixed_states": int((table == states).sum())}
    ok = True
    for g in range(_G):
        if not (table[apply_perm(g, states)] == apply_perm(g, table)).all():
            ok = False
            break
    report["equivariant"] = ok
    report["fixed_by_particle_number"] = [int(((table == states) & (pop == n)).sum()) for n in range(25)]
    report["states_by_particle_number"] = [int((pop == n).sum()) for n in range(25)]
    report["sha256"] = hashlib.sha256(table.tobytes()).hexdigest()
    return report


def write_table(table: np.ndarray) -> Path:
    digest = hashlib.sha256(table.tobytes()).hexdigest()
    root = Path(__file__).resolve().parents[3]
    directory = root / "engine" / "build_strict_hydro_tables"
    directory.mkdir(parents=True, exist_ok=True)
    path = directory / f"hydro_collision_{digest[:16]}.u32"
    path.write_bytes(table.astype("<u4").tobytes())
    (directory / "hydro_collision.sha256").write_text(digest + "\n", encoding="utf-8")
    return path
