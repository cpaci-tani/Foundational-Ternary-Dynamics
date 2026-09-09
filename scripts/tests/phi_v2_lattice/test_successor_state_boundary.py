"""Closed data domains for successor laws; only small synthetic state controls."""
from dataclasses import replace

import numpy as np
import pytest

from phi_v2_lattice import hydro_parity as H
from phi_v2_lattice import credit_exchange_binding as D
from phi_v2_lattice import sparse_credit_exchange_q4 as S


SAFE_INTEGERS = tuple(dict.fromkeys((int, np.int8, np.int16, np.int32, np.int64,
                                   np.uint8, np.uint16, np.uint32, np.uint64,
                                   np.intp, np.uintp, np.longlong, np.ulonglong)))


def sparse():
    return S.initialize(4, tuple(S.Carrier(site, slot, 1, 1, 0)
                                for site in (21, 42) for slot in (0, 1)), (), 0, 0)


def field():
    bank = np.zeros((4, 4, 4, 2, 24), dtype=bool)
    bank[1, 1, 1, 0, H.VELOCITIES.index((1, 0, 0))] = True
    return H.initialize(bank)


class CallbackArray(np.ndarray):
    calls = 0

    def copy(self, *args, **kwargs):
        type(self).calls += 1
        return super().copy(*args, **kwargs)

    def __array_function__(self, *args, **kwargs):
        type(self).calls += 1
        return super().__array_function__(*args, **kwargs)


def callback_array(array):
    result = np.ndarray.__new__(CallbackArray, array.shape, dtype=array.dtype)
    np.asarray(result)[...] = array
    CallbackArray.calls = 0
    return result


def test_hydro_rejects_behavior_bearing_bank_before_callbacks():
    state = replace(field(), microtick=1)
    bad = callback_array(state.bank)
    for operation in (lambda: H.initialize(bad), lambda: H.step(replace(state, bank=bad)),
                      lambda: H.checkpoint(replace(state, bank=bad))):
        with pytest.raises(ValueError):
            operation()
    assert CallbackArray.calls == 0


@pytest.mark.parametrize('name', D.NAMES)
def test_dense_rejects_each_behavior_bearing_array_before_callbacks(name):
    state = S.to_dense(sparse())
    arrays = {key: getattr(state, key) for key in D.NAMES}
    arrays[name] = callback_array(arrays[name])
    bad = replace(state, **{name: arrays[name]})
    for operation in (lambda: D.initialize(4, *(arrays[key] for key in D.NAMES)),
                      lambda: D.step(bad), lambda: D.checkpoint(bad)):
        with pytest.raises(ValueError):
            operation()
    assert CallbackArray.calls == 0


class CallbackInt(int):
    calls = 0

    def __int__(self):
        type(self).calls += 1
        return super().__int__()


class CallbackNumpyInt(np.int64):
    pass


class SpoofIntegerType(type):
    def __hash__(cls):
        return hash(int)

    def __eq__(cls, other):
        return other is int


class SpoofInteger(int, metaclass=SpoofIntegerType):
    pass


@pytest.mark.parametrize('kind', (CallbackInt, CallbackNumpyInt, SpoofInteger, bool, np.bool_))
def test_public_state_boundaries_reject_integer_subclasses_and_booleans(kind):
    h, s = field(), sparse()
    d = S.to_dense(s)
    CallbackInt.calls = 0
    for module, state in ((H, h), (D, d), (S, s)):
        for name, value in (('L', 4), ('microtick', 0)):
            with pytest.raises(ValueError):
                module.step(replace(state, **{name: kind(value)}))
    with pytest.raises(ValueError):
        D.initialize(kind(4), *(getattr(d, name) for name in D.NAMES))
    with pytest.raises(ValueError):
        S.initialize(kind(4), s.carriers, s.edges, 0, 0)
    with pytest.raises(ValueError):
        S.step(replace(s, carriers=(replace(s.carriers[0], credit=kind(1)), *s.carriers[1:])))
    assert CallbackInt.calls == 0


@pytest.mark.parametrize('kind', SAFE_INTEGERS)
def test_concrete_numpy_integer_admission_keeps_exact_continuations(kind):
    for module, state in ((H, field()), (D, S.to_dense(sparse())), (S, sparse())):
        normalized = replace(state, L=kind(4), microtick=kind(1))
        expected = module.step(replace(state, microtick=1))
        actual = module.step(normalized)
        expected_state = expected if module is H else expected[0]
        actual_state = actual if module is H else actual[0]
        assert module.checkpoint(actual_state) == module.checkpoint(expected_state)


def test_state_and_identity_subclasses_cannot_pass_as_complete_data():
    class ForeignField(H.FieldState):
        pass
    class ForeignExchange(D.ExchangeState):
        pass
    class ForeignSparse(S.SparseQ4State):
        pass
    class ForeignString(str):
        pass
    h, s = field(), sparse()
    d = S.to_dense(s)
    for module, bad in ((H, ForeignField(h.L, h.microtick, h.bank)),
                        (D, ForeignExchange(d.L, d.microtick, *(getattr(d, n) for n in D.NAMES))),
                        (S, ForeignSparse(s.L, s.microtick, s.carriers, s.edges, s.origin_code, s.charge_frame))):
        with pytest.raises(ValueError):
            module.step(bad)
    for module, state in ((H, h), (D, d), (S, s)):
        object.__setattr__(state, 'law_id', ForeignString(state.law_id))
        with pytest.raises(ValueError):
            module.step(state)


def test_plain_array_branch_accounts_and_replay_remain_exact():
    base = sparse()
    for phase in range(19):
        state = replace(base, microtick=phase)
        actual, events = S.step(state)
        dense, dense_events = D.step(S.to_dense(state))
        assert actual == S.from_dense(dense)
        assert events == dense_events
        assert D.account_units(dense) == 4 and D.populations(dense) == (2, 2)
        assert actual.microtick == phase + 1
        assert S.step(S.restore(S.checkpoint(state))) == (actual, events)
    for phase in range(2):
        state = replace(field(), microtick=phase)
        after = H.step(state)
        assert H.inventories(after) == H.inventories(state)
        assert H.checkpoint(H.step(H.restore(H.checkpoint(state)))) == H.checkpoint(after)
