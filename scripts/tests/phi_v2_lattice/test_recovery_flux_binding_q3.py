"""Finite Q3 representation, complete array lifts, and controlled exclusions."""
from dataclasses import replace
import hashlib
import json
from itertools import product
from pathlib import Path

import numpy as np
import pytest

from scripts.phi_v2_lattice import recovery_flux_binding_q3 as Q


def test_relative_census_and_all_node_encodings():
    rs = Q.records()
    assert len(rs) == len(set(rs)) == 7776
    assert [sum(len(r.path) == k for r in rs) for k in (1, 2, 3)] == [216, 2160, 5400]
    paths = {r.path for r in rs if len(r.path) == 3}
    assert len(paths) == 150
    assert sum(sum(abs(x) for x in Q.endpoint(p)) == 1 for p in paths) == 24
    for i in range(Q.STATE_COUNT):
        assert Q.encode(Q.decode(i)) == i


@pytest.mark.parametrize("L", (8, 10))
def test_each_geometry_and_frame_order_against_complete_runtime(L):
    from scripts.phi_v2_lattice import balanced_matching as R
    for index, record in enumerate(Q.records()[::36]):
        record = replace(record, velocities=(index % 6, (index*5+1) % 6))
        q = Q.QuotientState(index % 12, Q.COLORS[(index//12) % 8], record)
        for frame in range(6):
            axes = Q.AXES[frame]
            point = (L-1, 0, 1)
            before = Q.materialize(q, L, point, frame_permutation=frame)
            assert Q.observe(before) == q
            out, shift, _, exchange = Q.transition(q)
            actual, _ = R.step(before)
            physical_shift = tuple(shift[axes.index(a)] for a in range(3))
            expected = Q.materialize(out, L, tuple((point[a]+physical_shift[a]) % L for a in range(3)),
                                     q.phase+1, frame)
            assert actual.microtick == expected.microtick
            assert Q.observe(actual) == out
            for name in Q.ARRAY_NAMES:
                np.testing.assert_array_equal(getattr(actual, name), getattr(expected, name))
            assert R.account_units(actual) == 3
            assert R.populations(actual) == (1, 1)
            assert exchange[6]**2+exchange[8] == exchange[7]**2+exchange[9]


def test_observed_color_comes_from_background_not_coordinate():
    from scripts.phi_v2_lattice import balanced_matching as R
    q = Q.QuotientState(0, (0, 0, 0), Q.Record((2,), (0, 1), (1, 1)))
    base = Q.materialize(q, 8, (0, 0, 0))
    shifted = Q.materialize(q, 8, (1, 0, 0))
    assert Q.observe(base) == Q.observe(shifted) == q
    a, _ = R.step(base)
    b, _ = R.step(shifted)
    assert Q.observe(a) == Q.observe(b)
    for name in Q.ARRAY_NAMES:
        array = getattr(a, name)
        shape = (8, 8, 8) + array.shape[1:]
        translated = np.roll(array.reshape(shape), 1, axis=0).reshape(array.shape)
        np.testing.assert_array_equal(translated, getattr(b, name))


def test_large_ordinal_replay_for_all_phases_and_colors():
    from scripts.phi_v2_lattice import balanced_matching as R
    for phase, color in product(range(12), Q.COLORS):
        q = Q.QuotientState(phase, color, Q.records()[7000])
        tick = 12*10**5000+phase
        state = Q.materialize(q, 8, (7, 0, 7), tick)
        restored = R.restore(R.checkpoint(state))
        actual, _ = R.step(restored)
        out, dp, _, _ = Q.transition(q)
        expected = Q.materialize(out, 8, tuple((p+d) % 8 for p, d in zip((7, 0, 7), dp)), tick+1)
        assert actual.microtick == tick+1
        assert Q.observe(actual) == out
        for name in Q.ARRAY_NAMES:
            np.testing.assert_array_equal(getattr(actual, name), getattr(expected, name))


def test_runtime_receipt_checks_whole_outputs_and_pin():
    from scripts.phi_v2_lattice import balanced_matching as R
    digest = hashlib.sha256(Path(R.__file__).read_bytes()).hexdigest()
    receipt = Q.verify_runtime_range(19230, 19265, digest)
    assert receipt["checked"] == 35
    with pytest.raises(ValueError, match="registered"):
        Q.verify_runtime_range(0, 1, "0"*64)


@pytest.mark.parametrize("changes", (
    {"phase": True}, {"phase": 12}, {"color": (0, 0, 2)}, {"color": [0, 0, 0]},
    {"record": Q.Record((), (0, 0), (1, 1))},
    {"record": Q.Record((0, 1, 0), (0, 0), (0, 0))},
    {"record": Q.Record((0,), (0, 0), (0, 1))},
    {"record": Q.Record((0, 0, 0), (0, False), (0, 0))},
    {"record": Q.Record((0, 0, 0, 0), (0, 0), (0, 0))},
))
def test_invalid_quotient_rejected(changes):
    with pytest.raises(ValueError):
        Q.transition(replace(Q.decode(0), **changes))


@pytest.mark.parametrize("kwargs", ({"L": 6}, {"L": 9}, {"microtick": 1},
                                      {"positive_position": (8, 0, 0)}, {"frame_permutation": 6}))
def test_invalid_lift_rejected(kwargs):
    with pytest.raises(ValueError):
        Q.materialize(Q.decode(0), **kwargs)


def test_exact_minimum_flow_length_is_not_adjacency():
    q = Q.QuotientState(0, (0, 0, 0), Q.Record((0, 2, 1), (0, 0), (0, 0)))
    assert len(q.record.path) == 3
    assert Q.endpoint(q.record.path) == (0, 1, 0)
    assert Q.observe(Q.materialize(q)) == q


def test_hold_exchange_records_existing_common_flux():
    q = Q.QuotientState(0, (0, 0, 0), Q.Record((0,), (2, 0), (1, 1)))
    out, plus, minus, exchange = Q.transition(q)
    assert plus == minus == (0, 0, 0)
    assert out.record == q.record
    assert exchange == (0, 1, 0, 0, 0, 0, 1, 1, 1, 1)


@pytest.mark.parametrize("integer", (int, np.int64, np.uint64))
def test_admitted_integer_types_support_negative_hop(integer):
    q = Q.QuotientState(integer(1), (integer(0),)*3,
                        Q.Record((integer(2),), (integer(1), integer(0)), (integer(1), integer(1))))
    Q.validate(q)
    expected = Q.QuotientState(1, (0, 0, 0), Q.Record((2,), (1, 0), (1, 1)))
    assert Q.transition(q) == Q.transition(expected)
    assert Q.transition(q)[1] == (-1, 0, 0)
    assert type(Q.encode(q)) is int
    assert Q.observe(Q.materialize(q)) == expected


def _test_acceptance(tmp_path):
    """Synthetic test-only files; never used as scientific acceptance."""
    sources = {}
    for name in ("balanced_matching.py", "recorded_matching.py", "flux_binding.py"):
        relative = "scripts/phi_v2_lattice/"+name
        p = tmp_path/relative
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_text("test fixture "+name, encoding="utf-8")
        sources[relative] = Q._sha(p)
    audit = tmp_path/"audit.md"
    audit.write_text("synthetic test fixture", encoding="utf-8")
    receipt = {"schema": "strict-balanced-matching-acceptance-1", "law_id": Q.LAW_ID,
               "runtime_sha256": sources["scripts/phi_v2_lattice/balanced_matching.py"],
               "verdict": "PASS_SCOPED_FINITE_RUNTIME", "canonical_adoption": False,
               "gates": {k: "PASS" for k in ("local_causality", "accounting", "spatial_covariance",
                                             "balanced_conjugation", "checkpoint_replay")},
               "source_sha256": sources, "audit_path": "audit.md", "audit_sha256": Q._sha(audit)}
    path = tmp_path/"acceptance.json"
    Q._write_json(path, receipt)
    return path, receipt


@pytest.mark.parametrize("corruption", ("receipt_hash", "failed_gate", "source", "audit", "law", "missing_source"))
def test_campaign_rejects_unaccepted_or_changed_upstream(tmp_path, corruption):
    path, receipt = _test_acceptance(tmp_path)
    digest = Q._sha(path)
    assert len(Q._accepted_upstream(tmp_path, path, digest, receipt["runtime_sha256"])) == 5
    if corruption == "receipt_hash":
        digest = "0"*64
    elif corruption == "source":
        (tmp_path/"scripts/phi_v2_lattice/flux_binding.py").write_text("changed", encoding="utf-8")
    elif corruption == "audit":
        (tmp_path/"audit.md").write_text("changed", encoding="utf-8")
    else:
        if corruption == "failed_gate":
            receipt["gates"]["balanced_conjugation"] = "FAIL"
        elif corruption == "law":
            receipt["law_id"] = "different-law"
        else:
            del receipt["source_sha256"]["scripts/phi_v2_lattice/recorded_matching.py"]
        Q._write_json(path, receipt)
        digest = Q._sha(path)
    with pytest.raises(ValueError):
        Q._accepted_upstream(tmp_path, path, digest, receipt["runtime_sha256"])


def test_completed_and_failed_ranges_are_immediately_retained(tmp_path, monkeypatch):
    directory = tmp_path/"ranges"

    def controlled(start, stop, digest):
        if start:
            assert (directory/f"{start-1:07d}.json").is_file()
        if start == 1:
            raise AssertionError("injected mismatch")
        return {"start": start, "stop": stop, "checked": stop-start}

    monkeypatch.setattr(Q, "verify_runtime_range", controlled)
    rows = Q._collect_ranges([(0, 1), (1, 2), (2, 3)], 1, "unused-test-pin", directory)
    assert [r["status"] for r in rows] == ["PASS", "FAIL", "PASS"]
    assert sum(r["checked"] for r in rows) == 2
    assert "injected mismatch" in rows[1]["error"]
    assert rows == [json.loads(p.read_text()) for p in sorted(directory.glob("*.json"))]


def test_campaign_retains_parent_failure_and_post_run_drift(tmp_path, monkeypatch):
    from scripts.phi_v2_lattice import balanced_matching as R
    sentinel = tmp_path/"test-only-pinned-input"
    sentinel.write_text("before", encoding="utf-8")
    monkeypatch.setattr(Q, "_accepted_upstream", lambda *args: [sentinel])

    def failed_graph():
        sentinel.write_text("after", encoding="utf-8")
        raise RuntimeError("injected graph failure")

    monkeypatch.setattr(Q, "graph", failed_graph)
    output = tmp_path/"attempt"
    with pytest.raises(ValueError, match="incomplete"):
        Q.certify(output, Q._sha(R.__file__), "unused-test-receipt", "unused-test-pin", workers=1)
    report = json.loads((output/"report.json").read_text())
    assert not report["engineering_complete"]
    assert report["runtime_states_checked"] == 0
    assert report["source_drift"] == [str(sentinel)]
    assert "injected graph failure" in report["failure"]
    assert report["artifact_sha256"]["lock.json"] == Q._sha(output/"lock.json")
    with pytest.raises(ValueError, match="preserve every attempt"):
        Q.certify(output, Q._sha(R.__file__), "unused-test-receipt", "unused-test-pin", workers=1)
