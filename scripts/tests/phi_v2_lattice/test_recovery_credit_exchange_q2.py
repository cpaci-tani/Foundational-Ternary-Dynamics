"""Bounded lift/observer and campaign-integrity checks, not the full census."""
from dataclasses import replace
from pathlib import Path
import json

import numpy as np
import pytest

from scripts.phi_v2_lattice import recovery_credit_exchange_q2 as q2


def record(stage=0, path=(0, 2), velocities=(1, 4), credits=(0, 0), attempts=(1, 0), color=(1, 0, 1)):
    return q2.QuotientState(stage, color, q2.Record(path, velocities, credits, attempts))


def test_complete_finite_record_alphabet_and_index_chart():
    records = q2.records()
    assert len(records) == len(set(records)) == 6192
    assert {R: sum(len(r.path) == R for r in records) for R in range(3)} == {0: 144, 1: 1728, 2: 4320}
    for stage, color in ((0, (0, 0, 0)), (18, (1, 1, 1))):
        for r in records:
            q = q2.QuotientState(stage, color, r)
            assert q2.decode(q2.encode(q)) == q
    assert q2.encode(q2.decode(q2.STATE_COUNT-1)) == q2.STATE_COUNT-1
    with pytest.raises(TypeError):
        q2.indices()[records[0]] = 99


@pytest.mark.parametrize("q", [record(stage=True), record(stage=19), record(path=(0, 1)),
                               record(credits=(1, 0)), record(attempts=(0, 2)), record(color=(True, 0, 0)),
                               record(velocities=(-1, 0)), record(path=[0, 2])])
def test_reject_illegal_complete_quotient(q):
    with pytest.raises(ValueError):
        q2.normalized(q)


def test_generic_first_hit_includes_unreachable_cycle_and_transients():
    successor = np.array([1, 2, 0, 1, 3, 6, 5, 6], dtype=np.int32)
    target = np.array([False, True, False, False, False, False, False, False])
    hit = q2.first_hits(successor, [[0, 1, 2], [5, 6]], [4, 7, 3], target)
    assert hit.tolist() == [1, 0, 2, 1, 2, -1, -1, -1]
    target[4] = True
    assert q2.first_hits(successor, [[0, 1, 2], [5, 6]], [4, 7, 3], target)[4] == 0


@pytest.mark.parametrize("L", [6, 8, 10])
@pytest.mark.parametrize("axis_order", range(6))
@pytest.mark.parametrize("stage", range(19))
def test_lift_observer_and_complete_runtime_at_seam(L, axis_order, stage):
    from scripts.phi_v2_lattice import credit_exchange_binding as runtime
    # R0, R1 and bent/straight R2; headings and bits vary independently of phase.
    rs = [record(stage, (), (5, 0), (1, 1), (1, 0)),
          record(stage, (1,), (2, 5), (1, 0), (0, 1)),
          record(stage, (1, 1), (0, 4), (0, 0), (0, 0)),
          record(stage, (1, 4), (5, 1), (0, 0), (1, 1))]
    for q in rs:
        point = (0, L-1, 0)
        state = q2.materialize(q, L, point, frame_permutation=axis_order)
        assert q2.observe(state) == q
        before = runtime.checkpoint(state)
        out, dp, _, _ = q2.transition(q)
        actual, _ = runtime.step(state)
        axes = q2.AXES[axis_order]
        physical_dp = [0, 0, 0]
        for a in range(3):
            physical_dp[axes[a]] = dp[a]
        new_point = tuple((point[a]+physical_dp[a]) % L for a in range(3))
        expected = q2.materialize(out, L, new_point, stage+1, axis_order)
        assert runtime.checkpoint(actual) == runtime.checkpoint(expected)
        assert q2.observe(actual) == out
        assert runtime.checkpoint(state) == before
        assert runtime.account_units(actual) == 2
        for name in q2.ARRAY_NAMES:
            assert getattr(actual, name).flags.owndata
            assert not np.shares_memory(getattr(actual, name), getattr(state, name))


@pytest.mark.parametrize("stage", range(19))
def test_huge_ordinal_and_same_clock_complete_conjugation(stage):
    from scripts.phi_v2_lattice import credit_exchange_binding as runtime
    q = record(stage)
    state = q2.materialize(q, microtick=19*10**5000+stage)
    def conjugate(s):
        return runtime.ExchangeState(s.L, s.microtick, s.direction[:, ::-1].copy(), s.credit[:, ::-1].copy(),
                                     -s.flux.copy(), s.matching_frame.copy(), 1-s.charge_frame.copy(),
                                     s.attempted[:, ::-1].copy())
    out, _ = runtime.step(state)
    restored, _ = runtime.step(runtime.restore(runtime.checkpoint(state)))
    assert runtime.checkpoint(out) == runtime.checkpoint(restored)
    conjugate_out, _ = runtime.step(conjugate(state))
    assert runtime.checkpoint(conjugate_out) == runtime.checkpoint(conjugate(out))
    assert out.microtick == state.microtick+1


def test_integral_normalization_precedes_negative_displacements():
    original = q2.QuotientState(np.uint64(7), (np.uint8(0),)*3,
        q2.Record((), (np.uint8(1), np.uint8(1)), (np.uint8(1), np.uint8(1)), (np.uint8(0), np.uint8(0))))
    assert q2.normalized(original) == q2.QuotientState(7, (0, 0, 0), q2.Record((), (1, 1), (1, 1), (0, 0)))
    assert q2.transition(original) == q2.transition(q2.normalized(original))


def test_failed_range_preserves_completed_prefix(monkeypatch):
    from scripts.phi_v2_lattice import credit_exchange_binding as runtime
    real_step = runtime.step
    calls = 0
    def failed_second(state):
        nonlocal calls
        calls += 1
        if calls == 2:
            raise RuntimeError("injected second-transition failure")
        return real_step(state)
    monkeypatch.setattr(runtime, "step", failed_second)
    result = q2.verify_runtime_range(0, 3, q2._sha(runtime.__file__))
    assert result["passed"] is False
    assert result["checked"] == result["completed_stop"] == 1
    assert "injected second-transition failure" in result["failure"]


def test_write_once_receipts_and_array_archive(tmp_path):
    path = tmp_path/"receipt.json"
    q2._write_json(path, {"failure": "retained"})
    before = path.read_bytes()
    with pytest.raises(FileExistsError):
        q2._write_json(path, {"passed": True})
    assert path.read_bytes() == before
    arrays = {"signed_displacement": np.array([[1, -1, 0]], dtype=np.int8), "ordinal_phase": np.array([18], dtype="<i4")}
    q2._archive_arrays(tmp_path/"a.npz", arrays)
    q2._archive_arrays(tmp_path/"b.npz", arrays)
    assert (tmp_path/"a.npz").read_bytes() == (tmp_path/"b.npz").read_bytes()


def test_missing_acceptance_cannot_enter_graph_or_create_attempt(tmp_path, monkeypatch):
    from scripts.phi_v2_lattice import credit_exchange_binding as runtime
    receipt = tmp_path/"rejected.json"
    q2._write_json(receipt, {"schema": "credit-exchange-q2-acceptance-1", "verdict": "PENDING"})
    def prohibited():
        raise AssertionError("graph must not execute")
    monkeypatch.setattr(q2, "graph", prohibited)
    output = tmp_path/"attempt"
    with pytest.raises(ValueError, match="unaccepted"):
        q2.certify(output, q2._sha(runtime.__file__), receipt, q2._sha(receipt))
    assert not output.exists()
