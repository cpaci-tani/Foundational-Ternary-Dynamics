import numpy as np
from phi_v2_lattice import channels as C, geometry as G, state as S, tick as T
from phi_v2_lattice._proofs import BLANK, encode, rotate, readout

def _tables():
    return C.load_collision_tables()

def _isolated(L=4, owner=None, axis=0, phase=0, pol=+1, slot=1):
    st = S.blank(L)
    owner = (L**3)//2 if owner is None else owner
    st.sc[owner, axis, slot] = S.idx_of(encode(phase, pol))
    return st, owner

def test_bal3():
    assert [T.bal3(q) for q in (-3, -2, -1, 0, 1, 2, 3)] == [0, 1, -1, 0, 1, -1, 0]

def test_isolated_relation_reproduces_the_certified_period_eight_square_wave():
    """Lift of the constitution's C15/C16: one token, reserve slot, phase 0, no field."""
    st, owner = _isolated(); tables = _tables()
    tail, head = G.sc_endpoints(st.L, owner, 0)
    seq_tail, seq_head, occ = [], [], []
    st0 = S.copy(st)
    for t in range(16):
        st, ev = T.tick(st, tables)
        seq_tail.append(int(st.s[tail])); seq_head.append(int(st.s[head]))
        occ.append(int(readout(S.z_of(st.sc[owner, 0, 0]))[0]))
    assert occ == [1,1,1,1,0,0,0,0]*2                      # primary owned 4 of 8, exact period 8
    assert seq_tail == [1,1,1,1,0,0,0,0]*2                 # tail site +1 while primary-owned
    assert seq_head == [-1,-1,-1,-1,0,0,0,0]*2             # head site -1: a relation manifests as a dipole
    # after 8 ticks the pair returns exactly
    assert st.sc[owner, 0].tolist() == st0.sc[owner, 0].tolist()

def test_gauss_identity_holds_on_random_states():
    rng = np.random.default_rng(1); tables = _tables(); L = 4; N = L**3
    for _ in range(5):
        st = S.blank(L)
        st.sc[:] = rng.integers(0, 9, size=st.sc.shape).astype(np.int8)
        st.fcc[:] = rng.integers(0, 9, size=st.fcc.shape).astype(np.int8)
        st.bank[:] = rng.random((N, 384)) < 0.01
        st.ell[:] = rng.integers(0, 3, size=N).astype(np.int8)
        Q_before = T._incidence(st, st.sc, st.fcc)
        new, ev = T.tick(st, tables)
        Q_after = T._incidence(new, new.sc, new.fcc)
        J = T._current(st, new)                            # per-relation oriented current
        div = T._divergence(st, J)
        assert np.array_equal(Q_after - Q_before + div, np.zeros(N, dtype=int))
        assert np.array_equal(new.s, np.vectorize(T.bal3)(Q_after))

def test_work_units_are_conserved_by_the_tick():
    rng = np.random.default_rng(2); tables = _tables(); L = 4; N = L**3
    for _ in range(5):
        st = S.blank(L)
        st.sc[:] = rng.choice([S.BLANK_IDX, S.idx_of(encode(2, +1))], size=st.sc.shape, p=[0.7, 0.3]).astype(np.int8)
        st.bank[:] = rng.random((N, 384)) < 0.02
        before = int(st.bank.sum()) + int((st.sc != S.BLANK_IDX).sum()) + int((st.fcc != S.BLANK_IDX).sum())
        new, ev = T.tick(st, tables)
        after = int(new.bank.sum()) + int((new.sc != S.BLANK_IDX).sum()) + int((new.fcc != S.BLANK_IDX).sum())
        assert before == after

def test_streaming_never_write_collides():
    rng = np.random.default_rng(3); L = 4; N = L**3
    st = S.blank(L); st.bank[:] = rng.random((N, 384)) < 0.3
    st.s[:] = rng.integers(-1, 2, size=N).astype(np.int8)     # random manifested sites (half-turn path)
    out = T.stream(st, st.bank)
    assert int(out.sum()) == int(st.bank.sum())               # a permutation of occupied slots

def test_absorption_admits_only_unique_proposals_onto_blank_edges():
    tables = _tables(); L = 4; st = S.blank(L); x = 0
    # one phase-2 channel with tangent +e_0 at x
    c = next(c for c in range(384) if C.phase(c) == 2 and C.tangent(c) == (1, 0, 0) and C.polarity(c) == +1)
    st.bank[x, c] = True
    adm = T.admitted_absorptions(st)
    assert adm == {(x, 0): (x, c)}
    # a competing proposal from the far endpoint (tangent -e_0) makes both fail closed
    y = G.shift(L, x, (1, 0, 0))
    c2 = next(c for c in range(384) if C.phase(c) == 2 and C.tangent(c) == (-1, 0, 0) and C.polarity(c) == +1)
    st.bank[y, c2] = True
    assert T.admitted_absorptions(st) == {}
    # and an occupied slot blocks admission
    st.bank[y, c2] = False; st.sc[x, 0, 1] = S.idx_of(encode(0, +1))
    assert T.admitted_absorptions(st) == {}

def test_absorption_writes_blank_primary_and_rotated_token_to_the_reserve():
    tables = _tables(); L = 4; st = S.blank(L); x = 5
    c = next(c for c in range(384) if C.phase(c) == 2 and C.tangent(c) == (0, 1, 0) and C.polarity(c) == -1)
    st.bank[x, c] = True
    new, ev = T.tick(st, tables)
    assert ev.absorptions == [(x, c, x, 1)]
    assert not new.bank.any()                                  # the channel was cleared, nothing else streamed
    assert S.z_of(new.sc[x, 1, 0]) == BLANK
    assert S.z_of(new.sc[x, 1, 1]) == rotate(encode(2, -1))     # (lambda', rho') = (0, R z)  (C13/C14)

def test_tick_is_a_pure_function_of_the_prestate():
    rng = np.random.default_rng(4); tables = _tables(); L = 3; N = 27
    st = S.blank(L); st.bank[:] = rng.random((N, 384)) < 0.02
    st.sc[:] = rng.integers(0, 9, size=st.sc.shape).astype(np.int8)
    a, _ = T.tick(S.copy(st), tables); b, _ = T.tick(S.copy(st), tables)
    for f in ("s", "ell", "bank", "sc", "fcc"):
        assert np.array_equal(getattr(a, f), getattr(b, f))
