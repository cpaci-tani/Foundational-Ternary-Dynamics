import numpy as np
from phi_v2_lattice import prepare as Pz, tick as T, channels as C, conservation as K, journal as J, state as S

def test_r5_vacuum_conserves_work_units_layer_sums_and_gauss_for_20_ticks():
    tables = C.load_collision_tables(); st = Pz.r5_vacuum(4, seed=11)
    w0, ls0 = K.work_units(st), K.layer_sum(st)
    for t in range(20):
        new, ev = T.tick(st, tables)
        assert K.gauss_residual(st, new, ev) == 0
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
