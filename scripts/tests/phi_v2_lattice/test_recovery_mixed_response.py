"""Instrument falsifiers; no outcome search or GPU measurements in unit tests."""
from dataclasses import asdict
import json
import numpy as np
import pytest
from phi_v2_lattice import recovery_mixed_response as M
from phi_v2_lattice import state as S, staged as P, geometry as G, native_codec as N
from phi_v2_lattice._proofs import encode


def small(tick=0):
    result = P.initialize(S.blank(5))
    result.microtick = tick
    return result


def test_matrix_and_exact_control_inventory():
    assert len(M.cases()) == 10
    for case in M.cases():
        state = M.prepare(case)
        assert state.microtick == 0 and not state.lattice.s.any() and not state.lattice.ell.any()
        assert int(state.lattice.bank.sum()) == (27 if case.probe else 0)
        assert int((state.lattice.sc != S.BLANK_IDX).sum()) == 3*M.L**3
        assert int((state.lattice.fcc != S.BLANK_IDX).sum()) == 6*M.L**3
        assert P.work_units(state) == 9*M.L**3 + (27 if case.probe else 0)
    assert M.registration()["M1_recovery_certified"] is False


def test_initial_defects_have_exact_stored_owner_support():
    reference = M.prepare(M.cases()[0])
    for case in M.cases()[2::2]:
        delta = M.differences(reference, M.prepare(case))
        assert np.flatnonzero(M.owner_support(delta)).tolist() == [G.site_index(17,8,8,8)]
        assert not delta["bank"].any()


def test_fcc_endpoint_is_not_stored_owner():
    a, b = small(), small()
    owner = G.site_index(5,2,2,2)
    b.lattice.fcc[owner,0,1,0] = S.idx_of(encode(0,1))
    row = M.pair_history([a],[b])[0]
    assert row["owner_support"] == [owner]
    assert row["relation_control_endpoint_support"] == sorted(G.fcc_endpoints(5,owner,0,1))
    assert owner not in row["relation_control_endpoint_support"]


def test_dilation_uses_periodic_moore_neighbors():
    mask = np.zeros(125,dtype=bool); mask[0] = True
    result = M.dilate_periodic(mask,5)
    assert result.sum() == 27
    assert result[G.site_index(5,4,4,4)]
    assert not result[G.site_index(5,2,0,0)]


def test_complete_support_rejects_two_hop_influence_in_pending_record():
    a,b = small(1),small(1)
    a2,b2 = small(2),small(2)
    origin = G.site_index(5,2,2,2)
    b.gate_sc[origin,0] = True
    b2.gate_fcc[G.shift(5,origin,(2,0,0)),0,1] = True
    with pytest.raises(ValueError,match="support exceeds"):
        M.pair_history([a,a2],[b,b2])


def test_identical_initial_states_cannot_spontaneously_diverge():
    a,b = small(1),small(1)
    a2,b2 = small(2),small(2)
    b2.gate_sc[0,0] = True
    with pytest.raises(ValueError,match="support exceeds"):
        M.pair_history([a,a2],[b,b2])


def test_mismatched_clock_and_missing_tick_rejected():
    with pytest.raises(ValueError,match="same domain"):
        M.differences(small(),small(1))
    with pytest.raises(ValueError,match="missing or repeated"):
        M.pair_history([small(),small(2)],[small(),small(2)])


@pytest.mark.parametrize("tick",range(4))
def test_all_physical_stage_bytes_and_events_verified(tick):
    before = small(tick)
    after,events = P.step(before)
    M.verify_transition(before,after,asdict(events))
    after.lattice.s[0] = 1
    with pytest.raises(ValueError,match="complete Python transition"):
        M.verify_transition(before,after,asdict(events))


def test_missing_mandatory_collision_event_rejected():
    before = small(1)
    before.lattice.bank[0,[0,34]] = True
    after,events = P.step(before)
    assert events.collisions
    forged = asdict(events); forged["collisions"] = []
    with pytest.raises(ValueError,match="event list"):
        M.verify_transition(before,after,forged)


@pytest.fixture
def locked(tmp_path,monkeypatch):
    runner = tmp_path/"runner"; runner.write_bytes(b"unit test stand-in; never executed")
    monkeypatch.setattr(M,"RUNNER",runner)
    monkeypatch.setattr(M,"L",3)
    folder = tmp_path/"campaign"
    M.prepare_campaign(folder)
    return folder


def test_lock_requires_complete_source_key_set(locked):
    path = locked/"lock.json"
    lock = json.loads(path.read_text())
    lock["instrument_sha256"].pop("scripts/phi_v2_lattice/recovery_mixed_response.py")
    path.write_text(json.dumps(lock))
    with pytest.raises(ValueError,match="source closure"):
        M.validate_lock(locked)


def test_lock_rejects_input_corruption_even_if_manifest_hash_updated(locked):
    path = locked/"mixed_00.t00.bin"
    st = N.decode(path.read_bytes()); st.lattice.s[0] = 1
    blob = N.encode(st); path.write_bytes(blob)
    lock_path = locked/"lock.json"; lock=json.loads(lock_path.read_text())
    lock["manifest"][0]["initial_sha256"] = M._sha(blob)
    lock["manifest_sha256"] = M._sha(M._json(lock["manifest"]).encode())
    lock_path.write_text(json.dumps(lock))
    with pytest.raises(ValueError,match="deterministic registered"):
        M.validate_lock(locked)


def test_fresh_campaign_refuses_overwrite(locked):
    M.validate_lock(locked)
    with pytest.raises(ValueError,match="fresh empty"):
        M.prepare_campaign(locked)


def test_artifact_resource_cap_is_checked(locked,monkeypatch):
    # Change only the local resource threshold after a valid lock validation.
    M.validate_lock(locked)
    (locked/"oversized.bin").write_bytes(b'x'*500000)
    original = M.registration
    frozen = original()
    monkeypatch.setattr(M,"ARTIFACT_CAP",100000)
    monkeypatch.setattr(M,"registration",lambda:frozen)
    with pytest.raises(ValueError,match="resource cap"):
        M.validate_lock(locked)


def test_execution_receipt_must_bind_preflight_to_lock(locked):
    (locked/"preflight.json").write_text('{"lock_sha256":"stale"}')
    (locked/"execution.json").write_text('{"preflight_sha256":"stale"}')
    with pytest.raises(ValueError,match="receipt chain"):
        M.audit_campaign(locked)
