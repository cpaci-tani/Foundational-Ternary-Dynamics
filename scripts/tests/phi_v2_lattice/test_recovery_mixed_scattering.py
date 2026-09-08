"""Heterogeneous protocol preparation/provenance tests; no outcome tuning."""
from dataclasses import asdict
import json
import numpy as np
import pytest
from phi_v2_lattice import recovery_mixed_scattering as W
from phi_v2_lattice import recovery_mixed_response as M
from phi_v2_lattice import staged as P, state as S, native_codec as N


def test_exact_heterogeneous_matrix_and_unmodified_relation_controls():
    assert len(W.cases()) == 10 and W.HORIZON == 32
    for case in W.cases():
        st = W.prepare(case)
        assert st.microtick == 0 and st.lattice.L == 17
        reference = M.prepare(next(c for c in M.cases() if c.defect == case.defect and not c.probe))
        for name in N.NAMES:
            if name != 'bank':
                assert np.array_equal(M.arrays(st)[name], M.arrays(reference)[name])
        assert int(st.lattice.bank.sum()) == (54 if case.probe else 0)
        if case.probe:
            assert np.flatnonzero(st.lattice.bank.any(axis=0)).tolist() == [0,32]
            assert np.count_nonzero(st.lattice.bank.sum(axis=1)==2) == 27
    assert W.registration()['M1_recovery_certified'] is False


def test_shared_readonly_full_state_support_and_transition_helpers():
    assert W.pair_history is M.pair_history and W.verify_transition is M.verify_transition
    st = P.initialize(S.blank(3)); st.microtick = 1
    st.lattice.bank[0,[0,32]] = True
    after, events = P.step(st)
    W.verify_transition(st,after,asdict(events))
    assert len(events.collisions) == 1
    assert tuple(events.collisions[0][2]) != tuple(events.collisions[0][3])
    forged = asdict(events); forged['collisions'] = []
    with pytest.raises(ValueError,match='event list'):
        W.verify_transition(st,after,forged)


def test_explicit_source_closure_extends_frozen_previous_protocol():
    assert W.instrument_paths() >= M.instrument_paths()
    assert len(W.instrument_paths()-M.instrument_paths()) == 5
    assert 'scripts/phi_v2_lattice/recovery_mixed_scattering.py' in W.instrument_paths()


@pytest.fixture
def locked(tmp_path,monkeypatch):
    runner=tmp_path/'runner'; runner.write_bytes(b'unit test only; never executed')
    monkeypatch.setattr(W,'RUNNER',runner)
    folder=tmp_path/'campaign'; W.prepare_campaign(folder)
    return folder


def test_lock_checks_new_instrument_and_snapshot_bytes(locked):
    W.validate_lock(locked)
    path=locked/'scatter_01.t00.bin'
    state=N.decode(path.read_bytes()); state.lattice.bank[0,0]=True
    path.write_bytes(N.encode(state))
    with pytest.raises(ValueError,match='preparation bytes'):
        W.validate_lock(locked)


def test_missing_source_closure_entry_rejected(locked):
    path=locked/'lock.json'; lock=json.loads(path.read_text())
    lock['instrument_sha256'].pop('scripts/phi_v2_lattice/recovery_mixed_scattering.py')
    path.write_text(json.dumps(lock))
    with pytest.raises(ValueError,match='source closure'):
        W.validate_lock(locked)


def test_receipt_not_bound_to_lock_rejected(locked):
    (locked/'preflight.json').write_text('{"lock_sha256":"wrong"}')
    (locked/'execution.json').write_text('{"preflight_sha256":"wrong"}')
    with pytest.raises(ValueError,match='receipt chain'):
        W.audit_campaign(locked)


def test_reject_overwrite_of_existing_protocol(locked):
    with pytest.raises(ValueError,match='fresh empty'):
        W.prepare_campaign(locked)
