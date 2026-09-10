"""Bounded admission checks for passive exact observations; no physics claims."""
from types import SimpleNamespace

import numpy as np
import pytest

from phi_v2_lattice import resolved as R, staged as P, state as S, tick as T


def test_integer_tensor_preserves_concrete_integer_and_boolean_values():
    values = [True, False, np.bool_(True), -(2 ** 90), 2 ** 100]
    values.extend(kind(3) for kind in P._INTEGER_TYPES)
    tensor = R.IntTensor.freeze(np.array(values, dtype=object))
    assert tensor.shape == (len(values),)
    assert tensor.values == tuple(int(value) for value in values)
    assert all(type(value) is int for value in tensor.values)
    assert R.IntTensor.freeze([[-1, np.uint64(2 ** 64 - 1)]]).values == (-1, 2 ** 64 - 1)


@pytest.mark.parametrize("value", [1.5, 1.0, np.float64(2), 1 + 0j, "3", None])
def test_integer_tensor_rejects_lossy_or_noninteger_leaves(value):
    with pytest.raises(ValueError):
        R.IntTensor.freeze(np.array([value], dtype=object))


def test_integer_tensor_does_not_invoke_user_integer_conversion():
    class ForeignInteger:
        def __int__(self):
            raise AssertionError("foreign integer conversion must not run")

    class IntegerSubclass(int):
        def __int__(self):
            raise AssertionError("integer subclass conversion must not run")

    for value in (ForeignInteger(), IntegerSubclass(3)):
        with pytest.raises(ValueError):
            R.IntTensor.freeze(np.array([value], dtype=object))
        with pytest.raises(ValueError):
            R.IntTensor.freeze([[value]])


def test_integer_tensor_does_not_invoke_foreign_array_conversion():
    class ForeignArray:
        def __array__(self, *args, **kwargs):
            raise AssertionError("foreign array conversion must not run")

    with pytest.raises(ValueError):
        R.IntTensor.freeze(ForeignArray())


def test_reference_l2_observations_and_transfers_remain_available():
    before = S.blank(2)
    after = S.copy(before)
    observation = R.restrict(before, 1)
    assert observation.counts.lattice_size == 2
    assert observation.microtick is None
    transfer = R.boundary_transfers(before, after, T.TickEvents(), 1, start_tick=0)
    assert transfer.clock == "reference_cycle"
    assert transfer.residual() == (0,) * 8


def test_staged_observations_and_transfers_retain_controls_and_clock():
    before = P.initialize(S.blank(3))
    after, events = P.step(before)
    after.gate_sc[0, 0] = True  # Admissible pending control at phase one.
    observation = R.restrict(after, 1)
    assert (observation.microtick, observation.phase) == (1, 1)
    assert next(row for row in observation.pending if row.name == "gate_sc").data.values[0] == 1
    transfer = R.boundary_transfers(before, after, events, 1)
    assert (transfer.clock, transfer.start_tick, transfer.stop_tick) == ("staged_microtick", 0, 1)


def test_foreign_law_wrappers_are_rejected_at_both_transfer_endpoints():
    before = P.initialize(S.blank(3))
    after, events = P.step(before)
    foreign_before = SimpleNamespace(**vars(before), phase=before.phase, law_id="foreign-law")
    foreign_after = SimpleNamespace(**vars(after), phase=after.phase, law_id="foreign-law")
    for foreign in (foreign_before, foreign_after):
        with pytest.raises(ValueError):
            R.restrict(foreign, 1)
    for initial, final in ((foreign_before, after), (before, foreign_after),
                           (foreign_before, foreign_after)):
        with pytest.raises(ValueError):
            R.boundary_transfers(initial, final, events, 1)


def test_state_subclasses_are_not_admitted_as_the_declared_law():
    class ReferenceSubclass(S.LatticeState):
        pass

    class StagedSubclass(P.StagedState):
        pass

    base = S.blank(3)
    staged = P.initialize(base)
    for foreign in (ReferenceSubclass(**vars(base)), StagedSubclass(**vars(staged))):
        with pytest.raises(ValueError):
            R.restrict(foreign, 1)


def test_invalid_staged_controls_are_rejected_before_observation():
    invalid = P.initialize(S.blank(3))
    invalid.gate_sc[0, 0] = True  # Phase zero must have cleared controls.
    valid = P.initialize(S.blank(3))
    valid.microtick = 1
    with pytest.raises(ValueError):
        R.restrict(invalid, 1)
    with pytest.raises(ValueError):
        R.boundary_transfers(invalid, valid, T.TickEvents(), 1)
    assert invalid.gate_sc[0, 0]  # Rejection does not repair caller data.
