"""LatticeState of the successor: (s, bank[192]) per site; A9 relation records unchanged."""
from __future__ import annotations
from dataclasses import dataclass
import numpy as np
from ..state import BLANK_IDX, z_of, idx_of  # noqa: F401  (re-exported for the hydro modules)
from .channels import N_CHANNELS
from ..staged import _plain_integer


@dataclass
class LatticeState:
    L: int
    s: np.ndarray      # int8 (N,)
    bank: np.ndarray   # bool (N,192): c = pol*96 + k*24 + v
    sc: np.ndarray     # int8 (N,3,2)
    fcc: np.ndarray    # int8 (N,3,2,2)


def n_sites(st: LatticeState) -> int:
    return _plain_integer(st.L, "periodic lattice side", 3) ** 3


def blank(L: int) -> LatticeState:
    L = _plain_integer(L, "periodic lattice side", 3)
    N = L ** 3
    return LatticeState(L, np.zeros(N, np.int8), np.zeros((N, N_CHANNELS), bool),
                        np.full((N, 3, 2), BLANK_IDX, np.int8), np.full((N, 3, 2, 2), BLANK_IDX, np.int8))


def copy(st: LatticeState) -> LatticeState:
    return LatticeState(st.L, st.s.copy(), st.bank.copy(), st.sc.copy(), st.fcc.copy())
