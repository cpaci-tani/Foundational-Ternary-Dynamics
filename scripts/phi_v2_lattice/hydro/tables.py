"""Deterministic G-equivariant collision involution on the 24-velocity occupancy sets.

Class key (n, P): particle number and 3-momentum. Orbits under the 96-element group
are bucketed by class and by the conjugacy class of their stabilizer, paired in
canonical order, and extended equivariantly. Unpaired orbits are fixed. No parameter
is fitted; the collision efficiency is a measured property of the rule.

Correction to the pairing search (found by exhaustive verify_table, not a rule change):
stabilizer conjugacy alone underdetermines the conjugating element g for a pair
(a, b) -- several g in G satisfy g.Stab(b).g^-1 == Stab(a) (a coset of N_G(Stab(a))),
and a non-identity g in general rotates the momentum vector, so not every such g
sends rep_b to a state with rep_a's exact momentum. F(a) = g.b must conserve momentum,
so build_table additionally requires the momentum of g.rep_b to match rep_a's before
accepting g. This is a bug fix in *which* g realizes "find g with g.Stab(b).g^-1 =
Stab(a)", not a change to the pairing rule itself; see build_table for the worked
counterexample that exposed it.
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


def build_table() -> np.ndarray:
    pop, P = state_arrays()
    rep, g_of = canonical_orbits()
    reps = np.unique(rep)
    key = (pop[reps].astype(np.int64) << 24) | ((P[reps, 0].astype(np.int64) + 8) << 16) \
        | ((P[reps, 1].astype(np.int64) + 8) << 8) | (P[reps, 2].astype(np.int64) + 8)
    stab = stabilizer_rows(reps)
    conj_hi, conj_lo = conjugacy_keys(stab)
    order = np.lexsort((reps, conj_lo, conj_hi, key))
    partner_member = np.array(reps, dtype=np.uint32)   # default: fixed orbit
    pairs_a, pairs_b = [], []
    i = 0
    while i < len(order) - 1:
        p, q = order[i], order[i + 1]
        if key[p] == key[q] and conj_hi[p] == conj_hi[q] and conj_lo[p] == conj_lo[q]:
            pairs_a.append(p); pairs_b.append(q); i += 2
        else:
            i += 1
    pairs_a = np.array(pairs_a, dtype=np.int64); pairs_b = np.array(pairs_b, dtype=np.int64)
    a_states, b_states = reps[pairs_a], reps[pairs_b]
    g_pair = np.full(len(pairs_a), -1, dtype=np.int64)
    for g in range(_G):
        # membership of h in g Stab(b) g^-1 equals stab_b[g^-1 h g]; CONJ[INVERSE[g]] indexes that
        conj_b = stab[pairs_b][:, H.CONJ[H.INVERSE[g]]]
        # Stabilizer conjugacy alone underdetermines g: several g in G satisfy
        # g.Stab(b).g^-1 == Stab(a) (they form a coset of N_G(Stab(a))), and not all
        # of them send rep_b to a state with the same momentum vector as rep_a (a
        # non-identity g in general ROTATES the momentum vector; conjugating stabilizers
        # says nothing about that rotation). F(a) = g.b must additionally conserve
        # momentum exactly, so among the stabilizer-conjugating candidates we require
        # the momentum of g.rep_b to equal the momentum of rep_a (== momentum of rep_b,
        # since both share the same class key) before accepting g. Verified against the
        # brief's own worked example ({+x w0,+y w0,+z w1} / {edge(1,1,0), +z w1}): the
        # unfiltered first match (ascending g) violates momentum; g=59 (present under
        # both CONJ conventions) is the smallest that satisfies both conditions.
        cand = apply_perm(g, b_states)
        momentum_ok = (P[cand] == P[a_states]).all(axis=1)
        match = (conj_b == stab[pairs_a]).all(axis=1) & momentum_ok & (g_pair < 0)
        g_pair[match] = g
    assert (g_pair >= 0).all(), "conjugate, momentum-matching stabilizers must be conjugate by some g"
    partner_member[pairs_a] = apply_perm(g_pair, b_states)                # F(a) = g . b
    partner_member[pairs_b] = apply_perm(H.INVERSE[g_pair], a_states)     # F(b) = g^-1 . a
    lookup = np.zeros(N, dtype=np.uint32)
    lookup[reps] = partner_member
    base = lookup[rep]                                                    # partner member of each state's rep
    return apply_perm(g_of.astype(np.int64), base)                        # F(g . rep) = g . partner


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
