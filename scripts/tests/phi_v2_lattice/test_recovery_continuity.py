from dataclasses import FrozenInstanceError, replace
from fractions import Fraction

import numpy as np
import pytest

from phi_v2_lattice import channels as C, geometry as G, staged as P, state as S
from phi_v2_lattice import recovery_continuity as R, recovery_kinetic_reference as K


@pytest.mark.parametrize("polarity", (1, -1))
@pytest.mark.parametrize("direction", ((1, 0, 0), (-1, 0, 0), (0, -1, 0), (0, 0, -1)))
def test_admission_owner_current_including_negative_seam(polarity, direction):
    state = P.initialize(S.blank(3))
    channel = next(c for c in range(384) if C.phase(c) == 2
                   and C.polarity(c) == polarity and C.tangent(c) == direction)
    state.lattice.bank[0, channel] = True
    frozen = P.checkpoint(state)
    after, flow = R.advance_observed(state)
    assert P.checkpoint(state) == frozen
    assert not after.lattice.bank.any()
    assert flow.hop_count == int(-1 in direction)
    values = tuple(Fraction(i*i, 7) for i in range(27))
    left, right = flow.weak_balance(values, polarity)
    assert left == right
    if flow.hop_count:
        assert left == values[G.shift(3, 0, direction)] - values[0]
        with pytest.raises(ValueError, match="continuity"):
            replace(flow, transfers=())


@pytest.mark.parametrize("preparation", ("empty", "collision", "dense", "both_polarities"))
def test_all_stages_exact_local_and_weak_balance(preparation):
    bank = np.zeros((27, 384), dtype=bool)
    if preparation == "collision":
        bank[13, [0, 32]] = True
    elif preparation == "dense":
        bank[:, ::3] = True
    elif preparation == "both_polarities":
        bank[[0, 26], [0, 383]] = True
    state = K.prepare_bank(bank, 3)
    initial = P.work_units(state)
    for tick in range(8):
        expected, _ = P.step(state)
        state, flow = R.advance_observed(state)
        assert P.checkpoint(state) == P.checkpoint(expected)
        assert sum(map(sum, flow.after)) == initial
        assert flow.start_tick == tick
        for polarity in (1, -1):
            a, b = flow.weak_balance(tuple(Fraction(i % 5 - 2, 3) for i in range(27)), polarity)
            assert a == b
        if tick % 4 != 2:
            assert not flow.transfers
        with pytest.raises(FrozenInstanceError):
            flow.start_tick = 0


def test_reports_reject_corrupted_balance_stage_duplicates_and_mutability():
    bank = np.zeros((27, 384), dtype=bool)
    bank[0, 0] = True
    state = K.phase_lift(K.prepare_bank(bank, 3), 2)
    _, flow = R.advance_observed(state)
    with pytest.raises(ValueError):
        replace(flow, start_tick=1)
    with pytest.raises(ValueError):
        replace(flow, transfers=flow.transfers*2)
    with pytest.raises(ValueError):
        replace(flow, before=list(flow.before))
    with pytest.raises(ValueError):
        replace(flow, transfers=(replace(flow.transfers[0], count=-1),))
    with pytest.raises(ValueError):
        replace(flow, transfers=(replace(flow.transfers[0], polarity=1.0),))
    with pytest.raises(ValueError):
        flow.weak_balance([0]*27, polarity=1.0)
    with pytest.raises(ValueError):
        flow.weak_balance([0.5]*27)
    with pytest.raises(ValueError):
        flow.weak_balance([0]*26)


def test_finite_scale_taylor_bound_exact_arithmetic_and_ballistic_refinement():
    # M normalized tokens, H=T/tau opportunities: a/tau=1, T=B=1.
    for resolution in (4, 8, 16, 32):
        assert R.weak_current_error_bound(7*resolution, Fraction(1, resolution), 1, 7) == Fraction(1, 2*resolution)
    assert R.weak_current_error_bound(0, 1, 4) == 0
    for args in ((-1, 1, 1), (True, 1, 1), (1, 0, 1), (1, .1, 1), (1, 1, -1), (1, 1, 1, 0)):
        with pytest.raises(ValueError):
            R.weak_current_error_bound(*args)


def test_supplied_algebraic_reports_canonicalize_numpy_integers_without_overflow():
    # Mathematical reports are not runtime-authenticated finite-alphabet states.
    huge = np.int64(2**63-1)
    before = [(0, 0)]*27
    after = [(0, 0)]*27
    before[0] = before[1] = (huge, 0)
    after[9] = after[10] = (huge, 0)
    edges = tuple(R.Transfer(np.int64(x), (np.int64(1), 0, 0), np.int64(1), huge, "stream") for x in (0, 1))
    flow = R.ContinuityStep(np.int64(3), np.int64(2), tuple(before), tuple(after), edges)
    assert flow.hop_count == 2*(2**63-1)
    assert type(flow.hop_count) is int
    assert flow.weak_balance(range(27)) == (18*(2**63-1),)*2
    assert R.weak_current_error_bound(flow.hop_count, 1, 1) == 2**63-1
