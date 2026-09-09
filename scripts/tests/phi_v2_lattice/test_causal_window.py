"""Finite completion controls, not exhaustive enumeration of lattice states."""
from dataclasses import FrozenInstanceError
import json

import numpy as np
import pytest

from phi_v2_lattice import causal_window as W, checkpoint_integrity as I
from phi_v2_lattice import staged as P, state as S


def prepared(side, phase, seed):
    rng = np.random.default_rng(seed)
    lattice = S.blank(side)
    lattice.s[:] = rng.integers(-1, 2, lattice.s.shape)
    lattice.ell[:] = rng.integers(0, 3, lattice.ell.shape)
    lattice.bank[:] = rng.random(lattice.bank.shape) < .01
    lattice.sc[:] = rng.integers(0, 9, lattice.sc.shape)
    lattice.fcc[:] = rng.integers(0, 9, lattice.fcc.shape)
    # A guaranteed admission/collision witness, alongside both FCC orientations.
    center = (side // 2 * side + side // 2) * side + side // 2
    lattice.bank[center] = False
    lattice.bank[center, [0, 34]] = True
    lattice.sc[center] = S.BLANK_IDX
    state = P.initialize(lattice)
    for _ in range(phase):
        state, _ = P.step(state)
    return state


def records(state):
    side = state.lattice.L
    return {name: (array := getattr(state.lattice if i < 5 else state, name)).reshape(
                (side,) * 3 + array.shape[1:]) for i, name in enumerate(W.NAMES)}


@pytest.mark.parametrize("phase", range(4))
@pytest.mark.parametrize("outer_seed", [7301, 7302])
def test_every_record_matches_different_larger_completions(phase, outer_seed):
    small = prepared(7, phase, 7300)
    larger = prepared(9, phase, outer_seed)
    for name, array in records(larger).items():
        array[1:8, 1:8, 1:8] = records(small)[name]
    P.validate(larger)
    initial_work = P.work_units(small)
    window = W.CausalWindow(small, origin=(-20, 2**70, 10))
    for elapsed in range(1, 4):
        larger, _ = P.step(larger)
        small, _ = P.step(small)
        window = window.advance()
        observation = window.observe()
        for name, array in records(larger).items():
            np.testing.assert_array_equal(observation.record(name),
                                          array[1+elapsed:8-elapsed, 1+elapsed:8-elapsed, 1+elapsed:8-elapsed])
        assert P.work_units(small) == initial_work
        assert observation.tick_interval == (phase + elapsed,) * 2
        assert observation.generation == elapsed
    assert window.remaining_microticks == 0
    with pytest.raises(ValueError, match="halo"):
        window.advance()


@pytest.mark.parametrize("phase", range(4))
def test_restart_nested_restriction_and_input_ownership(phase):
    state = prepared(7, phase, 7390)
    window = W.CausalWindow(state)
    before = window.checkpoint()
    state.lattice.bank[:] = False
    assert window.checkpoint() == before
    expected = window.advance(3)
    halfway = window.advance(1)
    restored = W.CausalWindow.restore(halfway.checkpoint())
    assert restored.checkpoint() == halfway.checkpoint()
    assert restored.advance(2).checkpoint() == expected.checkpoint()
    observation = halfway.observe()
    direct = halfway.observe((3, 3, 3), (4, 4, 4))
    nested = observation.restrict((2, 2, 2), (5, 5, 5)).restrict((3, 3, 3), (4, 4, 4))
    assert nested == direct
    with pytest.raises(ValueError):
        observation.record("bank").flat[0] = True
    with pytest.raises(ValueError):
        observation.record("bank").setflags(write=True)
    with pytest.raises(FrozenInstanceError):
        observation.microtick = 0
    with pytest.raises(AttributeError):
        halfway._tick = 0
    assert halfway.checkpoint() == restored.checkpoint()


def test_boundary_exhaustion_and_batch_failure_never_change_input(monkeypatch):
    window = W.CausalWindow(prepared(7, 0, 7400))
    before = window.checkpoint()
    with pytest.raises(ValueError, match="halo"):
        window.advance(4)
    original = P.step
    calls = 0

    def fail_second(state):
        nonlocal calls
        calls += 1
        if calls == 2:
            raise ValueError("injected step failure")
        return original(state)

    monkeypatch.setattr(P, "step", fail_second)
    with pytest.raises(ValueError, match="injected"):
        window.advance(2)
    assert window.checkpoint() == before
    with pytest.raises(ValueError):
        window.observe((0, 0, 0), (8, 7, 7))
    assert window.advance(0) is window


def test_checkpoint_corruption_and_false_bounds_are_rejected():
    window = W.CausalWindow(P.initialize(S.blank(5))).advance(1)
    raw = window.checkpoint()
    for index in [0, 8, 16, I.HEADER.size, len(raw) - 1]:
        altered = bytearray(raw)
        altered[index] ^= 1
        with pytest.raises(ValueError):
            W.CausalWindow.restore(bytes(altered))
    for altered in [raw[:-1], raw + b"x", b"", bytearray(raw)]:
        with pytest.raises(ValueError):
            W.CausalWindow.restore(altered)
    original = json.loads(I.unpack(raw))
    changes = [{"initial_tick": 2}, {"initial_tick": -1}, {"initial_tick": True},
               {"origin": [True, 0, 0]}, {"origin": [0, 0]},
               {"schema": "other"}, {"boundary": "periodic"}, {"extra": 0}]
    for change in changes:
        payload = {**original, **change}
        with pytest.raises(ValueError):
            W.CausalWindow.restore(I.pack(json.dumps(payload).encode()))
    duplicate = I.unpack(raw).replace(b'"initial_tick":0', b'"initial_tick":0,"initial_tick":0')
    with pytest.raises(ValueError, match="duplicate"):
        W.CausalWindow.restore(I.pack(duplicate))


def test_integrity_envelope_detects_valid_alphabet_corruption():
    state = P.initialize(S.blank(3))
    original = P.checkpoint(state)
    envelope = I.pack(original)
    assert I.unpack(envelope) == original
    # Both payloads are valid law checkpoints, but changing one into the other
    # inside the old envelope must fail rather than silently change the state.
    state.lattice.bank[0, 0] = True
    changed = P.checkpoint(state)
    assert len(changed) == len(original)
    with pytest.raises(ValueError, match="digest"):
        I.unpack(envelope[:I.HEADER.size] + changed)


def test_window_behavior_cannot_expand_bounds_without_checkpoint_changes():
    with pytest.raises(TypeError, match="subclasses"):
        class Wider(W.CausalWindow):
            @property
            def bounds(self):
                return (0, 0, 0), (5, 5, 5)

    window = W.CausalWindow(P.initialize(S.blank(5))).advance(1)
    saved = window.checkpoint()
    with pytest.raises(AttributeError, match="one-shot"):
        window._set(window._snapshot, (0, 0, 0), 1, 1, 5)
    with pytest.raises(AttributeError, match="deleted"):
        del window._snapshot
    with pytest.raises(ValueError, match="concrete"):
        W.CausalWindow._from_snapshot.__func__(object, window._snapshot, (0, 0, 0), 0)
    with pytest.raises(ValueError, match="certified"):
        window.observe((0, 0, 0), (5, 5, 5))
    assert window.checkpoint() == saved
