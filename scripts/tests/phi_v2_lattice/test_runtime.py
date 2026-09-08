import json
from dataclasses import FrozenInstanceError, replace

import pytest

from phi_v2_lattice import prepare as P, runtime as R, staged


def test_observations_and_zoom_do_not_advance_or_replace_state():
    runtime = R.StrictRuntime(P.isolated_relation(4))
    before = runtime.checkpoint()
    fine, blocks = runtime.observe(1), runtime.observe(2)
    assert runtime.checkpoint() == before
    assert fine.generation == blocks.generation == 0
    assert fine.tick_end == blocks.tick_end == 0
    assert sum(fine.payload.relation_tokens) == sum(blocks.payload.relation_tokens) == 1
    with pytest.raises(FrozenInstanceError):
        fine.phase = 2


def test_same_owner_replay_across_all_microtick_phases():
    runtime = R.StrictRuntime(P.isolated_relation(4))
    for phase in range(4):
        assert runtime.diagnostics()["phase"] == phase
        saved = runtime.checkpoint()
        obs = runtime.observe(2)
        runtime.advance(5)
        expected = runtime.checkpoint()
        runtime.restore(saved)
        assert runtime.observe(2).generation > obs.generation
        runtime.advance(5)
        assert runtime.checkpoint() == expected
        runtime.restore(saved)
        runtime.advance(1)


def test_batch_failure_never_commits_partial_evolution(monkeypatch):
    runtime = R.StrictRuntime(P.isolated_relation(4))
    before, generation = runtime.checkpoint(), runtime.diagnostics()["generation"]
    original = staged.step
    calls = 0

    def fail_second(state, *args, **kwargs):
        nonlocal calls
        calls += 1
        if calls == 2:
            raise ValueError("injected stage failure")
        return original(state, *args, **kwargs)

    monkeypatch.setattr(staged, "step", fail_second)
    with pytest.raises(ValueError):
        runtime.advance(2)
    assert runtime.checkpoint() == before
    assert runtime.diagnostics()["generation"] == generation


def test_wire_encoding_keeps_large_integer_ticks_exact():
    observation = R.StrictRuntime(P.isolated_relation(4)).observe(2)
    big = 2 ** 65 + 1
    observation = replace(observation, tick_start=big, tick_end=big, generation=big)
    wire = json.loads(json.dumps(observation.to_wire()))
    assert wire["tick_end"] == str(big)
    assert wire["generation"] == str(big)
    assert wire["payload"]["width"] == "2"
    assert wire["physical_calibration"] == "unidentified"


@pytest.mark.parametrize("ticks", [-1, .5, True, "4"])
def test_invalid_advance_leaves_owner_unchanged(ticks):
    runtime = R.StrictRuntime(P.isolated_relation(4))
    before = runtime.checkpoint()
    with pytest.raises(ValueError):
        runtime.advance(ticks)
    assert runtime.checkpoint() == before


def test_unavailable_physics_does_not_fall_back_to_effective_engines():
    runtime = R.StrictRuntime(P.isolated_relation(4))
    for name in ("particles", "atoms", "planets", "continuum"):
        with pytest.raises(ValueError):
            runtime.observe(1, name)
    for key in ("autonomous_coarse_evolution", "continuum_trajectory_certificate",
                "physical_units", "gravity_recovery", "canonical_adoption"):
        assert runtime.capabilities()[key] is False


def test_invalid_restore_keeps_generation_and_state():
    runtime = R.StrictRuntime(P.isolated_relation(4))
    before, diagnostics = runtime.checkpoint(), runtime.diagnostics()
    with pytest.raises((ValueError, TypeError)):
        runtime.restore(b"invalid")
    assert runtime.checkpoint() == before
    assert runtime.diagnostics() == diagnostics


def test_overlapping_advances_are_serialized_without_losing_a_tick(monkeypatch):
    from concurrent.futures import ThreadPoolExecutor
    from threading import Event

    runtime = R.StrictRuntime(P.isolated_relation(4))
    first_entered, release_first, second_entered, second_attempted = (Event() for _ in range(4))
    original = staged.step

    def paused_first(state, *args, **kwargs):
        if not first_entered.is_set():
            first_entered.set()
            assert release_first.wait(5)
        else:
            second_entered.set()
        return original(state, *args, **kwargs)

    def second_call():
        second_attempted.set()
        return runtime.advance(1)

    monkeypatch.setattr(staged, "step", paused_first)
    with ThreadPoolExecutor(max_workers=2) as pool:
        first = pool.submit(runtime.advance, 1)
        try:
            assert first_entered.wait(5)
            second = pool.submit(second_call)
            assert second_attempted.wait(5)
            assert not second_entered.wait(.1)
        finally:
            release_first.set()
        first.result(timeout=5)
        second.result(timeout=5)
    assert runtime.diagnostics()["microtick"] == 2
    assert runtime.diagnostics()["generation"] == 2


def test_resolved_wire_uses_same_state_lineage_and_keeps_pending_masks():
    runtime = R.StrictRuntime(P.isolated_relation(4))
    runtime.advance(1)
    counts, resolved = runtime.observe(2), runtime.observe(2, "resolved")
    assert (counts.generation, counts.tick_end, counts.phase) == (
        resolved.generation, resolved.tick_end, resolved.phase)
    encoded = json.loads(json.dumps(resolved.to_wire()))
    assert encoded["observable"] == "resolved"
    assert encoded["tick_end"] == "1"


def test_distinct_owners_cannot_alias_even_at_equal_generation_and_tick():
    a = R.StrictRuntime(P.isolated_relation(4))
    b = R.StrictRuntime(P.isolated_relation(4))
    first, second = a.observe(2), b.observe(2)
    assert first.owner_id != second.owner_id
    assert first.generation == second.generation == 0
    # Publication identity is external; identical physical states serialize identically.
    assert a.checkpoint() == b.checkpoint()
    saved = a.checkpoint()
    a.advance(2)
    a.restore(saved)
    after = a.observe(2)
    assert after.owner_id == first.owner_id
    assert after.generation > first.generation
    assert after.to_wire()["owner_id"] == first.owner_id
    assert a.diagnostics()["owner_id"] == first.owner_id
