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
        """s0 at index 0 (the initial state, before any tick), then the state after each of
        ticks 1..horizon: horizon + 1 entries total."""
        cur = int(self.s0[site]); out = [cur]
        rows = {r.tick: r.s_after for r in self.site_rows if r.site == site}
        for t in range(1, horizon + 1):
            cur = rows.get(t, cur); out.append(cur)
        return out

    def relation_series(self, kind: str, owner: int, idx: tuple, horizon: int) -> list[tuple[int, int]]:
        """sc0/fcc0 at index 0 (the initial state, before any tick), then the state after each of
        ticks 1..horizon: horizon + 1 entries total."""
        if kind == "sc":
            cur = (int(self.sc0[owner, idx[0], 0]), int(self.sc0[owner, idx[0], 1]))
        else:
            cur = (int(self.fcc0[owner, idx[0], idx[1], 0]), int(self.fcc0[owner, idx[0], idx[1], 1]))
        rows = {r.tick: (r.lam_after, r.rho_after) for r in self.relation_rows
                if r.kind == kind and r.owner == owner and tuple(r.idx) == tuple(idx)}
        out = [cur]
        for t in range(1, horizon + 1):
            cur = rows.get(t, cur); out.append(cur)
        return out
