from phi_v2_lattice import prepare as Pz, tick as T, channels as C, journal as J, census as X, state as S

def test_isolated_relation_census_is_exact():
    tables = C.load_collision_tables(); st = Pz.isolated_relation(4); jr = J.Journal(st); states = [st]
    for t in range(1, 33):
        new, ev = T.tick(st, tables); jr.record(t, st, new); st = new; states.append(st)
    rc = X.relation_census(jr, "sc", 32, (0,), 32)
    assert (rc.period, rc.duty, rc.orbit_constant, rc.polarity_constant, rc.in_place_flips) == (8, 4, True, True, 0)
    assert rc.reversals == 0 and rc.bounces == rc.nulls
    assert rc.first_occupied == 0 and rc.phase_steps_ok and rc.crossings_at_phase0 and rc.runs_mod4_ok
    sc = X.site_census(states)
    assert sc["flip_in_place"] == 0 and sc["null_reversal"] == 0


def test_absorbed_token_census_is_measured_from_first_occupation():
    """A token CREATED mid-horizon by absorption of a streaming channel: the census must judge it
    from its first occupation, and with no field left the gate is always even -> exact period 8."""
    from phi_v2_lattice import channels as C
    tables = C.load_collision_tables(); st = S.blank(4); x = 5
    c = next(c for c in range(384) if C.phase(c) == 0 and C.polarity(c) == +1)
    st.bank[x, c] = True                       # phase 0 -> streams twice -> phase 2 -> absorbed in tick 3
    jr = J.Journal(st)
    for t in range(1, 25):
        new, ev = T.tick(st, tables); jr.record(t, st, new); st = new
    seen = {(r.kind, r.owner, tuple(r.idx)) for r in jr.relation_rows}
    assert len(seen) == 1
    kind, owner, idx = next(iter(seen))
    rc = X.relation_census(jr, kind, owner, idx, 24)
    assert rc.first_occupied == 3   # index 0 is t=0; the token lands during tick 3
    assert rc.orbit_constant and rc.polarity_constant and rc.in_place_flips == 0 and rc.reversals == 0
    assert rc.phase_steps_ok and rc.crossings_at_phase0 and rc.runs_mod4_ok
    assert (rc.period, rc.duty) == (8, 4)

def test_never_occupied_relation_has_no_period():
    tables = C.load_collision_tables(); st = Pz.isolated_relation(4); jr = J.Journal(st)
    for t in range(1, 9):
        new, ev = T.tick(st, tables); jr.record(t, st, new); st = new
    rc = X.relation_census(jr, "sc", 0, (0,), 8)
    assert rc.period is None and rc.first_occupied is None
