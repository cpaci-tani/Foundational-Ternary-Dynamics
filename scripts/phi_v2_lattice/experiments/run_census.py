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
        seeded=sum(1 for r in rows if r.first_occupied == 0),
        created_mid_horizon=sum(1 for r in rows if r.first_occupied not in (0, None)),
        phase_step_violations=sum(1 for r in rows if not r.phase_steps_ok),
        crossings_not_at_phase0=sum(1 for r in rows if not r.crossings_at_phase0),
        runs_mod4_violations=sum(1 for r in rows if not r.runs_mod4_ok),
        site=X.site_census(states),
    )
    print("\n== Phi v2 composed-law census ==")
    for k, v in summary.items(): print(f"  {k}: {v}")
    a = (summary["orbit_violations"] == 0 and summary["polarity_violations"] == 0
         and summary["in_place_flips"] == 0 and summary["reversals"] == 0)
    b = (summary["phase_step_violations"] == 0 and summary["crossings_not_at_phase0"] == 0
         and summary["runs_mod4_violations"] == 0)
    c = (summary["period_other"] == 0 and summary["period_none"] == 0
         and summary["duty_4_of_8"] == summary["relations_seen"])
    print("\n  CRITERION (corrected FTD-1028), measured from each relation's first occupation:")
    print(f"  A carrier conservation (orbit, polarity, no in-place flip, no reversal): {'MET' if a else 'NOT MET'}")
    print(f"  B clock integrity (phase +1/tick, crossings only at phase 0, interior runs = 0 mod 4): {'MET' if b else 'NOT MET'}")
    print(f"  C exact period 8 / duty 4 of 8 (expected only under an always-even gate): {'MET' if c else 'NOT MET'}")
    verdict = ('CRITERION MET under composition' if (a and b and c)
               else 'A+B MET, C NOT MET under composition — gate-modulated clock, carrier intact' if (a and b)
               else 'CRITERION NOT MET under composition — see counts')
    print(f"  VERDICT: {verdict}")
    return summary


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--L", type=int, default=6); ap.add_argument("--seed", type=int, default=20260904)
    ap.add_argument("--tokens", type=int, default=24); ap.add_argument("--field", type=float, default=0.02)
    ap.add_argument("--horizon", type=int, default=64)
    a = ap.parse_args(); run(a.L, a.seed, a.tokens, a.field, a.horizon)
