import numpy as np
from phi_v2_lattice import prepare as Pz, state as S, channels as C
from phi_v2_lattice._proofs import encode, readout

def test_isolated_relation_places_one_token():
    st = Pz.isolated_relation(4, phase=1, polarity=-1)
    occ = (st.sc != S.BLANK_IDX).sum() + (st.fcc != S.BLANK_IDX).sum()
    assert occ == 1 and not st.bank.any() and not st.s.any()

def test_r5_vacuum_has_inert_relations_and_one_polarity_layer():
    st = Pz.r5_vacuum(4, seed=7)
    N = 64
    assert (st.s == 0).all() and (st.ell == 0).all()
    assert (st.sc != S.BLANK_IDX).all() and (st.fcc != S.BLANK_IDX).all()      # both slots occupied everywhere
    assert not st.bank[:, C.N_STATES:].any()                                  # conjugate polarity blank
    frac = st.bank[:, :C.N_STATES].mean()
    assert 0.45 < frac < 0.55

def test_sparse_material_token_count():
    st = Pz.sparse_material(4, seed=3, n_tokens=5)
    assert (st.sc != S.BLANK_IDX).sum() + (st.fcc != S.BLANK_IDX).sum() == 5
