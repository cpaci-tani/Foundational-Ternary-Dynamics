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
        gauss_ok &= (K.gauss_residual(st, new, ev) == 0)
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
