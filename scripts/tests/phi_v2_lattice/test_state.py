import numpy as np
from phi_v2_lattice import state as S
from phi_v2_lattice._proofs import BLANK, encode, rotate

def test_a9_index_roundtrip_and_blank():
    assert len(S.A9_LIST) == 9
    for i in range(9): assert S.idx_of(S.z_of(i)) == i
    assert S.z_of(S.BLANK_IDX) == BLANK

def test_blank_state_shapes():
    st = S.blank(3); N = 27
    assert st.s.shape == (N,) and st.ell.shape == (N,) and st.bank.shape == (N, 384)
    assert st.sc.shape == (N, 3, 2) and st.fcc.shape == (N, 3, 2, 2)
    assert (st.sc == S.BLANK_IDX).all() and (st.fcc == S.BLANK_IDX).all() and not st.bank.any()

def test_copy_is_deep():
    st = S.blank(3); cp = S.copy(st)
    cp.sc[0, 0, 1] = S.idx_of(encode(0, +1))
    assert st.sc[0, 0, 1] == S.BLANK_IDX
