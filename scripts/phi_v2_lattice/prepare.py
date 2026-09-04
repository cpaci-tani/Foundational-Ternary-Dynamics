from __future__ import annotations
import numpy as np
from . import channels as C, state as S
from ._proofs import encode


def isolated_relation(L, owner=None, axis=0, phase=0, polarity=+1, slot=1):
    st = S.blank(L)
    owner = (L ** 3) // 2 if owner is None else owner
    st.sc[owner, axis, slot] = S.idx_of(encode(phase, polarity))
    return st


def _fill_field(st, rng, occupation, eps):
    N = S.n_sites(st)
    lo = 0 if eps > 0 else C.N_STATES
    st.bank[:, lo:lo + C.N_STATES] = rng.random((N, C.N_STATES)) < occupation


def r5_vacuum(L, seed, occupation=0.5, eps=+1, background=None):
    """Spec section 5: ell=0, s=0, translation-invariant relation background with BOTH slots
    occupied (crossing and absorption inert, sources cancel), one polarity layer at `occupation`."""
    st = S.blank(L)
    z = encode(0, +1) if background is None else background
    st.sc[:] = S.idx_of(z); st.fcc[:] = S.idx_of(z)
    _fill_field(st, np.random.default_rng(seed), occupation, eps)
    return st


def sparse_material(L, seed, n_tokens, field_occupation=0.0, eps=+1):
    rng = np.random.default_rng(seed); st = S.blank(L); N = L ** 3
    slots = [("sc", i, (a,)) for i in range(N) for a in range(3)] + \
            [("fcc", i, (p, q)) for i in range(N) for p in range(3) for q in range(2)]
    for k in rng.choice(len(slots), size=n_tokens, replace=False):
        kind, i, idx = slots[k]
        z = S.idx_of(encode(int(rng.integers(0, 4)), int(rng.choice([-1, 1]))))
        which = int(rng.integers(0, 2))
        if kind == "sc":
            st.sc[i, idx[0], which] = z
        else:
            st.fcc[i, idx[0], idx[1], which] = z
    if field_occupation > 0:
        _fill_field(st, rng, field_occupation, eps)
    return st
