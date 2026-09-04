"""LatticeState: (s, ell, bank) per site; (lambda, rho) per SC edge and per FCC diagonal."""
from __future__ import annotations
from dataclasses import dataclass
import numpy as np
from ._proofs import A9, BLANK

A9_LIST = tuple(A9)
A9_INDEX = {z: i for i, z in enumerate(A9_LIST)}
BLANK_IDX = A9_INDEX[BLANK]


def z_of(idx: int):
    return A9_LIST[int(idx)]


def idx_of(z) -> int:
    return A9_INDEX[tuple(z)]


@dataclass
class LatticeState:
    L: int
    s: np.ndarray      # int8 (N,)
    ell: np.ndarray    # int8 (N,)   collision layer in {0,1,2}
    bank: np.ndarray   # bool (N,384)
    sc: np.ndarray     # int8 (N,3,2)      [site, axis, slot]        slot 0 = lambda, 1 = rho
    fcc: np.ndarray    # int8 (N,3,2,2)    [site, plane, diag, slot]


def n_sites(st: LatticeState) -> int:
    return st.L ** 3


def blank(L: int) -> LatticeState:
    N = L ** 3
    return LatticeState(
        L=L,
        s=np.zeros(N, dtype=np.int8),
        ell=np.zeros(N, dtype=np.int8),
        bank=np.zeros((N, 384), dtype=bool),
        sc=np.full((N, 3, 2), BLANK_IDX, dtype=np.int8),
        fcc=np.full((N, 3, 2, 2), BLANK_IDX, dtype=np.int8),
    )


def copy(st: LatticeState) -> LatticeState:
    return LatticeState(st.L, st.s.copy(), st.ell.copy(), st.bank.copy(), st.sc.copy(), st.fcc.copy())
