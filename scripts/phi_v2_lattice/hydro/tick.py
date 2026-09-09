"""Field-sector stages of the successor; gates and manifestation reused from phi_v2_lattice.tick."""
from __future__ import annotations
from dataclasses import dataclass, field
import numpy as np
from .. import geometry as G
from ..tick import manifest  # noqa: F401  (unchanged relation sector; re-exported for T.manifest in hydro/staged.py)
from . import channels as H, state as S


@dataclass
class TickEvents:
    absorptions: list = field(default_factory=list)   # (x, c, kind, owner, idx)
    collisions: list = field(default_factory=list)    # (site, pol, mask_before, mask_after)
    crossings: list = field(default_factory=list)
    gate_holds: list = field(default_factory=list)


def admitted_absorptions(st: S.LatticeState):
    """{(kind, owner, idx): (x, c)}: phase-2 channels whose target relation has both slots
    blank and exactly one proposal (both polarities compete). Face channels target SC edges,
    edge channels FCC diagonals."""
    proposals = {}
    xs, cs = np.nonzero(st.bank)
    for x, c in zip(xs.tolist(), cs.tolist()):
        pol, k, v = H.unpack(c)
        if k != 2:
            continue
        kind, offset, idx = H.relation_target(v)
        owner = G.shift(st.L, x, offset)
        proposals.setdefault((kind, owner, idx), []).append((x, c))
    admitted = {}
    for key, plist in proposals.items():
        if len(plist) != 1:
            continue
        kind, owner, idx = key
        slots = st.sc[owner, idx[0]] if kind == "sc" else st.fcc[owner, idx[0], idx[1]]
        if slots[0] != S.BLANK_IDX or slots[1] != S.BLANK_IDX:
            continue
        admitted[key] = plist[0]
    return admitted


def _mask(seg: np.ndarray):
    """Velocity occupancy mask and the phase label of each occupied velocity in canonical order."""
    grid = seg.reshape(4, 24)
    mask, labels = 0, []
    for v in range(24):
        ks = np.nonzero(grid[:, v])[0]
        if len(ks) > 1:
            raise RuntimeError("exclusion violated")
        if len(ks) == 1:
            mask |= 1 << v
            labels.append(int(ks[0]))
    return mask, labels


def collide_site(row: np.ndarray, table: np.ndarray):
    """Table collision per polarity with sorted reassignment of the passive k labels."""
    out = np.zeros_like(row)
    events = []
    for pol in range(2):
        mask, labels = _mask(row[pol * 96:(pol + 1) * 96])
        new = int(table[mask]) if mask else 0
        for k, v in zip(sorted(labels), [v for v in range(24) if new >> v & 1]):
            out[H.channel(pol, k, v)] = True
        if new != mask:
            events.append((pol, mask, new))
    return out, events


def stream(st: S.LatticeState, bank: np.ndarray) -> np.ndarray:
    """Advance every occupied (site, polarity, velocity) channel one passive-phase step.

    A write collision at `out[y, c2]` (two distinct source channels landing on
    the same destination) is impossible by construction. `channel(pol, k2, v)`
    encodes (pol, k2, v) injectively, so two sources sharing a destination
    channel c2 must share the same pol and the same v (k2 is then forced equal
    too, but is not needed for the argument). Two occupied source channels
    with the same pol and v cannot live at the same site x: the exclusion
    invariant maintained on `bank` caps occupancy at one particle per (site,
    polarity, velocity) regardless of phase (see `_mask`, which raises
    "exclusion violated" the moment more than one phase is set for a given
    velocity at a site), so equal (pol, v) forces distinct sources to distinct
    sites x != x'. Streaming then maps each source to y = shift(x, V[v]) with
    the same v for both -- and shift-by-a-fixed-vector is injective (it is a
    lattice translation), so x != x' forces y != y'. Distinct sources with
    equal velocity and polarity are therefore always at distinct sites, and
    distinct sites with the same velocity stream to distinct destinations; no
    two occupied source channels can ever target the same (y, c2).
    """
    out = np.zeros_like(bank)
    xs, cs = np.nonzero(bank)
    for x, c in zip(xs.tolist(), cs.tolist()):
        pol, k, v = H.unpack(c)
        y = G.shift(st.L, x, H.VELOCITIES[v])
        k2 = (k + 1) % 4
        if st.s[x] != 0:
            k2 = (k2 + 2) % 4
        c2 = H.channel(pol, k2, v)
        if out[y, c2]:
            raise RuntimeError("streaming write collision (exclusion makes this impossible)")
        out[y, c2] = True
    return out
