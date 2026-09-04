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
