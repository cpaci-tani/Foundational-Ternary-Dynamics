from phi_v2_lattice import geometry as G

def test_shift_is_periodic_and_invertible():
    L = 4
    for i in range(L**3):
        for a in range(3):
            j = G.shift(L, i, G.E[a]); assert G.shift(L, j, tuple(-x for x in G.E[a])) == i
    assert G.shift(L, G.site_index(L, 3, 0, 0), (1, 0, 0)) == G.site_index(L, 0, 0, 0)

def test_every_site_is_tail_of_nine_and_head_of_nine():
    L = 4
    for i in range(L**3):
        rels = G.relations_at(L, i)
        assert len(rels) == 18
        assert sum(1 for r in rels if r[3] == +1) == 9
        assert sum(1 for r in rels if r[3] == -1) == 9

def test_endpoints_are_consistent_with_relations_at():
    L = 4
    for i in range(L**3):
        for kind, owner, idx, role in G.relations_at(L, i):
            tail, head = (G.sc_endpoints(L, owner, *idx) if kind == 'sc' else G.fcc_endpoints(L, owner, *idx))
            assert (tail if role == +1 else head) == i

def test_sc_edge_of_maps_both_endpoints_to_the_same_edge():
    L = 4
    for x in range(L**3):
        for a in range(3):
            d = G.E[a]; md = tuple(-v for v in d)
            assert G.sc_edge_of(L, x, d) == (x, a)
            assert G.sc_edge_of(L, G.shift(L, x, d), md) == (x, a)
