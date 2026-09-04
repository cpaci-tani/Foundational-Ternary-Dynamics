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
from ._proofs import BLANK, readout, rotate, encode, relation_tick, phase_index

_N = C.N_STATES


@dataclass
class TickEvents:
    absorptions: list = field(default_factory=list)   # (x, channel, owner, axis)
    collisions: list = field(default_factory=list)    # (site, eps, before_pair, after_pair)
    crossings: list = field(default_factory=list)     # (kind, owner, idx, direction)
    gate_holds: list = field(default_factory=list)    # (kind, owner, idx): relations whose lone token
                                                       # sat at phase 0 but whose crossing was blocked
                                                       # by an odd gate this tick


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
        if out[y, c2]:
            raise RuntimeError(f"streaming write collision (spec says impossible): ({x}, {c}) -> ({y}, {c2})")
        out[y, c2] = True
    return out


# ---------------------------------------------------------------- 3.3 crossing
def gate(st: S.LatticeState, tail: int, head: int) -> int:
    total = int(st.bank[tail].sum()) + int(st.bank[head].sum())          # G2, pre-tick banks
    return 1 if total % 2 == 0 else 0


def _cross_pair(st, lam_idx, rho_idx, tail, head):
    lam, rho = S.z_of(lam_idx), S.z_of(rho_idx)
    lam_occ, rho_occ = readout(lam)[0], readout(rho)[0]
    one_owned = bool(lam_occ) != bool(rho_occ)
    token = lam if lam_occ else rho
    at_phase0 = one_owned and phase_index(token) == 0
    g = gate(st, tail, head)
    lam2, rho2 = relation_tick(lam, rho, even_gate=bool(g))
    direction = 0
    if readout(lam)[0] and not readout(lam2)[0]:
        direction = +1
    elif not readout(lam)[0] and readout(lam2)[0]:
        direction = -1
    held = at_phase0 and not g
    return S.idx_of(lam2), S.idx_of(rho2), direction, held


def cross_relations(st: S.LatticeState, absorbing: set):
    new_sc = st.sc.copy(); new_fcc = st.fcc.copy(); events = []; holds = []
    N = S.n_sites(st)
    for i in range(N):
        for a in range(3):
            if (i, a) in absorbing:
                continue
            tail, head = G.sc_endpoints(st.L, i, a)
            l2, r2, dirn, held = _cross_pair(st, st.sc[i, a, 0], st.sc[i, a, 1], tail, head)
            new_sc[i, a, 0], new_sc[i, a, 1] = l2, r2
            if dirn:
                events.append(("sc", i, (a,), dirn))
            if held:
                holds.append(("sc", i, (a,)))
        for p in range(3):
            for q in range(2):
                tail, head = G.fcc_endpoints(st.L, i, p, q)
                l2, r2, dirn, held = _cross_pair(st, st.fcc[i, p, q, 0], st.fcc[i, p, q, 1], tail, head)
                new_fcc[i, p, q, 0], new_fcc[i, p, q, 1] = l2, r2
                if dirn:
                    events.append(("fcc", i, (p, q), dirn))
                if held:
                    holds.append(("fcc", i, (p, q)))
    return new_sc, new_fcc, events, holds


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
    new_sc, new_fcc, crossings, holds = cross_relations(st, absorbing)
    ev.crossings.extend(crossings)
    ev.gate_holds.extend(holds)
    # 3.1 (write half): the absorbed token lands as (lambda', rho') = (BLANK, R z)
    for (owner, axis), (x, c) in adm.items():
        z = encode(2, C.polarity(c))
        new_sc[owner, axis, 0] = S.BLANK_IDX
        new_sc[owner, axis, 1] = S.idx_of(rotate(z))
    # 3.4 manifestation from post-crossing primaries
    new_s = manifest(st, new_sc, new_fcc)
    return S.LatticeState(st.L, new_s, new_ell, new_bank, new_sc, new_fcc), ev
