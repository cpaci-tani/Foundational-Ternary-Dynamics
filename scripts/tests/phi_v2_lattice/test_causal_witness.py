"""A regression recording the baseline law's composed-tick support failure."""
import numpy as np
from phi_v2_lattice import channels as C, geometry as G, state as S, tick as T


def test_baseline_admission_collision_streaming_has_distance_two_dependency():
    a = S.blank(7)
    x = G.site_index(7, 3, 3, 3)
    y = G.site_index(7, 4, 3, 3)
    z = G.site_index(7, 2, 3, 3)
    a.bank[x, [0, 34]] = True
    b = S.copy(a)
    b.bank[y, 2] = True
    tables = C.load_collision_tables()
    assert tables[0][(0, 34)] == (2, 32)
    aa, ae = T.tick(a, tables)
    bb, be = T.tick(b, tables)
    assert np.flatnonzero(aa.bank[z]).tolist() == [113]
    assert np.flatnonzero(bb.bank[z]).tolist() == [115]
    assert ae.absorptions == [(x, 34, x, 0)]
    assert be.absorptions == []
    assert max(abs(u - v) for u, v in zip(G.coords(7, y), G.coords(7, z))) == 2
